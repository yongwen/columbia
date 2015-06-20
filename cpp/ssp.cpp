/*	
ssp.cpp -  implementation of class SSP
$Revision: 21 $ 
Implements classes in ssp.h
  
Columbia Optimizer Framework
  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
  
	
*/

/*  ssp.cpp -   search space implementation */

#include "stdafx.h"
#include "tasks.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

#define SHRINK_INERVAL  10000
#define MAX_AVAIL_MEM	40000000	// available memory bound to 50M

extern COST GlobalEpsBound;

//##ModelId=3B0C08650054
SSP::SSP(): NewGrpID(-1)
{
	//initialize HashTbl to contain HashTableSize elements, each initially NULL.
	HashTbl = new M_EXPR* [HtblSize];
	for(ub4 i = 0 ; i < HtblSize ; i++)
		HashTbl[i] = NULL;
}

//##ModelId=3B0C08650055
void SSP::Init()
{
	EXPR *	Expr = Query->GetEXPR(); 
    
	// create the initial search space
	RootGID = NEW_GRPID;
	M_EXPR * MExpr = CopyIn( Expr, RootGID);
    
	InitGroupNum = NewGrpID;
	if(COVETrace)	//End Initializing Search Space
	{
		CString os = "EndInit\r\n";
		OutputCOVE.Write(os, os.GetLength());
	}
}

// free up memory
//##ModelId=3B0C0865005E
SSP::~SSP()
{
	for(int i=0; i< Groups.GetSize();i++)
		delete Groups[i] ;
	for (int j=0; j< M_WINNER::mc.GetSize(); j++)
		delete M_WINNER::mc[j];
	M_WINNER::mc.RemoveAll();
	delete [] HashTbl;
}

//##ModelId=3B0C086500EB
CString SSP::DumpHashTable()
{
	CString os;
	CString temp;
    
	os = "Hash Table BEGIN:\r\n";
	int total=0;
	for(int i=0; i < HtblSize; i++)
	{
		for(M_EXPR *mexpr = HashTbl[i]; mexpr != NULL; mexpr=mexpr->GetNextHash(),total++)
			os += mexpr->Dump();
		if(HashTbl[i]) os += "\r\n";
	}
	temp.Format("Hash Table END, total %d mexpr\r\n",total);
	os += temp;
    
	return os;
}

//##ModelId=3B0C086500E2
CString SSP::DumpChanged()
{
    CString os;
    GROUP* Group;
    
	for(int i=0; i< Groups.GetSize();i++)
		if(Groups[i]->is_changed()) 
		{
			Group = Groups[i];
			os += Group->Dump();
			Group->set_changed(false);
		}
		
		if(os!="") 	return ("Changed Search Space:\r\n" + os);
		else return ("Search Space not changed");
}

//##ModelId=3B0C086500C3
void SSP::Shrink()
{
	for(int i=InitGroupNum; i<Groups.GetSize();i++)	ShrinkGroup(i) ;
}

