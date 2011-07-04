#if !defined(AFX_ASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
#define AFX_ASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ASIOConfig.h : header file
//
#include "ASIOInterface.h"
/////////////////////////////////////////////////////////////////////////////
// CASIOConfig dialog


class CASIOConfig : public CDialogImpl<CASIOConfig>,public CWinDataExchange<CASIOConfig>
{
// Construction
public:
	int m_sampleRate;
	int	m_bufferSize;
	CASIOConfig();   // standard constructor
	ASIOInterface* pASIO;

	// Dialog Data

	enum { IDD = IDD_ASIO_CONFIG };

	CContainedWindowT<CComboBox>	m_driverComboBox;
	CContainedWindowT<WTL::CStatic>	m_latency;
	CContainedWindowT<WTL::CComboBox>	m_sampleRateCombo;
	CContainedWindowT<WTL::CComboBox>	m_bufferSizeCombo;
	int		m_driverIndex;

	BEGIN_DDX_MAP(CASIOConfig)
		DDX_INT(IDC_ASIO_DRIVER, m_driverIndex)
	END_DDX_MAP()

// Implementation
protected:
	void RecalcLatency();
	void FillBufferBox();
public:
	BEGIN_MSG_MAP(CASIOConfig)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDC_ASIO_DRIVER, CBN_SELCHANGE, OnCbnSelchangeAsioDriver)
		COMMAND_HANDLER(IDC_CONTROL_PANEL, BN_CLICKED, OnBnClickedControlPanel)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_ASIO_SAMPLERATE_COMBO, CBN_SELCHANGE, OnCbnSelchangeAsioSamplerateCombo)
		COMMAND_HANDLER(IDC_ASIO_BUFFERSIZE_COMBO, CBN_SELCHANGE, OnCbnSelchangeAsioBuffersizeCombo)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeAsioDriver(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedControlPanel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeAsioSamplerateCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeAsioBuffersizeCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
