// GearDistort.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "ChildView.h"
#include "GearDistort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearDistort dialog


CGearDistort::CGearDistort(CChildView* pParent /*=NULL*/)
	: CDialog(CGearDistort::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearDistort)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGearDistort::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearDistort)
	DDX_Control(pDX, IDC_SLIDER4, m_negc);
	DDX_Control(pDX, IDC_SLIDER3, m_negt);
	DDX_Control(pDX, IDC_SLIDER2, m_posc);
	DDX_Control(pDX, IDC_SLIDER1, m_post);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearDistort, CDialog)
	//{{AFX_MSG_MAP(CGearDistort)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearDistort message handlers

BOOL CGearDistort::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_post.SetRange(0, 128);
	m_posc.SetRange(0, 128);
	m_negt.SetRange(0, 128);
	m_negc.SetRange(0, 128);
	m_post.SetTicFreq(16);
	m_posc.SetTicFreq(16);
	m_negt.SetTicFreq(16);
	m_negc.SetTicFreq(16);

	// Initialize stuff
	m_post.SetPos(128-_pMachine->_posThreshold);
	m_posc.SetPos(128-_pMachine->_posClamp);
	m_negt.SetPos(128-_pMachine->_negThreshold);
	m_negc.SetPos(128-_pMachine->_negClamp);

	char buffer[64];
	sprintf(buffer,_pMachine->_editName);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGearDistort::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_posThreshold = 128-m_post.GetPos();

	*pResult = 0;
}

void CGearDistort::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_posClamp = 128-m_posc.GetPos();
	
	*pResult = 0;
}

void CGearDistort::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{

	_pMachine->_negThreshold = 128-m_negt.GetPos();

	*pResult = 0;
}

void CGearDistort::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{

	_pMachine->_negClamp = 128-m_negc.GetPos();

	*pResult = 0;
}

void CGearDistort::OnButton1() 
{
	m_posc.SetPos(128-_pMachine->_posThreshold);
}

void CGearDistort::OnButton3() 
{
	m_negc.SetPos(128-_pMachine->_negThreshold);
}

void CGearDistort::OnButton2() 
{
	m_negc.SetPos(m_posc.GetPos());
	m_negt.SetPos(m_post.GetPos());
}

BOOL CGearDistort::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearDistort::OnCancel()
{
	m_pParent->DistortionMachineDialog = NULL;
	DestroyWindow();
	delete this;
}
