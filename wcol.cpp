// wcol.cpp : Defines the class behaviors for the application.
//	$Revision: 8 $

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LINELENGTH 80

extern CString CatFile;
extern CString SQueryFile;
extern CString BQueryFile;
extern CString CMFile;
extern CString RSFile;
extern CString AppDir;

extern double GLOBAL_EPS;
extern bool FileTrace;		// global trace flag
extern bool WindowTrace;	// global trace flag
extern bool COVETrace;		// global trace flag
extern bool TraceFinalSSP;	// global trace flag
extern bool TraceOPEN;		// global trace flag
extern bool TraceSSP;		// global trace flag
extern bool Pruning;		// global pruning flag
extern bool CuCardPruning;	// global cucard pruning flag
extern bool GlobepsPruning;	// global epsilon pruning flag
extern int  RadioVal;		// radio value for query file
extern bool SingleLineBatch;// Single Line for Batch Query Output
extern bool Halt;			// halt flat
extern int  HaltGrpSize;	// halt when number of plans equals to 100% of group
extern int  HaltWinSize;    // window size for checking the improvement
extern int  HaltImpr;		// halt when the improvement is less than 20%

// global declaration
CWcolView* OutputWindow;
void Optimizer();

/////////////////////////////////////////////////////////////////////////////
// The one and only CWcolApp object
CWcolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWcolApp

BEGIN_MESSAGE_MAP(CWcolApp, CWinApp)
//{{AFX_MSG_MAP(CWcolApp)
ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
//ON_COMMAND(ID_APP_EXIT, OnAppExit)
//}}AFX_MSG_MAP
// Standard file based document commands
ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWcolApp construction

//##ModelId=3B0C085B01E1
CWcolApp::CWcolApp()
{
	m_hwndDialog = NULL;
	m_gpToolTip = NULL;
};

/////////////////////////////////////////////////////////////////////////////
// CWcolApp initialization

//##ModelId=3B0C085B023B
BOOL CWcolApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	
	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWcolDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWcolView));
	AddDocTemplate(pDocTemplate);
	
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	
	char buffer[_MAX_PATH];
	_getcwd( buffer, _MAX_PATH );  // get current working directory
	AppDir = buffer;
	
	CString OptionFile = AppDir + "\\option";
	ifstream fin(OptionFile.GetBuffer(OptionFile.GetLength()), ios::nocreate);
	
	// read in last option
	char TextLine[LINELENGTH];
	int length;
	
	if (fin)
	{
		// read CatFile option 
		fin.getline(TextLine, LINELENGTH);
		length = strlen(TextLine);
		CatFile.GetBufferSetLength(length);
		strncpy(CatFile.GetBuffer(length), TextLine, length);
		CatFile.ReleaseBuffer();
		
		// read QueryFile option 
		fin.getline(TextLine, LINELENGTH);
		length = strlen(TextLine);
		SQueryFile.GetBufferSetLength(length);
		strncpy(SQueryFile.GetBuffer(length), TextLine, length);
		SQueryFile.ReleaseBuffer();
		
		// read CmFile option 
		fin.getline(TextLine, LINELENGTH);
		length = strlen(TextLine);
		CMFile.GetBufferSetLength(length);
		strncpy(CMFile.GetBuffer(length), TextLine, length);
		CMFile.ReleaseBuffer();
		
		// read Rule Set File option 
		fin.getline(TextLine, LINELENGTH);
		length = strlen(TextLine);
		RSFile.GetBufferSetLength(length);
		strncpy(RSFile.GetBuffer(length), TextLine, length);
		RSFile.ReleaseBuffer();
		
		//read GLOBAL_EPS option
		fin.getline(TextLine, LINELENGTH);
		double tempEPS = atof(TextLine);
		GLOBAL_EPS = tempEPS;
		
		//read FileTrace, TraceFinalSSP, TraceOPEN, TraceSSP, WindowTrace
		//Pruning, CuCardPruning, GlobepsPruning, COVETrace
		fin.getline(TextLine, LINELENGTH);
		FileTrace = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		TraceFinalSSP = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		TraceOPEN = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		TraceSSP = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		WindowTrace = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		Pruning = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		CuCardPruning = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		GlobepsPruning = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		COVETrace = ( (strcmp(TextLine, "T") == 0) ? true : false);
		
		// read BQueryFile option 
		//OptionFile.Read(TextLine, LINELENGTH);
		fin.getline(TextLine, LINELENGTH);
		length = strlen(TextLine);
		BQueryFile.GetBufferSetLength(length);
		strncpy(BQueryFile.GetBuffer(length), TextLine, length);
		BQueryFile.ReleaseBuffer();
		
		//read RadioVal
		fin.getline(TextLine, LINELENGTH);
		int tempRV = atoi(TextLine);
		RadioVal = tempRV;
		
		//read Single Line Batch Output parameter
		fin.getline(TextLine, LINELENGTH);
		SingleLineBatch = ( (strcmp(TextLine, "T") == 0) ? true : false);
		
		//read Halt option and parameters
		fin.getline(TextLine, LINELENGTH);
		Halt = ( (strcmp(TextLine, "T") == 0) ? true : false);
		fin.getline(TextLine, LINELENGTH);
		HaltGrpSize = atoi(TextLine);
		fin.getline(TextLine, LINELENGTH);
		HaltWinSize = atoi(TextLine);
		fin.getline(TextLine, LINELENGTH);
		HaltImpr = atoi(TextLine);
		
		fin.close();
	}
	
	OptionFile.ReleaseBuffer();
	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->SetWindowText("Columbia Optimizer for Windows");
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

