#include <stdafx.h>
#include <psycle/host/psycle.h>
#include <psycle/host/MainFrm.h>
#include <psycle/host/constants.h>
#include <psycle/host/LoggingWindow.h>
#include <cstring>
///\file
///\brief implementation file for psycle::host::CLoggingWindow.
namespace psycle
{
	namespace host
	{
		CLoggingWindow::CLoggingWindow(CWnd * pParent) : CDialog(CLoggingWindow::IDD, pParent)
		{
			AfxInitRichEdit();
			//{{AFX_DATA_INIT(CLoggingWindow)
			//}}AFX_DATA_INIT
		}

		void CLoggingWindow::Validate()
		{
			pParentMain=(CMainFrame*)GetParentFrame();
		}

		void CLoggingWindow::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CLoggingWindow)
			DDX_Control(pDX, IDC_ERRORLOG_TXT, m_ErrorTxt);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CLoggingWindow, CDialog)
			//{{AFX_MSG_MAP(CLoggingWindow)
			//}}AFX_MSG_MAP
			ON_WM_SIZE()
		END_MESSAGE_MAP()

		BOOL CLoggingWindow::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			ResizeTextBox();
			defaultCF.crTextColor = RGB(0, 0, 0);
			std::strcpy(defaultCF.szFaceName, "Lucida Console");
			defaultCF.yHeight = 160;
			defaultCF.cbSize = sizeof(defaultCF);
			defaultCF.dwMask = CFM_SIZE | CFM_FACE |  CFM_COLOR;
			m_ErrorTxt.SetDefaultCharFormat(defaultCF);
			m_ErrorTxt.HideSelection(TRUE, TRUE);
			return TRUE;
		}

		void CLoggingWindow::AddEntry(const int & level, const std::string & string)
		{	
			LogVector.push_back(new LogEntry(level, string));
			// puts a separator
			{
				defaultCF.crTextColor = RGB(0, 0, 0);
				m_ErrorTxt.SetSelectionCharFormat(defaultCF);
				m_ErrorTxt.ReplaceSel("===\n");
			}
			switch(level)
			{
			case host::logger::trace: 
				defaultCF.crTextColor = RGB(128, 128, 128);
				break;
			case host::logger::info:
				defaultCF.crTextColor = RGB(0, 64, 0);
				break;
			case host::logger::crash:
				defaultCF.crTextColor = RGB(255, 0, 0);
				break;
			default:
				defaultCF.crTextColor = RGB(255, 0, 255);
			}
			m_ErrorTxt.SetSelectionCharFormat(defaultCF);
			m_ErrorTxt.ReplaceSel((**--LogVector.end()).string.c_str());
			m_ErrorTxt.ReplaceSel("\n");
			m_ErrorTxt.SetSelectionCharFormat(defaultCF);
			if(level > host::logger::info) Global::pLogWindow->ShowWindow(SW_SHOWNORMAL);
		}

		void CLoggingWindow::ResizeTextBox()
		{
			CRect rect;                               	
			GetClientRect(&rect);	
			m_ErrorTxt.SetWindowPos
				(
					&m_ErrorTxt, 
					0, 0, 
					rect.right, 
					rect.bottom, 
					SWP_NOZORDER | SWP_SHOWWINDOW
				);
		}

		void CLoggingWindow::OnSize(UINT nType, int cx, int cy) 
		{
			//::MessageBox(0, "test", "test", MB_OK | MB_ICONWARNING);
			CDialog::OnSize(nType, cx, cy);
			ResizeTextBox();
		}
	}
}
