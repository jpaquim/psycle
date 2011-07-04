#if !defined(AFX_MIDILEARN_H__A28856CD_9197_4DCB_9D3B_C5D8B8A94B5A__INCLUDED_)
#define AFX_MIDILEARN_H__A28856CD_9197_4DCB_9D3B_C5D8B8A94B5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiLearn.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiLearn dialog

class CMidiLearn : public CDialog
{
// Construction
public:
	CMidiLearn(CWnd* pParent = NULL);   // standard constructor

	int Message;
	void __cdecl MidiCallback_Test( HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );

// Dialog Data
	//{{AFX_DATA(CMidiLearn)
	enum { IDD = IDD_MIDI_LEARN };
	CStatic		m_Text;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiLearn)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMidiLearn)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDILEARN_H__A28856CD_9197_4DCB_9D3B_C5D8B8A94B5A__INCLUDED_)