//##ModelId=3B0C086500B9
void SSP::ShrinkGroup(GRP_ID group_no)
{
    GROUP* Group;
    M_EXPR * mexpr;
    M_EXPR * p;
    M_EXPR * prev;
    int DeleteCount=0;
    
	SET_TRACE Trace(true);
    
	Group = Groups[group_no];
    
	if( ! Group->is_optimized() && ! Group->is_explored() ) return ;   // may be pruned
    
	PTRACE("Shrinking group %d,", group_no);
    
	// Shrink the logical mexpr
	// init the rule mark of the first mexpr to 0, means all rules are allowed
	mexpr =  Group->GetFirstLogMExpr();
	mexpr->set_rule_mask( 0 );
    
	// delete all the mexpr except the first initial one
	mexpr = mexpr->GetNextMExpr();
    
	while( mexpr != NULL )
	{
		// maintain the hash link
		// find my self in the appropriate hash bucket
		ub4 hashval = mexpr->hash();
		for ( p = HashTbl[ hashval ], prev = NULL;
		p != mexpr;  
    				prev = p, p = p -> GetNextHash()) ;
					
					assert(p==mexpr);
					// link prev's next hash to next 
					if(prev) 
						prev->SetNextHash(mexpr->GetNextHash());
					else
						// the mexpr is the first in the bucket
						HashTbl[ hashval ] = mexpr->GetNextHash();
					
					p = mexpr;
					mexpr = mexpr->GetNextMExpr();
					
					delete p; 
					DeleteCount ++;
	}
    
	mexpr =  Group->GetFirstLogMExpr();
	mexpr->SetNextMExpr( NULL );
	// update the lastlogmexpr = firstlogmexpr;
	Group->SetLastLogMExpr(mexpr);
    
	// Shrink the physcal mexpr
	mexpr =  Group->GetFirstPhysMExpr();
    
	while( mexpr != NULL )
	{
		p = mexpr;
		mexpr = mexpr->GetNextMExpr();
		
		delete p; 
		DeleteCount ++;
	}
    
	mexpr =  Group->GetFirstPhysMExpr();
	mexpr->SetNextMExpr( NULL );
	// update the lastlogmexpr = firstlogmexpr;
	Group->SetLastPhysMExpr(mexpr);
    
	Group->set_changed(true);
	Group->set_exploring(false);
	
	PTRACE("Deleted %d mexpr!\r\n", DeleteCount);
}

//##ModelId=3B0C086500D7
CString SSP::Dump()
{
    CString os;
    GROUP* Group;
    
	os.Format("%s%d%s","RootGID:" , RootGID , "\r\n");
    
	for(int i=0; i< Groups.GetSize();i++)
	{
		Group = Groups[i];
		os += Group->Dump();
		Group->set_changed(false);
	}
	
	return os;
}

//##ModelId=3B0C086500E1
void SSP::FastDump()
{
	TRACE_FILE("SSP Content: RootGID: %d\r\n" , RootGID);
    
	for(int i=0; i< Groups.GetSize();i++)
	{
		Groups[i]->FastDump() ;
		Groups[i]->set_changed(false);
	}
}

//##ModelId=3B0C086500A5
M_EXPR * SSP::FindDup (M_EXPR & MExpr)
{
	int Arity = MExpr.GetArity();
	
	ub4 hashval = MExpr.hash ();
	M_EXPR * prev = HashTbl[hashval];
    
	int BucketSize = 0;
	if (!ForGlobalEpsPruning) OptStat->HashedMExpr ++;
	// try all expressions in the appropriate hash bucket
	for (M_EXPR * old = prev;  old != NULL;  prev = old, old = old -> GetNextHash(), BucketSize++ )
	{
		
		int input_no;
		
		// See if they have the same arities
		if (old -> GetArity() != Arity) {
			goto not_a_duplicate;
		}
		
		// finding yourself does not constitute a duplicate
		// compare pointers to see if EXPR_LISTs are the same
		if (old == &MExpr) {
			goto not_a_duplicate;
		}
		
		// compare the inputs
		// Compare the actual group pointers for every input
		for (input_no = Arity;  -- input_no >= 0;  ) 
			if ( MExpr.GetInput(input_no) != old->GetInput(input_no) )
			{
				PTRACE("Different at input %d", input_no);
				goto not_a_duplicate;
			}
			
			// finally compare the Op
			if( ! ( *(old->GetOp()) == MExpr.GetOp() ) ) 
			{
				PTRACE2("Different at Operator. %s : %s", old->Dump(), MExpr.Dump());
                goto not_a_duplicate;
            }
			
			// "expr" is a duplicate of "old"
			return ( old );
			
not_a_duplicate :
			continue;       // check next expression in hash bucket
	} // try all expressions in the appropriate hash bucket
    
	// no duplicate, insert into HashTable
	if(prev == NULL) 
		HashTbl[hashval] = & MExpr;
	else 
		prev->SetNextHash( & MExpr );
    
	if (!ForGlobalEpsPruning)
	{
		if(OptStat->MaxBucket < BucketSize) OptStat->MaxBucket = BucketSize;
	}
    
	return ( NULL );
} // SSP::FindDup

// merge two groups when duplicate found in these two groups
// means they should be the same group
// always merge bigger group_no group to smaller one.

