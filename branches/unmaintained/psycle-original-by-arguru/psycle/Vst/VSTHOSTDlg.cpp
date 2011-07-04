// VSTHOSTDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VSTHOST.h"
#include "VSTHOSTDlg.h"
#include "VstRack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
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
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSTHOSTDlg dialog

CVSTHOSTDlg::CVSTHOSTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVSTHOSTDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVSTHOSTDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVSTHOSTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVSTHOSTDlg)
	DDX_Control(pDX, IDC_LIST2, m_list2);
	DDX_Control(pDX, IDC_LIST1, m_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVSTHOSTDlg, CDialog)
	//{{AFX_MSG_MAP(CVSTHOSTDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	ON_LBN_DBLCLK(IDC_LIST2, OnDblclkList2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSTHOSTDlg message handlers

BOOL CVSTHOSTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	vst.ReadDlls("Vst");
	
	UpdatePlugList();
	UpdateInstList();

	m_list2.SetCurSel(0);
	m_list.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVSTHOSTDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVSTHOSTDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVSTHOSTDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

CVSTHOSTDlg::UpdateInstList()
{
	m_list2.ResetContent();

	char buf[256];

	for(int c=0;c<MAX_VST_INSTANCES;c++)
	{
		if(vst.Instanced[c])
		{
			sprintf(buf,"%.3d: %s",c,vst.GetPlugName(c));
			m_list2.AddString(buf);
		}
		else
		{
			sprintf(buf,"%.3d: ...",c);
			m_list2.AddString(buf);
		}
	}
}

CVSTHOSTDlg::UpdatePlugList()
{
	m_list.ResetContent();

	for(int c=0;c<vst.GetNumDrivers();c++)
	m_list.AddString(vst.GetDllName(c));
}

void CVSTHOSTDlg::OnDblclkList1() 
{
	char b[256];
	int const se=m_list.GetCurSel();
	int se2=m_list2.GetCurSel();
	if(se2<0)se2=0;

	m_list.GetText(se,b);
	
	vst.InstanciateDll(b,se2);
	UpdateInstList();
}

void CVSTHOSTDlg::OnDblclkList2() 
{
	int const se=m_list2.GetCurSel();
	CVstRack dlg;
	dlg.m_pVst=&vst;
	dlg.nPlug=se;
	dlg.DoModal();
}
