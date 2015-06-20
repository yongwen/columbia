/*  tasks.cpp : tasks implementation
$Revision: 30 $
 Implements classes in tasks.h
   
Columbia Optimizer Framework
 A Joint Research Project of Portland State University 
	and the Oregon Graduate Institute
 Directed by Leonard Shapiro and David Maier
 Supported by NSF Grants IRI-9610013 and IRI-9619977

  
*/

#include "stdafx.h"
#include "tasks.h"
#include "physop.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

/* Function to compare the promise of rule applications */
int compare_moves (void const *x, void const *y)
{
	// Cast arguments back to pointers to MOVEs 
	MOVE *a = (MOVE *) x;
	MOVE *b = (MOVE *) y;
    
	int result = 0;
	if (a -> promise > b -> promise)
		result = -1;
	else if (a -> promise < b -> promise)
		result = 1;
	else
		result = 0;
    
	return result;
} // compare_moves

/* Function to compare the cost of mexprs */
int compare_afters (void const *x, void const *y)
{
	// Cast arguments back to pointers to AFTER 
	AFTERS *a = (AFTERS *) x;
	AFTERS *b = (AFTERS *) y;
    
	int result = 0;
	if ((*a -> cost) < (*b -> cost))
		result = -1;
	else if ((*a -> cost) > (*b -> cost))
		result = 1;
	else
		result = 0;
    
	return result;
} // compare_afters

// **************** TASKS *********************
// base class of OPT tasks
//##ModelId=3B0C085D00C1
TASK::TASK (int ContextID, int parentTaskNo)
:ContextID(ContextID)
{
	ParentTaskNo = parentTaskNo;		// for debug
    
}; //TASK::TASK 

// **************** PTASKS *********************
//   List of un-done tasks
//##ModelId=3B0C085D0143
PTASKS::PTASKS()
:first(NULL)
{ };//PTASKS::PTASKS

//##ModelId=3B0C085D014D
PTASKS::~PTASKS()
{
	while (! empty ())	delete pop ();
}//PTASKS::~PTASKS

//##ModelId=3B0C085D0175
CString PTASKS::Dump()
{
	CString os;
    
	if (first == NULL)		// task list is empty
		os = "OPEN is empty\r\n";
	else			// task list has some tasks in it
	{
		os = "\t---- OPEN START ----\r\n";
		// loop over all tasks to be done
		int count=0;
		CString temp;
		for (TASK * task = first; task != NULL;  task = task -> next)
		{	
			temp.Format("\t%d -- %s\r\n", count++, task->Dump() );
			os += temp;
		}
		os += "\t---- OPEN END ----";
	} 
	return os;
} //PTASKS::Dump

//##ModelId=3B0C085D0157
bool PTASKS::empty ()
{
	return (first == NULL);
} // TASK_LIST::empty

//##ModelId=3B0C085D0161
void PTASKS::push (TASK * task)
{
	task -> next = first;
	first = task;   //Push Task
	if(COVETrace)
	{
		CString os;
		os.Format("PushTaskList {%s}\r\n", task ->Dump() );
		OutputCOVE.Write(os, os.GetLength());
	}
} //PTASKS::push

//##ModelId=3B0C085D016B
TASK * PTASKS::pop ()
{
	if (empty ()) return ( NULL );
    
	TASK * task = first;
	first = task -> next;
	if(COVETrace)   //Pop a Task
	{
		CString os;
		os.Format("PopTaskList\r\n");
		OutputCOVE.Write(os, os.GetLength());
	}
    
	return ( task );
} //PTASKS::pop

// ************  O_GROUP ******************

//    Task to optimize a group
//##ModelId=3B0C085D0215
O_GROUP::O_GROUP (GRP_ID grpID,	int ContextID,int parentTaskNo, bool last, COST * bound)
:TASK(ContextID,parentTaskNo),	GrpID(grpID), Last(last), EpsBound(bound)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_GROUP].New();
    
	// if INFBOUND flag is on, set the bound to be INF
#ifdef INFBOUND
	COST *INFCost = new COST(-1);
	CONT::vc[ContextID]->SetUpperBound(*INFCost);
#endif
    
} ;//O_GROUP::O_GROUP

    /*
    O_GROUP::perform
    {
    	see search_circle in declaration of class GROUP, for notation
    
        Call search_circle
        If cases (1) or (2), 
            terminate this task. //circle is prepared
        Cases (3) and (4) remain.  More search.

		IF (Group is not optimized) 
    		assert (this is case 3, never searched for this property before)
    		if (property is ANY)
    			Push O_EXPR on first logical expression
				add a winner for this context to the circle, with null plan
				  (i.e., initialize the winner's circle for this property.)
    		else
    			Push O_GROUP on this group with current context
    			Push O_GROUP on this group, with new context: ANY property and 
    				cost = current context cost - appropriate enforcer cost, last task.
					This is valid since the result of the ANY search will only be used with
					the enforcer.  If the cost is negative, then the enforcer cannot be used,
					so the ANY winner should be null.
					Since this is somewhat complex and prone to error, we will omit it for now
					and use the original context's cost
		else (Group is optimized)
			if (property is ANY)
    			assert (this is case 4)
    			push O_INPUTS on all physical mexprs
    		else (property is not ANY)
    			Push O_INPUTS on all physical mexprs with current context, last one is last task
    			If case (3) [i.e. appropriate enforcer is not in group], Push APPLY_RULE on 
    			 enforcer rule, not the last task
				add a winner for this context to the circle, with null plan.
				  (i.e., initialize the winner's circle for this property.)

    */
