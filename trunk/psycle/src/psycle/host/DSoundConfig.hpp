///\file
///\interface psycle::host::CDSoundConfig
#pragma once
#include "Psycle.hpp"
#include <afxwin.h>
#include <afxcmn.h> // CSpinButtonCtrl
#include "Registry.hpp"

#include <psycle/audiodrivers/microsoftdirectsoundout.h>
#include "Configuration.hpp"

namespace psycle { namespace host {

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

		class DSoundUi : public audiodrivers::DSoundUiInterface {
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

			virtual void WriteConfig(GUID device_guid, bool exclusive, bool dither,
								   int sample_rate, int buffer_size, int buffer_count) {
				Registry reg;
				if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
				{
					MessageBox(0, "Unable to write configuration to the registry", _T("DirectSound Output driver"), MB_OK | MB_ICONERROR);
					return;
				}
				if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\direct-sound") != ERROR_SUCCESS)
				{
					if(reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\direct-sound") != ERROR_SUCCESS)
					{
						MessageBox(0, "Unable to write configuration to the registry", _T("DirectSound Output driver"), MB_OK | MB_ICONERROR);
						return;
					}
				}
				reg.SetValue("DeviceGuid", device_guid);
				reg.SetValue("Exclusive", exclusive);
				reg.SetValue("Dither", dither);
				//reg.SetValue("BitDepth", _bitDepth);
				reg.SetValue("SamplesPerSec", sample_rate);
				reg.SetValue("BufferSize", buffer_size);
				reg.SetValue("NumBuffers", buffer_count);
				reg.CloseKey();
				reg.CloseRootKey();
			}

			virtual void ReadConfig(GUID& device_guid, bool& exclusive, bool& dither,
								   int& sample_rate, int& buffer_size, int& buffer_count) {
									   // default configuration
				bool saveatend(false);
				device_guid = GUID(); // DSDEVID_DefaultPlayback <-- unresolved external symbol
				exclusive = false;
				dither = false;
				sample_rate = 44100;
				buffer_size = 4096;
				buffer_count = 4;

				// read from registry
				Registry reg;
				reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT);
				if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\direct-sound") != ERROR_SUCCESS) // settings in version 1.8
				{
					reg.CloseRootKey();
					reg.OpenRootKey(HKEY_CURRENT_USER,PSYCLE__PATH__REGISTRY__ROOT "--1.7"); // settings in version 1.7 alpha
					if(reg.OpenKey("configuration\\devices\\direct-sound") != ERROR_SUCCESS)
					{
						reg.CloseRootKey();
						reg.OpenRootKey(HKEY_CURRENT_USER,"Software\\AAS\\Psycle\\CurrentVersion");
						if(reg.OpenKey("DirectSound") != ERROR_SUCCESS)
						{
							reg.CloseRootKey();
							return;
						}
					}
					saveatend=true;
				}
				reg.QueryValue("DeviceGuid", device_guid);
				reg.QueryValue("Exclusive", exclusive);
				reg.QueryValue("Dither", dither);
				reg.QueryValue("NumBuffers", buffer_count);
				reg.QueryValue("BufferSize", buffer_size);
				reg.QueryValue("SamplesPerSec", sample_rate);

				reg.CloseKey();
				reg.CloseRootKey();
				if(saveatend) WriteConfig(device_guid, exclusive, dither, sample_rate, buffer_size, buffer_count);
			}

			/*override*/ void Error(std::string const & msg) {
				MessageBox(0, msg.c_str(), "Error in DirectSound audiodriver", MB_OK | MB_ICONERROR);
			}

		private:
			CDSoundConfig dlg_;
		};

	}   // namespace
}   // namespace
