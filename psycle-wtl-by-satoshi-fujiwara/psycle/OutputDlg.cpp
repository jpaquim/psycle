/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
// OutputDlg.cpp : implementation file
//

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

#include "OutputDlg.h"
#include "MidiInput.h"
#include "configuration.h"
#include "Player.h"
#include "Song.h"
#include ".\outputdlg.h"



/////////////////////////////////////////////////////////////////////////////
// COutputDlg dialog
//IMPLEMENT_DYNCREATE(COutputDlg, CPropertyPage)

COutputDlg::COutputDlg()
{
	//{{AFX_DATA_INIT(COutputDlg)
	//}}AFX_DATA_INIT

	_numDrivers = 0;
	_numMidiDrivers = 0;
	m_midiHeadroom = 0;
	m_ppDrivers = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// COutputDlg message handlers

int COutputDlg::OnApply() 
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

	return PSNRET_NOERROR;
}

BOOL COutputDlg::OnQueryCancel()
{
	m_driverIndex = m_oldDriverIndex;
	m_midiDriverIndex = m_oldMidiDriverIndex;	// MIDI IMPLEMENTATION
	m_midiMachineViewSeqMode.SetCheck(Global::pConfig->_midiMachineViewSeqMode?1:0);
	return TRUE;
}


LRESULT COutputDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_midiDriverComboBox.Attach(GetDlgItem(IDC_MIDI_DRIVER));
	m_driverComboBox.Attach(GetDlgItem(IDC_DRIVER));
	m_midiSyncComboBox.Attach(GetDlgItem(IDC_SYNC_DRIVER));
	m_midiHeadroomEdit.Attach(GetDlgItem(IDC_MIDI_HEADROOM));
	m_midiHeadroomSpin.Attach(GetDlgItem(IDC_MIDI_HEADROOM_SPIN));
	m_midiMachineViewSeqMode.Attach(GetDlgItem(IDC_MIDI_MACHINE_VIEW_SEQ_MODE));
	DoDataExchange(FALSE);

	for (int i=0; i < _numDrivers; i++)
	{
		const TCHAR* psDesc = m_ppDrivers[i]->GetInfo()->_psName;
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
	str.Format(_T("%d"), m_midiHeadroom);
	m_midiHeadroomEdit.SetWindowText(str);
	m_midiHeadroomSpin.SetRange32(MIN_HEADROOM, MAX_HEADROOM);

	UDACCEL acc;
	acc.nSec = 0;
	acc.nInc = 50;
	m_midiHeadroomSpin.SetAccel(1, &acc);

	m_midiMachineViewSeqMode.SetCheck(Global::pConfig->_midiMachineViewSeqMode?1:0);

//	DoDataExchange(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT COutputDlg::OnBnClickedConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int index = m_driverComboBox.GetCurSel();
	m_ppDrivers[index]->Configure();
	if (Global::pPlayer->_playing)
	{
		Global::_pSong->SamplesPerTick((Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb));
	}
	else
	{
		Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin(), Global::_pSong->TicksPerBeat(), Global::pConfig->_pOutputDriver->_samplesPerSec);
	}
	return 0;
}
BOOL COutputDlg::OnKillActive(){
	DoDataExchange(TRUE);
	return TRUE;
}

