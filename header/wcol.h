// wcol.h : main header file for the WCOL application
//

#if !defined(AFX_WCOL1_H__1801B687_FC90_11D0_8172_ED1BA299B528__INCLUDED_)
#define AFX_WCOL1_H__1801B687_FC90_11D0_8172_ED1BA299B528__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CWcolApp:
// See wcol.cpp for the implementation of this class
//

//##ModelId=3B0C085B01D0
class CWcolApp : public CWinApp
{
public:
	//##ModelId=3B0C085B01E1
	CWcolApp();
	//##ModelId=3B0C085B01E2
	HWND m_hwndDialog;
	//##ModelId=3B0C085B0232
	CToolTipCtrl * m_gpToolTip;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWcolApp)
	public:
	//##ModelId=3B0C085B023B
	virtual BOOL InitInstance();
	//##ModelId=3B0C085B0246
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWcolApp)
	//##ModelId=3B0C085B0251
	afx_msg void OnAppAbout();
	//afx_msg void OnAppExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////
//##ModelId=3B0C085B0281
class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	//##ModelId=3B0C085B028C
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	//##ModelId=3B0C085B0295
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	//##ModelId=3B0C085B029F
	virtual ~CMainFrame();
#ifdef _DEBUG
	//##ModelId=3B0C085B02A9
	virtual void AssertValid() const;
	//##ModelId=3B0C085B02B3
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////
//##ModelId=3B0C085B02EF
class CWcolDoc : public CDocument
{
protected: // create from serialization only
	//##ModelId=3B0C085B02FA
	CWcolDoc();
	DECLARE_DYNCREATE(CWcolDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWcolDoc)
	public:
	//##ModelId=3B0C085B0303
	virtual BOOL OnNewDocument();
	//##ModelId=3B0C085B0305
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	//##ModelId=3B0C085B030F
	virtual ~CWcolDoc();
#ifdef _DEBUG
	//##ModelId=3B0C085B0318
	virtual void AssertValid() const;
	//##ModelId=3B0C085B0322
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWcolDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////
//##ModelId=3B0C085B03A3
class CWcolView : public CEditView
{
protected: // create from serialization only
	//##ModelId=3B0C085B03AE
	CWcolView();
	DECLARE_DYNCREATE(CWcolView)

// Attributes
public:
	//##ModelId=3B0C085B03B7
	CWcolDoc* GetDocument();

// Operations
public:
	//##ModelId=3B0C085B03B8
	void Print(LPCTSTR lpszMessage);
	//##ModelId=3B0C085B03C2
	void ClearWindow();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWcolView)
	public:
	//##ModelId=3B0C085B03CB
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	//##ModelId=3B0C085B03D5
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	//##ModelId=3B0C085B03DF
	virtual ~CWcolView();
#ifdef _DEBUG
	//##ModelId=3B0C085B03E1
	virtual void AssertValid() const;
	//##ModelId=3B0C085C0002
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWcolView)
	//##ModelId=3B0C085C000D
	afx_msg void OnBegin();
	//##ModelId=3B0C085C0016
	afx_msg void OnMemory();
	//##ModelId=3B0C085C0020
	afx_msg void OnOption();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in wcol1View.cpp
inline CWcolDoc* CWcolView::GetDocument()
   { return (CWcolDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionDialog dialog

//##ModelId=3B0C085C01F6
class COptionDialog : public CDialog
{
// Construction
public:
	//##ModelId=3B0C085C0200
	COptionDialog(CWnd* pParent = NULL);   // standard constructor
	//##ModelId=3B0C085C0202
	~COptionDialog();

// Dialog Data
	//{{AFX_DATA(COptionDialog)
	enum { IDD = IDD_OPTIONDLG };
	//##ModelId=3B0C085C0215
	CButton	m_oneline;
	//##ModelId=3B0C085C0229
	CButton	m_radiobut;
	//##ModelId=3B0C085C0232
	CString	m_catfile;
	//##ModelId=3B0C085C023C
	CString	m_queryfile;
	//##ModelId=3B0C085C0250
	CString m_batchqueryfile;
	//##ModelId=3B0C085C025A
	int		m_radio;
	//##ModelId=3B0C085C0264
	BOOL	m_singlelinebatch;
	//##ModelId=3B0C085C0278
	BOOL	m_covetrace;
	//##ModelId=3B0C085C0282
	BOOL	m_cucardpruning;
	//##ModelId=3B0C085C028C
	BOOL	m_filetrace;
	//##ModelId=3B0C085C02A0
	BOOL	m_pruning;
	//##ModelId=3B0C085C02AA
	BOOL	m_windowtrace;
	//##ModelId=3B0C085C02B4
	CString	m_cmfile;
	//##ModelId=3B0C085C02C8
	BOOL	m_epspruning;
	//##ModelId=3B0C085C02D2
	double	m_eps;
	//##ModelId=3B0C085C02DC
	BOOL	m_halt;
	//##ModelId=3B0C085C02F0
	int		m_halt_grpsize;
	//##ModelId=3B0C085C02FA
	int		m_halt_winsize;
	//##ModelId=3B0C085C030E
	int		m_halt_impr;
	//##ModelId=3B0C085C0318
	BOOL	m_finalssp;
	//##ModelId=3B0C085C0322
	BOOL	m_open;
	//##ModelId=3B0C085C0336
	BOOL	m_ssp;
	//##ModelId=3B0C085C0340
	CString m_rsfile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionDialog)
	protected:
	//##ModelId=3B0C085C034A
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//##ModelId=3B0C085C035F
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptionDialog)
	//##ModelId=3B0C085C0369
	afx_msg void OnBrowse1();
	//##ModelId=3B0C085C0373
	afx_msg void OnBrowse2();
	//##ModelId=3B0C085C0375
	virtual void OnOK();
	//##ModelId=3B0C085C037E
	virtual void OnCancel();
	//##ModelId=3B0C085C0388
	afx_msg void OnBrowse3();
	//##ModelId=3B0C085C0392
	afx_msg BOOL ShowTextFile(const CString& strFileName);
	//##ModelId=3B0C085C039C
	afx_msg void OnEdit1();
	//##ModelId=3B0C085C03A6
	afx_msg void OnEdit2();
	//##ModelId=3B0C085C03B0
	afx_msg void OnEdit3();
	//##ModelId=3B0C085C03BA
	virtual BOOL OnInitDialog();
	//##ModelId=3B0C085C03C3
	afx_msg void OnBrowse4();
	//##ModelId=3B0C085C03CD
	afx_msg void OnEdit4();
	//##ModelId=3B0C085C03D7
	afx_msg void OnRadioBatch();
	//##ModelId=3B0C085C03E1
	afx_msg void OnRadioSingle();
	//##ModelId=3B0C085C03E3
	afx_msg void OnBrowse5();
	//##ModelId=3B0C085D0004
	afx_msg void OnEdit5();
	//##ModelId=3B0C085D000E
	afx_msg void OnRadio1();
	//##ModelId=3B0C085D0018
	afx_msg void OnPruning();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//##ModelId=3B0C085D002B
	CToolTipCtrl * m_pToolTip;
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WCOL1_H__1801B687_FC90_11D0_8172_ED1BA299B528__INCLUDED_)
