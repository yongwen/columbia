/*
OP.H - Base Classes for Operators
$Revision: 3 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#ifndef OP_H
#define OP_H

#include "stdafx.h"

class OP;	//All operators - Abstract

//The next three classes are Abstract - cannot be instantiated
class LOG_OP;	//Logical Operators on Collections
class PHYS_OP;	//Physical Operators on Collections
class ITEM_OP;	//Item Operators on objects, used for predicates

class LEAF_OP;	//Leaf operators - place holder for a group, in a pattern.
//Patterns are used in rules.

/*
============================================================
OPERATORS AND ARGUMENTS - class OP
============================================================
*/
//##ModelId=3B0C087200CB
class OP
//Abstract Class.  Operator and its arguments.  
//Arguments could be attributes to project on, etc.
{
public:
	
#ifdef _DEBUG
	//##ModelId=3B0C087200D5
    CString 	name;	//Name of this operator
#endif
	
	//##ModelId=3B0C087200DF
	OP() {};
	
	// add assert to the following virtual functions, make sure the subclasses define them
	//##ModelId=3B0C087200E9
	virtual OP* Clone() = 0;
	
	//##ModelId=3B0C087200F3
	virtual ~OP() {};
	
	//##ModelId=3B0C087200F5
	virtual CString Dump() = 0;
	//##ModelId=3B0C087200FE
	virtual LOG_PROP *FindLogProp ( LOG_PROP ** input) = 0; 
	
    // For example, the operator SELECT has arity 2 (one bulk input, one predicate) and 
	//its name is SELECT.  These are not stored as member data since static does not
	//inherit and we don't want one in every object.
	//##ModelId=3B0C08720107
	virtual CString GetName() = 0; 
	//##ModelId=3B0C08720111
	virtual int GetNameId() = 0; 
	//##ModelId=3B0C08720113
	virtual int GetArity() = 0; 
	
	//##ModelId=3B0C0872011C
    virtual bool operator==( OP * other) { return (GetNameId()==other->GetNameId()); };
	
    //Used to compute the hash value of an mexpr.  Used only for logical operators, 
	//so we make it abort everywhere else.
	//##ModelId=3B0C08720126
    virtual ub4 hash() = 0; 
	
	//##ModelId=3B0C08720130
	virtual bool is_logical() {return false;};
	//##ModelId=3B0C08720139
	virtual bool is_physical() {return false;};
	//##ModelId=3B0C0872013B
	virtual bool is_leaf() {return false;};
	//##ModelId=3B0C08720144
	virtual bool is_item() { return false; }; 
	
	//attr_op, const_int_op, const_set_op, const_str_op are special 
	//case in O_INPUT::perform()
	//##ModelId=3B0C0872014E
	virtual bool is_const() { return false; };
	
	//##ModelId=3B0C08720157
    virtual COST * FindLocalCost (LOG_PROP * LocalLogProp, LOG_PROP ** InputLogProp)=0;
}; //class OP

/*
   ============================================================
   LOGICAL OPERATORS - class LOG_OP
   ============================================================
*/


//##ModelId=3B0C0872017F
class LOG_OP: public OP //Logical Operator Abstract Class

{
	
public:
	//##ModelId=3B0C08720193
	LOG_OP() {};
	//##ModelId=3B0C08720194
	virtual ~LOG_OP() {};
	
	//OpMatch (other) is true if this and other are the same operator,
	//independent of arguments.
	//OpMatch is used in preconditions for applying rules.
	//This should be moved to the OP class if we ever apply rules to
	//other than logical operators.
    //If someone writes a rule which uses member data, it could be made virtual
	//##ModelId=3B0C0872019E
	inline bool 	OpMatch  (LOG_OP * other)
	{	return ( GetNameId() == other->GetNameId());  };
	
	//##ModelId=3B0C087201A8
	inline bool	is_logical() { return true ; };
	
	//##ModelId=3B0C087201A9
	inline ub4 GetInitval() { 	return( lookup2(GetNameId(),0) ); } ;
	//Get the initial value for hashing, which depends
	//only on the name of the operator.
	
	// add assert to the following functions, 
	// make sure these methods of LOG_OP never called(log_op does not get cost)
	//##ModelId=3B0C087201B2
	COST * FindLocalCost (LOG_PROP * LocalLogProp, LOG_PROP ** InputLogProp)
	{ assert(false); return NULL; };
	
};  //class LOG_OP


/*
    ============================================================
    PHYSICAL OPERATOR - class PHYS_OP
    ============================================================
*/

//##ModelId=3B0C087201F8
class PHYS_OP: public OP	//Physical Operator

{
	
public:
	
