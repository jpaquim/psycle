// SwingFillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "SwingFillDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSwingFillDlg dialog


CSwingFillDlg::CSwingFillDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSwingFillDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSwingFillDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSwingFillDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSwingFillDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_CENTER_TEMPO, m_Tempo);
	DDX_Control(pDX, IDC_WIDTH, m_Width);
	DDX_Control(pDX, IDC_VARIANCE, m_Variance);
	DDX_Control(pDX, IDC_PHASE, m_Phase);
	DDX_Control(pDX, IDC_OFFSET, m_Offset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSwingFillDlg, CDialog)
	//{{AFX_MSG_MAP(CSwingFillDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwingFillDlg message handlers

BOOL CSwingFillDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	bGo = FALSE;
	
	char buf[32];
	sprintf(buf,"%d",tempo);
	m_Tempo.SetWindowText(buf);
	m_Tempo.SetSel(-1,-1,false);

	sprintf(buf,"%d",width);
	m_Width.SetWindowText(buf);
	m_Width.SetSel(-1,-1,false);

	sprintf(buf,"%.2f",variance);
	m_Variance.SetWindowText(buf);
	m_Variance.SetSel(-1,-1,false);
	
	sprintf(buf,"%.2f",phase);
	m_Phase.SetWindowText(buf);
	m_Phase.SetSel(-1,-1,false);

	m_Offset.SetCheck(offset?1:0);

	return FALSE;
}

void CSwingFillDlg::OnOK() 
{
	bGo = TRUE;
	char buf[32];
	m_Tempo.GetWindowText(buf,32);
	tempo=atoi(buf);
	if (tempo < 33)
		tempo = 33;
	else if (tempo > 999)
		tempo = 999;

	m_Width.GetWindowText(buf,32);
	width=atoi(buf);
	if (width < 1)
		width = 1;

	m_Variance.GetWindowText(buf,32);
	variance=float(atof(buf));

	m_Phase.GetWindowText(buf,32);
	phase=float(atof(buf));

	offset = m_Offset.GetCheck()?true:false;

	CDialog::OnOK();
}
