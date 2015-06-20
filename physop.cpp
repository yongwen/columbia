/*	
physop.cpp -  implementation of classes of physical operators
as defined in physop.h

	$Revision: 6 $
	Columbia Optimizer Framework

	A Joint Research Project of Portland State University 
	   and the Oregon Graduate Institute
	Directed by Leonard Shapiro and David Maier
	Supported by NSF Grants IRI-9610013 and IRI-9619977

	
*/

#include "stdafx.h"

#include "physop.h"
#include "cat.h"
#include "cm.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

/* need for group pruning, calculate the TouchCopy cost of the expr
TouchCopyCost = 
TouchCopy() * |G| +     //From top join
TouchCopy() * sum(cucard(Ai) i = 2, ..., n-1) +  // from other, nontop,
*/                                                        // joins

double TouchCopyCost(LOG_COLL_PROP * LogProp)
{
	double Total;
	
	if(LogProp->Card==-1)	Total = 0 ;		// Card is unknown
	else		Total = LogProp->Card ;		// from top join
	
	// from A2 -- An-1 , means excluding the min and max cucard(i)
	double Min= 3.4E+38; 
	double Max=0;
	for(int i=0; i < LogProp->Schema->GetTableNum(); i++)	
	{	
		float CuCard = LogProp->Schema->GetTableMaxCuCard(i);
		if( Min > CuCard ) Min = CuCard;
		if( Max < CuCard ) Max = CuCard;
		Total += CuCard ;
	}
	
	// exclude min and max
	Total -= Min;
	Total -= Max;
	
	return ( Total * Cm->touch_copy() );
}; 

/* for cucard pruning, calculate the minimun cost of fetching cucard tuples from disc
	FetchingCost = 
        Fetch() * sum(cucard(Ai) i = 1, ..., n) // from leaf fetches
	For each Ai which has no index on a join (interesting) order, replace
	cucard(Ai) with |Ai| and it is still a lower bound.
*/
double FetchingCost(LOG_COLL_PROP * LogProp)
{
	double Total = 0;
	
	for(int i=0; i < LogProp->Schema->GetTableNum(); i++)
	{	
		float CuCard = LogProp->Schema->GetTableMaxCuCard(i);
		float Width = LogProp->Schema->GetTableWidth(i);
		Total += ceil( CuCard * Width ) * 
			(  Cm->cpu_read() +           // cpu cost of reading from disk
			Cm->io() );				   // i/o cost of reading from disk
	}
	
	return Total;
}

/*********  FILE_SCAN implementation **************/

FILE_SCAN :: FILE_SCAN (const int fileId)
:FileId(fileId) 	
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_FILE_SCAN].New();
#ifdef _DEBUG
	name = GetName() + GetCollName(FileId);
#endif
};

//##ModelId=3B0C086E0153
FILE_SCAN::FILE_SCAN ( FILE_SCAN& Op)
:FileId(Op.GetFileId())
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_FILE_SCAN].New();
#ifdef _DEBUG
	name = Op.name;
#endif
}

// get the physical prop according to the order of the collection
//##ModelId=3B0C086E017A
PHYS_PROP * FILE_SCAN::FindPhysProp(PHYS_PROP ** input_phys_props)
{
	COLL_PROP * CollProp = Cat->GetCollProp(FileId); 
	
	if (CollProp->Order==any)
	{
		return new PHYS_PROP(any);
	}
	else
	{	PHYS_PROP * phys_prop = new PHYS_PROP(new KEYS_SET(*(CollProp->Keys)), CollProp->Order); 
	if (CollProp->Order == sorted)
		for (int i=0; i<CollProp->KeyOrder.GetSize(); i++)
			phys_prop->KeyOrder.Add(CollProp->KeyOrder[i]);
		return phys_prop;
	}
	
	//else return NULL;
}

//##ModelId=3B0C086E0170
COST * FILE_SCAN::FindLocalCost (
								 LOG_PROP    * LocalLogProp,
								 LOG_PROP ** InputLogProp)
{
	
	float Card = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	float Width = ((LOG_COLL_PROP *) LocalLogProp)->Schema->GetTableWidth(0);
	COST * Result = new COST ( 
		ceil(Card * Width) * 
		( Cm->cpu_read() +              // cpu cost of reading from disk
		Cm->io() )				    // i/o cost of reading from disk
		);
	return (Result);
}

/************ LOOPS_JOIN FUNCTIONS ****************/

