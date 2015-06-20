/*	
supp.cpp -  implementation of supplement classes
	$Revision: 4 $
  Implements classes in supp.h

	Columbia Optimizer Framework
    
	A Joint Research Project of Portland State University 
	   and the Oregon Graduate Institute
	Directed by Leonard Shapiro and David Maier
	Supported by NSF Grants IRI-9610013 and IRI-9619977

	
*/

#include "stdafx.h"
#include "cat.h"
#include "item.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

//*************  Hash Function ******************
//needed for hashing, used for duplicate elimination.  See ../doc/dupelim

#define mix(a,b,c) \
{ \
	a -= b; a -= c; a ^= (c>>13); \
	b -= c; b -= a; b ^= (a<<8);  \
	c -= a; c -= b; c ^= (b>>13); \
	a -= b; a -= c; a ^= (c>>12); \
	b -= c; b -= a; b ^= (a<<16); \
	c -= a; c -= b; c ^= (b>>5);  \
	a -= b; a -= c; a ^= (c>>3);  \
	b -= c; b -= a; b ^= (a<<10); \
	c -= a; c -= b; c ^= (b>>15); \
}

ub4 lookup2( 
			register ub4  k,	    // the key to be hashed
			register ub4  initval)  // the previous hash, or an arbitrary value
{
	register ub4 a,b,c;
	
	/* Set up the internal state */
	a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
	c = initval;         /* the previous hash value */
	
	a += (k<<8);
	b += (k<<16);
	c += (k<<24);
	mix(a,b,c);
	
	return c;
}

ub4 lookup2( 
			register CString  k,	// the key to be hashed
			register ub4  length,   // the length of the key
			register ub4  initval)  // the previous hash, or an arbitrary value
{
	register ub4 a,b,c,len;
	
	/* Set up the internal state */
	len = length;
	a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
	c = initval;         /* the previous hash value */
	register i = 0;		// How many bytes of k have we processed so far?
	
	/*---------------------------------------- handle most of the key */
	while (len >= 12)
	{
		a += (k[i+0] +((ub4)k[i+1]<<8) +((ub4)k[i+2]<<16) +((ub4)k[i+3]<<24));
		b += (k[i+4] +((ub4)k[i+5]<<8) +((ub4)k[i+6]<<16) +((ub4)k[i+7]<<24));
		c += (k[i+8] +((ub4)k[i+9]<<8) +((ub4)k[i+10]<<16)+((ub4)k[i+11]<<24));
		mix(a,b,c);
		i += 12; len -= 12;
		
	}
	
	/*------------------------------------- handle the last 11 bytes */
	c += length;
	switch(len)              /* all the case statements fall through */
	{
	case 11: c+=((ub4)k[i+10]<<24);
	case 10: c+=((ub4)k[i+9]<<16);
	case 9 : c+=((ub4)k[i+8]<<8);
		/* the first byte of c is reserved for the length */
	case 8 : b+=((ub4)k[i+7]<<24);
	case 7 : b+=((ub4)k[i+6]<<16);
	case 6 : b+=((ub4)k[i+5]<<8);
	case 5 : b+=k[i+4];
	case 4 : a+=((ub4)k[i+3]<<24);
	case 3 : a+=((ub4)k[i+2]<<16);
	case 2 : a+=((ub4)k[i+1]<<8);
	case 1 : a+=k[i+0];
		/* case 0: nothing left to add */
	}
	mix(a,b,c);
	/*-------------------------------------------- report the result */
	return c;
}

//*********** int array functions  *********

int * GetOtherAttr(int * attr, int size, int * part_attr, int part_size)
{
	int result_size = size- part_size;
	int * result = new int [result_size];
	int i, j;
	// copy all the attributes excepts for those are candidate key
	for (i=0; i<size; i++)
	{
		for (j=0; j<part_size; j++)
			if ( attr[i] == part_attr[j] ) break;
			if (j == part_size)
				result[i] = attr[i];
	}
	
	return result;
}

// return the copy of the int array
int * CopyArray(int *IntArray, int Size)
{
	int * result = new int [Size];
	memcpy(result, IntArray, Size * sizeof(int) );
	return result;
}

