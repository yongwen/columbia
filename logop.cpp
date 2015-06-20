/*	
logop.cpp -  implementation of classes of logical operators
as defined in logop.h

	$Revision: 5 $
	Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
  
	
*/
#include "stdafx.h"
#include "cat.h"
#include "logop.h"
#include "item.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

/*********** GET functions ****************/
//##ModelId=3B0C087301FB
GET::GET(int collId)
	:CollId(collId) 
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_GET].New();
#ifdef _DEBUG
	name = GetName() + GetCollName(CollId);		// for debug
#endif
};

//##ModelId=3B0C087301F0
GET::GET ( CString collection,  CString rangeVar)
{
	SET_TRACE Trace(true);
	RangeVar = rangeVar;
	if(collection == rangeVar)
		CollId = GetCollId(collection);
	else
	{
		//It's a nontrivial range variable.  Add new entries to all
		//relevant tables, with name of RangeVar and properties equal to those
		//of the collection.  
		int collectionID = GetCollId(collection);
		
		//Set GET's CollId to RangeVar's new ID.
		CollId = GetCollId(rangeVar);
		
		//Get all atts for this collection, then add to att tables
		INT_ARRAY* AttArray = Cat -> GetAttNames(collectionID);
		int Size = AttArray -> GetSize();
		ATTR * attr;
		for (int i = 0 ; i < Size; i++)  //For each attribute
		{
			attr = new ATTR(* (Cat -> GetAttr(AttArray -> GetAt(i)) ) );
			DOM_TYPE domain = Cat->GetDomain(AttArray->GetAt(i));
			Cat -> AddAttr(RangeVar, 
				TruncName(GetAttName(AttArray -> GetAt(i))) , attr, domain );
		}
		
		PTRACE2("Catalog content after fixing AttId-based tables for range %s:\r\n%s", 
			RangeVar, Cat->Dump());
		//OutputFile.Flush();
		
		//Get all indexes and ditto
		INT_ARRAY* IndArray = Cat -> GetIndNames(collectionID);
		if (IndArray)
		{
			Size = IndArray -> GetSize();
			for (i = 0 ; i < Size; i++)  //For each index
			{
				IND_PROP *indprop = new IND_PROP;
				IND_PROP* ip = (Cat -> GetIndProp(IndArray -> GetAt(i)) );
				*indprop = *ip;
				//Alter keys in the property object so they will refer to new range variable attributes
				indprop -> update(RangeVar);
				Cat -> AddIndex(RangeVar, 
					TruncName(GetIndName(IndArray -> GetAt(i))) , indprop );
			}
		}
		PTRACE2("Catalog content after fixing IndId-based tables for range %s:\r\n%s", 
			RangeVar, Cat->Dump());
		
		//Get all bit indexes and ditto
		INT_ARRAY* BitIndArray = Cat -> GetBitIndNames(collectionID);
		if (BitIndArray)
		{
			Size = BitIndArray -> GetSize();
			for (i = 0 ; i < Size; i++)  //For each index
			{
				BIT_IND_PROP *bitindprop = new BIT_IND_PROP;
				BIT_IND_PROP* ip = (Cat -> GetBitIndProp(BitIndArray -> GetAt(i)) );
				*bitindprop = *ip;
				//Alter keys in the property object so they will refer to new range variable attributes
				bitindprop -> update(RangeVar);
				Cat -> AddBitIndex(RangeVar, 
					TruncName(GetBitIndName(BitIndArray -> GetAt(i))) , bitindprop );
			}
		}
		PTRACE2("Catalog content after fixing BitIndId-based tables for range %s:\r\n%s", 
			RangeVar, Cat->Dump());
		
		//Populate all relevant CollId-based tables
		//Should use a cinstructor here but it's already in use.
		COLL_PROP* collp = new COLL_PROP;  
		*collp = *( Cat -> GetCollProp(collectionID) ); //Will be in catalog
		collp -> update(RangeVar);
		Cat -> AddColl(RangeVar, collp );
		PTRACE("Catalog content after fixing CollId-based tables:\r\n%s", Cat->Dump());
	}
	
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_GET].New();
#ifdef _DEBUG
	name = GetName() + GetCollName(CollId);		// for debug
