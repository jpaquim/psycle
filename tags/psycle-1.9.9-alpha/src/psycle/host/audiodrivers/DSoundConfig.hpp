///\file
///\interface psycle::host::CDSoundConfig
#pragma once
#include <psycle/host/detail/project.hpp>
#include <psycle/host/resources/resources.hpp>
#include <afxwin.h>
#include <afxcmn.h> // CSpinButtonCtrl
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// direct sound config window.
		class CDSoundConfig : public CDialog
		{
				DECLARE_MESSAGE_MAP()
			public:
				CDSoundConfig(CWnd * pParent = 0);
				int const static IDD = IDD_DSOUNDCONFIG;
				CButton         m_exclusiveCheck;
				CStatic         m_latency;
				CSpinButtonCtrl m_numBuffersSpin;
				CSpinButtonCtrl m_bufferSizeSpin;
				CButton         m_ditherCheck;
				CEdit           m_bufferSizeEdit;
				CEdit           m_numBuffersEdit;
				CComboBox       m_sampleRateCombo;
				CComboBox       m_deviceComboBox;
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);
			private:
				virtual BOOL OnInitDialog();
				virtual void OnOK();
				afx_msg void OnSelendokSamplerate();
				afx_msg void OnChangeBufnumEdit();
				afx_msg void OnChangeBufsizeEdit();
				afx_msg void OnSelchangeDevice();
				afx_msg void OnDestroy();
				afx_msg void OnExclusive();
				afx_msg void OnDither();
			public:
				GUID              device_guid;
				int               sample_rate;
				int               buffer_count;
				int  const static buffer_count_min = 2;
				int  const static buffer_count_max = 16;
				int               buffer_size;
				int  const static buffer_size_min = 512;
				int  const static buffer_size_max = 32768;
				bool              dither;
				bool              exclusive;
			private:
				void RecalcLatency();
		};
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
