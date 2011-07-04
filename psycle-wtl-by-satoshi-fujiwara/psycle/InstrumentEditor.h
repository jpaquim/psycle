#if !defined(AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_)
#define AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InstrumentEditor.h : header file
//

#include "resource.h"

class Song;
class CMainFrame;
/////////////////////////////////////////////////////////////////////////////
// CInstrumentEditor dialog

class CInstrumentEditor : public CDialogImpl<CInstrumentEditor>
{
// Construction
public:
	void UpdateNoteLabel();
	void Validate();

	CInstrumentEditor();   // standard constructor
	void WaveUpdate();
	void UpdateCombo();
	Song* _pSong;
	bool cando;
	bool initializingDialog;

	CMainFrame* pParentMain;


// Dialog Data
	static const int IDD = IDD_INSTRUMENT;
// Implementation
	BEGIN_MSG_MAP(CInstrumentEditor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BUTTON12, BN_CLICKED, OnBnClickedButton12)
		COMMAND_HANDLER(IDC_BUTTON13, BN_CLICKED, OnBnClickedButton13)
		COMMAND_HANDLER(IDC_BUTTON7, BN_CLICKED, OnBnClickedButton7)
		NOTIFY_HANDLER(IDC_PANSLIDER, NM_CUSTOMDRAW, OnNMCustomdrawPanslider)
		COMMAND_HANDLER(IDC_RPAN, BN_CLICKED, OnBnClickedRpan)
		COMMAND_HANDLER(IDC_RCUT, BN_CLICKED, OnBnClickedRcut)
		COMMAND_HANDLER(IDC_RRES, BN_CLICKED, OnBnClickedRres)
		COMMAND_HANDLER(IDC_CHECK4, BN_CLICKED, OnBnClickedCheck4)
		NOTIFY_HANDLER(IDC_SLIDER1, NM_CUSTOMDRAW, OnNMCustomdrawSlider1)
		NOTIFY_HANDLER(IDC_SLIDER2, NM_CUSTOMDRAW, OnNMCustomdrawSlider2)
		COMMAND_HANDLER(IDC_INS_DECOCTAVE, BN_CLICKED, OnBnClickedInsDecoctave)
		COMMAND_HANDLER(IDC_INS_DECNOTE, BN_CLICKED, OnBnClickedInsDecnote)
		COMMAND_HANDLER(IDC_INS_INCNOTE, BN_CLICKED, OnBnClickedInsIncnote)
		COMMAND_HANDLER(IDC_INS_INCOCTAVE, BN_CLICKED, OnBnClickedInsIncoctave)
		COMMAND_HANDLER(IDC_LOOPOFF, BN_CLICKED, OnBnClickedLoopoff)
		COMMAND_HANDLER(IDC_LOOPFORWARD, BN_CLICKED, OnBnClickedLoopforward)
		COMMAND_HANDLER(IDC_BUTTON8, BN_CLICKED, OnBnClickedButton8)
		COMMAND_HANDLER(IDC_INSTNAME, EN_CHANGE, OnEnChangeInstname)
		COMMAND_HANDLER(IDC_LOOPEDIT, EN_CHANGE, OnEnChangeLoopedit)
		COMMAND_HANDLER(IDC_NNA_COMBO, CBN_SELCHANGE, OnCbnSelchangeNnaCombo)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
private:

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedButton12(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton13(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton7(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawPanslider(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRpan(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRcut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRres(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheck4(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSlider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInsDecoctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInsDecnote(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInsIncnote(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInsIncoctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLoopoff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLoopforward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeInstname(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeLoopedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeNnaCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	CStatic	m_notelabel;
	CStatic	m_panlabel;
	CStatic	m_finelabel;
	CTrackBarCtrl	m_finetune;
	CEdit	m_loopedit;
	CButton	m_loopcheck;
	CButton	m_rres_check;
	CTrackBarCtrl	m_panslider;
	CButton	m_rpan_check;
	CButton	m_rcut_check;
	CComboBox	m_nna_combo;
	CEdit	m_instname;
	CStatic	m_volabel;
	CTrackBarCtrl	m_volumebar;
	CStatic	m_wlen;
	CStatic	m_loopstart;
	CStatic	m_loopend;
	CStatic	m_looptype;
	CStatic	m_instlabel;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_)
