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

int CGearRackDlg::DisplayMode = 0;

CGearRackDlg::CGearRackDlg(CChildView* pParent, CMainFrame* pMain /*=NULL*/)
	: CDialog(CGearRackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearRackDlg)
	//}}AFX_DATA_INIT
	m_pParent = pParent;
	pParentMain = pMain;
}


void CGearRackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearRackDlg)
	DDX_Control(pDX, IDC_PROPERTIES, m_props);
	DDX_Control(pDX, IDC_RADIO_INS, m_radio_ins);
	DDX_Control(pDX, IDC_RADIO_GEN, m_radio_gen);
	DDX_Control(pDX, IDC_RADIO_EFX, m_radio_efx);
	DDX_Control(pDX, ID_TEXT, m_text);
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
	ON_BN_CLICKED(IDC_RADIO_EFX, OnRadioEfx)
	ON_BN_CLICKED(IDC_RADIO_GEN, OnRadioGen)
	ON_BN_CLICKED(IDC_RADIO_INS, OnRadioIns)
	ON_BN_CLICKED(IDC_EXCHANGE, OnExchange)
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

	int selected;
	int b;

	switch (DisplayMode)
	{
	case 0:
		m_text.SetWindowText("Machines: Generators");
		m_props.SetWindowText("Properties");
		m_radio_gen.SetCheck(1);
		m_radio_efx.SetCheck(0);
		m_radio_ins.SetCheck(0);

		selected = Global::_pSong->seqBus;
		if (selected >= MAX_BUSES)
		{
			selected = 0;
		}
		for (b=0; b<MAX_BUSES; b++) // Check Generators
		{
			if(Global::_pSong->_pMachine[b])
			{
				sprintf(buffer,"%.2X: %s",b,Global::_pSong->_pMachine[b]->_editName);
				m_list.AddString(buffer);
			}
			else
			{
				sprintf(buffer,"%.2X: empty",b);
				m_list.AddString(buffer);
			}
		}
		break;
	case 1:
		m_text.SetWindowText("Machines: Effects");
		m_props.SetWindowText("Properties");
		m_radio_gen.SetCheck(0);
		m_radio_efx.SetCheck(1);
		m_radio_ins.SetCheck(0);

		selected = Global::_pSong->seqBus;
		if (selected < MAX_BUSES)
		{
			selected = 0;
		}
		else
		{
			selected -= MAX_BUSES;
		}
		for (b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
		{
			if(Global::_pSong->_pMachine[b])
			{
				sprintf(buffer,"%.2X: %s",b,Global::_pSong->_pMachine[b]->_editName);
				m_list.AddString(buffer);
			}
			else
			{
				sprintf(buffer,"%.2X: empty",b);
				m_list.AddString(buffer);
			}
		}
		break;
	case 2:
		m_text.SetWindowText("Sample Instruments");
		m_props.SetWindowText("Wave Editor");
		m_radio_gen.SetCheck(0);
		m_radio_efx.SetCheck(0);
		m_radio_ins.SetCheck(1);

		char buffer[64];
		for (int b=0;b<PREV_WAV_INS;b++)
		{
			sprintf(buffer, "%.2X: %s", b, Global::_pSong->_instruments[b]._sName);
			m_list.AddString(buffer);
		}
		CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
		if (cc->GetCurSel() == AUX_WAVES)
		{
			selected = Global::_pSong->instSelected;
		}
		else
		{
			cc->SetCurSel(AUX_WAVES);
			pParentMain->UpdateComboIns(true);
			selected = Global::_pSong->instSelected;
		}
		break;
	}

	m_list.SetCurSel(selected);
}

void CGearRackDlg::OnSelchangeGearlist() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	switch (DisplayMode)
	{
	case 1:
		tmac += MAX_BUSES;
	case 0:
		Global::_pSong->seqBus = tmac;
		pParentMain->UpdateComboGen();
		break;
	case 2:
		{
			CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
			if (cc->GetCurSel() == AUX_WAVES)
			{
				Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
				pParentMain->UpdateComboIns(false);
			}
			else
			{
				cc->SetCurSel(AUX_WAVES);
				Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
				pParentMain->UpdateComboIns(true);
			}
			pParentMain->m_wndInst.WaveUpdate();
		}
		break;
	}
}

