#if !defined(AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_)
#define AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OutputDlg.h : header file
//

#include "AudioDriver.h"

/////////////////////////////////////////////////////////////////////////////
// Defines

#define	MIN_HEADROOM	0
#define	MAX_HEADROOM	9999

/////////////////////////////////////////////////////////////////////////////
// COutputDlg dialog

class COutputDlg : public WTL::CPropertyPageImpl<COutputDlg>,public CWinDataExchange<COutputDlg>
//class COutputDlg : public CDialogImpl<COutputDlg>
{
//	DECLARE_DYNCREATE(COutputDlg)

// Construction
public:
	int m_driverIndex;
	int m_midiDriverIndex;		// MIDI IMPLEMENTATION
	int m_syncDriverIndex;
	int _numDrivers;
	int _numMidiDrivers;		// MIDI IMPLEMENTATION
	int m_midiHeadroom;

	AudioDriver** m_ppDrivers;

	COutputDlg();   // standard constructor

// Dialog Data
	enum { IDD = IDD_OUTPUT };

	CComboBox	m_midiDriverComboBox;
	CComboBox	m_midiSyncComboBox;
	CComboBox	m_driverComboBox;
	CEdit		m_midiHeadroomEdit;
	CUpDownCtrl	m_midiHeadroomSpin;
	CButton		m_midiMachineViewSeqMode;

	int OnApply();// - returns PSNRET_NOERROR to indicate the apply operation succeeded
	BOOL OnQueryCancel();
	BOOL OnKillActive();
	BEGIN_DDX_MAP(COutputDlg)
/*		DDX_CONTROL(IDC_MIDI_DRIVER, m_midiDriverComboBox);
		DDX_CONTROL(IDC_DRIVER, m_driverComboBox);
		DDX_CONTROL(IDC_SYNC_DRIVER, m_midiSyncComboBox);
		DDX_CONTROL(IDC_MIDI_HEADROOM, m_midiHeadroomEdit);
		DDX_CONTROL(IDC_MIDI_HEADROOM_SPIN, m_midiHeadroomSpin);
		DDX_CONTROL(IDC_MIDI_MACHINE_VIEW_SEQ_MODE, m_midiMachineViewSeqMode);*/
		DDX_INT(IDC_MIDI_HEADROOM, m_midiHeadroom);
		DDX_INT_RANGE(IDC_MIDI_HEADROOM,m_midiHeadroom, MIN_HEADROOM, MAX_HEADROOM);
	END_DDX_MAP()

// Implementation
protected:
	int m_oldDriverIndex;
	int m_oldMidiDriverIndex;		// MIDI IMPLEMENTATION
	int m_oldSyncDriverIndex;

public:
	BEGIN_MSG_MAP(COutputDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CONFIG, BN_CLICKED, OnBnClickedConfig)
//		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_)
