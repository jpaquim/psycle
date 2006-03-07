// GearGainer.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearGainer.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearGainer dialog


CGearGainer::CGearGainer(CChildView* pParent /*=NULL*/)
	: CDialog(CGearGainer::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearGainer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGearGainer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearGainer)
	DDX_Control(pDX, IDC_VOLABEL, m_volabel);
	DDX_Control(pDX, IDC_VOLSLIDER, m_volslider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearGainer, CDialog)
	//{{AFX_MSG_MAP(CGearGainer)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VOLSLIDER, OnCustomdrawVolSlider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearGainer message handlers

BOOL CGearGainer::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText(_pMachine->_editName);

	m_volslider.SetRange(0, 1024);
	m_volslider.SetPos(1024-_pMachine->_outWet);
	m_volslider.SetTic(0);
	m_volslider.SetTicFreq(64);

	return TRUE;
}

void CGearGainer::OnCustomdrawVolSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_outWet = 1024-m_volslider.GetPos();

	float wet = CValueMapper::Map_255_1(_pMachine->_outWet);
	char buffer[32];
	if (wet > 1.0f)
	{	
		sprintf(buffer,"+%.1f dB\n%.2f%%",20.0f * log10(wet),wet*100); 
	}
	else if (wet == 1.0f)
	{	
		sprintf(buffer,"0.0 dB\n100.00%%"); 
	}
	else if (wet > 0.0f)
	{	
		sprintf(buffer,"%.1f dB\n%.2f%%",20.0f * log10(wet),wet*100); 
	}
	else 
	{				
		sprintf(buffer,"-Inf. dB\n0.00%%"); 
	}

	m_volabel.SetWindowText(buffer);

	*pResult = 0;
}


BOOL CGearGainer::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearGainer::OnCancel()
{
	m_pParent->GainerMachineDialog = NULL;
	DestroyWindow();
	delete this;
}