#endif
	
}

//##ModelId=3B0C08730204
GET::GET( GET& Op )
	:CollId(Op.GetCollection())
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_GET].New();
#ifdef _DEBUG
	name = Op.name;		// for debug
#endif
}

//##ModelId=3B0C0873024A
CString GET::Dump()
{
	CString os;
	os.Format("%s%s%s%s",GetName(),"(" , GetCollName(CollId) , ")");
	return os;
}

// find the logical property of the collection, 
// also check the schema
//##ModelId=3B0C08730218
LOG_PROP* GET::FindLogProp (LOG_PROP ** input)
{
	COLL_PROP* CollProp = Cat->GetCollProp(CollId);
	assert(CollProp!=NULL);
	
	INT_ARRAY* AttrNames = Cat->GetAttNames(CollId);
	assert(AttrNames!=NULL);
	
	int Size = AttrNames->GetSize();
	assert(Size>0);
	
	SCHEMA * Schema = new SCHEMA(Size);
	Schema->TableNum = 1;
	Schema->TableId = new int;
	Schema->TableId[0] = CollId;
	
	ATTR	 *Attr;
	int i;
	for(i=0;i<Size;i++) 
	{
		Attr = new ATTR(*Cat->GetAttr((*AttrNames)[i]));
		assert(Attr!=NULL);
		//Attr = new ATTR((*AttrNames)[i], AttProp->CuCard);
		Schema->AddAttr( i, Attr ) ;
	}	
	
	KEYS_SET * cand_key;
	int cand_key_size = CollProp->CandidateKey->GetSize();
	int * temp_key = CollProp->CandidateKey->CopyOut();
	cand_key = new KEYS_SET(temp_key, cand_key_size );
	delete [] temp_key;
	
	LOG_COLL_PROP * result = new LOG_COLL_PROP(CollProp->Card, CollProp->UCard,Schema, 
		cand_key );
	//copy the foreign key info from catalog
	for (i=0; i<CollProp->FKeyArray.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*CollProp->FKeyArray[i]);
		result->FKeyList.Add(fk);
	}
	
	return result;
}

//##ModelId=3B0C08730253
ub4 GET::hash()
{
	ub4 hashval = GetInitval();
	
	hashval = lookup2(CollId, hashval) ;
	
	return (hashval % (HtblSize -1) ) ;
}


/*********** EQJOIN functions ****************/
//##ModelId=3B0C08730311
EQJOIN::EQJOIN(int *lattrs, int *rattrs, int size)
:lattrs(lattrs),rattrs(rattrs), size(size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_EQJOIN].New();
#ifdef _DEBUG
	name = GetName();		// for debug
#endif
	
};

//##ModelId=3B0C08730326
EQJOIN::EQJOIN( EQJOIN& Op)
:lattrs( CopyArray(Op.lattrs,Op.size) ), 
rattrs( CopyArray(Op.rattrs,Op.size) ), 
size(Op.size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_EQJOIN].New();
#ifdef _DEBUG
	name = Op.name;		// for debug
#endif
	
};

