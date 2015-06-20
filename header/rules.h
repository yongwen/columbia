/* $Id$
RULES.H - Base classes and actual rules

  Columbia Optimizer Framework
  
	A Joint Research Project of Portland State University 
	   and the Oregon Graduate Institute
	   Directed by Leonard Shapiro and David Maier
	   Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#ifndef RULES_H
#define RULES_H

#include "ssp.h" 

class BINDERY;
class RULE;
class RULE_SET;
class GET_TO_FILE_SCAN;
class SELECT_TO_FILTER;
class P_TO_PP;
class EQ_TO_LOOPS_INDEX;
class EQ_TO_MERGE;
class EQ_TO_HASH;
class EQ_TO_LOOPS;
class SORT_RULE;
class EQJOIN_COMMUTE;
class EQJOIN_LTOR;
class EQJOIN_RTOL;
class EXCHANGE;
class RM_TO_HASH_DUPLICATES;
class AL_TO_HGL;
class FO_TO_PFO;
class AGG_THRU_EQJOIN;
class SELECT_TO_INDEXED_FILTER;
class DUMMY_TO_PDUMMY;

//==========
/*
This enum list is used for the rule bit vector.  It must be consistent with NUMOFRULES 
and rule_set and with the rule set file read from disk. If there are more than 32 rules, 
put only logical to logical transformations in the rule vector. 
*/
//##ModelId=3B0C086802C6
typedef enum RULELABELS {
	    R_GET_TO_FILE_SCAN,
		R_SELECT_TO_FILTER,
		R_P_TO_PP,
		R_EQ_TO_LOOPS_INDEX,
		R_EQ_TO_MERGE,
		R_EQ_TO_LOOPS,
		R_SORT_RULE,
		R_EQJOIN_COMMUTE,	
		R_EQJOIN_LTOR,		
		R_EQJOIN_RTOL,		
		R_EXCHANGE,		
		R_RM_TO_HASH_DUPLICATES,
		R_AL_TO_HGL,
		R_FO_TO_PFO,
		//	R_OBY_TO_QSORT,
		R_AGG_THRU_EQJOIN,
		R_EQ_TO_BIT,
		R_SELECT_TO_INDEXED_FILTER,
		R_PROJECT_THRU_SELECT,
		R_EQ_TO_HASH,
		R_DUMMY_TO_PDUMMY,
} RULELABELS;

extern int RuleVector[];	// initialize in Rules.cpp

//##ModelId=3B0C086802F8
class RULE_SET
{
private: 
	//##ModelId=3B0C0868030D
	RULE ** rule_set;
	
public:
	//##ModelId=3B0C08680320
	int RuleCount;	// size of rule_set
	
	//##ModelId=3B0C0868032A
	RULE_SET(CString filename);
	//##ModelId=3B0C0868033E
	~RULE_SET();
	
	//##ModelId=3B0C08680348
	CString Dump();
	//##ModelId=3B0C08680352
	CString DumpStats();
	
	// return the RULE in the order Set
	//##ModelId=3B0C0868035C
	inline RULE* operator[](int n)  
	{ 
		if(RuleVector[n]) return rule_set[n];
		else return NULL;
	} 
	
}; // RULE_SET