//##ModelId=3B0C085D0235
void O_GROUP::perform ()
{
	SET_TRACE Trace(true);
	PTRACE ("O_GROUP %d performing", GrpID);
	
#ifndef IRPROP
	PTRACE2 ("Context ID: %d , %s", ContextID, CONT::vc[ContextID]->Dump() );
#endif
	PTRACE ("Last flag is %d", Last);
    
	GROUP * Group = Ssp->GetGroup(GrpID);
	M_EXPR * FirstLogMExpr = Group->GetFirstLogMExpr();
	
	if (FirstLogMExpr->GetOp()->is_const())
	{
		PTRACE("Group %d is const group", GrpID);
#ifndef IRPROP
		M_EXPR * WPlan = new M_EXPR(*FirstLogMExpr);
		Group->NewWinner(new PHYS_PROP(any), WPlan, new COST(0), true);
#endif
		return;
	}
    
	bool moreSearch, SCReturn;
    
#ifdef IRPROP
    
	PHYS_PROP * LocalReqdProp =  M_WINNER::mc[GrpID] -> GetPhysProp(ContextID);	//What prop is required of
	SCReturn = Group -> search_circle(GrpID, LocalReqdProp, moreSearch);
	if (!moreSearch)
	{	
		// group is completely optimized
		PTRACE("%s", "Winner's circle is prepared so terminate this task");
		delete this;
		return;
	}
	else	// group is not optimized at all
	{
		// Get the first logical mexpr in the group
		M_EXPR * LogMExpr = Group->GetFirstLogMExpr();
		
		// push the enforcer rule before pushing the first logical MEXPR
		PTRACE("%s", "Push APPLY_RULE on enforcer rule");
		RULE * Rule = (*RuleSet)[R_SORT_RULE];
		PTasks.push(new APPLY_RULE( Rule, FirstLogMExpr, false, 0, TaskNo, false));
		
		// push the O_EXPR on first logical expression
		PTasks.push( new O_EXPR( FirstLogMExpr, false, 0, TaskNo, true) );
	}
	
#else
    
	CONT * LocalCont = CONT::vc[ContextID];
	PHYS_PROP * LocalReqdProp =  LocalCont -> GetPhysProp();	//What prop is required 
	COST * LocalCost = LocalCont ->GetUpperBd();
    
	SCReturn = Group -> search_circle(LocalCont, moreSearch);
    
	//If case (2) or (1), terminate this task
	if ( !moreSearch)
	{
		PTRACE("%s", "Winner's circle is prepared so terminate this task");
		delete this;
		return;		
	}
	
	
	PTRACE("Group is %s optimized", Group->is_optimized()?"":"not");
	if(!Group -> is_optimized())
	{
		assert(moreSearch && !SCReturn); //assert (this is case 3)
		//if (property is ANY)
		if(LocalReqdProp->GetOrder() == any)
		{
			PTRACE("%s", "add winner with null plan, push O_EXPR on 1st logical expression");
			Group -> NewWinner(LocalReqdProp, NULL, new COST(*LocalCost), false);
			if (GlobepsPruning)
			{
				COST * eps_bound = new COST(*EpsBound);
				PTasks.push( new O_EXPR( FirstLogMExpr, false, ContextID, TaskNo, true, eps_bound) );
			}
			else
				PTasks.push( new O_EXPR( FirstLogMExpr, false, ContextID, TaskNo, true) );
		}
		else
		{
			PTRACE("%s","Push O_GROUP with current context, another with ANY context");
			assert(LocalReqdProp -> GetOrder() == sorted);	//temporary
			if (GlobepsPruning)
			{
				COST * eps_bound = new COST(*EpsBound);
				PTasks.push(new O_GROUP (GrpID, ContextID, TaskNo, true, eps_bound));
			}
			else
				PTasks.push(new O_GROUP (GrpID, ContextID, TaskNo, true));	
			COST *NewCost = new COST(*(LocalCont -> GetUpperBd()));
			CONT * NewContext = new CONT(new PHYS_PROP(any), NewCost, false);
			CONT::vc.Add(NewContext);
			if (GlobepsPruning)
			{
				COST * eps_bound = new COST(*EpsBound);
				PTasks.push(new O_GROUP (GrpID, CONT::vc.GetSize()-1, TaskNo, true, eps_bound));
			}
			else
				PTasks.push(new O_GROUP (GrpID, CONT::vc.GetSize()-1, TaskNo, true));
		}
	}
	else //Group is optimized
	{
		// if (property is ANY)
		// assert (this is case 4)
		// push O_INPUTS on all physical mexprs
		CArray <M_EXPR *, M_EXPR *> PhysMExprs;
		int count = 0;
		if(LocalReqdProp->GetOrder() == any)
		{
			PTRACE("%s","push O_INPUTS on all physical mexprs");
			assert(moreSearch && SCReturn); 
			for( M_EXPR * PhysMExpr = Group->GetFirstPhysMExpr();
			PhysMExpr ;  PhysMExpr = PhysMExpr->GetNextMExpr() ) 
			{
				PhysMExprs.Add(PhysMExpr);
				count++;
			}
			//push the last PhysMExpr
			if (--count >= 0)
			{
				PTRACE("pushing O_INPUTS %s", PhysMExprs[count]->Dump());
				if (GlobepsPruning)
				{
					COST * eps_bound = new COST(*EpsBound);
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, true,
						eps_bound) );
				}
				else
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, true) );
			}
			//push other PhysMExpr
			while (--count >= 0)
			{
				PTRACE("pushing O_INPUTS %s", PhysMExprs[count]->Dump());
				if (GlobepsPruning)
				{
					COST * eps_bound = new COST(*EpsBound);
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, false,
						eps_bound) );
				}
				else
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, false) );
			}
		}
		else //property is not ANY)
		{
			assert(LocalReqdProp -> GetOrder() == sorted);	//temporary
			//Push O_INPUTS on all physical mexprs with current context, last one is last task
			PTRACE("%s","Push O_INPUTS on all physical mexprs");
			for( M_EXPR * PhysMExpr = Group->GetFirstPhysMExpr();
			PhysMExpr ;  PhysMExpr = PhysMExpr->GetNextMExpr() ) 
			{
				PhysMExprs.Add(PhysMExpr);
				count++;
			}
			//push the last PhysMExpr
			if (--count>=0)
			{
				PTRACE("pushing O_INPUTS %s", PhysMExprs[count]->Dump());
				if (GlobepsPruning)
				{
					COST * eps_bound = new COST(*EpsBound);
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, true,
						eps_bound) );
				}
				else
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, true) );
			}
			//push other PhysMExpr
			while (--count >= 0)
			{
				PTRACE("pushing O_INPUTS %s", PhysMExprs[count]->Dump());
				if (GlobepsPruning)
				{
					COST * eps_bound = new COST(*EpsBound);
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, false,
						eps_bound) );
				}
				else
					PTasks.push( new O_INPUTS( PhysMExprs[count], ContextID, TaskNo, false) );
			}
			
			//If case (3) [i.e. appropriate enforcer is not in group], Push APPLY_RULE on 
			//	enforcer rule, not the last task
			if (!SCReturn)
			{
				PTRACE("%s", "Push APPLY_RULE on enforcer rule");
				if(LocalReqdProp -> GetOrder() == sorted)
				{
					RULE * Rule = (*RuleSet)[R_SORT_RULE];
					if (GlobepsPruning)
					{
						COST * eps_bound = new COST(*EpsBound);
						PTasks.push(new APPLY_RULE( Rule, FirstLogMExpr, 
							false, ContextID, TaskNo)); 
					}
					else
						PTasks.push(new APPLY_RULE( Rule, FirstLogMExpr, 
						false, ContextID, TaskNo, false));
				}
				else
				{
					assert(false);
				}
			}
			// add a winner to the circle, with null plan.
			//(i.e., initialize the winner's circle for this property.)
			PTRACE("%s", "Init winner's circle for this property");
			if(moreSearch && !SCReturn)
				Group -> NewWinner(LocalReqdProp, NULL, new COST(*LocalCost), false);
		}
		}
#endif
		
		delete this;
		
    } //O_GROUP::perform
    
//##ModelId=3B0C085D023E
    CString O_GROUP::Dump()
    {
		CString os;
		CString temp;
		
		os.Format("OPT_GROUP group %d,", GrpID);
		temp.Format(" parent task %d,", ParentTaskNo);
		os += temp;
		temp.Format(" %s", CONT::vc[ContextID]->Dump());
		os += temp;
		return os;
    } //O_GROUP::Dump
    
    // ************  E_GROUP ******************
    //    Task to explore a group
//##ModelId=3B0C085D02E8
    E_GROUP::E_GROUP (GRP_ID grpID,	int ContextID,int parentTaskNo, bool last, COST * bound)
		:TASK(ContextID,parentTaskNo),GrpID(grpID), Last(last), EpsBound(bound)
    {
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_E_GROUP].New();
    } ;//E_GROUP::E_GROUP
    
//##ModelId=3B0C085D0307
    void E_GROUP::perform ()
    {
		SET_TRACE Trace(true);
		
		PTRACE("E_GROUP %d performing", GrpID);
		PTRACE2 ("Context ID: %d , %s", ContextID, CONT::vc[ContextID]->Dump() );
		
		GROUP * Group = Ssp->GetGroup(GrpID);
		
		if (Group -> is_optimized())   //See discussion in E_GROUP class declaration
		{
			delete this;
			return;
		}
		else if (Group -> is_explored())   
		{
			delete this;
			return;
		}
		
		if(Group->is_exploring() ) assert(false);
		else
		{
			// the group will be explored, let other tasks don't do it again
			Group->set_exploring(true);
			
			// mark the group not explored since we will begin exploration
			Group->set_explored(false);
			
			M_EXPR * LogMExpr = Group->GetFirstLogMExpr();
			
			// only need to E_EXPR the first log expr, 
			// because it will generate all logical exprs by applying appropriate rules
			// it won't generate dups because rule bit vector 
			PTRACE("pushing O_EXPR exploring %s", LogMExpr->Dump()); 
			// this logical mexpr will be the last optimized one, mark it as the last task for this group
			if (GlobepsPruning)
			{
				COST * eps_bound = new COST(*EpsBound);
				PTasks.push( new O_EXPR( LogMExpr, true, ContextID, TaskNo, true, eps_bound) );
			}
			else
				PTasks.push( new O_EXPR( LogMExpr, true, ContextID, TaskNo, true) );
		}
		
		delete this;
    }//E_GROUP::perform
    
//##ModelId=3B0C085D0310
    CString E_GROUP::Dump()
    {
		CString os;
		os.Format("E_GROUP group %d,", GrpID);
		CString temp;
		temp.Format(" parent task %d", ParentTaskNo);
		os += temp;
		return os;
    }//E_GROUP::Dump
    
    // ************  O_EXPR ******************
    
    
//##ModelId=3B0C085E0018
    O_EXPR::O_EXPR (
		M_EXPR * mexpr,
		bool explore,
		int ContextID,
		int parent_task_no, bool last,
		COST * bound)
        : TASK(ContextID,parent_task_no), 
		MExpr(mexpr), explore(explore),
		Last(last), EpsBound(bound)
    {
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_EXPR].New();
    } ;//O_EXPR::O_EXPR
    