//##ModelId=3B0C08790003
class CAboutDlg : public CDialog
{
public:
	//##ModelId=3B0C08790018
	CAboutDlg();
	
	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	//##ModelId=3B0C08790021
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//##ModelId=3B0C08790018
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

//##ModelId=3B0C08790021
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
//##ModelId=3B0C085B0251
void CWcolApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
//{{AFX_MSG_MAP(CMainFrame)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

//##ModelId=3B0C085B028C
CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

//##ModelId=3B0C085B029F
CMainFrame::~CMainFrame()
{
}

//##ModelId=3B0C085B0295
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
//##ModelId=3B0C085B02A9
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

//##ModelId=3B0C085B02B3
void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWcolDoc

IMPLEMENT_DYNCREATE(CWcolDoc, CDocument)

BEGIN_MESSAGE_MAP(CWcolDoc, CDocument)
//{{AFX_MSG_MAP(CWcolDoc)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWcolDoc construction/destruction

//##ModelId=3B0C085B02FA
CWcolDoc::CWcolDoc()
{
};

//##ModelId=3B0C085B030F
CWcolDoc::~CWcolDoc()
{
};

//##ModelId=3B0C085B0303
BOOL CWcolDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	
	return TRUE;
}

//##ModelId=3B0C085B0305
void CWcolDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CWcolDoc diagnostics

#ifdef _DEBUG
//##ModelId=3B0C085B0318
void CWcolDoc::AssertValid() const
{
	CDocument::AssertValid();
}

//##ModelId=3B0C085B0322
void CWcolDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CWcolView

IMPLEMENT_DYNCREATE(CWcolView, CEditView)

BEGIN_MESSAGE_MAP(CWcolView, CEditView)
//{{AFX_MSG_MAP(CWcolView)
ON_COMMAND(IDD_BEGIN, OnBegin)
ON_COMMAND(IDD_MEMORY, OnMemory)
ON_COMMAND(ID_OPTION, OnOption)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWcolView construction/destruction

//##ModelId=3B0C085B03AE
CWcolView::CWcolView()
{
};

//##ModelId=3B0C085B03DF
CWcolView::~CWcolView()
{
};

//##ModelId=3B0C085B03D5
BOOL CWcolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	
	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping
	
	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CWcolView drawing