//##ModelId=3B0C086500AE
GRP_ID SSP::MergeGroups(GRP_ID group_no1, GRP_ID group_no2)
{
    //M_EXPR * mexpr;
    
	GRP_ID ToGid = group_no1;
	GRP_ID FromGid = group_no2;
    
	// always merge bigger group_no group to smaller one.
	if( group_no1 > group_no2) 
	{
		ToGid = group_no2;
		FromGid = group_no1;
	}
    
#ifdef UNIQ
	assert(false);
#endif
    
	return ToGid;
}// SSP::MergeGroups


//##ModelId=3B0C08650069
M_EXPR*	SSP::CopyIn(EXPR * Expr, GRP_ID& GrpID)
{
	GROUP *	Group ;
	bool win = true;	//will we initialize nontrivial winners in this group?
	//False if it is a subgroup of a DUMMY operator
    
	//Factor GrpID value into normal value plus win value
	if (GrpID == NEW_GRPID_NOWIN)
	{
		GrpID = NEW_GRPID;
#ifdef DUMNOWIN
		win = false;
#endif
	}
	
	// create the M_Expr which will reside in the group
	M_EXPR * MExpr = new M_EXPR(Expr,GrpID);
    
	// find duplicate.  Done only for logical, not physical, expressions.
	if(MExpr -> GetOp() -> is_logical()) {
		M_EXPR * DupMExpr  = FindDup(*MExpr);
		if( DupMExpr != NULL )		// not null ,there is a duplicate
		{
			if (!ForGlobalEpsPruning) OptStat->DupMExpr ++;		// calculate dup mexpr
			PTRACE("duplicate mexpr : %s", MExpr->Dump());
			
			// the duplicate is in the group the expr wanted to copyin
			if( GrpID == DupMExpr->GetGrpID() )
			{	
				delete MExpr;
				return NULL;
			}
			
			// If the Mexpr is supposed to be in a new group, set the group id 
			if( GrpID == NEW_GRPID )
			{
				GrpID = DupMExpr->GetGrpID();
				
				// because the NewGrpID increases when constructing 
				// an M_EXPR with NEW_GRPID, we need to decrease it
				NewGrpID --;
				
				delete MExpr;
				return NULL;
			}
			else
			{
				// otherwise, i.e., GrpID != DupMExpr->GrpID
				// need do the merge
				GrpID = MergeGroups(GrpID , DupMExpr->GetGrpID() );
				delete MExpr;
				return NULL;
			}
		}  // if( DupMExpr != NULL )
	} //If the expression is logical
	
    
	// no duplicate found
	if(GrpID == NEW_GRPID)
	{
		// create a new group
		Group = new GROUP( MExpr );
		
		// insert the new group into ssp
		GrpID = Group->GetGroupID();
		
		if(GrpID >= Groups.GetSize() )	Groups.SetSize( GrpID + 1 );
		Groups[GrpID] = Group;
		
		
#ifdef IRPROP	
		
		// For the topmost group and for the groups containing the item operator and constant
		// operator, set the only physical property as any and bound INF
		if ( !win || GrpID == 0 || ((MExpr->GetOp())->is_const()) || 
			((MExpr->GetOp())->is_item()) )
		{
			M_WINNER *MWin = new M_WINNER(1);
			M_WINNER::mc.SetAtGrow(GrpID,MWin);
		}
		else
		{
			KEYS_SET * tmpKeySet;
			
			// get the relevant attributes from the schema for this group
			tmpKeySet = (((LOG_COLL_PROP *)(Group->get_log_prop()))->Schema)->AttrStore();
			int ksize = tmpKeySet->GetSize();
			
			M_WINNER *MWin = new M_WINNER(ksize+1);	
			for (int i=1; i<ksize+1; i++)
			{
				int *Keys_Arr = tmpKeySet->CopyOutOne(i-1);
				KEYS_SET *MKeys_Set = new KEYS_SET(Keys_Arr,1);
				delete [] Keys_Arr;
				
				PHYS_PROP *Prop = new PHYS_PROP(MKeys_Set, sorted);
				Prop->KeyOrder.Add(ascending);
				MWin->SetPhysProp(i, Prop);
			}
			
			delete tmpKeySet;
			M_WINNER::mc.SetAtGrow(GrpID,MWin);
		}
#endif
		
	}
	else 
	{
		Group = GetGroup(GrpID);
		
		// include the new MEXPR
		Group->NewMExpr(MExpr);
	}
	// set the flag
	Group->set_changed(true);
    
	return MExpr;
    } // SSP::CopyIn
    