// return true if the contents of two arrays are equal
bool EqualArray(int *array1, int * array2, int size)
{
	for(int i=0;i<size;i++)
		if( array1[i] != array2[i] ) return false;
		
		return true;
}

// **********  BIT_VECTOR function ********** 
void  bit_on(BIT_VECTOR & bit_vect, int rule_no)   //Turn this bit on
{
	unsigned int n = (1 << rule_no);
	
    assert(rule_no >= 0 && rule_no < 32);
    //assert( (bit_vect & n) == 0 );  //Be sure this bit is off!
	
    bit_vect = bit_vect | n;
};

bool is_bit_off(BIT_VECTOR bit_vect, int rule_no)  //Is this bit off?
{
	unsigned int n = (1 << rule_no);
	
    if( (bit_vect & n) == 0)
		return(true);
	else
		return(false);
}; 

//*************  Function for KEYS_SET class  ************ 
//##ModelId=3B0C085F0395
bool KEYS_SET::AddKey(CString CollName, CString KeyName)
{
	int AttId = GetAttId(CollName, KeyName);
	
	// check duplicate element in vector
	for(int i=0; i < KeyArray.GetSize(); i++)
		if(AttId == KeyArray[i]) return false;
		
		// if unique
		KeyArray.Add(AttId);
		
		return true;
}


//##ModelId=3B0C085F03A9
bool KEYS_SET::AddKey(int AttId)
{
	// check duplicate element in vector
	for(int i=0; i < KeyArray.GetSize(); i++)
		if(AttId == KeyArray[i]) return false;
		
		// if unique
		KeyArray.Add(AttId);
		
		return true;
}

//##ModelId=3B0C085F03B3
bool KEYS_SET::ContainKey(int AttId)
{
	// check if the attid is in the vector
	for(int i=0; i < KeyArray.GetSize(); i++)
		if(AttId == KeyArray[i]) return true;
		
		return false;
}

// return the int array from the keys_set
//##ModelId=3B0C085F03C7
int * KEYS_SET::CopyOut()
{
	int size = GetSize();
	int * result = new int [size];
	for(int i=0; i< size; i++)
		result[i] = KeyArray[i];
	
	return result;
}

// return the int array of size one from the keys_set
//##ModelId=3B0C085F03D1
int * KEYS_SET::CopyOutOne(int i)
{
	int *result = new int[1];
	result[0] = KeyArray[i];
	
	return result;
}

//Transform each key from A.B to NewName.B (actually the IDs)
//##ModelId=3B0C085F03BD
void KEYS_SET::update(CString NewName)
{
	int Size = KeyArray.GetSize();
	for(int i=0; i< Size; i++)
		KeyArray[i] =  GetAttId ( NewName, TruncName(GetAttName(KeyArray[i])));
}

// CString temp = GetAttName(KeyArray[index]);
// returns A.X temp.Format("%s");

//Returns the CuCard of the attribute
//##ModelId=3B0C0860004E
float KEYS_SET::GetAttrCuCard(int index)
{
	ATTR *attr;
	attr = Cat->GetAttr(KeyArray[index]);
	return attr->CuCard;
}

// choose the attribute with Max CuCard
//##ModelId=3B0C0860004D
int KEYS_SET::ChMaxCuCard()
{
	CString cname1, cname2;
	float cucard1, cucard2;
	int collid;
	int win = 0;
	for (int i=0; i<GetSize(); i++)
	{
		collid = GetCollId(KeyArray[i]);
		cname1 = GetCollName(collid);
		cucard1 = GetAttrCuCard(i);
		collid = GetCollId(KeyArray[win]);
		cname2 = GetCollName(collid);
		if (strcmp(cname1, cname2) == 0)
		{
			cucard2 = GetAttrCuCard(win);
			if (cucard1 >= cucard2)
				win = i;
		}
	}
	return win;
}

// dump KEYS_SET
//This function is not used anywhere 
//but was crucial in writing another 
//useful function namely
//PHYS_PROP::bestKey()
/*KEYS_SET* KEYS_SET::best()
{
	KEYS_SET* bestKeySet=new KEYS_SET();
	int win = ChMaxCuCard();
    int* result = CopyOutOne(win);
    int value = result[0];
	bestKeySet->AddKey(value);
	/****************************
    	bestKeySet = this;
    int win = this->ChMaxCuCard();
    int Size = this->GetSize();
	for (int i=0; i<Size; i++)
	if (i != win)
	{
	 bestKeySet->RemoveKeysSet(i);
	}

	******************************
    

	return bestKeySet;

	  
		
}*/