//##ModelId=3B0C085B03CB
void CWcolView::OnDraw(CDC* pDC)
{
	CWcolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CWcolView diagnostics

#ifdef _DEBUG
//##ModelId=3B0C085B03E1
void CWcolView::AssertValid() const
{
	CEditView::AssertValid();
}

//##ModelId=3B0C085C0002
void CWcolView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

//##ModelId=3B0C085B03B7
CWcolDoc* CWcolView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWcolDoc)));
	return (CWcolDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWcolView message handlers

//##ModelId=3B0C085C000D
void CWcolView::OnBegin() 
{
	CEdit& EditCtrl = GetEditCtrl();
	
	GetEditCtrl().SetWindowText("");
	
	EditCtrl.SetReadOnly(false);	// set read only property
	GetEditCtrl().SetSel(0,-1);
	GetEditCtrl().Clear();
	
	//EditCtrl.SetLimitText(0x7fffffff);	// set the text limit to MAX
	
	OutputWindow  = this;
	
	// begin optimizing 
	Optimizer();
	EditCtrl.SetReadOnly(true);	// set read only property
	GetDocument()->SetModifiedFlag(false);
}

// clear the view window
//##ModelId=3B0C085B03C2
void CWcolView::ClearWindow()
{
	GetEditCtrl().SetSel(0,-1);
	GetEditCtrl().Clear();
}

// append output message to the View window
//##ModelId=3B0C085B03B8
void CWcolView::Print(LPCTSTR lpszMessage)
{
	GetEditCtrl().ReplaceSel(lpszMessage);
}

// outside reference
CString Trim(CString);

// check memory usage
//##ModelId=3B0C085C0016
void CWcolView::OnMemory() 
{
#define MAX_CHARS	100
#define MIN_MEM		0x00400000;
#define  MAX_MEM	0xC0000000
	
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	
	LPVOID lpScanAt = (LPVOID) MIN_MEM;
	LPVOID lpRegionBase = NULL;
	CString out_string;
	
	MEMORY_BASIC_INFORMATION mbi;
	char szModuleName[MAX_CHARS];
	
	while((LONG) lpScanAt < MAX_MEM)
	{
		VirtualQuery(lpScanAt, &mbi, sizeof(mbi));
		if(mbi.AllocationBase != lpRegionBase)
		{
			lpRegionBase = mbi.AllocationBase;
			lpScanAt = (LPVOID) ((PBYTE) lpScanAt + mbi.RegionSize);
			if(GetModuleFileName((HINSTANCE)lpRegionBase, 
				szModuleName, MAX_CHARS))
			{
				out_string.Format("Module: %s\r\nBaseAddress: %#x Size: %d\r\nMemoryUsed: %d", 
					Trim(szModuleName), lpRegionBase, mbi.RegionSize, 
					ms.dwTotalVirtual - ms.dwAvailVirtual);
				break;
			}
		}
		lpScanAt = (LPVOID) ((PBYTE) lpScanAt + si.dwPageSize);
	}
	
	AfxMessageBox(out_string);
}

//##ModelId=3B0C085C0020
void CWcolView::OnOption() 
{
	
	COptionDialog OptionDlg;
	
	OptionDlg.DoModal();
}
/////////////////////////////////////////////////////////////////////////////
// COptionDialog dialog


//##ModelId=3B0C085C0200
COptionDialog::COptionDialog(CWnd* pParent /*=NULL*/)
: CDialog(COptionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionDialog)
	m_catfile = CatFile;
	m_queryfile = SQueryFile;
	m_batchqueryfile = BQueryFile;
	m_radio = RadioVal;
	m_singlelinebatch = SingleLineBatch;
	m_covetrace = COVETrace;
	m_cucardpruning = CuCardPruning;
	m_filetrace = FileTrace;
	m_pruning = Pruning;
	m_windowtrace = WindowTrace;
	m_cmfile = CMFile;
	m_epspruning = GlobepsPruning;
	m_eps = GLOBAL_EPS;
	m_halt = Halt;
	m_halt_grpsize = HaltGrpSize;
	m_halt_winsize = HaltWinSize;
	m_halt_impr = HaltImpr;
	m_finalssp = TraceFinalSSP;
	m_open = TraceOPEN;
	m_ssp = TraceSSP;
	m_rsfile = RSFile;
	//}}AFX_DATA_INIT
	m_pToolTip = NULL;
}

//##ModelId=3B0C085C0202
COptionDialog::~COptionDialog()
{
	
	if (m_pToolTip != NULL) delete m_pToolTip;
}

