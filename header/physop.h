/*
PHYSOP.H - physical Operators
$Revision: 6 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

//physical operators

#ifndef PHYSOP_H
#define PHYSOP_H

#include "op.h"

class FILE_SCAN;
class LOOPS_JOIN;
class LOOPS_INDEX_JOIN;
class MERGE_JOIN;
class P_PROJECT;
class FILTER;
class INDEXED_FILTER;
class QSORT;
class HASH_DUPLICATES;
class HGROUP_LIST;
class P_FUNC_OP;
class BIT_JOIN;

/*
File scan
=========
Physical version of GET.  Retrieves all data from the specified file.
*/
//##ModelId=3B0C086E013E
class FILE_SCAN : public PHYS_OP
{
public :
	
	//##ModelId=3B0C086E0149
    FILE_SCAN (int fileId);
	//##ModelId=3B0C086E0153
	FILE_SCAN (FILE_SCAN& Op);
	
	//##ModelId=3B0C086E015D
	inline OP * Clone() { return new FILE_SCAN(*this); };
	
	//##ModelId=3B0C086E0166
	~FILE_SCAN() { if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_FILE_SCAN].Delete();};
	
	//##ModelId=3B0C086E0170
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	// get the physical prop according to the order of the collection
	//##ModelId=3B0C086E017A
	PHYS_PROP * FindPhysProp ( PHYS_PROP ** input_phys_props = NULL);
	
	//##ModelId=3B0C086E0184
    inline CString GetName() {return ("FILE_SCAN"); };	
	//##ModelId=3B0C086E018E
	inline int GetArity() {return(0);};
	//##ModelId=3B0C086E018F
	inline int GetFileId() { return FileId; };	
	
	//##ModelId=3B0C086E0198
	CString Dump()
	{	CString os;
	os.Format("%s%s%s%s",GetName(),"(",GetCollName(FileId),")");
	return os;
	};
	
	//File_scan does not have any InputReqProp
	//##ModelId=3B0C086E01A2
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible)
	{
		assert(false); // make sure ops should define this method except 0 arity op
		return NULL;
	};
	
	
private:
	//##ModelId=3B0C086E01AE
    int FileId;
	//##ModelId=3B0C086E01C0
    CString 	RangeVar;
	
}; // FILE_SCAN


/*
   Loops Join
   ==========
   A physical version of EQJOIN.  Nested loops, not index nested loops.
*/
//##ModelId=3B0C086E0210
class LOOPS_JOIN : public PHYS_OP
{
public:
	//##ModelId=3B0C086E0224
    int * 	lattrs;
	//##ModelId=3B0C086E022E
	int * 	rattrs;
	//##ModelId=3B0C086E0238
	int		size;
	
public :
	
	//##ModelId=3B0C086E0242
	LOOPS_JOIN(int * lattrs, int * rattrs, int size);
	//##ModelId=3B0C086E0256
	LOOPS_JOIN( LOOPS_JOIN& Op);
	
	//##ModelId=3B0C086E0260
	inline OP * Clone() { return new LOOPS_JOIN(*this); };
	
	//##ModelId=3B0C086E026A
	~LOOPS_JOIN() 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_JOIN].Delete();
	delete [] lattrs;
	delete [] rattrs;
	};
	
	//##ModelId=3B0C086E026B
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C086E0276
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	//##ModelId=3B0C086E0288
	inline int GetArity() {return(2);};
	//##ModelId=3B0C086E0292
    inline CString GetName() {return ("LOOPS_JOIN"); };  
	
	//##ModelId=3B0C086E029C
	CString Dump();
	
	// the following is used by Bill's Memory Manager
	// Redefine new and delete if memory manager is used.
#ifdef USE_MEMORY_MANAGER		// use bill's memory manager
	
public:									
	//##ModelId=3B0C086E02A7
	static BLOCK_ANCHOR * _anchor ;				