//##ModelId=3B0C085E0041
	void O_EXPR::perform()
    {
		PTRACE2 ("O_EXPR performing, %s mexpr: %s ", explore ? "exploring" : "optimizing", MExpr->Dump() ); 
#ifdef IRPROP
		int GrpNo = MExpr->GetGrpID();
		PTRACE2 ("ContextID: %d, %s", ContextID, (M_WINNER::mc[GrpNo]->GetPhysProp(ContextID))->Dump());
#else
		PTRACE2 ("Context ID: %d , %s", ContextID, CONT::vc[ContextID]->Dump() );
#endif
		PTRACE ("Last flag is %d", Last);
		
		if(explore)
			assert(MExpr->GetOp()->is_logical());	//explore is only for logical expression
		
		if(MExpr->GetOp()->is_item())
		{
			PTRACE("%s", "expression is an item_op");
			//push the O_INPUT for this item_expr
			PTRACE("pushing O_INPUTS %s", MExpr->Dump());
			if (GlobepsPruning)
			{
				COST * eps_bound = new COST(*EpsBound);
				PTasks.push( new O_INPUTS(MExpr, ContextID, TaskNo, true, eps_bound));
			}
			else
				PTasks.push( new O_INPUTS(MExpr, ContextID, TaskNo, true));
			delete this;
			return;
		}
		
		// identify valid and promising rules
		MOVE *Move = new MOVE[RuleSet->RuleCount]; // to collect valid, promising moves
		int moves = 0;	// # of moves already collected
		for (int RuleNo = 0;  RuleNo<RuleSet->RuleCount;  RuleNo ++ )
		{
			RULE * Rule = (*RuleSet)[RuleNo];
			
			if( Rule == NULL) continue;		// some rules may be turned off
			
#ifdef UNIQ
            if( !( MExpr -> can_fire(Rule -> get_index())) )
			{
                PTRACE("Rejected rule %d ",Rule -> get_index());
                continue;  // rule has already fired
            }
#endif
			if(explore && Rule->GetSubstitute()->GetOp()->is_physical() ) 
			{
                PTRACE("Rejected rule %d ",Rule -> get_index());
                continue;  // only fire transformation rule when exploring
            }
			
			int Promise = Rule -> promise(MExpr->GetOp(), ContextID);
			// insert a valid and promising move into the array
			if( Rule->top_match(MExpr->GetOp())  && Promise > 0)
			{
				Move [moves].promise = Promise;
				Move [moves ++ ].rule = Rule;
#ifdef _DEBUG
				TopMatch[RuleNo]++;
#endif
			} 
		} 
		
		PTRACE ("%d promising moves", moves);
		
		// order the valid and promising moves by their promise
		qsort ((char *) Move, moves, sizeof (MOVE), compare_moves);
			// optimize the rest rules in order of promise
		while ( -- moves >= 0)
		{
			bool Flag=false;
			if(Last)
				// this's the last task in the group,pass it to the new task
			{	Last = false;	// turn off this, since it's no longer the last task
			Flag = true;
			}
			
			// push future tasks in reverse order (due to LIFO stack)
			RULE * Rule = Move[moves].rule;
			PTRACE ("pushing rule `%s'", Rule->GetName() );
			
			// apply the rule
			if (GlobepsPruning)
			{
				COST * eps_bound = new COST(*EpsBound);
				PTasks.push(new APPLY_RULE (Rule, MExpr, explore, ContextID, TaskNo, Flag,
					eps_bound) );
			}
			else
				PTasks.push(new APPLY_RULE (Rule, MExpr, explore, ContextID, TaskNo, Flag) );
			
			// for enforcer and expansion rules, don't explore patterns
			EXPR *original = Rule->GetOriginal();
			if( original->GetOp()->is_leaf() ) continue;
			
			// earlier tasks: explore all inputs to match the original pattern	
			for (int input_no = original ->GetArity();  -- input_no >= 0;  )
			{
				// only explore the input with arity > 0
				if ( original->GetInput(input_no)->GetArity() )	 
				{
					// If not yet explored, schedule a task with new context
					GRP_ID grp_no = (MExpr->GetInput(input_no));
					if( !Ssp->GetGroup(grp_no)->is_exploring() )
					{
						//E_GROUP can not be the last task for the group
						if (GlobepsPruning)
						{
							COST * eps_bound = new COST(*EpsBound);
							PTasks.push ( new E_GROUP( grp_no, ContextID, TaskNo, false, eps_bound) );
						}
						else
							PTasks.push ( new E_GROUP( grp_no, ContextID, TaskNo, false) );
					}
				} 
			} // earlier tasks: explore all inputs to match the original pattern
			
		} // optimize in order of promise
		
		delete [] Move;
		delete this;
    } //O_EXPR::perform
    
//##ModelId=3B0C085E0040
    CString O_EXPR::Dump()
    {
		CString os;
		os.Format("O_EXPR group %s,", MExpr->Dump());
		CString temp;
		temp.Format(" parent task %d", ParentTaskNo);
		os += temp;
		return os;
    } //O_EXPR::Dump
    
    
    /*********** O_INPUTS FUNCTIONS ***************/
    
//##ModelId=3B0C085E02B7
    O_INPUTS::O_INPUTS (M_EXPR * MExpr, int ContextID, int ParentTaskNo, bool last, COST *bound, int ContNo)
		:MExpr(MExpr), TASK(ContextID,ParentTaskNo),
		InputNo(-1),Last(last), PrevInputNo(-1), EpsBound(bound), ContNo(ContNo)
    {
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_INPUTS].New();
		
		assert( MExpr -> GetOp() -> is_physical()|| MExpr->GetOp()->is_item());
		//We can only calculate cost for physical operators
		
		//Cache local properties
		OP * Op =  (PHYS_OP *)(MExpr -> GetOp());	// the op of the expr
		arity = Op -> GetArity();			// cache arity of mexpr
		
        // create the arrays of input costs and logical properties
		if(arity)
		{
			InputCost = new COST* [arity];
			InputLogProp = new LOG_PROP* [arity];
		}
		
    };
    
    
//##ModelId=3B0C085E02E9
    O_INPUTS::~O_INPUTS ()
    {
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_INPUTS].Delete();
		
		// localcost was new by find_local_cost, so need to delete it
		delete LocalCost;	
		if (EpsBound) delete EpsBound;
		
		if(arity)
		{
			delete [] InputCost;
			delete [] InputLogProp;
		}
		
    } // O_INPUTS::~O_INPUTS
    
	  /*
	  O_INPUTS::perform
	  
    NOTATION
    InputCost[]: Contains actual (or lower bound) costs of optimal inputs to G.
    CostSoFar: LocalCost + sum of all InputCost entries. 
    G: the group being optimized.  
    IG: various inputs to expressions in G.
    SLB: (Special Lower Bound) The Lower Bound of G, derived with fetch and cucard 
We use this term instead of Lower Bound since we will use other lower bounds.
    There are still three flags: Pruning (also called Group Pruning), CuCardPruning and GlobepsPruning, 
    with new meanings.  We plan to run benchmarks in four cases:
    
    1. Starburst - generate all expressions [!Pruning && !CuCardPruning]
    2. Group Pruning - aggressively check limits at all times [Pruning && !CuCardPruning]
    	aggressively check means if ( CostSoFar >= upper bound of context in G) then terminate.
    3. Lower Bound Pruning - if there is no winner, then use IG's SLB in InputCost[]. [CuCardPruning].
    	This case assumes that the Pruning flag is on, i.e. the code forces Pruning to be true
    	if CuCardPruning is true.  The SLB may involve cucard, fetch, copy, etc in the lower bound.
    4. Global Epsilon Pruning [GlobepsPruning].  If a plan costs <= GLOBAL_EPS, it is a winner for G.
    
    PSEUDOCODE
    
    On the first (and no other) execution, the code must initialize some O_INPUTS members.
    The idea here is to get a quick lower bound for the cost of the inputs.
    The only nontrivial member is InputCost; here is how to initialize it:
    //Initial values of InputCost are zero in the Starburst case
    For each input group IG
    	If (Starburst case)
    		InputCost is zero
    		continue
    	Determine property required of search in IG
    	If no such property, terminate this task.
    	call search_circle on IG with that property, infinite cost.
    
    	If case (1), no possibility of satisfying the context
    		terminate this task
    	If search_circle returns a non-null Winner from IG, case (2)
    		InputCost[IG] = cost of that winner
    	else if (!CuCardPruning) //Group Pruning case (since Starburst not relevant here)
    		InputCost[IG] = 0
    	//remainder is Lower Bound Pruning case
    	else if there has been no previous search for ReqdProp
    		InputCost[IG] = SLB 
    	else if there has been a previous search for ReqdProp
    		InputCost[IG] = max(cost of winner, IG's SLB) //This is a lower bound for IG
    	else
    		error - previous cost failed because of property
    
    //The rest of the code should be executed on every execution of the task.
    
    If (Pruning && CostSoFar >= upper bound) terminate.
    
    if (arity==0 and required property can not be satisfied)
    	terminate this task
    
    //Calculate cost of remaining inputs
    For each remaining (from InputNo to arity) input group IG
        Call search_circle()
    	If Starburst case and case (1)
    		error
    	else if case (1)
    		terminate this task
    	else If there is a non-null Winner in IG, case (2)
    		store its cost in InputCost
    		if (Pruning && CostSoFar exceeds G's context's upper bound) terminate task
        else if (we did not just return from O_GROUP on IG)
    		//optimize this input; seek a winner for it
    		push this task
    		push O_GROUP for IG, using current context's cost minus CostSoFar plus InputCost[InputNo]
    		terminate this task
        else // we just returned from O_GROUP on IG
    		Trace: This is an impossible plan
    		terminate this task
    	InputNo++;
    endFor //calculate the cost of remaining inputs 
    
    //Now all inputs have been optimized
    
    if (CostSoFar >  G's context's upper bound)
    	terminate this task
    
    //Now we know current expression satisfies current context.
    
    if(GlobepsPruning && CostSoFar <= GLOBAL_EPS)
    	Make current mexpression a done winner for G
    	mark the current context as done 
    	terminate this task
    
    //Now we consider the possible states of the relevant winner in G
    
    Search the winner's circle in G for the current task's physical property
    If there is no such winner
    	error - the search should have initialized a winner
    else If winner is done 
    	error - we are in the midst of a search, not yet done
    else If (winner is non-null and CostSoFar >= cost of this winner) || winner is null )
    	Replace existing winner with current mexpression and its cost, don't change done flag
    	Update the upper bound of the current context
     */
    

