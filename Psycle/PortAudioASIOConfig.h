#if !defined(AFX_PORTAUDIOASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
#define AFX_PORTAUDIOASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortAudioASIOConfig.h : header file
//

#include "PortAudio.h"
#include <mmsystem.h>
#include <dsound.h>


/////////////////////////////////////////////////////////////////////////////
// CPortAudioASIOConfig dialog

class CPortAudioASIOConfig : public CDialog
{
// Construction
public:
	GUID m_deviceGuid;
	LPCGUID m_pDeviceGuid;
	int m_sampleRate;
	CPortAudioASIOConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPortAudioASIOConfig)
	enum { IDD = IDD_PORTAUDIO_ASIO_CONFIG };
	CStatic	m_latency;
	CSpinButtonCtrl	m_numBuffersSpin;
	CSpinButtonCtrl	m_bufferSizeSpin;
	CButton	m_ditherCheck;
	CEdit	m_bufferSizeEdit;
	CEdit	m_numBuffersEdit;
	CComboBox	m_sampleRateCombo;
	CComboBox	m_deviceComboBox;
	int		m_numBuffers;
	int		m_bufferSize;
	int		m_deviceIndex;
	BOOL	m_dither;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortAudioASIOConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	void RecalcLatency();
	// Generated message map functions
	//{{AFX_MSG(CPortAudioASIOConfig)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelendokSamplerate();
	afx_msg void OnChangeBufnumEdit();
	afx_msg void OnChangeBufsizeEdit();
	afx_msg void OnSelchangeDevice();
	afx_msg void OnDestroy();
	afx_msg void OnExclusive();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeDsoundDevice();
	afx_msg void OnBnClickedOk();
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTAUDIOASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
