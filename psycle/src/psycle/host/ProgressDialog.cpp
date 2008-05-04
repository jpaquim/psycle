// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psycle::host::CProgressDialog.
#include <psycle/project.private.hpp>
#include "ProgressDialog.hpp"
#include "psycle.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		CProgressDialog::CProgressDialog(CWnd* pParent) : CDialog(CProgressDialog::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CProgressDialog)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
			m_pParent = pParent;
		}

		void CProgressDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CProgressDialog)
			DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
			//{{AFX_MSG_MAP(CProgressDialog)
				// NOTE: the ClassWizard will add message map macros here
			//}}AFX_MSG_MAP
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
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
