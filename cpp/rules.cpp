/*  rules.cpp : rules implementation
	$Revision: 16 $
		Implements classes in rules.h
	Columbia Optimizer Framework

	A Joint Research Project of Portland State University 
	   and the Oregon Graduate Institute
	Directed by Leonard Shapiro and David Maier
	Supported by NSF Grants IRI-9610013 and IRI-9619977

	
*/

#include "stdafx.h"
#include "tasks.h"
#include "physop.h"
#include "cat.h"

#define NUMOFRULES 20		// Number of elements in the enum RULELABELS in rules.h
#define LINEWIDTH 256		// buffer length of one text line

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

// use to turn some rules on/off in the optimizer

int RuleVector[NUMOFRULES];

//##ModelId=3B0C0868032A
RULE_SET::RULE_SET(CString filename) : RuleCount(NUMOFRULES)
{
	//read the RuleVector value from filename
	FILE *fp;		// file handle
	char TextLine[LINEWIDTH]; // text line buffer
	char *p;
	int rule_count=0;
	
	if((fp = fopen(filename,"r"))==NULL) 
		OUTPUT_ERROR("can not open file 'ruleset'");
	
	for(;;)
	{
		fgets(TextLine,LINEWIDTH,fp);
		if (feof(fp)) break;
		
		// skip the comment line
		if(IsCommentOrBlankLine(TextLine)) continue;
		p = SkipSpace(TextLine);
		
		//if the vectors are filled before end of file, raise an error
		if (rule_count >= NUMOFRULES) 
		{
			OUTPUT_ERROR("Rule Set File error!");
		}
		
		parseString(p);
		RuleVector[rule_count] = atoi(p);
		
		rule_count++;
	}
	// if the file ends before the vectors are filled, set the rest to off
	if (rule_count < NUMOFRULES)
	{
		for (; rule_count<NUMOFRULES; rule_count++)
			RuleVector[rule_count] = 0;
	}
	
	fclose(fp);
	
	rule_set = new RULE* [RuleCount];
	
	// file-scan implements get	    
	rule_set[R_GET_TO_FILE_SCAN] = new GET_TO_FILE_SCAN();		
	
	// filter implements select
	rule_set[R_SELECT_TO_FILTER] = new SELECT_TO_FILTER(); 
	
	// physical project implements project
	rule_set[R_P_TO_PP] = new P_TO_PP();
	
	// index loops join implements eqjoin
	rule_set[R_EQ_TO_LOOPS_INDEX] = new EQ_TO_LOOPS_INDEX();
	
	// sort merge implements eqjoin
	rule_set[R_EQ_TO_MERGE] = new EQ_TO_MERGE();
	
	// LOOPS JOIN implements EQJOIN
	rule_set[R_EQ_TO_LOOPS] = new EQ_TO_LOOPS();
	
	// sort enforcer rule
	rule_set[R_SORT_RULE] = new SORT_RULE();
	
	// Commute eqjoin
	rule_set[R_EQJOIN_COMMUTE] = new EQJOIN_COMMUTE();
	
	// Associativity of EQJOIN
	rule_set[R_EQJOIN_LTOR] = new EQJOIN_LTOR();
	
	// Associativity of EQJOIN
	rule_set[R_EQJOIN_RTOL] = new EQJOIN_RTOL();
	
	// Cesar's exchange rule
	rule_set[R_EXCHANGE] = new EXCHANGE();
	
	// hash_duplicates implements rm_duplicates
	rule_set[R_RM_TO_HASH_DUPLICATES] = new RM_TO_HASH_DUPLICATES(); 
	
	// hgroup_list implements agg_list
	// agg_ops1 and agg_ops2 just hold a place
	AGG_OP_ARRAY *agg_ops1 = new AGG_OP_ARRAY;
	AGG_OP_ARRAY *agg_ops2 = new AGG_OP_ARRAY;
	rule_set[R_AL_TO_HGL] = new AL_TO_HGL(agg_ops1, agg_ops2); 
	
	// p_func_op implements func_op
	rule_set[R_FO_TO_PFO] = new FO_TO_PFO(); 
	
	// agg_thru_eq push agg_list below eqjoin
	// agg_ops3 and agg_ops4 just hold a place
	AGG_OP_ARRAY *agg_ops3 = new AGG_OP_ARRAY;
	AGG_OP_ARRAY *agg_ops4 = new AGG_OP_ARRAY;
	rule_set[R_AGG_THRU_EQJOIN] = new AGG_THRU_EQJOIN(agg_ops3, agg_ops4); 
	
	// eq_to_bit EQJOIN to BIT_SEMIJOIN
	rule_set[R_EQ_TO_BIT] = new EQ_TO_BIT(); 
	
	// selectet_to_indexed filter
	rule_set[R_SELECT_TO_INDEXED_FILTER] = new SELECT_TO_INDEXED_FILTER();
	
	// project_through_select
	rule_set[R_PROJECT_THRU_SELECT] = new PROJECT_THRU_SELECT();
	
	// EQJOIN to HASH JOIN
	rule_set[R_EQ_TO_HASH] = new EQ_TO_HASH();
	
	// DUMMY to PDUMMY
	rule_set[R_DUMMY_TO_PDUMMY] = new DUMMY_TO_PDUMMY();
}; // rule set

//##ModelId=3B0C0868033E
RULE_SET::~RULE_SET()
{
	for(int i=0; i<RuleCount; i++) delete rule_set[i];
	delete rule_set;
}

//##ModelId=3B0C08680348
CString RULE_SET::Dump()
{
	CString os;
	CString temp;
	
	for(int i=0; i<RuleCount; i++) 
	{
		temp.Format(" %d  %s\r\n", RuleVector[i],rule_set[i]->GetName() );
		os += temp;
	}
	return os;
}

#ifdef _DEBUG
#ifndef _TABLE_
//##ModelId=3B0C08680352
CString RULE_SET::DumpStats()
{
	CString os;
	CString temp;
	
	temp.Format("%s", "Rule#\tTopMatch\t Bindings\tConditions\r\n");
	os += temp;
	for(int i=0; i<RuleCount; i++) 
	{
		temp.Format("%d\t%d\t%d\t%d\t%s\r\n",
			i, TopMatch[i], Bindings[i], Conditions[i], rule_set[i]->GetName() );
		
		os += temp;
	}
	return os;
}
#endif
#endif

// ====================

//##ModelId=3B0EA6DA0280
BINDERY::BINDERY (GRP_ID group_no, EXPR * original)
:state(start), group_no(group_no), cur_expr(NULL),
original(original), input(NULL), 
one_expr(FALSE)               // try all expressions within this group
{
    ASSERT (original);
	
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_BINDERY].New();
} // BINDERY::BINDERY

//##ModelId=3B0EA6DA0262
BINDERY::BINDERY (M_EXPR * expr,  EXPR *  original)
:state(start), cur_expr(expr), original(original), input(NULL), 
one_expr(TRUE)	// restricted to this log expr            
{
    group_no = expr -> GetGrpID();
    ASSERT (original);
	
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_BINDERY].New();
	
} // BINDERY::BINDERY


//##ModelId=3B0EA6DA0294
BINDERY::~BINDERY ()
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_BINDERY].Delete();
	
	if(input!=NULL) 
	{
		for(int i=0; i<original->GetOp()->GetArity(); i++) delete input[i];
		delete [] input;
	}
	
}; // BINDERY::~BINDERY


//##ModelId=3B0EA6DA02A8
EXPR * BINDERY::extract_expr ()
{
    EXPR * result;
	
    // If original pattern is NULL something weird is happening.
    assert(original);
	
    OP * patt_op = original -> GetOp();
	
    // Ensure that there has been a binding, so there is an 
    // expression to extract.
    assert(state == valid_binding || state == finished ||
		(patt_op -> is_leaf () && state == start) );
	
    // create leaf marked with group index
    if (patt_op -> is_leaf ())
    {
        result = new EXPR (
			new LEAF_OP(((LEAF_OP *)patt_op)->GetIndex(), group_no));
    } // create leaf marked with group index
    else // general invocation of new EXPR
    {
		//Top operator in the new EXPR will be top operator in cur_expr.  
		//Get it.  (Probably could use patt_op here.)
        OP * op_arg = cur_expr->GetOp()->Clone();
		
		//Need the arity of the top operator to construct inputs of new EXPR
        int arity = op_arg -> GetArity();
		
        // Inputs of new EXPR can be extracted from binderys stored in
		// BINDERY::input.  Put these in the array subexpr.
		if(arity)
		{
			EXPR ** subexpr = new EXPR* [arity];
			for (int input_no = 0;  input_no < arity; input_no++ )
				subexpr[input_no] = input[input_no]->extract_expr();
			
			// Put everything together for the result.
			result = new EXPR (op_arg, subexpr);
		}
		else
			result = new EXPR (op_arg);
		
    } // general invocation of new EXPR
	
    return  result;
} // BINDERY::extract_expr


