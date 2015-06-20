/* 
CAT.H - Catalog.  Info about stored data.
$Revision: 3 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/
#ifndef CAT_H
#define CAT_H

#include "stdafx.h"

class CAT;


/*
============================================================
CATALOG - class CAT
============================================================
*/

//##ModelId=3B0C0877029F
class CAT 
{
private: 	// Each array maps an integer into the elements of the array at that integer
	        // location, i.e. maps i to array[i].
	
	//##ModelId=3B0C087702B4
	CArray<COLL_PROP* , COLL_PROP*>	CollProps;//Array of collection properties
	//##ModelId=3B0C087702C8
	CArray<ATTR* , ATTR*>	Attrs; //Array of attributes , index is AttId
	//##ModelId=3B0C087702DC
	CArray<DOM_TYPE, DOM_TYPE> Domains; // Array of domains, index is AttId
	//##ModelId=3B0C087702F0
	CArray<INT_ARRAY* , INT_ARRAY*>	AttNames; //Attribute Names
	//##ModelId=3B0C0877030E
	CArray<IND_PROP* , IND_PROP*>	IndProps; //Properties of Indexes
	//##ModelId=3B0C08770322
	CArray<INT_ARRAY* , INT_ARRAY*>	IndNames; // Index Names
	//##ModelId=3B0C08770336
	CArray<BIT_IND_PROP* , BIT_IND_PROP*>	BitIndProps; //Properties of BitIndexes
	//##ModelId=3B0C08770354
	CArray<INT_ARRAY* , INT_ARRAY*>	BitIndNames; // BitIndex Names
	
public:
	
	//##ModelId=3B0C08770371
	CAT(CString filename);	//read information into catalog from some default file
	//##ModelId=3B0C0877037B
	~CAT();	// free the memory of the catalog structure
	
	//Each of the following functions retrieves data from one of the private arrays above.
	//If the index input is not within range it returns NULL.
	//##ModelId=3B0C08770385
	COLL_PROP* 	GetCollProp(int CollId);
	//##ModelId=3B0C0877038F
	ATTR* 	GetAttr(int AttId);
	//##ModelId=3B0C087703A3
	DOM_TYPE GetDomain(int AttId);
	//##ModelId=3B0C087703B7
	INT_ARRAY*	GetAttNames(int CollId);
	//##ModelId=3B0C087703C1
	INT_ARRAY*	GetIndNames(int CollId);
	//##ModelId=3B0C087703D6
	IND_PROP* 	GetIndProp(int IndId);
	//##ModelId=3B0C087703E0
	INT_ARRAY* GetBitIndNames(int CollId);
	//##ModelId=3B0C0878000C
	BIT_IND_PROP* 	GetBitIndProp(int BitIndId);
	
	// Each of the following functions adds data to the relevant table
	//They need to be public so we can add aliases (FROM emp AS e)
	
	// Add CollProp for this collection.  If Collection is new, also update CollTable
	//##ModelId=3B0C08780016
	void AddColl(CString CollName,COLL_PROP* CollProp);
	
	// fill tables related to attributes
	// If Attribute or Collection are new, add them to AttProps, AttTable, AttNames, resp.
	// Add AttProp to AttProps table, Attribute to Attnames
	//##ModelId=3B0C08780034
	void AddAttr(CString CollName,CString AttrName, ATTR* attr, DOM_TYPE domain);
	
	// If Index, COllection are new, add them to IndProps, IndNames, respectively.
	// Add IndProp, Index to IndProps, IndNames, resp.
	//##ModelId=3B0C08780066
	void AddIndex(CString CollName, CString IndexName, IND_PROP* indexprop);
	
	// If Index, COllection are new, add them to IndProps, IndNames, respectively.
	// Add IndProp, Index to IndProps, IndNames, resp.
	//##ModelId=3B0C0878008E
	void AddBitIndex(CString RelName, CString BitIndexName, BIT_IND_PROP* bitindexprop);
	
	// dump CAT content to a CString 
	//##ModelId=3B0C087800AC
	CString Dump();
	
private:	 // functions for parsing the catalog input file 
	// get Keys from line buf. format: (xxx,xxx)
	//##ModelId=3B0C087800B6
	void parseKeys(char *p, KEYS_SET* Keys, CString RelName);  
	
	// get attribute from line buf, fill tables related to attributes
	//##ModelId=3B0C087800DE
	void parseAttribute(char *p, CString& AttrName, ATTR *Attribute, DOM_TYPE & domain);
	
	// get index prop. from line buf
	//##ModelId=3B0C08780106
	void parseIndex(char *p,CString RelName,CString& IndexName, IND_PROP *Index);
	
	// get bit index prop. from line buf
	//##ModelId=3B0C08780138
	void parseBitIndex(char *p,CString RelName,CString& BitIndexName, BIT_IND_PROP *BitIndex);
	// get keys from line buf. format: (X.xx, X.xx)
	//##ModelId=3B0C0878016A
	void GetKey(char *p, KEYS_SET *Keys);
};

#endif //CAT_H
