// OutputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "OutputDlg.h"
#include "WaveOut.h"
#include "DirectSound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputDlg dialog


COutputDlg::COutputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COutputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COutputDlg)
	//}}AFX_DATA_INIT

	m_pDrivers[0] = new WaveOut;
	m_pDrivers[1] = new DirectSound;
}


void COutputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutputDlg)
	DDX_Control(pDX, IDC_DRIVER, m_driverComboBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COutputDlg, CDialog)
	//{{AFX_MSG_MAP(COutputDlg)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputDlg message handlers

BOOL COutputDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	const char* psDesc = m_pDrivers[0]->GetInfo()->_psName;
	m_driverComboBox.AddString(psDesc);
	psDesc = m_pDrivers[1]->GetInfo()->_psName;
	m_driverComboBox.AddString(psDesc);

	if (m_driverIndex >= m_driverComboBox.GetCount())
	{
		m_driverIndex = 0;
	}
	m_driverComboBox.SetCurSel(m_driverIndex);
	m_oldDriverIndex = m_driverIndex;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COutputDlg::OnOK() 
{
	m_driverIndex = m_driverComboBox.GetCurSel();
	for (int i=m_driverComboBox.GetCount()-1; i>=0; i--)
	{
		if (i != m_driverIndex)
		{
			delete m_pDrivers[i];
		}
	}
	CDialog::OnOK();
}

void COutputDlg::OnCancel() 
{
	for (int i=m_driverComboBox.GetCount()-1; i>=0; i--)
	{
		if (i != m_oldDriverIndex)
		{
			delete m_pDrivers[i];
		}
	}
	m_driverIndex = m_oldDriverIndex;
	CDialog::OnCancel();
}

void COutputDlg::OnConfig() 
{
	int index = m_driverComboBox.GetCurSel();
	m_pDrivers[index]->Configure();
}