//##ModelId=3B0C085E0307
void O_INPUTS::perform ()
{
	PTRACE2 ("O_INPUT performing Input %d, expr: %s", InputNo,  MExpr->Dump() );
#ifdef IRPROP
	int GrpNo = MExpr->GetGrpID();
	PTRACE2 ("ContextID: %d, %s", ContextID, (M_WINNER::mc[GrpNo]->GetPhysProp(ContextID))->Dump());
#else
	PTRACE2 ("Context ID: %d , %s", ContextID, CONT::vc[ContextID]->Dump() );
#endif
	PTRACE ("Last flag is %d", Last);
	
	//Cache local properties of G and the expression being optimized
	
	OP * Op = MExpr ->GetOp();  //the op of the expr
	assert(Op -> is_physical() );
	GROUP *	LocalGroup = Ssp -> GetGroup(MExpr -> GetGrpID());	//Group of the MExpr
    
#ifdef IRPROP
	PHYS_PROP * LocalReqdProp = M_WINNER::mc[GrpNo]->GetPhysProp(ContextID);
	COST * LocalUB = M_WINNER::mc[GrpNo]->GetUpperBd(LocalReqdProp);
	PTRACE ("Bound (LocalUB) is %s", LocalUB->Dump());
#else
	PHYS_PROP * LocalReqdProp =  CONT::vc[ContextID] -> GetPhysProp();	//What prop is required
	COST * LocalUB = CONT::vc[ContextID] -> GetUpperBd();
#endif
	
	//if global eps pruning happened, terminate this task
	if (GlobepsPruning && CONT::vc[ContextID] -> is_done())
	{
		PTRACE("%s", "Task terminated due to global eps pruning");
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_INPUTS].Delete();
		if(Last)
			// this's the last task for the group, so mark the group with completed optimizing
			LocalGroup->set_optimized(true);
		return;
	}
    
	//Declare locals
	GRP_ID IGNo;	//Input Group Number
	GROUP * IG;
	int input;		//index over input groups
	bool possible;	// is it possible to satisfy the required property?
    //	COST * CostSoFar = new COST(0);
	COST CostSoFar(0);
    
#ifndef IRPROP
	WINNER * LocalWinner = LocalGroup -> GetWinner(LocalReqdProp);	//Winner in G
