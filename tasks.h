/* $Id$
TASKS.H - OPTIMIZATION USING TASKS

  Columbia Optimizer Framework
  
	A Joint Research Project of Portland State University 
	and the Oregon Graduate Institute
	Directed by Leonard Shapiro and David Maier
	Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#ifndef TASKS_H
#define TASKS_H

#include "ssp.h"
#include "rules.h"

class TASKS;		//Abstract class for all task classes
class PTASKS;		//Pending tasks - some structure which contains all tasks waiting to execute
class O_GROUP;		//Optimize a GROUP - find the cheapest plan in the group satisfying a context
class O_EXPR;		//Optimize an EXPR - Fire all relevant rules for this expression
class E_GROUP;		//Explore a group - Fire all transformation rules in this group.
class O_INPUTS;		//Optimize inputs - determine if this expression satisfies the current context
class APPLY_RULE;	//Apply a single rule to a single M_EXPR

/* ============================================================ */

/*
Moves
=====
Pair of rule and promise, used to sort rules according to their promise
=======================================
*/
//##ModelId=3B0C085D003F
typedef struct MOVE
{
	int		promise;
	RULE	* rule;
} MOVE;

/*
Afters
=====
Pair of expr and cost value, used to sort expr according to their cost
=======================================
*/
//##ModelId=3B0C085D0053
typedef struct AFTERS
{
	M_EXPR * m_expr;
	COST * cost;
} AFTERS;

/*
        ============================================================
        TASKS   
        ============================================================
        A task is an activity within the search process.  The original task
        is to optimize the entire query.  Tasks create and schedule each
        other; when no pending tasks remain, optimization terminates.
    
        In Cascades and Columbia, tasks store winners in memos; they do not
        actually produce a best plan.  After the optimization terminates,
        SSP::CopyOut() is called to print the best plan.
    
        TASK is an abstract class.  Its subclasses are specific tasks.
    
    
        Tasks must destroy themselves when done!
*/

//##ModelId=3B0C085D007B
class TASK
{
	friend class PTASKS;
    
private :
	//##ModelId=3B0C085D008F
	TASK        * next;         // Used by class PTASK
    
protected :
	//##ModelId=3B0C085D00A3
	int		 ContextID;      // Index to CONT::vc, the shared set of contexts
	//##ModelId=3B0C085D00AD
	int      ParentTaskNo; // The task which created me
    
public :
    
	//##ModelId=3B0C085D00C1
	TASK (int ContextID, int ParentTaskNo);
	//##ModelId=3B0C085D00D5
	~TASK () {} ;
    
	//##ModelId=3B0C085D00D6
	virtual CString Dump()=0;
    
	//##ModelId=3B0C085D00F3
	virtual void perform ()=0;	//TaskNo is current task number, which will
}; // TASK

 /*
   ============================================================
   PTASKS - Pending Tasks
   ============================================================
   This collection of undone tasks is currently stored as a stack.
   Other structures are certainly appropriate, but in any case dependencies
   must be stored.  For example, a directed graph could be used to
   parallelize optimization.
*/

//##ModelId=3B0C085D0125
class PTASKS
{
    
private :
    
	//##ModelId=3B0C085D0130
	TASK        * first;        // anchor of PTASKS stack
    
public :
    
	//##ModelId=3B0C085D0143
	PTASKS ();
	//##ModelId=3B0C085D014D
	~PTASKS ();
    
	//##ModelId=3B0C085D0157
	bool empty ();
	//##ModelId=3B0C085D0161
	void push (TASK * task);
	//##ModelId=3B0C085D016B
	TASK * pop ();
    
	//##ModelId=3B0C085D0175
	CString Dump();
}; // PTASKS


   /*
        ============================================================
        O_GROUP - Task to Optimize a Group
        ============================================================
        This task finds the cheapest multiplan in this group, for a given
        context, and stores it (with the context) in the group's winner's circle.
        If there is no cheapest plan (e.g. the upper bound cannot be met),
        the context is stored in the winner's circle with a null plan.
    
        This task generates all relevant expressions in the group, costs them 
        and chooses the cheapest one.
    
        The determination of what is "cheapest" may include such issues as
        robustness, e.g. this task may choose the multiplan with smallest
        cost+variance.  Here variance is some measure of how much the cost
        varies as the statistics vary.
        
        There are at least two ways to implement this task; we will discover which 
        is best.
    
        First, Goetz' original way, is to process each multiexpression separately,
        in the order they appear in the list/collection of multiexpressions.  To
        process an expression means to determine all relevant rules, then fire them 
        in order of promise().
    
        Second, hinted at by Goetz, applies only when there are multiple expressions 
        in the group, e.g. after exploring.  Here we can consider all rules on all
        expressions, and fire them in order of their promise.  Promise here may include
    	a lower bound estimate for the expressions.
			 
*/

//##ModelId=3B0C085D01C5
class O_GROUP : public TASK
{
    
private :
    
