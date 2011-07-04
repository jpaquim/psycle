// PatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "PatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPatDlg dialog


CPatDlg::CPatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPatDlg)
	//}}AFX_DATA_INIT
}


void CPatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatDlg)
	DDX_Control(pDX, IDC_EDIT2, m_numlines);
	DDX_Control(pDX, IDC_EDIT1, m_patname);
	DDX_Control(pDX, IDC_SPIN1, m_spinlines);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatDlg, CDialog)
	//{{AFX_MSG_MAP(CPatDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatDlg message handlers

BOOL CPatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_spinlines.SetRange(1,MAX_LINES);
	m_patname.SetWindowText(songRef->LastPatternName);
	m_patname.SetLimitText(30);
	char buffer[16];
	itoa(songRef->LastPatternLines,buffer,10);
	m_numlines.SetWindowText(buffer);
	UDACCEL acc;
	acc.nSec = 4;
	acc.nInc = 16;
	m_spinlines.SetAccel(1, &acc);

	// Pass the focus to the texbox
	m_patname.SetFocus();
	m_patname.SetSel(0,-1);
	return FALSE;
}

void CPatDlg::OnOK() 
{
	char buffer[64];
	int ps=songRef->playOrder[songRef->editPosition];;

	m_numlines.GetWindowText(buffer,16);
	
	int nlines=atoi(buffer);

	if(nlines<1)nlines=1;
	if(nlines>MAX_LINES)nlines=MAX_LINES;

	m_patname.GetWindowText(buffer,31);
	songRef->AllocNewPattern(ps,buffer,nlines);
	songRef->LastPatternLines=nlines;
	
	sprintf(songRef->LastPatternName,buffer);

	CDialog::OnOK();
}
