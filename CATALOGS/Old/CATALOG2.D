//Catalog model D text file full name
// 06-22-98 Yu

RelName: LINEITEM			// string_t 	
Card: 6000000			// real_t
UCard: 6000000			// real_t
Width: 0.1
Order: sorted				// Enum: sorted hashed heap
OrderKey: (L_ORDERKEY, L_LINENUMBER)	// string_tCollection : Collection of Attribute
NumOfAttr: 18			// int_t
// Name (S)		Type (E)	UniqeCard(D)       Max (D)	Min (D)        Comment
L_ORDERKEY 		real_t		1500000		   			Foreign_key to O
L_PARTKEY 		real_t		200000		   			Foreign_key to P
L_SUPPKEY		real_t		10000					Foreign_key to S
L_LINENUMBER 		int_t		4
L_QUANTITY 		int_t		50		   50		1		
L_EXTENDEDPRICE		real_t		6000000		   
L_DISCOUNT	 	real_t		11		   0.1		0		   
L_TAX		 	real_t		9		   0.08		0
L_RETURNFLAG		string_t	3
L_LINESTATUS		string_t	2	
L_SHIPDATE		int_t		2556		   2555		0
L_SHIPYEAR		int_t		7		   98		92
L_COMMITDATE		int_t		2556		   2555		0
L_COMMITYEAR		int_t		7		   98		92
L_RECEIPTDATE		int_t		2556		   2555		0
L_RECEIPTYEAR		int_t		7		   98		92
L_SHIPINSTRUCT		string_t	4
L_SHIPMODE		string_t	7		
NumofIndex: 2
//IndexName(S) 	Attribute (K)	Order (E) 
L_PARTKEY		(L_PARTKEY)		btree
L_SHIPDATE		(L_SHIPDATE)		btree

RelName: CUSTOMER		// string_t 	
Card: 150000			// real_t
UCard: 150000			// real_t
Width: 0.1
Order: sorted				// Enum: sorted hashed heap
OrderKey: (C_CUSTKEY)		// string_tCollection : Collection of Attribute
NumOfAttr: 8			// int_t
// Name (S)		Type (E)	UniqeCard(D)       Max (D)	Min (D)        Comment
C_CUSTKEY 		real_t		150000		   			primary key 
C_NATIONKEY 		real_t		25		   			
C_NAME			string_t	150000					
C_ADDRESS 		string_t	150000
C_PHONE 		string_t	150000		 
C_ACCTBAL		real_t		150000		   10000	-1000
C_MKTSEGMENT	 	real_t		5		
C_COMMENT		string_t	150000
NumofIndex: 1
//IndexName(S) 	Attribute (K)	Order (E) 
C_CUSTKEY		(C_CUSTKEY)		btree

RelName: ORDER			// string_t 	
Card: 1500000			// real_t
UCard: 1500000			// real_t
Width: 0.1
Order: sorted			// Enum: sorted hashed heap
OrderKey: (O_ORDERKEY)		// string_tCollection : Collection of Attribute
NumOfAttr: 7			// int_t
// Name (S)		Type (E)	UniqeCard(D)       Max (D)	Min (D)        Comment
O_ORDERKEY 		real_t		1500000			   			primary key 
O_CUSTKEY 		real_t		150000		   			
O_ORDERSTATUS		string_t	3					
O_ORDERDATE 		int_t		2556		   2555		0
O_ORDERYEAR		int_t		7		   98	        92		 
O_ORDERPRIORITY		string_t	5		 
O_CLERK		 	string_t	1000		
NumofIndex: 1
//IndexName(S) 	Attribute (K)	Order (E) 
O_ORDERKEY		(O_ODERKEY)		btree
