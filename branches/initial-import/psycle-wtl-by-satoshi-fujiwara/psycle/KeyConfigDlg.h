#if !defined(AFX_KEYCONFIGDLG_H__14EA3837_809C_4274_9DE0_B3FF909E898A__INCLUDED_)
#define AFX_KEYCONFIGDLG_H__14EA3837_809C_4274_9DE0_B3FF909E898A__INCLUDED_
/** @file
 *  @brief header file
 *  $Date: 2004/10/07 21:27:21 $
 *  $Revision: 1.2 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyConfigDlg.h : header file
//

#include "InputHandler.h"

/////////////////////////////////////////////////////////////////////////////
// CKeyConfigDlg dialog

//class CKeyConfigDlg : public CDialogImpl<CKeyConfigDlg>
class CKeyConfigDlg : public CPropertyPageImpl<CKeyConfigDlg>
{
//	DECLARE_DYNCREATE(CKeyConfigDlg)

// Construction
public:
	void FillCmdList();
	void DoCommandList();
	CKeyConfigDlg();   // standard constructor
	TCHAR _skinPathBuf[MAX_PATH];
	int OnApply();// - returns PSNRET_NOERROR to indicate the apply operation succeeded
	BOOL OnQueryCancel();

// Dialog Data
	enum { IDD = IDD_KEYCONFIG };
	CUpDownCtrl	m_autosave_spin;
	CEdit	m_autosave_mins;
	CButton	m_autosave;
	CButton	m_save_reminders;
	CButton	m_tweak_smooth;
	CButton	m_record_unarmed;
	CButton	m_show_info;
	CButton	m_cmdShiftArrows;
	CButton	m_cmdFT2Del;
	CButton	m_cmdNewHomeBehaviour;
	CButton	m_cmdCtrlPlay;
	CListBox	m_lstCmds;
	CHotKeyCtrl	m_hotkey0;
	CButton	m_wrap;
	CButton	m_centercursor;
	CButton	m_cursordown;
	CEdit	m_numlines;
	CUpDownCtrl	m_spinlines;
	CStatic m_textlines;
// Implementation
protected:
	BOOL bInit;
	long m_prvIdx;
	void UpdateHotKey();	
	void SaveHotKey(long idx,WORD & key,WORD & mods);
	void FindKey(long idx,WORD & key,WORD & mods);
	CmdDef FindCmd(long idx);
public:
	BEGIN_MSG_MAP(CKeyConfigDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_NONE, BN_CLICKED, OnBnClickedNone)
		COMMAND_HANDLER(IDC_DEFAULTS3, BN_CLICKED, OnBnClickedDefaults3)
		COMMAND_HANDLER(IDC_DEFAULTS, BN_CLICKED, OnBnClickedDefaults)
		COMMAND_HANDLER(IDC_IMPORTREG, BN_CLICKED, OnBnClickedImportreg)
		COMMAND_HANDLER(IDC_EXPORTREG, BN_CLICKED, OnBnClickedExportreg)
//		COMMAND_HANDLER(IDC_TEXT_DEFLINES, STN_CLICKED, OnStnClickedTextDeflines)
		COMMAND_HANDLER(IDC_EDIT_DEFLINES, EN_CHANGE, OnEnChangeEditDeflines)
		COMMAND_HANDLER(IDC_CMDLIST, LBN_SELCHANGE, OnLbnSelchangeCmdlist)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedNone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDefaults3(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDefaults(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedImportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedExportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//	LRESULT OnStnClickedTextDeflines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditDeflines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLbnSelchangeCmdlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYCONFIGDLG_H__14EA3837_809C_4274_9DE0_B3FF909E898A__INCLUDED_)
