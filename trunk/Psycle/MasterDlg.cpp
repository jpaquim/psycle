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
	//}}AFX_DATA_INIT
}


void CMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMasterDlg)
	DDX_Control(pDX, IDC_MASTERPEAK, m_masterpeak);
	DDX_Control(pDX, IDC_SLIDERMASTER, m_slidermaster);
	DDX_Control(pDX, IDC_SLIDERM9, m_sliderm9);
	DDX_Control(pDX, IDC_SLIDERM8, m_sliderm8);
	DDX_Control(pDX, IDC_SLIDERM7, m_sliderm7);
	DDX_Control(pDX, IDC_SLIDERM6, m_sliderm6);
	DDX_Control(pDX, IDC_SLIDERM5, m_sliderm5);
	DDX_Control(pDX, IDC_SLIDERM4, m_sliderm4);
	DDX_Control(pDX, IDC_SLIDERM3, m_sliderm3);
	DDX_Control(pDX, IDC_SLIDERM2, m_sliderm2);
	DDX_Control(pDX, IDC_SLIDERM12, m_sliderm12);
	DDX_Control(pDX, IDC_SLIDERM11, m_sliderm11);
	DDX_Control(pDX, IDC_SLIDERM10, m_sliderm10);
	DDX_Control(pDX, IDC_SLIDERM1, m_sliderm1);
	DDX_Control(pDX, IDC_MIXERVIEW, m_mixerview);
	DDX_Control(pDX, IDC_AUTODEC, m_autodec);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterDlg)
	ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERMASTER, OnCustomdrawSlidermaster)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers

BOOL CMasterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_slidermaster.SetRange(0, 256, true);
	m_slidermaster.SetPos(256-_pMachine->_outDry);
	if (((Master*)_pMachine)->decreaseOnClip) m_autodec.SetCheck(1);
	else m_autodec.SetCheck(0);

	return TRUE;
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


void CMasterDlg::OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_outDry = 256-m_slidermaster.GetPos();
	
/*	float const mv = _pMachine->_outDry*0.00390625f;
	char buffer[16];
	
	if (mv > 0.0f)
	{
		sprintf(buffer,"%.1f dB",20.0f * log10(mv));
	}
	else
	{
		sprintf(buffer,"-Inf. dB");
	}
	m_dblevel.SetWindowText(buffer);*/
	*pResult = 0;
}
