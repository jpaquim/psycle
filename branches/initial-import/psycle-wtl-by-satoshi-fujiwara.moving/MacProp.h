#if !defined(AFX_MACPROP_H__C1C0B5E1_EAC7_11D3_8913_B46842196663__INCLUDED_)
#define AFX_MACPROP_H__C1C0B5E1_EAC7_11D3_8913_B46842196663__INCLUDED_

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MacProp.h : header file
//

#include "Machine.h"
#include "PsycleWTLView.h"

class Song;

/////////////////////////////////////////////////////////////////////////////
// CMacProp dialog

class CMacProp : public CDialogImpl<CMacProp>
{
// Construction
public:
	CPsycleWTLView *m_view;
	CMacProp();   // standard constructor
	Machine *pMachine;
	Song* pSong;
	int thisMac;
	TCHAR txt[32];
	bool deleted;
	enum { IDD = IDD_MACPROP };
	CButton	m_soloCheck;
	CButton	m_bypassCheck;
	CButton	m_muteCheck;
	CEdit	m_macname;
	
	BEGIN_MSG_MAP(CMacProp)
		COMMAND_HANDLER(IDC_EDIT1, EN_CHANGE, OnEnChangeEdit1)
		COMMAND_HANDLER(IDC_MUTE, BN_CLICKED, OnBnClickedMute)
		COMMAND_HANDLER(IDC_SOLO, BN_CLICKED, OnBnClickedSolo)
		COMMAND_HANDLER(IDC_BYPASS, BN_CLICKED, OnBnClickedBypass)
		COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
		COMMAND_HANDLER(IDC_CLONE, BN_CLICKED, OnBnClickedClone)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnEnChangeEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMute(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSolo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBypass(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedClone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACPROP_H__C1C0B5E1_EAC7_11D3_8913_B46842196663__INCLUDED_)