//##ModelId=3B0C08600061
CString KEYS_SET::Dump()
{
	CString os;
	CString temp;
	os = "(";
	int i;
	for(i=0; i< GetSize()-1; i++)
	{
		temp.Format("%s%s", GetAttName(KeyArray[i]) , "," );
		os += temp;
	}
	
	if(GetSize()) 
		temp.Format("%s%s", GetAttName(KeyArray[i]) ,")");
	else
		temp.Format("%s", ")");
	os += temp;
	
	return os;
}

//##ModelId=3B0C08600288
COLL_PROP::COLL_PROP(COLL_PROP& other)				// copy constructor
{
	Card = other.Card;
	Keys = other.Keys;
	CandidateKey = other.CandidateKey;
	for (int i=0; i<other.FKeyArray.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*other.FKeyArray[i]);
		FKeyArray.Add(fk);
	}
	Order = other.Order;
	UCard = other.UCard;
	Width = other.Width;
};

//##ModelId=3B0C0860026A
void COLL_PROP::update(CString NewName)
{
	Keys -> update(NewName);
	CandidateKey -> update(NewName);
	for (int i=0; i<FKeyArray.GetSize(); i++)
	{
		FKeyArray[i]->update(NewName);
	}
}

// dump collection property content
//##ModelId=3B0C0860027E
CString COLL_PROP::Dump()
{
	CString os, temp;
	os.Format("%s%.0f%s%.0f","  Card:" , Card,
		"  UCard:" , UCard);
	if (Keys->GetSize()>0)
	{
		temp.Format("%s%s", "  Order:" , OrderToString(Order) );
		os += temp;
	}
	
	temp.Format("%s%s", "  Keys:", (*Keys).Dump() );
	os += temp;
	
	temp.Format("%s%s%s", "  CandidateKey:" , (*CandidateKey).Dump(), "\r\n" );
	os += temp;
	
	if (FKeyArray.GetSize()>0)
	{
		temp.Format("%s", "  Foreign Keys:");
		os += temp;
		for(int i=0; i< FKeyArray.GetSize(); i++)
		{
			if (i<FKeyArray.GetSize()-1)
				temp.Format("%s%s", (*FKeyArray[i]).Dump(), "\r" );
			else temp.Format("%s%s", (*FKeyArray[i]).Dump(), "\r\n" );
			os += temp;
		}
	}
	
	return os;
}

//##ModelId=3B0C08600364
void IND_PROP::update(CString NewName)

{
	Keys -> update(NewName);
}

// dump index property content
//##ModelId=3B0C08600378
CString IND_PROP::Dump()
{
	CString os;
	os.Format("%s%s%s%s%s",	"  Type:" , IndexOrderToString(IndType),
		"  Keys:" , (*Keys).Dump(), (Clustered == true ? "  Clustered" : "  not Clustered") ) ;
	
	return os;
}

//##ModelId=3B0C08610078
void BIT_IND_PROP::update(CString NewName)

{
	BitAttr -> update(NewName);
}

// dump index property content
//##ModelId=3B0C0861008B
CString BIT_IND_PROP::Dump()
{
	CString os;
	os.Format("%s%s%s%s",	"  Bit Attributes:" , (* BitAttr).Dump(),
		"  Index Attributes:" , GetAttName(IndexAttr) );
	
	return os;
}

//##ModelId=3B0C0860010C
CString FOREIGN_KEY::Dump()
{
	CString os;
	os.Format("%s%s%s%s%s",	"(  Foreign Key:" , (* ForeignKey).Dump(),
		"  reference to:" , (* RefKey).Dump(), "  )" );
	
	return os;
}

//##ModelId=3B0C086101B0
ATTR::ATTR(CString range_var, int * atts, int size)
{	ATTR_EXP * ae = new ATTR_EXP(range_var, CopyArray(atts, size), size);
AttId = ae->GetAttNew()->AttId;
CuCard = ae->GetAttNew()->CuCard;
Min = ae->GetAttNew()->Min;
Max = ae->GetAttNew()->Max;

if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_ATTR].New(); 
delete ae;
}; //ATTR::ATTR(CString range_var, int * atts, int size)

