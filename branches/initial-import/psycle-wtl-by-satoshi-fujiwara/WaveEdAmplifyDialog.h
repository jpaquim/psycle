#if !defined(AFX_WAVEEDAMPLIFYDIALOG_H__6A36C2F3_B9B5_4246_9B1E_5B0395AB70C9__INCLUDED_)
#define AFX_WAVEEDAMPLIFYDIALOG_H__6A36C2F3_B9B5_4246_9B1E_5B0395AB70C9__INCLUDED_


/** @file 
 *  @brief
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define AMP_DIALOG_CANCEL -10000

class CWaveEdAmplifyDialog : public CDialogImpl<CWaveEdAmplifyDialog>
{
// Construction
public:
	CWaveEdAmplifyDialog();   // standard constructor

	static const int IDD = IDD_WAVED_AMPLIFY;
// Implementation

	// Message Map 
	BEGIN_MSG_MAP(CWaveEdAmplifyDialog)
		NOTIFY_HANDLER(IDC_SLIDER3, NM_CUSTOMDRAW, OnNMCustomdrawSlider3)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
	END_MSG_MAP()
private:
	LRESULT OnNMCustomdrawSlider3(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CEdit	m_dbedit;
	CEdit	m_edit;
	CTrackBarCtrl	m_slider;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDAMPLIFYDIALOG_H__6A36C2F3_B9B5_4246_9B1E_5B0395AB70C9__INCLUDED_)
