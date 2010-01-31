#pragma once
#include "Machine.h"
#include "ChildView.h"
///\file
///\brief interface file for psycle::host::CMacProp.
namespace psycle
{
	namespace host
	{
		class Song;

		/// machine properties window.
		class CMacProp : public CDialog
		{
		public:
			CChildView *m_view;
			CMacProp(CWnd* pParent = 0);
			Machine *pMachine;
			Song* pSong;
			int thisMac;
			char txt[32];
			bool deleted;
		// Dialog Data
			//{{AFX_DATA(CMacProp)
			enum { IDD = IDD_MACPROP };
			CButton	m_soloCheck;
			CButton	m_bypassCheck;
			CButton	m_muteCheck;
			CEdit	m_macname;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CMacProp)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CMacProp)
			virtual BOOL OnInitDialog();
			afx_msg void OnChangeEdit1();
			afx_msg void OnButton1();
			afx_msg void OnMute();
			afx_msg void OnBypass();
			afx_msg void OnSolo();
			afx_msg void OnClone();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}