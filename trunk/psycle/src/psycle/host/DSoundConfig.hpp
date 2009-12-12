///\file
///\interface psycle::host::CDSoundConfig
#pragma once
#include "Psycle.hpp"
#include <afxwin.h>
#include <afxcmn.h> // CSpinButtonCtrl

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/audiodrivers/microsoftdirectsoundout.h>
#endif

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
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

#if PSYCLE__CONFIGURATION__USE_PSYCORE
		class DSoundUi : public psy::core::DSoundUiInterface {
		public:
			DSoundUi() {}
			~DSoundUi() {}

			int DoModal() {
				return dlg_.DoModal();
			}

			virtual void SetValues(GUID device_guid, bool exclusive, bool dither,
								   int sample_rate, int buffer_size, int buffer_count) {
					dlg_.device_guid = device_guid;
					dlg_.exclusive = exclusive;
					dlg_.dither = dither;
					dlg_.sample_rate = sample_rate;
					dlg_.buffer_size = buffer_size;
					dlg_.buffer_count = buffer_count;
			}
			
			virtual void GetValues(GUID& device_guid, bool& exclusive, bool& dither,
								   int& sample_rate, int& buffer_size, int& buffer_count) {
					device_guid = dlg_.device_guid;
					exclusive = dlg_.exclusive;
					dither = dlg_.dither;
					sample_rate = dlg_.sample_rate;
					buffer_size = dlg_.buffer_size;
					buffer_count = dlg_.buffer_count;
			}

		private:
			CDSoundConfig dlg_;
		};
#endif

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