/*
    BINDERY
    ========
   At the heart of every rule- (transform-) based optimizer is the concept of
   a rule.  A rule includes two patterns, called the Original and Substitute
   patterns.  For example, the Left to Right (LTOR) Associative Rule includes the
   following patterns, where L(i) denotes the LEAF_OP with index i (L(i) is
   essentially a multiexpression indexed by i):
		Original: (L(1) join L(2)) join L(3)
		Substitute: L(1) join (L(2) join L(3))

   One of the first steps in applying this rule is to find a multiexpression in the search
   space which binds to the Original pattern.  This is the task of objects in the BINDERY
   class.

   Suppose the multiexpression
		[ G7 join G4 ] join G10,
   where Gi is the group with GRP_ID i and [ X ] denotes the group containing X, is in 
   the optimizer's search space.  Then there is a binding of the Original pattern to 
   this multiexpression, given by binding L(1) to G7, L(2) to G4 and L(3) to G10. An
   object in the BINDERY class, called a bindery, will, over its lifetime, produce
   all such bindings.

   In order to produce a binding, a bindery must spawn one bindery for each input subgroup.  
   For example, consider a bindery for the LTOR associativity rule.  It will spawn a 
   bindery for the left input, which will seek all bindings to the pattern L(1) join L(2)
   and a bindery for the right input, which will seek all bindings to the pattern  L(3).
   In our example the right bindery will find only one binding, to the right input group G10.  The left bindery will typically find many bindings, 
   However, the left bindery will find one binding for each join in the left input group, 
   for example one binding for G7 join G4 and one for G4 join G7.
    
   Is a binding made to a multiexpression or to an expression?  In our example, a binding
   is sought for to the multiexpression [G7 join G4] join G10 .  However, the bindings
   which are produced are to expressions, such as (G7 join G4) join G10 and
   (G4 join G7) join G10.  Those two are distinct as expressions but would be identical
   as multiexpressions.

   BINDERY objects (binderys) are of two types.  Expression binderys bind the Original 
   pattern to only one multi-expression in a group.  A rule first creates an expression
   bindery.  This expression bindery then spawns one group bindery for each input group,
   as explained above. Group binderys bind to all expressions in a group.  See
   APPLY_RULE::perform  for details .

   Because Columbia and its predecessors apply rules only to logical expressions, binderys 
   bind logical operators only.

   A bindery may produce several bindings, as mentioned above.  Thus a bindery may go 
   through several stages: start, then loop over several valid bindings, then finish.

*/
#ifdef _REUSE_SIB
//##ModelId=3B0C0868038E
class Node {
public:
	//##ModelId=3B0C086803A3
	BINDERY *bindery;
	//##ModelId=3B0C086803B9
	Node *next;
	//##ModelId=3B0C086803C0
	Node() { bindery = NULL; next = NULL; }
};
#endif

//##ModelId=3B0EA6D90080
class BINDERY
{
	
private: 
	//##ModelId=3B0EA6D90185
    EXPR	* 	original;      // bind with this original pattern
	
	//##ModelId=3B0EA6D901CC
    M_EXPR   *	cur_expr;     // bind the original pattern to this multi-expression
	
	//##ModelId=3B0EA6D901DF
    bool        one_expr;       // Is this an expression bindery?
	
	//##ModelId=3B0EA6DA01FF
	GRP_ID		group_no;		// group no of the cur_expr
	
	//##ModelId=3B0EA6DB005B
    typedef enum BINDERY_STATE
    {
            start,          // This is a new MExpression
			valid_binding,  // A binding was found.
			finished,       // Finished with this expression
    } BINDERY_STATE;
	
	//##ModelId=3B0EA6DA021D
    BINDERY_STATE     state;
	
	//##ModelId=3B0EA6DA023D
    BINDERY ** input;        // binderys for input expr's
#ifdef _REUSE_SIB
	Node *list, *last, *currentBind;
	//##ModelId=3B0EA6DA024E
	void test_delete (int arity);
	
#endif
	
public:
    // Create an Expression bindery
	//##ModelId=3B0EA6DA0262
    BINDERY (M_EXPR * expr, EXPR * original);
	
    // Create a Group bindery
	//##ModelId=3B0EA6DA0280
    BINDERY (GRP_ID group_no, EXPR *original);
	
	//##ModelId=3B0EA6DA0294
    ~BINDERY ();
	
    //advance() requests a bindery to produce its next binding, if one
    //exists.  This may cause the state of the bindery to change.
    //advance() returns true if a binding has been found.
	//##ModelId=3B0EA6DA029E
    bool advance ();
	
    // If a valid binding has been found, then return the bound EXPR.  
	//##ModelId=3B0EA6DA02A8
	EXPR * extract_expr ();
	
    //print the name of the current state
	//##ModelId=3B0EA6DA02B2
    CString print_state();
	
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
	
public:									
	//##ModelId=3B0EA6DA02BD
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0EA6DA02DA
	inline void * operator new(size_t my_size)	
	{ return memory_manager -> allocate(&_anchor, (int) my_size, 5 ); }
	
