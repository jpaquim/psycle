///\file
///\brief implementation file for psycle::host::CASIOConfig.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/audiodrivers/ASIOConfig.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		namespace
		{
			int const MIN_NUMBUF = 1;
			int const MAX_NUMBUF = 16;

			int const MIN_BUFSIZE = 256;
			int const MAX_BUFSIZE = 32768;
		}

		CASIOConfig::CASIOConfig(CWnd* pParent)
		:
			CDialog(CASIOConfig::IDD, pParent),
			m_bufferSize(1024),
			m_driverIndex(-1)
		{
		}

		void CASIOConfig::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_ASIO_DRIVER, m_driverComboBox);
			DDX_CBIndex(pDX, IDC_ASIO_DRIVER, m_driverIndex);
			DDX_Control(pDX, IDC_ASIO_SAMPLERATE_COMBO, m_sampleRateCombo);
			DDX_Control(pDX, IDC_ASIO_BUFFERSIZE_COMBO, m_bufferSizeCombo);
			DDX_Control(pDX, IDC_ASIO_LATENCY, m_latency);
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
			int lat = (sbuf * (1000)) / sr;
			str.Format("Latency: %dms", lat);
			m_latency.SetWindowText(str);
		}

		BOOL CASIOConfig::OnInitDialog() 
		{
			CString str;
			CDialog::OnInitDialog();

			for (int i = 0; i < pASIO->drivercount; i++)
			{
				m_driverComboBox.AddString(pASIO->szFullName[i]);
			}

			if (m_driverIndex < 0 || m_driverIndex >= m_driverComboBox.GetCount())
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

			return true;
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
			int g = pASIO->Granularity[pASIO->driverindex[m_driverIndex]];
			if (g < 0)
			{
				for (int i = pASIO->minSamples[pASIO->driverindex[m_driverIndex]]; i <= pASIO->maxSamples[pASIO->driverindex[m_driverIndex]]; i *= 2)
				{
					if (i < pASIO->currentSamples[pASIO->driverindex[m_driverIndex]])
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

				for (int i = pASIO->minSamples[pASIO->driverindex[m_driverIndex]]; i <= pASIO->maxSamples[pASIO->driverindex[m_driverIndex]]; i += g)
				{
					if (i < pASIO->currentSamples[pASIO->driverindex[m_driverIndex]])
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

			pASIO->currentSamples[pASIO->driverindex[m_driverIndex]] = atoi(str);
			m_driverIndex = m_driverComboBox.GetCurSel();

			FillBufferBox();
		}

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
