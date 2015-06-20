/*	
group.cpp -  implementation of class GROUP
	$Revision: 12 $
	Implements class GROUP as defined in ssp.h

	Columbia Optimizer Framework

	A Joint Research Project of Portland State University 
	   and the Oregon Graduate Institute
	Directed by Leonard Shapiro and David Maier
	Supported by NSF Grants IRI-9610013 and IRI-9619977

	
*/
#include "stdafx.h"
#include "ssp.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

//##ModelId=3B0C086603C7
GROUP::GROUP(M_EXPR * MExpr)
:GroupID(MExpr->GetGrpID()), 
FirstLogMExpr(MExpr), LastLogMExpr(MExpr), 
FirstPhysMExpr(NULL), LastPhysMExpr(NULL)
{
	if (!ForGlobalEpsPruning) ClassStat[C_GROUP].New();
	
	init_state();
	
	// find the log prop
	int arity = MExpr->GetArity();
	LOG_PROP ** InputProp=NULL;
	if(arity==0)
	{	LogProp = (MExpr->GetOp())->FindLogProp(InputProp);
	}
	else
	{
		InputProp = new LOG_PROP* [arity];
		GROUP *Group;
		for(int i=0; i<arity; i++)
		{	
			Group = Ssp->GetGroup(MExpr->GetInput(i));
			InputProp[i] = Group->LogProp;
		}
		
		LogProp = ((MExpr->GetOp())->FindLogProp(InputProp));
		
		delete [] InputProp;
	}
	
	/* Calculate the LowerBd, which is:
	TouchCopyCost:        
	touchcopy() * |G| +     //From top join
	touchcopy() * sum(cucard(Ai) i = 2, ..., n-1) +  // from other, nontop, joins
	+ FetchCost:
	fetchbound() * sum(cucard(Ai) i = 1, ..., n) // from leaf fetches
	*/
	double cost = 0;
	if( MExpr->GetOp()->is_logical() )
	{
		if( MExpr->GetOp()->GetName().Compare("GET") == 0 )
			cost = 0; //Since GET operator does not have a CopyOut cost
		else
			cost = TouchCopyCost( (LOG_COLL_PROP *)LogProp ) ;
		
		// Add in fetching cost if CuCard Pruning 
		if(CuCardPruning)
			cost += FetchingCost((LOG_COLL_PROP *)LogProp);
	}
	
	LowerBd = new COST(cost);
	
	/* if the operator is EQJOIN with m tables, estimate group size
	   is 2^m*2.5. else it is zero */
	if ( MExpr->GetOp()->GetName().Compare("EQJOIN") == 0)
	{
		int NumTables = this->EstimateNumTables(MExpr);
		EstiGrpSize = pow(2, NumTables)*2.5;
	}
	else
		EstiGrpSize = 0;
	
	// the initial value is -1, meaning no winner has been found
	count = -1;
	if(COVETrace)	//New Group
	{
		char buffer[10]; //Holds input group ID
		CString temp;
		if(arity){
			for(int i = 0; i < arity; i++){
				(void)itoa(MExpr -> GetInput(i), buffer, 10);
				temp = temp + " "+ buffer;
			}
		}
		temp += " ";
		
		CString os;
		os.Format("addGroup { %d %d \" %s  \"%s} %s",
			
			GroupID, int(MExpr), MExpr -> GetOp() -> Dump(),
			temp, LogProp -> DumpCOVE()
			);
		
		OutputCOVE.Write(os, os.GetLength());
	}
}

// free up memory
//##ModelId=3B0C086603C9
GROUP::~GROUP()
{
	if (!ForGlobalEpsPruning) ClassStat[C_GROUP].Delete();
	
	delete LogProp;
	delete LowerBd;
	
	M_EXPR * mexpr =FirstLogMExpr;
	M_EXPR * next = mexpr;
	while( next!=NULL )
	{	next = mexpr->GetNextMExpr();
		delete mexpr; 
		mexpr = next;
	}
	
	mexpr =FirstPhysMExpr;
	next = mexpr;
	while( next!=NULL )
	{	next = mexpr->GetNextMExpr();
		delete mexpr; 
		mexpr = next;
	}
	
#ifndef IRPROP
	for(int i=0;i<Winners.GetSize();i++)
		delete Winners[i];
#endif
	
}