	//##ModelId=3B0EA6DA02E5
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
	
}; // BINDERY


  /*
    Rules
    =====
	See BINDERY above.

    A rule is defined primarily by its original and substitute patterns.
    For example, the LTOR join associative rule has these patterns,
    in which L(i) stands for Leaf operator i:
           Original pattern: (L(1) join L(2)) join L(3)
         Substitute pattern: L(1) join (L(2) join L(3))

    The original and substitute patterns describe how to produce new 
	multi-expressions in the search space.  The production of these new 
	multi-expressions is done by APPLY_RULE::perform(), in two parts: 
    First a BINDERY object produces a binding of the original pattern to an EXPR 
    in the search space.  Then next_substitute() produces the new expression, 
    which is integrated into the search space by SSP::include().

    A rule is called an implementation rule if the root operator of its substitute
	pattern is a physical operator, for example the rule GET_TO_SCAN.

    Columbia and its ancestors use only rules for which all operators in the 
	original pattern are logical, and for which all operators in the substitute
	pattern are logical except perhaps the root operator.

    In O_EXPR::perform(), the optimizer decides which rules to push onto
    the PTASK stack.  It uses top_match() to check whether the root operator 
    of the original pattern of a rule matches the root operator of the 
	current multiexpression.

    The method O_EXPR::perform() must decide the order in which rules
    are pushed onto the PTASK stack.  For this purpose it uses promise().
    A promise value of 0 or less means do not schedule this rule here.  
    Higher promise values mean schedule this rule earlier.
    By default, an implementation  rule has a promise of 2 and others
    a promise of 1.  Notice that a rule's promise is evaluated before 
    inputs of the top-most operator are expanded (searched, transformed) 
    and matched against the rule's pattern; the promise value is used 
    to decide whether or not to explore those inputs.
    promise() is used for both exploring and optimizing, though for 
    exploration it is currently irrelevant.

    When a rule is applied (in APPLY_RULE::perform), and after a binding
    is found, the method condition() is invoked to determine whether 
    the rule actually applies.  condition() has available to it the entire
    EXPR matched to the rule's original pattern.  For example, the rule which
    pushes a select below a join requires a condition about compatibitily
    of schemas.  This condition cannot be checked until after the binding,
    since schemas of input groups are only available from the binding.

    The check() method verifies that a rule seems internally consistent,
    i.e., that a rule's given cardinality is consistent with its given
    pattern, and that pattern and substitute satisfy other
    requirements.  These requirements are:
    - leaves are numbered 0, 1, 2, ..., (arity-1)
    - all leaf numbers up to (arity-1) are used in the original pattern
    - each leaf number is used exactly once in the original pattern
    - the substitute uses only leaf numbers in the original pattern
    - (each leaf number may appear 0, 1, 2, or more times)
    - all operators in the original pattern are logical operators
    - all operators except the root in the substitute pattern are logical
*/
				   
#define		FILESCAN_PROMISE	5
#define		SORT_PROMISE		6
#define		MERGE_PROMISE		4
#define 	HASH_PROMISE		4
#define		PHYS_PROMISE		3
#define		LOG_PROMISE			1
#define 	ASSOC_PROMISE		2
				   
//##ModelId=3B0C08690303
class RULE
{
private : 
	//##ModelId=3B0C08690318
	EXPR    *  original;     // original pattern to match
	//##ModelId=3B0C08690336
	EXPR	*  substitute;   // replacement for original pattern
							 //"substitute" is used ONLY to tell if the rule is logical or physical,
							 // and, by check(), for consistency checks.  Its pattern is represented 
							 // in the method next_substitute()
	
	
	//##ModelId=3B0C0869033F
	int        arity;		// number of leaf operators in original pattern.
	//  Leaf ops must be numbered 0, 1, 2,..
	
	//##ModelId=3B0C08690353
	CString    name;
	
	//Used for unique rule sets
	//##ModelId=3B0C08690368
	BIT_VECTOR mask;	//Which rules to turn off in "after" expression
	//##ModelId=3B0C0869037B
	int index;			// index in the rule set
	
public :
	//##ModelId=3B0C0869038F
	RULE (CString name, int arity, EXPR * original, EXPR * substitute)
		:name(name),arity(arity),mask(0),
		original(original),substitute(substitute)
	{};
	
