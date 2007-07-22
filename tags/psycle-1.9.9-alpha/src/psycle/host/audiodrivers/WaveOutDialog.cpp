///\file
///\brief implementation file for psycle::host::CWaveOutDialog.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/audiodrivers/WaveOutDialog.hpp>
#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#include <mmsystem.h>
	#pragma comment(lib, "winmm")
#pragma warning(pop)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CWaveOutDialog::CWaveOutDialog(CWnd* pParent) : CDialog(CWaveOutDialog::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CWaveOutDialog)
			m_BufNum = 0;
			m_BufSize = 0;
			m_Device = -1;
			m_Dither = false;
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
			return true;
			// return true unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return false
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
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
