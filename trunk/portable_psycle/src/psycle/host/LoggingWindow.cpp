#include "stdafx.h"
#include "Psycle.h"
#include "MainFrm.h"
#include "constants.h"
#include "LoggingWindow.h"
///\file
///\brief implementation file for psycle::host::CLoggingWindow.
namespace psycle
{
	namespace host
	{

		CLoggingWindow::CLoggingWindow(CWnd* pParent)
			: CDialog(CLoggingWindow::IDD, pParent)
		{
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

			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CLoggingWindow, CDialog)
			//{{AFX_MSG_MAP(CLoggingWindow)

			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CLoggingWindow::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			return TRUE;
		}

	}
}