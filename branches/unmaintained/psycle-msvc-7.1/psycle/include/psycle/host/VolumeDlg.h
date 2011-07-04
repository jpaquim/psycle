#if !defined(AFX_VOLUMEDLG_H__4F5FAD34_8830_48D7_A9BD_5573B0EF82BA__INCLUDED_)
#define AFX_VOLUMEDLG_H__4F5FAD34_8830_48D7_A9BD_5573B0EF82BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VolumeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVolumeDlg dialog

class CVolumeDlg : public CDialog
{
// Construction
public:
	CVolumeDlg(CWnd* pParent = NULL);   // standard constructor
	float volume;
	int edit_type;

// Dialog Data
	//{{AFX_DATA(CVolumeDlg)
	enum { IDD = IDD_NEW_VOLUME };
	CEdit		m_db;
	CEdit		m_per;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVolumeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DrawDb();
	void DrawPer();
	bool go;

	// Generated message map functions
	//{{AFX_MSG(CVolumeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditDb();
	afx_msg void OnChangeEditPer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOLUMEDLG_H__4F5FAD34_8830_48D7_A9BD_5573B0EF82BA__INCLUDED_)
