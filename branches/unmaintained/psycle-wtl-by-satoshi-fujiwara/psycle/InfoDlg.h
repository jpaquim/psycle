#if !defined(AFX_INFODLG_H__078666A2_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
#define AFX_INFODLG_H__078666A2_E9D9_11D3_8913_950D7D66B163__INCLUDED_

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog

class CInfoDlg : public CDialogImpl<CInfoDlg>
{
// Construction
public:
	CInfoDlg();   // standard constructor
	
	Song* _pSong;
	void InitTimer();
	void UpdateInfo();
	int itemcount;
	
// Dialog Data
	enum { IDD = IDD_INFO };
	CStatic	m_mem_virtual;
	CStatic	m_mem_pagefile;
	CStatic	m_mem_phy;
	CStatic	m_mem_reso;
	CStatic	m_cpurout;
	CStatic	m_machscpu;
	CStatic	m_processor_label;
	CStatic	m_cpuidlelabel;
	CListViewCtrl	m_machlist;

	BEGIN_MSG_MAP(CInfoDlg)

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM nIDEvent, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__078666A2_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