//##ModelId=3B0C08730363
CString EQJOIN::Dump()
{
	CString os;
	CString temp;
	int i;
	
	os = GetName() + "(<";
	
	for (i=0; (size > 0) && (i< size-1); i++) 
	{
		temp.Format("%s%s", GetAttName(lattrs[i]), ",");
		os += temp;
	}
	
	if( size > 0 ) 
		temp.Format("%s%s%s", GetAttName(lattrs[i]),
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
}

//##ModelId=3B0C08730331
LOG_PROP* EQJOIN::FindLogProp (LOG_PROP ** input)
{
	LOG_COLL_PROP* Left = (LOG_COLL_PROP*)(input[0]);
	LOG_COLL_PROP* Right = (LOG_COLL_PROP*)(input[1]);
	
	assert(Left->Card >= 0);
	assert(Right->Card >= 0);
	assert(Left->UCard >= 0);
	assert(Right->UCard >= 0);
	
	//check the joined predicates(attributes) are in the schema
	int i;
	for(i=0; i< size; i++)
		assert( Left->Schema->InSchema(lattrs[i]) );
	
	for(i=0; i< size; i++)
		assert( Right->Schema->InSchema(rattrs[i]) );
	
	// union schema
	SCHEMA *Schema = Left->Schema->UnionSchema(Right->Schema);
	
	// compute join log_prop
	int ConditionNum = size;
	
	bool LeftFK = false;	//Is FK on left or right
	bool RightFK = false;
	bool IsFKJoin = false;	//Is this a FK join?
	bool IsFKSchema = false;	//Is it like an FK join except refkey is not a cand key of entire join
	float RefUcard=1;		//Unique card of refkey, needed to compute eqjoin card
	
	//Eqjoin card is card of ref input (as opposed to FK input) divided by RefUcard
	
	//check if lattr contains foreign key
	for (i=0; i<Left->FKeyList.GetSize(); i++)
	{
		//if lattr contains FK
		if (Left->FKeyList[i]->ForeignKey->IsSubSet(lattrs, size))
		{
			//eqjoin on the refkey
			if (Left->FKeyList[i]->RefKey->IsSubSet(rattrs, size))
			{
				LeftFK = true;
				//check if reference key is candidate key of Right input
				if ( (*Left->FKeyList[i]->RefKey) == (*Right->CandidateKey))
				{
					IsFKJoin = true;
					IsFKSchema = true;
					break;
				}
				else
				{
					IsFKJoin = false;
					IsFKSchema = true;
					//get Cucard for reference key;
					// not sure if it is the correct multiple	
					for (int j=0; j<Left->FKeyList[i]->RefKey->GetSize(); j++)
					{
						ATTR * Attr = Cat->GetAttr((*Left->FKeyList[i]->RefKey)[j]);
						RefUcard *= Attr->CuCard;
					}
				}
			}
		}
	}
	
	if (!LeftFK)		//continue to check the right one
	{
		for (i=0; i<Right->FKeyList.GetSize(); i++)
		{
			//if rattr contains FK
			if (Right->FKeyList[i]->ForeignKey->IsSubSet(rattrs, size))
			{
				//eqjoin on the refkey
				if (Right->FKeyList[i]->RefKey->IsSubSet(lattrs, size))
				{
					RightFK = true;
					//check if reference key is candidate key of Left input
					if ( (*Right->FKeyList[i]->RefKey) == (*Left->CandidateKey))
					{
						IsFKJoin = true;
						IsFKSchema = true;
						break;
					}
					else
					{
						IsFKJoin = false;
						IsFKSchema = true;
						//get Cucard for reference key; 
						// not sure if it is the correct multiple	
						for (int j=0; j<Right->FKeyList[i]->RefKey->GetSize(); j++)
						{
							ATTR * Attr = Cat->GetAttr((*Right->FKeyList[i]->RefKey)[j]);
							RefUcard *= Attr->CuCard;
						}
					}
				}
			}
		}
	}
	
	double Card;
	double UCard;
	switch(ConditionNum)
	{
	case 0 :	// no join condition, cross-product
		if(Left->Card == -1 || Right->Card == -1) Card = -1;
		else	Card = Left->Card * Right->Card ;
		if(Left->UCard == -1 || Right->UCard == -1) UCard = -1;
		else 	UCard = Left->UCard * Right->UCard ;
		break;
		
	default:	
		if(Left->Card == -1 || Right->Card == -1) Card = -1;
		else if (LeftFK)
		{
			if (IsFKJoin)
				Card = Left->Card;
			else if (IsFKSchema)
				Card = Left->Card * (Right->Card/RefUcard);
			else assert(false);
		}
		else if (RightFK)
		{
			if (IsFKJoin)
				Card = Right->Card;
			else if (IsFKSchema)
				Card = Right->Card * (Left->Card/RefUcard);
			else assert(false);
		}
		else{
			Card = (Left->Card > Right->Card) ? 
				Left->Card / pow(Right->Card,ConditionNum-1) : 
			Right->Card / pow(Left->Card,ConditionNum-1);
		}
		
		if(Left->UCard == -1 || Right->UCard == -1) UCard = -1;
		else if (LeftFK)
		{
			if (IsFKJoin)
				UCard = Left->UCard;
			else if (IsFKSchema)
				UCard = Left->UCard * (Right->Card/RefUcard);
			else assert(false);
		}
		else if (RightFK)
		{
			if (IsFKJoin)
				UCard = Right->UCard;
			else if (IsFKSchema)
				UCard = Right->UCard * (Left->Card/RefUcard);
			else assert(false);
		}
		else	UCard = (Left->UCard > Right->UCard) ? 
			Left->UCard / pow(Right->UCard,ConditionNum-1) : 
		Right->UCard / pow(Left->UCard,ConditionNum-1);
		break;
	}
	
	//the candidate key is the merge of two candidate keys from the inputs
	KEYS_SET * cand_key;
	if (IsFKJoin)
	{
		if (LeftFK)
		{
			int * temp_key = Left->CandidateKey->CopyOut();
			cand_key = new KEYS_SET(temp_key, Left->CandidateKey->GetSize());
			delete [] temp_key;
		}
		else if (RightFK)
		{
			int * temp_key = Right->CandidateKey->CopyOut();
			cand_key = new KEYS_SET(temp_key, Right->CandidateKey->GetSize());
			delete [] temp_key;
		}
	}
	else 
	{
		int * temp_key = Left->CandidateKey->CopyOut();
		cand_key = new KEYS_SET(temp_key, Left->CandidateKey->GetSize());
		delete [] temp_key;
		cand_key->Merge(* Right->CandidateKey);
	}
	
	LOG_COLL_PROP * result = new LOG_COLL_PROP((float)Card, (float)UCard, Schema, cand_key);
	
	//foreign key is the merge of left foreign keys and right foreign keys
	for (i=0; i<Left->FKeyList.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*Left->FKeyList[i]);
		result->FKeyList.Add(fk);
	}
	for (i=0; i<Right->FKeyList.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*Right->FKeyList[i]);
		result->FKeyList.Add(fk);
	}
	
	return result;
	
}