/*
    Function BINDERY::advance() walks the many trees embedded in the
    MEMO structure in order to find possible bindings.  It is called
    only by APPLY_RULE::perform.  The walking is done with a finite
    state machine, as follows.

    State start:
	If the original pattern is a leaf, we are done.  
		State = finished
		Return TRUE
	Skip over non-logical, non-matching expressions.  
		State = finished
		break
	Create a group bindery for each input and 
	   try to create a binding for each input.
	If successful
		State = valid_binding
		Return TRUE
	else
		delete input binderys
		State = finished
		break


    State valid_binding:
	Increment input bindings in right-to-left order.
	If we found a next binding, 
		State = valid_binding
		return TRUE
	else
		delete input binderys
		state = finished
		break


    State finished
	If original pattern is a leaf //second time through, so we are done
	   OR 
	   this is an expr bindery //we finished the first expression, so done
	   OR
	   there is no next expression
		return FALSE
	else
		state = start
		break
		   
*/
//##ModelId=3B0EA6DA029E
bool BINDERY::advance ()
{
#ifdef _REUSE_SIB
	///XXXX Leave these comments alone - fix later
	OP  * patt_op = original -> GetOp(); 
	// If the original pattern is a leaf, we will get one binding, 
	//   to the entire group, then we will be done
	if (patt_op -> is_leaf ())
	{
		switch(state)
		{
		case start:		
			state = finished;		//failure next time, but 
			return true;			// success now
			
		case finished:
			return false;
			
		default:
			assert(false);
		}
	}// if (patt_op -> is_leaf ())
	
    if( !one_expr && state == start )   // begin the group binding
	{  //Search entire group for bindings
		cur_expr = Ssp->GetGroup(group_no)->GetFirstLogMExpr(); // get the first mexpr
		
    }
	
    // loop until either failure or success
    for (;;)
    {
		// cache some function results
		OP  * op_arg = cur_expr -> GetOp();
		int arity = op_arg -> GetArity();
		int input_no;
		
		assert( op_arg->is_logical() ); 
		
        // state analysis and transitions
        switch (state)
        {
		case start:
			// is this expression unusable?
			if ( arity != patt_op->GetArity() ||
				!( patt_op->GetNameId() == op_arg->GetNameId())
				)	
			{
				state = finished;  // try next expression
				break;
			}
			
			if(arity==0)	// only the Operator, matched
			{
				state = valid_binding;
				return true;
			} // successful bindings for the Operator without inputs
			else
			{
				// Create a group bindery for each input
				input = new BINDERY* [arity];
				for (input_no = 0; input_no < arity; input_no ++ )
				{
					input[input_no] =  new BINDERY 
						( cur_expr -> GetInput(input_no), 
						original -> GetInput(input_no)) ;
				}
				// Try to advance each (new) input bindery to a binding
				// a failure is failure for the expr
				for (input_no = 0;  input_no < arity;  input_no ++ )
					if ( ! input[input_no]->advance() )
						break; // terminate this loop
					
					// check whether all inputs found a binding
					if (input_no == arity ) // successful!
					{
						state = valid_binding;
						return true;
					} // successful bindings for new expression
					else 
					{ // otherwise, failure! -- dealloc inputs
						test_delete (arity);
						state = finished;
						break;
					}
			}// if(arity)
			
		case valid_binding:
			for (input_no= arity; --input_no >= 0;)
			{
				if (currentBind == NULL)
					// try existing inputs in right-to-left order
					// first success is overall success
				{
					if ( input[input_no]->advance() )
					{
						for (int other_input_no=input_no; ++ other_input_no < arity;)
						{
							// input[other_input_no] = get_first_bindery_in_list;
							// currentBind->bindery = input[other_input_no];
							input[other_input_no] = list->bindery;
							currentBind = list;
						}
						state = valid_binding;
					}
				}
				else
				{
					currentBind = currentBind->next;
					if ( currentBind != NULL )
					{
						state = valid_binding;
						input[input_no] = currentBind->bindery;
						// input[input_no] = get_next_bindery_in_list;
						return true;
					}
					else
						return false;
				}
			}
			
			if ( arity != 0 && !one_expr )
			{
				Node *newNode = new Node();
				BINDERY *dup = new BINDERY(this);
				newNode->bindery = dup;
				if (list == NULL)
					list = last = newNode;
				else
				{
					last->next = newNode;
					last = last->next;
				}
				// add_to_the_list (this bindery);
			}
			state = finished;
			break;
			
		case finished :
			if( one_expr ||
				((cur_expr = cur_expr->GetNextMExpr()) == NULL ) 
				)	
				return false;
			else 
			{
				state = start;
				break;
			}
			
		default :
			assert (false);
			
        } // state analysis and transitions
    } // loop until either failure or success
	
    assert(false); // should never terminate this loop
#else
	OP  * patt_op = original -> GetOp(); 
	// If the original pattern is a leaf, we will get one binding, 
	//   to the entire group, then we will be done
	if (patt_op -> is_leaf ())
	{
		switch(state)
		{
		case start:		
			state = finished;		//failure next time, but 
			return true;			// success now
			
		case finished:
			return false;
			
		default:
			assert(false);
		}
	}// if (patt_op -> is_leaf ())
	
    if( !one_expr && state == start )   // begin the group binding
	{  //Search entire group for bindings
		cur_expr = Ssp->GetGroup(group_no)->GetFirstLogMExpr(); // get the first mexpr
		
    }
	
    // loop until either failure or success
    for (;;)
    {
		//PTRACE ("advancing the cur_expr: %s", cur_expr->Dump() );
		
		// cache some function results
		OP  * op_arg = cur_expr -> GetOp();
		int arity = op_arg -> GetArity();
		int input_no;
		
		assert( op_arg->is_logical() ); 
		
        // state analysis and transitions
        switch (state)
        {
		case start:
			
			// is this expression unusable?
			if ( arity != patt_op->GetArity() ||
				!( patt_op->GetNameId() == op_arg->GetNameId())
				)	
			{
				state = finished;  // try next expression
				break;
			}
			
			if(arity==0)	// only the Operator, matched
			{
				state = valid_binding;
				return true;
			} // successful bindings for the Operator without inputs
			else
			{
				// Create a group bindery for each input
				input = new BINDERY* [arity];
				for (input_no = 0; input_no < arity; input_no ++ )
					input[input_no] =  new BINDERY 
					( cur_expr -> GetInput(input_no), 
					original -> GetInput(input_no)) ;
				
				// Try to advance each (new) input bindery to a binding
				// a failure is failure for the expr
				for (input_no = 0;  input_no < arity;  input_no ++ )
					if ( ! input[input_no]->advance() )
						break; // terminate this loop
					
					// check whether all inputs found a binding
					if (input_no == arity ) // successful!
					{
						state = valid_binding;
						return true;
					} // successful bindings for new expression
					else 
					{ // otherwise, failure! -- dealloc inputs
						for (input_no = arity;  -- input_no >= 0; )
							delete input[input_no];
						delete [] input;  input = NULL;
						state = finished;
						break;
					}
			}// if(arity)
			
		case valid_binding :
			// try existing inputs in right-to-left order
			// first success is overall success
			for (input_no = arity;  -- input_no >= 0; )
			{
				if ( input[input_no]->advance() )
					// found one more binding
				{
					// If we have a new binding in a non-rightmost location,
					// we must create new binderys for all inputs to the
					// right of input_no, else we will not get all bindings.
					//  This is inefficient code since the each input on the
					//  right has multiple binderys created for it, and each
					//  bindery produces the same bindings as the others.
					//  The simplest example of this is the exchange rule.
					for (int other_input_no = input_no;
					++ other_input_no < arity;  )
					{
						delete input[other_input_no];
						input[other_input_no] = (new BINDERY (
							cur_expr->GetInput(other_input_no),
							original ->GetInput(other_input_no)) );
						
						if (! input[other_input_no]->advance() )
							// Impossible since we found these bindings earlier
							ASSERT(false) ; 
					}
					
					// return overall success
					state = valid_binding;
					return true;
				} // found one more binding
			} // try existing inputs in right-to-left order
			
			// There are no more bindings to this log expr; 
			//   dealloc input binderys.
			if(arity)
			{
				for (input_no = arity; -- input_no >= 0;  )
					delete input[input_no];
				delete [] input; input = NULL;
			}
			state = finished;
			break;
			
		case finished :
			
			if( one_expr ||
				((cur_expr = cur_expr->GetNextMExpr()) == NULL ) 
				)	
				return false;
			else 
			{
				state = start;
				break;
			}
			
		default :
			assert (false);
			
        } // state analysis and transitions
    } // loop until either failure or success
	
    assert(false); // should never terminate this loop
#endif
} // BINDERY::advance


#ifdef _REUSE_SIB
//##ModelId=3B0EA6DA024E
void BINDERY::test_delete (int arity) {
	int input_no;
	for (input_no = arity;  -- input_no >= 0; )
		//	for (;  -- input_no >= 0; )
		delete input[input_no];
	delete [] input;  input = NULL;
}
#endif


/*
Rule  Get -> File-scan
====  === == ====-====
*/
//##ModelId=3B0C086A00CB
GET_TO_FILE_SCAN::GET_TO_FILE_SCAN ()
: RULE ("GET_TO_FILE_SCAN", 0,
		new EXPR (new GET (0)),
		new EXPR (new FILE_SCAN (0) ) ) 
{
	// set rule index
	set_index(R_GET_TO_FILE_SCAN);
	
}; // GET_TO_FILE_SCAN::GET_TO_FILE_SCAN

//##ModelId=3B0C086A00DE
EXPR * GET_TO_FILE_SCAN::next_substitute ( EXPR * before, PHYS_PROP * ReqdProp)
{
    EXPR * result;
	
    // create transformed expression
    result = new EXPR (new FILE_SCAN ( ((GET *)before->GetOp())->GetCollection() ) );
	
    return result;
}; // GET_TO_FILE_SCAN::next_substitute

/*
   Rule  EQJOIN  -> LOOPS JOIN
   ====  ======  == ====  ====
*/

//##ModelId=3B0C086A0161
EQ_TO_LOOPS::EQ_TO_LOOPS ()
    : RULE ("EQJOIN->LOOPS_JOIN", 2,
	    new EXPR (new EQJOIN (0,0,0),
					  new EXPR (new LEAF_OP (0)),
					  new EXPR (new LEAF_OP (1))
				  ),
	     new EXPR (new LOOPS_JOIN (0,0,0),
					  new EXPR (new LEAF_OP (0)),
					  new EXPR (new LEAF_OP (1))
				  )
		    )
{
	// set rule index
	set_index(R_EQ_TO_LOOPS);
	
} // EQ_TO_LOOPS::EQ_TO_LOOPS

//##ModelId=3B0C086A0174
EXPR * EQ_TO_LOOPS::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
{
	EQJOIN * Op = (EQJOIN *) before->GetOp();
	int size =  Op->size;
    int * lattrs = CopyArray ( Op->lattrs, size);
    int * rattrs = CopyArray ( Op->rattrs, size);
	
    // create transformed expression
    EXPR * result = new EXPR (new LOOPS_JOIN (lattrs, rattrs, size),
		new EXPR(*(before->GetInput(0))), 
		new EXPR(*(before->GetInput(1)))
					   );
    return  result ;
} // EQ_TO_LOOPS::next_substitute
#ifdef CONDPRUNE
//Is the plan a goner because an input is group pruned?
//##ModelId=3B0C086A017E
bool EQ_TO_LOOPS::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
{
	COST inputs = *( Ssp->GetGroup(mexpr->GetInput(0))->GetLowerBd() );
	inputs += *( Ssp->GetGroup(mexpr->GetInput(1))->GetLowerBd() );
	
	if ( inputs >= *( CONT::vc[ContextID]->GetUpperBd() ) ) 
		return(false);
	
	return(true);
} //EQ_TO_LOOPS::condition
#endif

/*
  Rule  EQJOIN  -> LOOPS INDEX JOIN
  ====  ======  == ====  ===== ====
*/

//##ModelId=3B0C086A0342
EQ_TO_LOOPS_INDEX::EQ_TO_LOOPS_INDEX ()
    : RULE ("EQJOIN -> LOOPS_INDEX_JOIN", 1,
	    new EXPR (new EQJOIN (0, 0, 0),
			new EXPR (new LEAF_OP (0)),
			new EXPR (new GET(0))
			),
	    new EXPR (new LOOPS_INDEX_JOIN (0, 0, 0, 0),
			new EXPR (new LEAF_OP (0))
			)
		)
{
	// set rule index
	set_index(R_EQ_TO_LOOPS_INDEX);
	
} // EQ_TO_LOOPS_INDEX::EQ_TO_LOOPS_INDEX

