// GearRackDlg.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "GearRackDlg.h"
#include "Song.h"
#include "Machine.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearRackDlg dialog


CGearRackDlg::CGearRackDlg(CChildView* pParent, CMainFrame* pMain /*=NULL*/)
	: CDialog(CGearRackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearRackDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pParent = pParent;
	pParentMain = pMain;
}


void CGearRackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearRackDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_GEARLIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearRackDlg, CDialog)
	//{{AFX_MSG_MAP(CGearRackDlg)
	ON_BN_CLICKED(IDC_CREATE, OnCreate)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_LBN_DBLCLK(IDC_GEARLIST, OnDblclkGearlist)
	ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
	ON_BN_CLICKED(IDC_PARAMETERS, OnParameters)
	ON_LBN_SELCHANGE(IDC_GEARLIST, OnSelchangeGearlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearRackDlg message handlers

BOOL CGearRackDlg::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

BOOL CGearRackDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// fill our list box and select the currently selected machine
	RedrawList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGearRackDlg::OnCancel()
{
	pParentMain->pGearRackDialog = NULL;
	DestroyWindow();
	delete this;
}

BOOL CGearRackDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
	{
		m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CGearRackDlg::RedrawList() 
{
	char buffer[64];
	
	m_list.ResetContent();
	
	for (int b=0; b<MAX_BUSES; b++) // Check Generators
	{
		const int mac = Global::_pSong->busMachine[b]; 
		if( mac != 255 && Global::_pSong->_machineActive[mac])
		{
			sprintf(buffer,"%.2X: %s",b,Global::_pSong->_pMachines[mac]->_editName);
			m_list.AddString(buffer);
		}
		else
		{
			sprintf(buffer,"%.2X",b);
			m_list.AddString(buffer);
		}
	}
	m_list.AddString("--------------------------------------------------------------------------------------------------------");
	
	for (b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
	{
		const int mac = Global::_pSong->busEffect[b-MAX_BUSES];
		if(mac != 255 && Global::_pSong->_machineActive[mac])
		{
			sprintf(buffer,"%.2X: %s",b,Global::_pSong->_pMachines[mac]->_editName);
			m_list.AddString(buffer);
		}
		else
		{
			sprintf(buffer,"%.2X",b);
			m_list.AddString(buffer);
		}
	}

	int selected = Global::_pSong->seqBus;
	if (selected >= MAX_BUSES)
	{
		selected++;
	}
	m_list.SetCurSel(selected);
}

void CGearRackDlg::OnSelchangeGearlist() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	if (tmac == MAX_BUSES)
	{
		return;
	}
	if (tmac > MAX_BUSES)
	{
		tmac--;
	}
	Global::_pSong->seqBus = tmac;
	pParentMain->UpdateComboGen();
}

void CGearRackDlg::OnCreate() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	if (tmac == MAX_BUSES)
	{
		return;
	}
	if (tmac > MAX_BUSES)
	{
		tmac--;
	}
	Global::_pSong->seqBus = tmac;
	m_pParent->NewMachine(-1,-1,tmac);
	pParentMain->UpdateEnvInfo();
	pParentMain->UpdateComboGen();
	if (m_pParent->viewMode==VMMachine)
	{
		m_pParent->Repaint();
	}
}

void CGearRackDlg::OnDelete() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	if (tmac == MAX_BUSES)
	{
		return;
	}
	if (tmac > MAX_BUSES)
	{
		if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			tmac--;
			if (Global::_pSong->_machineActive[Global::_pSong->busEffect[tmac-MAX_BUSES]])
			{
				pParentMain->CloseMacGui(tmac-MAX_BUSES);
				Global::_pSong->DestroyMachine(Global::_pSong->busEffect[tmac-MAX_BUSES]);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				RedrawList();
				if (m_pParent->viewMode==VMMachine)
				{
					m_pParent->Repaint();
				}
			}
		}
	}
	else
	{
		if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			if (Global::_pSong->_machineActive[Global::_pSong->busMachine[tmac]])
			{
				pParentMain->CloseMacGui(tmac);
				Global::_pSong->DestroyMachine(Global::_pSong->busMachine[tmac]);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				RedrawList();
				if (m_pParent->viewMode==VMMachine)
				{
					m_pParent->Repaint();
				}
			}
		}
	}
}

void CGearRackDlg::OnDblclkGearlist() 
{
	// TODO: Add your control notification handler code here
	OnCreate();	
}


void CGearRackDlg::OnProperties() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	if (tmac == MAX_BUSES)
	{
		return;
	}
	if (tmac > MAX_BUSES)
	{
		tmac--;
		if (Global::_pSong->_machineActive[Global::_pSong->busEffect[tmac-MAX_BUSES]])
		{
			m_pParent->DoMacPropDialog(Global::_pSong->busEffect[tmac-MAX_BUSES]);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
	}
	else
	{
		if (Global::_pSong->_machineActive[Global::_pSong->busMachine[tmac]])
		{
			m_pParent->DoMacPropDialog(Global::_pSong->busMachine[tmac]);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
	}
}

void CGearRackDlg::OnParameters() 
{
	// TODO: Add your control notification handler code here
	POINT point;
	GetCursorPos(&point);
	int tmac = m_list.GetCurSel();
	if (tmac == MAX_BUSES)
	{
		return;
	}
	if (tmac > MAX_BUSES)
	{
		tmac--;
		if (Global::_pSong->_machineActive[Global::_pSong->busEffect[tmac-MAX_BUSES]])
		{
			pParentMain->ShowMachineGui(Global::_pSong->busEffect[tmac-MAX_BUSES],point);
		}
	}
	else
	{
		if (Global::_pSong->_machineActive[Global::_pSong->busMachine[tmac]])
		{
			pParentMain->ShowMachineGui(Global::_pSong->busMachine[tmac],point);
		}
	}
}