//##ModelId=3B0C08730362
ub4 EQJOIN::hash()
{
    ub4 hashval = GetInitval();
	
	//to check the equality of the conditions
	for(int i = size ; --i >= 0 ;)
	{
		hashval = lookup2(lattrs[i],hashval );
		hashval = lookup2(rattrs[i],hashval );
	}
	return (hashval % (HtblSize -1) ) ;
}
/*********** DUMMY functions ****************/
//##ModelId=3B0C0874002F
DUMMY::DUMMY()
{
#ifdef _DEBUG
	name = GetName();		// for debug
#endif
	
};

//##ModelId=3B0C08740030
DUMMY::DUMMY( DUMMY& Op)

{
#ifdef _DEBUG
	name = Op.name;		// for debug
#endif
	
};

//##ModelId=3B0C08740074
CString DUMMY::Dump()
{
	CString os;
	os.Format("%s",GetName());
	return os;
}

//##ModelId=3B0C08740043
LOG_PROP* DUMMY::FindLogProp (LOG_PROP ** input)
{
	LOG_COLL_PROP* Left = (LOG_COLL_PROP*)(input[0]);
	LOG_COLL_PROP* Right = (LOG_COLL_PROP*)(input[1]);
	
	assert(Left->Card >= 0);
	assert(Right->Card >= 0);
	assert(Left->UCard >= 0);
	assert(Right->UCard >= 0);
	
	// union schema
	SCHEMA *Schema = Left->Schema->UnionSchema(Right->Schema);
	
	double Card;
	double UCard;
	// no join condition, cross-product
	if(Left->Card == -1 || Right->Card == -1) Card = -1;
	else	Card = Left->Card * Right->Card ;
	if(Left->UCard == -1 || Right->UCard == -1) UCard = -1;
	else 	UCard = Left->UCard * Right->UCard ;
	
	//the candidate key is the merge of two candidate keys from the inputs
	KEYS_SET * cand_key;
	{
		int * temp_key = Left->CandidateKey->CopyOut();
		cand_key = new KEYS_SET(temp_key, Left->CandidateKey->GetSize());
		delete [] temp_key;
		cand_key->Merge(* Right->CandidateKey);
	}
	
	LOG_COLL_PROP * result = new LOG_COLL_PROP((float)Card, (float)UCard, Schema, cand_key);
	return result;
	
}

