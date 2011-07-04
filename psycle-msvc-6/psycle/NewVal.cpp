// NewVal.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "NewVal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewVal dialog


CNewVal::CNewVal(CWnd* pParent /*=NULL*/)
	: CDialog(CNewVal::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewVal)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Value=0;
}


void CNewVal::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewVal)
	DDX_Control(pDX, IDC_EDIT1, m_value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewVal, CDialog)
	//{{AFX_MSG_MAP(CNewVal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewVal message handlers

BOOL CNewVal::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(Title);

	char buf[32];
	sprintf(buf,"%d",m_Value);
	m_value.SetWindowText(buf);
	m_value.SetSel(-1,-1,false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewVal::OnOK() 
{
	// TODO: Add extra validation here
	char buffer[16];
	m_value.GetWindowText(buffer,16);
	m_Value=atoi(buffer);
	
	CDialog::OnOK();
}