//##ModelId=3B0C086A034B
EXPR * EQ_TO_LOOPS_INDEX::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
{
    EXPR * result;
	
	EQJOIN * Op = (EQJOIN *) before->GetOp();
	int size =  Op->size;
    int * lattrs = CopyArray ( Op->lattrs, size);
    int * rattrs = CopyArray ( Op->rattrs, size);
	
    // Get the GET logical operator in order to get the indexed collection 
    GET *g =  (GET *)before->GetInput(1)->GetOp();
	
    // create transformed expression
    result = new EXPR (new LOOPS_INDEX_JOIN (lattrs, rattrs, size, g->GetCollection()),
		new EXPR (*(before->GetInput(0)))
		);
	
    return ( result );
} // EQ_TO_LOOPS_INDEX::next_substitute

//  Need to check:
//	that the join is on a attribute from each table (not a multi-attribute
// 	join)
//	there is an index for the right join attribute -- need log-bulk-props
//##ModelId=3B0C086A0355
bool EQ_TO_LOOPS_INDEX::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
{
    // Get the GET logical operator in order to get the indexed collection 
    GET *g =  (GET *)before->GetInput(1)->GetOp();
	INT_ARRAY * Indices = Cat->GetIndNames(g->GetCollection());
	
	if(Indices == NULL ) return false;
	
    int * rattrs = ((EQJOIN *) before -> GetOp()) -> rattrs ;
	int size = ((EQJOIN *) before -> GetOp()) -> size ;
	
    // Loop thru indices
    for (int i=0; i < Indices->GetSize(); i++) 
		
		if ( size == 1 && Cat->GetIndProp((*Indices)[i])->Keys->ContainKey(rattrs[0])) 
			return ( true );
		
		return ( false );
		
		
} // EQ_TO_LOOPS_INDEX::condition

/*
  Rule  EQJOIN  -> MERGE JOIN
  ====  ======  == ===== ====
*/

//##ModelId=3B0C086A0214
EQ_TO_MERGE::EQ_TO_MERGE ()
    : RULE ("EQJOIN -> MERGE_JOIN", 2,
	    new EXPR (new EQJOIN (0,0,0),
			new EXPR (new LEAF_OP (0)),
			new EXPR (new LEAF_OP (1))
			),
	    new EXPR (new MERGE_JOIN (0,0,0),
			new EXPR (new LEAF_OP (0)),
			new EXPR (new LEAF_OP (1))
			)
		)
{
	// set rule index
	set_index(R_EQ_TO_MERGE);
} // EQ_TO_MERGE::EQ_TO_MERGE

//##ModelId=3B0C086A0215
int EQ_TO_MERGE::promise (OP* op_arg, int ContextID)
{
	// if the merge-join attributes set is empty, don't fire this rule
	int result = ( ((EQJOIN*)op_arg)->size == 0 ) ? 0 : MERGE_PROMISE ;
	
    return ( result );
} // EQ_TO_MERGE::promise 


//##ModelId=3B0C086A0228
EXPR * EQ_TO_MERGE::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
{
    EXPR * result;
	
	EQJOIN * Op = (EQJOIN *) before->GetOp();
	int size =  Op->size;
    int * lattrs = CopyArray ( Op->lattrs, size);
    int * rattrs = CopyArray ( Op->rattrs, size);
	
    // create transformed expression
    result = new EXPR (new MERGE_JOIN (lattrs, rattrs,size),
		new EXPR(*(before->GetInput(0))), 
		new EXPR(*(before->GetInput(1)))
		);
	
    return ( result );
} // EQ_TO_MERGE::next_substitute

#ifdef CONDPRUNE
//Is the plan a goner because an input is group pruned?
//##ModelId=3B0C086A0232
bool EQ_TO_MERGE::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
{
	COST inputs = *( Ssp->GetGroup(mexpr->GetInput(0))->GetLowerBd() );
	inputs += *( Ssp->GetGroup(mexpr->GetInput(1))->GetLowerBd() );
	
	if ( inputs >= *( CONT::vc[ContextID]->GetUpperBd() ) ) 
		return(false);
	
	return(true);
} //EQ_TO_MERGE::condition
#endif

/*
  Rule  EQJOIN  -> HASH JOIN
  ====  ======  == ===== ====
*/

//##ModelId=3B0C086A02B4
EQ_TO_HASH::EQ_TO_HASH ()
    : RULE ("EQJOIN->HASH_JOIN", 2,
	    new EXPR (new EQJOIN (0,0,0),
			new EXPR (new LEAF_OP (0)),
			new EXPR (new LEAF_OP (1))
			),
	    new EXPR (new HASH_JOIN (0,0,0),
			new EXPR (new LEAF_OP (0)),
			new EXPR (new LEAF_OP (1))
			)
		)
{
	// set rule index
	set_index(R_EQ_TO_HASH);
} // EQ_TO_HASH::EQ_TO_HASH

//##ModelId=3B0C086A02B5
int EQ_TO_HASH::promise (OP* op_arg, int ContextID)
{
	// if the hash-join attributes set is empty, don't fire this rule
	int result = ( ((EQJOIN*)op_arg)->size == 0 ) ? 0 : HASH_PROMISE ;
	
    return ( result );
} // EQ_TO_HASH::promise 

//##ModelId=3B0C086A02C8
EXPR * EQ_TO_HASH::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
{
    EXPR * result;
	
	EQJOIN * Op = (EQJOIN *) before->GetOp();
	int size =  Op->size;
    int * lattrs = CopyArray ( Op->lattrs, size);
    int * rattrs = CopyArray ( Op->rattrs, size);
	
    // create transformed expression
    result = new EXPR (new HASH_JOIN (lattrs, rattrs,size),
		new EXPR(*(before->GetInput(0))), 
		new EXPR(*(before->GetInput(1)))
		);
	
    return ( result );
} // EQ_TO_HASH::next_substitute

/*
  Rule  EQJOIN(A,B)  -> EQJOIN(B,A)
  ====  ===========  == ===========
*/
//##ModelId=3B0C086A03CE
EQJOIN_COMMUTE::EQJOIN_COMMUTE ()
    : RULE ("EQJOIN_COMMUTE", 2,
		new EXPR (new EQJOIN (0,0,0),
					  new EXPR (new LEAF_OP (0)),
					  new EXPR (new LEAF_OP (1))
				  ),
	    new EXPR (new EQJOIN (0,0,0),
					  new EXPR (new LEAF_OP (1)),
					  new EXPR (new LEAF_OP (0))
				  )
			 )
{ 
	
	// set rule mask and index
	set_index(R_EQJOIN_COMMUTE);
    set_mask( 1 << R_EQJOIN_COMMUTE |
		1 << R_EQJOIN_LTOR |
		1 << R_EQJOIN_RTOL |
		1 << R_EXCHANGE
		);
	
} // EQJOIN_COMMUTE::EQJOIN_COMMUTE

//##ModelId=3B0C086A03D8
EXPR * EQJOIN_COMMUTE::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
{
    // lattrs and rattrs
	EQJOIN * Op = (EQJOIN *) before->GetOp();
	int size =  Op->size;
    int * lattrs = CopyArray ( Op->lattrs, size);
    int * rattrs = CopyArray ( Op->rattrs, size);
	
    // create transformed expression 
    EXPR *result = new EXPR (new EQJOIN ( rattrs, lattrs, size),	// reverse l and r
		new EXPR(*(before->GetInput(1))), 
		new EXPR(*(before->GetInput(0)))
		);
	
    return result ;
} // EQJOIN_COMMUTE::next_substitute

/*
  Rule  EQJOIN (AB) C -> EQJOIN A (BC)
  ====  ============= == =============
*/
// assoc of join left to right
//##ModelId=3B0C086B00A3
EQJOIN_LTOR::EQJOIN_LTOR ()
    : RULE ("EQJOIN_LTOR", 3,
		new EXPR (new EQJOIN (0,0,0),
				      new EXPR (new EQJOIN (0,0,0),
								    new EXPR (new LEAF_OP (0)),	// A
								    new EXPR (new LEAF_OP (1))	// B
								),	
					   new EXPR (new LEAF_OP (2))				// C
				   ),	// original pattern
	    new EXPR (new EQJOIN (0,0,0),
					  new EXPR (new LEAF_OP (0)),
					  new EXPR (new EQJOIN (0,0,0),
								    new EXPR (new LEAF_OP (1)),
								    new EXPR (new LEAF_OP (2))
								)
				  )		// substitute
			)
								
{
	
	// set rule mask and index
	set_index ( R_EQJOIN_LTOR );
    set_mask (	1 << R_EQJOIN_LTOR | 
		1 << R_EQJOIN_RTOL | 
		1 << R_EXCHANGE
		);
	
} // EQJOIN_LTOR::EQJOIN_LTOR