//##ModelId=3B0C0874006B
ub4 DUMMY::hash()
{
    ub4 hashval = GetInitval();
	
	return (hashval % (HtblSize -1) ) ;
}

/*********** PROJECT functions ****************/

//##ModelId=3B0C08740205
PROJECT::PROJECT(int *attrs, int size)
:attrs(attrs), size(size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_PROJECT].New();
#ifdef _DEBUG
	name = GetName();		// for debug
#endif
	
};

//##ModelId=3B0C08740210
PROJECT::PROJECT( PROJECT& Op)
:attrs( CopyArray(Op.attrs,Op.size) ), 
size(Op.size)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_PROJECT].New();
#ifdef _DEBUG
	name = Op.name;		// for debug
#endif
};

//##ModelId=3B0C0874024C
ub4 PROJECT::hash()
{
    ub4 hashval = GetInitval();
	
	//to check the equality of the conditions
	for(int i = size ; --i >= 0 ;)
		hashval = lookup2(attrs[i],hashval );
	
	return (hashval % (HtblSize -1) ) ;
}

//##ModelId=3B0C08740223
LOG_PROP* PROJECT::FindLogProp (LOG_PROP ** input)
{
    float new_ucard=1, attr_cucard;
	
    LOG_COLL_PROP * rel_input = (LOG_COLL_PROP *)(input[0]);
	
    SCHEMA * schema = rel_input -> Schema -> projection( attrs, size);
	
    for (int att_index=0; att_index < schema->GetSize(); att_index++) 
	{
		attr_cucard = (*schema)[att_index] -> CuCard;
		if (attr_cucard != -1 ) new_ucard *= attr_cucard;
		else 
		{
			new_ucard = rel_input -> Card;
			break;
		}
    }
    new_ucard = MIN(new_ucard, rel_input -> Card);
	
	KEYS_SET * cand_key;
	if (rel_input->CandidateKey->IsSubSet(attrs, size))
	{
		int * temp_key = rel_input->CandidateKey->CopyOut();
		cand_key = new KEYS_SET(temp_key, rel_input->CandidateKey->GetSize());
		delete [] temp_key;
	}
	else cand_key = new KEYS_SET();
	
    LOG_COLL_PROP * result = new LOG_COLL_PROP(rel_input->Card, new_ucard, schema, cand_key);
	
	//if foreign keys are subset of project attrs, pass this foreign key
	for (int i=0; i<rel_input->FKeyList.GetSize(); i++)
	{
		if (rel_input->FKeyList[i]->ForeignKey->IsSubSet(attrs, size))
			result->FKeyList.Add(new FOREIGN_KEY(*rel_input->FKeyList[i]));
	}
	
	return result;
	
} // PROJECT::FindLogProp

//##ModelId=3B0C0874024D
CString PROJECT::Dump()
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
}

