#pragma once
#include "Machine.h"
#include "constants.h"
#include "resource.h"
///\file
///\brief interface file for psycle::host::CMixerDlg.
namespace psycle
{
	namespace host
	{
		class CMainFrame;

		class Song;

		/// wire monitor window.
		class CMixerDlg : public CDialog
		{
		public:
			CMixerDlg(CWnd* pParent);
			CMixerDlg();
	
		// Dialog Data
			//{{AFX_DATA(CMixerDlg)
			enum { IDD = IDD_MIXER };

			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CMixerDlg)
			public:
			protected:

			//}}AFX_VIRTUAL
		// Implementation
		protected:
			CMainFrame* m_pParent;

			// Generated message map functions
			//{{AFX_MSG(CMixerDlg)

			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
