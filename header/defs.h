/* 
DEFS.H - General programming support: #includes, #defines, typedefs, tracing. 
$Revision: 21 $    
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#ifndef DEFS_H
#define DEFS_H

#include "stdafx.h"

/*
============================================================
General definitions
============================================================
*/
#define KILO			1024       //For TPC-D sizes
#define LINE_STRING     ("===========")

#define MIN(a,b)	((a>b) ? b : a)
#define MAX(a,b)	((a>b) ? a : b)

/* number of slots in a (locally defined) array */
#define slotsof(ARRAY)		(sizeof (ARRAY) / sizeof (ARRAY [0]))

//needed for hashing, used for duplicate elimination.  
//See ../doc/dupelim and ../doc/dupelim.pcode
#define LOG2HTBL	13		//LOG2 of number of hash buckets to hold mexprs.
#define  HtblSize	( 1 << LOG2HTBL )	// hash table size we used
//##ModelId=3B0C087602F8
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
//##ModelId=3B0C0876030C
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

//##ModelId=3B0C0876032A
typedef  CArray < CString, CString >  STRING_ARRAY;
//##ModelId=3B0C08760348
typedef  CArray < int, int >  INT_ARRAY;

//##ModelId=3B0C0876035C
typedef	unsigned int	BIT_VECTOR;		// Used to implement unique rule set.  Note this
//restricts the number of transformational (logical) rules.	

extern bool ForGlobalEpsPruning;	//If true, we are running the optimizer to get an
//estimated cost to use for global epsilon pruning.

//Used to trace the number of objects in each class, 
// to determine where to use memory pooling.
//##ModelId=3B0C0876037A
typedef enum    CLASS{
	    C_AGG_LIST ,
		C_AGG_OP ,
        C_APPLY_RULE , 
		C_ATTR , 
		C_ATTR_EXP,
		C_ATTR_OP,
		C_BINDERY , 
		C_BIT_JOIN ,
		C_COMP_OP,
		C_CONST_INT_OP,
		C_CONST_SET_OP,
		C_CONST_STR_OP,
		C_CONT,	
		C_COST, 
		C_E_GROUP,
		C_EQJOIN, 
		C_EXPR, 
		C_FILE_SCAN, 
		C_FILTER,
		C_FUNC_OP,
		C_GET,
		C_GROUP,
		C_HASH_DUPLICATES,
		C_HGROUP_LIST,
		C_INDEXED_FILTER,
		C_KEYS_SET,
		C_LEAF_OP, 
		C_LOG_COLL_PROP,
		C_LOG_ITEM_PROP,
		C_LOOPS_INDEX_JOIN,
		C_LOOPS_JOIN,
		C_M_EXPR, 
		C_M_WINNER,
		C_MERGE_JOIN,
		C_HASH_JOIN,
		C_O_EXPR, 
		C_O_GROUP, 
		C_O_INPUTS,
		C_P_FUNC_OP,
		C_P_PROJECT,
		C_PHYS_PROP, 
		C_PROJECT,
		C_QSORT, 
		C_RM_DUPLICATES,
		C_SCHEMA,
		C_SELECT,
		C_WINNER
} CLASS;

//##ModelId=3B0C08760398
typedef enum	ORDER_AD{
	ascending, descending
} ORDER_AD;

//##ModelId=3B0C087603B6
typedef CArray <ORDER_AD, ORDER_AD> KeyOrderArray;

//any means any property
//##ModelId=3B0C087603D4
typedef enum    ORDER{
	any, heap, sorted,
		hashed//assume unique hash function for entire system
} ORDER;

//##ModelId=3B0C0877000A
typedef enum    ORDER_INDEX {
	btree,
		hash//assume unique hash function for entire system
} ORDER_INDEX;

//unknown is needed for attributes generated along the query tree
//e.g. sum(xxx) as .SUM, whose domain is unknown
//##ModelId=3B0C0877001E
typedef enum DOM_TYPE
{string_t, int_t, real_t, unknown} DOM_TYPE;