//##ModelId=3B0C08740101
SELECT::SELECT()
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_SELECT].New();
#ifdef _DEBUG
	name = GetName();		// for debug
#endif
};

//##ModelId=3B0C0874010A
SELECT::SELECT( SELECT& Op)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_SELECT].New();
#ifdef _DEBUG
	name = Op.name;		// for debug
#endif
};

//##ModelId=3B0C08740116
LOG_PROP* SELECT::FindLogProp (LOG_PROP ** input)
{
    LOG_COLL_PROP * rel_input = (LOG_COLL_PROP *) input[0];
    LOG_ITEM_PROP * pred_input = (LOG_ITEM_PROP *) input[1];
	
    double old_cucard, new_cucard, new_card;
    double sel = pred_input -> Selectivity;
	
    SCHEMA * new_schema = new SCHEMA(*(rel_input -> Schema));
    
    new_card = ceil(rel_input -> Card * sel);
    for (int i=0; i<new_schema->GetSize(); i++) 
	{
		old_cucard = (*new_schema)[i] -> CuCard;
		if (old_cucard!=-1) 
		{
			new_cucard = ceil(1 / (1/old_cucard - 1/(rel_input->Card) + 1/new_card) );
			
			// "select multiplicity error"
			ASSERT(new_cucard <= old_cucard + 1);
			
			(*new_schema)[i] -> CuCard = (float)new_cucard;
		}
    }
	
	KEYS_SET * cand_key;
	int * temp_key = rel_input->CandidateKey->CopyOut();
	cand_key = new KEYS_SET(temp_key, rel_input->CandidateKey->GetSize());
	delete [] temp_key;
	
	LOG_COLL_PROP * result = new LOG_COLL_PROP((float)new_card, (float)ceil(rel_input -> UCard * sel), 
		new_schema, cand_key ) ;
	
	//pass the foreign key info 
	for (i=0; i<rel_input->FKeyList.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*rel_input->FKeyList[i]);
		result->FKeyList.Add(fk);
	}
	
	return result;
	
} // SELECT::FindLogProp

//##ModelId=3B0C0874013D
ub4 SELECT::hash()
{
    ub4 hashval = GetInitval();
	
	return (hashval % (HtblSize -1) ) ;
}

//##ModelId=3B0C08740146
CString SELECT::Dump()
{
	return GetName();
}

//##ModelId=3B0C08740300
RM_DUPLICATES::RM_DUPLICATES()
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_RM_DUPLICATES].New();
#ifdef _DEBUG
	name = GetName();		// for debug
#endif
};

//##ModelId=3B0C08740309
RM_DUPLICATES::RM_DUPLICATES(RM_DUPLICATES & Op)
{
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_RM_DUPLICATES].New();
#ifdef _DEBUG
	name = Op.name;		// for debug
#endif
};

//##ModelId=3B0C08740314
LOG_PROP* RM_DUPLICATES::FindLogProp (LOG_PROP ** input)
{
    LOG_COLL_PROP * rel_input = (LOG_COLL_PROP *) input[0];
    
    float new_ucard, new_card;
    
    SCHEMA * new_schema = new SCHEMA(*(rel_input -> Schema));
    
    new_card = rel_input -> UCard;
	new_ucard = rel_input -> UCard;
    
	KEYS_SET * cand_key;
	int * temp_key = rel_input->CandidateKey->CopyOut();
	cand_key = new KEYS_SET(temp_key, rel_input->CandidateKey->GetSize());
	delete temp_key;
	
	LOG_COLL_PROP * result = new LOG_COLL_PROP(new_card, new_ucard, new_schema, cand_key ) ;
	
	//pass the foreign key info 
	for (int i=0; i<rel_input->FKeyList.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*rel_input->FKeyList[i]);
		result->FKeyList.Add(fk);
	}
	
	return result;
	
} // RM_DUPLICATES::FindLogProp

