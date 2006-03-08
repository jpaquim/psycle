///\file
///\brief interface file for psycle::host::CWaveOutDialog.
#pragma once
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// mme config window.
		class CWaveOutDialog : public CDialog
		{
			public:
				CWaveOutDialog(CWnd * pParent = 0);
				enum { IDD = IDD_WAVEOUTCONFIG };
				CStatic	m_Latency;
				CSpinButtonCtrl	m_BufSizeSpin;
				CSpinButtonCtrl	m_BufNumSpin;
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
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
				virtual BOOL OnInitDialog();
				virtual void OnOK();
				afx_msg void OnChangeConfigBufnum();
				afx_msg void OnChangeConfigBufsize();
				afx_msg void OnSelendokConfigSamplerate();
			DECLARE_MESSAGE_MAP()
			private:
				void RecalcLatency();
		};
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
