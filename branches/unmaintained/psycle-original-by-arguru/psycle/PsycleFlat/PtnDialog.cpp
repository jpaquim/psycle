// PtnDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "PtnDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPtnDialog dialog


CPtnDialog::CPtnDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPtnDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPtnDialog)
	//}}AFX_DATA_INIT
}


void CPtnDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPtnDialog)
	DDX_Control(pDX, IDOK, m_okbut);
	DDX_Control(pDX, IDC_PNLABEL, m_pnlabel);
	DDX_Control(pDX, IDC_LIST1, m_patlist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPtnDialog, CDialog)
	//{{AFX_MSG_MAP(CPtnDialog)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickList1)
	ON_BN_CLICKED(IDC_CREATEB, OnCreateb)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPtnDialog message handlers

BOOL CPtnDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ouPat=-1;

	m_patlist.InsertColumn(1,"Index/Name",LVCFMT_LEFT,162,0);
	m_patlist.InsertColumn(2,"Created",LVCFMT_LEFT,64,0);
	m_patlist.InsertColumn(3,"Lines",LVCFMT_LEFT,40,0);

	char buffer[64];
	
	int n=0;

	for(int c=0;c<MAX_PATTERNS;c++)
	{	
		sprintf(buffer,"%.3d: %s",c, songRef->patternName[c]);
		m_patlist.InsertItem(n,buffer);
		
		m_patlist.SetItem(n,1,LVIF_TEXT,"Yes",0,0,0,NULL);
		
		// Lines
		sprintf(buffer,"%d",songRef->patternLines[c]);
		m_patlist.SetItem(n,2,LVIF_TEXT,buffer,0,0,0,NULL);
		
		PIndex[n]=c;
		n++;
	}

	UpdateBut();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPtnDialog::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateSel();
	*pResult = 0;
}

void CPtnDialog::UpdateSel()
{
	POSITION sp=m_patlist.GetFirstSelectedItemPosition();
	int nItem = m_patlist.GetNextSelectedItem(sp);
   
	if (nItem>=0 && nItem<MAX_PATTERNS)
	ouPat=PIndex[nItem];
	
	UpdateBut();
}

void CPtnDialog::UpdateBut() 
{
	char buffer[64];
	
	if(ouPat==-1)
	{
	m_okbut.EnableWindow(false);
	}
	else
	{
	sprintf(buffer,"%d: %s",ouPat,songRef->patternName[ouPat]);
	m_pnlabel.SetWindowText(buffer);
	m_okbut.EnableWindow(true);
	}
}

void CPtnDialog::OnCreateb() 
{
	ouPat=-1;
	OnOK();
}

void CPtnDialog::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(ouPat>-1 && ouPat<MAX_PATTERNS)
	OnOK();
	
	*pResult = 0;
}
