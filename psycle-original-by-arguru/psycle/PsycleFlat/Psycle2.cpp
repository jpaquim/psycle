// Psycle2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Psycle2.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SPLASHSCREEN 1

/////////////////////////////////////////////////////////////////////////////
// CPsycle2App

BEGIN_MESSAGE_MAP(CPsycle2App, CWinApp)
	//{{AFX_MSG_MAP(CPsycle2App)
	ON_COMMAND(ID_ABOUTPSYCLE, OnAboutpsycle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsycle2App construction

CPsycle2App::CPsycle2App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPsycle2App object

CPsycle2App theApp;

/////////////////////////////////////////////////////////////////////////////
// CPsycle2App initialization

BOOL CPsycle2App::InitInstance()
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
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Arguru Audio Software"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_MAXIMIZE);
	pFrame->UpdateWindow();
	
	// Sets Icon
	HICON tIcon;
	tIcon=LoadIcon(IDR_MAINFRAME);
	pFrame->SetIcon(tIcon,false);
	
	// Show splash screen
	if (SPLASHSCREEN)OnAboutpsycle();

	pFrame->ConfigAudio();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPsycle2App message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CopyBlock();
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog

/////////////////////////////////////////////////////////////////////////////
// CPsycle2App message handlers

void CPsycle2App::OnAboutpsycle() 
{
// Shows About Box
CAboutDlg dlg;
dlg.DoModal();
}
