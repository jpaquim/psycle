// MacProp.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "MacProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMacProp dialog


CMacProp::CMacProp(CWnd* pParent /*=NULL*/)
	: CDialog(CMacProp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMacProp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMacProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMacProp)
	DDX_Control(pDX, IDC_EDIT1, m_macname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMacProp, CDialog)
	//{{AFX_MSG_MAP(CMacProp)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacProp message handlers

BOOL CMacProp::OnInitDialog() 
{
	CDialog::OnInitDialog();

	deleted=false;

	m_macname.SetLimitText(15);
	char buffer[32];
	sprintf(buffer,"%s Properties",machineRef->editName);
	SetWindowText(buffer);
	
	sprintf(buffer,"%s",machineRef->editName);
	m_macname.SetWindowText(buffer);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMacProp::OnChangeEdit1() 
{
	m_macname.GetWindowText(txt,16);
}

void CMacProp::OnButton1() 
{
	// Delete MACHINE!
	if(MessageBox("Are you sure to delete this machine?","Warning", MB_YESNO)==IDYES)
	{
	deleted=true;
	OnCancel();
	}
}
