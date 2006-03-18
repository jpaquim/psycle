// IBrowseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "IBrowseDlg.h"
#include "InstrumentEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIBrowseDlg dialog


CIBrowseDlg::CIBrowseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBrowseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIBrowseDlg)
	//}}AFX_DATA_INIT
}


void CIBrowseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIBrowseDlg)
	DDX_Control(pDX, IDC_LIST1, m_iblist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIBrowseDlg, CDialog)
	//{{AFX_MSG_MAP(CIBrowseDlg)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeIbList)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList)
	ON_BN_CLICKED(IDC_BUTTON1, OnEditBut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CIBrowseDlg::UpdateIList()
{
	m_iblist.ResetContent();
	anyInstrument=false;

	int n=0;

	for (int i=0;i<MAX_INSTRUMENTS;i++)
	{
		for(int w=0;w<MAX_WAVES;w++)
		{
			if(songRef->waveLength[i][w]>0)
			{
				char buffer[64];
				sprintf(buffer,"%.3d:  %s",i,songRef->instName[i]);
				m_iblist.AddString(buffer);
				ibIndex[n]=i;
				n++;
				anyInstrument=true;
			}
		}
	}

	if(!anyInstrument)
	{
		m_iblist.AddString("No instruments available");
		m_iblist.EnableWindow(false);
	}
	else
	{
		m_iblist.EnableWindow(true);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CIBrowseDlg message handlers

BOOL CIBrowseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateIList();

	return TRUE;
}

void CIBrowseDlg::OnSelchangeIbList() 
{
	if (anyInstrument)
	{
	int iSel=m_iblist.GetCurSel();
	songRef->instSelected=ibIndex[iSel];
	GetParent()->SetActiveWindow();
	}
}

void CIBrowseDlg::OnDblclkList() 
{
	ShowIE();
}

void CIBrowseDlg::OnEditBut() 
{
	ShowIE();	
}

void CIBrowseDlg::ShowIE() 
{
	CInstrumentEditor dlg;
	dlg.songRef=this->songRef;
	dlg.DoModal();
	UpdateIList();
}
