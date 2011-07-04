#if !defined(AFX_SONGPDLG_H__FCA5A9E3_260E_11D4_8913_AD975B428477__INCLUDED_)
#define AFX_SONGPDLG_H__FCA5A9E3_260E_11D4_8913_AD975B428477__INCLUDED_
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SongpDlg.h : header file



class Song;

/////////////////////////////////////////////////////////////////////////////
// CSongpDlg dialog

class CSongpDlg : public CDialogImpl<CSongpDlg>
{
// Construction
public:
	CSongpDlg();   // standard constructor
	Song* _pSong;

	enum { IDD = IDD_SONGPROP };

	CEdit	m_songcomments;
	CEdit	m_songcredits;
	CEdit	m_songtitle;
protected:

public:
	BEGIN_MSG_MAP(CSongpDlg)
		COMMAND_HANDLER(IDC_EDIT1, EN_CHANGE, OnEnChangeEdit1)
		COMMAND_HANDLER(IDC_EDIT3, EN_CHANGE, OnEnChangeEdit3)
		COMMAND_HANDLER(IDC_EDIT4, EN_CHANGE, OnEnChangeEdit4)
		COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnEnChangeEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEdit3(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEdit4(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONGPDLG_H__FCA5A9E3_260E_11D4_8913_AD975B428477__INCLUDED_)
