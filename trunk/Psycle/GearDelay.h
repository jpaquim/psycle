#if !defined(AFX_GEARDELAY_H__50335B20_0E81_11D4_8913_FACF4EED7F7C__INCLUDED_)
#define AFX_GEARDELAY_H__50335B20_0E81_11D4_8913_FACF4EED7F7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearDelay.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearDelay dialog

class CGearDelay : public CDialog
{
// Construction
public:
	CGearDelay(CChildView* pParent = NULL);   // standard constructor
	Delay* _pMachine;
	int* SPT;
	bool doit;
	BOOL Create();
	afx_msg void OnCancel();

// Dialog Data
	//{{AFX_DATA(CGearDelay)
	enum { IDD = IDD_GEAR_DELAY };
	CStatic	m_drylabel;
	CStatic	m_wetlabel;
	CSliderCtrl	m_wetslider;
	CSliderCtrl	m_dryslider;
	CStatic	m_msr;
	CStatic	m_msl;
	CStatic	m_flabel_r;
	CStatic	m_flabel_l;
	CSliderCtrl	m_sliderfeedback_r;
	CSliderCtrl	m_slidertime_r;
	CStatic	m_label_timer;
	CSliderCtrl	m_sliderfeedback_l;
	CStatic	m_label_timel;
	CSliderCtrl	m_slidertime_l;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearDelay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearDelay)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSliderTime(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTime2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton1();
	afx_msg void OnButton3();
	afx_msg void OnButton14();
	afx_msg void OnButton15();
	afx_msg void OnButton16();
	afx_msg void OnButton17();
	afx_msg void OnButton11();
	afx_msg void OnButton18();
	afx_msg void OnButton19();
	afx_msg void OnButton20();
	afx_msg void OnButton21();
	afx_msg void OnButton22();
	afx_msg void OnCustomdrawDryslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawWetslider(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARDELAY_H__50335B20_0E81_11D4_8913_FACF4EED7F7C__INCLUDED_)
