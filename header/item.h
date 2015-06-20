/* 
item.h - operators on items.  Typically part of a predicate.
$Revision: 3 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#ifndef	 ITEM_H
#define ITEM_H

#include "Op.h"

/* ------------------------------------------------------------ */

/*                         Attribute Operators

  ATTR_OP occupies two roles:

1. It is used in predicates as an item operator, transforming a
tuple into one of its attributes.

2. It is used to describe the logical property "schema".  Here it
refers to two different attribute types:
        a) Attributes read from a catalog, e.g. emp.age, where emp
           is the range variable (default = name of the file), and
           age is a typical attribute
        b) the name for an expression, as in
             SUM(PRICE(1-DISC)) AS revenue
           here "revenue" is the range variable and {PRICE, DISC} is
           the array of attributes (of type 2.a) used in the expression.

Goetz used ATTR_OP for 1 and 2a.
In Columbia, ATTR_OP is 1, ATTR is 2.a, and ATTR_EXP is 2.b .
*/

//ATTR_OP represents the value of an attribute, as in emp.age < 40
//##ModelId=3B0C0875027E
class ATTR_OP : public ITEM_OP
{
private:	
	//##ModelId=3B0C08750292
	int AttId;
	
public:
	//##ModelId=3B0C0875029C
	ATTR_OP(int attid) : AttId(attid) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR_OP].New();};
	
	//##ModelId=3B0C087502A6
	ATTR_OP( ATTR_OP& Op ) : AttId(Op.AttId) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR_OP].New();};
	
	//##ModelId=3B0C087502B0
	OP * Clone() {	return new ATTR_OP(*this); };
	
	//##ModelId=3B0C087502B1
	~ATTR_OP() 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR_OP].Delete();};
	
	//##ModelId=3B0C087502BA
	LOG_PROP * FindLogProp ( LOG_PROP ** input);
	
	//##ModelId=3B0C087502C4
    inline int get_value() { return AttId; }
	//##ModelId=3B0C087502C5
	inline int GetArity() {return(0);};
	//##ModelId=3B0C087502CE
    inline CString GetName() {return ("ATTR_OP"); };
	//##ModelId=3B0C087502D8
	inline bool is_const() { return true; };
	//##ModelId=3B0C087502D9
	inline COST * get_cost() { return new COST(0); };
	
	//##ModelId=3B0C087502E2
	CString Dump() { CString os; os.Format("ATTR(%s)",GetAttName(AttId)); return os;}
};

//##ModelId=3B0C08750328
class ATTR_EXP : public ITEM_OP
{
private:
	//##ModelId=3B0C0875033C
	CString RangeVar;
	//##ModelId=3B0C08750346
	int * Atts;
	//##ModelId=3B0C08750350
	int AttsSize;
	//##ModelId=3B0C08750365
	ATTR *AttNew;
	
public:
	//##ModelId=3B0C08750379
	ATTR_EXP(CString range_var, int * atts, int size);
	
	//##ModelId=3B0C0875038D
	ATTR_EXP( ATTR_EXP& Op )
		: RangeVar(Op.RangeVar), Atts( CopyArray(Op.Atts,Op.AttsSize) ), 
		AttsSize(Op.AttsSize) 
	{	AttNew = new ATTR(*Op.AttNew);
	   if(TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR_EXP].New();
	};
	
	//##ModelId=3B0C08750397
	OP * Clone() {	return new ATTR_EXP(*this); };
	
	//##ModelId=3B0C08750398
	~ATTR_EXP() 
	{	
		delete [] Atts;
		delete AttNew;
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR_EXP].Delete();
	};
	
	//inline int Get_AttId() { return (AttNew->AttId); };
	//##ModelId=3B0C08750399
	inline int GetArity() {return(0);};
	//##ModelId=3B0C087503A1
	inline int * GetAtts() { return (Atts); };
	//##ModelId=3B0C087503AB
	inline int GetAttsSize() { return (AttsSize); };
	//##ModelId=3B0C087503B5
	inline CString GetRangeVar() { return (RangeVar); };
	//##ModelId=3B0C087503B6
	inline ATTR * GetAttNew() { return (AttNew); };
	//##ModelId=3B0C087503BF
    inline CString GetName() {return ("ATTR_EXP"); };
	
	//##ModelId=3B0C087503C9
	CString Dump();
};

// constant op
//##ModelId=3B0C087503DD
class CONST_OP : public ITEM_OP
{
public:
	//##ModelId=3B0C08760009
	inline bool is_const() { return true;} ;
	//##ModelId=3B0C0876000A
	COST * get_cost() { return new COST(0); };
};

//Integer valued constant
//##ModelId=3B0C08760045
class CONST_INT_OP : public CONST_OP
{
private :
	//##ModelId=3B0C08760059
    int         value;
	
public :
    // constructors for constant int
	//##ModelId=3B0C08760063
    CONST_INT_OP (int value) : value(value) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_INT_OP].New();};
	
	//##ModelId=3B0C0876006D
	CONST_INT_OP( CONST_INT_OP& Op ) : value(Op.value) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_INT_OP].New();};
	
	//##ModelId=3B0C08760077
	OP * Clone() { return new CONST_INT_OP(*this); };
	
	//##ModelId=3B0C08760078
	~CONST_INT_OP() 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_INT_OP].Delete();};
	
	//##ModelId=3B0C08760081
    inline int get_value() { return value; }
	//##ModelId=3B0C08760082
	inline int GetArity() {return(0);};
	//##ModelId=3B0C0876008B
    inline CString GetName() {return ("INT_OP"); };
	//##ModelId=3B0C08760095
	inline bool is_const() { return true; };
	//inline COST * get_cost() { return new COST(0); };
	
	//##ModelId=3B0C0876009F
	CString Dump() { CString os; os.Format("INT(%d)",value); return os;}
}; // CONST_INT_OP


