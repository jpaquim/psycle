#if !defined(AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg dialog

class CSkinDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CSkinDlg)

// Construction
public:
	COLORREF _patternViewColor;
	COLORREF _fontColor;
	COLORREF _rowColor;
	COLORREF _beatColor;
	COLORREF _4beatColor;
	COLORREF _machineViewColor;
	COLORREF _vubColor;
	COLORREF _vugColor;
	COLORREF _vucColor;
	bool _gfxbuffer;

	CSkinDlg();
	~CSkinDlg();

// Dialog Data
	//{{AFX_DATA(CSkinDlg)
	enum { IDD = IDD_SKIN };
	CComboBox	m_cpresets;
	CButton	m_gfxbuffer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSkinDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool BrowseTo(char *rpath);

	// Generated message map functions
	//{{AFX_MSG(CSkinDlg)
	virtual void OnOK( );
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnColourMachine();
	afx_msg void OnButtonPattern();
	afx_msg void OnVuBarColor();
	afx_msg void OnVuBackColor();
	afx_msg void OnVuClipBar();
	afx_msg void OnRowc();
	afx_msg void OnFontc();
	afx_msg void OnBeatc();
	afx_msg void On4beat();
	afx_msg void OnDoublebuffer();
	afx_msg void OnSelendokPresetscombo();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void RepaintAllCanvas();
	void UpdateCanvasColour(int id,COLORREF col);
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