public:										
	// overload the new and delete methods
	//##ModelId=3B0C086E02BA
	inline void * operator new(size_t my_size)	
	{ return memory_manager -> allocate(&_anchor, (int) my_size); }
	
	//##ModelId=3B0C086E02C4
	inline void operator delete(void * dead_elem, size_t )	
	{ memory_manager -> deallocate(_anchor, dead_elem) ; }	
#endif
	
}; // LOOPS_JOIN

// Physical Dummy Operator.  Just to give the dummy operator a physical counterpart.

//##ModelId=3B0C086E0314
class PDUMMY : public PHYS_OP
{
	
public :
	
	//##ModelId=3B0C086E031F
	PDUMMY();
	//##ModelId=3B0C086E0328
	PDUMMY( PDUMMY& Op);
	
	//##ModelId=3B0C086E0333
	inline OP * Clone() { return new PDUMMY(*this); };
	
	//##ModelId=3B0C086E033C
	~PDUMMY() 
	{	};
	
	//##ModelId=3B0C086E0346
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp,
		LOG_PROP ** InputLogProp); 
	
	//##ModelId=3B0C086E0350
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	//##ModelId=3B0C086E035C
	inline int GetArity() {return(2);};
	//##ModelId=3B0C086E0364
    inline CString GetName() {return ("PDUMMY"); };  
	
	//##ModelId=3B0C086E036E
	CString Dump();
	
}; // PDUMMY

/*
   Nested loops index join
   =======================
*/

//##ModelId=3B0C086E03DD
class LOOPS_INDEX_JOIN : public PHYS_OP
{
public:
	//##ModelId=3B0C086F0009
    int * 	lattrs;				// left attr's that are the same
	//##ModelId=3B0C086F0013
	int * 	rattrs;				// right attr's that are the same
	//##ModelId=3B0C086F001D
	int		size;				// the number of the attrs
	//##ModelId=3B0C086F0027
	int		CollId;				// collection id accessed thru index
	
public :
	
	//##ModelId=3B0C086F0031
    LOOPS_INDEX_JOIN(int *lattrs,  int *rattrs, int size, int CollId);
	//##ModelId=3B0C086F0045
	LOOPS_INDEX_JOIN( LOOPS_INDEX_JOIN& Op);
	
	//##ModelId=3B0C086F004F
	inline OP * Clone() { return new LOOPS_INDEX_JOIN(*this); };
	
	//##ModelId=3B0C086F0050
	~LOOPS_INDEX_JOIN() 
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_INDEX_JOIN].Delete();
		delete [] lattrs;
		delete [] rattrs;
	};
	
	//##ModelId=3B0C086F0059
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C086F0064
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C086F0077
	inline int GetArity() {return(1);};
	//##ModelId=3B0C086F0078
    inline CString GetName() {return ("LOOPS_INDEX_JOIN"); };  
	
	//##ModelId=3B0C086F0081
	CString Dump();
}; // LOOPS_INDEX_JOIN

/*
   Merge join
   =================
   A Merge eqjoin
*/

//##ModelId=3B0C086F010D
class MERGE_JOIN : public PHYS_OP
{
	
public:
	//##ModelId=3B0C086F0118
    int * 	lattrs;				// left attr's that are the same
	//##ModelId=3B0C086F012B
	int * 	rattrs;				// right attr's that are the same
	//##ModelId=3B0C086F0135
	int		size;				// the number of the attrs
	
public :
	
	//##ModelId=3B0C086F013F
    MERGE_JOIN(int *lattrs,  int *rattrs, int size);
	
	//##ModelId=3B0C086F014B
	MERGE_JOIN( MERGE_JOIN& Op);
	
	//##ModelId=3B0C086F0154
	inline OP * Clone() { return new MERGE_JOIN(*this); };
	
	//##ModelId=3B0C086F015D
	~MERGE_JOIN() 
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_MERGE_JOIN].Delete();
		delete [] lattrs;
		delete [] rattrs;
	};
	
	//##ModelId=3B0C086F0167
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C086F0171
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C086F0185
	inline int GetArity() {return(2);};
	//##ModelId=3B0C086F0186
    inline CString GetName() {return ("MERGE_JOIN"); };  
	//##ModelId=3B0C086F018F
	PHYS_PROP * FindPhysProp(PHYS_PROP ** input_phys_props);
	//##ModelId=3B0C086F0199
	CString Dump();
}; // MERGE_JOIN

