// ConfigDlg.cpp : implementation file
//

#include "..\global.h"
#include "ConfigDlg.h"
#include "..\configuration.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog


CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDlg)
	DDX_Control(pDX, IDC_EDIT_NATIVE, m_editnative);
	DDX_Control(pDX, IDC_EDIT_VST, m_editvst);
	DDX_Control(pDX, IDC_SAMP_RATE, m_samprate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CConfigDlg)
	ON_BN_CLICKED(IDC_CHANGE_NATIVE, OnChangeNative)
	ON_BN_CLICKED(IDC_CHANGE_VST, OnChangeVst)
	ON_EN_CHANGE(IDC_EDIT_NATIVE, OnChangeEditNative)
	ON_EN_CHANGE(IDC_EDIT_VST, OnChangeEditVst)
	ON_CBN_SELCHANGE(IDC_SAMP_RATE, OnSelchangeSampRate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg message handlers

BOOL CConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigDlg::OnChangeNative() 
{
	// TODO: Add your control notification handler code here
	
}

void CConfigDlg::OnChangeVst() 
{
	// TODO: Add your control notification handler code here
	
}

void CConfigDlg::OnChangeEditNative() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CConfigDlg::OnChangeEditVst() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CConfigDlg::OnSelchangeSampRate() 
{
	// TODO: Add your control notification handler code here
	
}

void CConfigDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
