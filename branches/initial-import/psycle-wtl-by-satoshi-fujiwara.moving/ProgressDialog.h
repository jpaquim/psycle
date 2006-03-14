#if !defined(AFX_PROGRESSDIALOG_H__99A71CBC_6372_4C8C_A812_20DB620DDAA2__INCLUDED_)
#define AFX_PROGRESSDIALOG_H__99A71CBC_6372_4C8C_A812_20DB620DDAA2__INCLUDED_


/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog

class CProgressDialog : public CDialogImpl<CProgressDialog>
{
// Construction
public:
	CProgressDialog();   // standard constructor
//	BOOL Create();
	
	BEGIN_MSG_MAP(CProgressDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	void OnCancel();
	
	// Dialog Data
	enum { IDD = IDD_PROGRESS_DIALOG };
	
	CProgressBarCtrl m_Progress;
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
// Implementation
protected:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDIALOG_H__99A71CBC_6372_4C8C_A812_20DB620DDAA2__INCLUDED_)