	//##ModelId=3B0C085D01DA
	GRP_ID	GrpID;		//Which group to optimize
	//##ModelId=3B0C085D01ED
	bool    Last;       // if this task is the last task for this group
	//##ModelId=3B0C085D0202
	COST *  EpsBound;   // if global eps pruning is on, this is the eps bound for eps pruning
	// else it is zero
public :
    
	//##ModelId=3B0C085D0215
	O_GROUP(GRP_ID	GrpID, int ContextID, int parent_task_no, bool last = true,
		COST * epsbound = NULL);
	//##ModelId=3B0C085D0234
	~O_GROUP() 
	{
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_GROUP].Delete();
		if (EpsBound) delete EpsBound;
	};
	
	// Optimize the group by searching for a winner for the context.
	// Initialize or update the winner for the context's property
	//##ModelId=3B0C085D0235
	void perform ();
    
	//##ModelId=3B0C085D023E
	CString Dump();
    
}; // O_GROUP


   /*
   
        ============================================================
        E_GROUP - Task to Explore the Group
        ============================================================
        Some rules require that their inputs contain particular (target) operators.  For
        example, the associativity rule requires that one input contain a join.  The
        E_GROUP task explores a group by creating all target operators that could
        belong to the group, e.g., fire whatever rules are necessary to create all
        joins that could belong to the group.
    
        The simplest implementation of this task is to generate all logical 
        multiexpressions in the group.  
    
        More sophisticated implementations would fire only those rules which might 
        generate the target operator.  But it is hard to tell what those rules
        are (note that it may require a sequence of rules to get to the target).
        Furthermore, on a second E_GROUP task for a second target it may be
        difficult to use the results of the first visit intelligently.
    
        Because we are using the simple implementation, we do not need an E_EXPR task.
    	Instead we will use the O_GROUP task but ensure that it fires only transformation rules.
    
    	If we are lucky, groups will never need to be explored: physical rules are fired first,
    	and the firing of a physical rule will cause all inputs to be optimized, therefore explored.
    	This may not work if we are using pruning: we might skip physical rule firings because of
    	pruning, then need to explore.  For now we will put a flag in E_GROUP to catch when it does not work.
    */
//##ModelId=3B0C085D02A2
class E_GROUP : public TASK
{
    
private :
    
	//##ModelId=3B0C085D02B7
	GRP_ID    GrpID;	//Group to be explored
	//##ModelId=3B0C085D02C0
	bool	  Last;     // is it the last task in this group
	//##ModelId=3B0C085D02DF
	COST *  EpsBound;   // if global eps pruning is on, this is the eps bound for eps pruning
	// else it is zero
public :
    
	//##ModelId=3B0C085D02E8
	E_GROUP (	GRP_ID    GrpID,
		int ContextID,
		int parent_task_no, 
		bool last = false,
		COST * epsbound = NULL);
	//##ModelId=3B0C085D0306
	~E_GROUP()
	{
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_E_GROUP].Delete();
		if (EpsBound) delete EpsBound;
	};
    
	//##ModelId=3B0C085D0307
	void perform ( );
    
	//##ModelId=3B0C085D0310
	CString Dump();
}; // E_GROUP

/*
   ============================================================
   O_EXPR - Task to Optimize a multi-expression
   ============================================================ 
   This task is needed only if we implement O_GROUP in the original way.
   This task fires all rules for the expression, in order of promise.
   when it is used for exploring, fire only transformation rules to prepare for a transform
*/

//##ModelId=3B0C085D03A6
class O_EXPR : public TASK
{
    
private :
	//##ModelId=3B0C085D03BB
	M_EXPR	* MExpr;	//Which expression to optimize
	//##ModelId=3B0C085D03CE
	const  bool	explore;	// if this task is for exploring  Should not happen - see E_GROUP
	//##ModelId=3B0C085D03D8
	bool		Last;		// if this task is the last task for the group
	//##ModelId=3B0C085E000E
	COST *	  EpsBound;  // if global eps pruning is on, this is the eps bound of this task
	// else it is zero
    
public :
    
	//##ModelId=3B0C085E0018
	O_EXPR (M_EXPR *mexpr, bool explore, int ContextID, int parent_task_no, bool last=false,
		COST * epsbound = NULL);
    
	//##ModelId=3B0C085E0036
	~O_EXPR() 
	{ 
		if(Last)
		{
			
			GROUP * Group = Ssp->GetGroup(MExpr->GetGrpID());
			if (!explore)
			{
#ifndef IRPROP
				CONT * LocalCont = CONT::vc[ContextID];
				//What prop is required of
				PHYS_PROP * LocalReqdProp =  LocalCont -> GetPhysProp();	
				WINNER * Winner = Group->GetWinner(LocalReqdProp);
				
				// mark the winner as done
				if(Winner)
					assert(!Winner->GetDone());
				
				Winner -> SetDone(true);
#endif
				// this's still the last applied rule in the group, 
				// so mark the group with completed optimizing
				Group->set_optimized(true);
			}
			else Group->set_explored(true);
		}
		
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_O_EXPR].Delete();
		if (EpsBound) delete EpsBound;
	};
	
    
	//##ModelId=3B0C085E0040
	CString Dump();
    
	//##ModelId=3B0C085E0041
	void perform ();
    
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
    
