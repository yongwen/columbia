/* 
CM.H - cost model.  Information about the cost of many basic operations.
Does not include cost formulas, which are in physop.cpp as FindLocalCost()
$Revision: 3 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/
#ifndef CM_H
#define CM_H

#include "stdafx.h"

class CM;

/*
============================================================
Cost Model - class CM
============================================================
*/

//##ModelId=3B0C087700AA
class CM
{
private:
	//##ModelId=3B0C087700B4
	double CPU_READ;
	//##ModelId=3B0C087700C8
	double TOUCH_COPY;
	//##ModelId=3B0C087700D2
	double CPU_PRED;
	//##ModelId=3B0C087700E6
	double CPU_APPLY;
	//##ModelId=3B0C087700F0
	double CPU_COMP_MOVE;
	//##ModelId=3B0C087700FA
	double HASH_COST;
	//##ModelId=3B0C0877010E
	double HASH_PROBE;
	//##ModelId=3B0C08770119
	double INDEX_PROBE;
	//##ModelId=3B0C08770123
	double BF;
	//##ModelId=3B0C08770137
	double INDEX_BF;
	//##ModelId=3B0C0877014B
	double IO;
	//##ModelId=3B0C08770155
	double BIT_BF;
	
public:
	
	//##ModelId=3B0C0877015F
	CM(CString filename);	//read information into cost model from some default file
	
	//##ModelId=3B0C0877016A
	CString Dump(); //for debugging
	
	// Cost support functions
	
	// cpu cost of reading one block from the disk
	//##ModelId=3B0C08770173
	inline double cpu_read() { return CPU_READ; } 
	
	//cpu cost of copying one tuple to the next operator
	//##ModelId=3B0C08770174
	inline double touch_copy() { return TOUCH_COPY; }
	
	// cpu cost of evaluating one predicate
	//##ModelId=3B0C08770187
	inline double cpu_pred() { return CPU_PRED; }
	
	// cpu cost of applying function on one attribute
	//##ModelId=3B0C08770191
	inline double cpu_apply() { return CPU_APPLY; }
	
	// cpu cost of comparing and moving tuples
	//##ModelId=3B0C0877019B
	inline double cpu_comp_move() { return CPU_COMP_MOVE; }
	
	// cpu cost of building hash table
	//##ModelId=3B0C0877019C
	inline double hash_cost() {return HASH_COST; }
	
	// cpu cost of finding hash bucket
	//##ModelId=3B0C087701A5
	inline double hash_probe() { return HASH_PROBE; }
	
	// cpu cost of finding index
	//##ModelId=3B0C087701AF
	inline double index_probe() { return INDEX_PROBE; }
	
	// block factor of table file
	//##ModelId=3B0C087701B9
	inline double bf() { return BF ; }
	
	// block factor of index file
	//##ModelId=3B0C087701BA
	inline double index_bf() { return INDEX_BF; }
	
	// i/o cost of reading one block
	//##ModelId=3B0C087701C3
	inline double io() { return IO; }
	
	// block factor of bit index
	//##ModelId=3B0C087701CD
	inline double bit_bf() { return BIT_BF; }
	
};

#endif CM_H
