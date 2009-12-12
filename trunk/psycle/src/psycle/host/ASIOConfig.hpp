///\file
///\brief interface file for psycle::host::CASIOConfig.
#pragma once
#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/audiodrivers/asiointerface.h>
#include "Registry.hpp"
#include "configuration.hpp"
#else
#include "ASIOInterface.hpp"
#endif

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		/// asio config window.
		class CASIOConfig : public CDialog
		{
		public:
			int m_sampleRate;
			int	m_bufferSize;
			CASIOConfig(CWnd* pParent = 0);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::ASIOInterface* pASIO;
#else
			ASIOInterface* pASIO;
#endif
		// Dialog Data
			//{{AFX_DATA(CASIOConfig)
			enum { IDD = IDD_ASIO_CONFIG };
			CComboBox	m_driverComboBox;
			CStatic	m_latency;
			CComboBox	m_sampleRateCombo;
			CComboBox	m_bufferSizeCombo;
			int		m_driverIndex;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CASIOConfig)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			void RecalcLatency();
			void FillBufferBox();
			// Generated message map functions
			//{{AFX_MSG(CASIOConfig)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			afx_msg void OnSelendokSamplerate();
			afx_msg void OnSelendokBuffersize();
			afx_msg void OnDestroy();
			afx_msg void OnExclusive();
			afx_msg void OnControlPanel();
			afx_msg void OnSelchangeAsioDriver();
			afx_msg void OnBnClickedOk();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};


#if PSYCLE__CONFIGURATION__USE_PSYCORE
		class AsioUi : public psycle::core::AsioUiInterface {
		public:
			AsioUi() {}
			~AsioUi() {}

			int DoModal(ASIOInterface* asio) {
				dlg_.pASIO = asio;
				return dlg_.DoModal();
			}

			virtual void SetValues(int device_id, int sample_rate, int buffer_size) {
				dlg_.m_bufferSize = buffer_size;
				dlg_.m_driverIndex = device_id;
				dlg_.m_sampleRate = sample_rate;
			}
			
			virtual void GetValues(int& device_id, int& sample_rate, int& buffer_size) {
				device_id = dlg_.m_driverIndex;
				sample_rate = dlg_.m_sampleRate;
				buffer_size = dlg_.m_bufferSize;
			}

			virtual void WriteConfig(int device_id, int sample_rate, int buffer_size) {
				Registry reg;
				if (reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
				{
					MessageBox(0, "Unable to write configuration to the registry", _T("Asio Output driver"), MB_OK | MB_ICONERROR);
					return;
				}
				if (reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\asio") != ERROR_SUCCESS)
				{
					if (reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\asio") != ERROR_SUCCESS)
					{
						MessageBox(0, "Unable to write configuration to the registry", _T("Asio Output driver"), MB_OK | MB_ICONERROR);						
						return;
					}
				}
				reg.SetValue("BufferSize", buffer_size);
				reg.SetValue("DriverID", device_id);
				reg.SetValue("SamplesPerSec", sample_rate);
				reg.CloseKey();
				reg.CloseRootKey();
			}

			virtual void ReadConfig(int& device_id, int& sample_rate, int& buffer_size) {
				bool saveatend(false);
				Registry reg;
				reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT);
				if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\asio") != ERROR_SUCCESS) // settings in version 1.8
				{
					reg.CloseRootKey();
					reg.OpenRootKey(HKEY_CURRENT_USER,PSYCLE__PATH__REGISTRY__ROOT "--1.7"); // settings in version 1.7 alpha
					if(reg.OpenKey("configuration\\devices\\asio") != ERROR_SUCCESS)
					{
						reg.CloseRootKey();
						reg.OpenRootKey(HKEY_CURRENT_USER,"Software\\AAS\\Psycle\\CurrentVersion");
						if(reg.OpenKey("ASIOOut") != ERROR_SUCCESS)
						{
							reg.CloseRootKey();
							return;
						}
					}
					saveatend=true;
				}
				bool configured(true);
				int driverID(0);
				configured &= ERROR_SUCCESS == reg.QueryValue("BufferSize", buffer_size);
				configured &= ERROR_SUCCESS == reg.QueryValue("DriverID", device_id);
				configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", sample_rate);
				reg.CloseKey();
				reg.CloseRootKey();
				if ( saveatend ) WriteConfig(buffer_size, device_id, sample_rate);
			}

		private:
			CASIOConfig dlg_;
		};
#endif


		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