public:									
	//##ModelId=3B0C085E0055
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0C085E005E
	inline void * operator new(size_t my_size)	
	{ return memory_manager -> allocate(&_anchor, (int) my_size); }
    
	//##ModelId=3B0C085E0072
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
    
}; // O_EXPR

   /*
   
	 ========================================================
        O_INPUTS - Task to Optimize inputs 
        ========================================================
    	This task is rather misnamed.  It:
    	1) Determines whether the (physical) MExpr satisfies the task's context
    	2) As part of (1), it may optimize/cost some inputs
    	3) It may adjust the current context and the current winner.
    	It may use bounds, primarily upper bounds, in its work.
        
    	Member data InputNo, initially 0, indicates which input has been 
    	costed.  This task is unique in that it does not terminate after 
    	scheduling other tasks.  If the current input needs to be optimized, 
    	it first pushes itself onto the stack, then it schedules 
        the optimization of the current input.  If and when inputs are all
        costed, it calculates the cost of the entire physical expression.
    */

//##ModelId=3B0C085E01E5
class O_INPUTS : public TASK
{
    
private :
    
	//##ModelId=3B0C085E01FA
	M_EXPR *	MExpr;		// expression whose inputs we are optimizing
	//##ModelId=3B0C085E020D
	int			arity;		
	//##ModelId=3B0C085E0217
	int			InputNo;	// input currently being or about to be optimized, initially 0
	//##ModelId=3B0C085E022B
	int			PrevInputNo;// keep track of the previous optimized input no
	//##ModelId=3B0C085E0240
	COST *		LocalCost;	// the local cost of the mexpr
	//##ModelId=3B0C085E0249
	bool		Last;		// if this task is the last task for the group
	//##ModelId=3B0C085E0268
	COST *		EpsBound;   // if global eps pruning is on, this is the eps bound for eps pruning
	// else it is zero
	//##ModelId=3B0C085E0271
	int			ContNo;		// keep track of number of contexts 
    
	//Costs and properties of input winners and groups.  Computed incrementally
	// by this method.
	//##ModelId=3B0C085E0290
	COST ** InputCost;
	//##ModelId=3B0C085E02A4
	LOG_PROP ** InputLogProp;
    
public :
    
	//##ModelId=3B0C085E02B7
	O_INPUTS (
		M_EXPR * MExpr,
		int ContextID,
		int ParentTaskNo,
		bool last=false,
		COST * epsbound = NULL,
		int ContNo = 0);
    
	//##ModelId=3B0C085E02E9
	~O_INPUTS ();
    
	// return the new upper bd for the input
	//##ModelId=3B0C085E02F3
	COST* NewUpperBd(COST * OldUpperBd, int input);
	//##ModelId=3B0C085E0307
	void perform ();
    
	//##ModelId=3B0C085E0311
	CString Dump();
    
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
    
public:									
	//##ModelId=3B0C085E031C
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0C085E0325
	inline void * operator new(size_t my_size)	
	{ return memory_manager -> allocate(&_anchor, (int) my_size); }
    
	//##ModelId=3B0C085E0339
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
    
}; // O_INPUTS

/*
   ============================================================
   APPLY_RULE - Task to Apply a Rule to a Multi-Expression
   ============================================================ 
*/

//##ModelId=3B0C085F0092
class APPLY_RULE : public TASK
{
private : 
    
	//##ModelId=3B0C085F00A7
	RULE		* Rule;		// rule to apply
	//##ModelId=3B0C085F00BB
	M_EXPR		* MExpr;		// root of expr. before rule
	//##ModelId=3B0C085F00CE
	const  bool	explore;	// if this task is for exploring
	//##ModelId=3B0C085F00E2
	bool		Last;		// if this task is the last task for the group
	//##ModelId=3B0C085F00F7
	COST *  EpsBound;   // if global eps pruning is on, this is the eps bound for eps pruning
	// else it is zero
public :
	//##ModelId=3B0C085F0100
	APPLY_RULE (RULE * rule,
		M_EXPR * mexpr,
		bool explore,
		int ContextID,
		int parent_task_no,
		bool last=false,
		COST *epsbound = NULL);
    
	//##ModelId=3B0C085F0132
	~APPLY_RULE() ;
    
	//##ModelId=3B0C085F0133
	void perform ();
    
	//##ModelId=3B0C085F013C
	CString Dump();
    
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
    
public:									
	//##ModelId=3B0C085F0147
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0C085F0150
	inline void * operator new(size_t my_size)	
	{ return memory_manager -> allocate(&_anchor, (int) my_size, 100); }
    
	//##ModelId=3B0C085F0164
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
    
}; // APPLY_RULE


#endif //TASKS_H

