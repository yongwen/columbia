/* 	
CM.cpp -  implementation of COST MODEL parser
$Revision: 4 $
Implements classes in cm.h
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
  
*/
#include "stdafx.h"
#include "cm.h"

#define LINEWIDTH 256		// buffer length of one text line

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

// Keyword definition
#define KEYWORD_CPU_READ 		"CPU_READ:"
#define KEYWORD_TOUCH_COPY		"TOUCH_COPY:"
#define KEYWORD_CPU_PRED		"CPU_PRED:"
#define KEYWORD_CPU_APPLY 		"CPU_APPLY:"
#define KEYWORD_CPU_COMP_MOVE 	"CPU_COMP_MOVE:"
#define KEYWORD_HASH_COST 		"HASH_COST:"
#define KEYWORD_HASH_PROBE		"HASH_PROBE:"
#define KEYWORD_INDEX_PROBE 	"INDEX_PROBE:"
#define KEYWORD_BF				"BF:"
#define KEYWORD_INDEX_BF	 	"INDEX_BF:"
#define KEYWORD_IO				"IO:"
#define KEYWORD_BIT_BF			"BIT_BF:"

#define READ_IN(KEYWORD,Value)	if( p == strstr(p , KEYWORD) ) { \
									p += strlen(KEYWORD);		 \
									p = SkipSpace(p);			 \
									parseString(p);				 \
									Value = atof(p);			 \
									continue;	}
		
#define ValueToString(KEYWORD,Value)   temp.Format("%s %f\r\n", KEYWORD,Value); \
										os += temp;

// read catalog text file and store the information into CAT 
//##ModelId=3B0C0877015F
CM::CM(CString filename)
{
	FILE *fp;		// file handle
	char TextLine[LINEWIDTH]; // text line buffer
	char *p;
	
	if((fp = fopen(filename,"r"))==NULL) 
		OUTPUT_ERROR("can not open CM file");
	
	for(;;)
	{
		fgets(TextLine,LINEWIDTH,fp);
		if(feof(fp)) break;
		
		// skip the comment line
		if(IsCommentOrBlankLine(TextLine)) continue;
		
		p = SkipSpace(TextLine);
		
		READ_IN ( KEYWORD_CPU_READ , CPU_READ );
		READ_IN ( KEYWORD_TOUCH_COPY, TOUCH_COPY );
		READ_IN ( KEYWORD_CPU_PRED , CPU_PRED );
		READ_IN ( KEYWORD_CPU_APPLY , CPU_APPLY );
		READ_IN ( KEYWORD_CPU_COMP_MOVE , CPU_COMP_MOVE );
		READ_IN ( KEYWORD_HASH_COST , HASH_COST );		
		READ_IN ( KEYWORD_HASH_PROBE , HASH_PROBE );
		READ_IN ( KEYWORD_INDEX_PROBE , INDEX_PROBE );
		READ_IN ( KEYWORD_BF , BF );
		READ_IN ( KEYWORD_INDEX_BF , INDEX_BF );
		READ_IN ( KEYWORD_IO , IO );
		READ_IN ( KEYWORD_BIT_BF, BIT_BF);
		
	}	// end of parsing cm loop
	
	fclose(fp);
}

//##ModelId=3B0C0877016A
CString CM::Dump()
{
	CString os;
	CString temp;
	
	ValueToString( KEYWORD_CPU_READ , CPU_READ );
	ValueToString( KEYWORD_TOUCH_COPY, TOUCH_COPY );
	ValueToString( KEYWORD_CPU_PRED , CPU_PRED );
	ValueToString( KEYWORD_CPU_APPLY , CPU_APPLY );
	ValueToString( KEYWORD_CPU_COMP_MOVE , CPU_COMP_MOVE );
	ValueToString( KEYWORD_HASH_COST , HASH_COST );		
	ValueToString ( KEYWORD_HASH_PROBE , HASH_PROBE );
	ValueToString ( KEYWORD_INDEX_PROBE , INDEX_PROBE );
	ValueToString ( KEYWORD_BF , BF );
	ValueToString ( KEYWORD_INDEX_BF , INDEX_BF );
	ValueToString ( KEYWORD_IO , IO );
	ValueToString ( KEYWORD_BIT_BF , BIT_BF );
	
	return os;
}