//##ModelId=3B0C085C03BA
BOOL COptionDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (!m_pToolTip)
	{
		int rt;
		m_pToolTip = new CToolTipCtrl;
		rt = m_pToolTip->Create(this);
		ASSERT(rt != 0);
		((CWcolApp*)AfxGetApp())->m_gpToolTip = m_pToolTip;
		
		CRect rc(11,17,29,37);
		MapDialogRect(rc);
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_FILE), "Write all trace information to the file colout.txt");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_WINDOW), "Write all trace information to the window");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_COVE), "Write input for COVE (Columbia Optimizer Visualizer Environment) to cove.txt");
		ASSERT( rc!=0 );
		
		//rt = m_pToolTip->AddTool(this, "Option setting");
		//ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_FINALSSP), "Print the final search space in colout.txt");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_OPEN), "Trace all OPEN task lists to colout.txt");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_SSP), "Trace search space after every task, to colout.txt");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_PRUNING), "Aggressively check limits at all times");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_CUCARD), "In addition to group pruning, use lower bound pruning");
		ASSERT( rc!=0 );
		
		rt = m_pToolTip->AddTool(GetDlgItem(IDC_GBLEPS), "If a plan costs <= GLOBAL_EPS, it is a winner for G");
		ASSERT( rc!=0 );
		
		m_pToolTip->Activate(TRUE);
	}
	
	((CWcolApp*)AfxGetApp())->m_hwndDialog = m_hWnd;
	CWnd* BatchCtrl1;
	CWnd* BatchCtrl2;
	CWnd* BatchCtrl3;
	CWnd* SingleCtrl1;
	CWnd* SingleCtrl2;
	CWnd* SingleCtrl3;
	
	BatchCtrl1 = GetDlgItem(IDC_QUERYEDIT2);
	BatchCtrl2 = GetDlgItem(IDC_BROWSE5);
	BatchCtrl3 = GetDlgItem(IDC_EDIT5);
	SingleCtrl1 = GetDlgItem(IDC_QUERYEDIT);
	SingleCtrl2 = GetDlgItem(IDC_BROWSE2);
	SingleCtrl3 = GetDlgItem(IDC_EDIT2);
	if (m_radio == 0)
	{
		BatchCtrl1->EnableWindow(TRUE);
		BatchCtrl2->EnableWindow(TRUE);
		BatchCtrl3->EnableWindow(TRUE);
		SingleCtrl1->EnableWindow(FALSE);
		SingleCtrl2->EnableWindow(FALSE);
		SingleCtrl3->EnableWindow(FALSE);
	}
	else if (m_radio ==1)
	{
		BatchCtrl1->EnableWindow(FALSE);
		BatchCtrl2->EnableWindow(FALSE);
		BatchCtrl3->EnableWindow(FALSE);
		SingleCtrl1->EnableWindow(TRUE);
		SingleCtrl2->EnableWindow(TRUE);
		SingleCtrl3->EnableWindow(TRUE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//##ModelId=3B0C085C034A
void COptionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionDialog)
	DDX_Text(pDX, IDC_CATFILE_EDIT, m_catfile);
	DDX_Text(pDX, IDC_QUERYEDIT, m_queryfile);
	DDX_Text(pDX, IDC_QUERYEDIT2, m_batchqueryfile);
	DDX_Radio(pDX, IDC_RADIO_BATCH, m_radio);
	DDX_Check(pDX, IDC_COVE, m_covetrace);
	DDX_Check(pDX, IDC_CUCARD, m_cucardpruning);
	DDX_Check(pDX, IDC_FILE, m_filetrace);
	DDX_Check(pDX, IDC_PRUNING, m_pruning);
	DDX_Check(pDX, IDC_WINDOW, m_windowtrace);
	DDX_Text(pDX, IDC_CMEDIT, m_cmfile);
	DDX_Check(pDX, IDC_GBLEPS, m_epspruning);
	DDX_Text(pDX, IDC_EPS_EDIT, m_eps);
	DDX_Check(pDX, IDC_HALT, m_halt);
	DDX_Text(pDX, IDC_HALT_GRPSIZE, m_halt_grpsize);
	DDX_Text(pDX, IDC_HALT_WINSIZE, m_halt_winsize);
	DDX_Text(pDX, IDC_HALT_IMPR, m_halt_impr);
	DDX_Check(pDX, IDC_FINALSSP, m_finalssp);
	DDX_Check(pDX, IDC_OPEN, m_open);
	DDX_Check(pDX, IDC_SSP, m_ssp);
	DDX_Text(pDX, IDC_RULESETEDIT, m_rsfile);
	DDX_Check(pDX, IDC_CHECK5, m_singlelinebatch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionDialog, CDialog)
//{{AFX_MSG_MAP(COptionDialog)
ON_BN_CLICKED(IDC_BROWSE1, OnBrowse1)
ON_BN_CLICKED(IDC_BROWSE2, OnBrowse2)
ON_BN_CLICKED(IDC_BROWSE3, OnBrowse3)
ON_BN_CLICKED(IDC_EDIT1, OnEdit1)
ON_BN_CLICKED(IDC_EDIT2, OnEdit2)
ON_BN_CLICKED(IDC_EDIT3, OnEdit3)
ON_BN_CLICKED(IDC_BROWSE4, OnBrowse4)
ON_BN_CLICKED(IDC_EDIT4, OnEdit4)
ON_BN_CLICKED(IDC_RADIO_BATCH, OnRadioBatch)
ON_BN_CLICKED(IDC_RADIO_SINGLE, OnRadioSingle)
ON_BN_CLICKED(IDC_BROWSE5, OnBrowse5)
ON_BN_CLICKED(IDC_EDIT5, OnEdit5)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionDialog message handlers

//Catalog
//##ModelId=3B0C085C0369
void COptionDialog::OnBrowse1() 
{
	
	CFileDialog FileDlg(true);
	CString temp = AppDir + "\\Catalogs\\";
	FileDlg.m_ofn.lpstrInitialDir = temp;
	
	
	if( FileDlg.DoModal() == IDOK) 
	{
		m_catfile = FileDlg.GetPathName();
		SetDlgItemText(IDC_CATFILE_EDIT,m_catfile);
	}
}

//Single Query
//##ModelId=3B0C085C0373
void COptionDialog::OnBrowse2() 
{
	CFileDialog FileDlg(true);
	CString temp = AppDir + "\\Queries\\";
	FileDlg.m_ofn.lpstrInitialDir = temp;
	
	if( FileDlg.DoModal() == IDOK) 
	{
		m_queryfile = FileDlg.GetPathName();
		SetDlgItemText(IDC_QUERYEDIT,m_queryfile);
	}
	
}

#pragma warning(disable:4800)		// use of bool in wcol.cpp, casting int to bool takes time

//##ModelId=3B0C085C0375
void COptionDialog::OnOK() 
{
	
	UpdateData();
	CFile OptionFile;
	OptionFile.Open( AppDir + "\\option" , CFile::modeCreate | CFile::modeWrite );
	
	//write the options to a file
	CString tempString;
	tempString = m_catfile;
	tempString += "\n";
	OptionFile.Write(tempString, tempString.GetLength());
	tempString = m_queryfile;
	tempString += "\n";
	OptionFile.Write(tempString, tempString.GetLength());
	tempString = m_cmfile;
	tempString += "\n";
	OptionFile.Write(tempString, tempString.GetLength());
	tempString = m_rsfile;
	tempString += "\n";
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%f\n", m_eps);
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_filetrace ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_finalssp ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_open ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_ssp ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_windowtrace ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_pruning ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_cucardpruning ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_epspruning ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_covetrace ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString = m_batchqueryfile;
	tempString += "\n";
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%d\n", m_radio);
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_singlelinebatch ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%s\n", (m_halt ? "T" : "F"));
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%d\n", m_halt_grpsize);
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%d\n", m_halt_winsize);
	OptionFile.Write(tempString, tempString.GetLength());
	tempString.Format("%d\n", m_halt_impr);
	OptionFile.Write(tempString, tempString.GetLength());
	OptionFile.Close();
	
	CatFile = m_catfile;
	SQueryFile = m_queryfile;
	BQueryFile = m_batchqueryfile;
	RadioVal = m_radio;
	SingleLineBatch = m_singlelinebatch;
	CMFile = m_cmfile;
	RSFile = m_rsfile;
	GLOBAL_EPS = m_eps;
	FileTrace = m_filetrace;
	
	TraceFinalSSP = m_finalssp ;
	TraceOPEN = m_open ;
	TraceSSP = m_ssp ;
	
	WindowTrace = m_windowtrace;
	Pruning = m_pruning;
	CuCardPruning = m_cucardpruning;
	GlobepsPruning = m_epspruning;
	COVETrace = m_covetrace;
	
	Halt = m_halt;
	HaltGrpSize = m_halt_grpsize;
	HaltWinSize = m_halt_winsize;
	HaltImpr = m_halt_impr;
	
	if( CuCardPruning && ! Pruning ) 
		AfxMessageBox("CuCard pruning must come with group pruning option checked!");
	else
		CDialog::OnOK();
}

//##ModelId=3B0C085C037E
void COptionDialog::OnCancel() 
{	
	CDialog::OnCancel();
}

//Cost Models
//##ModelId=3B0C085C0388
void COptionDialog::OnBrowse3() 
{
	CFileDialog FileDlg(true);
	CString temp = AppDir + "\\CMs\\";
	FileDlg.m_ofn.lpstrInitialDir = temp;
	
	if( FileDlg.DoModal() == IDOK) 
	{
		m_cmfile = FileDlg.GetPathName();
		SetDlgItemText(IDC_CMEDIT,m_cmfile);
	}
	
}

//##ModelId=3B0C085C0392
BOOL COptionDialog::ShowTextFile(const CString& strFileName) 
{ 
	
	BOOL bFileDisplayed=FALSE; 
	
	CString strCommandLine(_T("Notepad.exe ") + strFileName); 
	
	PROCESS_INFORMATION processInfo; 
	
	STARTUPINFO startUpInfo; 
	
	startUpInfo.cb =sizeof(STARTUPINFO); 
	
	startUpInfo.lpReserved =NULL; 
	
	startUpInfo.lpDesktop =NULL; 
	
	startUpInfo.lpTitle =NULL; 
	
	startUpInfo.dwX =0; 
	
	startUpInfo.dwY =0; 
	
	startUpInfo.dwXSize =0; 
	
	startUpInfo.dwYSize =0; 
	
	startUpInfo.dwXCountChars =0; 
	
	startUpInfo.dwYCountChars =0; 
	
	startUpInfo.dwFillAttribute =0; 
	
	startUpInfo.dwFlags =STARTF_USESHOWWINDOW; 
	
	startUpInfo.wShowWindow =SW_SHOWDEFAULT; 
	
	startUpInfo.cbReserved2 =0; 
	
	startUpInfo.lpReserved2 =NULL; 
	
	startUpInfo.hStdInput =NULL; 
	
	startUpInfo.hStdOutput =NULL; 
	
	startUpInfo.hStdError =NULL; 
	
	bFileDisplayed=CreateProcess(NULL, 
		
		strCommandLine.GetBuffer(256), 
		
		NULL, 
		
		NULL, 
		
		TRUE, 
		
		0, 
		
		NULL, 
		
		NULL, 
		
		&startUpInfo, 
		
		&processInfo); 
	
	strCommandLine.ReleaseBuffer(); 
	
	ASSERT(bFileDisplayed==TRUE); 
	
	CloseHandle(processInfo.hThread); 
	
	CloseHandle(processInfo.hProcess); 
	
	strCommandLine.ReleaseBuffer();
	return bFileDisplayed; 
}

//##ModelId=3B0C085C039C
void COptionDialog::OnEdit1() 
{
	// TODO: Add your control notification handler code here
	ShowTextFile(m_catfile);
}

//##ModelId=3B0C085C03A6
void COptionDialog::OnEdit2() 
{
	// TODO: Add your control notification handler code here
	ShowTextFile(m_queryfile);	
}

//##ModelId=3B0C085C03B0
void COptionDialog::OnEdit3() 
{
	// TODO: Add your control notification handler code here
	ShowTextFile(m_cmfile);	
}

//##ModelId=3B0C085C035F
void COptionDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	CDialog::PostNcDestroy();
	((CWcolApp*)AfxGetApp())->m_gpToolTip = NULL;
	((CWcolApp*)AfxGetApp())->m_hwndDialog = NULL;
}