//##ModelId=3B0C086B00AD
EXPR * EQJOIN_LTOR::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
{
	/*
     * Join numbering convention:
     *          1  2             2  1		join number
     * EQJOIN (AxB)xC -> EQJOIN Ax(BxC)		original -> substitute patterns
     */
    // from upper (second) join		
	EQJOIN * Op2 = (EQJOIN *) before->GetOp();
	int size2 = Op2->size;
    int * lattrs2 = Op2->lattrs ;		// equal attr's from left		
    int * rattrs2 = Op2->rattrs ;    
	
    // from lower (first) join
	EQJOIN * Op1 = (EQJOIN *) before->GetInput(0)->GetOp() ;
	int size1 = Op1->size;
    int * lattrs1 = Op1->lattrs ;		// equal attr's from left		
    int * rattrs1 = Op1->rattrs ;    
	
    //	Calculate the new join conditions
	int i;
	// for new upper (second) join
	// first allocate bigger size, then set the new right size after addkeys 		
    int * nrattrs2 = new int [ size1+size2 ]; 
	int * nlattrs2 = new int [ size1+size2 ]; 
	// first copy lattrs1 to nlattrs2
    for (i = 0; i < size1; i++) 
	{
		nlattrs2[i] = lattrs1[i] ;
		nrattrs2[i] = rattrs1[i] ;
	}
	int nsize2=size1;
	
	// for new lower (first) join
	// first allocate bigger size, then set the new right size after addkeys 		
	int * nrattrs1 = new int [ size2 ]; 
	int * nlattrs1 = new int [ size2 ]; 
	int nsize1=0;
	
    //	B's schema determines new joining conditions.  Get it.
    EXPR * AB = before->GetInput(0);
	LEAF_OP * B = (LEAF_OP *) ( AB->GetInput(1)->GetOp() );
    GRP_ID group_no = B->GetGroup();
    GROUP * group = Ssp->GetGroup(group_no);
    SCHEMA *Bs_schema = ((LOG_COLL_PROP *)(group->get_log_prop())) ->Schema;
	
    // See where second join predicates of antecedent go
    for (i=0; i < size2; i++) 
	{
		if ( Bs_schema->InSchema( lattrs2[i] ) ) 
		{
			// lattrs2[i] is in B:  put this eq condition in first join
			nlattrs1[nsize1] = lattrs2[i]; 
			nrattrs1[nsize1] = rattrs2[i]; 
			nsize1 ++;
		} 
		else 
		{
			// lattrs2[i] is in A:  put this eq condition in second join
			nlattrs2[nsize2] = lattrs2[i]; 
			nrattrs2[nsize2] = rattrs2[i]; 
			nsize2 ++;
		}
    }
	
#ifdef _DEBUG
	// Check that each join is legal
	
    // Get C's schema
	LEAF_OP * C = (LEAF_OP *)(before -> GetInput(1) -> GetOp());
    group_no = C -> GetGroup();                
    group = Ssp->GetGroup(group_no);
    SCHEMA *Cs_schema = ((LOG_COLL_PROP *)group->get_log_prop()) ->Schema;
	
    // check that first join is legal
    for (i=0; i < nsize1; i++) 
	{
		assert (Bs_schema->InSchema(nlattrs1[i])); 
		assert (Cs_schema->InSchema(nrattrs1[i]));
    }
	
	//  Get A's schema
    LEAF_OP * A = (LEAF_OP *)( AB -> GetInput(0) -> GetOp() );
    group_no = A -> GetGroup() ;
    group = Ssp->GetGroup(group_no);;
    SCHEMA *As_schema = ((LOG_COLL_PROP *)group->get_log_prop()) -> Schema;
	
    //Check (mostly) that second join is legal
    for(i=0; i < nsize2; i++) 
	{
		assert (As_schema->InSchema(nlattrs2[i])); 
    }
#endif
	
    // create transformed expression
    EXPR * result = new EXPR (new EQJOIN (nlattrs2,nrattrs2,nsize2),
						   new EXPR(*(before->GetInput(0)->GetInput(0))),
						   new EXPR (new EQJOIN (nlattrs1, nrattrs1, nsize1),
										 new EXPR(*(before->GetInput(0)->GetInput(1))),
										 new EXPR(*(before->GetInput(1))) )
						);
    return result;
	
} // EQJOIN_LTOR::next_substitute


 /*
  *	Experimental Heuristic code to try to limit rule explosion
  *		This may eliminate useful join orders
  *		Global variable NoCart determines whether
  *		this condition function is ENABLED or DISABLED
  */
  
  // If we allow a non-Cartesian product to go to a Cartesian product, return true
  // If we do not allow a non-Cartesian product to go to a Cartesian product
  // Need to check:
  //	a. Whether original 2 joins       contained a cartesian product
  //	b. Whether the new  2 joins would contain   a cartesian product
  // Condition is:
  //	a || !b
  //	
//##ModelId=3B0C086B00B7
  bool EQJOIN_LTOR::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
  {
#ifdef NOCART
	  PTRACE("%s\n", "NOCART is On");
	  EQJOIN * Op2 = (EQJOIN *) before->GetOp();
	  int size2 = Op2->size;
	  int * lattrs2 = Op2->lattrs ;		// equal attr's from left		
	  int * rattrs2 = Op2->rattrs ;    
	  
	  // from lower (first) join
	  EQJOIN * Op1 = (EQJOIN *) before->GetInput(0)->GetOp() ;
	  int size1 = Op1->size;
	  int * lattrs1 = Op1->lattrs ;		// equal attr's from left		
	  int * rattrs1 = Op1->rattrs ;    
	  
	  //	Calculate the new join conditions
	  int i;
	  // for new upper (second) join
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs2 = new int [ size1+size2 ]; 
	  int * nlattrs2 = new int [ size1+size2 ]; 
	  // first copy lattrs1 to nlattrs2
	  for (i = 0; i < size1; i++) 
	  {
		  nlattrs2[i] = lattrs1[i] ;
		  nrattrs2[i] = rattrs1[i] ;
	  }
	  int nsize2=size1;
	  
	  // for new lower (first) join
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs1 = new int [ size2 ]; 
	  int * nlattrs1 = new int [ size2 ]; 
	  int nsize1=0;
	  
	  //	B's schema determines new joining conditions.  Get it.
	  EXPR * AB = before->GetInput(0);
	  LEAF_OP * B = (LEAF_OP *) ( AB->GetInput(1)->GetOp() );
	  GRP_ID group_no = B->GetGroup();
	  GROUP * group = Ssp->GetGroup(group_no);
	  SCHEMA *Bs_schema = ((LOG_COLL_PROP *)(group->get_log_prop())) ->Schema;
	  
	  // See where second join predicates of antecedent go
	  for (i=0; i < size2; i++) 
	  {
		  if ( Bs_schema->InSchema( lattrs2[i] ) ) 
		  {
			  // lattrs2[i] is in B:  put this eq condition in first join
			  nlattrs1[nsize1] = lattrs2[i]; 
			  nrattrs1[nsize1] = rattrs2[i]; 
			  nsize1 ++;
		  } 
		  else 
		  {
			  // lattrs2[i] is in A:  put this eq condition in second join
			  nlattrs2[nsize2] = lattrs2[i]; 
			  nrattrs2[nsize2] = rattrs2[i]; 
			  nsize2 ++;
		  }
	  }
	  
	  // Condition is that either one of the original joins had
	  // a cartesian product -- or that neither of the resulting
	  // joins has a cartesian product
	  if (size1 * size2 != 0  && nsize1 * nsize2 == 0)
		  return false;
	  return true;
#endif
	  return true;	
	  
  } // EQJOIN_LTOR::condition
  
	/*
    Rule  EQJOIN A (BC) -> EQJOIN (AB) C
    ====  ============= == =============
    Right to Left Associativity
  */
//##ModelId=3B0C086B016C
  EQJOIN_RTOL::EQJOIN_RTOL ()
    : RULE ("EQJOIN_RTOL", 3,
            new EXPR (new EQJOIN (0,0,0),
						  new EXPR (new LEAF_OP (0)),		//A
						  new EXPR (new EQJOIN (0,0,0),
										new EXPR (new LEAF_OP (1)),	//B
										new EXPR (new LEAF_OP (2))  //C
									)
					  ),	// original pattern
            new EXPR (new EQJOIN (0,0,0),
						  new EXPR (new EQJOIN (0,0,0),
										new EXPR (new LEAF_OP (0)), //A
										new EXPR (new LEAF_OP (1))  //B
									),
						   new EXPR (new LEAF_OP (2))	//C
					  )	// substitute
			   )            
  {
	  
	  // set rule mask and index
	  set_index ( R_EQJOIN_RTOL);
	  set_mask (	1 << R_EQJOIN_LTOR |
		  1 << R_EQJOIN_RTOL |
		  1 << R_EXCHANGE
		  );
  } // EQJOIN_RTOL::EQJOIN_RTOL
  
//##ModelId=3B0C086B0176
  EXPR * EQJOIN_RTOL::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
  /*
   * Join numbering convention:
   *         2  1               1  2		1 is the first join
   * EQJOIN Ax(BxC) -> EQJOIN (AxB)xC
   */
	  // from upper (second) join
	  EQJOIN * Op2 = (EQJOIN *) before->GetOp();
	  int size2 = Op2->size;
	  int * lattrs2 = Op2->lattrs ;		// equal attr's from left		
	  int * rattrs2 = Op2->rattrs ;    
	  
	  // from lower (first) join
	  EQJOIN * Op1 = (EQJOIN *) before->GetInput(1)->GetOp() ;
	  int size1 = Op1->size;
	  int * lattrs1 = Op1->lattrs ;		// equal attr's from left		
	  int * rattrs1 = Op1->rattrs ;    
	  
	  //	Calculate the new join conditions
	  int i;
	  // for new upper (second) join
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs2 = new int [ size1+size2 ]; 
	  int * nlattrs2 = new int [ size1+size2 ]; 
	  // first copy lattrs1 to nlattrs2
	  for (i = 0; i < size1; i++) 
	  {
		  nlattrs2[i] = lattrs1[i] ;
		  nrattrs2[i] = rattrs1[i] ;
	  }
	  int nsize2=size1;
	  
	  // for new lower (first) join
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs1 = new int [ size2 ]; 
	  int * nlattrs1 = new int [ size2 ]; 
	  int nsize1=0;
	  
	  // Get schema for B 
	  EXPR * BC = before->GetInput(1);
	  LEAF_OP * B = (LEAF_OP *) (BC->GetInput(0) -> GetOp());
	  GRP_ID group_no = B->GetGroup();
	  GROUP * group = Ssp->GetGroup(group_no);
	  LOG_PROP * LogProp = group->get_log_prop();	
	  SCHEMA *Bs_schema = ((LOG_COLL_PROP *)LogProp) ->Schema;
	  
	  // See where second join predicates of antecedent go
	  for (i=0; i < size2; i++) 
	  {
		  if ( Bs_schema->InSchema( rattrs2[i] ) ) 
		  {
			  // lattrs2[i] is in B:  put this eq condition in first join
			  nlattrs1[nsize1] = lattrs2[i] ; 
			  nrattrs1[nsize1] = rattrs2[i] ; 
			  nsize1 ++;
		  } 
		  else 
		  {
			  // lattrs2[i] is in A:  put this eq condition in second join
			  nlattrs2[nsize2] = lattrs2[i] ; 
			  nrattrs2[nsize2] = rattrs2[i] ; 
			  nsize2 ++;
		  }
	  }
	  
#ifdef _DEBUG
	  
	  // check (mostly) that first join is legal
	  for (i = 0; i < nsize1; i++) 
	  {
		  assert (Bs_schema ->InSchema(nrattrs1[i]) );
		  //"Bad B join atts"
	  }
	  
	  // Get C's schema
	  LEAF_OP * C = (LEAF_OP *) ( BC->GetInput(1)->GetOp() );
	  group_no = C->GetGroup();
	  group = Ssp->GetGroup(group_no);
	  LogProp = group->get_log_prop();
	  SCHEMA *Cs_schema = ((LOG_COLL_PROP *)LogProp) ->Schema;
	  
	  //Check (mostly) that second join is legal
	  for(i=0; i < nsize2; i++) {
		  assert (Cs_schema ->InSchema(nrattrs2[i]) );
		  //"Bad C join atts"
	  }
