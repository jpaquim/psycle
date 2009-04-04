///\file
///\brief interface file for psycle::host::CSongpDlg.
#pragma once
#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psy {
	namespace core {
		class Song;
	}
}
using namespace psy::core;
#endif


PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

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

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