//##ModelId=3B0C0865007C
    void SSP::CopyOut(GRP_ID GrpID, PHYS_PROP * PhysProp, int tabs)
    { 
		//Find the winner for this Physical Property.
		//print the Winner's Operator and cost
		GROUP * ThisGroup = Ssp -> GetGroup(GrpID);
		
#ifndef IRPROP
		WINNER * ThisWinner;
#endif
		
		M_EXPR * WinnerMExpr;
		OP * WinnerOp ;
		CString os;
		
		//special case : it's a const group
		if( ThisGroup->GetFirstLogMExpr()->GetOp()->is_const() )
		{
#ifdef IRPROP		
			WinnerMExpr = M_WINNER::mc[GrpID]->GetBPlan(PhysProp);
#else
			WinnerMExpr = ThisGroup->GetFirstLogMExpr();
#endif
			os = WinnerMExpr->GetOp()->Dump() ;
			os += ", Cost = 0\r\n" ;
			OUTPUTN(tabs, os); 
		}
		
		//It's an item group
		else if (ThisGroup->GetFirstLogMExpr()->GetOp()->is_item() )
		{
#ifdef IRPROP		
			WinnerMExpr = M_WINNER::mc[GrpID]->GetBPlan(PhysProp);
			if (WinnerMExpr == NULL)
			{
				os.Format("No optimal plan for group: %d with phys_prop: %s\r\n", GrpID, PhysProp->Dump());
				OUTPUTN(tabs, os);
				return;
			}
#else
			ThisWinner = ThisGroup -> GetWinner(PhysProp);
			
			if( ThisWinner == NULL )  
			{
				os.Format("No optimal plan for group: %d with phys_prop: %s\r\n", GrpID, PhysProp->Dump());
				OUTPUTN(tabs, os);
				return;
			}
			
			assert(ThisWinner->GetDone());
			WinnerMExpr = ThisWinner -> GetMPlan();
#endif
			WinnerOp = WinnerMExpr -> GetOp();
			
			os.Format("%s", WinnerOp -> Dump());
			
			os += ", Cost = " ;
			
			OUTPUTN(tabs, os); 
			
#ifdef IRPROP
			COST * WinnerCost = M_WINNER::mc[GrpID]->GetUpperBd(PhysProp);
#else
			COST * WinnerCost = ThisWinner -> GetCost();
#endif
			os.Format("%s\r\n", WinnerCost -> Dump() );
			
			OUTPUT("%s",os);
			PHYS_PROP *InputProp;
			//print the input recursively
			for(int i = 0; i < WinnerMExpr -> GetArity(); i++) 
			{
				InputProp = new PHYS_PROP(any);
				CopyOut(WinnerMExpr -> GetInput(i), InputProp, tabs+1);
				delete InputProp;
			}
		}
		
		//it's a normal group
		else
		{
			//First extract the winning expression for this property
#ifndef IRPROP
			ThisWinner = ThisGroup -> GetWinner(PhysProp);
			
			if( ThisWinner == NULL )  
			{
				os.Format("No optimal plan for group: %d with phys_prop: %s\r\n", GrpID, PhysProp->Dump());
				OUTPUTN(tabs, os);
				return;
			}
			
			assert(ThisWinner->GetDone());
			WinnerMExpr = ThisWinner -> GetMPlan();
			
#else
			WinnerMExpr = M_WINNER::mc[GrpID]->GetBPlan(PhysProp);
			if (WinnerMExpr == NULL)
			{
				os.Format("No optimal plan for group: %d with phys_prop: %s\r\n", GrpID, PhysProp->Dump());
				OUTPUTN(tabs, os);
				return;
			}
			
#endif
			
			//Now extract the operator from the expression and write it to the output string
			// along with   " cost = " .  Print output string to window.
			assert (WinnerMExpr != NULL);
			
			WinnerOp = WinnerMExpr -> GetOp();
			os.Format("%s", WinnerOp -> Dump());
			if(WinnerOp->GetName()=="QSORT") os += PhysProp->Dump();
			os += ", Cost = " ;
			
			
#ifndef _TABLE_
			if(!SingleLineBatch)
				OUTPUTN(tabs, os); 
#endif
			
			//Extract cost of the winner, write it to the output string and 
			// print output string to window.
#ifndef IRPROP
			COST * WinnerCost = ThisWinner -> GetCost();
#else
			COST * WinnerCost = M_WINNER::mc[GrpID]->GetUpperBd(PhysProp);
#endif
			os.Format("%s\r\n", WinnerCost -> Dump() );
			
#ifndef _TABLE_
			OUTPUT("%s",os);
			if(SingleLineBatch)	//In this case we want only the total cost of the Winner
			{
				OUTPUT("%s","\r\n");
				return;
			}
#else
			OUTPUT("\t%s\r\n", WinnerCost -> Dump() );
#endif
			
			
			//Recursively print inputs
#ifndef _TABLE_
			int Arity = WinnerOp -> GetArity();
			PHYS_PROP * ReqProp;
			bool possible;
			for(int i = 0; i < Arity ; i++) 
			{
				GRP_ID input_groupno = WinnerMExpr -> GetInput(i);
				
				ReqProp = ((PHYS_OP*)WinnerOp) -> InputReqdProp(PhysProp, 
					Ssp->GetGroup(input_groupno)->get_log_prop(),
					i, possible);
				
				assert(possible); //Otherwise optimization fails
				
				CopyOut(input_groupno, ReqProp, tabs+1);
				
				delete ReqProp ;  
			}
#endif
		}
    } //SSP::CopyOut()
    