#endif
	  
	  // create transformed expression
	  EXPR * result = new EXPR (new EQJOIN (nlattrs2, nrattrs2,nsize2),  // args reversed
		  new EXPR (new EQJOIN (nlattrs1, nrattrs1,nsize1),
										new EXPR(*(before -> GetInput(0))),
										new EXPR(*(before -> GetInput(1) -> GetInput(0)))
										),
										new EXPR(*(before -> GetInput(1) -> GetInput(1)))
										);
	  return  result;
  } // EQJOIN_RTOL::next_substitute
  
   /*
	*	Very experimental code to try to limit rule explosion
	*		This may eliminate very useful join orders
	*		Go to the very end of this function to see if
	*		this condition function is ENABLED or DISABLED
	*/
	// If we allow a non-Cartesian product to go to a Cartesian product, return true
	// If we do not allow a non-Cartesian product to go to a Cartesian product
	// Need to check:
	//	a. Whether original 2 joins       contained a cartesian product
	//	b. Whether the new  2 joins would contain   a cartesian product
	// Condition is:
	//	a || !b
	//	
//##ModelId=3B0C086B0181
	bool EQJOIN_RTOL::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
	{
#ifdef NOCART
		PTRACE("%s\n", "NOCART is On");
		EQJOIN * Op2 = (EQJOIN *) before->GetOp();
		int size2 = Op2->size;
		int * lattrs2 = Op2->lattrs ;		// equal attr's from left		
		int * rattrs2 = Op2->rattrs ;    
		
		// from lower (first) join
		EQJOIN * Op1 = (EQJOIN *) before->GetInput(0)->GetOp() ;
		int size1 = Op1->size;
		int * lattrs1 = Op1->lattrs ;		// equal attr's from left		
		int * rattrs1 = Op1->rattrs ;    
		
		//	Calculate the new join conditions
		int i;
		// for new upper (second) join
		// first allocate bigger size, then set the new right size after addkeys 		
		int * nrattrs2 = new int [ size1+size2 ]; 
		int * nlattrs2 = new int [ size1+size2 ]; 
		// first copy lattrs1 to nlattrs2
		for (i = 0; i < size1; i++) 
		{
			nlattrs2[i] = lattrs1[i] ;
			nrattrs2[i] = rattrs1[i] ;
		}
		int nsize2=size1;
		
		// for new lower (first) join
		// first allocate bigger size, then set the new right size after addkeys 		
		int * nrattrs1 = new int [ size2 ]; 
		int * nlattrs1 = new int [ size2 ]; 
		int nsize1=0;
		
		//	B's schema determines new joining conditions.  Get it.
		EXPR * AB = before->GetInput(0);
		LEAF_OP * B = (LEAF_OP *) ( AB->GetInput(1)->GetOp() );
		GRP_ID group_no = B->GetGroup();
		GROUP * group = Ssp->GetGroup(group_no);
		SCHEMA *Bs_schema = ((LOG_COLL_PROP *)(group->get_log_prop())) ->Schema;
		
		// See where second join predicates of antecedent go
		for (i=0; i < size2; i++) 
		{
			if ( Bs_schema->InSchema( lattrs2[i] ) ) 
			{
				// lattrs2[i] is in B:  put this eq condition in first join
				nlattrs1[nsize1] = lattrs2[i]; 
				nrattrs1[nsize1] = rattrs2[i]; 
				nsize1 ++;
			} 
			else 
			{
				// lattrs2[i] is in A:  put this eq condition in second join
				nlattrs2[nsize2] = lattrs2[i]; 
				nrattrs2[nsize2] = rattrs2[i]; 
				nsize2 ++;
			}
		}
		
		// Condition is that either one of the original joins had
		// a cartesian product -- or that neither of the resulting
		// joins has a cartesian product
		if (size1 * size2 != 0  && nsize1 * nsize2 == 0)
			return false;
		return true;
#endif
		return true;
	} // EQJOIN_RTOL::condition
	
	
/*
	Cesar's EXCHANGE rule: (AxB)x(CxD) -> (AxC)x(BxD)
	  ====  ============= =
*/
//##ModelId=3B0C086B023F
	EXCHANGE::EXCHANGE ()
    : RULE ("EXCHANGE", 4,
            new EXPR (new EQJOIN (0,0,0),
                new EXPR (new EQJOIN (0,0,0),
                    new EXPR (new LEAF_OP (0)),		//A
                    new EXPR (new LEAF_OP (1))),	//B
                new EXPR (new EQJOIN (0,0,0),
                    new EXPR (new LEAF_OP (2)),		//C
                    new EXPR (new LEAF_OP (3)))),	//D

            new EXPR (new EQJOIN (0,0,0),
                new EXPR (new EQJOIN (0,0,0),
                    new EXPR (new LEAF_OP (0)),		//A
                    new EXPR (new LEAF_OP (2))),	//C
                new EXPR (new EQJOIN (0,0,0),
                    new EXPR (new LEAF_OP (1)),		//B
                    new EXPR (new LEAF_OP (3)))))	//D
	{
		// set rule mask and index
		set_index ( R_EXCHANGE);
		set_mask (	1 << R_EQJOIN_COMMUTE |
			1 << R_EQJOIN_LTOR |
			1 << R_EQJOIN_RTOL |
			1 << R_EXCHANGE
			);
	} // EXCHANGE::EXCHANGE
	
//##ModelId=3B0C086B0249
	EXPR * EXCHANGE::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
	{
   /*
	* Join numbering convention:
	*   2  1  3        2  1  3
	* (AxB)x(CxD) -> (AxC)x(BxD)
	*/
		
		// from join 1
		EQJOIN * Op1 = (EQJOIN *) before->GetOp();
		int size1 = Op1->size;
		int * lattrs1 = Op1->lattrs ;		
		int * rattrs1 = Op1->rattrs ;    
		
		// from join 2
		EQJOIN * Op2 = (EQJOIN *) before->GetInput(0)->GetOp();
		int size2 = Op2->size;
		int * lattrs2 = Op2->lattrs ;		
		int * rattrs2 = Op2->rattrs ;    
		
		// from join 3
		EQJOIN * Op3 = (EQJOIN *) before->GetInput(1)->GetOp();
		int size3 = Op3->size;
		int * lattrs3 = Op3->lattrs ;		
		int * rattrs3 = Op3->rattrs ;    
		
/*
    First we factor lattrs1 into its A and B parts,
	lattrs1 = lattrs1A U lattrs1B
    and similarly
	rattrs1 = rattrs1C U rattrs1D

    The new attribute sets for join 1 are given by
	nlattrs1 = lattrs2 U lattrs3 U rattrs1C
	nrattrs1 = rattrs2 U rattrs3 U lattrs1B

    Those for joins 2, 3 are trickier - see the code below.  Approximately,
	nlattrs2 = lattrs1A, nrattrs2 = rattrs1C
	nlattrs3 = lattrs1B, nrattrs3 = rattrs1D
*/
		// for new join 1
		// first allocate bigger size, then set the new right size after addkeys 		
		int * nrattrs1 = new int [ size1+size2+size3 ]; 
		int * nlattrs1 = new int [ size1+size2+size3 ]; 
		int i;
		//Compute nlattrs1, nrattrs1 incrementally, first lattrs2 U lattrs3
		for (i = 0; i < size2; i++) 
		{
			nlattrs1[i] =  lattrs2[i];
			nrattrs1[i] =  rattrs2[i];
		}
		for (i = 0; i < size3; i++) 
		{
			nlattrs1[i+size2] = lattrs3[i];
			nrattrs1[i+size2] = rattrs3[i];
		}
		int nsize1=size2+size3;
		
		// for new join 2
		// first allocate bigger size, then set the new right size after addkeys 		
		int * nrattrs2  = new int [ size1 ]; 
		int * nlattrs2  = new int [ size1 ]; 
		int nsize2 = 0;
		
		// for new join 3
		// first allocate bigger size, then set the new right size after addkeys 		
		int * nrattrs3  = new int [ size1 ]; 
		int * nlattrs3  = new int [ size1 ]; 
		int nsize3 = 0;
		
		// Get schema for A
		EXPR * AB = before->GetInput(0);
		LEAF_OP * AA = (LEAF_OP *) AB->GetInput(0)->GetOp() ;
		GRP_ID  group_no = AA->GetGroup();
		GROUP * group = Ssp->GetGroup(group_no);
		LOG_PROP * log_prop = group->get_log_prop();	
		SCHEMA *AAA = ((LOG_COLL_PROP *)log_prop) ->Schema;
		
		// Get schema for C
		EXPR * CD = before->GetInput(1);
		LEAF_OP * CC = (LEAF_OP *) CD->GetInput(0)->GetOp() ;
		group_no = CC->GetGroup();
		group = Ssp->GetGroup(group_no);
		log_prop = group->get_log_prop();	
		SCHEMA *CCC = ((LOG_COLL_PROP *)log_prop) ->Schema;
		
		//If a pair of attributes in (lattrs1,rattrs1), is in (A,C),
		//then the pair should be in (nlattrs2,nrattrs2).
		//If the pair is in (B,D), put it in in (nlattrs3,nrattrs3).
		//If it is in (A, D), put it in (nlattrs1, nrattrs1).
		//Otherwise go to (nlattrs1, nrattrs1), but reversed.
		
		for ( i=0; i < size1; i++) 
		{
			int lkey = lattrs1[i];
			int rkey = rattrs1[i];
			if (
				AAA->InSchema(lkey) && //lattrs1 in A
				CCC->InSchema(rkey) 	//rattrs1 in C
				)
			{
				nlattrs2[nsize2] = lkey;	//Add to nlattrs2
				nrattrs2[nsize2] = rkey;	//Add to nrattrs2
				nsize2++;
			}
			else if (
				!(AAA->InSchema(lkey)) &&   //lattrs1 in B
				!(CCC->InSchema(rkey))      //rattrs1 in D
				)
			{
				nlattrs3[nsize3] =  lkey;	//Add to nlattrs3
				nrattrs3[nsize3] =  rkey;	//Add to nrattrs3
				nsize3++;
			}
			else if(
				(AAA->InSchema(lkey)) &&    //lattrs1 in A
				!(CCC->InSchema(rkey))      //rattrs1 in D
				)
			{
				nlattrs1[nsize1] = lkey;	//add to nlattrs1
				nrattrs1[nsize1] = rkey;    //add to nrattrs1
				nsize1++;
			}
			else                            //lattrs1 in B and rattrs1 in C
			{
				nlattrs1[nsize1] = rkey;	//add to nlattrs1, reversed
				nrattrs1[nsize1] = lkey;    //add to nrattrs1, reversed
				nsize1++;
			}
		}
		
		// create transformed expression
		EXPR * result = 
			new EXPR(new EQJOIN (nlattrs1, nrattrs1,nsize1),	
				new EXPR (new EQJOIN (nlattrs2, nrattrs2,nsize2),
						new EXPR ( *(before->GetInput(0)->GetInput(0)) ), //A
						new EXPR ( *(before->GetInput(1)->GetInput(0)) )  //C
					),        
					new EXPR (new EQJOIN (nlattrs3, nrattrs3,nsize3),
						new EXPR ( *(before->GetInput(0)->GetInput(1)) ), //B
            			new EXPR ( *(before->GetInput(1)->GetInput(1)) )  //D
						)
				 );  
		
		return result;
} // EXCHANGE::next_substitute

 /*
  *	Very experimental code to try to limit rule explosion
  *		This may eliminate very useful join orders
  *		Go to the very end of this function to see if
  *		this condition function is ENABLED or DISABLED
  */
  
  // If we allow a non-Cartesian product to go to a Cartesian product, return true
  // If we do not allow a non-Cartesian product to go to a Cartesian product
  // Need to check:
  //	a. Whether original 3 joins       contained a cartesian product
  //	b. Whether the new  3 joins would contain   a cartesian product
  // Condition is:
  //	a || !b
  //	