//##ModelId=3B0C086E0242
LOOPS_JOIN::LOOPS_JOIN(int * lattrs, int * rattrs, int size)
:lattrs(lattrs),rattrs(rattrs),size(size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // LOOPS_JOIN::LOOPS_JOIN

//##ModelId=3B0C086E0256
LOOPS_JOIN::LOOPS_JOIN( LOOPS_JOIN& Op)
:lattrs( CopyArray(Op.lattrs,Op.size) ), 
rattrs( CopyArray(Op.rattrs,Op.size) ), 
size(Op.size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
};

#pragma optimize("", off )	// turn of code optimization due to error result
//##ModelId=3B0C086E026B
COST * LOOPS_JOIN::FindLocalCost (
								  LOG_PROP    * LocalLogProp, 
								  LOG_PROP ** InputLogProp)
{ 
    float LeftCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
    float RightCard = ((LOG_COLL_PROP *) InputLogProp[1]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    COST * result = new COST (
        LeftCard * RightCard * Cm->cpu_pred()     // cpu cost of predicates
		+ OutputCard * Cm->touch_copy()           // cpu cost of copying result
											   	  // no i/o cost
		);
	
    return ( result );
} //FindLocalCost
#pragma optimize("", on )

// requirement is: left input must have the same props as the output,
//					while right input has no required prop
//##ModelId=3B0C086E0276
PHYS_PROP * LOOPS_JOIN::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									  int InputNo, bool & possible)
{
	if(PhysProp->GetOrder()!=any)		// check possibility: satisfied output prop
	{
		if(InputNo==0)	// left input's schema should include the prop keys
		{
			if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
			{
				possible = false;
				return NULL;
			}
		}
	}
	
	possible = true;
	
	
	if(InputNo==0)		// pass the prop to left input
		return new PHYS_PROP(*PhysProp);
	else				// no reqd prop for right input
		return new PHYS_PROP(any);
}

//##ModelId=3B0C086E029C
CString LOOPS_JOIN::Dump()
{	
	CString os;
    CString temp;
    int i;
	
	os = GetName() + "(<";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s",GetAttName(lattrs[i]),",");
		os += temp;
	}
	
	if( size > 0 ) 
		temp.Format("%s%s%s",GetAttName(lattrs[i]),
		">," , "<");
	else
		temp.Format("%s%s", ">," , "<");
	
	os += temp;
	
	for (i=0; (size > 0) && (i< size-1) ; i++) 
	{
		temp.Format("%s%s", GetAttName(rattrs[i]),",");
		os += temp;
	}
	
	if(size>0) 
		temp.Format("%s%s",GetAttName(rattrs[i]), ">)");
	else 
		temp.Format("%s",">)");
	
	os += temp;
	
	return os;
	
};

/************ PDUMMY FUNCTIONS ****************/

//##ModelId=3B0C086E031F
PDUMMY::PDUMMY()
{
#ifdef _DEBUG
	name = GetName();
#endif
} // PDUMMY::PDUMMY

//##ModelId=3B0C086E0328
PDUMMY::PDUMMY( PDUMMY& Op)
{
#ifdef _DEBUG
	name = GetName();
#endif
};

//Imitate LOOPS_JOIN - why not?
//##ModelId=3B0C086E0346
COST * PDUMMY::FindLocalCost (
							  LOG_PROP *  LocalLogProp, 
							  LOG_PROP ** InputLogProp)
{ 
    float LeftCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
    float RightCard = ((LOG_COLL_PROP *) InputLogProp[1]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    COST * result = new COST (
        LeftCard * RightCard * Cm->cpu_pred()     // cpu cost of predicates
		+ OutputCard * Cm->touch_copy()           // cpu cost of copying result
												  // no i/o cost
		);
	
    return ( result );
} //FindLocalCost

// requirement is: left input must have the same props as the output,
//					while right input has no required prop
//##ModelId=3B0C086E0350
PHYS_PROP * PDUMMY::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
								  int InputNo, bool & possible)
{
	if(PhysProp->GetOrder()!=any)		// check possibility: satisfied output prop
	{
		if(InputNo==0)	// left input's schema should include the prop keys
		{
			if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
			{
				possible = false;
				return NULL;
			}
		}
	}
	
	possible = true;
	
	
	if(InputNo==0)		// pass the prop to left input
		return new PHYS_PROP(*PhysProp);
	else				// no reqd prop for right input
		return new PHYS_PROP(any);
}

//##ModelId=3B0C086E036E
CString PDUMMY::Dump()
{	
	CString os = GetName();
	
	return os;
	
};

/*
Nested loops index join
=======================
*/

