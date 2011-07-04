#if !defined(AFX_MIDIMONITORDLG_H__02B9AD20_3290_11D5_B834_444553540000__INCLUDED_)
#define AFX_MIDIMONITORDLG_H__02B9AD20_3290_11D5_B834_444553540000__INCLUDED_

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	DARK_GREEN	RGB( 0, 128, 0 )
#define	DARK_RED	RGB( 128, 0, 0 )	

/////////////////////////////////////////////////////////////////////////////
// CMidiMonitorDlg dialog

class CMidiMonitorDlg : public CDialogImpl<CMidiMonitorDlg>
{
// Construction
public:
	CMidiMonitorDlg();   // standard constructor

	void InitTimer( void );		// start the timer
	void UpdateInfo( void );		// update the values in the dialog

	enum { IDD = IDD_MIDI_MONITOR };
	CStatic	m_tickSync;
	CStatic	m_midiSyncStart;
	CStatic	m_midiSyncStop;
	CStatic	m_midiSyncClock;
	CStatic	m_emulatedSyncStart;
	CStatic	m_emulatedSyncStop;
	CStatic	m_emulatedSyncClock;
	CStatic	m_syncronising;
	CStatic	m_resyncTriggered;
	CStatic	m_receivingMidiData;
	CStatic	m_psycleMidiActive;
	CStatic	m_syncLatency;
	CStatic	m_bufferCapacity;
	CStatic	m_midiVersion;
	CStatic	m_midiHeadroom;
	CStatic	m_syncOffset;
	CStatic	m_syncAdjust;
	CStatic	m_eventsLost;
	CStatic	m_bufferUsed;
	CStatic m_clearEventsLost;
	CListViewCtrl	m_channelMap;
	CStatic m_ch1;
	CStatic m_ch2;
	CStatic m_ch3;
	CStatic m_ch4;
	CStatic m_ch5;
	CStatic m_ch6;
	CStatic m_ch7;
	CStatic m_ch8;
	CStatic m_ch9;
	CStatic m_ch10;
	CStatic m_ch11;
	CStatic m_ch12;
	CStatic m_ch13;
	CStatic m_ch14;
	CStatic m_ch15;
	CStatic m_ch16;


//	virtual BOOL OnInitDialog();
	void fnClearEventsLost();


private:
	void SetStaticFlag( CStatic * pStatic, DWORD flags, DWORD flagMask );	// used for dot control
	void CreateChannelMap( void );	// create the channel map table
	void FillChannelMap( bool override = false );	// update the channel map table

	int m_clearCounter;		// use for the 'clear lost events' button
	CFont m_symbolFont;		// custom graphic font
public:
	BEGIN_MSG_MAP(CMidiMonitorDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CLEAR_EVENTS_LOST, STN_CLICKED, OnStnClickedClearEventsLost)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
//		MESSAGE_HANDLER(WM_CLOSE, OnClose)
MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnStnClickedClearEventsLost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
//	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDIMONITORDLG_H__02B9AD20_3290_11D5_B834_444553540000__INCLUDED_)
