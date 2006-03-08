// ASIOConfig.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "ASIOConfig.h"

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
// CASIOConfig dialog

CASIOConfig::CASIOConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CASIOConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CASIOConfig)
	m_bufferSize = 1024;
	m_driverIndex = -1;
	//}}AFX_DATA_INIT
}


void CASIOConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CASIOConfig)
	DDX_Control(pDX, IDC_ASIO_DRIVER, m_driverComboBox);
	DDX_Control(pDX, IDC_ASIO_LATENCY, m_latency);
	DDX_Control(pDX, IDC_ASIO_SAMPLERATE_COMBO, m_sampleRateCombo);
	DDX_CBIndex(pDX, IDC_ASIO_DRIVER, m_driverIndex);
	DDX_Control(pDX, IDC_ASIO_BUFFERSIZE_COMBO, m_bufferSizeCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CASIOConfig, CDialog)
	//{{AFX_MSG_MAP(CASIOConfig)
	ON_CBN_SELENDOK(IDC_ASIO_SAMPLERATE_COMBO, OnSelendokSamplerate)
	ON_CBN_SELENDOK(IDC_ASIO_BUFFERSIZE_COMBO, OnSelendokBuffersize)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CONTROL_PANEL, OnControlPanel)
	ON_CBN_SELCHANGE(IDC_ASIO_DRIVER, OnSelchangeAsioDriver)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CASIOConfig message handlers


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
	//
	str.Format("%d", m_sampleRate);
	int i = m_sampleRateCombo.SelectString(-1, str);
	if (i == CB_ERR)
	{
		i = m_sampleRateCombo.SelectString(-1, "44100");
	}

	// Check boxes
	//
	// recalc the buffers combo
	FillBufferBox();

	return TRUE;  // return TRUE unless you set the focus to a control
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