	//##ModelId=3B0C086903AD
	virtual ~RULE() 
	{
		delete original; 
		delete substitute; 
	};
	
	//##ModelId=3B0C086903AF
	inline CString GetName() {return(name) ; } ;
	//##ModelId=3B0C086903B7
	inline EXPR * GetOriginal() {return(original) ; } ;
	//##ModelId=3B0C086903C1
	inline EXPR * GetSubstitute() {return(substitute) ; } ;
	
	//##ModelId=3B0C086903CB
	bool top_match (OP *op_arg)
	{
		assert(op_arg->is_logical()); // to make sure never O_EXPR a physcial mexpr 
		
		// if original is a leaf, it represents a group, so it always matches
		if( original->GetOp()->is_leaf() ) return true;  
		
		// otherwise, the original pattern should have a logical root op
		return ( ((LOG_OP *)(original->GetOp()))->OpMatch((LOG_OP *)op_arg) ); 
		
	};
	
	// default value is 1.0, resulting in exhaustive search
	//##ModelId=3B0C086903D5
	virtual int promise ( OP * op_arg, int ContextID)
	{	return ( substitute->GetOp()->is_physical() ? PHYS_PROMISE : LOG_PROMISE); };
	
	// Does before satisfy this rule's condition, if we are using
	// context for the search?  mexpr is the multi-expression bound to
	// before, probably mexpr is not needed.
	// Default value is TRUE, i.e., rule applies
	//##ModelId=3B0C086A0001
	virtual bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID)
	{	return true; };
	
	//Given an expression which is a binding (before), this
	// returns the next substitute form (after) of the rule.
	//##ModelId=3B0C086A0015
	virtual EXPR * next_substitute (EXPR * before, PHYS_PROP * ReqdProp)=0;
	
	//##ModelId=3B0C086A0029
	bool check ();     // check that original & subst. patterns are legal
	
	//##ModelId=3B0C086A002A
	inline int  get_index()  {return(index);};		// get the rule's index in the rule set
	//##ModelId=3B0C086A0033
	inline BIT_VECTOR get_mask() {return(mask);};	// get the rule's mask
	
	//##ModelId=3B0C086A003D
	inline void set_index(int i) { index = i; };
	//##ModelId=3B0C086A0048
	inline void set_mask(BIT_VECTOR v) { mask = v; };
	
	//##ModelId=3B0C086A0053
	CString Dump()
	{	CString os;
	os.Format("RULE %s ", name);
	return os;
	};
	
	
	// if not stop generating logical expression when epsilon pruning is applied
	// need these to identify the substitue
#ifdef _GEN_LOG
	//##ModelId=3B0C086A005C
					   bool is_log_to_phys() { return(substitute->GetOp()->is_physical());};
	//##ModelId=3B0C086A005D
					   bool is_log_to_log() { return(substitute->GetOp()->is_logical());};
#endif
					   
				   }; // RULE
				   
/*
	 ============================================================
	 GET TO FILE SCAN
	 ============================================================
*/
//##ModelId=3B0C086A00C0
class GET_TO_FILE_SCAN : public RULE
{
public :
	
	//##ModelId=3B0C086A00CB
	GET_TO_FILE_SCAN ();
	//##ModelId=3B0C086A00D4
	~GET_TO_FILE_SCAN () {};
	//##ModelId=3B0C086A00DE
	EXPR * next_substitute (EXPR * before,PHYS_PROP * ReqdProp);
	//##ModelId=3B0C086A00E8
	int promise ( OP * op_arg, int ContextID)
	{	return FILESCAN_PROMISE; };
	
}; // GET_TO_FILE_SCAN

/*
   ============================================================
   EQJOIN to LOOPS Rule
   ============================================================
*/
//##ModelId=3B0C086A0156
class EQ_TO_LOOPS : public RULE
{
public :
	
	//##ModelId=3B0C086A0161
	EQ_TO_LOOPS ();
	//##ModelId=3B0C086A016A
	~EQ_TO_LOOPS () {};
	//##ModelId=3B0C086A0174
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
#ifdef CONDPRUNE
	//##ModelId=3B0C086A017E
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
#endif
	
}; // EQ_TO_LOOPS

