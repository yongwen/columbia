/*	
main.cpp -  main file of the columbia optimizer
$Revision: 27 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#include "stdafx.h"
#include "cat.h"
#include "cm.h"
#include "tasks.h"
#include "physop.h"

#include "global.h"			// global variables


#define LINEWIDTH 256		// buffer length of one text line
#define KEYWORD_NUMOFQRY "NumOfQuery:"
#define KEYWORD_QUERY "Query:"
#define KEYWORD_PIGGYBACK "PiggyBack"

#ifdef _DEBUG
	#define new DEBUG_NEW
	 CMemoryState oldMemState, newMemState, diffMemState;
#endif

#ifdef _DEBUG
//Rule Firing Statistics
INT_ARRAY TopMatch;
INT_ARRAY Bindings;
INT_ARRAY Conditions;
#endif


/*************  DO THE OPTIMIZATION  ********************/
void Optimizer()
{
	//Initialize
	TaskNo = 0;
	Memo_M_Exprs = 0;
	SET_TRACE Trace(true);
	if(CatFile=="catalog") CatFile = AppDir + "\\catalog";	
	if(CMFile=="cm") CMFile = AppDir + "\\cm";			
	if(RSFile=="ruleset") RSFile = AppDir + "\\ruleset";
	
	//Open general trace file and COVE trace file, clear main output window
	OutputFile.Open( AppDir + "\\colout.txt" , CFile::modeCreate | CFile::modeWrite );
	OutputCOVE.Open( AppDir + "\\script.cove" , CFile::modeCreate | CFile::modeWrite );
	OutputWindow->ClearWindow();
    
	// clean the statistics
    for(int i=0; i < CLASS_NUM; i++)
    {
		ClassStat[i].Count = 0;
		ClassStat[i].Max = 0;
		ClassStat[i].Total = 0;
    }
	
#ifdef USE_MEMORY_MANAGER
	// Pointer to global memory manager.
	memory_manager = new MEMORY_MANAGER();
#endif
	
	// Create objects to manage Opt stats, Cost model, Rule set, Heuristic cost.
	OptStat = new OPT_STAT;	
	Cm = new CM(CMFile);
	PTRACE("cost model content:\r\n%s", Cm->Dump());
	RuleSet = new RULE_SET(RSFile);
	PTRACE("Rule set content:\r\n%s", RuleSet->Dump());
	COST *HeuristicCost;
	HeuristicCost = new COST(0);
	
#ifdef _DEBUG
	//Initialize Rule Firing Statistics
	TopMatch.SetSize(RuleSet->RuleCount);
	Bindings.SetSize(RuleSet->RuleCount);
	Conditions.SetSize(RuleSet->RuleCount);	//625
	for (int RuleNum = 0; RuleNum < RuleSet->RuleCount ; RuleNum++)
	{
		TopMatch[RuleNum] = 0;
		Bindings[RuleNum] = 0;
		Conditions[RuleNum] = 0;
	}
#endif
	
	//SQueryFile, BQueryFile have been set to the Single or Batch Query File chosen in
	//  the option dialog.  The CString QueryFile will be set to the name of a file 
	//  containing the query currently being optimized.  In the single query case that 
	//  will be SQueryFile, in the Batch Query case it will be "tempquery", which will
	//  contain the current batch query, copied from BQueryFile.  The default case is 
	//  handled separately, with files named query and bquery.
	//NumQuery will be set here to the number of queries.
	CString QueryFile;
	int NumQuery;
	FILE *fp;	//for the batch query file BQueryFile
	char TextLine[LINEWIDTH]; // text line buffer
	char *p;
	if (RadioVal == 1)	// Single Query case
	{
		if(SQueryFile=="query") QueryFile = AppDir + "\\query";	//default case
		else QueryFile = SQueryFile;  //value entered in option dialog; should check it exists.
		NumQuery = 1;
	}
	else if (RadioVal == 0)	//Batch Query case
	{
		//Open BQueryFile
		if(BQueryFile=="bquery") BQueryFile = AppDir + "\\bquery";	//default case
		if((fp = fopen(BQueryFile,"r"))==NULL) 
			OUTPUT_ERROR("can not open the file you chose in the option dialogue");
		fgets(TextLine,LINEWIDTH,fp);
		if(feof(fp))OUTPUT_ERROR("Empty Input File");
		
	}
	
	//The following loop is executed once for the single query case.
	//For the batch case, it is executed once for each batch query 
	//sequence. Executes batch of batches too.
	do
	{
		
		if (RadioVal == 0)    //Batch Query case
		{
			
			//Set NumQuery = Number of Queries in the batch
			//Set the PiggyBack flag to be true if the batch  
			//query needs to be PiggyBacked,else the PiggyBack 
			//flag remains false.
			
			PiggyBack = false;
			for (;;)
			{
				// skip the comment line
				if (IsCommentOrBlankLine(TextLine) ) {
					fgets(TextLine, LINEWIDTH, fp);
					continue;
				}
				
				p = SkipSpace(TextLine);
				if( p == strstr(p , KEYWORD_PIGGYBACK) )
				{
					p += strlen(KEYWORD_PIGGYBACK); 
					p = SkipSpace(p); 
					
					PiggyBack=true;
					
					fgets(TextLine,LINEWIDTH,fp);
					
					continue;
				}
				if( p == strstr(p , KEYWORD_NUMOFQRY) )
				{
					p += strlen(KEYWORD_NUMOFQRY); 
					p = SkipSpace(p);
					parseString(p); 
					
					NumQuery = atoi(p);	// NumQuery is set here
					break;
				}
				else
					OUTPUT_ERROR("the first line in the batch file should be number of queries");
			}
		}
		
		//Header for single line batch output
		if(SingleLineBatch)
			OUTPUT("%s", "#\tTTask\tTGrp\tCME\tTME\tFR\tCOST\r\n");
		
		//For each query numbered q
		bool first = true; //treat the first "Query: n" specially
		for (int q = 0; q < NumQuery; q++)
		{
			if (RadioVal == 0)	//Batch case
			{
				// reset the interesting queries & M_WINNERs, clean the statistics                                                 
				IntOrdersSet.reset();
#ifdef IRPROP
				//In PiggyBack mode remove the winners only when the first
				//query in a batch is created,for the remaining queries read in keep 
				//the winners created for previous queries optimized
				if ((PiggyBack && (0 == q)) || (!PiggyBack))
				{M_WINNER::mc.RemoveAll();}
				
#endif
				
				for(int i=0; i < CLASS_NUM; i++)
					ClassStat[i].Count = ClassStat[i].Max = ClassStat[i].Total = 0;
				OptStat->DupMExpr  = OptStat->FiredRule  = OptStat->HashedMExpr = 0;
				OptStat->MaxBucket = OptStat->TotalMExpr = 0;
#ifdef _DEBUG
				for (int RuleNum = 0; RuleNum < RuleSet->RuleCount; RuleNum++)
				{
					TopMatch[RuleNum] = 0;
					Bindings[RuleNum] = 0;
					Conditions[RuleNum] = 0;
				}
#endif
				TaskNo = 0;
				Memo_M_Exprs = 0;
				
				QueryFile = AppDir + "\\tempquery";
				FILE *tempfp;
				if ((tempfp = fopen(QueryFile.GetBuffer(200), "w")) == NULL)
					OUTPUT_ERROR("can not create or truncate file 'tempquery'");
				QueryFile.ReleaseBuffer(-1);
				/*
				here the optimizer reads in one query at a time  
				until the keyword KEYWORD_QUERY,KEYWORD_NUMOFQRY
				or KEYWORD_PIGGYBACK or end of a file is encountered 
				from each query, be it from a single,batch or batch 
				of batches query and stores it in a file called tempquery.
				The line containing keyword KEYWORD_QUERY,KEYWORD_NUMOFQRY
				or KEYWORD_PIGGYBACK serves as a demarcator for the optimizer 
				to break out of the for loop ,process and optimize the query 
				last read before reentering the loop to write another query 
				,if any ,starting reading after the line containing the 
				keyword KEYWORD_QUERY,KEYWORD_NUMOFQRY
				or KEYWORD_PIGGYBACK,to the tempquery file to be processed
				and optimized
				
				*/	 
				for (;;)
				{
					fgets(TextLine, LINEWIDTH, fp);
					if (feof(fp))
					{
						fprintf(tempfp, "%s", TextLine);
						break;
					}
					if (IsCommentOrBlankLine(TextLine)) continue;
					p = SkipSpace(TextLine);
					if (p == strstr(p, KEYWORD_QUERY)    ||
						p == strstr(p, KEYWORD_NUMOFQRY) ||
						p == strstr(p, KEYWORD_PIGGYBACK)) {
						if (first) {
							first=false;
							continue;
						}
						else break;
					}
					else
						fprintf(tempfp, "%s", TextLine);
				}
				fclose(tempfp);
				
			}
			
			//Print the number of the query
#ifndef _TABLE_
			if(SingleLineBatch)
			{
				OUTPUT("%d\t",q);	//First entry in output line in window
			}
			else
				OUTPUT("Query: %d\r\n", q+1);	//In this case it's a full line
#endif
			
			
			// if GlobepsPruning, run optimizer without globepsPruning
			// to get the heuristic cost
			if(GlobepsPruning)
			{
				GlobepsPruning      = false;
				ForGlobalEpsPruning = true;
				Cat                 = new CAT(CatFile);
				Query               = new QUERY(QueryFile);
				Ssp                 = new SSP;
				Ssp->Init();
				delete Query;
				Ssp->optimize();
				PHYS_PROP * PhysProp = CONT::vc[0]->GetPhysProp();
				*HeuristicCost = *(Ssp->GetGroup(0)->GetWinner(PhysProp)->GetCost());
				assert(Ssp->GetGroup(0)->GetWinner(PhysProp) ->GetDone());
				GlobalEpsBound = (*HeuristicCost) * (GLOBAL_EPS);
				delete Ssp;
				for (int i = 0; i < CONT::vc.GetSize(); i++)
					delete CONT::vc[i];
				CONT::vc.RemoveAll();
				delete Cat;
				GlobepsPruning      = true;
				ForGlobalEpsPruning = false;
			}
#ifdef _DEBUG
			oldMemState.Checkpoint();
#endif
			
			//Since each optimization corrupts the catalog, we must create it anew
			Cat = new CAT(CatFile);
			PTRACE("Catalog content:\r\n%s", Cat->Dump());
			
#ifdef _TABLE_
			assert(!SingleLineBatch);	//These are incompatible
			
			//	Print Heading: EPS ...
			OUTPUT("%s", "EPS, EPS_BD, CUREXPR, TOTEXPR, TASKS, OPTCOST\r\n");
			
			// For each iteration of the global epsilon counter ii {
			for (double ii = 0; ii <= GLOBAL_EPS*10; ii++)
			{
				OUTPUT("%3.1f\t", ii / 10 );
				GlobalEpsBound = (*HeuristicCost) * ii / 10;
				ClassStat[C_M_EXPR].Count = ClassStat[C_M_EXPR].Total = 0;
#endif
				
				//Parse and print the query and its interesting orders
				Query = new QUERY(QueryFile);
				PTRACE("Original Query:\r\n%s", Query->Dump());
				PTRACE("The interesting orders in the query are:\r\n%s\n", Query->Dump_IntOrders());
				
				//Initialize and print the search space, delete the query
				// In PiggyBack mode create the search space only for 
				// the first query(q == 0)and keep expanding it as more and 
				// more queries are read
				//In non PiggyBack mode create a new search space(Ssp) for  
				//every query read because the search space for the previous 
				//query is deleted after it is optimized
				if ((PiggyBack && (0 == q)) || (!PiggyBack))
				{
					Ssp = new SSP;
				}
				
				Ssp->Init();
				PTRACE("Initial Search Space:\r\n%s", Ssp->Dump());
				delete Query;
				
				//Keep track of initial space and time
				PTRACE("---1--- memory statistics before optimization: %s", DumpStatistics());
				PTRACE("used memory before opt: %dK\r\n",GetUsedMemory()/1000);
				struct _timeb start, finish;
				char *timeline;
				_ftime( &start ); 
				timeline = ctime(&(start.time));
				CString tmpbuf;
				tmpbuf.Format("%.8s.%0.3hu", &timeline[11], start.millitm);
#ifndef _TABLE_
				if(!SingleLineBatch)
					OUTPUT("Optimization beginning time:\t\t%s (hr:min:sec.msec)\r\n", tmpbuf);
#endif
				
				Ssp->optimize();	//Later add an input condition so we can handle ORDER BY
				
#ifndef _TABLE_
				//OUTPUT elapsed time
				long time;			//total seconds from start to finish
				unsigned short msecs;	//milliseconds from start to finish
				_ftime( &finish );
				if (finish.millitm >= start.millitm)
				{
					time  = finish.time - start.time;
					msecs = finish.millitm - start.millitm;
				}
				else
				{
					time  = finish.time - start.time - 1;
					msecs = 1000 + finish.millitm - start.millitm;
				}
				long hrs, mins, secs;	// Print differences from start to finish
				secs = time % 60;
				mins = ((time - secs)/60) % 60;
				hrs  = (time - secs - mins * 60) / 3600 ;
				tmpbuf.Format("%0.2d:%0.2d:%0.2d.%0.3d\r\n", hrs, mins, secs, msecs);
				if (!SingleLineBatch)
				{
					OUTPUT( "Optimization elapsed time:\t\t%s", tmpbuf);
					OUTPUT("%s", "========  OPTIMAL PLAN =========\r\n");
				}
#endif
				
				
				//CopyOut optimal plan. TRACE memory, search space.  
				PHYS_PROP * PhysProp = CONT::vc[0]->GetPhysProp();
				/* CopyOut the Optimal plan starting from the RootGID (the root group of
				our Query )
				*/
				Ssp->CopyOut(Ssp->GetRootGID(), PhysProp, 0);
				PTRACE("used memory after opt: %dK\r\n",GetUsedMemory() / 1000);
				PTRACE("---2--- memory statistics after optimization: %s", DumpStatistics());
				if (TraceFinalSSP) {
					Ssp->FastDump();
				}
				else {
					PTRACE("final Search Space:\r\n%s", Ssp->Dump());
				}
				
				//Delete Contexts, close batch query file, delete search space
				if(!PiggyBack)
				{
					for (i = 0; i < CONT::vc.GetSize(); i++) 
						delete CONT::vc[i];
					CONT::vc.RemoveAll();
				}
				//if (RadioVal ==0 && q==NumQuery-1) fclose(fp);
				PTRACE("used memory before deleting the search space: %dM\r\n", GetUsedMemory()/1000);
				// Go on with the usual procedure of deleting the search space before 
				// reading in the next query of the batch query file if not in the 
				// PiggyBack mode 
				// else keep the search space for reuse
				if (!PiggyBack)
					delete Ssp;
				PTRACE("---3--- memory statistics after freeing searching space: %s", DumpStatistics());
				
				//OUTPUT Rule Set Statistics
#ifdef  _DEBUG
#ifndef _TABLE_
				if (!SingleLineBatch)
					OUTPUT("%s",RuleSet -> DumpStats());
#endif
#endif
				
#ifdef _TABLE_
		}
#endif
		
		//Report memory, delete catalog
		PTRACE("used memory before deleting the catalog: %dM\r\n", GetUsedMemory()/1000);
		delete Cat;
	}	//for each query
	if (RadioVal) 
		break;  //If single query case, execute only once
    
	OUTPUT("%s","\r\n");
	OUTPUT("%s"," =============END OF A SEQUENCE================ ");
	OUTPUT("%s","\r\n");
	
	//this will be executed only if in PiggyBack mode 
	//to delete the search space one last time
	if (PiggyBack) 
	{
		delete Ssp;
		for (i = 0; i < CONT::vc.GetSize(); i++)
			delete CONT::vc[i];
		CONT::vc.RemoveAll();
	}
	
   } while (!feof(fp));  // end of do loop  over each batch query sequence
   
   
   
	  //Free optimization stat object, cost model, rule set, heuristic cost
	  delete OptStat;	
	  delete Cm;		
	  delete RuleSet;	
	  delete (void*) HeuristicCost; 
	  
#ifdef USE_MEMORY_MANAGER
	  PTRACE("used memory before delete manager: %dM\r\n", GetUsedMemory()/1000);
	  delete memory_manager ;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	  
#endif
	  PTRACE("used memory after delete manager: %dM\r\n", GetUsedMemory()/1000);
	  
#ifdef _DEBUG
	  newMemState.Checkpoint();
	  if (diffMemState.Difference(oldMemState, newMemState))
	  {
		  PTRACE("%s", "Memory leaked after optimizer!\n") ;
		  oldMemState.DumpAllObjectsSince();
		  diffMemState.DumpStatistics();
	  }
#endif
	  OutputFile.Close();
	  OutputCOVE.Close();
}