//##ModelId=3B0C086F0031
LOOPS_INDEX_JOIN::LOOPS_INDEX_JOIN(int * lattrs, int * rattrs, int size, int CollId)
:lattrs(lattrs),rattrs(rattrs),size(size),CollId(CollId)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_INDEX_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // LOOPS_INDEX_JOIN::LOOPS_INDEX_JOIN

//##ModelId=3B0C086F0045
LOOPS_INDEX_JOIN::LOOPS_INDEX_JOIN( LOOPS_INDEX_JOIN& Op)
:lattrs( CopyArray(Op.lattrs,Op.size) ), 
rattrs( CopyArray(Op.rattrs,Op.size) ), 
size(Op.size),CollId(Op.CollId)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_INDEX_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
};


//##ModelId=3B0C086F0059
COST * LOOPS_INDEX_JOIN::FindLocalCost (
										LOG_PROP *  LocalLogProp, 
										LOG_PROP ** InputLogProp)
{ 
    float LeftCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
    float RightCard = Cat->GetCollProp(CollId) -> Card;
	float RightWidth = Cat->GetCollProp(CollId)->Width;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    COST * result = new COST (
		LeftCard * Cm->index_probe()					// cpu cost of finding index
        + OutputCard									// number of result tuples
		* (2 * Cm->cpu_read()							// cpu cost of reading right index and result
		+ Cm->touch_copy() )						// cpu cost of copying left result
        + MIN (LeftCard, ceil(RightCard / Cm->index_bf()) )	// number of index blocks
		* Cm->io()										// i/o cost of reading right index
        + MIN (OutputCard, ceil(RightCard * RightWidth) )	// number of result blocks
		* Cm->io()										// i/o cost of reading right result
		);
	
    return ( result );
} //FindLocalCost

//##ModelId=3B0C086F0064
PHYS_PROP * LOOPS_INDEX_JOIN::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
											int InputNo, bool & possible)
{
	assert(InputNo==0);		// only one input
	
	if(PhysProp->GetOrder() != any)		// check possibility: satisfied output prop
	{
		if(InputNo==0)	// left input's schema should include the prop keys
		{
			if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
			{
				possible = false;
				return NULL;
			}
		}
		
	}
	
	possible = true;
	
	// pass the prop to inputs
	return new PHYS_PROP(*PhysProp);
}

//##ModelId=3B0C086F0081
CString LOOPS_INDEX_JOIN::Dump()
{	
	CString os;
    CString temp;
    int i;
	
	os = GetName() + "(<";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s",GetAttName(lattrs[i]),",");
		os += temp;
	}
	
	if( size > 0 ) 
		temp.Format("%s%s%s",GetAttName(lattrs[i]),
		">," , "<");
	else
		temp.Format("%s%s", ">," , "<");
	
	os += temp;
	
	for (i=0; (size > 0) && (i< size-1) ; i++) 
	{
		temp.Format("%s%s", GetAttName(rattrs[i]),",");
		os += temp;
	}
	
	if(size>0) 
		temp.Format("%s%s",GetAttName(rattrs[i]), ">)");
	else 
		temp.Format("%s",">)");
	
	os += temp;
	
	temp.Format("Index on %s", GetCollName(CollId) );
	os += temp;
	
	return os;
	
};

/*
Merge join
==========
*/