//Does not require its inputs to be hashed
//##ModelId=3B0C086F0225
class HASH_JOIN : public PHYS_OP
{
	
public :
    int *lattrs, *rattrs;   // left, right attr's to be joined
	//##ModelId=3B0C086F0230
	int	size;				// the number of attrs
	
	//##ModelId=3B0C086F0239
    HASH_JOIN(int *lattrs, int *rattrs, int size);
	//##ModelId=3B0C086F024D
	HASH_JOIN( HASH_JOIN& Op);
	
	//##ModelId=3B0C086F024F
	inline OP * Clone() { return new HASH_JOIN(*this); };
	
	//##ModelId=3B0C086F0257
	~HASH_JOIN() 
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HASH_JOIN].Delete();
		delete [] lattrs;
		delete [] rattrs;
	};
	
	//##ModelId=3B0C086F0261
	COST * FindLocalCost ( LOG_PROP * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C086F026B
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C086F0278
	inline int GetArity() {return(2);};
	//##ModelId=3B0C086F0280
    inline CString GetName() {return ("HASH_JOIN"); };  
	//##ModelId=3B0C086F028A
	CString Dump();
	
}; // HASH_JOIN
//##ModelId=3B0C086F0320
class P_PROJECT : public PHYS_OP
{
	
public :
	
	//##ModelId=3B0C086F0334
    int * attrs;	// attr's to project on
	//##ModelId=3B0C086F033E
	int size;
	
public :
	
	//##ModelId=3B0C086F0348
    P_PROJECT(int *attrs,int size);
	//##ModelId=3B0C086F0352
	P_PROJECT( P_PROJECT& Op);
	
	//##ModelId=3B0C086F035C
	inline OP * Clone() { return new P_PROJECT(*this); };
	
	//##ModelId=3B0C086F0366
	~P_PROJECT() 
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_P_PROJECT].Delete();
		delete [] attrs; 
	};
	
	//##ModelId=3B0C086F0367
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C086F0372
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C086F0384
	inline int GetArity() {return(1);};
	//##ModelId=3B0C086F038E
    inline CString GetName() {return ("P_PROJECT"); };  
	
	//##ModelId=3B0C086F038F
	CString Dump();
	
}; // P_PROJECT

/*
   Filter
   ======
*/

//##ModelId=3B0C08700028
class FILTER : public PHYS_OP
{
	
public :
	
	//##ModelId=3B0C08700033
    FILTER() {if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_FILTER].New();};
	//##ModelId=3B0C0870003C
	FILTER( FILTER& Op) {if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_FILTER].New();};
	//##ModelId=3B0C0870003E
	~FILTER() {if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_FILTER].Delete();};
	
	//##ModelId=3B0C08700046
	inline OP * Clone() { return new FILTER(*this); };
	
	//##ModelId=3B0C08700050
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C0870005C
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C0870006E
	inline int GetArity() {return(2);};
	//##ModelId=3B0C08700078
    inline CString GetName() {return ("FILTER"); };  
	
	//##ModelId=3B0C08700079
	CString Dump() { return GetName(); };
	
}; // FILTER

/*
   Sorting
   =======
*/

//##ModelId=3B0C08700122
class QSORT : public PHYS_OP
{
public :
	
	//##ModelId=3B0C08700136
    QSORT();
	
	//##ModelId=3B0C08700140
	QSORT( QSORT& Op);
	
	//##ModelId=3B0C08700142
	inline OP * Clone() { return new QSORT(*this); };
	