#endif
	COST Zero(0);
	
	//On the first (and no other) execution, code must initialize some O_INPUTS members.
	//The only nontrivial member is InputCost.
	if( InputNo == -1 )			
	{
		// init inputLogProp
		for(input= 0; input< arity; input++)
		{
			GROUP * InputGroup = Ssp -> GetGroup(MExpr -> GetInput(input));  //Group of current input
			InputLogProp[input] = InputGroup -> get_log_prop() ;
		}
		
		// get the localcost of the mexpr being optimized in G
		LocalCost = Op->FindLocalCost ( LocalGroup->get_log_prop(), InputLogProp); 
		
		//For each input group IG
		for(input= 0; input< arity; input++)
		{
			// Initial values of InputCost are zero in the Starburst (no Pruning) case
			if(!Pruning)
			{
				if(!input)
					PTRACE("%s","Not pruning so all InputCost elements are set to zero");
				assert(!CuCardPruning);
				InputCost[input] = &Zero;
				continue;
			}
			
			IGNo = MExpr -> GetInput(input);
			IG = Ssp -> GetGroup(IGNo);  //Group of current input
			
			// special case: the input is a const group - in item class
			if(IG->GetFirstLogMExpr()->GetOp()->is_const())
			{
				PTRACE("Input %d is a const operator so its cost is zero", input);
				InputCost[input] = ((CONST_OP *)IG->GetFirstLogMExpr()->GetOp())->get_cost();
				continue;
			}
			
			PHYS_PROP * ReqProp;
			if (Op->is_physical())
			{
				// Determine property required of that input
				ReqProp = ((PHYS_OP *)Op)->InputReqdProp
					(LocalReqdProp, InputLogProp[input], input, possible);
				
				if( ! possible )  // if not possible, means no such input prop can satisfied
				{
					PTRACE ("Impossible search: Bad input %d", input);
					delete ReqProp;
					
					goto TerminateThisTask;
				}
			}
			else ReqProp = new PHYS_PROP(any);
			
			//call search_circle on IG with that property, infinite cost.
			bool moreSearch, SCReturn;
			COST * INFCost = new COST(-1);
			
#ifdef IRPROP
			// the ReqProp will already be set up in multiwinner
			SCReturn = IG -> search_circle(IGNo, ReqProp, moreSearch);
			if (!moreSearch && !SCReturn) // input group is optimized
			{
				// if winner's cost >= INFCost then "impossible search, bad input"
				PTRACE ("Impossible search: Bad input %d", input);
				delete INFCost;
				delete ReqProp;
				goto TerminateThisTask;
			}
			else if (!moreSearch && SCReturn)
			{
				COST *WinCost = M_WINNER::mc[IGNo]->GetUpperBd(ReqProp);
				InputCost[input] = WinCost;
			}
			else if (!CuCardPruning)  // Group Pruning case
				InputCost[input] = &Zero;
			else // group is not optimized or CuCard Pruning case
				InputCost[input] = IG -> GetLowerBd();
			
			delete ReqProp;
			delete INFCost;
#else
			
			CONT * IGContext = new CONT(ReqProp, INFCost, false);
			SCReturn = IG -> search_circle(IGContext,  moreSearch);
			PTRACE2("search_circle(): more search %s needed, return value is %s",
				moreSearch?"":"not", SCReturn?"true":"false");
			
			//If case (1), impossible, then terminate this task
			if (!moreSearch && !SCReturn)
			{
				PTRACE ("Impossible search: Bad input %d", input);
				delete IGContext;
				goto TerminateThisTask;
			}
			//If search_circle returns a non-null Winner from InputGroup, case (2)
			//InputCost[InputGroup] = cost of that winner
			else if(!moreSearch && SCReturn ) 
			{
				InputCost[input] = IG -> GetWinner(ReqProp)->GetCost();
				assert(IG->GetWinner(ReqProp)->GetDone());
			}
			//else if (!CuCardPruning) //Group Pruning case (since Starburst not relevant here)
			//InputCost[IG] = 0
			else if (!CuCardPruning)  
				InputCost[input] = &Zero;
			//remainder applies only in CuCardPruning case
			else 
				InputCost[input] = IG -> GetLowerBd();
			
			delete IGContext;
#endif
		}  // initialize some O_INPUTS members
		
		InputNo ++;		// Ensure that previous code will not be executed again; begin with Input 0
		}
		
		//If Global Pruning and cost so far is greater than upper bound for this context, then terminate
		CostSoFar.FinalCost(LocalCost, InputCost, arity);
		if(Pruning && CostSoFar >= *LocalUB) 
		{
			PTRACE2 ("Expr LowerBd %s, exceed Cond UpperBd %s,Pruning applied!",
				CostSoFar.Dump(), LocalUB -> Dump() );
			
			goto TerminateThisTask;
		}
		
		//Calculate the cost of remaining inputs
		for(input = InputNo ; input < arity ; input++)
		{
			//set up local variables
			IGNo = MExpr -> GetInput(input);
			IG = Ssp -> GetGroup(IGNo);  //Group of current input
			
			// special case: the input is an const_op, continue
			if(IG->GetFirstLogMExpr()->GetOp()->is_const())
			{	// the cost of item group will not be refined, always equal to ConstGroupCost (0)
				PTRACE("Input : %d is a const group", input);
				continue;
			}
			
			//generate appropriate property for search of IG
			PHYS_PROP * ReqProp;
			if (Op->is_physical())
			{
				// Determine property required of that input
				ReqProp = ((PHYS_OP *)Op)->InputReqdProp
					(LocalReqdProp, InputLogProp[input], input, possible);
				
				if(Pruning) assert(possible);	// should be possible since in the first pass, we checked it
				if(!possible) 
				{
					delete ReqProp;
					goto TerminateThisTask;
				}
			}
			else ReqProp = new PHYS_PROP(any);
			
			bool moreSearch, SCReturn;
			COST * INFCost = new COST(-1);
			
#ifdef IRPROP
			
			SCReturn = IG->search_circle(IGNo, ReqProp, moreSearch);
			if (!moreSearch && !SCReturn) // input group is optimized
			{
				PTRACE ("Impossible search: Bad input %d", input);
				delete INFCost;
				delete ReqProp;
				goto TerminateThisTask;
			}
			else if (!moreSearch && SCReturn) // there is a winner with nonzero plan
			{
				PTRACE("Found Winner for Input : %d", input);
				COST *WinCost =  M_WINNER::mc[IGNo]->GetUpperBd(ReqProp);
				InputCost[input] =	WinCost;
				CostSoFar.FinalCost(LocalCost, InputCost, arity);				
				
				//if (Pruning && CostSoFar >= upper bound) terminate this task
				if(Pruning &&  CostSoFar >= *LocalUB ) 
				{
					PTRACE2 ("Expr LowerBd %s, exceed Cond UpperBd %s,Pruning applied!",
						CostSoFar.Dump(), LocalUB->Dump() );
					PTRACE("This happened at group %d ", IGNo);
					
					delete ReqProp;
					delete INFCost;
					goto TerminateThisTask;
				}
				delete ReqProp;
				delete INFCost;
			}
			
			// group is not optimized 
			else if( input != PrevInputNo ) // no winner, and we did not just return from O_GROUP
			{
				PTRACE("No Winner for Input : %d", input);
				
				//Adjust PrevInputNo and InputNo to track progress after returning from pushes
				PrevInputNo = input;
				InputNo = input;
				
				//push this task
				PTasks.push(this);
				PTRACE("push myself, %s", "O_INPUT");
				
				COST * InputBd = new COST(*LocalUB); //Start with upper bound of G's context
				if(Pruning)
				{
					PTRACE("LocalCost is %s", LocalCost->Dump());
					CostSoFar.FinalCost(LocalCost, InputCost, arity);
					*InputBd -= CostSoFar; //Subtract CostSoFar
					*InputBd += *InputCost[input]; //Add IG's contribution to CostSoFar
				}
				
				// update the new motivating bounds, but do not do so if need INFBOUND 
				//M_WINNER::mc[IGNo]->SetUpperBound(InputBd, ReqProp);
				
				PTRACE ("push O_GROUP %d", IGNo);
				PTasks.push(new O_GROUP(IGNo, 0, TaskNo, true) ); // pass context as "any", as the group is not at all optimized
				
				delete InputBd;
				delete ReqProp;
				delete INFCost;
				return;
			}
			else // We just returned from O_GROUP on IG
			{
				// impossible plan for this context
				PTRACE ("impossible plan since no winner possible at input %d", InputNo);
				delete ReqProp;
				delete INFCost;
				goto TerminateThisTask;
			}
#else
			
			//call search_circle on IG with that property, infinite cost.
			CONT * IGContext = new CONT(ReqProp, INFCost, false);
			SCReturn = IG -> search_circle(IGContext,  moreSearch);
			
			//If case (1), impossible so terminate
			if(!moreSearch && !SCReturn)
			{
				PTRACE ("Impossible search: Bad input %d", input);
				delete IGContext;
				goto TerminateThisTask;
			}
			
			//else if case (2)
			else if ( !moreSearch && SCReturn)
			{	//There is a winner with nonzero plan, in current input	
				PTRACE("Found Winner for Input : %d", input);
				WINNER * Winner = IG -> GetWinner(ReqProp);
				assert(Winner->GetDone());
				
				//store its cost in InputCost[]
				InputCost[input] = Winner -> GetCost();
				
				CostSoFar.FinalCost(LocalCost, InputCost, arity);				
				//if (Pruning && CostSoFar >= upper bound) terminate this task
				if(Pruning &&  CostSoFar >= *LocalUB ) 
				{
					PTRACE2 ("Expr LowerBd %s, exceed Cond UpperBd %s,Pruning applied!",
						CostSoFar.Dump(), LocalUB->Dump() );
					PTRACE("This happened at group %d ", IGNo);
					
					delete IGContext;
					goto TerminateThisTask;
				}
				delete IGContext;
			}
			
			//Remaining cases are (3) and (4)
			else if( input != PrevInputNo ) // no winner, and we did not just return from O_GROUP
			{
				PTRACE("No Winner for Input : %d", input);
				
				//Adjust PrevInputNo and InputNo to track progress after returning from pushes
				PrevInputNo = input;
				InputNo = input;
				
				//push this task
				PTasks.push(this);
				PTRACE("push myself, %s", "O_INPUT");
				
				//Build a context for the input group task
				//First calculate the upper bound for search of input group.
				//Upper bounds are irrelevant unless we are pruning
				COST * InputBd = new COST(*LocalUB); //Start with upper bound of G's context
				if(Pruning)
				{
					PTRACE("LocalCost is %s", LocalCost->Dump());
					CostSoFar.FinalCost(LocalCost, InputCost, arity);
					*InputBd -= CostSoFar; //Subtract CostSoFar
					*InputBd += *InputCost[input]; //Add IG's contribution to CostSoFar
				}
				
				PHYS_PROP *InputProp = new PHYS_PROP(*ReqProp);
				// update the bound in multiwinner to InputBd
				CONT * InputContext = new CONT(InputProp, InputBd, false);
				CONT::vc.Add (InputContext);
				//Push O_GROUP
				int ContID = CONT::vc.GetSize()-1;
				PTRACE2("push O_GROUP %d, %s", IGNo, CONT::vc[ContID] -> Dump());
				
				if (GlobepsPruning)
				{
					COST * eps_bound;
					if (*EpsBound > *LocalCost)
					{
						eps_bound = new COST(*EpsBound);
						// calculate the cost, the lower nodes should have lower eps bound
						(* eps_bound) -= (*LocalCost);
					}
					else eps_bound = new COST(0);
					if (arity >0) (* eps_bound) /= arity;
					PTasks.push(new O_GROUP(IGNo, ContID, TaskNo, true, eps_bound) );
				}
				else
					PTasks.push(new O_GROUP(IGNo, ContID, TaskNo, true) );
				
				//delete (void*) CostSoFar;
				delete IGContext;
				return;
			}
			else // We just returned from O_GROUP on IG
			{
				// impossible plan for this context
				PTRACE ("impossible plan since no winner possible at input %d", InputNo);
				delete IGContext;
				goto TerminateThisTask;
			}
#endif
		} //Calculate the cost of remaining inputs
		
		// If arity is zero, we need to ensure that this expression can
		// satisfy this required property.
		if( arity ==0 && LocalReqdProp->GetOrder()!= any && Op->is_physical())
		{
			PHYS_PROP * OutputPhysProp = ((PHYS_OP *)Op)->FindPhysProp();
			if( !(*LocalReqdProp == *OutputPhysProp) )
			{
				PTRACE2("physical epxr: %s does not satisfy required phys_prop: %s", 
					MExpr->Dump(), LocalReqdProp->Dump());
				delete  OutputPhysProp;
				
				goto TerminateThisTask;
			}
			delete  OutputPhysProp;
		}
		
		//All inputs have been been optimized, so compute cost of the expression being optimized.
		