// ATTR dump function
//##ModelId=3B0C086101CC
CString ATTR::Dump()
{
	CString os;
	os.Format("%s%s%s%s%.0f%s%.0f%s%.0f ", GetAttName(AttId), " Domain:" , 
		DomainToString(Cat->GetDomain(AttId)), " CuCard:", CuCard, " Min:", Min, " Max:", Max);
	return os;
};

//##ModelId=3B0C086101D5
CString ATTR::attrDump()
{
	CString os;
	os.Format("%s", GetAttName(AttId));
	return os;
}

//##ModelId=3B0C086101D6
CString ATTR::DumpCOVE()
{
	CString os;
	os.Format("%s %d ", GetAttName(AttId), (int)CuCard);
	return os;
};

// SCHEMA function
//##ModelId=3B0C08620028
bool    SCHEMA::AddAttr(int Index, ATTR *attr)
{
	assert( Index < Size );
	Attrs[Index]= attr ;
	
	return true;
}

// return true if the relname.attname is in the schema
//##ModelId=3B0C08620033
bool SCHEMA::InSchema(int AttId)
{
	for(int i=0;i<Size; i++)
		if( AttId==Attrs[i]->AttId )
			break;
		
		if(i<Size) return true;
		else return false;
}

// max cucard of each tables in the schema
//##ModelId=3B0C08620079
float SCHEMA::GetTableMaxCuCard(int TableIndex)
{
	float Max = 0;
	
	for(int i=0;i<Size; i++)
	{
		int CollId = GetCollId(Attrs[i]->AttId);
		// 0 is used for attr generated by rangevar(e.g. func_op(<A.X> as sum) )
		// along the query tree, they are not from any table
		if (CollId == 0) return Max;
		if( CollId == TableId[TableIndex] )  // the attr is from the table
		{	
			if( Max < Attrs[i]->CuCard ) Max = Attrs[i]->CuCard;
		}
	}
	
	return Max;
}

// width of the table in the schema
//##ModelId=3B0C08620083
float SCHEMA::GetTableWidth(int TableIndex)
{	
	//add Width=0 for Table "", used for AGG_OP
	if (TableId[TableIndex] == 0) return 0;
	return Cat->GetCollProp(TableId[TableIndex])->Width ; 
}

// projection
// 	projection of attrs onto schema
//##ModelId=3B0C08620047
SCHEMA * SCHEMA::projection( int * attrs, int size)
{
    SCHEMA * new_schema = new SCHEMA(size);
	
	new_schema->TableNum = 0;
	new_schema->TableId = new int [this->TableNum] ;
	
    //add attribute sets from left operand
    for (int i = 0;  i < size;  i++)
    {
		for(int index=0; index < this->Size; index++)
		{
			if ( attrs[i] == this->Attrs[index]->AttId )  
			{
				// has attr op in projection list -- add it in:
				//ATTR * Attr = new ATTR(Attrs[index]->AttId , Attrs[index]->CuCard, -1, -1);
				ATTR * Attr = new ATTR(*Attrs[index]);
				new_schema -> AddAttr( i, Attr ) ;
				
				// get the table info for the new schema
				int CollId = GetCollId(Attr->AttId);
				
				for(int i=0; i < new_schema->TableNum; i++)
					if( CollId == new_schema->TableId[i] )	break;
					
					if( i == new_schema->TableNum )		                            // a new table in the schema
						new_schema->TableId[ (new_schema->TableNum)++ ] = CollId;	// store the table id
					
					break;
			}
		}
		if(index == Size)  assert(false);	// project list not in schema
    }
	
    return (new_schema);
	
} // projection(attrs)

