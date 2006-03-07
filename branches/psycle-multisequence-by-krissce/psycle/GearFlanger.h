#if !defined(AFX_GEARFLANGER_H__FA758DA0_2AA7_11D4_8913_D7ED284FD848__INCLUDED_)
#define AFX_GEARFLANGER_H__FA758DA0_2AA7_11D4_8913_D7ED284FD848__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearFlanger.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearFlanger dialog

class CGearFlanger : public CDialog
{
// Construction
public:
	CGearFlanger(CChildView* pParent = NULL);   // standard constructor
	Flanger* _pMachine;
	BOOL Create();
	afx_msg void OnCancel();


// Dialog Data
	//{{AFX_DATA(CGearFlanger)
	enum { IDD = IDD_GEAR_FLANGER };
	CButton	m_resample;
	CComboBox	m_presetcombo;
	CSliderCtrl	m_wet_slider;
	CSliderCtrl	m_dry_slider;
	CStatic	m_wet_label;
	CStatic	m_dry_label;
	CStatic	m_rf_label;
	CSliderCtrl	m_rf_slider;
	CSliderCtrl	m_phase_slider;
	CStatic	m_phase_label;
	CSliderCtrl	m_lf_slider;
	CStatic	m_lf_label;
	CSliderCtrl	m_speed_slider;
	CStatic	m_speed_label;
	CStatic	m_amp_label;
	CSliderCtrl	m_amp_slider;
	CStatic	m_delay_time_label;
	CSliderCtrl	m_flanger_delay_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearFlanger)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearFlanger)
	afx_msg void OnCustomdrawFlangerDelaySlider(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawAmpSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderSpeed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawLfSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderPhase(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderFr(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawWetslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawDryslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangePresetcombo();
	afx_msg void OnCheckResampler();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARFLANGER_H__FA758DA0_2AA7_11D4_8913_D7ED284FD848__INCLUDED_)
