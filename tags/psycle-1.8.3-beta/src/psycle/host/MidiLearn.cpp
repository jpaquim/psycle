///\file
///\brief implementation file for psycle::host::CMidiLearn.
#include <project.private.hpp>
#include "Psycle.hpp"
#include "OutputDlg.hpp"
#include "MidiInput.hpp"
#include "Configuration.hpp"
#include "MidiLearn.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
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
	NAMESPACE__END
NAMESPACE__END