#ifdef FIRSTPLAN
//##ModelId=3B0C0867021A
	bool GROUP::firstplan = false;
#endif
    
    /* bool GROUP::search_circle(CONT * C, bool & moresearch) 
    	{
    		First search for a winner with property P.
    		If there is no such winner, case (3)
		If there is a winner, denote its plan component by WPlan and 
			its cost component by WCost.
			Context cost component is CCost
    
		If (WPlan is non-null) //Cheapest plan costs *WCost;
                         //we seek a plan costing *CCost or less
			If (*WCost <= *CCost) 
    				Case (2)
			else if (*CCost < *WCost)
    				Case (1)
		else If (WPlan is null) //All plans cost more than *WCost
			if( *CCost <= *WCost)
    				Case (1)
			else if (*WCost < *CCost) //There might be a plan between WCost and CCost
    				Case (4)
    */
    
#ifdef IRPROP
//##ModelId=3B0C08670095
    bool GROUP::search_circle(int GrpNo, PHYS_PROP *PhysProp, bool & moreSearch)
    {
		// check if there is a winner for property "any"
		M_EXPR *Winner = M_WINNER::mc[GrpNo]->GetBPlan(0);
		if (Winner == NULL)
			moreSearch = true;	// group is not optimized, moreSearch needed
		else
			moreSearch = false; // the group is completely optimized
		
		COST *CCost = new COST(-1);
		if (!moreSearch) // group is optimized
		{
			M_EXPR *MWin = M_WINNER::mc[GrpNo]->GetBPlan(PhysProp);
			COST *WinCost = M_WINNER::mc[GrpNo]->GetUpperBd(PhysProp);
			if (MWin != NULL)
			{
				// winner's cost is within the context's bound
				if (*CCost >= *WinCost)
				{
					delete CCost;
					return true;
				}
				else
				{
					delete CCost;
					return false;
				}
			}
			else // since the group is optimized, NULL plan means winner not possible
			{
				delete CCost;
				return false;
			}
		}
		else // group not optimized 
		{
			delete CCost;
			return false;
		}
    }
#endif
    
	/* GROUP::search_circle
	Map between four cases (see header file) and the way they arise:

    No winner for this property:  (3)

					WCost >= CCost	WCost < CCost	WCost <= CCost	WCost > CCost
	
	MPlan is Null			(1)			  (4)
	
	MPlan not Null										  (2)			  (1)

    */