	//##ModelId=3B0C0872020D
	PHYS_OP() {};
	//##ModelId=3B0C0872020E
	virtual ~PHYS_OP() {};
	
	//FindPhysProp() establishes the physical properties of an 
	//algorithm's output.	
	// right now, only implemented by operators with 0 arity. no input_phys_props 
	//##ModelId=3B0C08720217
	virtual PHYS_PROP * FindPhysProp ( PHYS_PROP ** input_phys_props = NULL)
	{	
		assert(false); 
		return NULL;
	}; 
	
	// FindLocalCost() finds the local cost of the operator,
    // including output but not input costs.  Thus we compute output costs
    // only once, and get input costs from (as part of) the input operators' cost.
	//##ModelId=3B0C08720221
	virtual COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp)=0; //uses primarily cardinalities
	
	   /*  Some algorithms and implementation rules require that
       their inputs be optimized for multiple physical property
       combinations, e.g., a merge-join with multiple equality clauses
       "R.a == S.a && R.b == S.b" could benefit from sort order on
       "a", "b", "a, b", and "b, a".  For now we optimize for only
	   one ordering, but later we may need:
    	virtual int opt_combs() */
	
	//If we require the physical property Prop of this operator, what
	//property from input number InputNo will guarantee it?
	//A false return value for possible means there is no value which will work.
	//If possible is true, a NULL return says any property is OK.
	//Should never be called for arity 0 operators
	//##ModelId=3B0C0872022B
	virtual PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible)=0;
	
	//##ModelId=3B0C0872023E
	inline bool	is_physical() { return true ; };
	// add assert to the following functions, 
	// make sure these methods of PHYS_OP never called(log_prop is not passed by phys_op)
	//##ModelId=3B0C0872023F
	LOG_PROP *FindLogProp ( LOG_PROP ** input) {assert(false); return NULL;};
	//##ModelId=3B0C08720249
	inline int GetNameId() {assert(false); return 0;};
	//##ModelId=3B0C08720252
	ub4 hash(){ assert(false); return 0; } ;
};  //class PHYS_OP


/*
    ============================================================
	ITEM OPERATORS - ACT ON INDIVIDUAL OBJECTS - USED IN PREDICATES - class ITEM_OP
    ============================================================
*/

//##ModelId=3B0C0872028E
class ITEM_OP: public OP   //Item Operator - both logical and physical
//Can we do multiple inheritance?  That would be ideal.
{
public:
	//##ModelId=3B0C08720299
	ITEM_OP() {};
	//##ModelId=3B0C087202A2
	~ITEM_OP(){};
	
	//For now we assume no expensive predicates
	//##ModelId=3B0C087202A3
    COST * FindLocalCost (LOG_PROP * LocalLogProp, LOG_PROP ** InputLogProp)
	{return (new COST(0));};
	
	//##ModelId=3B0C087202AE
	LOG_PROP * FindLogProp (LOG_PROP ** input)
	{
		KEYS_SET empty_arr;
		return( new LOG_ITEM_PROP (-1,-1,-1, 0, empty_arr) );
	} 
	
	//##ModelId=3B0C087202B7
	inline bool is_item() { return true; }; 
	
	// add assert to the following functions, 
	// make sure these methods of ITEM_OP never called
	//##ModelId=3B0C087202C0
	inline int GetNameId() {assert(false); return 0;};
	//##ModelId=3B0C087202C1
	ub4 hash(){ assert(false); return 0; } ;
	
};  //class ITEM_OP


	/*
    ============================================================
    LEAF OPERATORS - USED IN RULES - class LEAF_OP
    ============================================================
*/

//##ModelId=3B0C08720374
class LEAF_OP: public OP	
//Used in rules only.  Placeholder for a Group
{
private: 
	
	//##ModelId=3B0C08720393
	GRP_ID		Group;	//Identifies the group bound to this leaf, after binding.
	                    // == -1 until binding
	//##ModelId=3B0C087203A7
	GRP_ID	Index;	    //Used to distinguish this leaf in a rule
	
public:
	
	//##ModelId=3B0C087203BA
	LEAF_OP(GRP_ID index, GRP_ID group = -1)
		: Index(index), Group(group)
	{ 
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LEAF_OP].New(); 
#ifdef _DEBUG
		name = GetName();		// for debug
#endif
	};
	
	//##ModelId=3B0C087203C5
	LEAF_OP( LEAF_OP& Op)
		:Index(Op.Index),Group(Op.Group)
	{ 
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LEAF_OP].New(); 
#ifdef _DEBUG
		name = Op.name;		// for debug
