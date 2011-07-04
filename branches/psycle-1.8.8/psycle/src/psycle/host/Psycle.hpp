///\file
///\brief interface file for psycle::host::CPsycleApp.
#pragma once
#include "resources/resources.hpp" // main symbols
#include "Global.hpp"
#include <psycle/helpers/hexstring_to_integer.hpp>

namespace psycle { namespace host {

		class CMainFrame; // forward declaration

		/// root class.
		class CPsycleApp : public CWinApp
		{
		public:
			CPsycleApp();
			virtual ~CPsycleApp() throw();
		public:
			static bool BrowseForFolder(HWND hWnd_, char* title_, std::string& rpath);
			void RestoreRecentFiles();
		protected:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual BOOL InitInstance();
			virtual int ExitInstance();
			virtual BOOL IsIdleMessage( MSG* pMsg );
			virtual BOOL OnIdle(LONG lCount);
		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnAppAbout();
		public:
			Global global_;
		private:
			void ProcessCmdLine(LPSTR cmdline);

			UINT m_uUserMessage;
		};


		/////////////////////////////////////////////////////////////////////////////
		// CAboutDlg dialog used for App About

		class CAboutDlg : public CDialog
		{
		public:
			CAboutDlg();
			enum { IDD = IDD_ABOUTBOX };
			CStatic	m_asio;
			CEdit	m_sourceforge;
			CEdit	m_psycledelics;
			CStatic	m_steincopyright;
			CButton	m_showabout;
			CStatic	m_headercontrib;
			CStatic	m_aboutbmp;
			CEdit	m_contrib;
			CStatic m_versioninfo;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnContributors();
			afx_msg void OnShowatstartup();
		};



}}
