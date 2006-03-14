#if !defined(AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_)
#define AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "constants.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg dialog

class CSaveWavDlg : public CDialogImpl<CSaveWavDlg>
{
// Construction
public:
	void SaveTick(void);
	CSaveWavDlg();   // standard constructor
	void SaveEnd(void);
	int kill_thread;
	int threadopen;

	enum { IDD = IDD_SAVEWAVDLG };
	CButton	m_cancel;
	CButton	m_savewave;
	CButton	m_savewires;
	CButton	m_savegens;
	CButton	m_savetracks;
	CEdit	m_rangestart;
	CEdit	m_rangeend;
	CProgressBarCtrl	m_progress;
	CEdit	m_patnumber;
	CEdit	m_filename;
	CStatic m_text;
	int		m_recmode;
	CButton m_radio_recmode;
	CComboBox	m_rate;
	CComboBox	m_bits;
	CComboBox	m_channelmode;

// Implementation
protected:
	HANDLE thread_handle;

	int lastpostick;
	int lastlinetick;
	int tickcont;

	static int rate;
	static int bits;
	static int channelmode;

	int current;

	TCHAR rootname[MAX_PATH];

	static BOOL savetracks;
	static BOOL savewires;
	static BOOL savegens;

	bool _Muted[MAX_BUSES];

	bool autostop;
	bool playblock;
	bool sel[MAX_SONG_POSITIONS];
	bool saving;

	void SaveWav(TCHAR* file, int bits, int rate, int channelmode);
public:
	BEGIN_MSG_MAP(CSaveWavDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_FILEBROWSE, BN_CLICKED, OnBnClickedFilebrowse)
		COMMAND_HANDLER(IDC_SAVEWIRESSEPARATED, BN_CLICKED, OnBnClickedSavewiresseparated)
		COMMAND_HANDLER(IDC_SAVETRACKSSEPARATED, BN_CLICKED, OnBnClickedSavetracksseparated)
		COMMAND_HANDLER(IDC_SAVEGENERATORSEPARATED, BN_CLICKED, OnBnClickedSavegeneratorseparated)
		COMMAND_HANDLER(IDC_RECSONG, BN_CLICKED, OnBnClickedRecsong)
		COMMAND_HANDLER(IDC_RECPATTERN, BN_CLICKED, OnBnClickedRecpattern)
		COMMAND_HANDLER(IDC_RECRANGE, BN_CLICKED, OnBnClickedRecrange)
		COMMAND_HANDLER(IDC_COMBO_RATE, CBN_SELCHANGE, OnCbnSelchangeComboRate)
		COMMAND_HANDLER(IDC_COMBO_BITS, CBN_SELCHANGE, OnCbnSelchangeComboBits)
		COMMAND_HANDLER(IDC_COMBO_CHANNELS, CBN_SELCHANGE, OnCbnSelchangeComboChannels)
		COMMAND_HANDLER(IDC_SAVEWAVE, BN_CLICKED, OnBnClickedSavewave)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedFilebrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSavewiresseparated(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSavetracksseparated(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSavegeneratorseparated(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRecsong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRecpattern(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRecrange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeComboRate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeComboBits(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeComboChannels(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSavewave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_)