	//##ModelId=3B0C0870014B
	~QSORT() 
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_QSORT].Delete();	};
	
	//##ModelId=3B0C0870014C
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C0870015F
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C0870016B
	inline int GetArity() {return(1);};
	//##ModelId=3B0C08700173
    inline CString GetName() {return ("QSORT"); };  
	
	//##ModelId=3B0C0870017D
	CString Dump();
}; // QSORT

//##ModelId=3B0C08700213
class HASH_DUPLICATES : public PHYS_OP
{
public :
	
	//##ModelId=3B0C08700227
    HASH_DUPLICATES(){if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HASH_DUPLICATES].New();};
	//##ModelId=3B0C08700228
	HASH_DUPLICATES( HASH_DUPLICATES& Op){if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HASH_DUPLICATES].New();};
	
	//##ModelId=3B0C08700231
	inline OP * Clone() { return new HASH_DUPLICATES(*this); };
	
	//##ModelId=3B0C08700232
	~HASH_DUPLICATES() 
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HASH_DUPLICATES].Delete();
	};
	
	//##ModelId=3B0C0870023B
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C08700245
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C08700251
	inline int GetArity() {return(1);};
	//##ModelId=3B0C08700259
    inline CString GetName() {return ("HASH_DUPLICATES"); };  
	
	//##ModelId=3B0C08700263
	CString Dump();
	
}; // HASH_DUPLICATES

//Hash-based AGG_LIST
//##ModelId=3B0C0870033F
class HGROUP_LIST : public PHYS_OP
{
public :
	//##ModelId=3B0C08700354
	AGG_OP_ARRAY * AggOps;
	//##ModelId=3B0C0870035D
	int * GbyAtts;
	//##ModelId=3B0C08700371
	int GbySize;
	
public :
	
	//##ModelId=3B0C0870037B
    HGROUP_LIST(int * gby_atts, int gby_size, AGG_OP_ARRAY * agg_ops)
		:GbyAtts(gby_atts), GbySize(gby_size), AggOps(agg_ops)
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HGROUP_LIST].New();};
	
	//##ModelId=3B0C08700387
	HGROUP_LIST( HGROUP_LIST& Op)
		:GbyAtts(CopyArray(Op.GbyAtts, Op.GbySize)), GbySize(Op.GbySize)
	{	AggOps = new AGG_OP_ARRAY;
	AggOps->SetSize(Op.AggOps->GetSize());
	for (int i=0; i<Op.AggOps->GetSize(); i++)
	{
		(*AggOps)[i] = new AGG_OP( *(*Op.AggOps)[i]);
	}
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HGROUP_LIST].New();};
	
	//##ModelId=3B0C08700390
	inline OP * Clone() { return new HGROUP_LIST(*this); };
	
	//##ModelId=3B0C08700399
	~HGROUP_LIST() 
	{	
		for (int i=0; i<AggOps->GetSize(); i++) delete (*AggOps)[i];
		delete AggOps;
		delete [] GbyAtts;
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HGROUP_LIST].Delete();
	};
	
	//##ModelId=3B0C087003A3
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C087003AD
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C087003C1
	inline int GetArity() {return(1);};
	//##ModelId=3B0C087003CB
    inline CString GetName() {return ("HGROUP_LIST"); };  
	//##ModelId=3B0C087003CC
	PHYS_PROP * FindPhysProp(PHYS_PROP ** input_phys_props);
	//##ModelId=3B0C087003D6
	CString Dump();
	
}; // HASH_DUPLICATES

//##ModelId=3B0C087100CA
class P_FUNC_OP : public PHYS_OP
{
public :
	//##ModelId=3B0C087100DE
	CString RangeVar;
	//##ModelId=3B0C087100E8
	int * Atts;
	//##ModelId=3B0C087100F2
	int AttsSize;
	