// estimate the number of tables in EQJOIN
//##ModelId=3B0C086701FC
int  GROUP::EstimateNumTables(M_EXPR * MExpr)
{
	int table_num;
	int total=0;
	GROUP *Group;
	int arity = MExpr->GetArity();
	// if the input is EQJOIN, continue to count all the input
	for(int i=0; i<arity; i++)
	{	
		Group = Ssp->GetGroup(MExpr->GetInput(i));
		if (Group->GetFirstLogMExpr()->GetOp()->GetName().Compare("EQJOIN") == 0)
		{
			table_num = Group->EstimateNumTables(Group->GetFirstLogMExpr());
		}
		else table_num=1;
		total += table_num;
	}
	return total;
}

//##ModelId=3B0C08670089
void GROUP::NewMExpr(M_EXPR *MExpr) 
{
	// link to last mexpr
	if(MExpr->GetOp()->is_logical())
	{
		LastLogMExpr->SetNextMExpr(MExpr);	
		LastLogMExpr = MExpr;
	}
	else
	{
		if(LastPhysMExpr)
			LastPhysMExpr->SetNextMExpr(MExpr);	
		else
			FirstPhysMExpr = MExpr;
		
		LastPhysMExpr = MExpr;
		
	}
	
	// if there is a winner found before, count the number of plans
	if (count != -1) count++;
	if(COVETrace)	//New MExpr
	{
		char buffer[10]; //Holds input group ID
		CString temp;
		int arity = MExpr -> GetArity();
		if(arity){
			for(int i = 0; i < arity; i++){
				(void)itoa(MExpr -> GetInput(i), buffer, 10);
				temp = temp + " "+ buffer;
			}
		}
		temp += " ";
		
		CString os;
		os.Format("addExp { %d %d \" %s  \"%s} %s",
			
			GroupID, int(MExpr), MExpr -> GetOp() -> Dump(),
			temp, LogProp -> DumpCOVE()
			);
		
		OutputCOVE.Write(os, os.GetLength());
	}
}  

//##ModelId=3B0C08670044
void GROUP::set_optimized (bool is_optimized) 
{ 
	SET_TRACE Trace(true);
	
	if(is_optimized)  
	{
		PTRACE("group %d is completed", GroupID); 
	}
	
	State.optimized = is_optimized; 
	
}

//##ModelId=3B0C086700BC
void GROUP::ShrinkSubGroup()
{
	for(M_EXPR * MExpr = FirstLogMExpr; 
		MExpr!=NULL; 
		MExpr=MExpr->GetNextMExpr())
	{
		for(int i=0; i< MExpr->GetArity(); i++)
			Ssp->ShrinkGroup(MExpr->GetInput(i));
	}
}

// Delete a physical MExpr from a group, save memory
//##ModelId=3B0C086700BD
void GROUP::DeletePhysMExpr (M_EXPR * PhysMExpr)
{
	M_EXPR * MExpr = FirstPhysMExpr;
	M_EXPR *next;
	if (MExpr == PhysMExpr)
	{
		FirstPhysMExpr = MExpr->GetNextMExpr();
		// if the MExpr to be deleted is the only one in the group, 
		// set both First and Last Physical MExpr to be NULL
		if (FirstPhysMExpr == NULL)
			LastPhysMExpr = NULL;
		delete MExpr;
		MExpr = NULL;
	}
	else
	{
		// search for the MExpr to be deleted in the link list
		while (MExpr != NULL)
		{
			next = MExpr->GetNextMExpr();
			// if found, manipulate the pointers and delete the necessary one
			if (next == PhysMExpr)
			{
				MExpr->SetNextMExpr(next->GetNextMExpr());
				if (next->GetNextMExpr() == NULL)
					LastPhysMExpr = MExpr;
				delete next;
				next = NULL;
				break;
			}
			MExpr = MExpr->GetNextMExpr();
		}
	}
} 