//##ModelId=3B0C0874033C
ub4 RM_DUPLICATES::hash()
{
    ub4 hashval = GetInitval();
	
	return (hashval % (HtblSize -1) ) ;
}

//##ModelId=3B0C08740345
CString RM_DUPLICATES::Dump()
{
	return GetName();
}

//##ModelId=3B0C0875009D
AGG_LIST::AGG_LIST(int * gby_atts, int gby_size, AGG_OP_ARRAY * agg_ops)
:GbyAtts(gby_atts), GbySize(gby_size), AggOps(agg_ops)
{
	//produce a flattened list
	if (AggOps) {
		int NumOps = AggOps->GetSize();
		FAttsSize = 0;
		int i, j, index;
		int * TempAtts;
		for (i=0; i< NumOps; i++ ) FAttsSize += (* AggOps)[i]->GetAttsSize();
		FlattenedAtts = new int [FAttsSize];
		index = 0;
		for (i=0; i< NumOps; i++)
		{
			TempAtts = CopyArray((* AggOps)[i]->GetAtts(), (* AggOps)[i]->GetAttsSize()); 
			for (j=0; j < (* AggOps)[i]->GetAttsSize(); j++, index++)
				FlattenedAtts[index] = TempAtts[j];
			delete [] TempAtts;
		}
	}
	else 
	{
		FlattenedAtts = 0;
		FAttsSize = 0;
	}
	
	if (TraceOn && !ForGlobalEpsPruning) ClassStat[C_AGG_LIST].New();	
#ifdef _DEBUG
	name = GetName();		// for debug
#endif
}

//##ModelId=3B0C087500EE
ub4 AGG_LIST::hash()
{
    ub4 hashval = GetInitval();
	int i;
    
	//to check the equality of the gby attributes
	for(i = GbySize ; --i >= 0 ;)
	{
		hashval = lookup2(GbyAtts[i],hashval );
	}
	
	//to check the equality of the FlattenedAtts
	for (i = FAttsSize; --i >= 0 ;)
	{
		hashval = lookup2(FlattenedAtts[i], hashval );
	}
	
	return (hashval % (HtblSize -1) ) ;
}

//##ModelId=3B0C087500BD
LOG_PROP* AGG_LIST::FindLogProp (LOG_PROP ** input)
{
	LOG_COLL_PROP * rel_input = (LOG_COLL_PROP *) input[0];
	SCHEMA * temp_schema = rel_input -> Schema -> projection( GbyAtts, GbySize);
	
    float new_cucard, gby_cucard;
	float new_card=1;
	const float card_reduction_factor = (float) 0.7;
    int i;
	
	bool CuCardKnown = true;
	for (i=0; i<GbySize; i++)
	{
		gby_cucard = (* temp_schema)[i]->CuCard;
		// check for overflow
		if (gby_cucard != -1) new_card *= gby_cucard;
		else
		{
			new_card =1;
			CuCardKnown = false;
			break;
		}
	}
    if (!CuCardKnown)
	{
		new_card = rel_input->Card * card_reduction_factor;
	}
	
    new_card = MIN(new_card, rel_input -> Card * card_reduction_factor);
	new_cucard = new_card;
    
	// add ATTR_EXP for every AGG_OP
	
	int NumOps = AggOps->GetSize();
	SCHEMA * agg_schema = new SCHEMA(NumOps);
	for (i=0; i< NumOps; i++)
	{
		AGG_OP * aggop = (* AggOps)[i];
		ATTR * new_attr = new ATTR(aggop->GetRangeVar(), aggop->GetAtts(), aggop->GetAttsSize());
		//CuCard is the same as group by
		new_attr->CuCard = new_cucard;
		agg_schema->AddAttr(i, new_attr);
	}
	
	agg_schema->TableNum = 1;
	agg_schema->TableId = new int[1];
	agg_schema->TableId[0] = 0;
	
	SCHEMA *result_schema = temp_schema->UnionSchema(agg_schema);
	delete temp_schema;
	delete agg_schema;
	
	//new candidate key is: 
	//if input candidate_key in GbyAtts, input candidate_key
	//else GbyAtts
	KEYS_SET * cand_key;
	
	if (rel_input->CandidateKey->IsSubSet(GbyAtts, GbySize))
	{
		int * temp_key = rel_input->CandidateKey->CopyOut();
		cand_key = new KEYS_SET(temp_key, rel_input->CandidateKey->GetSize());
		delete temp_key;
	}
	else
		cand_key = new KEYS_SET(GbyAtts, GbySize);
	
	LOG_COLL_PROP * result = new LOG_COLL_PROP(new_card, new_cucard, result_schema, cand_key ) ;
	
	//if foreign keys are subset of gby attrs, pass this foreign key
	for (i=0; i<rel_input->FKeyList.GetSize(); i++)
	{
		if (rel_input->FKeyList[i]->ForeignKey->IsSubSet(GbyAtts, GbySize))
			result->FKeyList.Add(new FOREIGN_KEY(*rel_input->FKeyList[i]));
	}
	return result;
} // AGG_LIST::FindLogProp

