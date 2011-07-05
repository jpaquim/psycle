///\file
///\brief interface file for psycle::host::CASIOConfig.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

#include <psycle/audiodrivers/asiointerface.h>
#include "Registry.hpp"
#include "configuration.hpp"

namespace psycle { namespace host {

		/// asio config window.
		class CASIOConfig : public CDialog
		{
		public:
			int m_sampleRate;
			int	m_bufferSize;
			CASIOConfig(CWnd* pParent = 0);
			audiodrivers::ASIOInterface* pASIO;

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


		class AsioUi : public audiodrivers::AsioUiInterface {
		public:
			AsioUi() {}
			~AsioUi() {}

			int DoModal(psycle::audiodrivers::ASIOInterface* asio) {
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
				configured &= ERROR_SUCCESS == reg.QueryValue("BufferSize", buffer_size);
				configured &= ERROR_SUCCESS == reg.QueryValue("DriverID", device_id);
				configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", sample_rate);
				reg.CloseKey();
				reg.CloseRootKey();
				if ( saveatend ) WriteConfig(buffer_size, device_id, sample_rate);
			}

			/*override*/ void Error(std::string const & msg) {
				MessageBox(0, msg.c_str(), "Error in ASIO audiodriver", MB_OK | MB_ICONERROR);
			}
		private:
			CASIOConfig dlg_;
		};


		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}   // namespace
}   // namespace
