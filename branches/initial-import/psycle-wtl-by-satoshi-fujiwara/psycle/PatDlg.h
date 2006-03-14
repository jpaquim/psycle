#if !defined(AFX_PATDLG_H__38327E2C_ED0C_11D3_8913_B689BEC79464__INCLUDED_)
#define AFX_PATDLG_H__38327E2C_ED0C_11D3_8913_B689BEC79464__INCLUDED_

/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $ */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CPatDlg dialog

class CPatDlg : public CDialogImpl<CPatDlg>,CWinDataExchange<CPatDlg>
{
// Construction
public:
	CPatDlg();   // standard constructor
	int patLines;
	TCHAR patName[32];

// Dialog Data
	enum { IDD = IDD_PATDLG };
	CButton	m_adaptsizeCheck;
	CEdit	m_numlines;
	CEdit	m_patname;
	CUpDownCtrl	m_spinlines;
	BOOL	m_adaptsize;
	CStatic m_text;

	BEGIN_DDX_MAP(CPatDlg)
		DDX_CHECK(IDC_CHECK1, m_adaptsize)
	END_DDX_MAP()

// Implementation
protected:
	BOOL bInit;
	void OnUpdateNumLines();

public:
	BEGIN_MSG_MAP(CPatDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CHECK1, BN_CLICKED, OnBnClickedCheck1)
//		COMMAND_HANDLER(IDC_EDIT2, EN_CHANGE, OnEnChangeEdit2)
		COMMAND_HANDLER(IDC_EDIT2, EN_UPDATE, OnEnUpdateEdit2)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedCheck1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//	LRESULT OnEnChangeEdit2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnUpdateEdit2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATDLG_H__38327E2C_ED0C_11D3_8913_B689BEC79464__INCLUDED_)