//##ModelId=3B0C086603D1
CString GROUP::Dump()
{
	CString os;
	int Size = 0;
	CString temp;
	M_EXPR* MExpr;
	
	os.Format("%s%d%s\r\n","----- Group ", GroupID , " : -----");
	
	for(MExpr=FirstLogMExpr; MExpr!=NULL; MExpr = MExpr->GetNextMExpr() )	
	{
		os += MExpr->Dump();
		os += " ; ";
		Size++;
	}
	for(MExpr=FirstPhysMExpr; MExpr!=NULL; MExpr = MExpr->GetNextMExpr() )	
	{
		os += MExpr->Dump();
		os += " ; ";
		Size++;
	}
	temp.Format("\r\n%s%d%s", "----- has " , Size , " MExprs -----\r\n");
	os += temp;
	
	//Print Winner's circle
	os += "Winners:\r\n" ;
	
#ifdef IRPROP
	os += M_WINNER::mc[GroupID]->Dump();
	os += "\r\n";
#else
	
	Size = Winners.GetSize();
	PHYS_PROP * PhysProp;
	if(!Size) os += "\tNo Winners\r\n";
	for(int i = 0; i < Size; i++) 
	{
		PhysProp = Winners[i] -> GetPhysProp();
		os += "\t" ;
		os += PhysProp -> Dump();
		os += ", " ;
		os += ( Winners[i]->GetMPlan() ? Winners[i]->GetMPlan()-> Dump() : "NULL Plan");
		os += ", " ;
		os += (Winners[i]->GetCost() ? Winners[i]->GetCost()->Dump() : "NULL Cost");
		os += ", " ;
		os += (Winners[i]->GetDone() ? "Done" : "Not done");
		os += "\r\n";
	}
#endif
	
	temp.Format("LowerBound: %s\r\n", LowerBd->Dump() );
	os += temp;
	
	os += "log_prop: ";
	os += (*LogProp).Dump();
	
	return os;
}

//##ModelId=3B0C086603D2
void GROUP::FastDump()
{
	int Size = 0;
	M_EXPR* MExpr;
	
	TRACE_FILE("----- Group %d : -----\r\n", GroupID);
	
	
	for(MExpr=FirstLogMExpr; MExpr!=NULL; MExpr = MExpr->GetNextMExpr() )
	{
		TRACE_FILE("%s ; ", MExpr->Dump() );
		Size++;
	}
	for(MExpr=FirstPhysMExpr; MExpr!=NULL; MExpr = MExpr->GetNextMExpr() )	
	{
		TRACE_FILE("%s ; ", MExpr->Dump() );
		Size++;
	}
	
	TRACE_FILE("\r\n----- has %d MExprs -----\r\n", Size );
	
	//Print Winner's circle
	TRACE_FILE("%s", "Winners:\r\n" );
	
#ifdef IRPROP
	TRACE_FILE("\t%s\n", M_WINNER::mc[GroupID]->Dump());
#else
	Size = Winners.GetSize();
	PHYS_PROP * PhysProp;
	if(!Size) TRACE_FILE("%s", "\tNo Winners\r\n" );
	for(int i = 0; i < Size; i++) 
	{
		PhysProp = Winners[i] -> GetPhysProp();
		TRACE_FILE("\t%s", PhysProp -> Dump() );
		TRACE_FILE(", %s", ( Winners[i]->GetMPlan() ? Winners[i]->GetMPlan()-> Dump() : "NULL") );
		TRACE_FILE(", %s", (Winners[i]->GetCost() ? Winners[i]->GetCost()->Dump() : "-1") );
		TRACE_FILE(", %s\r\n", (Winners[i]->GetDone() ? "Done" : "Not done") );
	}
#endif
	
	TRACE_FILE("LowerBound: %s\r\n", LowerBd->Dump() );
	
	TRACE_FILE("log_prop: %s", (*LogProp).Dump() );
}