//##ModelId=3B0C086F013F
MERGE_JOIN::MERGE_JOIN(int * lattrs, int * rattrs, int size)
:lattrs(lattrs),rattrs(rattrs),size(size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_MERGE_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // MERGE_JOIN::MERGE_JOIN

//##ModelId=3B0C086F014B
MERGE_JOIN::MERGE_JOIN( MERGE_JOIN& Op)
:lattrs( CopyArray(Op.lattrs,Op.size) ), 
rattrs( CopyArray(Op.rattrs,Op.size) ), 
size(Op.size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_MERGE_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
};

//##ModelId=3B0C086F0167
COST * MERGE_JOIN::FindLocalCost (
								  LOG_PROP *  LocalLogProp, 
								  LOG_PROP ** InputLogProp)
{ 
    float LeftCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
    float RightCard = ((LOG_COLL_PROP *) InputLogProp[1]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    COST * result = new COST (
        (LeftCard + RightCard) * Cm->cpu_pred() // cpu cost of predicates
		+ OutputCard * Cm->touch_copy()         // cpu cost of copying result
											    // no i/o cost
		);
	
    return ( result );
} //FindLocalCost

//##ModelId=3B0C086F0199
CString MERGE_JOIN::Dump()
{	
	CString os;
    CString temp;
    int i;
	
	os = GetName() + "(<";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s",GetAttName(lattrs[i]),",");
		os += temp;
	}
	
	if( size > 0 ) 
		temp.Format("%s%s%s",GetAttName(lattrs[i]),
		">," , "<");
	else
		temp.Format("%s%s", ">," , "<");
	
	os += temp;
	
	for (i=0; (size > 0) && (i< size-1) ; i++) 
	{
		temp.Format("%s%s", GetAttName(rattrs[i]),",");
		os += temp;
	}
	
	if(size>0) 
		temp.Format("%s%s",GetAttName(rattrs[i]), ">)");
	else 
		temp.Format("%s",">)");
	
	os += temp;
	
	return os;
	
};

// inputs must be sorted
//##ModelId=3B0C086F0171
PHYS_PROP * MERGE_JOIN::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									  int InputNo, bool & possible)
{
	if(PhysProp->GetOrder() != any)			// If specific output property is required
	{
		if( !PhysProp->Keys->Equal(lattrs, size) &&
			!PhysProp->Keys->Equal(rattrs, size) ) 
		{
			possible = false;
			return NULL;
		}
	}
	
	possible = true;
	
	KEYS_SET *Keys;
	if(InputNo==0)  Keys = new KEYS_SET(lattrs, size);
	if(InputNo==1)  Keys = new KEYS_SET(rattrs, size);
	
	PHYS_PROP * result = new PHYS_PROP(Keys, sorted );	// require sort on input
	//the KeyOrder is ascending for all keys
	for (int i=0; i<size; i++) result->KeyOrder.Add(ascending);
	
	return result;
	
}// MERGE_JOIN::InputReqdProp

// the physprop of the output is sorted on lattrs, rattrs, in the order of 
// attrs of the EQJOIN operator
//##ModelId=3B0C086F018F
PHYS_PROP * MERGE_JOIN::FindPhysProp(PHYS_PROP ** input_phys_props)
{
	KEYS_SET *Keys, *Keys2;
	Keys = new KEYS_SET(lattrs, size);
	Keys2 = new KEYS_SET(rattrs, size);
	Keys->Merge(* Keys2);
	
	PHYS_PROP * result = new PHYS_PROP( Keys, sorted );	//sorted on lattrs, and rattrs
	//the KeyOrder is ascending for all keys
	for (int i=0; i<size+size; i++) result->KeyOrder.Add(ascending);
	
	return result;
}//MERGE_JOIN::FindPhysProp

 /*
 HASH join
 =========
	Like Merge join, but inputs can have any property.  Operator constructs a hash table.
*/

//##ModelId=3B0C086F0239
HASH_JOIN::HASH_JOIN(int * lattrs, int * rattrs, int size)
:lattrs(lattrs),rattrs(rattrs),size(size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HASH_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // HASH_JOIN::HASH_JOIN

//##ModelId=3B0C086F024D
HASH_JOIN::HASH_JOIN( HASH_JOIN& Op)
:lattrs( CopyArray(Op.lattrs,Op.size) ), 
rattrs( CopyArray(Op.rattrs,Op.size) ), 
size(Op.size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_HASH_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
};

//##ModelId=3B0C086F0261
COST * HASH_JOIN::FindLocalCost (
								 LOG_PROP *  LocalLogProp, 
								 LOG_PROP ** InputLogProp)
{ 
    float LeftCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
    float RightCard = ((LOG_COLL_PROP *) InputLogProp[1]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    COST * result = new COST (
        RightCard * Cm->hash_cost()          // cpu cost of building hash table
		+ LeftCard * Cm->hash_probe()      // cpu cost of finding hash bucket
		+ OutputCard * Cm->touch_copy()    // cpu cost of copying result
		);									 // no i/o cost
	
    return ( result );
} //FindLocalCost

//##ModelId=3B0C086F028A
CString HASH_JOIN::Dump()
{	
	CString os;
    CString temp;
    int i;
	
	os = GetName() + "(<";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s",GetAttName(lattrs[i]),",");
		os += temp;
	}
	
	if( size > 0 ) 
		temp.Format("%s%s%s",GetAttName(lattrs[i]),
		">," , "<");
	else
		temp.Format("%s%s", ">," , "<");
	
	os += temp;
	
	for (i=0; (size > 0) && (i< size-1) ; i++) 
	{
		temp.Format("%s%s", GetAttName(rattrs[i]),",");
		os += temp;
	}
	
	if(size>0) 
		temp.Format("%s%s",GetAttName(rattrs[i]), ">)");
	else 
		temp.Format("%s",">)");
	
	os += temp;
	
	return os;
	
};

// No properties are required of inputs 
//##ModelId=3B0C086F026B
PHYS_PROP * HASH_JOIN::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									 int InputNo, bool & possible)
{
	if(PhysProp->GetOrder()!=any)		// check possibility: satisfied output prop
	{
		if(InputNo==0)	// left input's schema should include the prop keys
		{
			if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
			{
				possible = false;
				return NULL;
			}
		}
	}
	
	possible = true;
	
	
	if(InputNo==0)		// pass the prop to left input
		return new PHYS_PROP(*PhysProp);
	else				// no reqd prop for right input
		return new PHYS_PROP(any);
	
}//HASH_JOIN::InputReqdProp


 /*
 Filter
 ======
*/
//##ModelId=3B0C08700050
COST * FILTER::FindLocalCost (
							  LOG_PROP *  LocalLogProp, 
							  LOG_PROP ** InputLogProp)
{ 
    float InputCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
	// Need to have a cost for 0 tuples case	+ 1 ?? 
    COST * result = new COST (
		InputCard * Cm->cpu_pred()		  // cpu cost of predicates
		+ OutputCard * Cm->touch_copy()   // cpu cost of copying result
										  // no i/o cost
		);
	
    return ( result );
} //FindLocalCost