//##ModelId=3B0C086B025C
  bool EXCHANGE::condition (EXPR * before,M_EXPR *mexpr,int ContextID)
  {
#ifdef NOCART
 /*
  * Join numbering convention:
  *   2  1  3        2  1  3
  * (AxB)x(CxD) -> (AxC)x(BxD)
  */
	  
	  // from join 1
	  EQJOIN * Op1 = (EQJOIN *) before->GetOp();
	  int size1 = Op1->size;
	  int * lattrs1 = Op1->lattrs ;		
	  int * rattrs1 = Op1->rattrs ;    
	  
	  // from join 2
	  EQJOIN * Op2 = (EQJOIN *) before->GetInput(0)->GetOp();
	  int size2 = Op2->size;
	  int * lattrs2 = Op2->lattrs ;		
	  int * rattrs2 = Op2->rattrs ;    
	  
	  // from join 3
	  EQJOIN * Op3 = (EQJOIN *) before->GetInput(1)->GetOp();
	  int size3 = Op3->size;
	  int * lattrs3 = Op3->lattrs ;		
	  int * rattrs3 = Op3->rattrs ;    
	  
	  // for new join 1
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs1 = new int [ size1+size2+size3 ]; 
	  int * nlattrs1 = new int [ size1+size2+size3 ]; 
	  int i;
	  //Compute nlattrs1, nrattrs1 incrementally, first lattrs2 U lattrs3
	  for (i = 0; i < size2; i++) 
	  {
		  nlattrs1[i] =  lattrs2[i];
		  nrattrs1[i] =  rattrs2[i];
	  }
	  for (i = 0; i < size3; i++) 
	  {
		  nlattrs1[i+size2] = lattrs3[i];
		  nrattrs1[i+size2] = rattrs3[i];
	  }
	  int nsize1=size2+size3;
	  
	  // for new join 2
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs2  = new int [ size1 ]; 
	  int * nlattrs2  = new int [ size1 ]; 
	  int nsize2 = 0;
	  
	  // for new join 3
	  // first allocate bigger size, then set the new right size after addkeys 		
	  int * nrattrs3  = new int [ size1 ]; 
	  int * nlattrs3  = new int [ size1 ]; 
	  int nsize3 = 0;
	  
	  // Get schema for A
	  EXPR * AB = before->GetInput(0);
	  LEAF_OP * AA = (LEAF_OP *) AB->GetInput(0)->GetOp() ;
	  GRP_ID  group_no = AA->GetGroup();
	  GROUP * group = Ssp->GetGroup(group_no);
	  LOG_PROP * log_prop = group->get_log_prop();	
	  SCHEMA *AAA = ((LOG_COLL_PROP *)log_prop) ->Schema;
	  
	  // Get schema for C
	  EXPR * CD = before->GetInput(1);
	  LEAF_OP * CC = (LEAF_OP *) CD->GetInput(0)->GetOp() ;
	  group_no = CC->GetGroup();
	  group = Ssp->GetGroup(group_no);
	  log_prop = group->get_log_prop();	
	  SCHEMA *CCC = ((LOG_COLL_PROP *)log_prop) ->Schema;
	  
	  //If a pair of attributes in (lattrs1,rattrs1), is in (A,C),
	  //then the pair should be in (nlattrs2,nrattrs2).
	  //If the pair is in (B,D), put it in in (nlattrs3,nrattrs3).
	  //If it is in (A, D), put it in (nlattrs1, nrattrs1).
	  //Otherwise go to (nlattrs1, nrattrs1), but reversed.
	  
	  for ( i=0; i < size1; i++) 
	  {
		  int lkey = lattrs1[i];
		  int rkey = rattrs1[i];
		  if (
			  AAA->InSchema(lkey) && //lattrs1 in A
              CCC->InSchema(rkey) 	//rattrs1 in C
			  )
		  {
			  nlattrs2[nsize2] = lkey;	//Add to nlattrs2
			  nrattrs2[nsize2] = rkey;	//Add to nrattrs2
			  nsize2++;
		  }
		  else if (
              !(AAA->InSchema(lkey)) && //lattrs1 in B
              !(CCC->InSchema(rkey))    //rattrs1 in D
			  )
		  {
			  nlattrs3[nsize3] =  lkey;	//Add to nlattrs3
			  nrattrs3[nsize3] =  rkey;	//Add to nrattrs3
			  nsize3++;
		  }
		  else if(
              (AAA->InSchema(lkey)) &&  //lattrs1 in A
              !(CCC->InSchema(rkey))    //rattrs1 in D
			  )
		  {
			  nlattrs1[nsize1] = lkey;	  //add to nlattrs1
			  nrattrs1[nsize1] = rkey;    //add to nrattrs1
			  nsize1++;
		  }
		  else                            //lattrs1 in B and rattrs1 in C
		  {
			  nlattrs1[nsize1] = rkey;	  //add to nlattrs1, reversed
			  nrattrs1[nsize1] = lkey;    //add to nrattrs1, reversed
			  nsize1++;
		  }
	  }
	  // Condition is that either one of the original joins had
	  // a cartesian product -- or that neither of the resulting
	  // joins has a cartesian product
	  if (size1 * size2 * size3 != 0  && nsize1 * nsize2 * nsize3 == 0)
		  return false;
	  return true;
#endif
	  return true;
} // EXCHANGE::condition

/*
  Rule  SELECT  -> FILTER
  ====  ======  == ======
*/
  
//##ModelId=3B0C086B0361
  SELECT_TO_FILTER::SELECT_TO_FILTER ()
    : RULE ("SELECT -> FILTER", 2,
	    new EXPR (new SELECT,
			new EXPR (new LEAF_OP (0)),				// table
			new EXPR (new LEAF_OP (1))				// predicate
			),
	    new EXPR (new FILTER,
			new EXPR (new LEAF_OP (0)),
			new EXPR (new LEAF_OP (1))
			)
		)
  {
	  // set rule index
	  set_index(R_SELECT_TO_FILTER);
  } // SELECT_TO_FILTER::SELECT_TO_FILTER
  
//##ModelId=3B0C086B036A
  EXPR * SELECT_TO_FILTER::next_substitute (EXPR * before, PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  // create transformed expression
	  result = new EXPR (new FILTER,
		  new EXPR (*(before->GetInput(0))),
		  new EXPR (*(before->GetInput(1)))
		  );
	  
	  return ( result );
  } // SELECT_TO_FILTER::next_substitute
  
/*
    Rule  PROJ -> P PROJ
    ====  ==== == =-====
*/
  
//##ModelId=3B0C086B02DE
  P_TO_PP::P_TO_PP ()
    : RULE ("PROJECT -> P_PROJECT", 1,
	    new EXPR (new PROJECT (0,0),
			new EXPR (new LEAF_OP (0))
			),
	    new EXPR (new P_PROJECT (0,0),
			new EXPR (new LEAF_OP (0))
			)
		)
  {
	  // set rule index
	  set_index(R_P_TO_PP);
  } // P_TO_PP::P_TO_PP
  
//##ModelId=3B0C086B02DF
  EXPR * P_TO_PP::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  PROJECT * Op = (PROJECT *) before->GetOp();
	  int size =  Op->size;
	  int * attrs = CopyArray ( Op->attrs, size);
	  
#ifdef _DEBUG
	  
	  // Get input's schema
	  LEAF_OP * A = (LEAF_OP *) ( before->GetInput(0)->GetOp() );
	  GRP_ID group_no = A->GetGroup();
	  GROUP* group = Ssp->GetGroup(group_no);
	  LOG_PROP * LogProp = group->get_log_prop();
	  SCHEMA *As_schema = ((LOG_COLL_PROP *)LogProp) ->Schema;
	  
	 /*
	  * Check that project list is in incoming schema
	  */
	  for(int i=0; i < size; i++)   assert (As_schema ->InSchema(attrs[i]));
	  
#endif
	  
	  // create transformed expression
	  result = new EXPR (new P_PROJECT (attrs, size),
		  new EXPR (*(before->GetInput(0)))
		  );
	  
	  return ( result );
  } // P_TO_PP::next_substitute
  
/*
    Rule  Sort enforcer
    ====  ============
*/
  
//##ModelId=3B0C086C0037
  SORT_RULE::SORT_RULE ()
    : RULE ("SORT enforcer", 1,
	    new EXPR (new LEAF_OP (0)),
	    new EXPR (new QSORT(),   //bogus should this be oby?
			new EXPR (new LEAF_OP (0))
			)
		)
  {
	  // set rule index
	  set_index(R_SORT_RULE);
  } // SORT_RULE::SORT_RULE
  
//##ModelId=3B0C086C004C
  EXPR * SORT_RULE::next_substitute (EXPR * before, PHYS_PROP * ReqdProp)
  {
	  // create transformed expression
	  EXPR * result = new EXPR (new QSORT(),
		  new EXPR(*before)
		  );
	  return ( result );
  }
  
//##ModelId=3B0C086C0041
  int SORT_RULE::promise (OP* op_arg, int ContextID)
  {
	  CONT * Cont = CONT::vc[ContextID];
	  PHYS_PROP * ReqdProp =  Cont -> GetPhysProp();	//What prop is required of
	  
	  int result = ( ReqdProp->GetOrder()==any ) ? 0 : SORT_PROMISE ;
	  
	  return ( result );
  } // SORT_RULE::promise 
  
/*
    Rule  RM_DUPLICATES  -> HASH_DUPLICATES
    ====  ======  == ======
*/
  
//##ModelId=3B0C086C00EB
  RM_TO_HASH_DUPLICATES::RM_TO_HASH_DUPLICATES ()
    : RULE ("RM_DUPLICATES  -> HASH_DUPLICATES", 1,
	    new EXPR (new RM_DUPLICATES(),
			new EXPR (new LEAF_OP (0))				// input table
			),
	    new EXPR (new HASH_DUPLICATES(),
			new EXPR (new LEAF_OP (0))
			)
		)
  {
	  // set rule index
	  set_index(R_RM_TO_HASH_DUPLICATES);
  } // RM_TO_HASH_DUPLICATES::RM_TO_HASH_DUPLICATES 
  
