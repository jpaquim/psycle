///\interface psycle::host::CASIOConfig
#pragma once
#include <psycle/audiodrivers/ASIOInterface.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// asio config window.
		class CASIOConfig : public CDialog
		{
			DECLARE_MESSAGE_MAP()
			public:
				CASIOConfig(CWnd* pParent = 0);
				enum { IDD = IDD_ASIO_CONFIG };
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);
				virtual BOOL OnInitDialog();
				virtual void OnOK();
				afx_msg void OnSelendokSamplerate();
				afx_msg void OnSelendokBuffersize();
				afx_msg void OnDestroy();
				afx_msg void OnExclusive();
				afx_msg void OnControlPanel();
				afx_msg void OnSelchangeAsioDriver();
				afx_msg void OnBnClickedOk();
			private:
				CComboBox m_driverComboBox;
				CComboBox m_sampleRateCombo;
				CComboBox m_bufferSizeCombo;
				CStatic   m_latency;

			protected:
				void RecalcLatency();
				void FillBufferBox();
			private:
				friend class ASIOInterface;
				ASIOInterface* pASIO;
				int m_driverIndex;
				int m_sampleRate;
				int m_bufferSize;
		};
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
