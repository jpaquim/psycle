/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */
// ASIOConfig.cpp : implementation file
//

#include "stdafx.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//#include "Psycle2.h"
#include "resource.h"
#include "ASIOConfig.h"
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#define MIN_NUMBUF 1
#define MAX_NUMBUF 16

#define MIN_BUFSIZE 256
#define MAX_BUFSIZE 32768

// some external references


/////////////////////////////////////////////////////////////////////////////
// CASIOConfig dialog

CASIOConfig::CASIOConfig()
{
	m_bufferSize = 1024;
	m_driverIndex = -1;
}


/////////////////////////////////////////////////////////////////////////////
// CASIOConfig message handlers


void CASIOConfig::RecalcLatency()
{
	ATL::CString str;
	m_bufferSizeCombo.GetWindowText(str);
	int sbuf = _tstoi(str);
	m_sampleRateCombo.GetWindowText(str);
	int sr = _tstoi(str);
	int lat = (sbuf * (1000)) / sr;
	str.Format(SF::CResourceString(IDS_MSG0069), lat);
	m_latency.SetWindowText(str);
}

LRESULT CASIOConfig::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExecuteDlgInit(IDD);
	CString str;
	m_driverComboBox.Attach(GetDlgItem(IDC_ASIO_DRIVER));
	m_latency.Attach(GetDlgItem(IDC_ASIO_LATENCY));
	m_sampleRateCombo.Attach(GetDlgItem(IDC_ASIO_SAMPLERATE_COMBO));
	m_bufferSizeCombo.Attach(GetDlgItem(IDC_ASIO_BUFFERSIZE_COMBO));

//	CDialogImpl<CASIOConfig>:OnInitDialog();

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
	str.Format(_T("%d"), m_sampleRate);
	i = m_sampleRateCombo.SelectString(-1, str);
	if (i == CB_ERR)
	{
		i = m_sampleRateCombo.SelectString(-1, _T("44100"));
	}

	// Check boxes
	//
	// recalc the buffers combo
	DoDataExchange(FALSE);
	FillBufferBox();
	return 0;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CASIOConfig::FillBufferBox()
{
	// hmm we had better recalc our buffer options

	int prefindex = 0;
	TCHAR buf[8];
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
			_stprintf(buf,_T("%d"),i);
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
			_stprintf(buf,_T("%d"),i);
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


LRESULT CASIOConfig::OnBnClickedOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_driverComboBox.GetCount() > 0)
	{

		ATL::CString str;
		m_sampleRateCombo.GetWindowText(str);
		m_sampleRate = _tstoi(str);

		m_bufferSizeCombo.GetWindowText(str);
		m_bufferSize = _tstoi(str);
	}
	DoDataExchange(TRUE);
	EndDialog(wID);
	return 0;
}

LRESULT CASIOConfig::OnCbnSelchangeAsioDriver(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ATL::CString str;
	m_bufferSizeCombo.GetWindowText(str);

	pASIO->currentSamples[pASIO->driverindex[m_driverIndex]] = _tstoi(str);
	m_driverIndex = m_driverComboBox.GetCurSel();

	FillBufferBox();

	return 0;
}

LRESULT CASIOConfig::OnBnClickedControlPanel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_driverIndex = m_driverComboBox.GetCurSel();
	pASIO->ControlPanel(m_driverIndex);
	return 0;
}

LRESULT CASIOConfig::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	EndDialog(wID);
	return 0;
}

LRESULT CASIOConfig::OnCbnSelchangeAsioSamplerateCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!::IsWindow((HWND)m_sampleRateCombo))
	{
		return 0;
	}
	RecalcLatency();

	return 0;
}

LRESULT CASIOConfig::OnCbnSelchangeAsioBuffersizeCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!::IsWindow((HWND)m_bufferSizeCombo))
	{
		return 0;
	}
	RecalcLatency();

	return 0;
}
