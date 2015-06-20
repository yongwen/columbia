// Query Tree Text file  
// only one query per file, LISP style. (indenting is not required)
// supported Log Ops:  	( GET(sName) )
//			( EQJOIN(sKey), sTable1, sTable2 )	
//			( PROJECT(sKey), sTable )
//  ?????
// Grammar: 	Table 	-> 	GET(QuoStr) |  
//			 	( EQJOIN(Keys), Table, Table) |
//				( PROJECT(Keys), Table) |	
//				( SELECT(Pred), Table)
//		Pred 	->	op(Item [,Item]*)
//		Item	->	ATTR |
//				INT(QuoStr) |
//				STR(QuoStr)
//		op	->	OP_EQ | OP_LT | OP_GT 
//		Keys 	->	Attr [,Attr]*
//		Attr	->	string.string
//		QuoStr	->	"string"		

// sample 1 : join(A,B)
// output: 	getAExpr = new GET("A")
//		getBExpr = new GET("B")
//		keys1 = new KEYS("A","X")
//		keys2 = new KEYS("B","Z")
//		joinOp = new EQJOIN(keys1,keys2)
//		joinExpr = new EXPR(joinOp, getAExpr, getBExpr)
//(GET("A"))

//(EQJOIN (A.X , B.X), 
//        GET ("A"), 
//        GET ("B") 
//)

(EQJOIN (PART.P_PARTKEY , SUPPLIER.S_NAME), 
    (EQJOIN (PART.P_NAME , PARTSUPP.PS_SUPPKEY), 
        GET ("PART"), 
        GET ("PARTSUPP") 
    ),
    GET ("SUPPLIER")
)