// union the attributes from the two joined SCHEMA.
// also check the joined predicates(attributes) are in the catalog(schema)
// calculate the ATT_PROP
//##ModelId=3B0C0862005A
SCHEMA * SCHEMA::UnionSchema(SCHEMA * other)
{
	int i,j;
	
	// union the schemas
	int LSize = this->GetSize();
	int RSize = other->GetSize();
	
	SCHEMA * Schema = new SCHEMA(LSize+RSize);
	
	Schema->TableNum = this->TableNum + other->TableNum;
	Schema->TableId = new int [Schema->TableNum];
	for(i=0; i<this->TableNum; i++) Schema->TableId[i] = this->TableId[i];
	for(j=0; j<other->TableNum; j++) Schema->TableId[i+j] = other->TableId[j];
	
	ATTR* Attr;
	for(i=0;i<LSize;i++)
	{
		// from cascade
		// we calculate new cucards, in a very very crude way.
		// New cucards are half the old ones :)
		float CuCard = (*this)[i]->CuCard;
		CuCard = (CuCard != -1) ? CuCard / 2 : -1 ;
		float min = (*this)[i]->Min;
		float max = (*this)[i]->Max;
		Attr = new ATTR((*this)[i]->AttId,  CuCard, min, max );
		Schema->AddAttr( i, Attr ) ;	
	}
	
	for(j = 0; j < RSize; j++)
	{
		// from cascade
		// we calculate new cucards, in a very very crude way.
		// New cucards are half the old ones :)
		float CuCard = (*other)[j]->CuCard;
		CuCard = (CuCard != -1) ? CuCard / 2 : -1 ;
		float min = (*other)[j]->Min;
		float max = (*other)[j]->Max;
		Attr = new ATTR((*other)[j]->AttId,	CuCard, min, max );
		Schema->AddAttr( i+j, Attr ) ;
	}
	
	return Schema;
}

// return true if contains all the keys
//##ModelId=3B0C0862003D
bool SCHEMA::Contains( KEYS_SET * Keys)
{
    for (int i = 0;  i < Keys->GetSize();  i++)
    {
		if( ! InSchema((*Keys)[i]) ) return false;
    }
	
    return true;
	
} // Contains

// free up memory
//##ModelId=3B0C0862001F
SCHEMA::~SCHEMA()
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_SCHEMA].Delete();
	for(int i=0;i<Size;i++) delete Attrs[i] ;
	delete Attrs;
	
	delete [] TableId;
}

// SCHEMA dump function
//##ModelId=3B0C086200A1
CString SCHEMA::Dump()
{
	CString os;
	for(int i=0;i<Size;i++)
	{
		os += (*(Attrs[i])).Dump();
		os += "\r\n";
	}
	return os;
}

//##ModelId=3B0C086200AA
CString SCHEMA::DumpCOVE()
{
	CString os;
	for(int i=0;i<Size;i++)
		os += (*(Attrs[i])).DumpCOVE();
	return os;
}

// SCHEMA attributes store function
//##ModelId=3B0C086200A0
KEYS_SET* SCHEMA::AttrStore()
{
	KEYS_SET *largeKeySet = new KEYS_SET();
	//PTRACE ("Schema Size is %d", GetSize());
	for(int i=0;i<GetSize();i++)
	{
		//os += (*(Attrs[i])).attrDump();
		//PTRACE("attribute dump is %s", (*(Attrs[i])).attrDump());
		if ((IntOrdersSet.ContainKey(Attrs[i]->AttId)) == true)
		{
			largeKeySet->AddKey(Attrs[i]->AttId);
		}
	}
	return largeKeySet;
}

// LOG_COLL_PROP dump function
//##ModelId=3B0C086202E5
CString LOG_COLL_PROP::Dump()
{
	CString os, temp;
	os.Format("%s%.0f%s%.0f%s%s%s", "  Card: " , Card , "  UCard: " , UCard ,"\r\n",
				    "Schema:\r\n", (*Schema).Dump());
	if (CandidateKey->GetSize()>0)
	{
		temp.Format("%s%s%s", "CandidateKey:", (*CandidateKey).Dump(), "\r\n" );
		os += temp;
	}
	
	if (FKeyList.GetSize()>0)
	{
		temp.Format("%s", "  Foreign Keys:");
		os += temp;
		for(int i=0; i< FKeyList.GetSize(); i++)
		{
			if (i<FKeyList.GetSize()-1)
				temp.Format("%s%s", (*FKeyList[i]).Dump(), "\r" );
			else temp.Format("%s%s", (*FKeyList[i]).Dump(), "\r\n" );
			os += temp;
		}
	}
	return os;
};

