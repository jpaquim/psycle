///\file
///\brief implementation file for psycle::host::CASIOConfig.

#include <psycle/host/detail/project.private.hpp>
#include "ASIOConfig.hpp"
#include "ASIOInterface.hpp"

namespace psycle { namespace host {

		CASIOConfig::CASIOConfig(CWnd* pParent) : CDialog(CASIOConfig::IDD, pParent)
		{
			m_bufferSize = 1024;
			m_driverIndex = -1;
		}

		void CASIOConfig::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_ASIO_DRIVER, m_driverComboBox);
			DDX_Control(pDX, IDC_ASIO_LATENCY, m_latency);
			DDX_Control(pDX, IDC_ASIO_SAMPLERATE_COMBO, m_sampleRateCombo);
			DDX_CBIndex(pDX, IDC_ASIO_DRIVER, m_driverIndex);
			DDX_Control(pDX, IDC_ASIO_BUFFERSIZE_COMBO, m_bufferSizeCombo);
		}

		BEGIN_MESSAGE_MAP(CASIOConfig, CDialog)
			ON_CBN_SELENDOK(IDC_ASIO_SAMPLERATE_COMBO, OnSelendokSamplerate)
			ON_CBN_SELENDOK(IDC_ASIO_BUFFERSIZE_COMBO, OnSelendokBuffersize)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_CONTROL_PANEL, OnControlPanel)
			ON_CBN_SELCHANGE(IDC_ASIO_DRIVER, OnSelchangeAsioDriver)
			ON_BN_CLICKED(IDOK, OnBnClickedOk)
		END_MESSAGE_MAP()

		void CASIOConfig::RecalcLatency()
		{
			CString str;
			m_bufferSizeCombo.GetWindowText(str);
			int sbuf = atoi(str);
			m_sampleRateCombo.GetWindowText(str);
			int sr = atoi(str);
			//Multiplied by two becauase we have two buffers
			int lat = (sbuf * 2 * 1000) / sr;
			str.Format("Latency: %dms", lat);
			m_latency.SetWindowText(str);
		}

		BOOL CASIOConfig::OnInitDialog() 
		{
			CString str;
			CDialog::OnInitDialog();
			pASIO->RefreshAvailablePorts();
			std::vector<std::string> ports;
			pASIO->GetPlaybackPorts(ports);
			std::vector<std::string>::const_iterator it;
			for (it = ports.begin(); it != ports.end(); ++it)
			{
				m_driverComboBox.AddString(it->c_str());
			}

			if (m_driverIndex < 0)
			{
				m_driverIndex = 0;
			}

			else if (m_driverIndex >= m_driverComboBox.GetCount())
			{
				m_driverIndex = 0;
			}
			m_driverComboBox.SetCurSel(m_driverIndex);

			// Sample rate
			str.Format("%d", m_sampleRate);
			int i = m_sampleRateCombo.SelectString(-1, str);
			if (i == CB_ERR)
			{
				i = m_sampleRateCombo.SelectString(-1, "44100");
			}

			// Check boxes
			// recalc the buffers combo
			FillBufferBox();

			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CASIOConfig::OnOK() 
		{
			if (m_driverComboBox.GetCount() > 0)
			{

				CString str;
				m_sampleRateCombo.GetWindowText(str);
				m_sampleRate = atoi(str);

				m_bufferSizeCombo.GetWindowText(str);
				m_bufferSize = atoi(str);

				CDialog::OnOK();
				return;
			}
			CDialog::OnCancel();
		}

		void CASIOConfig::OnSelendokSamplerate() 
		{
			if (!IsWindow(m_sampleRateCombo.GetSafeHwnd()))
			{
				return;
			}
			RecalcLatency();
		}

		void CASIOConfig::OnSelendokBuffersize() 
		{
			if (!IsWindow(m_bufferSizeCombo.GetSafeHwnd()))
			{
				return;
			}
			RecalcLatency();
		}

		void CASIOConfig::OnDestroy() 
		{
			CDialog::OnDestroy();
		}

		void CASIOConfig::OnBnClickedOk()
		{
			OnOK();
		}

		void CASIOConfig::OnControlPanel() 
		{
			m_driverIndex = m_driverComboBox.GetCurSel();
			pASIO->ControlPanel(m_driverIndex);
		}
		void CASIOConfig::FillBufferBox()
		{
			// hmm we had better recalc our buffer options
			int prefindex = 0;
			char buf[8];
			m_bufferSizeCombo.ResetContent();
			ASIOInterface::DriverEnum driver = pASIO->GetDriverFromidx(m_driverIndex);
			int g = driver.granularity;
			if (g <= 0)
			{
				for (int i = driver.minSamples; i <= driver.maxSamples; i *= 2)
				{
					if (i < m_bufferSize)
					{
						prefindex++;
					}
					sprintf(buf,"%d",i);
					m_bufferSizeCombo.AddString(buf);
				}
			}
			else
			{
				if (g < 64)
				{
					g = 64;
				}

				for (int i = driver.minSamples; i <= driver.maxSamples; i += g)
				{
					if (i < m_bufferSize)
					{
						prefindex++;
					}
					sprintf(buf,"%d",i);
					m_bufferSizeCombo.AddString(buf);
				}
			}
			if (prefindex >= m_bufferSizeCombo.GetCount())
			{
				prefindex=m_bufferSizeCombo.GetCount()-1;
			}
			m_bufferSizeCombo.SetCurSel(prefindex);
			RecalcLatency();
		}

		void CASIOConfig::OnSelchangeAsioDriver() 
		{
			CString str;
			m_bufferSizeCombo.GetWindowText(str);

			m_driverIndex = m_driverComboBox.GetCurSel();

			FillBufferBox();

		}
	}   // namespace
}   // namespace