//##ModelId=3B0C0877003C
typedef int CONT_ID;	//ID of a context

//##ModelId=3B0C0877005A
typedef int GRP_ID;     //ID of a Group

/*============================================================
    TRACE stuff
  ============================================================
*/

// trace one object.  Output is newline==, TraceDepth==, file, line -----, then
//object in the format given.
#define PTRACE(format, object)  {if(TraceOn && !ForGlobalEpsPruning){ CString OutputString;	\
	CString temp;				            \
	temp.Format(format,object);		        \
	OutputString.Format("%s%d%s%s%s%d%s",   \
	"\r\n==", TraceDepth, "==",             \
	Trim(__FILE__),",",__LINE__," ----- "); \
	OutputString += temp;			        \
	if(WindowTrace) OutputWindow->Print(OutputString); \
    if(FileTrace) OutputFile.Write(OutputString, OutputString.GetLength());	}}
    

// trace two objects.  Format as above.
#define PTRACE2(format,obj1,obj2)  {if(TraceOn&& !ForGlobalEpsPruning){ CString OutputString,temp;\
	temp.Format(format,obj1,obj2);		               \
	OutputString.Format("%s%d%s%s%s%d%s",	           \
	"\r\n==", TraceDepth, "==",                        \
	Trim(__FILE__),",",__LINE__," ----- ");            \
	OutputString += temp;			                   \
	if(WindowTrace) OutputWindow->Print(OutputString); \
if(FileTrace) OutputFile.Write(OutputString, OutputString.GetLength());	}}

//Print n tabs, then the character string.  No newlines except as in string.
#define OUTPUTN(n, string) {if (!ForGlobalEpsPruning)\
{ \
	CString OutputString, temp; \
	temp.Format("    ");        \
	for(int i = 0; i < n ; i++) \
{ \
	OutputString += temp; \
} \
	temp.Format("%s", string);         \
	OutputString += temp;              \
	OutputWindow->Print(OutputString); \
OutputFile.Write(OutputString, OutputString.GetLength());	}}

// trace without line and file info.  No newlines except in format input.
#define WTRACE(format, object)  {if(TraceOn){ CString OutputString;	      \
	OutputString.Format(format,object) ;	                              \
	if(WindowTrace) OutputWindow->Print(OutputString);                    \
if(FileTrace) OutputFile.Write(OutputString, OutputString.GetLength());	}}

// Output the object to window and OutputFile, even if tracing is off.    
// No newlines except in format input.
//First one writes to window and trace file, second to file only.
#define OUTPUT(format, object) {if (!ForGlobalEpsPruning) { CString OutputString;	\
	OutputString.Format(format,object) ;	                                        \
	OutputWindow->Print(OutputString);                                              \
OutputFile.Write(OutputString, OutputString.GetLength());	}}

//Write the object to OutputFile
#define TRACE_FILE(format, object)  { CString OutputString;	\
	OutputString.Format(format,object) ;	\
OutputFile.Write(OutputString, OutputString.GetLength());	}

// display error message to Window and OutputFile
#define OUTPUT_ERROR(text)	{ CString OutputString;          \
	OutputString.Format("%s%s%s%s%s%d%s",                    \
	"\r\nERROR:" , text , ",file:" , Trim(__FILE__) ,        \
	",line:" , __LINE__ ,"\r\n");                            \
	OutputWindow->Print(OutputString);                       \
	OutputFile.Write(OutputString, OutputString.GetLength());\
abort(); }

/* ==========  Optimizer related ============  */

class CWcolView;
class QUERY;
class PTASKS;
class SSP;
class CAT;
class RULE;
class OPT_STAT;
class CLASS_STAT;
class SET_TRACE;
#ifdef USE_MEMORY_MANAGER
class MEMORY_MANAGER;
#endif
class RULE_SET;
class CM;
class KEYS_SET;
class M_EXPR;