// LOG_COLL_PROP dump function for COVE script
//##ModelId=3B0C086202E6
CString LOG_COLL_PROP::DumpCOVE()
{
	CString os;
	os.Format("%d %d { %s }\r\n",(int)Card, (int)UCard,  (*Schema).DumpCOVE());
	return os;
};

// misc functions

// Get Collection id from name, using CollTable dictionary
// If not present, add it
int GetCollId(int AttId)
{
	if(AttId == 0 ) return 0;
	assert( AttId < AttCollTable.GetSize() ) ;
	return AttCollTable [AttId];
}

// Get the ids from names
int GetCollId(CString CollName)
{
	int Size = CollTable.GetSize();
	for(int i=0; i < Size; i++)
		if( CollName == CollTable[i] ) break;
		
		if(i == Size) 
			CollTable.Add(CollName);
		
		return i;	
}

// Get Att id from name, using AttTable dictionary
// If not present, add full Att name to AttTable, entry to AttCollTable

int GetAttId(CString CollName, CString AttName)
{
	
	CString Name = CollName + "." + AttName; 
	int Size = AttTable.GetSize();
	for(int i=0; i < Size; i++)
	{
		if( Name == AttTable[i] ) break;
	}
	
	if(i == Size) // the entry not exist, new it
	{
		AttTable.Add(Name);
		AttCollTable.Add( GetCollId(CollName) ) ;
	}
	
	return i;	
}

int GetAttId(CString Name)
{
	int pos = Name.Find('.');
	assert(pos!= -1);
	
	int Size = AttTable.GetSize();
	for(int i=0; i < Size; i++)
		if( Name == AttTable[i] ) break;
		
		if(i == Size) // the entry not exist, new it
		{
			AttTable.Add(Name);
			CString CollName = Name.Left(pos);
			AttCollTable.Add( GetCollId(CollName) ) ;
		}
		
		return i;	
}

// Get the ids from names
int GetIndId(CString CollName, CString IndName)
{
	CString Name = CollName + "." + IndName; 
	int Size = IndTable.GetSize();
	for(int i=0; i < Size; i++)
		if( Name == IndTable[i] ) break;
		
		if(i == Size) // the entry not exist, new it
			IndTable.Add(Name);
		
		return i;	
}

// Get the ids from names
int GetBitIndId(CString CollName, CString BitIndName)
{
	CString Name = CollName + "." + BitIndName; 
	int Size = BitIndTable.GetSize();
	for(int i=0; i < Size; i++)
		if( Name == BitIndTable[i] ) break;
		
		if(i == Size) // the entry not exist, new it
			BitIndTable.Add(Name);
		
		return i;	
}

// Get the names from Ids
CString GetCollName(int CollId)
{
	if( CollId == 0 ) return "";
	assert( CollId < CollTable.GetSize() ) ;	
	return CollTable [CollId];
}

CString GetAttName(int AttId)
{
	if(AttId == 0 ) return "";
	assert( AttId < AttTable.GetSize() ) ;
	return AttTable [AttId];
}

//Transform A.B to B
CString TruncName(CString AttName)
{
	char *p = strstr(AttName, ".");
	assert(p);  //Input was not of the form A.B
	p++; //skip over .
	return p;
}

CString GetIndName(int IndId)
{
	if(IndId == 0 ) return "";
	assert( IndId < IndTable.GetSize() ) ;
	return IndTable [IndId];
}

CString GetBitIndName(int BitIndId)
{
	if(BitIndId == 0 ) return "";
	assert( BitIndId < BitIndTable.GetSize() ) ;
	return BitIndTable [BitIndId];
}

DOM_TYPE atoDomain(char *p)
{
	if(strcmp(p,"string_t")==0) return string_t;
	if(strcmp(p,"int_t")==0) return int_t;
	if(strcmp(p,"real_t")==0) return real_t;
	if(strcmp(p, "unknown")==0) return unknown;
	OUTPUT_ERROR("Domain type");
	return string_t;
}

