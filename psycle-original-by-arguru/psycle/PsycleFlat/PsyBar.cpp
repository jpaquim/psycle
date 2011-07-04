// PsyBar.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "PsyBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsyBar dialog


CPsyBar::CPsyBar(CWnd* pParent /*=NULL*/)
	: CDialog(CPsyBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPsyBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPsyBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPsyBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPsyBar, CDialog)
	//{{AFX_MSG_MAP(CPsyBar)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsyBar message handlers
