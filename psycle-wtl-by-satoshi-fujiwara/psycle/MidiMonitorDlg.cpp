/** @file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "MidiMonitorDlg.h"
#include "MidiInput.h"
#include "song.h"
#include "Instrument.h"
#include "machine.h"




/////////////////////////////////////////////////////////////////////////////
// MidiMonitorDlg dialog


CMidiMonitorDlg::CMidiMonitorDlg():	m_clearCounter( 0 )
{
	//{{AFX_DATA_INIT(CMidiMonitorDlg)
	//}}AFX_DATA_INIT
}




/////////////////////////////////////////////////////////////////////////////
// CMidiMonitorDlg message handlers


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
		MessageBox(SF::CResourceString(IDS_ERR_MSG0111),_T(""), MB_ICONERROR + MB_OK );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UpdateInfo
//
// DESCRIPTION	  : Update the dialog
// PARAMETERS     : <void>
// RETURNS		  : <void>

void CMidiMonitorDlg::UpdateInfo( void )
{
	TCHAR tmp[ 64 ];

	// fill in the numeric stats
	MIDI_STATS * pStats = CMidiInput::Instance()->GetStatsPtr();

	_stprintf( tmp, _T("%d\0"), pStats->bufferCount );
	m_bufferUsed.SetWindowText( tmp );
	_stprintf( tmp, _T("%d\0"), pStats->bufferSize );
	m_bufferCapacity.SetWindowText( tmp );
	_stprintf( tmp, _T("%d\0"), pStats->eventsLost );
	m_eventsLost.SetWindowText( tmp );
	_stprintf( tmp, _T("%d\0"), pStats->syncEventLatency );
	m_syncLatency.SetWindowText( tmp );
	_stprintf( tmp, _T("%d\0"), pStats->syncAdjuster );
	m_syncAdjust.SetWindowText( tmp );
	_stprintf( tmp, _T("%d\0"), pStats->syncOffset );
	m_syncOffset.SetWindowText( tmp );

	// add the config
	MIDI_CONFIG * pConfig = CMidiInput::Instance()->GetConfigPtr();

	m_midiVersion.SetWindowText( pConfig->versionStr );
	_stprintf( tmp, _T("%d\0"), pConfig->midiHeadroom );
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
	TCHAR tmp[ 2 ];

	// write the correct char
	if( flags & flagMask )
	{
		_tcscpy( tmp, _T(".") );	// (dot char)
	}
	else
	{
		tmp[ 0 ] = 0;	// (blank)
	}

	// set into the control
	pStatic->SetWindowText( tmp );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FillChannelMap
//
// DESCRIPTION	  : Fill the channel mapping table
// PARAMETERS     : bool override - always fill?
// RETURNS		  : <void>

void CMidiMonitorDlg::FillChannelMap( bool override )
{
	// get the midi input interface
	CMidiInput * pMidiInput = CMidiInput::Instance();
	if( !override && (!pMidiInput || pMidiInput->GetStatsPtr()->channelMapUpdate == false) )
	{
		return;
	}

	TCHAR txtBuffer[ 128 ];

	// for all MIDI channels
	for( int ch = 0; ch<MAX_MIDI_CHANNELS; ch++ )
	{
		// get generator/fx for this channel
		int genFxIdx = pMidiInput->GetGenMap( ch );

		// machine mapped & active?
		if( genFxIdx >= 0 && genFxIdx < MAX_MACHINES )
		{
			if( Global::_pSong->pMachine( genFxIdx ) )
			{
				// machine
				Machine * pMachine = Global::_pSong->pMachine( genFxIdx );
				_stprintf( txtBuffer, _T("%02d: %s\0"), genFxIdx, pMachine->_editName );
				m_channelMap.SetItem( ch, 1, LVIF_TEXT, txtBuffer, 0, 0, 0, NULL );

				// instrument
				int instrument = pMidiInput->GetInstMap( ch );
				
				// required? (instruments only apply for samplers)
				if( pMachine->_type == MACH_SAMPLER )
				{
					_stprintf( txtBuffer, _T("%03d: %s\0"), instrument, Global::_pSong->pInstrument( instrument )->_sName );
					m_channelMap.SetItem( ch, 2, LVIF_TEXT, txtBuffer, 0, 0, 0, NULL );
				}
				else
				{
					// n/a
					m_channelMap.SetItem( ch, 2, LVIF_TEXT, _T("n/a"), 0, 0, 0, NULL );
				}

				// note on/off status
				if( pMidiInput->GetNoteOffStatus( ch ) )
				{
					// recognised
					m_channelMap.SetItem( ch, 3, LVIF_TEXT, _T("Yes"), 0, 0, 0, NULL );
				}
				else
				{
					// ignoored
					m_channelMap.SetItem( ch, 3, LVIF_TEXT, _T("No"), 0, 0, 0, NULL );
				}
			}
		}
		else
		{
			// channel not mapped at all
			m_channelMap.SetItem( ch, 1, LVIF_TEXT, _T("-"), 0, 0, 0, NULL );
			m_channelMap.SetItem( ch, 2, LVIF_TEXT, _T("-"), 0, 0, 0, NULL );
			m_channelMap.SetItem( ch, 3, LVIF_TEXT, _T("-"), 0, 0, 0, NULL );
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
	TCHAR txtBuffer[ 128 ];

	// for all MIDI channels
	for( int ch = 0; ch<MAX_MIDI_CHANNELS; ch++ )
	{
		_stprintf( txtBuffer, _T("Ch %d\0"), (ch+1) );
		m_channelMap.InsertItem( ch, txtBuffer, NULL );
	}
}

LRESULT CMidiMonitorDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_tickSync.Attach(GetDlgItem(IDF_SYNC_TICK));
	m_emulatedSyncStart.Attach(GetDlgItem(IDF_EMULATED_SYNC_START));
	m_emulatedSyncStop.Attach(GetDlgItem(IDF_EMULATED_SYNC_STOP));
	m_emulatedSyncClock.Attach(GetDlgItem(IDF_EMULATED_SYNC_CLOCK));
	m_midiSyncStart.Attach(GetDlgItem(IDF_MIDI_SYNC_START));
	m_midiSyncStop.Attach(GetDlgItem(IDF_MIDI_SYNC_STOP));
	m_midiSyncClock.Attach(GetDlgItem(IDF_MIDI_SYNC_CLOCK));
	m_syncronising.Attach(GetDlgItem(IDF_SYNCRONISING));
	m_resyncTriggered.Attach(GetDlgItem(IDF_RESYNC_TRIGGERED));
	m_receivingMidiData.Attach(GetDlgItem(IDF_RECEIVING_MIDI_DATA));
	m_psycleMidiActive.Attach(GetDlgItem(IDF_PSYCLE_MIDI_ACTIVE));
	m_syncLatency.Attach(GetDlgItem(IDC_SYNC_LATENCY));
	m_bufferCapacity.Attach(GetDlgItem(IDC_BUFFER_CAPACITY));
	m_midiVersion.Attach(GetDlgItem(IDC_MIDI_VERSION));
	m_midiHeadroom.Attach(GetDlgItem(IDC_MIDI_HEADROOM));
	m_syncOffset.Attach(GetDlgItem(IDC_SYNC_OFFSET));
	m_syncAdjust.Attach(GetDlgItem(IDC_SYNC_ADJUST));
	m_eventsLost.Attach(GetDlgItem(IDC_EVENTS_LOST));
	m_bufferUsed.Attach(GetDlgItem(IDC_BUFFER_USED));
	m_clearEventsLost.Attach(GetDlgItem(IDC_CLEAR_EVENTS_LOST));
	m_channelMap.Attach(GetDlgItem(IDC_CHANNEL_MAP));
	m_ch1.Attach(GetDlgItem(IDC_CH1));
	m_ch2.Attach(GetDlgItem(IDC_CH2));
	m_ch3.Attach(GetDlgItem(IDC_CH3));
	m_ch4.Attach(GetDlgItem(IDC_CH4));
	m_ch5.Attach(GetDlgItem(IDC_CH5));
	m_ch6.Attach(GetDlgItem(IDC_CH6));
	m_ch7.Attach(GetDlgItem(IDC_CH7));
	m_ch8.Attach(GetDlgItem(IDC_CH8));
	m_ch9.Attach(GetDlgItem(IDC_CH9));
	m_ch10.Attach(GetDlgItem(IDC_CH10));
	m_ch11.Attach(GetDlgItem(IDC_CH11));
	m_ch12.Attach(GetDlgItem(IDC_CH12));
	m_ch13.Attach(GetDlgItem(IDC_CH13));
	m_ch14.Attach(GetDlgItem(IDC_CH14));
	m_ch15.Attach(GetDlgItem(IDC_CH15));
	m_ch16.Attach(GetDlgItem(IDC_CH16));

	// create the table's columns
	m_channelMap.InsertColumn( 0,SF::CResourceString(IDS_MSG0078),LVCFMT_LEFT,54,0 );
	m_channelMap.InsertColumn( 1,SF::CResourceString(IDS_MSG0080),LVCFMT_LEFT,144,0 );
	m_channelMap.InsertColumn( 2,SF::CResourceString(IDS_MSG0081),LVCFMT_LEFT,144,0  );
	m_channelMap.InsertColumn( 3,SF::CResourceString(IDS_MSG0079),LVCFMT_LEFT,58,0  );

	// create the custom fonts
	m_symbolFont.CreatePointFont( 120, _T("Symbol"));

	// set them into the controls
	m_psycleMidiActive.SetFont((HFONT)m_symbolFont );
	m_tickSync.SetFont( (HFONT)m_symbolFont );
	m_emulatedSyncStart.SetFont( (HFONT)m_symbolFont );
	m_emulatedSyncStop.SetFont( (HFONT)m_symbolFont );
	m_emulatedSyncClock.SetFont( (HFONT)m_symbolFont );
	m_midiSyncStart.SetFont( (HFONT)m_symbolFont );
	m_midiSyncStop.SetFont( (HFONT)m_symbolFont );
	m_midiSyncClock.SetFont( (HFONT)m_symbolFont );
	m_syncronising.SetFont( (HFONT)m_symbolFont );
	m_resyncTriggered.SetFont( (HFONT)m_symbolFont );
	m_receivingMidiData.SetFont( (HFONT)m_symbolFont );
	m_ch1.SetFont( (HFONT)m_symbolFont );
	m_ch2.SetFont( (HFONT)m_symbolFont );
	m_ch3.SetFont( (HFONT)m_symbolFont );
	m_ch4.SetFont( (HFONT)m_symbolFont );
	m_ch5.SetFont( (HFONT)m_symbolFont );
	m_ch6.SetFont( (HFONT)m_symbolFont );
	m_ch7.SetFont( (HFONT)m_symbolFont );
	m_ch8.SetFont( (HFONT)m_symbolFont );
	m_ch9.SetFont( (HFONT)m_symbolFont );
	m_ch10.SetFont( (HFONT)m_symbolFont );
	m_ch11.SetFont( (HFONT)m_symbolFont );
	m_ch12.SetFont( (HFONT)m_symbolFont );
	m_ch13.SetFont( (HFONT)m_symbolFont );
	m_ch14.SetFont( (HFONT)m_symbolFont );
	m_ch15.SetFont( (HFONT)m_symbolFont );
	m_ch16.SetFont( (HFONT)m_symbolFont );

	// create the channel table
	CreateChannelMap();

	// initial update
	FillChannelMap( true );

	// start the dialog timer
	InitTimer();
	bHandled = FALSE;
	return TRUE;
}

LRESULT CMidiMonitorDlg::OnStnClickedClearEventsLost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	// clear the events lost counter
	MIDI_STATS * pStats = CMidiInput::Instance()->GetStatsPtr();
	pStats->eventsLost = 0;

	// disable ourselves until next after next n updates
	m_clearEventsLost.EnableWindow( false );
	m_clearCounter = 2;
	
	bHandled = FALSE;
	return 0;
}

LRESULT CMidiMonitorDlg::OnCtlColorDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// get the default background brush

	WTL::CDC _dc((HDC)wParam);
	
	HWND _ctl = (HWND) lParam;

	MIDI_STATS * pStats = CMidiInput::Instance()->GetStatsPtr();

	// set required static colours

	if( _ctl == (HWND)m_psycleMidiActive )
		_dc.SetTextColor( DARK_GREEN );

	if( _ctl == (HWND)m_receivingMidiData )
		_dc.SetTextColor( DARK_GREEN );

	if( _ctl == (HWND)m_syncAdjust )
	{
		if( pStats->syncAdjuster < 0 )
		{
			// bad
			_dc.SetTextColor( DARK_RED );
		}
		else
		{
			// good
			_dc.SetTextColor( DARK_GREEN );
		}
	}

	if( _ctl == (HWND)m_resyncTriggered )
	{
		_dc.SetTextColor( DARK_RED );
	}
	
	_dc.Detach();
	bHandled = TRUE;
	return 0;
}

LRESULT CMidiMonitorDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	
	UpdateInfo();
	bHandled = FALSE;
	return 0;
}

//LRESULT CMidiMonitorDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//{
//	EndDialog();
//	return 0;
//}

LRESULT CMidiMonitorDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	ShowWindow(SW_HIDE);
	bHandled = FALSE;	
	return 0;
}
