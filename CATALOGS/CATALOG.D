//Catalog text file
// 03-27-97

RelName: PART			// string_t 	
Card: 200000			// real_t
UCard: 200000			// real_t
Width: 0.1
Order: sorted				// Enum: sorted hashed heap
OrderKey: (P_PARTKEY)	// string_tCollection : Collection of Attribute
NumOfAttr: 9			// int_t
// Name (S)		Type (E)	UniqeCard(D)       Max (D)	Min (D)        Comment
P_PARTKEY 		real_t		200000		   200000	0	       Primary Key
P_NAME 			string_t	200000		   
P_MFGR 			string_t	100000
P_BRAND 		string_t	10000
P_TYPE 			string_t	1000	
P_SIZE 			int_t		10000		   2000000	1000
P_CONTAINER 	string_t	10000		   
P_RETAILPRICE 	int_t		20000		   999999999	0.01
P_COMMENT 		string_t	100		
NumofIndex: 2
//IndexName(S) 	Attribute (K)	Order (E) 
P_PARTKEY		(P_PARTKEY)		btree
P_NAME			(P_NAME)		hash

RelName: SUPPLIER
Card: 10000			// real_t
UCard: 200000			// real_t
Width: 0.1
Order: hashed			// Enum: sorted hashed heap
OrderKey: (S_SUPPKEY)	// string_tCollection : Collection of Attribute
NumOfAttr: 7			// int_t
// Name (S)	Type (E)	UniqeCard(D)       Max (D)   	Min (D)        Comment
S_SUPPKEY 	real_t		200000		   200000	0	       Primary Key
S_NAME 		string_t	200000		   
S_ADDRESS 	string_t	200000
S_NATIONKEY 	real_t	25		   24		0	Foreign_key to N
S_PHONE 	string_t	200000
S_ACCTBAL 	int_t		20000		   9999999.99	0.01	     
S_COMMENT 	string_t	100
NumofIndex: 1
//IndexName(S) 	Attribute (K)	Order (E) 
S_SUPPKEY		(S_SUPPKEY)		hash

RelName: PARTSUPP
Card: 800000		// real_t
UCard: 800000		// real_t
Width: 0.1
Order: sorted			// Enum: sorted hashed heap
OrderKey: (PS_PARTKEY,PS_SUPPKEY)	// string_tCollection : Collection of Attribute
NumOfAttr: 5			// int_t
// Name (S)	Type (E)	UniqeCard(D)       Max (D)   	Min (D)        Comment
PS_PARTKEY 	real_t		800000		   800000	0	Foreign_key  to P
PS_SUPPKEY 	real_t 		800000		   800000	0	Foreign_key to S
PS_AVAILQTY 	int_t		100000		   100000	0
PS_SUPPLYCOST 	int_t		20000		    9999999.99	0.01	     
PS_COMMENT 	string_t		100
NumofIndex: 1
//IndexName(S) 	Attribute (K)			Order (E) 
PS_PARTSUPPKEY	(PS_PARTKEY, PS_SUPPKEY)	btree

RelName: NATION
Card: 25			// real_t
UCard: 25			// real_t
Width: 0.1
Order: heap			// Enum: sorted hashed heap
OrderKey: ( )			// string_tCollection : Collection of Attribute
NumOfAttr: 4			// int_t
// Name (S)	Type (E)	UniqeCard(D)       Max (D)   	Min (D)        Comment
N_NATIONKEY 	real_t		25		   25		1	       Primary Key	
N_NAME 		string_t		25		
N_REGIONKEY 	real_t  	5		   5		1	Foreign_key to R
N_COMMENT 	string_t		10

RelName: REGION
Card: 5				// real_t
UCard: 5			// real_t
Width: 0.1
Order: heap			// Enum: sorted hashed heap
OrderKey: ( )			// string_tCollection : Collection of Attribute
NumOfAttr: 3			// int_t
// Name (S)	Type (E)	UniqeCard(D)       Max (D)   	Min (D)        Comment
R_REGIONKEY 	real_t		5		   5 		1	       Primary Key	
R_NAME 		string_t		5
R_COMMENT 	string_t		0