//##ModelId=3B0C0870005C
PHYS_PROP * FILTER::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
								  int InputNo, bool & possible)
{
	if(InputNo==1)	 // right input is Item Group, no reqd prop for it
	{
		possible = true;
		return (new PHYS_PROP(any));
		//return NULL;
	}
	
	// else, for left input
	if(PhysProp->GetOrder()!=any)		// check possibility: satisfied output prop
	{
		if(InputNo==0)	// left input's schema should include the prop keys
		{
			if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
			{
				possible = false;
				return NULL;
			}
		}		
	}
	
	possible = true;
	
	// pass the prop to inputs
	return (new PHYS_PROP(*PhysProp));
}


//##ModelId=3B0C086F0348
P_PROJECT::P_PROJECT(int * attrs, int size)
:attrs(attrs),size(size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_P_PROJECT].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // P_PROJECT::P_PROJECT

//##ModelId=3B0C086F0352
P_PROJECT::P_PROJECT( P_PROJECT& Op)
:attrs( CopyArray(Op.attrs,Op.size) ), 
size(Op.size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_P_PROJECT].New();
#ifdef _DEBUG
	name = GetName();
#endif
};

//##ModelId=3B0C086F0367
COST * P_PROJECT::FindLocalCost (
								 LOG_PROP *  LocalLogProp, 
								 LOG_PROP ** InputLogProp)
{ 
    float InputCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
	assert(InputCard == OutputCard);
	
	// Need to have a cost for 0 tuples case	+ 1 ?? 
    COST * result = new COST (
		InputCard * Cm->touch_copy()          // cpu cost of copying result
											  // no i/o cost
		);
	
    return ( result );
} //FindLocalCost

//##ModelId=3B0C086F0372
PHYS_PROP * P_PROJECT::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									 int InputNo, bool & possible)
{
	assert(InputNo==0);		// only one input
	
	if(PhysProp->GetOrder() != any)		// check possibility: satisfied output prop
	{
		if(InputNo==0)	// left input's schema should include the prop keys
		{
			if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
			{
				possible = false;
				return NULL;
			}
		}
		
	}
	
	possible = true;
	
	// pass the prop to inputs
	return new PHYS_PROP(*PhysProp);
}

//##ModelId=3B0C086F038F
CString P_PROJECT::Dump()
{
	CString os;
	CString temp;
	int i;
	
	os = GetName() + "(";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s", GetAttName(attrs[i]), ",");
		os += temp;
	}
	
	temp.Format("%s)", GetAttName(attrs[i]));
	os += temp;
	
	return os;
} //P_PROJECT::Dump

/*
  Sorting
  =======
*/

//##ModelId=3B0C08700136
QSORT::QSORT() 
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_QSORT].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // QSORT::QSORT

//##ModelId=3B0C08700140
QSORT::QSORT( QSORT& Op)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_QSORT].New();
#ifdef _DEBUG
	name = GetName();
#endif
};

//##ModelId=3B0C0870017D
CString QSORT::Dump()
{
	return GetName();
} //QSORT::Dump

