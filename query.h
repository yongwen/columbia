/* query.h - Definition of query parser
$Revision: 3 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
  
	
*/

#ifndef QUERY_H
#define QUERY_H

#include "logop.h"
#include "item.h"

class EXPR;
class QUERY;

/*
============================================================
QUERY Parser - class QUERY
============================================================
*/

//##ModelId=3B0C086D02FF
class QUERY
{
public:
	
	// free up memory
	//##ModelId=3B0C086D0309
	~QUERY();
	
	// get the EXPR from the query tree representation text file
	//##ModelId=3B0C086D0313
	QUERY(CString filename);
	
	// return the EXPR pointer
	//##ModelId=3B0C086D031D
	EXPR * GetEXPR();
	
	//##ModelId=3B0C086D031E
	CString Dump();
	
	//print the interesting orders
	//##ModelId=3B0C086D0327
	CString Dump_IntOrders();
	
private:
	
	//##ModelId=3B0C086D0332
	EXPR *QueryExpr;	// point to the query read in
	//##ModelId=3B0C086D033B
	CString ExprBuf;	// store the original query string
	
	// get an expression
	//##ModelId=3B0C086D034F
	EXPR *ParseExpr(char *&ExprStr);
	
	// get one element of the expression
	//##ModelId=3B0C086D035A
	char *GetOneElement(char *&Expr);
	
	// get the project keys
	//##ModelId=3B0C086D0364
	void QUERY::ParsePJKeys(char *& p, KEYS_SET & Keys);
	
	// get left and right KEYS_SET
	//##ModelId=3B0C086D036F
	void ParseKeys(char *p, KEYS_SET & Keys1, KEYS_SET & Keys2);
	
	// get one KEYS_SET
	//##ModelId=3B0C086D0383
	void GetOneKeys(char *& p, KEYS_SET & Keys);
	
	// get one KeySET, and add to KEYS_SET
	//##ModelId=3B0C086D0395
	void GetKey(char *& p, KEYS_SET & Keys);
	
    // p points to optional white space followed by a quote mark (").
    // ParseOneParameter returns the string following that ", up to the next ".
	// Upon return, p points to that next quote mark.
	//##ModelId=3B0C086D03A9
	CString ParseOneParameter(char *&p);
	
	// get an AGG_OP_ARRAY
	//##ModelId=3B0C086D03B3
	void QUERY::ParseAggOps(char *&p, AGG_OP_ARRAY & AggOps);
	
	// get the group by keys
	//##ModelId=3B0C086D03BE
	void QUERY::ParseGby(char *&p, KEYS_SET & Keys);
	
	//	get one AGG_OP
	//##ModelId=3B0C086D03C9
	AGG_OP * QUERY::GetOneAggOp(char *& p);
};

/*
============================================================
EXPRESSIONS - class EXPR
============================================================
*/

//##ModelId=3B0C086E0039
class EXPR // An EXPR corresponds to a detailed solution to 
// the original query or a subquery.
// An EXPR is modeled as an operator with arguments (class OP), 
// plus input expressions (class EXPR).

// EXPRs are used to calculate the initial query and the final plan, 
// and are also used in rules.

{
private:
	//##ModelId=3B0C086E004E
	OP * Op;	//Operator
	//##ModelId=3B0C086E0057
	int 		arity;	//Number of input expressions.
	//##ModelId=3B0C086E0078
	EXPR **		Inputs;	//Input expressions
public:
	//##ModelId=3B0C086E007F
	EXPR(OP * Op, EXPR * First = NULL, EXPR * Second = NULL, 
		EXPR * Third = NULL, EXPR * Fourth = NULL);
	
	//##ModelId=3B0C086E00A7
	EXPR(OP * Op, EXPR ** inputs);	
	
	//##ModelId=3B0C086E00B1
	EXPR( EXPR& Expr);
	
	//##ModelId=3B0C086E00BB
	~EXPR();
	
	//##ModelId=3B0C086E00BC
	inline OP * GetOp() {return Op;};
	//##ModelId=3B0C086E00C5
	inline int GetArity() {return arity;};
	//##ModelId=3B0C086E00CF
	inline EXPR *  GetInput(int i)  { return Inputs[i];}; 
	
	//##ModelId=3B0C086E00DA
	CString Dump();
	
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
	
public:									
	//##ModelId=3B0C086E00EF
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0C086E00F8
	inline void * operator new(size_t my_size)	
	{ return  memory_manager -> allocate(&_anchor, (int) my_size, 100); }
	
	//##ModelId=3B0C086E0103
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
	
};  // class  EXPR

#endif	//QUERY_H
