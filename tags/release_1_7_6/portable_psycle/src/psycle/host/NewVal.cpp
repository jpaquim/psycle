#include "stdafx.h"

// NewVal.cpp : implementation file
//

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
	DDX_Control(pDX, IDC_TEXT, m_text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewVal, CDialog)
	//{{AFX_MSG_MAP(CNewVal)
	ON_EN_UPDATE(IDC_EDIT1, OnUpdateEdit1)
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

void CNewVal::OnUpdateEdit1() 
{
	// TODO: Add your control notification handler code here
	char buffer[256];
	m_value.GetWindowText(buffer,16);
	m_Value=atoi(buffer);

	if (m_Value < min)
	{
		m_Value = min;
		sprintf(buffer,"Below Range. Use this HEX value: twk %.2X %.2X %.4X",paramindex,macindex,m_Value-min);
	}
	else if(m_Value > max)
	{
		m_Value = max;
		sprintf(buffer,"Above Range. Use this HEX value: twk %.2X %.2X %.4X",paramindex,macindex,m_Value-min);
	}
	else
	{
		sprintf(buffer,"Use this HEX value: twk %.2X %.2X %.4X",paramindex,macindex,m_Value-min);
	}
	m_text.SetWindowText(buffer);
}

