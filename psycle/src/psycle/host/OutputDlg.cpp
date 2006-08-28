///\file
///\brief implementation file for psycle::host::COutputDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/OutputDlg.hpp>
#include <psycle/engine/MidiInput.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/player.hpp>
#include <psycle/engine/song.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

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
				const char* psDesc = m_ppDrivers[i]->GetInfo()->_psName;
				m_driverComboBox.AddString(psDesc);
			}

			if (m_driverIndex >= _numDrivers)
			{
				m_driverIndex = 0;
			}
			m_driverComboBox.SetCurSel(m_driverIndex);
			m_oldDriverIndex = m_driverIndex;

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

			m_midiMachineViewSeqMode.SetCheck(Global::configuration()._midiMachineViewSeqMode);

			return true;
		}

		void COutputDlg::OnOK() 
		{
			m_driverIndex = m_driverComboBox.GetCurSel();
			if (m_driverIndex != m_oldDriverIndex)
			{
				m_ppDrivers[m_oldDriverIndex]->Enable(false);
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

			Global::configuration()._midiMachineViewSeqMode = m_midiMachineViewSeqMode.GetCheck();
			CDialog::OnOK();
		}

		void COutputDlg::OnCancel() 
		{
			m_driverIndex = m_oldDriverIndex;
			m_midiDriverIndex = m_oldMidiDriverIndex;
			m_midiMachineViewSeqMode.SetCheck(Global::configuration()._midiMachineViewSeqMode);
			CDialog::OnCancel();
		}

		void COutputDlg::OnConfig() 
		{

			int index = m_driverComboBox.GetCurSel();
			m_ppDrivers[index]->Configure();
			Global::player().SampleRate(Global::configuration().GetSamplesPerSec());
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
