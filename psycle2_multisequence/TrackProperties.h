#pragma once


// CTrackProperties dialog

class CTrackProperties : public CDialog
{
	DECLARE_DYNAMIC(CTrackProperties)

public:
	CTrackProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrackProperties();

// Dialog Data
	enum { IDD = IDD_TRACKPROP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	char trackName[32];
	afx_msg void OnBnClickedOk();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnEnChangeTracknameeb();
};
