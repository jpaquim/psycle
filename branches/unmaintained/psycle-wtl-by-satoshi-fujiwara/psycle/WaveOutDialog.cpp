/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
// ADriverDialog.cpp : implementation file
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

#include <mmsystem.h>
#include ".\waveoutdialog.h"


/////////////////////////////////////////////////////////////////////////////
// CWaveOutDialog dialog


CWaveOutDialog::CWaveOutDialog()
{
	m_BufNum = 2;
	m_BufSize = 512;
	m_Device = -1;
	m_Dither = FALSE;

}


/*
BEGIN_MESSAGE_MAP(CWaveOutDialog, CDialog)
	//{{AFX_MSG_MAP(CWaveOutDialog)
	ON_EN_CHANGE(IDC_CONFIG_BUFNUM, OnChangeConfigBufnum)
	ON_EN_CHANGE(IDC_CONFIG_BUFSIZE, OnChangeConfigBufsize)
	ON_CBN_SELENDOK(IDC_CONFIG_SAMPLERATE, OnSelendokConfigSamplerate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
*/
#define MIN_NBUF		2
#define MAX_NBUF		8
#define MIN_SBUF		512
#define MAX_SBUF		(32768 - 512)

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDialog message handlers


void CWaveOutDialog::RecalcLatency()
{
	CString str;
	m_BufNumEdit.GetWindowText(str);
	int nbuf = _tstoi(str);

	if (nbuf < MIN_NBUF)
	{
		nbuf = MIN_NBUF;
		str.Format(_T("%d"), nbuf);
		m_BufNumEdit.SetWindowText(str);
	}
	else if (nbuf > MAX_NBUF)
	{
		nbuf = MAX_NBUF;
		str.Format(_T("%d"), nbuf);
		m_BufNumEdit.SetWindowText(str);
	}

	m_BufSizeEdit.GetWindowText(str);
	int sbuf = _tstoi(str);

	if (sbuf < MIN_SBUF)
	{
		sbuf = MIN_SBUF;
		str.Format(_T("%d"), sbuf);
		m_BufSizeEdit.SetWindowText(str);
	}
	else if (sbuf > MAX_SBUF)
	{
		sbuf = MAX_SBUF;
		str.Format(_T("%d"), sbuf);
		m_BufSizeEdit.SetWindowText(str);
	}
	
	m_SampleRateBox.GetWindowText(str);
	int sr = _tstoi(str);

	int totalbytes = nbuf * sbuf;

	int lat = (totalbytes * (1000 / 4)) / sr;

	str.Format(SF::CResourceString(IDS_MSG0071), lat);
	m_Latency.SetWindowText(str);
}


LRESULT CWaveOutDialog::OnEnChangeConfigBufnum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	// TODO :  ここにコントロール通知ハンドラ コードを追加してください。
	if (!::IsWindow((HWND)m_BufNumEdit))
		return 0;

	RecalcLatency();
	return 0;
}

LRESULT CWaveOutDialog::OnEnChangeConfigBufsize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。
	if (!::IsWindow((HWND)m_BufSizeEdit))
		return 0;

	RecalcLatency();

	return 0;
}

LRESULT CWaveOutDialog::OnCbnSelendokConfigDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!::IsWindow((HWND)m_SampleRateBox))
		return 0;
	
	RecalcLatency();
	return 0;
}

LRESULT CWaveOutDialog::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString str;
	m_SampleRateBox.GetWindowText(str);
	m_SampleRate = _tstoi(str);
	EndDialog(wID);
	return 0;
}

LRESULT CWaveOutDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExecuteDlgInit(IDD);

		 m_Latency.Attach(GetDlgItem(IDC_CONFIG_LATENCY));
		 m_BufSizeSpin.Attach(GetDlgItem(IDC_CONFIG_BUFSIZE_SPIN));
		 m_BufNumSpin.Attach(GetDlgItem(IDC_CONFIG_BUFNUM_SPIN));
		 m_BufSizeEdit.Attach(GetDlgItem(IDC_CONFIG_BUFSIZE));
		 m_BufNumEdit.Attach(GetDlgItem(IDC_CONFIG_BUFNUM));
		 m_DitherCheck.Attach(GetDlgItem(IDC_CONFIG_DITHER));
		 m_DeviceList.Attach(GetDlgItem(IDC_CONFIG_DEVICE));
		 m_SampleRateBox.Attach(GetDlgItem(IDC_CONFIG_SAMPLERATE));

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
		str.Format(_T("%d"), m_SampleRate);
		
		int i = m_SampleRateBox.SelectString(-1, str);
		if (i == CB_ERR)
			i = m_SampleRateBox.SelectString(-1, _T("44100"));
	}

	// dither

	m_DitherCheck.SetCheck(m_Dither ? 1 : 0);

	// buffers
	
	{
		CString str;
		
		str.Format(_T("%d"), m_BufNum);
		m_BufNumEdit.SetWindowText(str);
		m_BufNumSpin.SetRange(MIN_NBUF, MAX_NBUF);

		str.Format(_T("%d"), m_BufSize);
		m_BufSizeEdit.SetWindowText(str);
		m_BufSizeSpin.SetRange(MIN_SBUF, MAX_SBUF);
	
		UDACCEL acc;
		acc.nSec = 0;
		acc.nInc = 512;
		m_BufSizeSpin.SetAccel(1, &acc);
	}

	RecalcLatency();
	
	DoDataExchange(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