//##ModelId=3B0C086C00F5
  EXPR * RM_TO_HASH_DUPLICATES::next_substitute (EXPR * before, PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  // create transformed expression
	  result = new EXPR (new HASH_DUPLICATES(),
		  new EXPR (*(before->GetInput(0)))
		  );
	  
	  return ( result );
  } // RM_TO_HASH_DUPLICATES::next_substitute
  
  
/*
    Rule  AGG_LIST  -> HGROUP_LIST
    ====  ======  == ======
*/
  
//##ModelId=3B0C086C018C
  AL_TO_HGL::AL_TO_HGL(AGG_OP_ARRAY *list1, AGG_OP_ARRAY *list2)
	: RULE("AGG_LIST  -> HGROUP_LIST", 1,
			new EXPR (new AGG_LIST(0,0,list1),
				new EXPR (new LEAF_OP (0))				// input table
				),
			new EXPR (new HGROUP_LIST(0,0,list2),
				new EXPR (new LEAF_OP (0))
				)
			)
  {
	  // set rule index
	  set_index(R_AL_TO_HGL);
  }// AL_TO_HGL::AL_TO_HGL
  
//##ModelId=3B0C086C0196
  EXPR * AL_TO_HGL::next_substitute (EXPR * before, PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  AGG_LIST * Op = (AGG_LIST *) before->GetOp();
	  int size =  Op->GbySize;
	  int * attrs = CopyArray ( Op->GbyAtts, size);
	  
	  int i;
	  AGG_OP_ARRAY * agg_ops = new AGG_OP_ARRAY;
	  agg_ops->SetSize(Op->AggOps->GetSize());
	  for (i=0; i<Op->AggOps->GetSize(); i++)
	  {
		  (*agg_ops)[i] = new AGG_OP( *(* Op->AggOps)[i]);
	  }
	  
	  // create transformed expression
	  result = new EXPR (new HGROUP_LIST(attrs, size, agg_ops),
		  new EXPR (*(before->GetInput(0)))
		  );
	  
	  return ( result );
  } // AL_TO_HGL::next_substitute
  
/*
    Rule  FUNC_OP  -> P_FUNC_OP
    ====  ======  == ======
*/
  
//##ModelId=3B0C086C0235
  FO_TO_PFO::FO_TO_PFO()
    : RULE ("FUNC_OP  -> P_FUNC_OP", 1,
	    new EXPR (new FUNC_OP("",0,0),
			new EXPR (new LEAF_OP (0))				// input table
			),
	    new EXPR (new P_FUNC_OP("",0,0),
			new EXPR (new LEAF_OP (0))
			)
		)
  {
	  // set rule index
	  set_index(R_FO_TO_PFO);
  } // FO_TO_PFO::FO_TO_PFO
  
//##ModelId=3B0C086C0240
  EXPR * FO_TO_PFO::next_substitute (EXPR * before, PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  FUNC_OP * Op = (FUNC_OP *) before->GetOp();
	  int size =  Op->AttsSize;
	  int * attrs = CopyArray ( Op->Atts, size);
	  
	  // create transformed expression
	  result = new EXPR (new P_FUNC_OP(Op->RangeVar, attrs, size),
		  new EXPR (*(before->GetInput(0)))
		  );
	  
	  return ( result );
  } // FO_TO_PFO::next_substitute
  
/*
    Rule  AGGREGATE EQJOIN  -> JOIN AGGREGATE
    ====  ======  == ===== ====
*/
  
//##ModelId=3B0C086C0308
  AGG_THRU_EQJOIN::AGG_THRU_EQJOIN (AGG_OP_ARRAY *list1, AGG_OP_ARRAY *list2)
	:RULE ("AGGREGATE EQJOIN -> JOIN AGGREGATE", 2,
			new EXPR (new AGG_LIST(0,0,list1),
				new EXPR (new EQJOIN (0,0,0),
					new EXPR (new LEAF_OP (0)),
					new EXPR (new LEAF_OP (1))
					)
				),
			new EXPR (new EQJOIN(0,0,0),
				new EXPR (new LEAF_OP(0)),
				new EXPR (new AGG_LIST(0,0,list2),
					new EXPR (new LEAF_OP (1))
					)
				)
			)
  {
	  // set rule index
	  set_index(R_AGG_THRU_EQJOIN);
  }// AGG_THRU_EQ::AGG_THRU_EQ
  
//##ModelId=3B0C086C031B
  EXPR * AGG_THRU_EQJOIN::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  int i;
	  // get GbyAtts and AggOps
	  AGG_LIST * agg_op = (AGG_LIST *) before->GetOp();
	  int * gby_atts = CopyArray(agg_op->GbyAtts, agg_op->GbySize);
	  AGG_OP_ARRAY *	agg_ops = new AGG_OP_ARRAY;
	  agg_ops->SetSize(agg_op->AggOps->GetSize());
	  for (i=0; i<agg_op->AggOps->GetSize(); i++)
	  {
		  (*agg_ops)[i] = new AGG_OP( *(*agg_op->AggOps)[i]);
	  }
	  
	  //EQJOIN is the only input to AGG_LIST
	  EQJOIN * Op = (EQJOIN *) before->GetInput(0)->GetOp();
	  int size =  Op->size;
	  int * lattrs = CopyArray ( Op->lattrs, size);
	  int * rattrs = CopyArray ( Op->rattrs, size);
	  //get the schema of the right input
	  LEAF_OP * r_op = (LEAF_OP *) before->GetInput(0)->GetInput(1)->GetOp();
	  GRP_ID r_gid = r_op->GetGroup();
	  GROUP * r_group = Ssp->GetGroup(r_gid);
	  LOG_COLL_PROP * r_prop = (LOG_COLL_PROP *) r_group->get_log_prop();
	  SCHEMA * right_schema = r_prop->Schema;
	  
	  //new_gby = (gby^right_schema) u ratts 
	  int new_gby_size = 0;
	  int index = 0;
	  
	  for (i=0; i<agg_op->GbySize; i++)
	  {
		  if (right_schema->InSchema(gby_atts[i])) new_gby_size++;
	  }
	  new_gby_size += size;
	  int * new_gby_atts = new int [new_gby_size];
	  for (i=0; i<size; i++, index++)
	  {
		  new_gby_atts[index] = rattrs[i];
	  }
	  for (i=0; i<agg_op->GbySize; i++, index++)
	  {
		  if (right_schema->InSchema(gby_atts[i])) new_gby_atts[index] = gby_atts[i];
	  }
	  
	  delete [] gby_atts;
	  // create transformed expression
	  result = new EXPR (new EQJOIN (lattrs, rattrs, size),
		  new EXPR (*(before->GetInput(0)->GetInput(0))),
		  new EXPR ( new AGG_LIST (new_gby_atts, new_gby_size, agg_ops),
		  new EXPR (*(before->GetInput(0)->GetInput(1))) )
		  );
	  
	  return ( result );
  } // AGG_THRU_EQJOIN::next_substitute
  
/* Conditions:
	0. only joining on a single equality condition - fix later
	1. All attributes used in aggregates must be in the right schema
	2. The attributes referenced in the EQJOIN predicate are in the group by list
	3. The EQJOIN is a foreign key join: 
	Conditon 3 is checked by if lattrs contains candidate_key of left input
*/
//##ModelId=3B0C086C0325
  bool AGG_THRU_EQJOIN::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
  {
	  // get attributes used in aggregates, and groupby attributes
	  AGG_LIST * agg_op = (AGG_LIST *) before->GetOp();
	  int * agg_atts = agg_op->FlattenedAtts;
	  int * gby_atts = agg_op->GbyAtts;
	  
	  //EQJOIN is the only input to AGG_LIST
	  EQJOIN * Op = (EQJOIN *) before->GetInput(0)->GetOp();
	  int size =  Op->size;
	  int * lattrs = Op->lattrs;
	  int * rattrs = Op->rattrs;
	  //get the schema of the right input
	  LEAF_OP * r_op = (LEAF_OP *) before->GetInput(0)->GetInput(1)->GetOp();
	  GRP_ID r_gid = r_op->GetGroup();
	  GROUP * r_group = Ssp->GetGroup(r_gid);
	  LOG_COLL_PROP * r_prop = (LOG_COLL_PROP *) r_group->get_log_prop();
	  SCHEMA * right_schema = r_prop->Schema;
	  //get candidatekey of the left input
	  LEAF_OP * l_op = (LEAF_OP *) before->GetInput(0)->GetInput(0)->GetOp();
	  GRP_ID l_gid = l_op->GetGroup();
	  GROUP * l_group = Ssp->GetGroup(l_gid);
	  LOG_COLL_PROP * l_prop = (LOG_COLL_PROP *) l_group->get_log_prop();
	  KEYS_SET * l_cand_key = l_prop->CandidateKey;
	  
	  int i, j;
	  //check condition 1 agg_atts in right schema
	  for (i=0; i<agg_op->FAttsSize; i++)
	  {
		  if (! right_schema->InSchema(agg_atts[i])) return (false);
	  }
	  //check condition 2 ratts in gby or latts in gby
	  for (i=0; i<size; i++)
	  {
		  for (j=0; j<agg_op->GbySize; j++)
			  if (rattrs[i] == gby_atts[j]) break;
			  if (j == agg_op->GbySize) break;
	  }
	  if (i < size)
	  {
		  //continue to check lattrs
		  for (i=0; i<size; i++)
		  {
			  for (j=0; j<agg_op->GbySize; j++)
				  if (lattrs[i] == gby_atts[j]) break;
				  if (j == agg_op->GbySize) break;
		  }
	  }
	  if (i < size) return (false);
	  
	  //check condition 3, left_cand_key is contained in lattrs
	  if (l_cand_key->IsSubSet(lattrs, size)) return (true);
	  
	  return (false);
  } // AGG_THRU_EQJOIN::condition
  
/*
    Rule  EQJOIN  -> BIT INDEX JOIN
    ====  ======  == ===== ====
*/
  
//##ModelId=3B0C086D0010
  EQ_TO_BIT::EQ_TO_BIT ()
    : RULE ("EQJOIN -> BIT_JOIN", 2,
        new EXPR (new EQJOIN (0,0,0),
			new EXPR (new LEAF_OP (0)),
		    new EXPR (new SELECT,
				new EXPR (new GET (0)),					// table
				new EXPR (new LEAF_OP (1))				// predicate
			)
		),
	    new EXPR (new BIT_JOIN(0,0,0,0),
			new EXPR (new LEAF_OP (0)),
			new EXPR (new LEAF_OP (1))
		)
	)
  {
	  // set rule index
	  set_index(R_EQ_TO_BIT);
  } // EQ_TO_BIT::EQ_TO_BIT
  