//##ModelId=3B0C0870014C
COST * QSORT::FindLocalCost (
							 LOG_PROP *  LocalLogProp, 
							 LOG_PROP ** InputLogProp)
{ 
    float InputCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    // very very bogus
    // double card = MAX(1, input_card);	// bogus NaN error
    // double card = MAX(1, 10000 * (1/input_card));	// bogus NaN error
    float card = MAX(1, OutputCard);	
	
    COST * result = new COST ( 
		2 * card * log(card)/log(2.0)     // number of comparison and move
		* Cm->cpu_comp_move()             // cpu cost of compare and move
										  // no i/o cost
		);
	
    return ( result );
} // QSORT::FindLocalCost

//##ModelId=3B0C0870015F
PHYS_PROP * QSORT::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
								 int InputNo, bool & possible)
{
	if( PhysProp->GetOrder() == any )
	{	
		possible = false;  // enforcer does not satisfy the ANY prop
		return NULL;
	}
	
	if(PhysProp -> GetOrder() == sorted)
	{
		//check the keys
		if (((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys)) 
			possible = true;
		else possible = false;
	}
	else
		assert(false);
	
	return new PHYS_PROP(any);
	//return NULL;	// any input will result in a sorted output
	
}//QSORT::InputReqdProp

//##ModelId=3B0C0870023B
COST * HASH_DUPLICATES::FindLocalCost (
									   LOG_PROP *  LocalLogProp, 
									   LOG_PROP ** InputLogProp)
{ 
    float InputCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
	// Need to have a cost for 0 tuples case	+ 1 ?? 
    COST * result = new COST (
		InputCard * Cm->hash_cost()       //cpu cost of hashing
										  //assume hash collisions add negligible cost
		+ OutputCard * Cm->touch_copy()   // cpu cost of copying result
										  // no i/o cost
		);
	
    return ( result );
} //FindLocalCost

//##ModelId=3B0C08700245
PHYS_PROP * HASH_DUPLICATES::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
										   int InputNo, bool & possible)
{
	assert(InputNo==0);  //one input
	possible = true;
	// pass the prop to inputs
	return new PHYS_PROP(*PhysProp);
}

//##ModelId=3B0C08700263
CString HASH_DUPLICATES::Dump()
{
	return GetName();
} //HASH_DUPLICATES::Dump

// this cost does not model the cost of the average aggregate function very well
// since it actually requires more than one pass. 
// One pass to group, count and sum. and one pass to divide sum by count
//##ModelId=3B0C087003A3
COST * HGROUP_LIST::FindLocalCost (
								   LOG_PROP *  LocalLogProp, 
								   LOG_PROP ** InputLogProp)
{ 
    float InputCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    // Need to have a cost for 0 tuples case	+ 1 ?? 
    COST * result = new COST (
		InputCard *( Cm->hash_cost()	//cost of hashing
					   +Cm->cpu_apply()*(AggOps->GetSize()))//apply the aggregate operation
					   + OutputCard * (Cm->touch_copy())  //copy out the result
					   );
	
    return ( result );
} //HGROUP_LIST::FindLocalCost

//##ModelId=3B0C087003AD
PHYS_PROP * HGROUP_LIST::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									   int InputNo, bool & possible)
{
	assert(InputNo==0);  //one input
	
	possible = true;
	// pass the prop to inputs
	return new PHYS_PROP(*PhysProp);
}

//the physprop of the output is sorted on Gby attrs
//##ModelId=3B0C087003CC
PHYS_PROP * HGROUP_LIST::FindPhysProp(PHYS_PROP ** input_phys_props)
{
	KEYS_SET *Keys;
	Keys = new KEYS_SET(GbyAtts, GbySize);
	
	PHYS_PROP * result = new PHYS_PROP( Keys, sorted );	//sorted on group_by attributes
	for (int i=0; i<GbySize; i++) result->KeyOrder.Add(ascending);
	
	return result;
}//HGROUP_LIST::FindPhysProp

//##ModelId=3B0C087003D6
CString HGROUP_LIST::Dump()
{
	CString os;
	CString temp;
	int i;
	
	os = GetName() + "( Group By:";
	
	for (i=0; (i< GbySize-1); i++) 
	{
		temp.Format("%s%s", GetAttName(GbyAtts[i]), ",");
		os += temp;
	}
	
	if (GbySize>0) temp.Format("%s )", GetAttName(GbyAtts[i]));
	else temp.Format("%s", "Empty set )" );
	os += temp;
	
	//dump AggOps
	temp.Format("%s", "( Aggregating: ");
	os += temp;
	int NumOps = AggOps->GetSize();
	for (i=0; i< NumOps-1; i++)
	{
		temp = (* AggOps)[i]->Dump();
		os += temp;
	}
	if (NumOps>0) 
	{
		temp = (* AggOps)[i]->Dump();
		os += temp;
		os += " )";
	}
	else 
	{
		temp.Format("%s", "Empty set )" );
		os += temp;
	}
	
	return os;
} //HGROUP_LIST::Dump

