// OutputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "OutputDlg.h"
#include "MidiInput.h"
#include "Configuration.h"
#include "Player.h"
#include "Song.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputDlg dialog
IMPLEMENT_DYNCREATE(COutputDlg, CPropertyPage)

COutputDlg::COutputDlg() : CPropertyPage(COutputDlg::IDD)
{
	//{{AFX_DATA_INIT(COutputDlg)
	//}}AFX_DATA_INIT

	_numDrivers = 0;
	_numMidiDrivers = 0;
	m_midiHeadroom = 0;
	m_ppDrivers = NULL;
}


void COutputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutputDlg)
	DDX_Control(pDX, IDC_MIDI_DRIVER, m_midiDriverComboBox);
	DDX_Control(pDX, IDC_DRIVER, m_driverComboBox);
	DDX_Control(pDX, IDC_SYNC_DRIVER, m_midiSyncComboBox);
	DDX_Control(pDX, IDC_MIDI_HEADROOM, m_midiHeadroomEdit);
	DDX_Control(pDX, IDC_MIDI_HEADROOM_SPIN, m_midiHeadroomSpin);

	DDX_Control(pDX, IDC_MIDI_MACHINE_VIEW_SEQ_MODE, m_midiMachineViewSeqMode);
	DDX_Text(pDX, IDC_MIDI_HEADROOM, m_midiHeadroom);
	DDV_MinMaxInt(pDX, m_midiHeadroom, 0, 9999);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COutputDlg, CDialog)
	//{{AFX_MSG_MAP(COutputDlg)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputDlg message handlers

BOOL COutputDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for (int i=0; i < _numDrivers; i++)
	{
		const char* psDesc = m_ppDrivers[i]->GetInfo()->_psName;
		m_driverComboBox.AddString(psDesc);
	}

	if (m_driverIndex >= _numDrivers)
	{
		m_driverIndex = 0;
	}
	m_driverComboBox.SetCurSel(m_driverIndex);
	m_oldDriverIndex = m_driverIndex;

	// MIDI IMPLEMENTATION
	_numMidiDrivers = CMidiInput::Instance()->PopulateListbox( &m_midiDriverComboBox );
	if (m_midiDriverIndex > _numMidiDrivers)
	{
		m_midiDriverIndex = 0;
	}

	m_midiDriverComboBox.SetCurSel(m_midiDriverIndex);
	m_oldMidiDriverIndex = m_midiDriverIndex;

	_numMidiDrivers = CMidiInput::Instance()->PopulateListbox( &m_midiSyncComboBox );
	if (m_syncDriverIndex > _numMidiDrivers)
	{
		m_syncDriverIndex = 0;
	}

	m_midiSyncComboBox.SetCurSel(m_syncDriverIndex);
	m_oldSyncDriverIndex = m_syncDriverIndex;

	// setup spinner
	CString str;
	str.Format("%d", m_midiHeadroom);
	m_midiHeadroomEdit.SetWindowText(str);
	m_midiHeadroomSpin.SetRange32(MIN_HEADROOM, MAX_HEADROOM);

	UDACCEL acc;
	acc.nSec = 0;
	acc.nInc = 50;
	m_midiHeadroomSpin.SetAccel(1, &acc);

	m_midiMachineViewSeqMode.SetCheck(Global::pConfig->_midiMachineViewSeqMode?1:0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COutputDlg::OnOK() 
{
	m_driverIndex = m_driverComboBox.GetCurSel();

	if (m_driverIndex != m_oldDriverIndex)
	{
		m_ppDrivers[m_oldDriverIndex]->Enable(false);
	}

	// MIDI IMPLEMENTATION
	m_midiDriverIndex = m_midiDriverComboBox.GetCurSel();
	if( m_oldMidiDriverIndex != m_midiDriverIndex )
	{
		CMidiInput::Instance()->Close();
		CMidiInput::Instance()->SetDeviceId( DRIVER_MIDI, m_midiDriverIndex-1 );
	}
	m_syncDriverIndex = m_midiSyncComboBox.GetCurSel();
	if( m_oldSyncDriverIndex != m_syncDriverIndex )
	{
		CMidiInput::Instance()->Close();
		CMidiInput::Instance()->SetDeviceId( DRIVER_SYNC, m_syncDriverIndex-1 );
	}
	CMidiInput::Instance()->GetConfigPtr()->midiHeadroom = m_midiHeadroom;

	Global::pConfig->_midiMachineViewSeqMode = m_midiMachineViewSeqMode.GetCheck()?true:false;
	CDialog::OnOK();
}

void COutputDlg::OnCancel() 
{
	m_driverIndex = m_oldDriverIndex;
	m_midiDriverIndex = m_oldMidiDriverIndex;	// MIDI IMPLEMENTATION
	m_midiMachineViewSeqMode.SetCheck(Global::pConfig->_midiMachineViewSeqMode?1:0);
	CDialog::OnCancel();
}

void COutputDlg::OnConfig() 
{

	int index = m_driverComboBox.GetCurSel();
	m_ppDrivers[index]->Configure();
	if (Global::pPlayer->_playing)
	{
		Global::_pSong->SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
	}
	else
	{
		Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin, Global::_pSong->_ticksPerBeat, Global::pConfig->_pOutputDriver->_samplesPerSec);
	}
}

