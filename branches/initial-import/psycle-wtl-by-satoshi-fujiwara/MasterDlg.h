#if !defined(AFX_MASTERDLG_H__F83FC440_4A98_11D4_8913_E55936BE8F41__INCLUDED_)
#define AFX_MASTERDLG_H__F83FC440_4A98_11D4_8913_E55936BE8F41__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/**@file MasterDlg.h : header file
 * @brief Master Dialog
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 *        マスターダイアログ */
#include "Machine.h"
#include "constants.h"

class CPsycleWTLView;
/** @class CMasterDlg Class
 *	@brief Master Volume Dialog
 *	@brief マスターボリュームダイアログ */
class CMasterDlg : public CDialogImpl<CMasterDlg>
{
// Construction
public:
	CMasterDlg(CPsycleWTLView* pParent);///< standard constructor
//	BOOL Create();
	/**  Paint Volume */ 
	void PaintNumbers(int val /**< volume */, int x /**< xpos */, int y /**< ypos*/); 
	/**  Paint Volume DC*/ 
	void PaintNumbersDC(CDC* dc /**< Device Context*/,CDC* memDC /**< Memory DC*/,int val ,int x ,int y);
	void PaintNames(const TCHAR* name,int x,int y);
	CBitmap m_numbers;
	CFont namesFont;
	Master* _pMachine;
	TCHAR macname[MAX_CONNECTIONS][32];
// Dialog Data
	enum { IDD = IDD_MASTERDLG };
	
	CStatic& Masterpeak(void){return m_masterpeak;};
	
	CTrackBarCtrl	m_slidermaster;
	CTrackBarCtrl	m_sliderm9;
	CTrackBarCtrl	m_sliderm8;
	CTrackBarCtrl	m_sliderm7;
	CTrackBarCtrl	m_sliderm6;
	CTrackBarCtrl	m_sliderm5;
	CTrackBarCtrl	m_sliderm4;
	CTrackBarCtrl	m_sliderm3;
	CTrackBarCtrl	m_sliderm2;
	CTrackBarCtrl	m_sliderm12;
	CTrackBarCtrl	m_sliderm11;
	CTrackBarCtrl	m_sliderm10;
	CTrackBarCtrl	m_sliderm1;
	CStatic	m_mixerview;
	CButton	m_autodec;

// Implementation
protected:
	CPsycleWTLView* m_pParent;
public:
	BEGIN_MSG_MAP(CMasterDlg)
		if ((uMsg == WM_KEYDOWN) || (uMsg == WM_KEYUP))
		{
			::SendMessage((HWND)(*m_pParent),uMsg,wParam,lParam);
		}
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_HANDLER(IDC_AUTODEC, BN_CLICKED, OnBnClickedAutodec)
		NOTIFY_HANDLER(IDC_SLIDERMASTER, NM_CUSTOMDRAW, OnNMCustomdrawSlidermaster)
		NOTIFY_HANDLER(IDC_SLIDERM1, NM_CUSTOMDRAW, OnNMCustomdrawSliderm1)
		NOTIFY_HANDLER(IDC_SLIDERM2, NM_CUSTOMDRAW, OnNMCustomdrawSliderm2)
		NOTIFY_HANDLER(IDC_SLIDERM3, NM_CUSTOMDRAW, OnNMCustomdrawSliderm3)
		NOTIFY_HANDLER(IDC_SLIDERM4, NM_CUSTOMDRAW, OnNMCustomdrawSliderm4)
		NOTIFY_HANDLER(IDC_SLIDERM5, NM_CUSTOMDRAW, OnNMCustomdrawSliderm5)
		NOTIFY_HANDLER(IDC_SLIDERM6, NM_CUSTOMDRAW, OnNMCustomdrawSliderm6)
		NOTIFY_HANDLER(IDC_SLIDERM7, NM_CUSTOMDRAW, OnNMCustomdrawSliderm7)
		NOTIFY_HANDLER(IDC_SLIDERM8, NM_CUSTOMDRAW, OnNMCustomdrawSliderm8)
		NOTIFY_HANDLER(IDC_SLIDERM9, NM_CUSTOMDRAW, OnNMCustomdrawSliderm9)
		NOTIFY_HANDLER(IDC_SLIDERM10, NM_CUSTOMDRAW, OnNMCustomdrawSliderm10)
		NOTIFY_HANDLER(IDC_SLIDERM11, NM_CUSTOMDRAW, OnNMCustomdrawSliderm11)
		NOTIFY_HANDLER(IDC_SLIDERM12, NM_CUSTOMDRAW, OnNMCustomdrawSliderm12)
//		MESSAGE_HANDLER(WM_CANCELMODE, OnCancelMode)
//		MESSAGE_HANDLER(WM_CANCELMODE, OnCancelMode)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()
	virtual void OnFinalMessage(HWND hWnd);
	void Cancel(){
		m_pParent->MasterMachineDialog = NULL;
		DestroyWindow();
	}

private:
	CStatic	m_masterpeak;

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedAutodec(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSlidermaster(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm3(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm4(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm5(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm6(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm7(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm8(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm9(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm10(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm11(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderm12(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASTERDLG_H__F83FC440_4A98_11D4_8913_E55936BE8F41__INCLUDED_)
