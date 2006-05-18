///\file
///\brief implementation file for psycle::host::CWaveEdInsertSilenceDialog.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "WaveEdInsertSilenceDialog.hpp"
#include "psycle.hpp"
#include <psycle/host/engine/helpers.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CWaveEdInsertSilenceDialog::CWaveEdInsertSilenceDialog(CWnd* pParent)
			: CDialog(CWaveEdInsertSilenceDialog::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CWaveEdInsertSilenceDialog)
			//}}AFX_DATA_INIT

		}

		void CWaveEdInsertSilenceDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CWaveEdInsertSilenceDialog)
			DDX_Control(pDX, IDC_EDIT1, m_time);
			DDX_Control(pDX, IDC_RADIO_ATSTART, m_atStart);
			DDX_Control(pDX, IDC_RADIO_ATEND, m_atEnd);
			DDX_Control(pDX, IDC_RADIO_ATCURSOR, m_atCursor);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CWaveEdInsertSilenceDialog, CDialog)
			//{{AFX_MSG_MAP(CWaveEdInsertSilenceDialog)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CWaveEdInsertSilenceDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_atStart.SetCheck(BST_CHECKED);
			m_time.SetWindowText("0.000");
			return TRUE;
		}

		void CWaveEdInsertSilenceDialog::OnOK() 
		{
			char temp[16];
			m_time.GetWindowText(temp, 15);
			if(!(timeInSecs=atof(temp)))
				EndDialog(IDCANCEL);

			if(m_atStart.GetCheck()) 
				insertPos=at_start;
			else if(m_atEnd.GetCheck()) 
				insertPos=at_end;
			else 
				insertPos=at_cursor;


			CDialog::OnOK();
		}

		void CWaveEdInsertSilenceDialog::OnCancel() 
		{
			CDialog::OnCancel();
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
