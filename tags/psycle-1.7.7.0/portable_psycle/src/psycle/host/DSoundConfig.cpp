#include "stdafx.h"
#include "Psycle.h"
#include "DSoundConfig.h"
///\file
///\brief implementation file for psycle::host::CDSoundConfig.
namespace psycle
{
	namespace host
	{
		#define MIN_NUMBUF 2
		#define MAX_NUMBUF 16

		#define MIN_BUFSIZE 512
		#define MAX_BUFSIZE 32768

		CDSoundConfig::CDSoundConfig(CWnd* pParent) : CDialog(CDSoundConfig::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CDSoundConfig)
			m_numBuffers = 0;
			m_bufferSize = 0;
			m_deviceIndex = -1;
			m_dither = FALSE;
			m_exclusive = FALSE;
			//}}AFX_DATA_INIT
		}

		void CDSoundConfig::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CDSoundConfig)
			DDX_Control(pDX, IDC_EXCLUSIVE, m_exclusiveCheck);
			DDX_Control(pDX, IDC_DSOUND_LATENCY, m_latency);
			DDX_Control(pDX, IDC_DSOUND_BUFNUM_SPIN, m_numBuffersSpin);
			DDX_Control(pDX, IDC_DSOUND_BUFSIZE_SPIN, m_bufferSizeSpin);
			DDX_Control(pDX, IDC_DSOUND_DITHER, m_ditherCheck);
			DDX_Control(pDX, IDC_DSOUND_BUFSIZE_EDIT, m_bufferSizeEdit);
			DDX_Control(pDX, IDC_DSOUND_BUFNUM_EDIT, m_numBuffersEdit);
			DDX_Control(pDX, IDC_DSOUND_SAMPLERATE_COMBO, m_sampleRateCombo);
			DDX_Control(pDX, IDC_DSOUND_DEVICE, m_deviceComboBox);
			DDX_Text(pDX, IDC_DSOUND_BUFNUM_EDIT, m_numBuffers);
			DDV_MinMaxInt(pDX, m_numBuffers, 2, 16);
			DDX_Text(pDX, IDC_DSOUND_BUFSIZE_EDIT, m_bufferSize);
			DDV_MinMaxInt(pDX, m_bufferSize, 512, 32768);
			DDX_CBIndex(pDX, IDC_DSOUND_DEVICE, m_deviceIndex);
			DDX_Check(pDX, IDC_DSOUND_DITHER, m_dither);
			DDX_Check(pDX, IDC_EXCLUSIVE, m_exclusive);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CDSoundConfig, CDialog)
			//{{AFX_MSG_MAP(CDSoundConfig)
			ON_CBN_SELENDOK(IDC_DSOUND_SAMPLERATE_COMBO, OnSelendokSamplerate)
			ON_EN_CHANGE(IDC_DSOUND_BUFNUM_EDIT, OnChangeBufnumEdit)
			ON_EN_CHANGE(IDC_DSOUND_BUFSIZE_EDIT, OnChangeBufsizeEdit)
			ON_CBN_SELCHANGE(IDC_DSOUND_DEVICE, OnSelchangeDevice)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_EXCLUSIVE, OnExclusive)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CALLBACK
		CDSoundConfig::EnumDSoundDevices
			(
				LPGUID lpGUID,
				const char* psDesc,
				const char* psDrvName,
				void* context
			)
		{
			CComboBox* pBox = (CComboBox*)context;
			LPGUID pTemp = NULL;
		 
			if (lpGUID != NULL)
			{
				if ((pTemp = new GUID) == NULL)
				{
					return TRUE;
				}
				memcpy(pTemp, lpGUID, sizeof(GUID));
			}

			pBox->AddString(psDesc);
			pBox->SetItemData(pBox->FindString(0, psDesc), (DWORD)pTemp);
			return TRUE;
		}

		void CDSoundConfig::RecalcLatency()
		{
			CString str;
			m_numBuffersEdit.GetWindowText(str);
			int nbuf = atoi(str);
			m_bufferSizeEdit.GetWindowText(str);
			int sbuf = atoi(str);
			m_sampleRateCombo.GetWindowText(str);
			int sr = atoi(str);
			int totalbytes = nbuf * sbuf;
			int lat = (totalbytes * (1000 / 4)) / sr;
			str.Format("Latency: %dms", lat);
			m_latency.SetWindowText(str);
		}

		BOOL CDSoundConfig::OnInitDialog() 
		{
			CString str;
			CDialog::OnInitDialog();
			// DirectSound devices
			DirectSoundEnumerate(EnumDSoundDevices, &m_deviceComboBox);
			if (m_deviceIndex >= m_deviceComboBox.GetCount())
			{
				m_deviceIndex = 0;
			}
			m_deviceComboBox.SetCurSel(m_deviceIndex);
			// Sample rate
			str.Format("%d", m_sampleRate);
			int i = m_sampleRateCombo.SelectString(-1, str);
			if (i == CB_ERR)
			{
				i = m_sampleRateCombo.SelectString(-1, "44100");
			}
			// Check boxes
			m_ditherCheck.SetCheck(m_dither ? 1 : 0);
			m_exclusiveCheck.SetCheck(m_exclusive ? 1 : 0);

			str.Format("%d", m_numBuffers);
			m_numBuffersEdit.SetWindowText(str);
			m_numBuffersSpin.SetRange(MIN_NUMBUF, MAX_NUMBUF);

			str.Format("%d", m_bufferSize);
			m_bufferSizeEdit.SetWindowText(str);
			m_bufferSizeSpin.SetRange32(MIN_BUFSIZE, MAX_BUFSIZE);

			UDACCEL acc;
			acc.nSec = 0;
			acc.nInc = 512;
			m_bufferSizeSpin.SetAccel(1, &acc);

			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CDSoundConfig::OnOK() 
		{

			if (m_deviceComboBox.GetCount() > 0)
			{
				CString str;
				m_sampleRateCombo.GetWindowText(str);
				m_sampleRate = atoi(str);

				m_pDeviceGuid = (LPCGUID)m_deviceComboBox.GetItemData(m_deviceComboBox.GetCurSel());
				if (m_pDeviceGuid != NULL)
				{
					memcpy(&m_deviceGuid, m_pDeviceGuid, sizeof(GUID));
					CDialog::OnOK();
					return;
				}
			}
			CDialog::OnCancel();
		}

		void CDSoundConfig::OnSelendokSamplerate() 
		{
			if (!IsWindow(m_sampleRateCombo.GetSafeHwnd()))
			{
				return;
			}
			RecalcLatency();
		}

		void CDSoundConfig::OnChangeBufnumEdit() 
		{
			if (!IsWindow(m_numBuffersEdit.GetSafeHwnd()))
			{
				return;
			}
			RecalcLatency();
		}

		void CDSoundConfig::OnChangeBufsizeEdit() 
		{
			if (!IsWindow(m_bufferSizeEdit.GetSafeHwnd()))
			{
				return;
			}
			RecalcLatency();
		}

		void CDSoundConfig::OnSelchangeDevice() 
		{
		}

		void CDSoundConfig::OnDestroy() 
		{
			for (int i = m_deviceComboBox.GetCount()-1; i >= 0; i--)
			{
				LPGUID pGuid = (LPGUID)m_deviceComboBox.GetItemData(i);
				if (pGuid != NULL)
				{
					delete pGuid;
				}
			}
			CDialog::OnDestroy();
		}

		void CDSoundConfig::OnExclusive() 
		{
			m_exclusive = (m_exclusiveCheck.GetState() & 0x0001);
		}
	}
}
