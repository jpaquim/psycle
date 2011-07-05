///\file
///\brief implementation file for psycle::host::COutputDlg.

#include <psycle/host/detail/project.private.hpp>
#include "OutputDlg.hpp"

#include "MidiInput.hpp"
#include "Configuration.hpp"

#include <psycle/core/player.h>
#include <psycle/core/song.h>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

		IMPLEMENT_DYNCREATE(COutputDlg, CPropertyPage)

		COutputDlg::COutputDlg()
		:
			CPropertyPage(IDD),
			_numDrivers(0),
			m_driverIndex(0),
			_numMidiDrivers(0),
			m_midiDriverIndex(0),
			m_syncDriverIndex(0),
			m_midiHeadroom(0),
			m_ppDrivers(0)
		{
		}

		void COutputDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_MIDI_DRIVER, m_midiDriverComboBox);
			DDX_Control(pDX, IDC_DRIVER, m_driverComboBox);
			DDX_Control(pDX, IDC_SYNC_DRIVER, m_midiSyncComboBox);
			DDX_Control(pDX, IDC_MIDI_HEADROOM, m_midiHeadroomEdit);
			DDX_Control(pDX, IDC_MIDI_HEADROOM_SPIN, m_midiHeadroomSpin);
			DDX_Control(pDX, IDC_MIDI_MACHINE_VIEW_SEQ_MODE, m_midiMachineViewSeqMode);
			DDX_Text(pDX, IDC_MIDI_HEADROOM, m_midiHeadroom);
			DDV_MinMaxInt(pDX, m_midiHeadroom, MIN_HEADROOM, MAX_HEADROOM);
		}

		BEGIN_MESSAGE_MAP(COutputDlg, CDialog)
			ON_BN_CLICKED(IDC_CONFIG, OnConfig)
		END_MESSAGE_MAP()

		BOOL COutputDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			for (int i=0; i < _numDrivers; i++)
			{
				std::string name = m_ppDrivers[i]->info().header();
				m_driverComboBox.AddString(name.c_str());
			}

			if (m_driverIndex >= _numDrivers)
			{
				m_driverIndex = 0;
			}
			m_driverComboBox.SetCurSel(m_driverIndex);
			m_oldDriverIndex = m_driverIndex;

			_numMidiDrivers = CMidiInput::Instance()->PopulateListbox( &m_midiDriverComboBox, false );
			if (m_midiDriverIndex > _numMidiDrivers)
			{
				m_midiDriverIndex = 0;
			}

			m_midiDriverComboBox.SetCurSel(m_midiDriverIndex);
			m_oldMidiDriverIndex = m_midiDriverIndex;

			_numMidiDrivers = CMidiInput::Instance()->PopulateListbox( &m_midiSyncComboBox , true );
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

			m_midiMachineViewSeqMode.SetCheck(Global::pConfig->_midiMachineViewSeqMode);

			return TRUE;
		}

		void COutputDlg::OnOK() 
		{
			m_driverIndex = m_driverComboBox.GetCurSel();
			if (m_driverIndex != m_oldDriverIndex)
			{
				m_ppDrivers[m_oldDriverIndex]->set_started(false);
			}
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

			Global::pConfig->_midiMachineViewSeqMode = m_midiMachineViewSeqMode.GetCheck();
			CDialog::OnOK();
		}

		void COutputDlg::OnCancel() 
		{
			m_driverIndex = m_oldDriverIndex;
			m_midiDriverIndex = m_oldMidiDriverIndex;
			m_midiMachineViewSeqMode.SetCheck(Global::pConfig->_midiMachineViewSeqMode);
			CDialog::OnCancel();
		}

		void COutputDlg::OnConfig() 
		{

			int index = m_driverComboBox.GetCurSel();
			Player &player = Player::singleton();
			if (player.driver().info().name() != m_ppDrivers[index]->info().name()) {
				player.driver().set_opened(false);
				m_ppDrivers[index]->Configure();
				player.setDriver(*m_ppDrivers[index]);
			}
			else {
				m_ppDrivers[index]->Configure();
			}
			player.samples_per_second(m_ppDrivers[index]->playbackSettings().samplesPerSec());
		}

	}   // namespace
}   // namespace
