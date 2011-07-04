#if !defined(AFX_GREETDIALOG_H__9EABC421_1EC3_11D4_8913_F2F1E1B2CE72__INCLUDED_)
#define AFX_GREETDIALOG_H__9EABC421_1EC3_11D4_8913_F2F1E1B2CE72__INCLUDED_

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GreetDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGreetDialog dialog

class CGreetDialog : public CDialogImpl<CGreetDialog>
{
// Construction
public:
	CGreetDialog();   // standard constructor

// Dialog Data
	enum { IDD = IDD_GREETS };
	CListBox	m_greetz;


// Overrides
	BEGIN_MSG_MAP(CGreetDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
	END_MSG_MAP()
// Implementation
protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GREETDIALOG_H__9EABC421_1EC3_11D4_8913_F2F1E1B2CE72__INCLUDED_)
