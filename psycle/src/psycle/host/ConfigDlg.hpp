///\file
///\brief interface file for psycle::host::CConfigDlg.
#pragma once
#include "Psycle.hpp"
#include "DirectoryDlg.hpp"
#include "SkinDlg.hpp"
#include "OutputDlg.hpp"
#include "MidiInputDlg.hpp"
#include "Configuration.hpp"
#include "KeyConfigDlg.hpp"

namespace psycle { namespace host {

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
			void Init(Configuration* pConfig);
			void AddControlPages();
		// Overrides
			public:
			virtual INT_PTR DoModal();
		// Implementation
		public:
			virtual ~CConfigDlg();
			// Generated message map functions
		protected:
			Configuration* _pConfig;
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace
}   // namespace