//##ModelId=3B0C087500EF
CString AGG_LIST::Dump()
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
} //AGG_LIST::Dump

//##ModelId=3B0C087500DB
bool AGG_LIST::operator== (OP * other)
{
	bool result;
	result = other->GetNameId() == GetNameId() && 
		EqualArray( ((AGG_LIST*)other)->GbyAtts, GbyAtts, GbySize) ;
	
	// traverse the agg_ops
	if (result)
	{
		int NumOps = AggOps->GetSize();
		for (int i=0; i<NumOps && result; i++)
		{
			AGG_OP * oth_op = ( * ((AGG_LIST*)other)->AggOps)[i];
			AGG_OP * thi_op = ( * AggOps)[i];
			
			result = (oth_op == thi_op);
		}
		
	}
	
	return result;
}//AGG_LIST::==

//##ModelId=3B0C08750224
LOG_PROP* FUNC_OP::FindLogProp (LOG_PROP ** input)
{
	LOG_COLL_PROP * rel_input = (LOG_COLL_PROP *) input[0];
    SCHEMA * temp_schema = new SCHEMA(*(rel_input -> Schema));
	
	SCHEMA * new_schema = new SCHEMA(1);
	ATTR * new_attr = new ATTR( RangeVar, Atts,	AttsSize);
	//the CuCard is the UCard of the input relation
	new_attr->CuCard = rel_input->UCard;
	new_schema->AddAttr(0, new_attr);
	
	new_schema->TableId = 0;
	new_schema->TableNum = 0;
	SCHEMA * result_schema = temp_schema->UnionSchema(new_schema);
	delete temp_schema;
	delete new_schema;
	
	KEYS_SET * cand_key;
	int * temp_key = rel_input->CandidateKey->CopyOut();
	cand_key = new KEYS_SET(temp_key, rel_input->CandidateKey->GetSize());
	delete temp_key;
	
	LOG_COLL_PROP * result = 
		new LOG_COLL_PROP(rel_input->Card, rel_input->UCard, result_schema, cand_key ) ;
	
	//pass the foreign key info 
	for (int i=0; i<rel_input->FKeyList.GetSize(); i++)
	{
		FOREIGN_KEY * fk = new FOREIGN_KEY(*rel_input->FKeyList[i]);
		result->FKeyList.Add(fk);
	}
	
	return result;
} // FUNC_OP::FindLogProp

//##ModelId=3B0C08750256
ub4 FUNC_OP::hash()
{
    ub4 hashval = GetInitval();
    
	//to check the equality of the attributes
	for(int i = AttsSize ; --i >= 0 ;)
	{
		hashval = lookup2(Atts[i],hashval );
	}
	
	return (hashval % (HtblSize -1) ) ;
}

//##ModelId=3B0C08750257
CString FUNC_OP::Dump()
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

