#if !defined(AFX_GEARFILTER_H__9190FCA0_1BD1_11D4_8913_BD0B45FB6E70__INCLUDED_)
#define AFX_GEARFILTER_H__9190FCA0_1BD1_11D4_8913_BD0B45FB6E70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Gearfilter.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearfilter dialog

class CGearfilter : public CDialog
{
// Construction
public:
	CGearfilter(CChildView* pParent = NULL);   // standard constructor
	Filter2p* _pMachine;
	BOOL Create();
	afx_msg void OnCancel();

// Dialog Data
	//{{AFX_DATA(CGearfilter)
	enum { IDD = IDD_GEAR_FILTER };
	CStatic	m_ParamInf6;
	CStatic	m_ParamInf5;
	CStatic	m_ParamInf4;
	CStatic	m_ParamInf3;
	CStatic	m_ParamInf2;
	CComboBox	m_filtercombo;
	CSliderCtrl	m_lfo_phase_slider;
	CSliderCtrl	m_lfo_amp_slider;
	CSliderCtrl	m_lfo_speed_slider;
	CSliderCtrl	m_filter_reso_slider;
	CSliderCtrl	m_filter_cutoff_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearfilter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateStatus();
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearfilter)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawFCS(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawFRS(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawLSS(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawLAS(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderPhase(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeFilter();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARFILTER_H__9190FCA0_1BD1_11D4_8913_BD0B45FB6E70__INCLUDED_)
