// MidiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "MidiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiDlg dialog


CMidiDlg::CMidiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMidiDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMidiDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMidiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMidiDlg, CDialog)
	//{{AFX_MSG_MAP(CMidiDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiDlg message handlers