	//##ModelId=3B0C087100FC
    P_FUNC_OP(CString range_var, int * atts, int size)
		:RangeVar(range_var), Atts(atts), AttsSize(size)
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_P_FUNC_OP].New(); };
	
	//##ModelId=3B0C08710109
	P_FUNC_OP( P_FUNC_OP& Op)
		:RangeVar(Op.RangeVar), Atts(CopyArray(Op.Atts, Op.AttsSize)), AttsSize(Op.AttsSize)
	{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_P_FUNC_OP].New();};
	
	//##ModelId=3B0C08710111
	inline OP * Clone() { return new P_FUNC_OP(*this); };
	
	//##ModelId=3B0C0871011A
	~P_FUNC_OP() 
	{	
		delete [] Atts;
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_P_FUNC_OP].Delete();
	};
	
	//##ModelId=3B0C0871011B
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C08710125
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C08710138
	inline int GetArity() {return(1);};
	//##ModelId=3B0C08710139
    inline CString GetName() {return ("P_FUNC_OP"); };  
	
	//##ModelId=3B0C08710142
	CString Dump();
	
}; // P_FUNC_OP

/*
   Bit index join    This is a semi-join
   =======================
*/

//##ModelId=3B0C08710278
class BIT_JOIN : public PHYS_OP
{
public:
	//##ModelId=3B0C0871028C
    int * 	lattrs;				// left attr's that are the same
	//##ModelId=3B0C08710296
	int * 	rattrs;				// right attr's that are the same
	//##ModelId=3B0C087102A0
	int		size;				// the number of the attrs
	//##ModelId=3B0C087102AA
	int		CollId;				// collection id accessed thru BIT index
	
public :
	
	//##ModelId=3B0C087102B4
    BIT_JOIN(int *lattrs,  int *rattrs, int size, int CollId);
	//##ModelId=3B0C087102C1
	BIT_JOIN( BIT_JOIN& Op);
	
	//##ModelId=3B0C087102C9
	inline OP * Clone() { return new BIT_JOIN(*this); };
	
	//##ModelId=3B0C087102D3
	~BIT_JOIN() 
	{	
		if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_BIT_JOIN].Delete();
		delete [] lattrs;
		delete [] rattrs;
	};
	
	//##ModelId=3B0C087102D4
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C087102DF
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C087102F1
	PHYS_PROP * FindPhysProp(PHYS_PROP ** input_phys_props);
	
	//##ModelId=3B0C087102F3
	inline int GetArity() {return(2);};
	//##ModelId=3B0C087102FB
    inline CString GetName() {return ("BIT_JOIN"); };  
	
	//##ModelId=3B0C08710305
	CString Dump();
}; // BIT_JOIN

/*
   Indexed_Filter
   ======
*/

//##ModelId=3B0C0872002B
class INDEXED_FILTER : public PHYS_OP
{
	
public :
	
	//##ModelId=3B0C08720036
    INDEXED_FILTER(const int fileId);
	//##ModelId=3B0C0872003F
	INDEXED_FILTER( INDEXED_FILTER& Op);
	//##ModelId=3B0C08720049
	~INDEXED_FILTER() {if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_INDEXED_FILTER].Delete();};
	
	//##ModelId=3B0C0872004A
	inline OP * Clone() { return new INDEXED_FILTER(*this); };
	
	//##ModelId=3B0C08720053
	COST * FindLocalCost (
		LOG_PROP        * LocalLogProp, // uses primarily the card of the Group
		LOG_PROP ** InputLogProp); //uses primarily cardinalities
	
	//##ModelId=3B0C0872005D
	PHYS_PROP * InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
		int InputNo, bool & possible);
	
	//##ModelId=3B0C08720068
	inline int GetArity() {return(1);};
	//##ModelId=3B0C08720071
    inline CString GetName() {return ("INDEXED_FILTER"); };  
	//##ModelId=3B0C08720072
	inline int GetFileId() { return FileId; };	
	
	//##ModelId=3B0C0872007B
	CString Dump()
	{	CString os;
	os.Format("%s%s%s%s",GetName(),"(",GetCollName(FileId),")");
	return os;
	};
	
private:
	//##ModelId=3B0C0872007C
    int FileId;
	//##ModelId=3B0C0872008F
    CString 	RangeVar;
	
}; // INDEXED_FILTER

#endif //PHYSOP_H
