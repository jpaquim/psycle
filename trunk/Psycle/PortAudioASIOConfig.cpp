// PortAudioASIOConfig.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "PortAudioASIOConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_NUMBUF 1
#define MAX_NUMBUF 16

#define MIN_BUFSIZE 256
#define MAX_BUFSIZE 32768

// some external references

/////////////////////////////////////////////////////////////////////////////
// CPortAudioASIOConfig dialog

CPortAudioASIOConfig::CPortAudioASIOConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CPortAudioASIOConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPortAudioASIOConfig)
	m_numBuffers = 0;
	m_bufferSize = 0;
	m_deviceIndex = -1;
	m_dither = FALSE;
	//}}AFX_DATA_INIT
}


void CPortAudioASIOConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortAudioASIOConfig)
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_LATENCY, m_latency);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_BUFNUM_SPIN, m_numBuffersSpin);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_BUFSIZE_SPIN, m_bufferSizeSpin);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_DITHER, m_ditherCheck);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_BUFSIZE_EDIT, m_bufferSizeEdit);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_BUFNUM_EDIT, m_numBuffersEdit);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_SAMPLERATE_COMBO, m_sampleRateCombo);
	DDX_Control(pDX, IDC_PORTAUDIO_ASIO_DEVICE, m_deviceComboBox);
	DDX_Text(pDX, IDC_PORTAUDIO_ASIO_BUFNUM_EDIT, m_numBuffers);
	DDV_MinMaxInt(pDX, m_numBuffers, 1, 16);
	DDX_Text(pDX, IDC_PORTAUDIO_ASIO_BUFSIZE_EDIT, m_bufferSize);
	DDV_MinMaxInt(pDX, m_bufferSize, 256, 32768);
	DDX_CBIndex(pDX, IDC_PORTAUDIO_ASIO_DEVICE, m_deviceIndex);
	DDX_Check(pDX, IDC_PORTAUDIO_ASIO_DITHER, m_dither);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortAudioASIOConfig, CDialog)
	//{{AFX_MSG_MAP(CPortAudioASIOConfig)
	ON_CBN_SELENDOK(IDC_PORTAUDIO_ASIO_SAMPLERATE_COMBO, OnSelendokSamplerate)
	ON_EN_CHANGE(IDC_PORTAUDIO_ASIO_BUFNUM_EDIT, OnChangeBufnumEdit)
	ON_EN_CHANGE(IDC_PORTAUDIO_ASIO_BUFSIZE_EDIT, OnChangeBufsizeEdit)
	ON_CBN_SELCHANGE(IDC_PORTAUDIO_ASIO_DEVICE, OnSelchangeDevice)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_DSOUND_DEVICE, OnCbnSelchangeDsoundDevice)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortAudioASIOConfig message handlers


void CPortAudioASIOConfig::RecalcLatency()
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

BOOL CPortAudioASIOConfig::OnInitDialog() 
{
	CString str;
	CDialog::OnInitDialog();
	
	
	// ASIO devices
	//
	int n = Pa_CountDevices();
	
	const PaDeviceInfo	*info;

	for (int a = 0; a < n; a++)
	{
		info = 	Pa_GetDeviceInfo(a);
		m_deviceComboBox.AddString(info->name);
	}
	
	if (m_deviceIndex >= n)
			m_deviceIndex = 0;

	m_deviceComboBox.SetCurSel(m_deviceIndex);

	// Sample rate
	//
	str.Format("%d", m_sampleRate);
	int i = m_sampleRateCombo.SelectString(-1, str);
	if (i == CB_ERR)
	{
		i = m_sampleRateCombo.SelectString(-1, "44100");
	}

	// Check boxes
	//
	m_ditherCheck.SetCheck(m_dither ? 1 : 0);
	
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

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPortAudioASIOConfig::OnOK() 
{
	CString str;
	m_sampleRateCombo.GetWindowText(str);
	m_sampleRate = atoi(str);

	m_pDeviceGuid = (LPCGUID)m_deviceComboBox.GetItemData(m_deviceComboBox.GetCurSel());
	if (m_pDeviceGuid != NULL)
	{
		memcpy(&m_deviceGuid, m_pDeviceGuid, sizeof(GUID));
	}
	CDialog::OnOK();
}

void CPortAudioASIOConfig::OnSelendokSamplerate() 
{
	if (!IsWindow(m_sampleRateCombo.GetSafeHwnd()))
	{
		return;
	}
	RecalcLatency();
}

void CPortAudioASIOConfig::OnChangeBufnumEdit() 
{
	if (!IsWindow(m_numBuffersEdit.GetSafeHwnd()))
	{
		return;
	}
	RecalcLatency();
}

void CPortAudioASIOConfig::OnChangeBufsizeEdit() 
{
	if (!IsWindow(m_bufferSizeEdit.GetSafeHwnd()))
	{
		return;
	}
	RecalcLatency();
}

void CPortAudioASIOConfig::OnSelchangeDevice() 
{
	
	
}

void CPortAudioASIOConfig::OnDestroy() 
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


void CPortAudioASIOConfig::OnCbnSelchangeDsoundDevice()
{
	// TODO: Add your control notification handler code here
}

void CPortAudioASIOConfig::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
