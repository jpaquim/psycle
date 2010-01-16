///\file
///\brief interface file for psycle::host::CSongpDlg.
#pragma once
#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psycle {
	namespace core {
		class Song;
	}
}
using namespace psycle::core;
#endif


namespace psycle {
	namespace host {

#if !PSYCLE__CONFIGURATION__USE_PSYCORE
	class Song; // forward declaration
#endif

		/// song dialog window.
		class CSongpDlg : public CDialog
		{
		public:
			/// mfc compliant constructor.
			CSongpDlg(Song *song, CWnd* pParent = 0);
			Song* _pSong;
		// Dialog Data
			enum { IDD = IDD_SONGPROP };
			CEdit	m_songcomments;
			CEdit	m_songcredits;
			CEdit	m_songtitle;
			bool	readonlystate;
		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		public:
			void SetReadOnly();
		protected:
			// Generated message map functions
			virtual BOOL OnInitDialog();
			afx_msg void OnOk();
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace
}   // namespace