#ifdef FIRSTPLAN
		//If we are in the root group and no plan in it has been costed
		if(!(MExpr -> GetGrpID() ) && !(LocalGroup -> getfirstplan()))
		{
			OUTPUT("First Plan is costed at task %d\r\n", TaskNo);
			LocalGroup -> setfirstplan(true);
#ifndef _TABLE_
			long time;	//total seconds from start to finish
			unsigned short msecs;	//milliseconds from start to finish
			struct _timeb start, finish;
			_ftime( &finish );
			if (finish.millitm >= start.millitm)
			{
				time = finish.time - start.time;
				msecs = finish.millitm - start.millitm;
			}
			else
			{
				time = finish.time - start.time - 1;
				msecs = 1000 + finish.millitm - start.millitm;
			}
			long hrs, mins, secs;	// Printed differences from start to finish
			secs = time % 60;
			mins = ((time-secs)/60) % 60;
			hrs = (time-secs-60*mins)/3600 ;
			CString tmpbuf;
			tmpbuf.Format("%0.2d:%0.2d:%0.2d.%0.3d\r\n",hrs, mins, secs, msecs );
			OUTPUT( "elapsed time:\t%s", tmpbuf);
#endif
		}
#endif
		
		CostSoFar.FinalCost( LocalCost, InputCost, arity);
		PTRACE ("Expression's Cost is %s",CostSoFar.Dump());
#ifdef _COSTS_
		OUTPUT("COSTED %s  ", MExpr-> Dump() );
		OUTPUT("%s\r\n", CostSoFar.Dump() );
#endif	
		
		if (GlobepsPruning)
		{
			PTRACE("Current Epsilon Bound is %s", EpsBound->Dump());
			//If global epsilon pruning is on, we may have an easy winner
			if( *EpsBound >= CostSoFar )
			{
				//we are done with this search, we have a final winner
				PTRACE("Global Epsilon Pruning fired, %s",
					"got a final winner for this context");
				
				COST * WinCost = new COST(CostSoFar);
				//COST WinCost(CostSoFar);
				LocalGroup -> NewWinner(LocalReqdProp, MExpr, WinCost, true);
				// update the upperbound of the current context
				CONT::vc[ContextID]->SetUpperBound(CostSoFar);
				CONT::vc[ContextID]->done();
				goto TerminateThisTask;
			}
		}
		
		// if halt, halt optimize the group when either number of plans since the
		// last winner >= HaltGrpSize*EstiGrpSize or the improvement in last HaltWinSize
		// winners is <= HaltImpr. This only works for EQJOIN
#ifndef IRPROP
		if (Halt)
		{
			if (LocalGroup->GetFirstLogMExpr()->GetOp()->GetName().Compare("EQJOIN") == 0)
			{
				double esti_grp_size = LocalGroup->GetEstiGrpSize();
				int    plan_count = LocalGroup->GetCount();
				double halt_size = esti_grp_size*HaltGrpSize/100;
				
				PTRACE("Estimate Group Size is %f", esti_grp_size);
				PTRACE("Number of Plans Since Last Winner is %d", plan_count);
				PTRACE("Halt Size is %f", halt_size);
				
				// if count>halt_size, we can stop here, and set the current winner as final winner
				if (plan_count >= halt_size)
				{
					PTRACE("Halting condition satisfied, %s",
						"got a final winner for this context");
					
					if(! LocalWinner -> GetMPlan() ||   ///If there is no non-null local winner
						CostSoFar < *(LocalWinner->GetCost())  //and current expression is more expensive
						) 
					{
						
						//update the winner
						COST * WinCost = new COST(CostSoFar);
						LocalGroup -> NewWinner(LocalReqdProp, MExpr, WinCost, true);
						// update the upperbound of the current context
						CONT::vc[ContextID]->SetUpperBound(CostSoFar);
						CONT::vc[ContextID]->done();
					}
					else
					{
						LocalWinner->SetDone(true);
						CONT::vc[ContextID]->done();
					}
					goto TerminateThisTask;
				}
			}
		}
#endif
		
		//Check that winner satisfies current context 
		if( CostSoFar >= *LocalUB ) 
		{
			PTRACE2 ("total cost too expensive: totalcost %s >= upperbd %s",
				CostSoFar.Dump(), LocalUB->Dump());
			
			goto TerminateThisTask;			
		}
		
		// compare cost to current winner for this context
		// update the winner and upperbound accordingly
#ifdef IRPROP
		if((M_WINNER::mc[GrpNo]-> GetBPlan(LocalReqdProp) != NULL) && 
			(CostSoFar >= *(M_WINNER::mc[GrpNo]->GetUpperBd(LocalReqdProp))) )
		{
			goto TerminateThisTask;
		}
		else
		{
			GROUP * Group = Ssp->GetGroup(GrpNo);
			COST * WinCost = new COST(CostSoFar);
			
			M_EXPR *OldWinner = M_WINNER::mc[GrpNo]->GetBPlan(LocalReqdProp);
			
			if (OldWinner != NULL)
			{
				// decrement the counter of old winner and if it becomes 0, delete it
				OldWinner->DecCounter();
				
				if (OldWinner->GetCounter() == 0)
				{
					PTRACE("Deleted Physical MExpr %s  !!!\n", OldWinner->Dump());
					Group->DeletePhysMExpr(OldWinner);
				}
			}
			
			// update the multiwinner with new winner MEXPR and its cost
			M_WINNER::mc[GrpNo]->SetBPlan(MExpr, ContextID);
			M_WINNER::mc[GrpNo]->SetUpperBound(WinCost, LocalReqdProp);
			
			// inc the count of number of winner pointing to this MEXPR
			MExpr->IncCounter();
			
			if(Last)
				// this's the last task for the group, so mark the group with completed optimizing
				Group->set_optimized(true);
			
			// set the flag, so that the changed search space is output onto the trace
			Group->set_changed(true);
			
			// if the new winner is not good for any of the contexts, delete it
			if ((ContNo ==0 ) && (MExpr->GetCounter() == 0) )
			{
				PTRACE("New winner %s is not cheaper than old winner, so deleted !!!", MExpr->Dump());
				Group->DeletePhysMExpr(MExpr);
			}
			
			delete this;
			return;
		}
#else
		if(LocalWinner -> GetMPlan( ) &&			//If there is already a non-null local winner
			CostSoFar >= *(LocalWinner->GetCost())  //and current expression is more expensive
			) 
			goto TerminateThisTask;	//Leave the non-null local winner alone
		else
		{  
			//The expression being optimized is a new winner
			
			COST * WinCost = new COST(CostSoFar);
			LocalGroup -> NewWinner(LocalReqdProp, MExpr, WinCost, Last);
			
			// update the upperbound of the current context
			CONT::vc[ContextID]->SetUpperBound(CostSoFar);
			
			PTRACE ("New winner, update upperBd : %s", CostSoFar.Dump() );	
			//delete CostSoFar;
			
			goto TerminateThisTask;
		}
#endif
		
TerminateThisTask :
		
		PTRACE ("O_INPUTS %s","this task is terminating.");
		//delete (void*) CostSoFar;
		
		// if this is the last task in the group, set the localwinner done=true
#ifndef IRPROP
		if (Last) {
			LocalWinner = LocalGroup ->GetWinner(LocalReqdProp);
			LocalWinner->SetDone(true);
#ifdef _DEBUG
			CString os;
			M_EXPR * TempME = LocalWinner ->GetMPlan();
			os.Format("Terminate: replaced winner with %s, %s, %s\r\n", 
				LocalReqdProp-> Dump(), TempME? TempME ->Dump(): " NULL ", 
				LocalWinner->GetCost()->Dump());
			PTRACE("%s", os);
#endif
		}
#endif
		
		if(NO_PHYS_IN_GROUP)
			// delete this physical mexpr to save memory; it may not be used again.
			delete MExpr;	
		
		if(Last)
			// this's the last task for the group, so mark the group with completed optimizing
			Ssp->GetGroup(MExpr->GetGrpID())->set_optimized(true);
		
#ifdef IRPROP
		// if the new MExpr is not good for any contexts, delete it
		if ((ContNo ==0 ) && (MExpr->GetCounter() == 0) )
		{
			assert( MExpr->GetOp()->is_physical() );
			(Ssp->GetGroup(GrpNo))->DeletePhysMExpr(MExpr);
		}
#endif
		
		// tasks must destroy themselves
        delete this;
		
    } // O_INPUTS::perform
    
    
