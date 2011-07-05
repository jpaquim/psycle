///\file
///\brief implementation file for psycle::host::CWaveOutDialog.

#include <psycle/host/detail/project.private.hpp>
#include "WaveOutDialog.hpp"
#include "Registry.hpp"
#include "Configuration.hpp"

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

	#include <mmsystem.h>
	#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "winmm")
	#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

namespace psycle { namespace host {

		CWaveOutDialog::CWaveOutDialog(CWnd* pParent) : CDialog(CWaveOutDialog::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CWaveOutDialog)
			m_BufNum = 0;
			m_BufSize = 0;
			m_Device = -1;
			m_Dither = FALSE;
			//}}AFX_DATA_INIT
		}

		void CWaveOutDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CWaveOutDialog)
			DDX_Control(pDX, IDC_CONFIG_LATENCY, m_Latency);
			DDX_Control(pDX, IDC_CONFIG_BUFSIZE_SPIN, m_BufSizeSpin);
			DDX_Control(pDX, IDC_CONFIG_BUFNUM_SPIN, m_BufNumSpin);
			DDX_Control(pDX, IDC_CONFIG_BUFSIZE, m_BufSizeEdit);
			DDX_Control(pDX, IDC_CONFIG_BUFNUM, m_BufNumEdit);
			DDX_Control(pDX, IDC_CONFIG_DITHER, m_DitherCheck);
			DDX_Control(pDX, IDC_CONFIG_DEVICE, m_DeviceList);
			DDX_Control(pDX, IDC_CONFIG_SAMPLERATE, m_SampleRateBox);
			DDX_Text(pDX, IDC_CONFIG_BUFNUM, m_BufNum);
			DDV_MinMaxInt(pDX, m_BufNum, 2, 8);
			DDX_Text(pDX, IDC_CONFIG_BUFSIZE, m_BufSize);
			DDV_MinMaxInt(pDX, m_BufSize, 512, 32256);
			DDX_CBIndex(pDX, IDC_CONFIG_DEVICE, m_Device);
			DDX_Check(pDX, IDC_CONFIG_DITHER, m_Dither);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CWaveOutDialog, CDialog)
			//{{AFX_MSG_MAP(CWaveOutDialog)
			ON_EN_CHANGE(IDC_CONFIG_BUFNUM, OnChangeConfigBufnum)
			ON_EN_CHANGE(IDC_CONFIG_BUFSIZE, OnChangeConfigBufsize)
			ON_CBN_SELENDOK(IDC_CONFIG_SAMPLERATE, OnSelendokConfigSamplerate)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		#define MIN_NBUF 2
		#define MAX_NBUF 8
		#define MIN_SBUF 512
		#define MAX_SBUF (32768 - 512)

		BOOL CWaveOutDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			// device list
			{
				int n = waveOutGetNumDevs();
			
				for (int i = 0; i < n; i++)
				{
					WAVEOUTCAPS caps;
					waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS));

					m_DeviceList.AddString(caps.szPname);
				}
					
				if (m_Device >= n)
					m_Device = 0;

				m_DeviceList.SetCurSel(m_Device);
			}
			// samplerate
			{
				CString str;
				str.Format("%d", m_SampleRate);
				
				int i = m_SampleRateBox.SelectString(-1, str);
				if (i == CB_ERR)
					i = m_SampleRateBox.SelectString(-1, "44100");
			}
			// dither
			m_DitherCheck.SetCheck(m_Dither ? 1 : 0);
			// buffers
			{
				CString str;
				str.Format("%d", m_BufNum);
				m_BufNumEdit.SetWindowText(str);
				m_BufNumSpin.SetRange(MIN_NBUF, MAX_NBUF);
				str.Format("%d", m_BufSize);
				m_BufSizeEdit.SetWindowText(str);
				m_BufSizeSpin.SetRange(MIN_SBUF, MAX_SBUF);
				UDACCEL acc;
				acc.nSec = 0;
				acc.nInc = 512;
				m_BufSizeSpin.SetAccel(1, &acc);
			}
			RecalcLatency();
			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CWaveOutDialog::OnOK() 
		{
			CString str;
			m_SampleRateBox.GetWindowText(str);
			m_SampleRate = atoi(str);
			
			CDialog::OnOK();
		}

		void CWaveOutDialog::RecalcLatency()
		{
			CString str;
			m_BufNumEdit.GetWindowText(str);
			int nbuf = atoi(str);

			if (nbuf < MIN_NBUF)
			{
				nbuf = MIN_NBUF;
				str.Format("%d", nbuf);
				m_BufNumEdit.SetWindowText(str);
			}
			else if (nbuf > MAX_NBUF)
			{
				nbuf = MAX_NBUF;
				str.Format("%d", nbuf);
				m_BufNumEdit.SetWindowText(str);
			}

			m_BufSizeEdit.GetWindowText(str);
			int sbuf = atoi(str);

			if (sbuf < MIN_SBUF)
			{
				sbuf = MIN_SBUF;
				str.Format("%d", sbuf);
				m_BufSizeEdit.SetWindowText(str);
			}
			else if (sbuf > MAX_SBUF)
			{
				sbuf = MAX_SBUF;
				str.Format("%d", sbuf);
				m_BufSizeEdit.SetWindowText(str);
			}
			
			m_SampleRateBox.GetWindowText(str);
			int sr = atoi(str);

			int totalbytes = nbuf * sbuf;

			int lat = (totalbytes * (1000 / 4)) / sr;

			str.Format("Latency: %dms", lat);
			m_Latency.SetWindowText(str);
		}

		void CWaveOutDialog::OnChangeConfigBufnum() 
		{
			if (!IsWindow(m_BufNumEdit.GetSafeHwnd()))
				return;

			RecalcLatency();
		}

		void CWaveOutDialog::OnChangeConfigBufsize() 
		{
			if (!IsWindow(m_BufSizeEdit.GetSafeHwnd()))
				return;

			RecalcLatency();
		}

		void CWaveOutDialog::OnSelendokConfigSamplerate() 
		{
			if (!IsWindow(m_SampleRateBox.GetSafeHwnd()))
				return;
			
			RecalcLatency();
		}



		void MMEUi::SetValues(
			int device_idx, bool dither,
			int sample_rate, int buffer_size, int buffer_count) {
			dlg_.m_BufNum = buffer_count;
			dlg_.m_BufSize = buffer_size;
			dlg_.m_Device = device_idx;
			dlg_.m_Dither = dither;
			dlg_.m_SampleRate = sample_rate;
		}

		void MMEUi::GetValues(
			int & device_idx, bool & dither,
			int & sample_rate, int & buffer_size, int & buffer_count) {
			buffer_count = dlg_.m_BufNum;
			buffer_size = dlg_.m_BufSize;
			device_idx = dlg_.m_Device;
			dither = (bool)dlg_.m_Dither;
			sample_rate = dlg_.m_SampleRate;
		}
			
		void MMEUi::WriteConfig(
				int device_idx, bool dither,
				int sample_rate, int buffer_size, int buffer_count) {
			Registry reg;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
			{
				Error("Unable to write configuration to the registry");
				return;
			}
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\mme") != ERROR_SUCCESS)
			{
				if (reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\mme") != ERROR_SUCCESS)
				{
					Error("Unable to write configuration to the registry");
					return;
				}
			}
			reg.SetValue("NumBlocks", buffer_count);
			reg.SetValue("BlockSize", buffer_size);
			reg.SetValue("DeviceID", device_idx);
			// Current driver doesn't use polling
			//reg.SetValue("PollSleep", _pollSleep);
			int dither_ = dither;
			reg.SetValue("Dither", dither_);
			reg.SetValue("SamplesPerSec", sample_rate);
			//reg.SetValue("BitDepth", _bitDepth);
			reg.CloseKey();
			reg.CloseRootKey();
		}

		void MMEUi::ReadConfig(
				int & device_idx, bool & dither,
				int & sample_rate, int & buffer_size, int & buffer_count) {
			bool saveatend=false;
			Registry reg;
			reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT);
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\mme") != ERROR_SUCCESS) // settings in version 1.8
			{
				reg.CloseRootKey();
				reg.OpenRootKey(HKEY_CURRENT_USER,PSYCLE__PATH__REGISTRY__ROOT "--1.7"); // settings in version 1.7 alpha
				if(reg.OpenKey("configuration\\devices\\mme") != ERROR_SUCCESS)
				{
					reg.CloseRootKey();
					reg.OpenRootKey(HKEY_CURRENT_USER,"Software\\AAS\\Psycle\\CurrentVersion");
					if(reg.OpenKey("WaveOut") != ERROR_SUCCESS)
					{
						reg.CloseRootKey();
						return;
					}
				}
				saveatend=true;
			}
			reg.QueryValue("NumBlocks", buffer_count);
			reg.QueryValue("BlockSize", buffer_size);
			reg.QueryValue("DeviceID", device_idx);
			// Current driver doesn't use polling
			//reg.QueryValue("PollSleep", _pollSleep);
			int dither_;
			reg.QueryValue("Dither", dither_);
			dither = dither_;
			reg.QueryValue("SamplesPerSec", sample_rate);
			//configured &= ERROR_SUCCESS == reg.QueryValue("BitDepth", _bitDepth);
			reg.CloseKey();
			reg.CloseRootKey();
			if(saveatend) WriteConfig(device_idx, dither, sample_rate, buffer_size, buffer_count);
		}

		void MMEUi::Error(std::string const & msg) {
			MessageBox(0, msg.c_str(), "Error in MME Waveout audiodriver", MB_OK | MB_ICONERROR);
		}
	}   // namespace
}   // namespace