//String valued constant
//##ModelId=3B0C087600EF
class CONST_STR_OP : public CONST_OP
{
private :
	//##ModelId=3B0C08760104
    CString     value;
	
public :
    // constructor for constant
	//##ModelId=3B0C0876010D
    CONST_STR_OP (CString value) : value(value) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_STR_OP].New();};
	
	//##ModelId=3B0C08760118
	CONST_STR_OP( CONST_STR_OP& Op ) : value(Op.value)
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_STR_OP].New();};
	
	//##ModelId=3B0C08760121
	OP * Clone() { return new CONST_STR_OP(*this); };
	
	//##ModelId=3B0C08760122
	~CONST_STR_OP() 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_STR_OP].Delete();};
	
	//##ModelId=3B0C0876012B
	LOG_PROP * FindLogProp ( LOG_PROP ** input);
	
	//##ModelId=3B0C0876012D
    inline CString get_value() { return value; }
	//##ModelId=3B0C08760135
	inline int GetArity() {return(0);};
	//##ModelId=3B0C0876013F
    inline CString GetName() {return ("STR_OP"); };
	//##ModelId=3B0C08760140
	inline bool is_const() { return true; };
	//inline COST * get_cost() { return new COST(0); };
	
	//##ModelId=3B0C08760149
	CString Dump() { CString os; os.Format("STR(%s)",value); return os;}
	
}; // CONST_STR_OP

//String valued constant SET
//##ModelId=3B0C08760199
class CONST_SET_OP : public CONST_OP
{
private :
	//##ModelId=3B0C087601AD
    CString     value;
	
public :
    // constructor for constant
	//##ModelId=3B0C087601B7
    CONST_SET_OP (CString value) : value(value) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_SET_OP].New();};
	
	//##ModelId=3B0C087601C1
	CONST_SET_OP (CONST_SET_OP& Op ) : value(Op.value)
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_SET_OP].New();};
	
	//##ModelId=3B0C087601C3
	OP * Clone() { return new CONST_SET_OP(*this); };
	
	//##ModelId=3B0C087601CB
	~CONST_SET_OP() 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONST_SET_OP].Delete();};
	
	//##ModelId=3B0C087601CC
    inline CString get_value() { return value; }
	//##ModelId=3B0C087601D5
	inline int GetArity() {return(0);};
	//##ModelId=3B0C087601DF
    inline CString GetName() {return ("SET_OP"); };
	//##ModelId=3B0C087601E0
	inline bool is_const() { return true; };
	//inline COST * get_cost() { return new COST(0); };
	
	//##ModelId=3B0C087601E9
	CString Dump() { CString os; os.Format("SET(%s)",value); return os;};
	
}; // CONST_SET_OP

/* ------------------------------------------------------------ */

//Boolean Operator
//##ModelId=3B0C087601FD
class BOOLE_OP : public ITEM_OP
{
	
private:
    
public:
	
}; // BOOLE_OP

/* ------------------------------------------------------------ */

//##ModelId=3B0C0876021B
typedef enum COMP_OP_CODE
{
	OP_AND , OP_OR , OP_NOT,
	OP_EQ , OP_LT , OP_LE ,	OP_GT , OP_GE , OP_NE ,
	OP_LIKE , OP_IN 
} COMP_OP_CODE;

//Comparison Operators
//##ModelId=3B0C08760294
class COMP_OP : public BOOLE_OP
{
	
private :
	//##ModelId=3B0C087602A9
    COMP_OP_CODE	op_code;
	
public :
	
	//##ModelId=3B0C087602BC
    COMP_OP (COMP_OP_CODE op_code) : op_code(op_code) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_COMP_OP].New();};
	
	//##ModelId=3B0C087602C6
	COMP_OP ( COMP_OP & Op ) : op_code(Op.op_code) 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_COMP_OP].New();};
	
	//##ModelId=3B0C087602D0
	OP * Clone() { return new COMP_OP (*this); };
	
	//##ModelId=3B0C087602D1
	~COMP_OP () 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_COMP_OP].Delete();};
	
	//##ModelId=3B0C087602D2
	LOG_PROP * FindLogProp ( LOG_PROP ** input);
	
	//##ModelId=3B0C087602DB
	inline int GetArity() 
	{	if(op_code==OP_NOT) return(1);
	    else return (2);
	};
	
	//##ModelId=3B0C087602E4
    inline CString GetName() {return ("COMP_OP"); };
	
	//##ModelId=3B0C087602EE
	CString Dump() 
	{	CString os; 
	switch(op_code)
	{
	case OP_AND:  os.Format("%s","OP_AND"); break;
	case OP_OR:  os.Format("%s","OP_OR"); break;
	case OP_NOT:  os.Format("%s","OP_NOT"); break;
	case OP_EQ:  os.Format("%s","OP_EQ"); break;
	case OP_LT:  os.Format("%s","OP_LT"); break;
	case OP_LE:  os.Format("%s","OP_LE"); break;
	case OP_GT:  os.Format("%s","OP_GT"); break;
	case OP_GE:  os.Format("%s","OP_GE"); break;
	case OP_NE:  os.Format("%s","OP_NE"); break;
	case OP_LIKE:  os.Format("%s","OP_LIKE"); break;
	case OP_IN:  os.Format("%s","OP_IN"); break;
		
	default:	assert(false);
	}
	return os;
	}
	
}; // COMP_OP

#endif	 ITEM_H
