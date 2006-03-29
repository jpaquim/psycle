///\file
///\brief interface file for psycle::host::CConfigDlg.
#pragma once
#include <psycle/host/gui/DirectoryDlg.hpp>
#include <psycle/host/gui/SkinDlg.hpp>
#include <psycle/host/gui/OutputDlg.hpp>
#include <psycle/host/gui/MidiInputDlg.hpp>
#include <psycle/host/Configuration.hpp>
#include <psycle/host/gui/KeyConfigDlg.hpp>
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
			void Init(Configuration* pConfig,int dlgnum=0);
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
			Configuration* _pConfig;
			//{{AFX_MSG(CConfigDlg)
 				// NOTE - the ClassWizard will add and remove member functions here.
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
