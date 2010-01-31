#if !defined(AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_)
#define AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearTracker.h : header file
//


#include "Sampler.h"
#include "constants.h"

class CPsycleWTLView;

/////////////////////////////////////////////////////////////////////////////
// CGearTracker dialog

class CGearTracker : public CDialogImpl<CGearTracker>
{
// Construction
public:
	CGearTracker();   // standard constructor
	CGearTracker(CPsycleWTLView* parent) {m_pParent = parent;}
	Sampler* _pMachine;
//	BOOL Create();
	void OnCancel();

	enum { IDD = IDD_GEAR_TRACKER };
	CComboBox	m_interpol;
	CTrackBarCtrl	m_polyslider;
	CStatic	m_polylabel;



// Implementation
protected:
	CPsycleWTLView* m_pParent;
/*
	// Generated message map functions
	//{{AFX_MSG(CGearTracker)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	BEGIN_MESSAGE_MAP(CGearTracker, CDialog)
		//{{AFX_MSG_MAP(CGearTracker)
		ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACKSLIDER2, OnCustomdrawTrackslider2)
		ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
*/
public:
	BEGIN_MSG_MAP(CGearTracker)
		NOTIFY_HANDLER(IDC_TRACKSLIDER2, NM_CUSTOMDRAW, OnNMCustomdrawTrackslider2)
		COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeCombo1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	LRESULT OnNMCustomdrawTrackslider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void OnFinalMessage(HWND hWnd);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_)