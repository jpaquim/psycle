#if !defined(AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_)
#define AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InstrumentEditor.h : header file
//

#include "resource.h"

class Song;

/////////////////////////////////////////////////////////////////////////////
// CInstrumentEditor dialog

class CInstrumentEditor : public CDialog
{
// Construction
public:
	void Validate();

	CInstrumentEditor(CWnd* pParent = NULL);   // standard constructor
	
	void WaveUpdate();
	void UpdateCombo();
	Song* _pSong;
	bool cando;

// Dialog Data
	//{{AFX_DATA(CInstrumentEditor)
	enum { IDD = IDD_INSTRUMENT };
	CStatic	m_finelabel;
	CSliderCtrl	m_finetune;
	CEdit	m_loopedit;
	CButton	m_loopcheck;
	CButton	m_rres_check;
	CSliderCtrl	m_panslider;
	CButton	m_rpan_check;
	CButton	m_rcut_check;
	CStatic	m_nnaval;
	CComboBox	m_nna_combo;
	CButton	m_pabu;
	CEdit	m_instname;
	CStatic	m_volabel;
	CSliderCtrl	m_volumebar;
	CStatic	m_wlen;
	CStatic	m_loopstart;
	CStatic	m_loopend;
	CStatic	m_looptype;
	CStatic	m_chanlabel;
	CStatic	m_status;
	CStatic	m_instlabel;
	CEdit	m_wavename;
	CStatic	m_layer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInstrumentEditor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInstrumentEditor)
	afx_msg void OnLayer1();
	afx_msg void OnLayer2();
	afx_msg void OnLayer3();
	afx_msg void OnLayer4();
	afx_msg void OnLayer5();
	afx_msg void OnLayer6();
	afx_msg void OnLayer7();
	afx_msg void OnLayer8();
	afx_msg void OnLayer9();
	afx_msg void OnLayer10();
	afx_msg void OnLayer11();
	afx_msg void OnLayer12();
	afx_msg void OnLayer13();
	afx_msg void OnLayer14();
	afx_msg void OnLayer15();
	afx_msg void OnLayer16();
	virtual BOOL OnInitDialog();
	afx_msg void OnLoopoff();
	afx_msg void OnLoopforward();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeInstname();
	afx_msg void OnChangeWavename();
	afx_msg void OnSelchangeNnaCombo();
	afx_msg void OnPrevInstrument();
	afx_msg void OnNextInstrument();
	afx_msg void OnEnvButton();
	afx_msg void OnCustomdrawPanslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRpan();
	afx_msg void OnRcut();
	afx_msg void OnRres();
	afx_msg void OnLoopCheck();
	afx_msg void OnChangeLoopedit();
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillInstrument();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_)
