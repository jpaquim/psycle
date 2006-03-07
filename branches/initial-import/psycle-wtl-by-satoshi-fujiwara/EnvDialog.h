#if !defined(AFX_ENVDIALOG_H__F8DA64E0_1074_11D4_8913_B6FCFCBB2D72__INCLUDED_)
#define AFX_ENVDIALOG_H__F8DA64E0_1074_11D4_8913_B6FCFCBB2D72__INCLUDED_

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Song;

/////////////////////////////////////////////////////////////////////////////
// CEnvDialog dialog

class CEnvDialog : public CDialogImpl<CEnvDialog>
{
// Construction
public:
	CEnvDialog();   // standard constructor
	Song* _pSong;

	void DrawADSR(int AX,int BX,int CX,int DX);
	void DrawADSRFil(int AX,int BX,int CX,int DX);

// Dialog Data
	
	enum { IDD = IDD_ENVDIALOG};
	

// Overrides
	BEGIN_MSG_MAP_EX(CEnvDialog)
		NOTIFY_HANDLER_EX(IDC_SLIDER1,NM_CUSTOMDRAW,OnDrawAmpAttackSlider)
		NOTIFY_HANDLER_EX(IDC_SLIDER2,NM_CUSTOMDRAW,OnDrawAmpDecaySlider)
		NOTIFY_HANDLER_EX(IDC_SLIDER3, NM_CUSTOMDRAW,OnDrawAmpSustainSlider)
		NOTIFY_HANDLER_EX(IDC_SLIDER4, NM_CUSTOMDRAW, OnDrawAmpReleaseSlider)
		NOTIFY_HANDLER_EX(IDC_F_SLIDER1, NM_CUSTOMDRAW, OnCustomdrawFSlider1)
		NOTIFY_HANDLER_EX(IDC_F_SLIDER2, NM_CUSTOMDRAW, OnCustomdrawFSlider2)
		NOTIFY_HANDLER_EX(IDC_F_SLIDER3, NM_CUSTOMDRAW, OnCustomdrawFSlider3)
		NOTIFY_HANDLER_EX(IDC_F_SLIDER4,NM_CUSTOMDRAW, OnCustomdrawFSlider4)
		NOTIFY_HANDLER_EX(IDC_SLIDER9, NM_CUSTOMDRAW, OnCustomdrawSliderCutoff)
		NOTIFY_HANDLER_EX(IDC_SLIDER10, NM_CUSTOMDRAW, OnCustomdrawSliderQ)
		NOTIFY_HANDLER_EX(IDC_ENVELOPE, NM_CUSTOMDRAW, OnCustomdrawEnvelope)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeCombo1)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
// Implementation

private:
	
	LRESULT OnDrawAmpAttackSlider(NMHDR* pNMHDR);
	LRESULT OnDrawAmpDecaySlider(NMHDR* pNMHDR);
	LRESULT OnDrawAmpSustainSlider(NMHDR* pNMHDR);
	LRESULT OnDrawAmpReleaseSlider(NMHDR* pNMHDR);
	LRESULT OnCustomdrawFSlider1(NMHDR* pNMHDR);
	LRESULT OnCustomdrawFSlider2(NMHDR* pNMHDR);
	LRESULT OnCustomdrawFSlider3(NMHDR* pNMHDR);
	LRESULT OnCustomdrawFSlider4(NMHDR* pNMHDR);
	LRESULT OnCustomdrawSliderCutoff(NMHDR* pNMHDR);
	LRESULT OnCustomdrawSliderQ(NMHDR* pNMHDR);
	LRESULT OnCustomdrawEnvelope(NMHDR* pNMHDR);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
		EndDialog(IDOK);
		return 0;
	};

private:

	CStatic	m_envelope_label;
	CTrackBarCtrl	m_envelope_slider;
	CComboBox	m_filtercombo;
	CStatic	m_q_label;
	CStatic	m_cutoff_label;
	CTrackBarCtrl	m_q_slider;
	CTrackBarCtrl	m_cutoff_slider;
	CStatic	m_f_s_label;
	CStatic	m_f_r_label;
	CStatic	m_f_d_label;
	CStatic	m_f_a_label;
	CTrackBarCtrl	m_f_release_slider;
	CTrackBarCtrl	m_f_sustain_slider;
	CTrackBarCtrl	m_f_decay_slider;
	CTrackBarCtrl	m_f_attack_slider;
	CStatic	m_a_a_label;
	CStatic	m_filframe;
	CStatic	m_a_s_label;
	CStatic	m_a_r_label;
	CStatic	m_a_d_label;
	CStatic	m_ampframe;
	CTrackBarCtrl	m_a_release_slider;
	CTrackBarCtrl	m_a_sustain_slider;
	CTrackBarCtrl	m_a_decay_slider;
	CTrackBarCtrl	m_a_attack_slider;

public:
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENVDIALOG_H__F8DA64E0_1074_11D4_8913_B6FCFCBB2D72__INCLUDED_)