#endif
	};
	
	//##ModelId=3B0C087203CF
	inline OP * Clone() { return new LEAF_OP(*this); };
	
	//##ModelId=3B0C087203D8
	~LEAF_OP()
	{if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LEAF_OP].Delete();};
	
	//##ModelId=3B0C087203D9
	inline int GetArity() {return(0);};
	//##ModelId=3B0C087203E2
	inline CString GetName() {return ("LEAF_OP"); };
	//##ModelId=3B0C08730004
	inline GRP_ID GetGroup() {return (Group);};
	//##ModelId=3B0C08730005
	inline int GetIndex() {return(Index);};
	
	//##ModelId=3B0C0873000E
	inline bool is_leaf() {return true;};
	
	//##ModelId=3B0C08730018
	CString Dump()
	{	
		CString os;
		os.Format("%s%s%d%s%d%s",GetName(),"<",Index,",",Group,">");
		return os;
	};
	
	// add assert to the following functions, 
	// make sure these methods of LEAF_OP never called
	//##ModelId=3B0C08730022
	inline int GetNameId() {assert(false); return 0;};
	//##ModelId=3B0C0873002C
	ub4 hash(){ assert(false); return 0; } ;
	
	//##ModelId=3B0C0873002D
	COST * FindLocalCost (LOG_PROP * LocalLogProp, LOG_PROP ** InputLogProp)
	{ assert(false); return NULL; };
	
	//##ModelId=3B0C08730037
	LOG_PROP *FindLogProp ( LOG_PROP ** input) {assert(false); return NULL;};
	
	
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
	
public:									
	//##ModelId=3B0C0873004B
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0C08730069
	inline void * operator new(size_t my_size)	
	{ return memory_manager -> allocate(&_anchor, (int) my_size, 50 ); }
	
	//##ModelId=3B0C08730073
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
	
};  //class LEAF_OP

/*
    ============================================================
    AGG_OP - USED IN Aggregates - Not an Operator
    ============================================================
*/

// AGG_OP represents an operator which applies an aggregate function to the
// attributes in "attrs".  The function could be SUM COUNT MIN MAX etc.;
// AGG_OP may also involve a more complex expression, e.g., with more than one function.
// It is part of the logical operator AGG_LIST and physical op HGROUP_LIST
//##ModelId=3B0C08730113
class AGG_OP
{
private:
	//##ModelId=3B0C08730127
	CString RangeVar;	//Name assigned to the function, e.g. REVENUE = SUM(xxx)
	//##ModelId=3B0C08730131
	int * Atts;			//Attributes involved in the function
	//##ModelId=3B0C0873013B
	int   AttsSize;		//Number of attributes above
public:
	
	//##ModelId=3B0C0873014F
	AGG_OP(CString range_var, int * atts, int size)
		: RangeVar(range_var), Atts(atts), AttsSize(size)
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_AGG_OP].New(); 	};
	
	//##ModelId=3B0C08730159
	AGG_OP( AGG_OP& Op)
		:RangeVar(Op.RangeVar), AttsSize(Op.AttsSize), Atts(CopyArray(Op.Atts, Op.AttsSize))
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_AGG_LIST].New();	
	};
	
	//##ModelId=3B0C0873015B
	~AGG_OP()
	{	delete [] Atts;
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_AGG_OP].Delete();};
	
	//##ModelId=3B0C08730163
	inline CString GetName() { return ("AGG_OP"); };
	//##ModelId=3B0C0873016D
	inline CString GetRangeVar() { return (RangeVar); };
	//##ModelId=3B0C0873016E
	inline int * GetAtts() { return (Atts); };
	//##ModelId=3B0C08730177
	inline int GetAttsSize() { return (AttsSize); };
	
	//##ModelId=3B0C08730181
	CString Dump()
	{	
		CString os;
		CString temp;
		os.Format("%s%s",GetName()," <");
		for (int i=0; i<AttsSize-1; i++)
		{
			temp.Format("%s%s", GetAttName(Atts[i]), ",");
			os += temp;
		}
		if (AttsSize==0) temp.Format("%s", "Empty set");
		else temp.Format("%s", GetAttName(Atts[AttsSize-1]) );
		os += temp;
		
		temp.Format("%s%s%s", " AS ", RangeVar, " >");
		os += temp;
		return os;
	};
	
	//##ModelId=3B0C08730182
	bool operator== (AGG_OP *other)
	{
		return ( other->GetName() == this->GetName() &&
			EqualArray( other->GetAtts(), Atts, AttsSize) &&
			other->GetRangeVar() == RangeVar);
	};
};

//##ModelId=3B0C0873019F
typedef CArray <AGG_OP *, AGG_OP *> AGG_OP_ARRAY;

#endif //OP_H

