// MasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "MasterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog


CMasterDlg::CMasterDlg(CChildView* pParent /*=NULL*/)
	: CDialog(CMasterDlg::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CMasterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMasterDlg)
	DDX_Control(pDX, IDC_AUTODEC, m_autodec);
	DDX_Control(pDX, IDC_DBLEVEL, m_dblevel);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterDlg)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers

BOOL CMasterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_slider.SetRange(0, 256, true);
	m_slider.SetPos(_pMachine->_outDry);
	if (((Master*)_pMachine)->decreaseOnClip) m_autodec.SetCheck(1);
	else m_autodec.SetCheck(0);
	
	return TRUE;
}

void CMasterDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_outDry = m_slider.GetPos();

	float const mv = _pMachine->_outDry*0.00390625f;
	char buffer[16];

	if (mv > 0.0f)
	{
		sprintf(buffer,"%.1f dB",20.0f * log10(mv));
	}
	else
	{
		sprintf(buffer,"-Inf. dB");
	}
	m_dblevel.SetWindowText(buffer);
	*pResult = 0;
}

void CMasterDlg::OnAutodec() 
{
	if (m_autodec.GetState() &0x0003)
	{
		((Master*)_pMachine)->decreaseOnClip=true;
	}
	else ((Master*)_pMachine)->decreaseOnClip=false;
}

BOOL CMasterDlg::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CMasterDlg::OnCancel()
{
	m_pParent->MasterMachineDialog = NULL;
	DestroyWindow();
}