//##ModelId=3B0C085E0311
    CString O_INPUTS::Dump()
    {
		CString os;
		CString temp;
		os.Format("O_INPUTS expression: %s,", MExpr->Dump());
		temp.Format(" parent task %d,", ParentTaskNo);
		os += temp;
#ifdef IRPROP
		int GrpNo = MExpr->GetGrpID();
		temp.Format(" %s", (M_WINNER::mc[GrpNo]->GetPhysProp(ContextID))->Dump());
		os += temp;
		return os;
#else
		temp.Format(" %s", CONT::vc[ContextID]->Dump());
		os += temp;
		return os;
#endif
    } //Dump
    
    //  ***************  APPLY_RULE  *****************
//##ModelId=3B0C085F0100
    APPLY_RULE::APPLY_RULE (
		RULE *rule,
		M_EXPR * mexpr,
		bool explore,
		int ContextID,
		int parent_task_no,
		bool last,
		COST * bound)
        : TASK(ContextID,parent_task_no), 
		Rule(rule), MExpr(mexpr), explore(explore),
		Last(last), EpsBound(bound)
    {
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_APPLY_RULE].New();
    } ; // APPLY_RULE::APPLY_RULE
    
//##ModelId=3B0C085F0132
    APPLY_RULE::~APPLY_RULE()
    {
		if(Last)
		{
			GROUP * Group = Ssp->GetGroup(MExpr->GetGrpID());
			if(!explore)
			{
#ifndef IRPROP
				CONT * LocalCont = CONT::vc[ContextID];
				//What prop is required of
				PHYS_PROP * LocalReqdProp =  LocalCont -> GetPhysProp();	
				WINNER * Winner = Group->GetWinner(LocalReqdProp);
				
				if(Winner)
					assert(!Winner->GetDone());
				
				// mark the winner as done
				Winner->SetDone(true);
#endif
				// this's still the last applied rule in the group, 
				// so mark the group with completed optimization or exploration
				Ssp->GetGroup(MExpr->GetGrpID())->set_optimized(true);
			}
			else Ssp->GetGroup(MExpr->GetGrpID())->set_explored(true);
		}
		
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_APPLY_RULE].Delete();
		if (EpsBound) delete EpsBound;
    };// APPLY_RULE::~APPLY_RULE
    
