// GearGainer.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearGainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearGainer dialog


CGearGainer::CGearGainer(CWnd* pParent /*=NULL*/)
	: CDialog(CGearGainer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearGainer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

doit=false;
}


void CGearGainer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearGainer)
	DDX_Control(pDX, IDC_VOLABEL, m_volabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volsider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearGainer, CDialog)
	//{{AFX_MSG_MAP(CGearGainer)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearGainer message handlers

BOOL CGearGainer::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char buffer[64];
	sprintf(buffer,_pMachine->_editName);
	SetWindowText(buffer);

	m_volsider.SetRange(0, 1024);
	m_volsider.SetPos(1024-_pMachine->_outWet);
	m_volsider.SetTic(0);
	m_volsider.SetTicFreq(64);

	doit = true;

	return TRUE;
}

void CGearGainer::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (doit)
	{
		_pMachine->_outWet = 1024-m_volsider.GetPos();
	}

	float wet = (float)_pMachine->_outWet*0.390625f;
	char buffer[32];
	sprintf(buffer, "%.2f%%", wet);
	m_volabel.SetWindowText(buffer);

	*pResult = 0;
}