//##ModelId=3B0C0871011B
COST * P_FUNC_OP::FindLocalCost (
								 LOG_PROP *  LocalLogProp, 
								 LOG_PROP ** InputLogProp)
{ 
	float InputCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    // Need to have a cost for 0 tuples case	+ 1 ?? 
    COST * result = new COST (
		InputCard * Cm->cpu_apply()	    //cpu cost of applying aggregate operation
		+OutputCard * Cm->touch_copy()	//copy out the result
		);
    return ( result );
} //P_FUNC_OP::FindLocalCost

//##ModelId=3B0C08710125
PHYS_PROP * P_FUNC_OP::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									 int InputNo, bool & possible)
{
	assert(InputNo==0);  //one input
	possible = true;
	// pass the prop to inputs
	return new PHYS_PROP(*PhysProp);
}

//##ModelId=3B0C08710142
CString P_FUNC_OP::Dump()
{
	CString os;
	CString temp;
	int i;
	
	os = GetName() + "(";
	
	for (i=0; (AttsSize > 0) && (i< AttsSize-1); i++) 
	{
		temp.Format("%s%s", GetAttName(Atts[i]), ",");
		os += temp;
	}
	
	temp.Format("%s)", GetAttName(Atts[i]));
	os += temp;
	
	temp.Format("%s%s", " AS ", RangeVar);
	os += temp;
	return os;
}

//##ModelId=3B0C087102B4
BIT_JOIN::BIT_JOIN(int * lattrs, int * rattrs, int size, int CollId)
:lattrs(lattrs),rattrs(rattrs),size(size),CollId(CollId)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_BIT_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
} // BIT_JOIN::BIT_JOIN

//##ModelId=3B0C087102C1
BIT_JOIN::BIT_JOIN( BIT_JOIN& Op)
:lattrs( CopyArray(Op.lattrs,Op.size) ), 
rattrs( CopyArray(Op.rattrs,Op.size) ), 
size(Op.size), CollId(Op.CollId)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_LOOPS_INDEX_JOIN].New();
#ifdef _DEBUG
	name = GetName();
#endif
};

//##ModelId=3B0C087102D4
COST * BIT_JOIN::FindLocalCost (
								LOG_PROP *  LocalLogProp, 
								LOG_PROP ** InputLogProp)
{ 
    float LeftCard =  ((LOG_COLL_PROP *) InputLogProp[0]) -> Card;
	
    float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	
    COST * result = new COST (
		LeftCard * Cm->cpu_read()					// cpu cost of reading bit vector
		+ LeftCard * Cm->cpu_pred()					// cpu cost of check bit vector
													// the above is overstated:
													//	1. The read assumes we read 1
													//	   bit at a time
													//	2. cost of evaluating a predicate
													//	   is just checking a single bit
//		+OutputCard									// number of result tuples
//		 * Cm->touch_copy()							// cpu cost of projecting and 
													// copying result
		+ (LeftCard /Cm->bit_bf())					// number of bit vector blocks
          * Cm->io()								// i/o cost of reading bit vector
    );
	
    return ( result );
} //BIT_JOIN::FindLocalCost 

//##ModelId=3B0C087102DF
PHYS_PROP * BIT_JOIN::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
									int InputNo, bool & possible)
{
	if(InputNo==1)	 // no reqd prop for right input
	{
		possible = true;
		return new PHYS_PROP(any);
	}
	
	if(PhysProp->GetOrder()!=any)			// If specific output property is required
	{
		if( ! ((LOG_COLL_PROP*)InputLogProp)->Schema->Contains(PhysProp->Keys) )
		{
			possible = false;
			return NULL;
		}
	}
	
	possible = true;
	
	return new PHYS_PROP(*PhysProp);
}

//##ModelId=3B0C08710305
CString BIT_JOIN::Dump()
{	
	CString os;
    CString temp;
    int i;
	
	os = GetName() + "(<";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s",GetAttName(lattrs[i]),",");
		os += temp;
	}
	
	if( size > 0 ) 
		temp.Format("%s%s%s",GetAttName(lattrs[i]),
		">," , "<");
	else
		temp.Format("%s%s", ">," , "<");
	
	os += temp;
	
	for (i=0; (size > 0) && (i< size-1) ; i++) 
	{
		temp.Format("%s%s", GetAttName(rattrs[i]),",");
		os += temp;
	}
	
	if(size>0) 
		temp.Format("%s%s",GetAttName(rattrs[i]), ">)");
	else 
		temp.Format("%s",">)");
	
	os += temp;
	
	temp.Format("Using bit Index on %s", GetCollName(CollId) );
	os += temp;
	
	return os;
	
};