//##ModelId=3B0C085F0133
	void APPLY_RULE::perform()
    {
		CONT * Context = CONT::vc[ContextID];
		
		PTRACE2 ("APPLY_RULE performing, rule: %s expression: %s", 
			Rule->GetName(), MExpr->Dump() );
		PTRACE2 ("Context ID: %d , %s", ContextID, CONT::vc[ContextID]->Dump() );
		PTRACE ("Last flag is %d", Last);
		
        //if stop generating logical expression when epsilon prune is applied
		//if this context is done, stop
#ifndef _GEN_LOG
        //Check that this context is not done
        if(Context -> is_done())
		{
            PTRACE("Context: %s is done", 
				Context->GetPhysProp()->Dump());
            delete this;
            return;
        }
#else
		//if not stop generating logical expression when epsilon prune is applied
		//if this context is done and the substitute is physical, if the substitute
		//is logical continue
        if(Context -> is_done() && Rule->is_log_to_phys())
		{
            PTRACE("Context: %s is done", 
				Context->GetPhysProp()->Dump());
            delete this;
            return;
        }
#endif
		
#ifdef UNIQ
        //Check again to see that the rule has not been fired since this was put on the stack
        if( !( MExpr->can_fire(Rule->get_index()) ) )
        {
			// tasks must destroy themselves
            PTRACE("Rejected rule %d ", Rule -> get_index());
			
			delete this;
			return;
        }
#endif
		
		if (!ForGlobalEpsPruning) OptStat->FiredRule ++ ; // Count invocations of this task
		
        // main variables for the loop over all possible bindings
        BINDERY	* bindery;  // Expression bindery.
		//    Used to bind MExpr to rule's original pattern
        EXPR	* before;	// see below
        EXPR	* after;	// see below
        M_EXPR	* NewMExpr;	// see below
		
        //Guide to closely related variables
		
        //	original pattern 
        //     APPLY_RULE has a rule member data.  The original pattern is 
		//	   member data of that rule.  It describes (as an EXPR) existing 
		//	   expressions to be bound. 
		//  substitute pattern
        //     from the same rule, as with original pattern.  Describes 
		//	   (as an EXPR) the new expression after the rule is applied.
        //  before
        //     the existing expression which is currently bound to the original
		//	   pattern by the bindery.
        //  after
        //     the new expression, in EXPR form, corresponding to the substitute.
        //  NewMExpr
        //     the new expression, in MEXPR form, which has been included in the
        //     search space.
		
		// Loop over all Bindings of MExpr to the original pattern of the rule
		bindery = new BINDERY(MExpr, Rule->GetOriginal() );
#ifndef _SORT_AFTERS
		for( ; bindery -> advance (); delete before )
		{
			// There must be a Binding since advance() returned non-null.
			// Extract the bound EXPR from the bindery
			before = bindery -> extract_expr ();
			PTRACE ("new Binding is: %s", before->Dump() );
#ifdef _DEBUG
			Bindings[Rule->get_index()]++;
#endif
			// check the rule's condition function
			CONT * Cont = CONT::vc[ContextID] ;
			PHYS_PROP * ReqdProp =  Cont -> GetPhysProp();	//What prop is required of
			
			if (! Rule->condition ( before, MExpr, ContextID) )
			{ 
				PTRACE ("Binding FAILS condition function, expr: %s",MExpr->Dump() );
				continue;	// try to find another binding
			}
			PTRACE ("Binding SATISFIES condition function.  Mexpr: %s",MExpr->Dump() );
			
#ifdef _DEBUG
			Conditions[Rule -> get_index()]++;
#endif
			// try to derive a new substitute expression
			after = Rule -> next_substitute (before, ReqdProp);
			
			assert(after != NULL) ;
			
			PTRACE("substitute expr is : %s", after->Dump() );
			
			// include substitute in MEMO, find duplicates, etc.
			GRP_ID group_no = MExpr->GetGrpID();
			
			if(NO_PHYS_IN_GROUP)
			{	// don't include physical mexprs into group
				if(after->GetOp()->is_logical())
					NewMExpr = Ssp->CopyIn(after, group_no);
				else
					NewMExpr = new M_EXPR(after,group_no);
			}
			else  //include physical mexpr into group
				NewMExpr = Ssp->CopyIn(after, group_no);
			
			// If substitute was already known 
			if (NewMExpr == NULL)
			{
				PTRACE("duplicate substitute %s", after->Dump());
				
				delete after;		// "after" no longer used
				
				continue;	// try to find another substitute
			}	
			
			PTRACE("New Mexpr is : %s", NewMExpr->Dump() );
			Memo_M_Exprs++;
			PTRACE("New MEXPR %d", 3);
			PTRACE("Memo_M_Exprs value is %d", Memo_M_Exprs);
			
			delete after;		// "after" no longer used
			
			//Give this expression the rule's mask
			NewMExpr -> set_rule_mask ( Rule -> get_mask() );
			
			//We need to handle this case for rules like project -> NULL,
			//by merging groups
			assert(MExpr->GetGrpID() == NewMExpr->GetGrpID() );
			
			bool Flag=false;
			if(Last) 			
			// this's the last applied rule in the group,pass it to the new task
			{	Last = false;			// turn off this, since it's no longer the last task
			Flag = true;
			}
			
			// follow-on tasks
			if (explore ) // optimizer is exploring, the new mexpr must be logical expr
			{
				assert( NewMExpr->GetOp()->is_logical() );
				PTRACE ("new task to explore new expression, \
					pushing O_EXPR exploring expr: %s", NewMExpr->Dump() );
				if (GlobepsPruning)
				{
					COST * eps_bound = new COST(*EpsBound);
					PTasks.push (new O_EXPR (NewMExpr, true, ContextID, TaskNo, Flag, eps_bound));
				}
				else
					PTasks.push (new O_EXPR (NewMExpr, true, ContextID, TaskNo, Flag));
			} // optimizer is exploring
			else // optimizer is optimizing
			{
				// for a logical op, try further transformations
				if (NewMExpr->GetOp()->is_logical() )
				{
					PTRACE ("new task to optimize new expression,pushing O_EXPR, expr: %s", NewMExpr->Dump() );
					if (GlobepsPruning)
					{
						COST * eps_bound = new COST(*EpsBound);
						PTasks.push (new O_EXPR (NewMExpr, false, ContextID, TaskNo, Flag, eps_bound));
					}
					else
						PTasks.push (new O_EXPR (NewMExpr, false, ContextID, TaskNo, Flag));
				} // further transformations to optimize new expr
				else
				{
					// for a physical operator, optimize the inputs
					/* must be done even if op_arg -> arity == 0 in order to calculate costs */
					assert( NewMExpr->GetOp()->is_physical() );
					
					PTRACE ("new task to optimize inputs,pushing O_INPUT, epxr: %s", NewMExpr->Dump() );
					if (GlobepsPruning)
					{
						COST * eps_bound = new COST(*EpsBound);
						PTasks.push (new O_INPUTS (NewMExpr, ContextID, TaskNo, Flag, eps_bound) );	
					}
					else
					{
						int contextNo = 0; 
						int j = 0;
#ifdef IRPROP
						if(Last)
							Last = false;
						
						int GrpNo = NewMExpr->GetGrpID();
						if ((NewMExpr->GetOp())->GetName() == "QSORT")
							j = 1;
						for (int i=j; i<M_WINNER::mc[GrpNo]->GetWide(); i++)
						{
							if (i != ContextID)
							{
								PTasks.push (new O_INPUTS (NewMExpr, i, TaskNo, Flag, NULL, contextNo++) );
							}
						}
						if (!(j==1 && ContextID == 0))
							PTasks.push (new O_INPUTS (NewMExpr, ContextID, TaskNo, Flag, NULL, contextNo++) );
#else
						PTasks.push (new O_INPUTS (NewMExpr, ContextID, TaskNo, Flag, NULL) );
#endif
					}
					
				} // for a physical operator, optimize the inputs
				
			} // optimizer is optimizing
			
		} // try all possible bindings
#endif
		
#ifdef _SORT_AFTERS
		// a temporary array just for holding the elements
		CArray <AFTERS, AFTERS> AfterArray;
		// get all the substitutions, put them in the array, sort the array
		// according to the estimanted cost, and push the most expensive task 
		// first, so that we can get lowest LB soon
		for( ; bindery -> advance (); delete before )
		{
			// There must be a Binding since advance() returned non-null.
			// Extract the bound EXPR from the bindery
			before = bindery -> extract_expr ();
			PTRACE ("new Binding is: %s", before->Dump() );
			
			// check the rule's context function
			CONT * Cont = CONT::vc[ContextID] ;
			PHYS_PROP * ReqdProp =  Cont -> GetPhysProp();	//What prop is required of
			if (! Rule->condition ( before, MExpr, ReqdProp) )
			{ 
				PTRACE ("Binding FAILS condition function, expr: %s",MExpr->Dump() );
				continue;	// try to find another binding
			}
			PTRACE ("Binding SATISFIES condition function.  Mexpr: %s",MExpr->Dump() );
			
			// try to derive a new substitute expression
			after = Rule -> next_substitute (before, ReqdProp);
			
			assert(after != NULL) ;
			
			PTRACE("substitute expr is : %s", after->Dump() );
			
			// include substitute in MEMO, find duplicates, etc.
			GRP_ID group_no = MExpr->GetGrpID();
			
			if(NO_PHYS_IN_GROUP)
			{	// don't include physical mexprs into group
				if(after->GetOp()->is_logical())
					NewMExpr = Ssp->CopyIn(after, group_no);
				else		
					NewMExpr = new M_EXPR(after,group_no);
			}
			else  //include physcial mexpr into group
				NewMExpr = Ssp->CopyIn(after, group_no);
			
			// If substitute was already known 
			if (NewMExpr == NULL)
			{
				PTRACE("duplicate substitute %s", after->Dump());
				
				delete after;		// "after" no longer used
				
				continue;	// try to find another substitute
			}	
			
			PTRACE("New Mexpr is : %s", NewMExpr->Dump() );
			
			delete after;		// "after" no longer used
			
			//Give this expression the rule's mask
			NewMExpr -> set_rule_mask ( Rule -> get_mask() );
			
			AFTERS element;
			element.m_expr = NewMExpr;
			//calculate the estimate cost
			COST ** InputCost;
			COST *TotalCost = new COST(0);
			COST *LocalCost;
			LOG_PROP ** InputLogProp;
			int arity = NewMExpr->GetArity();
			if (arity)
			{
				InputCost = new COST*[arity];
				InputLogProp = new LOG_PROP*[arity];
				int input;
				for(input= 0; input< arity; input++)
				{
					GRP_ID IGNo;	//Input Group Number
					GROUP * IG;
					IGNo = NewMExpr -> GetInput(input);
					IG = Ssp -> GetGroup(IGNo);  //Group of current input
					InputCost[input] = IG->GetLowerBd();
					InputLogProp[input] = IG->get_log_prop();
				}
			}
			LOG_PROP * LogProp = Ssp->GetGroup(group_no)->get_log_prop();
			
			// if it is physical operator, plus the local cost
			if (NewMExpr->GetOp()->is_physical()) 
				LocalCost = NewMExpr->GetOp()->FindLocalCost(LogProp, InputLogProp);
			else LocalCost = new COST(0);
			TotalCost->FinalCost(LocalCost, InputCost, arity);
			element.cost = TotalCost;
			
			if (arity)
			{
				delete [] InputCost;
				delete [] InputLogProp;
			}
			delete LocalCost;
			AfterArray.Add(element);
		}
		int num_afters = AfterArray.GetSize();
		// copy the array to static array
		AFTERS *Afters = new AFTERS[num_afters];
		for (int array_index =0; array_index <num_afters; array_index++)
		{
			Afters[array_index].m_expr = AfterArray[array_index].m_expr;
			Afters[array_index].cost = AfterArray[array_index].cost;
		}
		if (num_afters>1)
			qsort (Afters, num_afters, sizeof (AFTERS), compare_afters);
		// push tasks in the order of estimate cost, most expensive first
		while ( -- num_afters >= 0)
		{
			//Give this expression the rule's mask
			Afters[num_afters].m_expr -> set_rule_mask ( Rule -> get_mask() );
			
			//We need to handle this case for rules like project -> NULL,
			//by merging groups
			assert(MExpr->GetGrpID() == Afters[num_afters].m_expr->GetGrpID() );
			
			bool Flag=false;
			if(Last) 			
				// this's the last applied rule in the group,pass it to the new task
			{	Last = false;			// turn off this, since it's no longer the last task
			Flag = true;
			}
			
			// follow-on tasks
			if (explore ) // optimizer is exploring, the new mexpr must be logical expr
			{
				assert( AfterArray[num_afters].m_expr->GetOp()->is_logical() );
				PTRACE ("new task to explore new expression, \
					pushing O_EXPR exploring expr: %s", Afters[num_afters].m_expr->Dump() );
				PTasks.push (new O_EXPR (Afters[num_afters].m_expr, true, ContextID, TaskNo, Flag));
				
			} // optimizer is exploring
			else // optimizer is optimizing
			{
				// for a logical op, try further transformations
				if (Afters[num_afters].m_expr->GetOp()->is_logical() )
				{
					PTRACE ("new task to optimize new expression,pushing O_EXPR, expr: %s", 
						Afters[num_afters].m_expr->Dump() );
					PTasks.push (new O_EXPR (Afters[num_afters].m_expr, false, ContextID, TaskNo, Flag));
				} // further transformations to optimize new expr
				else
				{
					// for a physical operator, optimize the inputs
					/* must be done even if op_arg -> arity == 0 in order to calculate costs */
					assert( Afters[num_afters].m_expr->GetOp()->is_physical() );
					
					PTRACE ("new task to optimize inputs,pushing O_INPUT, epxr: %s", 
						Afters[num_afters].m_expr->Dump() );
					PTasks.push (new O_INPUTS (Afters[num_afters].m_expr, ContextID, TaskNo, Flag) );
				} // for a physical operator, optimize the inputs
				
			} // optimizer is optimizing
			
			delete Afters[num_afters].cost;
			
		} // end while
		delete [] Afters;
#endif
		
		delete bindery;
		
        //Mark rule vector to show that this rule has fired
		MExpr -> fire_rule(Rule -> get_index() );
		
		// tasks must destroy themselves
		delete this;
		
    } // APPLY_RULE::perform
	
//##ModelId=3B0C085F013C
    CString APPLY_RULE::Dump()
    {
		CString os;
		
		os.Format("APPLY_RULE rule: %s, mexpr %s, parent task %d", Rule->Dump(), 
			MExpr->Dump(), ParentTaskNo);
		
		return os;
    } //Dump