/*
   ============================================================
   EQJOIN to Merge Join Rule
   ============================================================
*/
//##ModelId=3B0C086A0200
class EQ_TO_MERGE : public RULE
{
public :
	//##ModelId=3B0C086A0214
	EQ_TO_MERGE ();
	//##ModelId=3B0C086A0215
	int promise (OP* op_arg, int ContextID);
	//##ModelId=3B0C086A0228
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
#ifdef CONDPRUNE
	//##ModelId=3B0C086A0232
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
#endif
}; // EQ_TO_MERGE

   /*
   ============================================================
   EQJOIN to Hash Join Rule
   ============================================================
*/
//##ModelId=3B0C086A02A0
class EQ_TO_HASH : public RULE
{
public :
	//##ModelId=3B0C086A02B4
	EQ_TO_HASH ();
	//##ModelId=3B0C086A02B5
	int promise (OP* op_arg, int ContextID);
	//##ModelId=3B0C086A02C8
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // EQ_TO_HASH

/*
   ============================================================
   EQJOIN to LOOPS Index Rule
   ============================================================
*/
//##ModelId=3B0C086A0337
class EQ_TO_LOOPS_INDEX : public RULE
{
public :
	//##ModelId=3B0C086A0342
	EQ_TO_LOOPS_INDEX ();
	//##ModelId=3B0C086A034B
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086A0355
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID) ;	
}; // EQ_TO_LOOPS_INDEX

   /*
   ============================================================
   EQJOIN Commutativity Rule
   ============================================================
*/
//##ModelId=3B0C086A03C3
class EQJOIN_COMMUTE : public RULE
{
public :
	
	//##ModelId=3B0C086A03CE
	EQJOIN_COMMUTE ();
	//##ModelId=3B0C086A03D7
	~EQJOIN_COMMUTE() {};
	//##ModelId=3B0C086A03D8
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // EQJOIN_COMMUTE

/*
   ============================================================
   EQJOIN Associativity Rule	Left to Right
   ============================================================
*/
//##ModelId=3B0C086B008F
class EQJOIN_LTOR : public RULE
{
public :
	
	//##ModelId=3B0C086B00A3
	EQJOIN_LTOR ();
	//##ModelId=3B0C086B00A4
	~EQJOIN_LTOR() {};
	//##ModelId=3B0C086B00AD
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086B00B7
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID) ;		
	//##ModelId=3B0C086B00C3
	int promise ( OP * op_arg, int ContextID) { return ASSOC_PROMISE;};
	
}; // EQJOIN_LTOR

/*
   ============================================================
   EQJOIN Associativity Rule		Right to Left
   ============================================================
*/
//##ModelId=3B0C086B0161
class EQJOIN_RTOL : public RULE
{
public :
	
	//##ModelId=3B0C086B016C
	EQJOIN_RTOL ();
	//##ModelId=3B0C086B0175
	~EQJOIN_RTOL() {};
	//##ModelId=3B0C086B0176
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086B0181
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
}; // EQJOIN_RTOL

/*
   ============================================================
   Cesar's EXCHANGE Rule: (AxB)x(CxD) -> (AxC)x(BxD)
   ============================================================
*/
//##ModelId=3B0C086B0234
class EXCHANGE : public RULE
{
public :
	
	//##ModelId=3B0C086B023F
	EXCHANGE ();
	//##ModelId=3B0C086B0248
	~EXCHANGE() {};
	//##ModelId=3B0C086B0249
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086B025C
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
	
}; // EXCHANGE

/*
 * Project to Physical Project Rule
 */
//##ModelId=3B0C086B02CA
class P_TO_PP : public RULE
{
	
public :
	
	//##ModelId=3B0C086B02DE
	P_TO_PP ();
	//##ModelId=3B0C086B02DF
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // P_TO_PP

/*
 * Select to filter rule
 */
//##ModelId=3B0C086B0356
class SELECT_TO_FILTER : public RULE
{
	
public :
	
