#if !defined(AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_)
#define AFX_INSTRUMENTEDITOR_H__D1E9B5CD_F954_11D3_8913_444553540000__INCLUDED_

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
	CMainFrame* pParentMain;


// Dialog Data
	//{{AFX_DATA(CInstrumentEditor)
	enum { IDD = IDD_INSTRUMENT };
	CStatic	m_panlabel;
	CStatic	m_finelabel;
	CSliderCtrl	m_finetune;
	CEdit	m_loopedit;
	CButton	m_loopcheck;
	CButton	m_rres_check;
	CSliderCtrl	m_panslider;
	CButton	m_rpan_check;
	CButton	m_rcut_check;
	CComboBox	m_nna_combo;
	CEdit	m_instname;
	CStatic	m_volabel;
	CSliderCtrl	m_volumebar;
	CStatic	m_wlen;
	CStatic	m_loopstart;
	CStatic	m_loopend;
	CStatic	m_looptype;
	CStatic	m_instlabel;
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
	virtual BOOL OnInitDialog();
	afx_msg void OnLoopoff();
	afx_msg void OnLoopforward();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeInstname();
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
