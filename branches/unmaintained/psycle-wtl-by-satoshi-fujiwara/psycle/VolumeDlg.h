#if !defined(AFX_VOLUMEDLG_H__4F5FAD34_8830_48D7_A9BD_5573B0EF82BA__INCLUDED_)
#define AFX_VOLUMEDLG_H__4F5FAD34_8830_48D7_A9BD_5573B0EF82BA__INCLUDED_
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CVolumeDlg dialog

class CVolumeDlg : public CDialogImpl<CVolumeDlg>
{
// Construction
public:
	CVolumeDlg();   // standard constructor
	float volume;
	int edit_type;

	enum { IDD = IDD_NEW_VOLUME };
	CEdit		m_db;
	CEdit		m_per;

protected:
	void DrawDb();
	void DrawPer();
	bool go;
public:
	BEGIN_MSG_MAP(CVolumeDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_EDIT_DB, EN_CHANGE, OnEnChangeEditDb)
		COMMAND_HANDLER(IDC_EDIT_PER, EN_CHANGE, OnEnChangeEditPer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditDb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditPer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOLUMEDLG_H__4F5FAD34_8830_48D7_A9BD_5573B0EF82BA__INCLUDED_)
