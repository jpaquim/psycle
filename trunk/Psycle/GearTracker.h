#if !defined(AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_)
#define AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearTracker.h : header file
//

#include "Sampler.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearTracker dialog

class CGearTracker : public CDialog
{
// Construction
public:
	CGearTracker(CChildView* pParent = NULL);   // standard constructor
	Sampler* _pMachine;
	bool doit;
	BOOL Create();
	afx_msg void OnCancel();

// Dialog Data
	//{{AFX_DATA(CGearTracker)
	enum { IDD = IDD_GEAR_TRACKER };
	CComboBox	m_interpol;
	CSliderCtrl	m_polyslider;
	CStatic	m_polylabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearTracker)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearTracker)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_)