CString DomainToString(DOM_TYPE p)
{
	if(p==string_t) return "string_t";
	if(p==int_t) return "int_t";
	if(p==real_t) return "real_t";
	if(p==unknown) return "unknown";
	OUTPUT_ERROR("Domain type");
	return "";
}

ORDER_AD atoKeyOrder(char *p)
{
	if(strcmp(p,"ascending")==0) return ascending;
	if(strcmp(p,"descending")==0) return descending;
	OUTPUT_ERROR("Key order type");
	return ascending;
}

ORDER atoCollOrder(char *p)
{
	if(strcmp(p,"heap")==0) return heap;
	if(strcmp(p,"hashed")==0) return hashed;
	if(strcmp(p,"sorted")==0) return sorted;
	if(strcmp(p, "any")==0) return any;
	OUTPUT_ERROR("Coll order type");
	return heap;
}

CString	OrderToString(ORDER p)
{
	if(p==heap) return "heap";
	if(p==hashed) return "hashed";
	if(p==sorted) return "sorted";
	OUTPUT_ERROR("Coll order type");
	return "";
}

ORDER_INDEX atoIndexOrder(char *p)
{
	if(strcmp(p,"btree")==0) return btree;
	if(strcmp(p,"hash")==0) return hash;
	OUTPUT_ERROR("Index order type");
	return btree;
}

CString IndexOrderToString(ORDER_INDEX p)
{
	if(p==btree) return "btree";
	if(p==hash) return "hash";
	OUTPUT_ERROR("Index order type");
	return "";
}

// return only the file name (without path)
// used by TRACE function
CString Trim(CString PathName)
{
	int pos = PathName.ReverseFind('\\');
	return (PathName.Mid( pos+1 ));
};

// skip the blank space	
char *SkipSpace(char *p)
{
	while (*p==' ' || *p=='\t' ) p++;  
	return p;
}

// return true if the string line is Comment or blank line
bool IsCommentOrBlankLine(char *p)
{
	p = SkipSpace(p);
	
	if(*p=='\n'||*p==0)  return true;		// blank line
	
	if( *p == '/' && *(p+1) == '/')  return true;	// comment line
	else  return false;
}

void parseString(char *p)
{
	p = SkipSpace(p);
	while (*p!=' '&& *p!='\t' && *p!='\n') p++; 	  // keep the char until blank space
	*p=0;
}

//=============  PHYS_PROP Methods  ===================
//##ModelId=3B0C0863023C
PHYS_PROP::PHYS_PROP( KEYS_SET * Keys, ORDER Order)
: Keys(Keys), Order(Order)
{	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_PHYS_PROP].New(); };

//a constructor for ANY property
//##ModelId=3B0C08630250
PHYS_PROP::PHYS_PROP(ORDER Order)
: Keys(NULL), Order(Order)
{	assert(Order == any);
if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_PHYS_PROP].New(); 
}

//##ModelId=3B0C0863025A
PHYS_PROP::PHYS_PROP(PHYS_PROP& other)
: Keys(other.Order==any ? NULL: new KEYS_SET(*(other.Keys)) ),
Order(other.Order)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_PHYS_PROP].New(); 
	if (Order == sorted) 
	{
		assert(other.KeyOrder.GetSize()==other.Keys->GetSize());
		for (int i=0; i<other.KeyOrder.GetSize(); i++)
			this->KeyOrder.Add(other.KeyOrder[i]);
	}
}

//##ModelId=3B0C08630279
void PHYS_PROP::Merge(PHYS_PROP& other)
{
	assert(Order == other.Order);	// only idential orders can be merge
	
	Keys->Merge(*(other.Keys));
	if (Order == sorted)
	{
		for (int i=0; i<other.KeyOrder.GetSize(); i++)
			this->KeyOrder.Add(other.KeyOrder[i]);
	}
}

//##ModelId=3B0C0863028D
bool PHYS_PROP::operator== (PHYS_PROP & other)
{
	
	//Note that operator== is defined properly for enums,
    //by default
	if(other.Order==any && Order==any) return true;
	
	//if one is any and the other is not any
	if(other.Order==any ||  Order==any) return false;
	
    if ( other.Order == Order && *(other.Keys) == *Keys )
	{
		if (Order == sorted)
		{
			if (KeyOrder.GetSize() != other.KeyOrder.GetSize())
				return false;
			
			for (int i=0; i<KeyOrder.GetSize(); i++)
				if (KeyOrder[i] != other.KeyOrder[i]) return false;
		}
		return true;
	}
	return false;
} // PHYS_PROP::operator==