//##ModelId=3B0C086D001A
  EXPR * EQ_TO_BIT::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  //EQJOIN is the only input to PROJECT
	  EQJOIN * EqOp = (EQJOIN *) before->GetOp();
	  int size = EqOp->size;
	  int * lattrs = CopyArray ( EqOp->lattrs, size);
	  int * rattrs = CopyArray ( EqOp->rattrs, size);
	  
	  //Get is the left input to SELECT
	  GET * GetOp = (GET *) before->GetInput(1)->GetInput(0)->GetOp();
	  
	  result = new EXPR (new BIT_JOIN (lattrs, rattrs, size, GetOp->GetCollection()),
		  new EXPR (*(before->GetInput(0))),
		  new EXPR (*(before->GetInput(1)->GetInput(1)))
		  );
	  
	  return ( result );
  } // AGG_THRU_EQJOIN::next_substitute
  
  /* Conditions:
	1. only joining on a single equality condition  
	2. index_attr candidate key of LEAF(0)
	3. EQJOIN is a foreign key join, with Lattr the foreign key (rattr is the candidate
	key of relation 'collection')
	4.   predicate.free_variables == bit_attr('collection')
	AND:
	predicate=bit_pred('collection')
   */
//##ModelId=3B0C086D0024
  bool EQ_TO_BIT::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
  {
	  EQJOIN * EqOp = (EQJOIN *) before->GetOp();
	  int size =  EqOp->size;
	  int * lattrs = EqOp->lattrs;
	  int * rattrs = EqOp->rattrs;
	  
	  //get left input to eqjoin
	  // Get schema for LEAF(0)
	  LEAF_OP * LEAF0 = (LEAF_OP *) (before->GetInput(0)->GetOp());
	  GRP_ID group_no = LEAF0->GetGroup();
	  GROUP * group = Ssp->GetGroup(group_no);
	  LOG_PROP * LogProp = group->get_log_prop();	
	  KEYS_SET * l_cand_key = ((LOG_COLL_PROP *)LogProp) ->CandidateKey;
	  
	  //Get is the left input to SELECT
	  GET * GetOp = (GET *) before->GetInput(1)->GetInput(0)->GetOp();
	  //get the candidate_keys and bit index
	  int CollId = GetOp->GetCollection();
	  // get candidate keys on 'collection'
	  KEYS_SET * candidate_key = Cat->GetCollProp(CollId)->CandidateKey;
	  // get the bitindex names of the 'collection
	  INT_ARRAY * BitIndices = Cat->GetBitIndNames(CollId);
	  
	  //Get is the predicate of SELECT
	  LEAF_OP * PredOp = (LEAF_OP *) before->GetInput(1)->GetInput(1)->GetOp();
	  GRP_ID Pred_GID = PredOp->GetGroup();
	  GROUP * leaf_group = Ssp->GetGroup(Pred_GID);
	  LOG_PROP * leaf_prop = leaf_group->get_log_prop();
	  KEYS_SET pred_freevar = ((LOG_ITEM_PROP *)leaf_prop)->FreeVars;
	  
	  //conditon 1: only joining on a single equality condition  
	  if (size == 1 && l_cand_key && BitIndices)
	  {
		  // check all the BitIndex
		  for (int CurrBitIndex =0; CurrBitIndex < BitIndices->GetSize(); CurrBitIndex++)
		  {
			  int index_attr = Cat->GetBitIndProp((*BitIndices)[CurrBitIndex])->IndexAttr;
			  KEYS_SET *bit_attrs = Cat->GetBitIndProp((*BitIndices)[CurrBitIndex])->BitAttr;
			  
			  // condition2: index_attr is a candidate key of LEAF(0)
			  if (l_cand_key->GetSize()!=1 || ((*l_cand_key)[0])!= index_attr ) 
				  continue;		
			  
			  //conditon 3:the EQJOIN is a foreign key join, with lattr the foreign key
			  if (!candidate_key->IsSubSet(rattrs, 1)) 
			  { 
				  return (false); 
			  }
			  
			  //conditon 4:   predicate.free_variables == bit_attr('collection')
			  //			   AND:
			  //				predicate=bit_pred('collection')
			  if (bit_attrs)
			  {
				  //assume all the value are covered
				  if ((pred_freevar) == (*bit_attrs) ) return (true);
			  }
		  }
	  }
	  
	  return (false);
  } 
  // EQ_TO_BIT::condition
  
  
  /*
  Rule  SELECT  -> INDEXED_FILTER
  ====  ======  == ===== ====
  */
  
//##ModelId=3B0C086D00F6
  SELECT_TO_INDEXED_FILTER::SELECT_TO_INDEXED_FILTER ()
	  : RULE ("SELECT -> INDEXED_FILTER", 1,
		    new EXPR (new SELECT,
			new EXPR (new GET (0)),				// table
			new EXPR (new LEAF_OP (0))			// predicate
			),
			new EXPR (new INDEXED_FILTER(0),
			new EXPR (new LEAF_OP (0))
			)
			)
  {
	  // set rule index
	  set_index(R_SELECT_TO_INDEXED_FILTER);
  } // SELECT_TO_INDEXED_FILTER::SELECT_TO_INDEXED_FILTER
  
//##ModelId=3B0C086D0100
  EXPR * SELECT_TO_INDEXED_FILTER::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  
	  // create transformed expression
	  result = new EXPR (new INDEXED_FILTER( ((GET *)before->GetInput(0)->GetOp())->GetCollection() ),
		  new EXPR (*(before->GetInput(1)))
		  );
	  return result;
  } // GET_TO_FILE_SCAN::next_substitute
  
  //      Need to check:
  //      Predicate only has one free variable
  //      Index exists for the free variable
//##ModelId=3B0C086D010A
  bool SELECT_TO_INDEXED_FILTER::condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
  {
	  //get the index list of the GET collection
	  int CollId = ((GET *)before->GetInput(0)->GetOp())->GetCollection();
	  INT_ARRAY * Indices = Cat->GetIndNames(CollId);
	  
	  //get the predicate free variables
	  LEAF_OP * Pred = (LEAF_OP *) (before->GetInput(1)->GetOp());
	  GRP_ID GrpNo = Pred->GetGroup();
	  GROUP * PredGrp = Ssp->GetGroup(GrpNo);
	  LOG_PROP * log_prop = PredGrp->get_log_prop();
	  KEYS_SET FreeVar = ((LOG_ITEM_PROP *)log_prop)->FreeVars;
	  
	  if (Indices && FreeVar.GetSize() == 1)
	  {
		  for (int i=0; i<Indices->GetSize(); i++)
		  {
			  if ((*Cat->GetIndProp((*Indices)[i])->Keys) == FreeVar) return (true);
		  }
	  }
	  
	  return (false);
  }
  
  /* ============================================================ */
  /*
  Rule  PROJ SEL -> SEL PROJ
  ====  ==== === == === ====
  */
//##ModelId=3B0C086D01B5
  PROJECT_THRU_SELECT::PROJECT_THRU_SELECT ()
    : RULE ("PROJECT_THRU_SELECT", 2,
		    new EXPR (new PROJECT(0,0),
				new EXPR (new SELECT,				// table
					new EXPR (new LEAF_OP (0)),
					new EXPR (new LEAF_OP (1))		// predicate
				)
			),
	    new EXPR (new PROJECT(0,0),
			new EXPR (new SELECT,
				new EXPR (new PROJECT(0,0),
					new EXPR (new LEAF_OP (0))),
				new EXPR (new LEAF_OP (1))
			)
		)
	)
  {
	  // set rule index
	  set_index(R_PROJECT_THRU_SELECT);
	  set_mask (	1 << R_PROJECT_THRU_SELECT	);
  } // PROJECT_THRU_SELECT::PROJECT_THRU_SELECT()
  
//##ModelId=3B0C086D01BE
  EXPR * PROJECT_THRU_SELECT::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
	  EXPR * result;
	  int * top_pattrs;
	  int top_size, pattr_size;
	  int * pattrs;
	  
	  // Get the projection attributes
	  top_pattrs = ((PROJECT *)before->GetOp())->attrs;
	  top_size = ((PROJECT *)before->GetOp())->size;
	  
	  // Get the select predicate free variables spfv
	  LEAF_OP * Pred = (LEAF_OP *) (before->GetInput(0)->GetInput(1)->GetOp());
	  GRP_ID GrpNo = Pred->GetGroup();
	  GROUP * PredGrp = Ssp->GetGroup(GrpNo);
	  LOG_PROP * log_prop = PredGrp->get_log_prop();
	  KEYS_SET FreeVar = ((LOG_ITEM_PROP *)log_prop)->FreeVars;
	  
	  // merge free variables of predicate into the project list
	  KEYS_SET * temp = new KEYS_SET(top_pattrs, top_size);
	  temp->Merge(FreeVar);
	  pattrs = temp->CopyOut();
	  pattr_size = temp->GetSize();
	  
	  // create transformed expression
	  result =
		  new EXPR (new PROJECT(CopyArray(top_pattrs, top_size), top_size),
		  new EXPR (new SELECT,
		  //new EXPR (before->GetInput(0)->GetOp(),	//select
		  new EXPR (new PROJECT(pattrs, pattr_size),
		  new EXPR((*(before->GetInput(0)->GetInput(0))))		//LEAF(0)
		  ),
		  new EXPR((*(before->GetInput(0)->GetInput(1))))			//LEAF(1)
		  )
		  );
	  return result;
  } // PROJECT_THRU_SELECT::next_substitute
  
/*
    Rule  DUMMY  -> PDUMMY
    ====  =====  == ======
*/
  
//##ModelId=3B0C086D026A
 DUMMY_TO_PDUMMY::DUMMY_TO_PDUMMY ()
    : RULE ("DUMMY->PDUMMY", 2,
	    new EXPR (new DUMMY (),
					  new EXPR (new LEAF_OP (0)),
					  new EXPR (new LEAF_OP (1))
				  ),
	     new EXPR (new PDUMMY (),
					  new EXPR (new LEAF_OP (0)),
					  new EXPR (new LEAF_OP (1))

				  )
		    )
  {
	  // set rule index
	  set_index(R_DUMMY_TO_PDUMMY);
	  
  } // DUMMY_TO_PDUMMY::DUMMY_TO_PDUMMY
  
//##ModelId=3B0C086D0273
  EXPR * DUMMY_TO_PDUMMY::next_substitute (EXPR * before,PHYS_PROP * ReqdProp)
  {
	  DUMMY * Op = (DUMMY *) before->GetOp();
	  
	  // create transformed expression
	  EXPR * result = new EXPR (new PDUMMY (),
		  new EXPR(*(before->GetInput(0))), 
		  new EXPR(*(before->GetInput(1)))
		  );
	  return  result ;
  } // DUMMY_TO_PDUMMY::next_substitute
