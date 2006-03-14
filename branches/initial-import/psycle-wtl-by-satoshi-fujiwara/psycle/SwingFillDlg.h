#if !defined(AFX_SWINGFILLDLG_H__4108AB3F_F019_484E_8177_E98B6637C612__INCLUDED_)
#define AFX_SWINGFILLDLG_H__4108AB3F_F019_484E_8177_E98B6637C612__INCLUDED_
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SwingFillDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CSwingFillDlg dialog

class CSwingFillDlg : public CDialogImpl<CSwingFillDlg>
{
// Construction
public:
	CSwingFillDlg();   // standard constructor
	BOOL bGo;
	int tempo;
	int width;
	float variance;
	float phase;
	BOOL offset;

	enum { IDD = IDD_SWINGFILL };
	CEdit	m_Tempo;
	CEdit	m_Width;
	CEdit	m_Variance;
	CEdit	m_Phase;
	CButton m_Offset;


protected:

public:
	BEGIN_MSG_MAP(CSwingFillDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
//		COMMAND_HANDLER(IDC_OFFSET, BN_CLICKED, OnBnClickedOffset)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//	LRESULT OnBnClickedOffset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWINGFILLDLG_H__4108AB3F_F019_484E_8177_E98B6637C612__INCLUDED_)
