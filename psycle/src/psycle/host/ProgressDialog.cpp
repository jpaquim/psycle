///\file
///\brief implementation file for psycle::host::CProgressDialog.

#include "ProgressDialog.hpp"

namespace psycle { namespace host {

		CProgressDialog::CProgressDialog(CWnd* pParent) : CDialog(CProgressDialog::IDD, pParent)
		{
			m_pParent = pParent;
		}

		void CProgressDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
		}

		BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
		END_MESSAGE_MAP()

		BOOL CProgressDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_Progress.SetPos(0);
			m_Progress.SetRange(0,16384);
			AfxGetApp()->DoWaitCursor(1);
			return true;
		}

		BOOL CProgressDialog::Create()
		{
			return CDialog::Create(IDD, m_pParent);
		}

		void CProgressDialog::OnCancel()
		{
			DestroyWindow();
			AfxGetApp()->DoWaitCursor(-1); 
		}

	}   // namespace
}   // namespace