//##ModelId=3B0C085B0246
BOOL CWcolApp::ProcessMessageFilter(int code, LPMSG lpMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_hwndDialog != NULL)
	{
		if (lpMsg->hwnd == m_hwndDialog || ::IsChild(m_hwndDialog, lpMsg->hwnd))
		{
			if (m_gpToolTip != NULL)
				m_gpToolTip->RelayEvent(lpMsg);
		}
	}
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}

//Rule Set
//##ModelId=3B0C085C03C3
void COptionDialog::OnBrowse4() 
{
	// TODO: Add your control notification handler code here
	CFileDialog FileDlg(true);
	CString temp = AppDir + "\\RuleSets\\";
	FileDlg.m_ofn.lpstrInitialDir = temp;
	
	if( FileDlg.DoModal() == IDOK) 
	{
		m_rsfile = FileDlg.GetPathName();
		SetDlgItemText(IDC_RULESETEDIT,m_rsfile);
	}
}

//##ModelId=3B0C085C03CD
void COptionDialog::OnEdit4() 
{
	// TODO: Add your control notification handler code here
	ShowTextFile(m_rsfile);	
}

//##ModelId=3B0C085C03D7
void COptionDialog::OnRadioBatch() 
{
	// TODO: Add your control notification handler code here
	CWnd* BatchCtrl1;
	CWnd* BatchCtrl2;
	CWnd* BatchCtrl3;
	CWnd* SingleCtrl1;
	CWnd* SingleCtrl2;
	CWnd* SingleCtrl3;
	
	BatchCtrl1 = GetDlgItem(IDC_QUERYEDIT2);
	BatchCtrl2 = GetDlgItem(IDC_BROWSE5);
	BatchCtrl3 = GetDlgItem(IDC_EDIT5);
	SingleCtrl1 = GetDlgItem(IDC_QUERYEDIT);
	SingleCtrl2 = GetDlgItem(IDC_BROWSE2);
	SingleCtrl3 = GetDlgItem(IDC_EDIT2);
	
	BatchCtrl1->EnableWindow(TRUE);
	BatchCtrl2->EnableWindow(TRUE);
	BatchCtrl3->EnableWindow(TRUE);
	SingleCtrl1->EnableWindow(FALSE);
	SingleCtrl2->EnableWindow(FALSE);
	SingleCtrl3->EnableWindow(FALSE);
}

