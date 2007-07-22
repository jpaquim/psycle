///\file
///\brief interface file for psycle::host::CConfigDlg.
#pragma once
#include <psycle/host/DirectoryDlg.hpp>
#include <psycle/host/SkinDlg.hpp>
#include <psycle/host/OutputDlg.hpp>
#include <psycle/host/MidiInputDlg.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/host/KeyConfigDlg.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// config window.
		class CConfigDlg : public CPropertySheet
		{
			DECLARE_DYNAMIC(CConfigDlg)
		public:
			CConfigDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
			CConfigDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		// Attributes
		public:
			CDirectoryDlg _dirDlg;
			CSkinDlg _skinDlg;
			COutputDlg _outputDlg;
			CMidiInputDlg _midiDlg;
			CKeyConfigDlg _keyDlg;
		// Operations
		public:
			void Init(UIConfiguration* pConfig,int dlgnum=0);
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CConfigDlg)
			public:
			virtual int DoModal();
			//}}AFX_VIRTUAL
		// Implementation
		public:
			virtual ~CConfigDlg();
			// Generated message map functions
		protected:
			UIConfiguration* _pConfig;
			//{{AFX_MSG(CConfigDlg)
 				// NOTE - the ClassWizard will add and remove member functions here.
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
