///\file
///\brief implementation file for psycle::host::CMidiLearn.
#include <psycle/project.private.hpp>
#include "MidiLearn.hpp"
#include "Psycle.hpp"
#include "OutputDlg.hpp"
#include "MidiInput.hpp"
#include "Configuration.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		CMidiLearn::CMidiLearn(CWnd* pParent) : CDialog(CMidiLearn::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CMidiLearn)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
			Message = -1;
			//Global::pConfig->_pMidiInput->fnMidiCallback_Test = (MidiCallback_Test);
		}

		void CMidiLearn::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CMidiLearn)
				// NOTE: the ClassWizard will add DDX and DDV calls here
			DDX_Control(pDX, IDC_TEXT, m_Text);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CMidiLearn, CDialog)
			//{{AFX_MSG_MAP(CMidiLearn)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		void CMidiLearn::OnCancel() 
		{
			Message = -1;
			CDialog::OnCancel();
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