//##ModelId=3B0C085C03E1
void COptionDialog::OnRadioSingle() 
{
	// TODO: Add your control notification handler code here
	CWnd* BatchCtrl1;
	CWnd* BatchCtrl2;
	CWnd* BatchCtrl3;
	CWnd* SingleCtrl1;
	CWnd* SingleCtrl2;
	CWnd* SingleCtrl3;
	
	BatchCtrl1 = GetDlgItem(IDC_QUERYEDIT2);
	BatchCtrl2 = GetDlgItem(IDC_BROWSE5);
	BatchCtrl3 = GetDlgItem(IDC_EDIT5);
	SingleCtrl1 = GetDlgItem(IDC_QUERYEDIT);
	SingleCtrl2 = GetDlgItem(IDC_BROWSE2);
	SingleCtrl3 = GetDlgItem(IDC_EDIT2);
	
	BatchCtrl1->EnableWindow(FALSE);
	BatchCtrl2->EnableWindow(FALSE);
	BatchCtrl3->EnableWindow(FALSE);
	SingleCtrl1->EnableWindow(TRUE);
	SingleCtrl2->EnableWindow(TRUE);
	SingleCtrl3->EnableWindow(TRUE);
}

//Batch Query
//##ModelId=3B0C085C03E3
void COptionDialog::OnBrowse5() 
{
	// TODO: Add your control notification handler code here
	CFileDialog FileDlg(true);
	CString temp = AppDir + "\\Queries\\";
	FileDlg.m_ofn.lpstrInitialDir = temp;
	
	if( FileDlg.DoModal() == IDOK) 
	{
		m_batchqueryfile = FileDlg.GetPathName();
		SetDlgItemText(IDC_QUERYEDIT2,m_batchqueryfile);
	}
	
}

//##ModelId=3B0C085D0004
void COptionDialog::OnEdit5() 
{
	// TODO: Add your control notification handler code here
	ShowTextFile(m_batchqueryfile);	
}


/*void CWcolApp::OnAppExit() 
{
// TODO: Add your command handler code here
exit(0);
}*/
