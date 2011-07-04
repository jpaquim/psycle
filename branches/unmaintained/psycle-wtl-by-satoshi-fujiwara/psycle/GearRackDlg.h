#if !defined(AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_)
#define AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearRackDlg.h : header file
//


#include "PsycleWTLView.h"

/////////////////////////////////////////////////////////////////////////////
// CGearRackDlg dialog

class CGearRackDlg : public CDialogImpl<CGearRackDlg>
{
// Construction
public:
	void ExchangeIns(int one,int two);
	void ExchangeMacs(int one,int two);
	CGearRackDlg();   // standard constructor
	CGearRackDlg(CPsycleWTLView* pParent,CMainFrame* pPMain)
	{
		m_pParent = pParent;
		pParentMain = pPMain;

	};
	CPsycleWTLView* m_pParent;
	CMainFrame* pParentMain;
	void RedrawList();
	void OnCancel();
	static int DisplayMode;

	static const DWORD IDD = IDD_GEAR_RACK;
	
	CButton	m_props;
	CButton	m_radio_ins;
	CButton	m_radio_gen;
	CButton	m_radio_efx;
	CButton	m_text;
	CListBox	m_list;

/*
	virtual BOOL PreTranslateMessage(MSG* pMsg);
*/
	BEGIN_MSG_MAP(CGearRackDlg)
		COMMAND_HANDLER(IDC_CREATE, BN_CLICKED, OnBnClickedCreate)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_DELETE, BN_CLICKED, OnBnClickedDelete)
		COMMAND_HANDLER(IDC_PARAMETERS, BN_CLICKED, OnBnClickedParameters)
		COMMAND_HANDLER(IDC_PROPERTIES, BN_CLICKED, OnBnClickedProperties)
		COMMAND_HANDLER(IDC_EXCHANGE, BN_CLICKED, OnBnClickedExchange)
		COMMAND_HANDLER(IDC_CLONEMACHINE, BN_CLICKED, OnBnClickedClonemachine)
		COMMAND_HANDLER(IDC_GEARLIST, LBN_SELCHANGE, OnLbnSelchangeGearlist)
		COMMAND_HANDLER(IDC_RADIO_GEN, BN_CLICKED, OnBnClickedRadioGen)
		COMMAND_HANDLER(IDC_RADIO_EFX, BN_CLICKED, OnBnClickedRadioEfx)
		COMMAND_HANDLER(IDC_RADIO_INS, BN_CLICKED, OnBnClickedRadioIns)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
private:
	LRESULT OnBnClickedCreate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedExchange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedClonemachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLbnSelchangeGearlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioGen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioEfx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioIns(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	virtual void OnFinalMessage(HWND hWnd);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_)