void CGearRackDlg::OnCreate() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	switch (DisplayMode)
	{
	case 1:
		tmac += MAX_BUSES;
	case 0:
		Global::_pSong->seqBus = tmac;
		m_pParent->NewMachine(-1,-1,tmac);
		pParentMain->UpdateEnvInfo();
		pParentMain->UpdateComboGen();
		if (m_pParent->viewMode==VMMachine)
		{
			m_pParent->Repaint();
		}
		break;
	case 2:
		{
			CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
			cc->SetCurSel(AUX_WAVES);
			Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}
		pParentMain->OnLoadwave();
		pParentMain->UpdateComboIns(true);
		break;
	}
	RedrawList();
}

void CGearRackDlg::OnDelete() 
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	switch (DisplayMode)
	{
	case 0:
		if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			if (Global::_pSong->_pMachine[tmac])
			{
				pParentMain->CloseMacGui(tmac);
				Global::_pSong->DestroyMachine(tmac);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				if (m_pParent->viewMode==VMMachine)
				{
					m_pParent->Repaint();
				}
			}
		}
		break;
	case 1:
		if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
			{
				pParentMain->CloseMacGui(tmac+MAX_BUSES);
				Global::_pSong->DestroyMachine(tmac+MAX_BUSES);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				if (m_pParent->viewMode==VMMachine)
				{
					m_pParent->Repaint();
				}
			}
		}
		break;
	case 2:
		{
			CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
			cc->SetCurSel(AUX_WAVES);
			Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}
		Global::_pSong->DeleteInstrument(Global::_pSong->instSelected);
		pParentMain->UpdateComboIns(true);
		break;
	}
	RedrawList();
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
	switch (DisplayMode)
	{
	case 0:
		if (Global::_pSong->_pMachine[tmac])
		{
			m_pParent->DoMacPropDialog(tmac);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
		break;
	case 1:
		if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
		{
			m_pParent->DoMacPropDialog(tmac+MAX_BUSES);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
		break;
	case 2:
		{
			CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
			cc->SetCurSel(AUX_WAVES);
			Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}

		pParentMain->m_pWndWed->ShowWindow(SW_SHOWNORMAL);
		pParentMain->m_pWndWed->SetActiveWindow();
		break;
	}
	RedrawList();
}

void CGearRackDlg::OnParameters() 
{
	// TODO: Add your control notification handler code here
	POINT point;
	GetCursorPos(&point);
	int tmac = m_list.GetCurSel();
	switch (DisplayMode)
	{
	case 0:
		if (Global::_pSong->_pMachine[tmac])
		{
			pParentMain->ShowMachineGui(tmac,point);
		}
		break;
	case 1:
		if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
		{
			pParentMain->ShowMachineGui(tmac+MAX_BUSES,point);
		}
		break;
	case 2:
		{
			CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
			cc->SetCurSel(AUX_WAVES);
			Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}
		pParentMain->ShowInstrumentEditor();
		pParentMain->UpdateComboIns(true);
		break;
	}
	RedrawList();
}

void CGearRackDlg::OnRadioGen() 
{
	// TODO: Add your control notification handler code here
	DisplayMode = 0;
	RedrawList();
}

void CGearRackDlg::OnRadioEfx() 
{
	// TODO: Add your control notification handler code here
	DisplayMode = 1;
	RedrawList();
	
}

void CGearRackDlg::OnRadioIns() 
{
	// TODO: Add your control notification handler code here
	DisplayMode = 2;
	RedrawList();
	
}

void CGearRackDlg::OnExchange() 
{
	if ( m_list.GetSelCount() != 2 )
	{
		MessageBox("This option requires that you select two entries","Gear Rack Dialog");
		return;
	}
	int sel[2],j=0;
	const int maxitems=m_list.GetCount();
	for (int c=0;c<maxitems;c++) 
	{
		if ( m_list.GetSel(c) != 0) sel[j++]=c;
	}
	Machine* tmp;
	switch (DisplayMode) // should be necessary to rename opened parameter windows.
	{
	case 0:
		tmp = Global::_pSong->_pMachine[sel[0]];
		Global::_pSong->_pMachine[sel[0]] = Global::_pSong->_pMachine[sel[1]];
		Global::_pSong->_pMachine[sel[1]] = tmp;
		pParentMain->UpdateComboGen(true);
		break;
	case 1:
		tmp = Global::_pSong->_pMachine[sel[0]+MAX_BUSES];
		Global::_pSong->_pMachine[sel[0]+MAX_BUSES] = Global::_pSong->_pMachine[sel[1]+MAX_BUSES];
		Global::_pSong->_pMachine[sel[1]+MAX_BUSES] = tmp;
		pParentMain->UpdateComboGen(true);
		break;
	case 2:
		Global::_pSong->Invalided=true;
		ExchangeIns(sel[0],sel[1]);
		
		Global::_pSong->Invalided=false;
		pParentMain->UpdateComboIns(true);
		break;
	}
	
	pParentMain->RedrawGearRackList();
}

void CGearRackDlg::ExchangeIns(int one,int two)
{
	Song* pSong = Global::_pSong;
	Instrument tmpins;
	
	tmpins=pSong->_instruments[one];
	pSong->_instruments[one]=pSong->_instruments[two];
	pSong->_instruments[two]=tmpins;
	//The above works because we are not creating new objects, just swaping them.
	//this means that no new data is generated/deleted,and the information is just
	//copied. If not, we would have had to define the operator=() function and take
	//care of it.


	char waveName[MAX_WAVES][32];
	unsigned short waveVolume[MAX_WAVES];
	signed short *waveDataL[MAX_WAVES];
	signed short *waveDataR[MAX_WAVES];
	unsigned int waveLength[MAX_WAVES];
	unsigned int waveLoopStart[MAX_WAVES];
	unsigned int waveLoopEnd[MAX_WAVES];
	int waveTune[MAX_WAVES];
	int waveFinetune[MAX_WAVES];	
	bool waveLoopType[MAX_WAVES];
	bool waveStereo[MAX_WAVES];

	for(int i=0;i<MAX_WAVES;i++)
	{
		//one to tmp
		strcpy(waveName[i],pSong->waveName[one][i]);
		waveVolume[i]=pSong->waveVolume[one][i];
		waveDataL[i]=pSong->waveDataL[one][i];
		waveDataR[i]=pSong->waveDataR[one][i];
		waveLength[i]=pSong->waveLength[one][i];
		waveLoopStart[i]=pSong->waveLoopStart[one][i];
		waveLoopEnd[i]=pSong->waveLoopEnd[one][i];
		waveTune[i]=pSong->waveTune[one][i];
		waveFinetune[i]=pSong->waveFinetune[one][i];
		waveLoopType[i]=pSong->waveLoopType[one][i];
		waveStereo[i]=pSong->waveStereo[one][i];

		//two to one
		strcpy(pSong->waveName[one][i],pSong->waveName[two][i]);
		pSong->waveVolume[one][i]=pSong->waveVolume[two][i];
		pSong->waveDataL[one][i]=pSong->waveDataL[two][i];
		pSong->waveDataR[one][i]=pSong->waveDataR[two][i];
		pSong->waveLength[one][i]=pSong->waveLength[two][i];
		pSong->waveLoopStart[one][i]=pSong->waveLoopStart[two][i];
		pSong->waveLoopEnd[one][i]=pSong->waveLoopEnd[two][i];
		pSong->waveTune[one][i]=pSong->waveTune[two][i];
		pSong->waveFinetune[one][i]=pSong->waveFinetune[two][i];
		pSong->waveLoopType[one][i]=pSong->waveLoopType[two][i];
		pSong->waveStereo[one][i]=pSong->waveStereo[two][i];

		//tmp to two
		strcpy(pSong->waveName[two][i],waveName[i]);
		pSong->waveVolume[two][i]=waveVolume[i];
		pSong->waveDataL[two][i]=waveDataL[i];
		pSong->waveDataR[two][i]=waveDataR[i];
		pSong->waveLength[two][i]=waveLength[i];
		pSong->waveLoopStart[two][i]=waveLoopStart[i];
		pSong->waveLoopEnd[two][i]=waveLoopEnd[i];
		pSong->waveTune[two][i]=waveTune[i];
		pSong->waveFinetune[two][i]=waveFinetune[i];
		pSong->waveLoopType[two][i]=waveLoopType[i];
		pSong->waveStereo[two][i]=waveStereo[i];
	}
}