//##ModelId=3B0C0867008B
    bool GROUP::search_circle(CONT * C, bool & moreSearch)
    {
		//First search for a winner with property P.
		WINNER * Winner = GetWinner(C -> GetPhysProp());
		
		//If there is no such winner, case (3)
		if(!Winner) 
		{
			moreSearch = true;
			return(false);
		}
		
		assert(Winner->GetDone());	//This is not a recursive query
		
		//If there is a winner, denote its plan, cost components by M and WCost
		//Context cost component is CCost
		M_EXPR * M = Winner -> GetMPlan();
		COST * WCost = Winner -> GetCost();
		COST * CCost = C -> GetUpperBd();
		assert(CCost);	//Did we get rid of all cruft?
		
		if (M)// there is a non-null winner
		{
			if(*CCost >= *WCost) //Real winner; CCost is less of a constraint.  Case (2)
			{
				moreSearch = false;
				return(true);
			}
			else //search is impossible as winner's cost is more than required context cost (1)
			{
				moreSearch = false;
				return(false);
			}
		}
		else  //Winner's Mplan is null.
		{
			if( *WCost >= *CCost)  //Previous search failed and CCost is more of a constraint. (1)
			{
				moreSearch = false;
				return(false);
			}
			else //Previous search failed but CCost is less of a constraint.  (4)
			{
				moreSearch = true;
				return(true);
			}
		}
    }
    
//##ModelId=3B0C086700A7
    WINNER * GROUP::GetWinner(PHYS_PROP * PhysProp)
    {
		int Size = Winners.GetSize();
        for(int i = 0; i<Size; i++)
		{
			PHYS_PROP * WinPhysProp = Winners[i] -> GetPhysProp();
			
			if( *WinPhysProp == *PhysProp) return( Winners[i]);
		}
		
        //No matching winner
        return(NULL);
		
    } // GROUP::GetWinner
    
//##ModelId=3B0C086700B1
    void GROUP::NewWinner(PHYS_PROP * ReqdProp, M_EXPR * MExpr, COST * TotalCost, 
		bool done)
    {
		if(COVETrace && MExpr)	//New Winner
		{
			CString os;
			os.Format("NewWin %d \"%s\"%s  { %d %d \"%s\" %s }\r\n",
				MExpr -> GetGrpID(), ReqdProp -> Dump(), TotalCost -> Dump(),
				MExpr -> GetGrpID(),int(MExpr), 
				MExpr -> Dump(), done?"Done":"Not Done");
			
			OutputCOVE.Write(os, os.GetLength());
			
		}
		
		this -> set_changed(true);
		
		//Seek winner with property ReqdProp in the winner's circle
        for(int i = Winners.GetSize(); --i>=0;)
		{
			if( *(Winners[i] -> GetPhysProp()) == *ReqdProp) 
			{
				//Update the winner for the new search just begun
				delete Winners[i];
				Winners[i] = new WINNER(MExpr, ReqdProp, TotalCost, done);
				return;
			}
		}
		
		//No matching winner for this property
		Winners . Add(new WINNER(MExpr, ReqdProp, TotalCost, done));
		
		return;
    }
    
//##ModelId=3B0C086700C6
    bool GROUP::CheckWinnerDone()
    {
		//Search Winner's circle.  If there is a winner done, return true
		
		int Size = Winners.GetSize();
		
		for(int i = 0; i<Size; i++)
		{
			if (Winners[i] -> GetDone() ) return (true);
		}
		
		//No winner is done
		return(false);
    } // GROUP::CheckWinnerDone
    
	
//##ModelId=3B0C086703BE
    WINNER::WINNER(M_EXPR *MExpr, PHYS_PROP *PhysProp, COST *Cost, bool done)
		:Cost(Cost), 
		MPlan( (MExpr==NULL) ? NULL : (new M_EXPR(*MExpr)) ), 
		PhysProp(PhysProp),	
		Done(done)
    { if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_WINNER].New(); };  
    
    
