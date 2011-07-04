#if !defined(AFX_ADRIVERDIALOG_H__FE0D36F1_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_ADRIVERDIALOG_H__FE0D36F1_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Adriver header
//

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDialog dialog

class CWaveOutDialog : public CDialogImpl<CWaveOutDialog>,public CWinDataExchange<CWaveOutDialog>
{
// Construction
public:
	CWaveOutDialog();   // standard constructor

	enum { IDD = IDD_WAVEOUTCONFIG };

	CStatic	m_Latency;
	CUpDownCtrl	m_BufSizeSpin;
	CUpDownCtrl	m_BufNumSpin;
	CEdit	m_BufSizeEdit;
	CEdit	m_BufNumEdit;
	CButton	m_DitherCheck;
	CComboBox	m_DeviceList;
	CComboBox	m_SampleRateBox;
	
	int		m_BufNum;
	int		m_BufSize;
	int		m_Device;
	BOOL	m_Dither;

	int m_SampleRate;

	BEGIN_DDX_MAP(CWaveOutDialog)
		DDX_INT_RANGE(IDC_CONFIG_BUFNUM,m_BufNum, 2, 8);
		DDX_INT_RANGE(IDC_CONFIG_BUFSIZE, m_BufSize,512, 32256);
		DDX_COMBO_INDEX(IDC_CONFIG_DEVICE, m_Device);
		DDX_CHECK(IDC_CONFIG_DITHER, m_Dither);
	END_DDX_MAP()


// Implementation
protected:

private:
	void RecalcLatency();
public:
	BEGIN_MSG_MAP(CWaveOutDialog)
		COMMAND_HANDLER(IDC_CONFIG_BUFNUM, EN_CHANGE, OnEnChangeConfigBufnum)
		COMMAND_HANDLER(IDC_CONFIG_BUFSIZE, EN_CHANGE, OnEnChangeConfigBufsize)
		COMMAND_HANDLER(IDC_CONFIG_DEVICE, CBN_SELENDOK, OnCbnSelendokConfigDevice)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnEnChangeConfigBufnum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeConfigBufsize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelendokConfigDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__8E063812_97DE_11D1_A298_2CC769000000__INCLUDED_)
