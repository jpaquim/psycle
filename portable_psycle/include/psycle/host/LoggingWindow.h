#pragma once
#include "resource.h"
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
			void AddEntry(const int & level, const std::string & string);
		private:
			void ResizeTextBox();
			CHARFORMAT defaultCF;
			CHARRANGE charrange;
			class LogEntry
			{
			public:
				const int level;
				const std::string string;
				inline LogEntry(const int & level, const std::string & string) : level(level), string(string) {}
			};
			typedef std::vector<LogEntry*> LogEntries;
			LogEntries LogVector;

		public:
			CLoggingWindow(CWnd * pParent = 0);
			void Validate();
			CMainFrame * pParentMain;
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
		};
		
		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