//##ModelId=3B0C0868021B
    M_WINNER::M_WINNER(int S)
    {
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_M_WINNER].New();
		
		wide = S;
		PhysProp = new PHYS_PROP* [S];
		Bound = new COST* [S];
		BPlan = new M_EXPR* [S];
		
		// set the first physical property as "any" for all groups
		PhysProp[0] = new PHYS_PROP(any);
		
		// set the cost to INF and plan to NULL initially for all groups
		for (int i=0; i<S; i++)
		{
			Bound[i] = new COST(-1);
			BPlan[i] = NULL;
		}
    };
    
//##ModelId=3B0C08680192
    CArray< M_WINNER * , M_WINNER* > M_WINNER::mc;
//##ModelId=3B0C086801A4
	COST M_WINNER::InfCost(-1);
	
	int TaskNo;
	int Memo_M_Exprs;
	
//##ModelId=3B0C08650068
    void SSP::optimize()
    {
#ifdef FIRSTPLAN
		Ssp -> GetGroup(0) -> setfirstplan(false);
#endif
		
		SET_TRACE Trace(true);
		
		//Create initial context, with no requested properties, infinite upper bound,
		// zero lower bound, not yet done.  Later this may be specified by user.
		if (CONT::vc.GetSize() == 0)
		{
			CONT * InitCont = new CONT( new PHYS_PROP(any), new COST(-1), false);
			//Make this the first context
			CONT::vc.Add (InitCont);
		}
		//assert(CONT::vc.GetSize() == 1);
		
		// start optimization with root group, 0th context, parent task of zero.  
		if (GlobepsPruning)
		{
			COST * eps_bound = new COST(GlobalEpsBound);
			PTasks.push (new O_GROUP (RootGID, 0, 0, true, eps_bound));
		}
		else
			PTasks.push (new O_GROUP (RootGID, 0, 0));
		
		PTRACE ("initial OPEN:\r\n %s\r\n", PTasks.Dump() );
		
		// main loop of optimization
		// while there are tasks undone, do one
		while (! PTasks.empty ())
		{
			TaskNo ++;
			PTRACE ("Starting task %d", TaskNo);
			
			TASK * NextTask = PTasks.pop ();
			NextTask -> perform ();
			
			if(TraceSSP) 
			{ 
				TRACE_FILE("\r\n====== SSP after task %d: ", TaskNo);
				TRACE_FILE ("%s\r\n", DumpChanged() ); 
			}
			else		 
			{ PTRACE ("%s", DumpChanged() ); }
			
			if(TraceOPEN) 
			{
				TRACE_FILE("\r\n====== OPEN after task %d:\r\n", TaskNo);
				TRACE_FILE("%s\r\n",PTasks.Dump());
			}
			else	
			{	PTRACE2 ("OPEN after task %d:\r\n %s\r\n", TaskNo, PTasks.Dump() ); }
		} // main optimization loop over remaining tasks in task list
		
		PTRACE ("Optimizing completed: %d tasks\r\n", TaskNo);
#ifdef _TABLE_
		OUTPUT("%s\t", GlobalEpsBound.Dump() );
		OUTPUT("%d\t", ClassStat[C_M_EXPR].Count);
		OUTPUT("%d\t", ClassStat[C_M_EXPR].Total);
		OUTPUT("%d\t", TaskNo);
#else
		if(SingleLineBatch)
		{
			CString os;
			os.Format("%d\t%d\t%d\t%d\t%d\t",TaskNo, ClassStat[C_GROUP].Count,
				ClassStat[C_M_EXPR].Count, ClassStat[C_M_EXPR].Total,
				OptStat -> FiredRule);
			OUTPUT("%s",os);
		}
		else
		{
			OUTPUT("TotalTask : %d\r\n", TaskNo);
			OUTPUT("TotalGroup : %d\r\n", ClassStat[C_GROUP].Count);
			OUTPUT("CurrentMExpr : %d\r\n", ClassStat[C_M_EXPR].Count);
			OUTPUT("TotalMExpr : %d\r\n", ClassStat[C_M_EXPR].Total);
			OUTPUT("TotalMExpr in MEMO: %d\r\n", Memo_M_Exprs);
			OUTPUT("%s", OptStat->Dump() );
		}
#endif
    }  // SSP::optimize()