//the physprop of the output is the left input prop
//##ModelId=3B0C087102F1
PHYS_PROP * BIT_JOIN::FindPhysProp(PHYS_PROP ** input_phys_props)
{
	return input_phys_props[0];
}//BIT_JOIN::FindPhysProp

//INDEXED_FILTER
//##ModelId=3B0C08720036
INDEXED_FILTER :: INDEXED_FILTER (const int fileId)
:FileId(fileId) 	
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_INDEXED_FILTER].New();
#ifdef _DEBUG
	name = GetName() + GetCollName(FileId);
#endif
};

//##ModelId=3B0C0872003F
INDEXED_FILTER::INDEXED_FILTER ( INDEXED_FILTER& Op)
:FileId(Op.GetFileId())
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_INDEXED_FILTER].New();
#ifdef _DEBUG
	name = Op.name;
#endif
}

//##ModelId=3B0C08720053
COST * INDEXED_FILTER::FindLocalCost (
									  LOG_PROP *  LocalLogProp,
									  LOG_PROP ** InputLogProp)
{
	float InputCard = Cat->GetCollProp(FileId)->Card;
	float Width = Cat->GetCollProp(FileId)->Width ; 
	INT_ARRAY * Indices = Cat->GetIndNames(FileId);
	
	float OutputCard = ((LOG_COLL_PROP *) LocalLogProp) -> Card;
	KEYS_SET FreeVar = ((LOG_ITEM_PROP *) InputLogProp[0])->FreeVars;
	
	double data_cost, index_cost;
	double pred_cost=0;
	
    bool Clustered = false;
	
	//for model D, redicate must be on a single attribute to use this physical operator. 
	assert(FreeVar.GetSize() == 1);
	
	for (int i=0; i<Indices->GetSize(); i++)
	{
		int IndexId = Indices->GetAt(i);
		if (IndexId == FreeVar[0] //FreeVar is contained in index list
			&& Cat->GetIndProp(IndexId)->IndType == btree // index type is btree
			&& Cat->GetIndProp(IndexId)->Clustered == true //index is clustered
			)
		{
			Clustered = true;
			break;
		}
	}
	
	if (Clustered)
	{
   /*
	* BTREE clustered
	*      (assuming a single range predicate e.g. 8 < value < 23 )
	*
	*      Algorithm:
	*      Lookup FIRST tuple in range using index.
	*      Read all tuples from that point on, evaluating predicate
	*      on each tuple, until encounter tuple where predicate
	*      (e.g. value < 23) is FALSE
    */
		// single read index
		index_cost = Cm->cpu_read() // cpu cost of reading index from disk;
			+ Cm->io(); // io cost of reading index from disk
		// read data until predicate fails
		data_cost = ceil(OutputCard * Width) * (Cm->cpu_read() //cpu cost of reading data from disk
			+ Cm->io());  // io cost of reading data from disk
		// evaluate predicate until predicate fails
		pred_cost = OutputCard*Cm->cpu_pred();
	}
	else
	{
   /*
	* Not clustered
	*
	*      Algorithm:
	*      Lookup EACH tuple in range (satisfying predicate) using index.
	*
	*      (if small number of resulting tuples)
	*      For each tuple in index:
	*          Read the block the tuple is on.
	*
	*      OR
	*
	*      (if large number of resulting tuples)
	*      Gather all pointers to resulting tuples
	*      Read each block of file, retrieve resulting tuples
	*/
		// read as many index blocks as necessary
		index_cost = ceil(OutputCard/Cm->index_bf()) * (Cm->cpu_read() + Cm->io()); 
		// read all data blocks or read data for every index entry found
		data_cost = MIN(ceil(InputCard * Width), OutputCard) * (Cm->cpu_read()
			+ Cm->io());
	}
	
	COST * Result = new COST (index_cost+data_cost+pred_cost);
	
    return (Result);
}

//##ModelId=3B0C0872005D
PHYS_PROP * INDEXED_FILTER::InputReqdProp(PHYS_PROP * PhysProp, LOG_PROP * InputLogProp, 
										  int InputNo, bool & possible)
{
	assert(InputNo==0);  //one input
	possible = true;
	// no requirement for inputs
	return new PHYS_PROP(any);
	//return( NULL );   
}
