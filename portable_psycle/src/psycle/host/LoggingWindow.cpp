#include "stdafx.h"
#include "Psycle.h"
#include "MainFrm.h"
#include "constants.h"
#include "LoggingWindow.h"
#include "..\..\..\include\psycle\host\loggingwindow.h"

///\file
///\brief implementation file for psycle::host::CLoggingWindow.
namespace psycle
{
	namespace host
	{

		CLoggingWindow::CLoggingWindow(CWnd* pParent)
			: CDialog(CLoggingWindow::IDD, pParent)
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
			
			defaultCF.crTextColor = RGB(255,0,0);		
			strcpy (defaultCF.szFaceName, "Lucida Console");
			defaultCF.yHeight = 160;
			defaultCF.cbSize = sizeof(defaultCF);
			defaultCF.dwMask = CFM_SIZE | CFM_FACE |  CFM_COLOR;

			m_ErrorTxt.SetDefaultCharFormat(defaultCF);
			m_ErrorTxt.HideSelection(FALSE, FALSE);

			return TRUE;
		}


		void CLoggingWindow::AddEntry(int Level, std::string String)
		{	
			switch (Level)
			{
				case 0: 
					defaultCF.crTextColor = RGB(255,0,0);
					break;
				case 1:
					defaultCF.crTextColor = RGB(0,255,0);
					break;
				case 2:
					defaultCF.crTextColor = RGB(0,0,255);
					break;
				default:
					defaultCF.crTextColor = RGB(0,0,255);
					break;
			}
			m_ErrorTxt.SetDefaultCharFormat(defaultCF);
			LogVector.push_back(new LogEntry(Level, String));
			m_ErrorTxt.ReplaceSel(LogVector[(LogVector.size()-1)]->string.c_str());
			m_ErrorTxt.ReplaceSel("\n");
			
		}

		void CLoggingWindow::ResizeTextBox()
		{
			CRect rect;                               	
			GetClientRect(&rect);	
			m_ErrorTxt.SetWindowPos(&m_ErrorTxt, 
									0, 0, 
									rect.right, 
									rect.bottom, 
									SWP_NOZORDER | SWP_SHOWWINDOW); 	
		}

		void CLoggingWindow::OnSize(UINT nType, int cx, int cy) 
		{
			//::MessageBox(0, "test", "test", MB_OK | MB_ICONWARNING);
			CDialog::OnSize(nType, cx, cy);

			// TODO: Add your message handler code here

			// ADD THE FOLLOWING CODE
			ResizeTextBox();
      			
		}

	}
}

