#include "stdafx.h"
#include "Psycle2.h"
#include "WaveEdAmplifyDialog.h"
#include "Helpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveEdAmplifyDialog dialog


CWaveEdAmplifyDialog::CWaveEdAmplifyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CWaveEdAmplifyDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaveEdAmplifyDialog)
	//}}AFX_DATA_INIT
}


void CWaveEdAmplifyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaveEdAmplifyDialog)
	DDX_Control(pDX, IDC_EDIT2, m_dbedit);
	DDX_Control(pDX, IDC_SLIDER3, m_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaveEdAmplifyDialog, CDialog)
	//{{AFX_MSG_MAP(CWaveEdAmplifyDialog)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveEdAmplifyDialog message handlers

BOOL CWaveEdAmplifyDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_slider.SetRange(0, 14400); 	// Don't use (-,+) range. It fucks up with the "0"
	m_slider.SetPos(9600);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWaveEdAmplifyDialog::OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char tmp[6];
	float db;
	db =  (float) (m_slider.GetPos()-9600)*0.01f;
	_gcvt(db ,4 ,tmp);
	if (tmp[strlen(tmp) -1] == '.') tmp[strlen(tmp) -1] = 0; //Remove dot if last.
	
	m_dbedit.SetWindowText(tmp);
	*pResult = 0;
}

void CWaveEdAmplifyDialog::OnOK() 
{
	
	char db_t[6];
	int db_i = 0;
	m_dbedit.GetLine(0, db_t);
	db_i = (int)(100*atof(db_t));
	if (db_i) EndDialog( db_i );
	else EndDialog( AMP_DIALOG_CANCEL);
	
	//	CDialog::OnOK();
}

void CWaveEdAmplifyDialog::OnCancel() 
{
	EndDialog( AMP_DIALOG_CANCEL );
	
	//CDialog::OnCancel();
}
