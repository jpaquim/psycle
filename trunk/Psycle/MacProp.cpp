// MacProp.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "MacProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMacProp dialog


CMacProp::CMacProp(CWnd* pParent /*=NULL*/)
	: CDialog(CMacProp::IDD, pParent)
{
	m_view=NULL;
	//{{AFX_DATA_INIT(CMacProp)
	//}}AFX_DATA_INIT
}


void CMacProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMacProp)
	DDX_Control(pDX, IDC_SOLO, m_soloCheck);
	DDX_Control(pDX, IDC_BYPASS, m_bypassCheck);
	DDX_Control(pDX, IDC_MUTE, m_muteCheck);
	DDX_Control(pDX, IDC_EDIT1, m_macname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMacProp, CDialog)
	//{{AFX_MSG_MAP(CMacProp)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_MUTE, OnMute)
	ON_BN_CLICKED(IDC_BYPASS, OnBypass)
	ON_BN_CLICKED(IDC_SOLO, OnSolo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacProp message handlers

BOOL CMacProp::OnInitDialog() 
{
	CDialog::OnInitDialog();

	deleted=false;

	m_macname.SetLimitText(15);
	char buffer[32];
	sprintf(buffer,"%.2X : %s Properties",Global::_pSong->FindBusFromIndex(thisMac),pMachine->_editName);
	SetWindowText(buffer);

	m_macname.SetWindowText(pMachine->_editName);

	m_muteCheck.SetCheck(pMachine->_mute);
	m_soloCheck.SetCheck(pSong->machineSoloed == thisMac);
	m_bypassCheck.SetCheck(pMachine->_bypass);
	if (pMachine->_mode == MACHMODE_GENERATOR ) m_bypassCheck.ShowWindow(SW_HIDE);
	else m_soloCheck.ShowWindow(SW_HIDE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMacProp::OnChangeEdit1() 
{
	m_macname.GetWindowText(txt, 16);
}

void CMacProp::OnButton1() 
{
	// Delete MACHINE!
	if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
	{
		deleted = true;
		OnCancel();
	}
}

void CMacProp::OnMute() 
{
	pMachine->_mute = (m_muteCheck.GetCheck() == 1);
	pMachine->_volumeCounter=0.0f;
	pMachine->_volumeDisplay = 0;
	if ( m_view != NULL )
	{
		m_view->updatePar=thisMac;
		m_view->Repaint(DMMacRefresh);
	}
}
void CMacProp::OnBypass() 
{
	pMachine->_bypass = (m_bypassCheck.GetCheck() == 1);
	if ( m_view != NULL )
	{
		m_view->updatePar=thisMac;
		m_view->Repaint(DMMacRefresh);
	}
}

void CMacProp::OnSolo() 
{
	 if (m_soloCheck.GetCheck() == 1)
	 {
		for ( int i=0;i<MAX_MACHINES;i++ )
		{
			if (( pSong->_machineActive[i] ) && 
				( pSong->_pMachines[i]->_mode == MACHMODE_GENERATOR ))
			{
				pSong->_pMachines[i]->_mute = true;
				pSong->_pMachines[i]->_volumeCounter=0.0f;
				pSong->_pMachines[i]->_volumeDisplay =0;
			}
		}
		pMachine->_mute = false;
		if ( m_muteCheck.GetCheck() ) m_muteCheck.SetCheck(0);
		pSong->machineSoloed = thisMac;
	 }
	 else
	 {
		pSong->machineSoloed = 0;
		for ( int i=0;i<MAX_MACHINES;i++ )
		{
			if (( pSong->_machineActive[i] ) && 
				( pSong->_pMachines[i]->_mode == MACHMODE_GENERATOR ))
					pSong->_pMachines[i]->_mute = false;
		}
		if ( m_muteCheck.GetCheck() ) m_muteCheck.SetCheck(0);
	}
	if ( m_view != NULL )
	{
		m_view->Repaint(DMAllMacsRefresh);
	}
}
