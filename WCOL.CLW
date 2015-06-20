; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=COptionDialog
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "wcol.h"
LastPage=0

ClassCount=6
Class1=CWcolApp
Class2=CAboutDlg
Class3=CMainFrame
Class4=CWcolDoc
Class5=CWcolView
Class6=COptionDialog

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDD_OPTIONDLG
Resource3=IDR_MAINFRAME

[CLS:CWcolApp]
Type=0
BaseClass=CWinApp
HeaderFile=wcol.h
ImplementationFile=wcol.cpp
Filter=N
VirtualFilter=AC
LastObject=CWcolApp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=wcol.cpp
ImplementationFile=wcol.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=wcol.h
ImplementationFile=wcol.cpp
LastObject=ID_APP_EXIT
Filter=T
VirtualFilter=fWC

[CLS:CWcolDoc]
Type=0
BaseClass=CDocument
HeaderFile=wcol.h
ImplementationFile=wcol.cpp

[CLS:CWcolView]
Type=0
BaseClass=CEditView
HeaderFile=wcol.h
ImplementationFile=wcol.cpp

[CLS:COptionDialog]
Type=0
BaseClass=CDialog
HeaderFile=wcol.h
ImplementationFile=wcol.cpp
Filter=D
VirtualFilter=dWC
LastObject=COptionDialog

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_OPTIONDLG]
Type=1
Class=COptionDialog
ControlCount=47
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_CATFILE_EDIT,edit,1350631552
Control5=IDC_BROWSE1,button,1342242816
Control6=IDC_QUERYEDIT,edit,1350631552
Control7=IDC_BROWSE2,button,1342242816
Control8=IDC_STATIC,button,1342177287
Control9=IDC_FILE,button,1342259203
Control10=IDC_WINDOW,button,1342242819
Control11=IDC_PRUNING,button,1342242819
Control12=IDC_CUCARD,button,1342242819
Control13=IDC_COVE,button,1342242819
Control14=IDC_GBLEPS,button,1342242819
Control15=IDC_STATIC,static,1342308352
Control16=IDC_CMEDIT,edit,1350631552
Control17=IDC_BROWSE3,button,1342242816
Control18=IDC_EPS_EDIT,edit,1350631552
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,button,1342177287
Control21=IDC_STATIC,button,1342177287
Control22=IDC_FINALSSP,button,1342242819
Control23=IDC_OPEN,button,1342242819
Control24=IDC_SSP,button,1342242819
Control25=IDC_EDIT1,button,1342242816
Control26=IDC_EDIT2,button,1342242816
Control27=IDC_EDIT3,button,1342242816
Control28=IDC_STATIC,static,1342308352
Control29=IDC_RULESETEDIT,edit,1350631552
Control30=IDC_BROWSE4,button,1342242816
Control31=IDC_EDIT4,button,1342242816
Control32=IDC_STATIC,button,1342177287
Control33=IDC_RADIO_BATCH,button,1342308361
Control34=IDC_RADIO_SINGLE,button,1342177289
Control35=IDC_QUERYEDIT2,edit,1350631552
Control36=IDC_BROWSE5,button,1342242816
Control37=IDC_EDIT5,button,1342242816
Control38=IDC_STATIC,button,1342177287
Control39=IDC_HALT,button,1342242819
Control40=IDC_HALT_GRPSIZE,edit,1350631552
Control41=IDC_STATIC,static,1342308352
Control42=IDC_STATIC,static,1342308352
Control43=IDC_HALT_WINSIZE,edit,1350631552
Control44=IDC_STATIC,static,1342308352
Control45=IDC_HALT_IMPR,edit,1350631552
Control46=IDC_STATIC,static,1342308352
Control47=IDC_CHECK5,button,1342242819

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=IDD_BEGIN
Command2=IDD_MEMORY
Command3=ID_OPTION
Command4=ID_EDIT_FIND
Command5=ID_EDIT_REPEAT
Command6=ID_APP_ABOUT
Command7=ID_APP_EXIT
CommandCount=7

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_EDIT_FIND
Command2=ID_EDIT_REPEAT
CommandCount=2

