// OutputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "OutputDlg.h"
#include "MidiInput.h"
#include "Configuration.h"

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
	DDX_Control(pDX, IDC_MIDI_RECORD_VELOCITY, m_midiRecordVelocityButton);
	DDX_Control(pDX, IDC_MIDI_RECORD_PITCH, m_midiRecordPitchButton);
	DDX_Control(pDX, IDC_MIDI_RECORD_MODWHEEL, m_midiRecordModwheelButton);
	DDX_Control(pDX, IDC_MIDI_VEL_TYPE, m_midiVelocityTypeComboBox);
	DDX_Control(pDX, IDC_MIDI_VEL_COMMAND, m_midiVelocityCommandEdit);
	DDX_Control(pDX, IDC_MIDI_VEL_FROM, m_midiVelocityFromEdit);
	DDX_Control(pDX, IDC_MIDI_VEL_TO, m_midiVelocityToEdit);
	DDX_Control(pDX, IDC_MIDI_PITCH_TYPE, m_midiPitchTypeComboBox);
	DDX_Control(pDX, IDC_MIDI_PITCH_COMMAND, m_midiPitchCommandEdit);
	DDX_Control(pDX, IDC_MIDI_PITCH_FROM, m_midiPitchFromEdit);
	DDX_Control(pDX, IDC_MIDI_PITCH_TO, m_midiPitchToEdit);
	DDX_Control(pDX, IDC_MIDI_MODWHEEL_TYPE, m_midiModWheelTypeComboBox);
	DDX_Control(pDX, IDC_MIDI_MODWHEEL_COMMAND, m_midiModWheelCommandEdit);
	DDX_Control(pDX, IDC_MIDI_MODWHEEL_FROM, m_midiModWheelFromEdit);
	DDX_Control(pDX, IDC_MIDI_MODWHEEL_TO, m_midiModWheelToEdit);
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

	// new midi stuff

	m_midiRecordVelocityButton.SetCheck(Global::pConfig->_midiRecordVelocity?1:0);
	m_midiRecordPitchButton.SetCheck(Global::pConfig->_midiRecordPitchBend?1:0);
	m_midiRecordModwheelButton.SetCheck(Global::pConfig->_midiRecordModWheel?1:0);

	m_midiVelocityTypeComboBox.AddString("cmd");
	m_midiVelocityTypeComboBox.SetCurSel(0);
	m_midiPitchTypeComboBox.AddString("cmd");
	m_midiPitchTypeComboBox.AddString("twk");
	m_midiPitchTypeComboBox.SetCurSel(Global::pConfig->_midiPitchBendType);
	m_midiModWheelTypeComboBox.AddString("cmd");
	m_midiModWheelTypeComboBox.AddString("twk");
	m_midiModWheelTypeComboBox.SetCurSel(Global::pConfig->_midiModWheelType);

	str.Format("%x", Global::pConfig->_midiVelocityCommand);
	m_midiVelocityCommandEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiVelocityFrom);
	m_midiVelocityFromEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiVelocityTo);
	m_midiVelocityToEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiPitchBendCommand);
	m_midiPitchCommandEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiPitchBendFrom);
	m_midiPitchFromEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiPitchBendTo);
	m_midiPitchToEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiModWheelCommand);
	m_midiModWheelCommandEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiModWheelFrom);
	m_midiModWheelFromEdit.SetWindowText(str);
	str.Format("%x", Global::pConfig->_midiModWheelTo);
	m_midiModWheelToEdit.SetWindowText(str);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int _httoi(const TCHAR *value)
{
  struct CHexMap
  {
    TCHAR chr;
    int value;
  };
  const int HexMapL = 16;
  CHexMap HexMap[HexMapL] =
  {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13},
    {'E', 14}, {'F', 15}
  };
  TCHAR *mstr = _tcsupr(_tcsdup(value));
  TCHAR *s = mstr;
  int result = 0;
  if (*s == '0' && *(s + 1) == 'X') s += 2;
  bool firsttime = true;
  while (*s != '\0')
  {
    bool found = false;
    for (int i = 0; i < HexMapL; i++)
    {
      if (*s == HexMap[i].chr)
      {
        if (!firsttime) result <<= 4;
        result |= HexMap[i].value;
        found = true;
        break;
      }
    }
    if (!found) break;
    s++;
    firsttime = false;
  }
  free(mstr);
  return result;
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

	Global::pConfig->_midiRecordVelocity = m_midiRecordVelocityButton.GetCheck()?true:false;
	Global::pConfig->_midiRecordPitchBend = m_midiRecordPitchButton.GetCheck()?true:false;
	Global::pConfig->_midiRecordModWheel = m_midiRecordModwheelButton.GetCheck()?true:false;

	Global::pConfig->_midiPitchBendType = m_midiPitchTypeComboBox.GetCurSel();
	Global::pConfig->_midiModWheelType = m_midiModWheelTypeComboBox.GetCurSel();

	char str[7];
	m_midiVelocityCommandEdit.GetWindowText(str,6);
	Global::pConfig->_midiVelocityCommand = _httoi(str);

	m_midiVelocityFromEdit.GetWindowText(str,6);
	Global::pConfig->_midiVelocityFrom = _httoi(str);

	m_midiVelocityToEdit.GetWindowText(str,6);
	Global::pConfig->_midiVelocityTo = _httoi(str);

	m_midiPitchCommandEdit.GetWindowText(str,6);
	Global::pConfig->_midiPitchBendCommand = _httoi(str);

	m_midiPitchFromEdit.GetWindowText(str,6);
	Global::pConfig->_midiPitchBendFrom = _httoi(str);

	m_midiPitchToEdit.GetWindowText(str,6);
	Global::pConfig->_midiPitchBendTo = _httoi(str);

	m_midiModWheelCommandEdit.GetWindowText(str,6);
	Global::pConfig->_midiModWheelCommand = _httoi(str);

	m_midiModWheelFromEdit.GetWindowText(str,6);
	Global::pConfig->_midiModWheelFrom = _httoi(str);

	m_midiModWheelToEdit.GetWindowText(str,6);
	Global::pConfig->_midiModWheelTo = _httoi(str);

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
}

