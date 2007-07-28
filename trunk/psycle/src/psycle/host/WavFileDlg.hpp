///\file
///\brief interface file for psycle::host::CWavFileDlg.
#pragma once
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class Song;

		/// wave file dialog window.
		class CWavFileDlg : public CFileDialog
		{
			DECLARE_DYNAMIC(CWavFileDlg)
		public:
			Song *_pSong;
			CString _lastFile;
			CWavFileDlg(
				BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
				LPCTSTR lpszDefExt = NULL,
				LPCTSTR lpszFileName = NULL,
				DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				LPCTSTR lpszFilter = NULL,
				CWnd* pParentWnd = NULL);

			virtual void OnFileNameChange();
		protected:
			//{{AFX_MSG(CWavFileDlg)
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