extern OPT_STAT * OptStat;		// stat. info. of Optimizer
extern CLASS_STAT ClassStat[];  // stat. info of all classes
extern int CLASS_NUM;

#ifdef _DEBUG
//Rule Firing Statistics
extern INT_ARRAY TopMatch;
extern INT_ARRAY Bindings;
extern INT_ARRAY Conditions;
#endif

extern STRING_ARRAY  CollTable;		// collection name table
extern STRING_ARRAY  AttTable;		// attribute name table
extern STRING_ARRAY  IndTable;		// index name table
extern INT_ARRAY  AttCollTable;		// attribute to collid table
extern STRING_ARRAY  BitIndTable;	// Bitind name table

extern CFile OutputFile;	// global output file
extern CFile OutputCOVE;	// global COVE script file
extern int TraceDepth;		// Not the stack depth, but the number of times SET_TRACE
// objects have been created in current stack functions.
extern bool TraceOn;		// Are we tracing?
extern bool FileTrace;		// Are we sending the trace output to the output file?
extern bool COVETrace;		// Are we doing COVE tracing?
extern bool WindowTrace;	// Are we sending the tracing to the Window?
extern bool TraceFinalSSP;	// Does the trace print the final search space?
extern bool TraceOPEN;		// Should we force tracing of the OPEN stack?
extern bool TraceSSP;		// Should we force tracing of the Search Space?
extern bool Pruning;		// global pruning flag
extern bool CuCardPruning;		// global cucard pruning flag
extern bool GlobepsPruning;	// global epsilon pruning flag
extern int RadioVal;		// Radio value for queryfile
extern bool SingleLineBatch;	//Should output of batch queries be one line per query?
//SingleLineBatch and _TABLE_ cannot both be true
extern bool Halt;				// halt flat
extern int  HaltGrpSize;		// halt when number of plans equals to 100% of group
extern int  HaltWinSize;       // window size for checking the improvement
extern int  HaltImpr;			// halt when the improvement is less than 20%
extern int TaskNo;			//Number of the current task.
extern int	Memo_M_Exprs;	//How Many M_EXPRs in the MEMO Structure?

extern double GLOBAL_EPS;	// global epsilon value

extern CWcolView* OutputWindow;
extern QUERY*	Query;
extern PTASKS	PTasks;
extern SSP*	Ssp;
extern CAT* Cat;
extern RULE_SET* RuleSet;
extern CM* Cm;
extern KEYS_SET IntOrdersSet;	// set of Interesting Orders

#ifdef USE_MEMORY_MANAGER
extern MEMORY_MANAGER * memory_manager;
#endif

extern bool NO_PHYS_IN_GROUP;

/* ======= Pragmas ====== */

#pragma warning(disable:4291)	// Allows Bill's memory mgr to work w/o warnings

#endif 

/* ======= Symbolic Constants ====== */

	/*
	IRPROP: off == Optimize each group for one property only
			on == Optimize each group for all Interesting Relevant Properties
	UNIQ:   on == Use the unique rule set rules
	USE_MEMORY_MANAGER: Use memory pooling for selected classes
	SORT_AFTERS: sort possible moves in order of estimated cost
	INFBOUND: When optimizing a group, ignore the initial upper bound; use infinity instead
	FIRSTPLAN: trace when the first complete plan is costed
	_COSTS_ Prints the cost of each mexpr as it is costed, in the output window
	_TABLE_: prints one summary line for each optimization, using different epsilons
	_GEN_LOG: Used to control the generation of logical expressions when eps pruning is done.
	REUSE_SIB: An attempt to improve pattern matching by reusing one side of generated mexprs.
	NOCART: Do not allow Cartesian products during optimization.
	SAFETY: Used within Bill's memory manager.  Higher level of error checking.
	CONDPRUNE: Use Group Pruning technique as a condition for firing some rules
	*/
	
//DEFS_H



