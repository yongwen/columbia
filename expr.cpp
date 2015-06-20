/*	
expr.cpp -  implementation of class EXPR
$Revision: 3 $
Implements class EXPR as defined in query.h

Columbia Optimizer Framework
A Joint Research Project of Portland State University 
and the Oregon Graduate Institute
Directed by Leonard Shapiro and David Maier
Supported by NSF Grants IRI-9610013 and IRI-9619977

  
*/
#include "stdafx.h"
#include "query.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

//##ModelId=3B0C086E007F
EXPR::EXPR(OP * LogOp, EXPR * First, EXPR * Second, 
		   EXPR * Third, EXPR * Fourth)
		   :Op(LogOp), arity(0)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_EXPR].New();
	
	if(First)		arity++;
	if(Second) 		arity++;
	if(Third) 		arity++;
	if(Fourth) 		arity++;
	
	if(arity)
	{
		Inputs = new EXPR* [arity];
		if(First)	Inputs[0] = First;
		if(Second) 	Inputs[1] = Second;
		if(Third) 	Inputs[2] = Third;
		if(Fourth) 	Inputs[3] = Fourth;
	}
};

//##ModelId=3B0C086E00A7
EXPR::EXPR(OP * Op, EXPR ** inputs)
:Op(Op), Inputs(inputs), arity( Op->GetArity())

{ if (TraceOn&& !ForGlobalEpsPruning) ClassStat[C_EXPR].New(); };

//##ModelId=3B0C086E00B1
EXPR::EXPR( EXPR& Expr)
:Op(Expr.GetOp()->Clone()), arity(Expr.GetArity())
{
	if (TraceOn&& !ForGlobalEpsPruning) ClassStat[C_EXPR].New(); 
	if(arity)
	{
		Inputs = new EXPR* [arity];
		for(int i=0; i<arity; i++)	Inputs[i] = new EXPR( *(Expr.GetInput(i)) );
	}
}	

//##ModelId=3B0C086E00BB
EXPR::~EXPR()
{
	if (TraceOn&& !ForGlobalEpsPruning) ClassStat[C_EXPR].Delete();
	
	delete Op; Op = NULL;
	if(arity)
	{
		for(int i=0;i<arity;i++) delete Inputs[i];
		delete [] Inputs;
	}
	
};
// dump function
//##ModelId=3B0C086E00DA
CString EXPR::Dump()
{
	CString os;
	os += "(" ;
	os += (*Op).Dump();	
	int i;
	for(i=0; i<arity; i++)
	{
		os += "," ;
		os += Inputs[i]->Dump();
	}
	os += ")";
	return os;
}
