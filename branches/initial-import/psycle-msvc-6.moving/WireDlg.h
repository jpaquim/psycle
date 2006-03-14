#if !defined(AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
#define AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WireDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CWireDlg dialog

class CWireDlg : public CDialog
{
// Construction
public:
	CWireDlg(CWnd* pParent = NULL);   // standard constructor
	int wireIndex;
	Song* _pSong;
	int isrcMac;
	bool Inval;
	Machine* _pSrcMachine;
	Machine* _pDstMachine;
	int _dstWireIndex;
	char destName[32];
// Dialog Data
	//{{AFX_DATA(CWireDlg)
	enum { IDD = IDD_WIREDIALOG };
	CStatic	m_volabel;
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

	// Generated message map functions
	//{{AFX_MSG(CWireDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
