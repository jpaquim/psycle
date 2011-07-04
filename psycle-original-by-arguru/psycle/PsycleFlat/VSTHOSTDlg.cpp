// VSTHOSTDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "VSTHOSTDlg.h"
#include "VstRack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	ON_BN_CLICKED(IDC_BUTTON2, OnInstancePlug)
	ON_BN_CLICKED(IDC_BUTTON1, OnEditPlug)
	ON_BN_CLICKED(IDCANCEL2, OnAboutHost)
	ON_BN_CLICKED(IDC_BUTTON3, OnExitButton)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSTHOSTDlg message handlers

BOOL CVSTHOSTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	editor=false;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	UpdatePlugList();
	UpdateInstList();

	m_list2.SetCurSel(0);
	m_list.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVSTHOSTDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
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
		if(m_vst->Instanced[c])
		{
			sprintf(buf,"%.3d: %s",c,m_vst->GetPlugName(c));
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

	for(int c=0;c<m_vst->GetNumDrivers();c++)
	m_list.AddString(m_vst->GetDllName(c));
}

void CVSTHOSTDlg::OnDblclkList1() 
{
	char b[256];
	int const se=m_list.GetCurSel();
	int se2=m_list2.GetCurSel();
	if(se2<0)se2=0;

	// Check if any machine is playing with this plug already
	for(int m=0;m<MAX_MACHINES;m++)
	{
		if(songRef->Activemachine[m])
		{
			if(songRef->machine[m]->ovst.ANY)
			{
				if(songRef->machine[m]->ovst.instance==se2)
				{
					MessageBox("There is a machine that is using already this plugin, eject it before replace","Error",MB_ICONWARNING);
					return;
				}
			}
		}
	}

	m_list.GetText(se,b);
	if(m_vst->InstanciateDll(b,se2))
	UpdateInstList();
}

void CVSTHOSTDlg::OnDblclkList2() 
{
	int const se=m_list2.GetCurSel();
	
	if(se>-1 && se<MAX_VST_INSTANCES && m_vst->Instanced[se])
	{
		if(m_vst->effect[se]->flags & effFlagsHasEditor)
		{
			if(!editor)
			{
			editor=true;
			vstgui=new CVstGui(0);
			sprintf(vstgui->cWndTitle,m_vst->GetPlugName(se));
			vstgui->effect=m_vst->effect[se];			
			vstgui->LoadFrame(IDR_VSTFRAME,
				WS_OVERLAPPED
				| WS_CAPTION
				| WS_SYSMENU
				| FWS_ADDTOTITLE);

			vstgui->DoTheInit();
			vstgui->ShowWindow(SW_SHOWNORMAL);
			vstgui->editorOpen=&editor;
			}
			else
			MessageBox("Close previous dialog","VST Error",MB_OK | MB_ICONERROR);
		}
		else
		{
		CVstRack dlg;
		dlg.m_pVst=m_vst;
		dlg.nPlug=se;
		dlg.DoModal();
		}
	}
	else
	MessageBox("Select a valid VST plugin first","VST Instance error",MB_OK | MB_ICONERROR);

}

void CVSTHOSTDlg::OnInstancePlug() 
{
	OnDblclkList1();	
}

void CVSTHOSTDlg::OnEditPlug() 
{
	OnDblclkList2();
}

void CVSTHOSTDlg::OnAboutHost() 
{
	MessageBox("Codigo alojamiento VST-HOST escrito por J. Arguelles Rius, Mathieu Routhier y Stijn Kuipers\n\nVST v2.0 API by Steinberg");
}

void CVSTHOSTDlg::OnExitButton() 
{
	OnClose();	
}

void CVSTHOSTDlg::OnClose() 
{		
	if(editor)
	MessageBox("Close VST editor first");
	else
	CDialog::OnClose();
}

int CVSTHOSTDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	return 0;
}