	//##ModelId=3B0C086B0361
	SELECT_TO_FILTER ();
	//##ModelId=3B0C086B036A
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // SELECT_TO_FILTER


//##ModelId=3B0C086C002C
class SORT_RULE : public RULE
{
	
public :
	
	//##ModelId=3B0C086C0037
	SORT_RULE ();
	//##ModelId=3B0C086C0040
	~SORT_RULE() {};
	
	//##ModelId=3B0C086C0041
	int promise ( OP* op_arg, int ContextID);
	
	//##ModelId=3B0C086C004C
	EXPR * next_substitute (EXPR * before, PHYS_PROP* ReqdProp);
}; // SORT_RULE


//##ModelId=3B0C086C00D7
class RM_TO_HASH_DUPLICATES : public RULE
{
	
public :
	
	//##ModelId=3B0C086C00EB
	RM_TO_HASH_DUPLICATES ();
	//##ModelId=3B0C086C00EC
	~RM_TO_HASH_DUPLICATES () {};
	//##ModelId=3B0C086C00F5
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // RM_TO_HASH_DUPLICATES

//##ModelId=3B0C086C0177
class AL_TO_HGL : public RULE
{
	
public :
	
	//##ModelId=3B0C086C018C
	AL_TO_HGL (AGG_OP_ARRAY *list1, AGG_OP_ARRAY *list2);
	//##ModelId=3B0C086C0195
	~AL_TO_HGL () {};
	//##ModelId=3B0C086C0196
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // AL_TO_HGL

//##ModelId=3B0C086C0221
class FO_TO_PFO : public RULE
{
	
public :
	
	//##ModelId=3B0C086C0235
	FO_TO_PFO ();
	//##ModelId=3B0C086C023F
	~FO_TO_PFO () {};
	//##ModelId=3B0C086C0240
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
}; // FO_TO_PFO

//##ModelId=3B0C086C02FD
class AGG_THRU_EQJOIN : public RULE
{
	
public :
	
	//##ModelId=3B0C086C0308
	AGG_THRU_EQJOIN (AGG_OP_ARRAY *list1, AGG_OP_ARRAY *list2);
	//##ModelId=3B0C086C0313
	~AGG_THRU_EQJOIN () {};
	//##ModelId=3B0C086C031B
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086C0325
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
}; // AGG_THRU_EQJOIN

//##ModelId=3B0C086C03E4
class EQ_TO_BIT : public RULE
{
public :
	
	//##ModelId=3B0C086D0010
	EQ_TO_BIT ();
	//##ModelId=3B0C086D0011
	~EQ_TO_BIT () {};
	//##ModelId=3B0C086D001A
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086D0024
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
}; // EQ_TO_BIT

//##ModelId=3B0C086D00E2
class SELECT_TO_INDEXED_FILTER : public RULE
{
	
public :
	
	//##ModelId=3B0C086D00F6
	SELECT_TO_INDEXED_FILTER ();
	//##ModelId=3B0C086D00F7
	~SELECT_TO_INDEXED_FILTER () {};
	//##ModelId=3B0C086D0100
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//##ModelId=3B0C086D010A
	bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
}; // SELECT_TO_INDEXED_FILTER

//##ModelId=3B0C086D01AA
class PROJECT_THRU_SELECT : public RULE
{
	
public :
	
	//##ModelId=3B0C086D01B5
	PROJECT_THRU_SELECT ();
	//##ModelId=3B0C086D01B6
	~PROJECT_THRU_SELECT () {};
	//##ModelId=3B0C086D01BE
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	//bool condition ( EXPR * before, M_EXPR *mexpr, int ContextID);
}; // PROJECT_THRU_SELECT
/*
   ============================================================
   DUMMY to PDUMMY Rule
   ============================================================
*/

//##ModelId=3B0C086D025F
class DUMMY_TO_PDUMMY : public RULE
{
public :
	
	//##ModelId=3B0C086D026A
	DUMMY_TO_PDUMMY ();
	//##ModelId=3B0C086D026B
	~DUMMY_TO_PDUMMY () {};
	//##ModelId=3B0C086D0273
	EXPR * next_substitute (EXPR * before,PHYS_PROP* ReqdProp);
	
}; // DUMMY_TO_PDUMMY

#endif //RULES_H

