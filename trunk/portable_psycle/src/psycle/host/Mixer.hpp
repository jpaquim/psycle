///\file
///\brief interface file for psycle::host::CMixerDlg.
#pragma once
#include "Machine.hpp"
#include "constants.hpp"
#include "resources/resources.hpp"
namespace psycle
{
	namespace host
	{
		class CMainFrame;

		class Song;

		/// mixer window.
		class CMixerDlg : public CDialog
		{
		public:
			CMixerDlg(CWnd* pParent);
			CMixerDlg();
	
		// Dialog Data
			//{{AFX_DATA(CMixerDlg)
			enum { IDD = IDD_MIXER };
			CEdit	m_Numbers[16];
			CSliderCtrl	m_slider[16];
			CButton m_db[16];
			CButton m_percent[16];
			CEdit	m_Label[16];
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CMixerDlg)
			public:
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

			//}}AFX_VIRTUAL
		// Implementation
		protected:
			CMainFrame* m_pParent;

			// Generated message map functions
			//{{AFX_MSG(CMixerDlg)
			virtual BOOL OnInitDialog();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
