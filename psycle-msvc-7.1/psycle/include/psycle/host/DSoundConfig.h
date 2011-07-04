#if !defined(AFX_DSOUNDCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
#define AFX_DSOUNDCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DSoundConfig.h : header file
//

#include <mmsystem.h>
#include <dsound.h>


/////////////////////////////////////////////////////////////////////////////
// CDSoundConfig dialog

class CDSoundConfig : public CDialog
{
// Construction
public:
	GUID m_deviceGuid;
	LPCGUID m_pDeviceGuid;
	int m_sampleRate;
	CDSoundConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDSoundConfig)
	enum { IDD = IDD_DSOUNDCONFIG };
	CButton	m_exclusiveCheck;
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
	BOOL	m_exclusive;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSoundConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static BOOL CALLBACK EnumDSoundDevices(
		LPGUID lpGUID,
		const char* psDesc,
		const char* psDrvName,
		void* pDirectSound);

	void RecalcLatency();
	// Generated message map functions
	//{{AFX_MSG(CDSoundConfig)
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
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSOUNDCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
