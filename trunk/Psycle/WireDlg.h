#if !defined(AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
#define AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WireDlg.h : header file
//
#include "Machine.h"
#include "constants.h"

class CChildView;

#define MAX_SCOPE_BANDS 32
#define SCOPE_BUF_SIZE 256
#define SCOPE_SPEC_SAMPLES	256

class Song;

/////////////////////////////////////////////////////////////////////////////
// CWireDlg dialog

class CWireDlg : public CDialog
{

// Construction
public:
	CWireDlg(CChildView* pParent = NULL);   // standard constructor
	BOOL Create();
	afx_msg void OnCancel();
	UINT this_index;
	int wireIndex;
	int isrcMac;
	bool Inval;
	Machine* _pSrcMachine;
	Machine* _pDstMachine;
	int _dstWireIndex;
	float invol;

	int scope_mode;
	int scope_osc_freq;
	int scope_osc_rate;
	int scope_spec_bands;
	int scope_spec_rate;
// Dialog Data
	//{{AFX_DATA(CWireDlg)
	enum { IDD = IDD_WIREDIALOG };
	CSliderCtrl	m_slider;
	CSliderCtrl	m_slider2;
	CStatic	m_volabel;
	CButton m_mode;
	CSliderCtrl	m_volslider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWireDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	inline int GetY(float f);
	void SetMode();
	CChildView* m_pParent;
	CBitmap* bmpDC;
	CRect rc;
	CFont font;
	CFont* oldFont;
	int pos;
	BOOL hold;
	int bar_heightsl[MAX_SCOPE_BANDS];
	int bar_heightsr[MAX_SCOPE_BANDS];

	// Generated message map functions
	//{{AFX_MSG(CWireDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton1();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMode();
	afx_msg void OnHold();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
