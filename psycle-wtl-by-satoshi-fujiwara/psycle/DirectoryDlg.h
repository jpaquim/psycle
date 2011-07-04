#if !defined(AFX_DIRECTORYDLG_H__E34FBFE3_98A4_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_DIRECTORYDLG_H__E34FBFE3_98A4_11D4_937A_B3D2C98D5E39__INCLUDED_
/** @file
 *  @brief header file
 *  $Date: 2004/10/07 21:27:21 $
 *  $Revision: 1.3 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DirectoryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDirectoryDlg dialog

class CDirectoryDlg : public CPropertyPageImpl<CDirectoryDlg>
//class CDirectoryDlg : public CDialogImpl<CDirectoryDlg>,public CWinDataExchange<CDirectoryDlg>
{
//	DECLARE_DYNCREATE(CDirectoryDlg)

// Construction
public:

	bool _instPathChanged;
	TCHAR _instPathBuf[MAX_PATH];
	bool _songPathChanged;
	TCHAR _songPathBuf[MAX_PATH];
	bool _pluginPathChanged;
	TCHAR _pluginPathBuf[MAX_PATH];
	bool _vstPathChanged;
	TCHAR _vstPathBuf[MAX_PATH];
	bool _skinPathChanged;
	TCHAR _skinPathBuf[MAX_PATH];
	bool initializingDlg;

	CDirectoryDlg();
	~CDirectoryDlg();

// Dialog Data

	enum { IDD = IDD_DIRECTORIES };
/*
	CContainedWindowT<CEdit>	m_vstEdit;
	CContainedWindowT<CEdit>	m_pluginEdit;
	CContainedWindowT<CEdit>	m_songEdit;
	CContainedWindowT<CEdit>	m_instEdit;
	CContainedWindowT<CEdit>	m_skinEdit;
*/
	CEdit	m_vstEdit;
	CEdit	m_pluginEdit;
	CEdit	m_songEdit;
	CEdit	m_instEdit;
	CEdit	m_skinEdit;

// Overrides
/*
	BEGIN_DDX_MAP(CDirectoryDlg)

		DDX_CONTROL(IDC_VSTEDIT, m_vstEdit)
		DDX_CONTROL(IDC_PLUGINEDIT, m_pluginEdit)
		DDX_CONTROL(IDC_SONGEDIT, m_songEdit)
		DDX_CONTROL(IDC_INSTEDIT, m_instEdit)
		DDX_CONTROL(IDC_SKINEDIT, m_skinEdit)

	END_DDX_MAP()
*/	
	BEGIN_MSG_MAP(CDirectoryDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_HANDLER(IDC_BROWSESONG, BN_CLICKED, OnBnClickedBrowsesong)
		COMMAND_HANDLER(IDC_BROWSEINST, BN_CLICKED, OnBnClickedBrowseinst)
		COMMAND_HANDLER(IDC_BROWSEPLUGIN, BN_CLICKED, OnBnClickedBrowseplugin)
		COMMAND_HANDLER(IDC_BROWSEVST, BN_CLICKED, OnBnClickedBrowsevst)
		COMMAND_HANDLER(IDC_BROWSESKIN, BN_CLICKED, OnBnClickedBrowseskin)

		COMMAND_HANDLER(IDC_PLUGINEDIT, EN_CHANGE, OnEnChangePluginedit)
		COMMAND_HANDLER(IDC_SONGEDIT, EN_CHANGE, OnEnChangeSongedit)
		COMMAND_HANDLER(IDC_INSTEDIT, EN_CHANGE, OnEnChangeInstedit)
		COMMAND_HANDLER(IDC_VSTEDIT, EN_CHANGE, OnEnChangeVstedit)
		COMMAND_HANDLER(IDC_SKINEDIT, EN_CHANGE, OnEnChangeSkinedit)
	END_MSG_MAP()

protected:

	const bool BrowseForFolder(TCHAR * const rpath);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeSongedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowsesong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeInstedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowseinst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangePluginedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowseplugin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowsevst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBrowseskin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeVstedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeSkinedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTORYDLG_H__E34FBFE3_98A4_11D4_937A_B3D2C98D5E39__INCLUDED_)
