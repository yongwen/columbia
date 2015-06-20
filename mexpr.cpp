/*	
  mexpr.cpp -  implementation of class M_EXPR as defined in ssp.h
	$Revision: 8 $
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

//##ModelId=3B0C086502F3
M_EXPR::M_EXPR( EXPR * Expr, GRP_ID grpid)
:Op(Expr->GetOp()->Clone()), NextMExpr(NULL),
GrpID( (grpid==NEW_GRPID) ? Ssp->GetNewGrpID() : grpid),
HashPtr(NULL), RuleMask(0)
{
	GRP_ID GID;
	EXPR * input;
	counter = 0;
	
	if (!ForGlobalEpsPruning) ClassStat[C_M_EXPR].New(); 
	
	// copy in the sub-expression
	int arity = GetArity();
	if(arity) 
	{
		Inputs = new GRP_ID [arity];
		for(int i=0; i<arity; i++)
		{
			input = Expr->GetInput(i);
			
			if( input -> GetOp() -> is_leaf() )	// deal with LEAF_OP, sharing the existing group
				GID = ((LEAF_OP *)input->GetOp() ) -> GetGroup();
			else
			{		
				// create a new sub group
				if (Op ->GetName() == "DUMMY")
					GID = NEW_GRPID_NOWIN;	//DUMMY subgroups have only trivial winners
				else
					GID = NEW_GRPID;
				M_EXPR * MExpr = Ssp->CopyIn( input ,  GID);
			}
			
			Inputs[i] = GID ;
		}  
	}  // if(arity)
}

// copy constructor
//##ModelId=3B0C08650307
M_EXPR::M_EXPR(M_EXPR & other)
:GrpID(other.GrpID), HashPtr(other.HashPtr),
NextMExpr(other.NextMExpr),
Op(other.Op->Clone()), RuleMask(other.RuleMask)
{
	if (!ForGlobalEpsPruning) ClassStat[C_M_EXPR].New(); 
	
	//Inputs are the only member data left to copy.
	int arity = Op->GetArity() ;
	if (arity)
	{
		Inputs = new GRP_ID [ arity ];
		while(--arity >= 0 )
			Inputs[arity] = other.GetInput(arity);
	}
	
}

//##ModelId=3B0C086502E9
M_EXPR::~M_EXPR()
{
	if (!ForGlobalEpsPruning) ClassStat[C_M_EXPR].Delete(); 
	if(GetArity()) 
	{
		delete [] Inputs;
	}
	
	delete Op ; Op = NULL;
};

//##ModelId=3B0C086503E4
ub4 M_EXPR::hash()
{
    ub4 hashval = Op->hash();
	
	//to check the equality of the inputs
    for(int input_no = Op->GetArity();  -- input_no >= 0;) 
        hashval = lookup2( GetInput(input_no), hashval);
	
	return (hashval % (HtblSize -1) ) ;
}

//##ModelId=3B0C08660006
CString M_EXPR::Dump()
{
	CString os;
	
	os = (*Op).Dump();
	
	int Size = GetArity();
	for(int i=0;i<Size;i++)
	{
		CString temp;
		temp.Format("%s%d", " , ", Inputs[i]);
		os += temp;
	}
	
	return os;
}

