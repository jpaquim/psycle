#include "stdafx.h"
#include "Psycle.h"
#include "Machine.h"
#include "Mixer.h"
#include "Helpers.h"
#include "ChildView.h"
#include "InputHandler.h"
#include "VolumeDlg.h"
///\file
///\brief implementation file for psycle::host::CMixerDlg.
namespace psycle
{
	namespace host
	{
		CMixerDlg::CMixerDlg(CWnd* pParent = 0) : CDialog(CMixerDlg::IDD, pParent = 0)
		{
			m_pParent = (CMainFrame*)pParent;
			//{{AFX_DATA_INIT(CMixerDlg)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
		}

		CMixerDlg::CMixerDlg() : CDialog()
		{
			//{{AFX_DATA_INIT(CMixerDlg)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
		}

		BEGIN_MESSAGE_MAP(CMixerDlg, CDialog)
			//{{AFX_MSG_MAP(CMixerDlg)

			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()
			
	}
}