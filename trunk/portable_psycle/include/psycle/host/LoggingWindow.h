#pragma once
#include "resource.h"
#include "LogEntry.h"
#include <vector>
#include <string>

///\file
///\brief interface file for psycle::host::CLoggingWindow.
namespace psycle
{
	namespace host
	{

		class CMainFrame;

		/// logging window.
		class CLoggingWindow : public CDialog
		{
		public:
			CLoggingWindow(CWnd* pParent = 0);
			void Validate();

			CMainFrame* pParentMain;
			
			// Dialog Data
			//{{AFX_DATA(CLoggingWindow)
			enum { IDD = IDD_ERRORLOGGER };
			CRichEditCtrl m_ErrorTxt;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CLoggingWindow)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CInstrumentEditor)
			virtual BOOL OnInitDialog();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnSize(UINT nType, int cx, int cy);
			void AddEntry(int Level, std::string String);

		private:
			void ResizeTextBox();
			CHARFORMAT defaultCF;
			CHARRANGE charrange;
			typedef std::vector<LogEntry*> LogEntries;
			LogEntries LogVector;

		};
		
		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
