#if !defined(XM_SAMPLER_GENERAL_UI)
#define XM_SAMPLER_GENERAL_UI
/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearTracker.h : header file
//

#include "constants.h"
#pragma unmanaged

class CPsycleWTLView;
namespace SF {
/////////////////////////////////////////////////////////////////////////////
// XMSamplerUIGeneral dialog
class XMSampler;
class XMSamplerUIGeneral : public CPropertyPageImpl<XMSamplerUIGeneral>
//class XMSamplerUIGeneral : public CDialogImpl<XMSamplerUIGeneral>
{
// Construction
public:
	XMSamplerUIGeneral();   // standard constructor
	XMSamplerUIGeneral(CPsycleWTLView* parent) {m_pParent = parent;}
	XMSampler * const pMachine() { return _pMachine;};
	void pMachine(XMSampler * const p) { _pMachine = p;};

	//	BOOL Create();
	void OnCancel();

	enum { IDD = IDD_XM_GENERAL };



// Implementation
private:

	XMSampler* _pMachine;
	CComboBox	m_interpol;
	CTrackBarCtrl	m_polyslider;
	CStatic	m_polylabel;

	CEdit m_Tempo;
	CEdit m_Speed;
	bool m_bInitialize;
//	CEdit m_GlobalVolume;

protected:
	CPsycleWTLView* m_pParent;
/*
	// Generated message map functions
	//{{AFX_MSG(XMSamplerUIGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	BEGIN_MESSAGE_MAP(XMSamplerUIGeneral, CDialog)
		//{{AFX_MSG_MAP(XMSamplerUIGeneral)
		ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACKSLIDER2, OnCustomdrawTrackslider2)
		ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
*/
public:
	BEGIN_MSG_MAP(XMSamplerUIGeneral)
		NOTIFY_HANDLER(IDC_TRACKSLIDER2, NM_CUSTOMDRAW, OnNMCustomdrawTrackslider2)
		COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeCombo1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_EDIT_SPEED, EN_CHANGE, OnEnChangeEditSpeed)
		COMMAND_HANDLER(IDC_EDIT_TEMPO, EN_CHANGE, OnEnChangeEditTempo)
		COMMAND_HANDLER(IDC_EDIT_TEMPO, EN_CHANGE, OnEnChangeEditTempo)

	END_MSG_MAP()

	LRESULT OnNMCustomdrawTrackslider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	virtual void OnFinalMessage(HWND hWnd);
//	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditSpeed(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditTempo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

}
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_)