//##ModelId=3B0C08630296
CString PHYS_PROP::Dump()
{
	CString os;
	
	if (Order == any) { os.Format("%s", "Any Prop"); }
	else
	{
		os.Format("%s on %s", 	
			Order ==heap ? "heap"
			: Order == sorted ? "sorted"
			: Order == hashed ? "hashed"
			: "UNKNOWN"
			,	 Keys -> Dump() );
	}
	if (Order == sorted)
	{
		CString temp;
		os += "  KeyOrder: (";
		int i;
		for (i=0; i<KeyOrder.GetSize()-1; i++)
		{
			temp.Format("%s, ", KeyOrder[i]==ascending? "ascending" : "descending");
			os += temp;
		}
		temp.Format("%s)", KeyOrder[i]==ascending? "ascending" : "descending");
		os +=temp;
	}
	
	return os;
}


//##ModelId=3B0C0863028C
void PHYS_PROP::bestKey()
{
	//find the index of the KeyArray 
	//with maximum unique cardinality
	int win = Keys->ChMaxCuCard();
    int Size = Keys->GetSize();
    int SizeKeyOrder = KeyOrder.GetSize();
	
    //make sure that both the CArrays ,
    //Keys and KeyOrder are equal in size
    // and also that win is within the size
    //of these CArrays
    assert(Size == SizeKeyOrder);
    assert(win <= Size);
	
    //copy out in an array just the element at
    //position win and store its value in KeyValue
    int* result = Keys->CopyOutOne(win);
    int KeyValue = result[0];
	
    //Set the size of the original Keys
    //to 0 and then fill the above KeyValue in it 
    Keys->reset();
    Keys->AddKey(KeyValue);
	
    KeyOrder[0]=KeyOrder[win];
    KeyOrder.SetSize(1);
	delete result;
}

//=============  CONT Methods  ===================

//##ModelId=3B0C0864032E
CONT::CONT( PHYS_PROP *RP,  COST *U, bool D):
ReqdPhys(RP), UpperBd(U), Finished(false)
{
	//If the Physical Property has >1 attribute, use only the most selective attribute
	if(RP && (RP->GetKeysSet()) && (RP->GetKeysSet()->GetSize() > 1) )
		//RP-> SetKeysSet( RP->GetKeysSet() -> best());
		RP->bestKey();
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_CONT].New(); };
	
//##ModelId=3B0C086402DF
	CArray< CONT * , CONT* > CONT::vc;
	
	//=============  COST Methods  ===================
	
//##ModelId=3B0C086400B7
	void COST::FinalCost ( COST * LocalCost, COST ** TotalInputCost, int Size)
	{
        *this  = *LocalCost ;
		
        for(int i = Size; --i>=0 ; ) 
		{
			assert(TotalInputCost[i]);
			*this  += *(TotalInputCost[i]);
			
        }
        return;
	}
	
//##ModelId=3B0C08640125
	CString COST::Dump()
	{
		CString os;
		
		os.Format(" %.3f ",Value);
		return os;
	}
	
//##ModelId=3B0C085F02AF
	CString OPT_STAT::Dump()
	{
		CString os , Temp;
		Temp.Format("Duplicate MExpr: %d \r\n",DupMExpr);
		os += Temp;
		Temp.Format("Hashed Logical MExpr: %d \r\n",HashedMExpr);
		os += Temp;
		Temp.Format("Max Overflow Buckets: %d \r\n",MaxBucket);
		os += Temp;
		Temp.Format("FiredRules: %d \r\n",FiredRule);
		os += Temp;
		
		return os;
	}
	
	CString DumpStatistics()
	{
		CString os;
		CString temp;
		
		os = "Class Statistics:\r\n";
		for(int i=0; i < CLASS_NUM; i++)
			os += ClassStat[i].Dump();
		
		return os;
	}
	
	// get used physical memory
	int GetUsedMemory()
	{
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&ms);
		
		return (ms.dwTotalVirtual - ms.dwAvailVirtual);
	}
	
