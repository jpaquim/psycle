// SkinDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "SkinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg dialog


CSkinDlg::CSkinDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSkinDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSkinDlg)
	//}}AFX_DATA_INIT
}


void CSkinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkinDlg)
	DDX_Control(pDX, IDC_CHECK2, m_pv_check);
	DDX_Control(pDX, IDC_CHECK1, m_mv_check);
	DDX_Control(pDX, IDC_EDIT3, m_pv_skin_edit);
	DDX_Control(pDX, IDC_EDIT1, m_mv_skin_edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSkinDlg, CDialog)
	//{{AFX_MSG_MAP(CSkinDlg)
	ON_BN_CLICKED(IDC_BROWSE_MV, OnBrowseMv)
	ON_BN_CLICKED(IDC_BROWSE_PV, OnBrowsePv)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
	ON_BN_CLICKED(IDC_BUTTON1, OnColourMachine)
	ON_BN_CLICKED(IDC_BUTTON3, OnButtonPattern)
	ON_BN_CLICKED(IDC_BUTTON2, OnGenColor)
	ON_BN_CLICKED(IDC_BUTTON4, OnEffColor)
	ON_BN_CLICKED(IDC_BUTTON5, OnMasterColor)
	ON_BN_CLICKED(IDC_BUTTON23, OnVuBarColor)
	ON_BN_CLICKED(IDC_BUTTON24, OnVuBackColor)
	ON_BN_CLICKED(IDC_BUTTON25, OnVuClipBar)
	ON_BN_CLICKED(IDC_BUTTON26, OnColorMisc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg message handlers

void CSkinDlg::OnOK()
{
	m_mv_skin_edit.GetWindowText(mv_pathbuf,_MAX_PATH);
	m_pv_skin_edit.GetWindowText(pv_pathbuf,_MAX_PATH);

	CDialog::OnOK();
}

void CSkinDlg::OnBrowseMv() 
{
	char buffer[_MAX_PATH];
	m_mv_skin_edit.GetWindowText(buffer,_MAX_PATH);
	
	if(BrowseTo(buffer))
	{
	m_mv_skin_edit.SetWindowText(buffer);
	m_mv_check.SetCheck(true);
	UpdateChecks();
	}
}

void CSkinDlg::OnBrowsePv() 
{
	char buffer[_MAX_PATH];
	m_pv_skin_edit.GetWindowText(buffer,_MAX_PATH);

	if(BrowseTo(buffer))
	{
	m_pv_skin_edit.SetWindowText(buffer);
	m_pv_check.SetCheck(true);
	UpdateChecks();
	}
}

bool CSkinDlg::BrowseTo(char *rpath) 
{

bool val=false;

OPENFILENAME ofn;       // common dialog box structure
char szFile[_MAX_PATH];       // buffer for file name

sprintf(szFile,"*.bmp");

// Initialize OPENFILENAME
ZeroMemory(&ofn, sizeof(OPENFILENAME));
ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = GetParent()->m_hWnd;
ofn.lpstrFile = szFile;
ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFilter = "Windows BMP files\0*.bmp\0All files\0*.*";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = NULL;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)==TRUE)
	{
	val=true;
	sprintf(rpath,szFile);
	}

return val;
}

void CSkinDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CSkinDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	InitChecks();

	m_mv_skin_edit.SetWindowText(mv_pathbuf);
	m_pv_skin_edit.SetWindowText(pv_pathbuf);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSkinDlg::OnCheck1() {UpdateChecks();}
void CSkinDlg::OnCheck2() {UpdateChecks();}

CSkinDlg::UpdateChecks()
{
mv_check=m_mv_check.GetCheck();
pv_check=m_pv_check.GetCheck();
m_mv_skin_edit.EnableWindow(mv_check);
m_pv_skin_edit.EnableWindow(pv_check);
}

CSkinDlg::InitChecks()
{
m_mv_check.SetCheck(mv_check);
m_pv_check.SetCheck(pv_check);
UpdateChecks();
}

void CSkinDlg::OnColourMachine() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
mv_c=dlg.GetColor();
}

void CSkinDlg::OnButtonPattern() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
pv_c=dlg.GetColor();
}

void CSkinDlg::OnGenColor() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
gen_c=dlg.GetColor();
}

void CSkinDlg::OnEffColor() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
eff_c=dlg.GetColor();
}

void CSkinDlg::OnMasterColor() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
mas_c=dlg.GetColor();	
}

void CSkinDlg::OnVuBarColor() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
vub_c=dlg.GetColor();		
}

void CSkinDlg::OnVuBackColor() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
vug_c=dlg.GetColor();		
}

void CSkinDlg::OnVuClipBar() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
vuc_c=dlg.GetColor();			
}


void CSkinDlg::OnColorMisc() 
{
CColorDialog dlg;
if(dlg.DoModal()==IDOK)
plg_c=dlg.GetColor();	
}
