#if !defined(AFX_GEARPLUGIN_H__DDC86A06_2B89_11D4_8913_B29429FB8B48__INCLUDED_)
#define AFX_GEARPLUGIN_H__DDC86A06_2B89_11D4_8913_B29429FB8B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearPlugin.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGearPlugin dialog

class CGearPlugin : public CDialog
{
// Construction
public:
	CGearPlugin(CWnd* pParent = NULL);   // standard constructor
	psyGear* machineRef;
	psySong* songRef;

private:
	void UpdateDialog();
	void SetCurrentParameter(int p);
	bool doit;


// Dialog Data
	//{{AFX_DATA(CGearPlugin)
	enum { IDD = IDD_GEAR_PLUGIN };
	CListBox	m_parlist;
	CButton	m_btr;
	CButton	m_btl;
	CStatic	m_plugdev;
	CButton	m_combut1;
	CStatic	m_short;
	CStatic	m_display;
	CSliderCtrl	m_parcontrol;
	CComboBox	m_combopar;
	CStatic	m_plugname;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearPlugin)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGearPlugin)
	afx_msg void OnBrowsePlug();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboPar();
	afx_msg void OnCustomdrawSliderPar(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReset();
	afx_msg void OnPreset();
	afx_msg void OnSelchangeParlist();
	afx_msg void OnRandomTweak();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARPLUGIN_H__DDC86A06_2B89_11D4_8913_B29429FB8B48__INCLUDED_)
