///\file
///\brief implementation file for psycle::host::CMidiMonitorDlg.

#include "MidiMonitorDlg.hpp"
#include "MidiInput.hpp"
#ifdef use_psycore
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
using namespace psy::core;
#else
#include "Song.hpp"
#include "Machine.hpp"
#endif
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		CMidiMonitorDlg::CMidiMonitorDlg(CWnd* pParent)
			: CDialog(CMidiMonitorDlg::IDD, pParent)
			, m_clearCounter( 0 )
		{
			//{{AFX_DATA_INIT(CMidiMonitorDlg)
			//}}AFX_DATA_INIT
		}

		void CMidiMonitorDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CMidiMonitorDlg)
			DDX_Control(pDX, IDF_SYNC_TICK, m_tickSync);
			DDX_Control(pDX, IDF_EMULATED_SYNC_START, m_emulatedSyncStart);
			DDX_Control(pDX, IDF_EMULATED_SYNC_STOP, m_emulatedSyncStop);
			DDX_Control(pDX, IDF_EMULATED_SYNC_CLOCK, m_emulatedSyncClock);
			DDX_Control(pDX, IDF_MIDI_SYNC_START, m_midiSyncStart);
			DDX_Control(pDX, IDF_MIDI_SYNC_STOP, m_midiSyncStop);
			DDX_Control(pDX, IDF_MIDI_SYNC_CLOCK, m_midiSyncClock);
			DDX_Control(pDX, IDF_SYNCRONISING, m_syncronising);
			DDX_Control(pDX, IDF_RESYNC_TRIGGERED, m_resyncTriggered);
			DDX_Control(pDX, IDF_RECEIVING_MIDI_DATA, m_receivingMidiData);
			DDX_Control(pDX, IDF_PSYCLE_MIDI_ACTIVE, m_psycleMidiActive);
			DDX_Control(pDX, IDC_SYNC_LATENCY, m_syncLatency);
			DDX_Control(pDX, IDC_BUFFER_CAPACITY, m_bufferCapacity);
			DDX_Control(pDX, IDC_MIDI_VERSION, m_midiVersion);
			DDX_Control(pDX, IDC_MIDI_HEADROOM, m_midiHeadroom);
			DDX_Control(pDX, IDC_SYNC_OFFSET, m_syncOffset);
			DDX_Control(pDX, IDC_SYNC_ADJUST, m_syncAdjust);
			DDX_Control(pDX, IDC_EVENTS_LOST, m_eventsLost);
			DDX_Control(pDX, IDC_BUFFER_USED, m_bufferUsed);
			DDX_Control(pDX, IDC_CLEAR_EVENTS_LOST, m_clearEventsLost );
			DDX_Control(pDX, IDC_CHANNEL_MAP, m_channelMap );
			DDX_Control(pDX, IDC_CH1, m_ch1 );
			DDX_Control(pDX, IDC_CH2, m_ch2 );
			DDX_Control(pDX, IDC_CH3, m_ch3 );
			DDX_Control(pDX, IDC_CH4, m_ch4 );
			DDX_Control(pDX, IDC_CH5, m_ch5 );
			DDX_Control(pDX, IDC_CH6, m_ch6 );
			DDX_Control(pDX, IDC_CH7, m_ch7 );
			DDX_Control(pDX, IDC_CH8, m_ch8 );
			DDX_Control(pDX, IDC_CH9, m_ch9 );
			DDX_Control(pDX, IDC_CH10, m_ch10 );
			DDX_Control(pDX, IDC_CH11, m_ch11 );
			DDX_Control(pDX, IDC_CH12, m_ch12 );
			DDX_Control(pDX, IDC_CH13, m_ch13 );
			DDX_Control(pDX, IDC_CH14, m_ch14 );
			DDX_Control(pDX, IDC_CH15, m_ch15 );
			DDX_Control(pDX, IDC_CH16, m_ch16);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CMidiMonitorDlg, CDialog)
			//{{AFX_MSG_MAP(CMidiMonitorDlg)
			ON_WM_TIMER()
			ON_WM_CTLCOLOR()
			ON_COMMAND( IDC_CLEAR_EVENTS_LOST, fnClearEventsLost )
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CMidiMonitorDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			// create the table's columns
			m_channelMap.InsertColumn( 0,"Channel",LVCFMT_LEFT,54,0 );
			m_channelMap.InsertColumn( 1,"Generator/Effect",LVCFMT_LEFT,144,0 );
			m_channelMap.InsertColumn( 2,"Instrument",LVCFMT_LEFT,144,0  );
			m_channelMap.InsertColumn( 3,"Note Off",LVCFMT_LEFT,54,0  );

			// create the custom fonts
			m_symbolFont.CreatePointFont( 120, "Symbol" );

			// set them into the controls
			m_psycleMidiActive.SetFont( &m_symbolFont );
			m_tickSync.SetFont( &m_symbolFont );
			m_emulatedSyncStart.SetFont( &m_symbolFont );
			m_emulatedSyncStop.SetFont( &m_symbolFont );
			m_emulatedSyncClock.SetFont( &m_symbolFont );
			m_midiSyncStart.SetFont( &m_symbolFont );
			m_midiSyncStop.SetFont( &m_symbolFont );
			m_midiSyncClock.SetFont( &m_symbolFont );
			m_syncronising.SetFont( &m_symbolFont );
			m_resyncTriggered.SetFont( &m_symbolFont );
			m_receivingMidiData.SetFont( &m_symbolFont );
			m_ch1.SetFont( &m_symbolFont );
			m_ch2.SetFont( &m_symbolFont );
			m_ch3.SetFont( &m_symbolFont );
			m_ch4.SetFont( &m_symbolFont );
			m_ch5.SetFont( &m_symbolFont );
			m_ch6.SetFont( &m_symbolFont );
			m_ch7.SetFont( &m_symbolFont );
			m_ch8.SetFont( &m_symbolFont );
			m_ch9.SetFont( &m_symbolFont );
			m_ch10.SetFont( &m_symbolFont );
			m_ch11.SetFont( &m_symbolFont );
			m_ch12.SetFont( &m_symbolFont );
			m_ch13.SetFont( &m_symbolFont );
			m_ch14.SetFont( &m_symbolFont );
			m_ch15.SetFont( &m_symbolFont );
			m_ch16.SetFont( &m_symbolFont );

			// create the channel table
			CreateChannelMap();

			// initial update
			FillChannelMap( true );

			// start the dialog timer
			InitTimer();

			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// InitTimer
		//
		// DESCRIPTION	  : Start the dialog's update timer
		// PARAMETERS     : <void>
		// RETURNS		  : <void>

		void CMidiMonitorDlg::InitTimer()
		{
			// failed to setup timer?
			if( !SetTimer( 1, 250, NULL ) )
			{
				AfxMessageBox( "Could not start the dialog's update timer?", MB_ICONERROR + MB_OK );
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// OnTimer
		//
		// DESCRIPTION	  : Update the dialog on timer callback
		// PARAMETERS     : UNIT nIDEvent - timer ID
		// RETURNS		  : <void>

		void CMidiMonitorDlg::OnTimer(UINT nIDEvent) 
		{
			UpdateInfo();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// UpdateInfo
		//
		// DESCRIPTION	  : Update the dialog
		// PARAMETERS     : <void>
		// RETURNS		  : <void>

		void CMidiMonitorDlg::UpdateInfo( void )
		{
			char tmp[ 64 ];

			// fill in the numeric stats
			MIDI_STATS * pStats = CMidiInput::Instance()->GetStatsPtr();

			sprintf( tmp, "%d\0", pStats->bufferCount );
			m_bufferUsed.SetWindowText( tmp );
			sprintf( tmp, "%d\0", pStats->bufferSize );
			m_bufferCapacity.SetWindowText( tmp );
			sprintf( tmp, "%d\0", pStats->eventsLost );
			m_eventsLost.SetWindowText( tmp );
			sprintf( tmp, "%d\0", pStats->syncEventLatency );
			m_syncLatency.SetWindowText( tmp );
			sprintf( tmp, "%d\0", pStats->syncAdjuster );
			m_syncAdjust.SetWindowText( tmp );
			sprintf( tmp, "%d\0", pStats->syncOffset );
			m_syncOffset.SetWindowText( tmp );

			// add the config
			MIDI_CONFIG * pConfig = CMidiInput::Instance()->GetConfigPtr();

			m_midiVersion.SetWindowText( pConfig->versionStr );
			sprintf( tmp, "%d\0", pConfig->midiHeadroom );
			m_midiHeadroom.SetWindowText( tmp );

			// fill in the flags
			SetStaticFlag( &m_psycleMidiActive, pStats->flags, FSTAT_ACTIVE );
			SetStaticFlag( &m_receivingMidiData, pStats->flags, FSTAT_MIDI_INPUT );

			SetStaticFlag( &m_syncronising, pStats->flags, FSTAT_SYNC );
			SetStaticFlag( &m_resyncTriggered, pStats->flags, FSTAT_RESYNC );
			SetStaticFlag( &m_tickSync, pStats->flags, FSTAT_SYNC_TICK );

			SetStaticFlag( &m_midiSyncStart, pStats->flags, FSTAT_FASTART );
			SetStaticFlag( &m_midiSyncClock, pStats->flags, FSTAT_F8CLOCK );
			SetStaticFlag( &m_midiSyncStop, pStats->flags, FSTAT_FCSTOP );

			SetStaticFlag( &m_emulatedSyncStart, pStats->flags, FSTAT_EMULATED_FASTART );
			SetStaticFlag( &m_emulatedSyncClock, pStats->flags, FSTAT_EMULATED_F8CLOCK );
			SetStaticFlag( &m_emulatedSyncStop, pStats->flags, FSTAT_EMULATED_FCSTOP );

			// fill in the channel flags
			SetStaticFlag( &m_ch1, pStats->channelMap, (0x01 << 0) );
			SetStaticFlag( &m_ch2, pStats->channelMap, (0x01 << 1) );
			SetStaticFlag( &m_ch3, pStats->channelMap, (0x01 << 2) );
			SetStaticFlag( &m_ch4, pStats->channelMap, (0x01 << 3) );
			SetStaticFlag( &m_ch5, pStats->channelMap, (0x01 << 4) );
			SetStaticFlag( &m_ch6, pStats->channelMap, (0x01 << 5) );
			SetStaticFlag( &m_ch7, pStats->channelMap, (0x01 << 6) );
			SetStaticFlag( &m_ch8, pStats->channelMap, (0x01 << 7) );
			SetStaticFlag( &m_ch9, pStats->channelMap, (0x01 << 8) );
			SetStaticFlag( &m_ch10, pStats->channelMap, (0x01 << 9) );
			SetStaticFlag( &m_ch11, pStats->channelMap, (0x01 << 10) );
			SetStaticFlag( &m_ch12, pStats->channelMap, (0x01 << 11) );
			SetStaticFlag( &m_ch13, pStats->channelMap, (0x01 << 12) );
			SetStaticFlag( &m_ch14, pStats->channelMap, (0x01 << 13) );
			SetStaticFlag( &m_ch15, pStats->channelMap, (0x01 << 14) );
			SetStaticFlag( &m_ch16, pStats->channelMap, (0x01 << 15) );

			FillChannelMap();

			// clear down the flags
			pStats->flags &= FSTAT_CLEAR_WHEN_READ;
			pStats->channelMap = 0;

			// enable clear events lost
			if( m_clearCounter )
			{
				m_clearCounter--;
				if( m_clearCounter <= 0 )
				{
					m_clearEventsLost.EnableWindow( true );
				}
			}
			
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SetStaticFlag
		//
		// DESCRIPTION	  : Sets the static's text character depending on state of a flag
		// PARAMETERS     : CStatic * pStatic - the control
		//                : DWORD flags - the flag set
		//                : DWORD flagMask - the flag to check for
		// RETURNS		  : <void>

		void CMidiMonitorDlg::SetStaticFlag( CStatic * pStatic, DWORD flags, DWORD flagMask )
		{
			char tmp[ 2 ];

			// write the correct char
			if( flags & flagMask )
			{
				strcpy( tmp, "�" );	// (dot char)
			}
			else
			{
				tmp[ 0 ] = 0;	// (blank)
			}

			// set into the control
			pStatic->SetWindowText( tmp );
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// fnClearEventsLost
		//
		// DESCRIPTION	  : Reset the 'events lost' count
		// PARAMETERS     : <void>
		// RETURNS		  : <void>

		void CMidiMonitorDlg::fnClearEventsLost( void )
		{
			// clear the events lost counter
			MIDI_STATS * pStats = CMidiInput::Instance()->GetStatsPtr();
			pStats->eventsLost = 0;

			// disable ourselves until next after next n updates
			m_clearEventsLost.EnableWindow( false );
			m_clearCounter = 2;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// OnCtlColor
		//
		// DESCRIPTION	  : Intercept the dialog's colour control message
		// PARAMETERS     : <various MFC>
		// RETURNS		  : HBRUSH - background painting brush

		HBRUSH CMidiMonitorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
		{
			// get the default background brush
			HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

			MIDI_STATS * pStats = CMidiInput::Instance()->GetStatsPtr();

			// set required static colours

			if( pWnd == &m_psycleMidiActive )
				pDC->SetTextColor( DARK_GREEN );

			if( pWnd == &m_receivingMidiData )
				pDC->SetTextColor( DARK_GREEN );

			if( pWnd == &m_syncAdjust )
			{
				if( pStats->syncAdjuster < 0 )
				{
					// bad
					pDC->SetTextColor( DARK_RED );
				}
				else
				{
					// good
					pDC->SetTextColor( DARK_GREEN );
				}
			}

			if( pWnd == &m_resyncTriggered )
			{
				pDC->SetTextColor( DARK_RED );
			}

			return hbr;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// FillChannelMap
		//
		// DESCRIPTION	  : Fill the channel mapping table
		// PARAMETERS     : bool overridden - always fill?
		// RETURNS		  : <void>

		void CMidiMonitorDlg::FillChannelMap( bool overridden )
		{
			// get the midi input interface
			CMidiInput * pMidiInput = CMidiInput::Instance();
			if( !overridden && (!pMidiInput || pMidiInput->GetStatsPtr()->channelMapUpdate == false) )
			{
				return;
			}

			char txtBuffer[ 128 ];

			// for all MIDI channels
			for( int ch = 0; ch<MAX_MIDI_CHANNELS; ch++ )
			{
				// get generator/fx for this channel
				int genFxIdx = pMidiInput->GetGenMap( ch );

				// machine mapped & active?
				if( genFxIdx >= 0 && genFxIdx < MAX_MACHINES )
				{
					if( Global::_pSong->machine( genFxIdx ) )
					{
						// machine
						Machine * pMachine = Global::_pSong->machine( genFxIdx );
						sprintf( txtBuffer, "%02d: %s\0", genFxIdx, pMachine->GetEditName().c_str() );
						m_channelMap.SetItem( ch, 1, LVIF_TEXT, txtBuffer, 0, 0, 0, NULL );

						// instrument
						int instrument = pMidiInput->GetInstMap( ch );
#ifdef use_psycore
						if (pMachine->getMachineKey() == MachineKey::sampler()) {
							sprintf( txtBuffer, "%03d: %s\0", instrument, Global::_pSong->_pInstrument[instrument]->_sName );
							m_channelMap.SetItem( ch, 2, LVIF_TEXT, txtBuffer, 0, 0, 0, NULL );
						}
						else if ( pMachine->getMachineKey() == MachineKey::sampulse()) {
							sprintf( txtBuffer, "%03d: %s\0", instrument, Global::_pSong->rInstrument(instrument).Name() );
							m_channelMap.SetItem( ch, 2, LVIF_TEXT, txtBuffer, 0, 0, 0, NULL );
						}
#else
						if( pMachine->_type == MACH_SAMPLER )
						{
							sprintf( txtBuffer, "%03d: %s\0", instrument, Global::_pSong->_pInstrument[ instrument ]->_sName );
							m_channelMap.SetItem( ch, 2, LVIF_TEXT, txtBuffer, 0, 0, 0, NULL );
						}
						else if( pMachine->_type == MACH_XMSAMPLER )
						{
						}
#endif
						else
						{
							// n/a
							m_channelMap.SetItem( ch, 2, LVIF_TEXT, "n/a", 0, 0, 0, NULL );
						}

						// note on/off status
						if( pMidiInput->GetNoteOffStatus( ch ) )
						{
							// recognised
							m_channelMap.SetItem( ch, 3, LVIF_TEXT, "Yes", 0, 0, 0, NULL );
						}
						else
						{
							// ignoored
							m_channelMap.SetItem( ch, 3, LVIF_TEXT, "No", 0, 0, 0, NULL );
						}
					}
				}
				else
				{
					// channel not mapped at all
					m_channelMap.SetItem( ch, 1, LVIF_TEXT, "-", 0, 0, 0, NULL );
					m_channelMap.SetItem( ch, 2, LVIF_TEXT, "-", 0, 0, 0, NULL );
					m_channelMap.SetItem( ch, 3, LVIF_TEXT, "-", 0, 0, 0, NULL );
				}
			}

			// clear update strobe
			pMidiInput->GetStatsPtr()->channelMapUpdate = false;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CreateChannelMap
		//
		// DESCRIPTION	  : Create the initial channel table
		// PARAMETERS     : <void>
		// RETURNS		  : <void>

		void CMidiMonitorDlg::CreateChannelMap( void )
		{
			char txtBuffer[ 128 ];

			// for all MIDI channels
			for( int ch = 0; ch<MAX_MIDI_CHANNELS; ch++ )
			{
				sprintf( txtBuffer, "Ch %d\0", (ch+1) );
				m_channelMap.InsertItem( ch, txtBuffer, NULL );
			}
		}
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
