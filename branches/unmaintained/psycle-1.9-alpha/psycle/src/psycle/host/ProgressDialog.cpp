///\file
///\brief implementation file for psycle::host::CProgressDialog.

#include <psycle/host/detail/project.private.hpp>
#include "ProgressDialog.hpp"

namespace psycle { namespace host {

		BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
			 ON_WM_SHOWWINDOW()
		END_MESSAGE_MAP()

		CProgressDialog::CProgressDialog(CWnd* pParent) 
			: CDialog(IDD_PROGRESS_DIALOG, pParent) {
			CDialog::Create(IDD_PROGRESS_DIALOG, pParent);
			progress_ctrl_.SetPos(0);
			progress_ctrl_.SetRange(0,16384);
		}

		void CProgressDialog::DoDataExchange(CDataExchange* pDX) {
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_PROGRESS1, progress_ctrl_);
		}

		void CProgressDialog::OnShowWindow(BOOL bShow, UINT nStatus) {
			CDialog::OnShowWindow(bShow, nStatus);
			AfxGetApp()->DoWaitCursor(bShow ? 1 : -1);
			::Sleep(1);
		}

	}   // namespace
}   // namespace
