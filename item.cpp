/*	
item.cpp -  
	$Revision: 3 $
	Implements operators on items,as defined in classes ATTR_EXP, LOG_PROP,
	CONJ_OP, typically part of a predicate, as defined in item.h
	
	  Columbia Optimizer Framework

	A Joint Research Project of Portland State University 
	   and the Oregon Graduate Institute
	Directed by Leonard Shapiro and David Maier
	Supported by NSF Grants IRI-9610013 and IRI-9619977

	
*/
 

#include "stdafx.h"
#include "item.h"

extern bool ForGlobalEpsPruning;

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

//##ModelId=3B0C08750379
ATTR_EXP::ATTR_EXP(CString range_var, int * atts, int size)
		: RangeVar(range_var), Atts(atts), AttsSize(size)
{	
	//do not add to CAT since it is not a real collection
	//add .RangeVar to AttTable, 
	//add AttCollTable[AttId]=0, since table "" id is 0
	//CollTable[0]=""
	int AttId = GetAttId( "", RangeVar);
	//we do not know the cadinalities, use (-1, -1, -1)
	AttNew = new ATTR(AttId, -1, -1, -1);
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR_EXP].New(); 
};

//##ModelId=3B0C087503C9
CString ATTR_EXP::Dump()
{	CString os;
	os.Format("%s%s",GetName()," <");
	for (int i=0; i<AttsSize-1; i++)
	{
		os.Format("%s%s", GetAttName(Atts[i]), ",");
	}
	if (AttsSize==0) os.Format("%s", "Empty set");
	else os.Format("%s", GetAttName(Atts[AttsSize-1]) );
	os.Format("%s%s%s", "as ", RangeVar, " >");
	return os;
};


//##ModelId=3B0C087502BA
LOG_PROP * ATTR_OP::FindLogProp ( LOG_PROP ** input)
{
	KEYS_SET fv;
	
	fv.AddKey(AttId);
	
	/* For an attr_cat, no idea what the stats are, so make them all -1 */
    LOG_PROP * result = new LOG_ITEM_PROP(-1,-1,-1, 0, fv);
	
    return ( result );
} // ATTR_OP::FindLogProp

 /*
  * Currently this function returns exactly the same
  * answer for ALL inputs:
  *	1	-1	-1
  * That is a unique cardinality of 1 since its a constant,
  * min and max of -1 and -1 since string comparison is
  * assumed not to be used.  (e.g. Give all animals > aardvark)
  * 11/13/95 kgb
  */
//##ModelId=3B0C0876012B
LOG_PROP * CONST_STR_OP::FindLogProp ( LOG_PROP ** input)
{
	KEYS_SET fv;
	
    LOG_PROP * result = new LOG_ITEM_PROP(-1,-1,-1, 0, fv);
	
    return ( result );
} // CONST_STR_OP::FindLogProp

//##ModelId=3B0C087602D2
LOG_PROP * COMP_OP::FindLogProp (	LOG_PROP ** input)
{
    double selectivity;
	
	LOG_ITEM_PROP * LeftProp = (LOG_ITEM_PROP *)input[0];
	LOG_ITEM_PROP * RightProp = (LOG_ITEM_PROP *)input[1];
	
    // Figure out the free variables
    KEYS_SET fv = LeftProp -> FreeVars;
	
    // OP_NOT is the only unary CONJ_OP
    if (op_code != OP_NOT) 
		fv.Merge( RightProp -> FreeVars);
    
	// for COMP_OP
	
    double attr_min, attr_max, attr_cucard, const_value;
	
    // if any of the inputs is ATTR_EXP or both of the them are ATTR_CATs, 
    //  use magic number
    if ( LeftProp->CuCard == -1 || RightProp->CuCard == -1 ||
		( LeftProp->CuCard != 1 && RightProp->CuCard !=1 )	|| 
		op_code == OP_LIKE) 
	{
		// magic numbers
		switch (op_code) 
		{
		case OP_LIKE : 
			// MINIMUM selectivity .05
			if (LeftProp->CuCard==1) 
			{
				ASSERT(LeftProp->Min == LeftProp->Max);
				attr_cucard = RightProp->CuCard;
				ASSERT(attr_cucard >= 1);
				selectivity = MAX(0.05, 1 / attr_cucard); 
			} 
			else 
				if (RightProp->CuCard==1) 
				{
					ASSERT(RightProp->Min == RightProp->Max);
					attr_cucard = LeftProp->CuCard;
					ASSERT(attr_cucard >= 1);
					selectivity = MAX(0.05, 1 / attr_cucard); 
				} 
				else 
				{
					// Neither left or right operand is constant
					selectivity = 0.05; 
				}
				break;
		case OP_EQ : 
			selectivity = 0.1; 
			break;
		case OP_NE :
			selectivity = 0.9; 
			break;
		case OP_LE:
		case OP_LT:
		case OP_GE:
		case OP_GT:
			selectivity = 0.5; 
			break;
		default:
			selectivity = 0.5; 
			break;
		};
    } 
	else 
	{
		if (LeftProp->CuCard==1) 
		{
			ASSERT(LeftProp->Min == LeftProp->Max);
			const_value = LeftProp->Min;
			attr_min = RightProp->Min;
			attr_max = RightProp->Max;
			attr_cucard = RightProp->CuCard;
		} 
		else 
		{
			ASSERT(RightProp->Min == RightProp->Max);
			const_value = RightProp->Min;
			attr_min = LeftProp->Min;
			attr_max = LeftProp->Max;
			attr_cucard = LeftProp->CuCard;
		}
		
		switch (op_code) 
		{
		case OP_IN : 
			// left or right input should be a CONST_SET_OP
			// const_val = cmin is the NUMBER OF ELEMENTS in the SET!
			// Example query 16 predicate:
			//     P_SIZE  IN ([size1], [size2], [size3], [size4], [size5], 
			//     [size6], [size7], [size8])
			// So below 8 is the number of elements in the set
			selectivity = const_value / (attr_cucard); break;
		case OP_EQ : 
			selectivity = 1 / attr_cucard; break;
		case OP_NE :
			selectivity = 1 - 1 / attr_cucard; break;
		case OP_LE:
		case OP_LT:
			selectivity = (MIN(attr_max, const_value) - 
				MIN(attr_min, const_value)) / (attr_max - attr_min);
			break;
		case OP_GE:
		case OP_GT:
			selectivity = (MAX(attr_max, const_value) - 
				MAX(attr_min, const_value)) / (attr_max - attr_min);
			break;
		};
    }
	
    // using independence assumption ... for CONJ_OP
	switch(op_code)
	{
	case OP_AND: 
		selectivity = LeftProp->Selectivity * RightProp->Selectivity; 
		break;
	case OP_OR: 
		selectivity = LeftProp->Selectivity + RightProp->Selectivity -
			LeftProp->Selectivity * RightProp->Selectivity; 
		break;
	case OP_NOT: 
		selectivity = 1 - LeftProp->Selectivity ; 
		break;
	}
	
    // Assert selectivity in range
    if (selectivity < .00001 ) 
	{
		// Warning
		OUTPUT("small selectivity = %.3f\r\n", selectivity);
	}
	
	//"Small selectivity -- shouldn't be a problem -- but is!"
    ASSERT (selectivity > .0000001); 
	
	// "Big selectivity (> 1)"
    ASSERT (selectivity < 1 );
	
    LOG_PROP * result = new LOG_ITEM_PROP (-1,-1,-1, (float)selectivity, fv);
	
    return ( result );
} // CONJ_OP::FindLogProp
