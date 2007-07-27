///\file
///\brief implementation file for psycle::host::CPatDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/engine/constants.hpp>
#include <psycle/host/PatDlg.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CPatDlg::CPatDlg(CWnd* pParent) : CDialog(CPatDlg::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CPatDlg)
			m_adaptsize = false;
			//}}AFX_DATA_INIT
			bInit = false;
		}

		void CPatDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CPatDlg)
			DDX_Control(pDX, IDC_CHECK1, m_adaptsizeCheck);
			DDX_Control(pDX, IDC_EDIT2, m_numlines);
			DDX_Control(pDX, IDC_EDIT1, m_patname);
			DDX_Control(pDX, IDC_SPIN1, m_spinlines);
			DDX_Check(pDX, IDC_CHECK1, m_adaptsize);
			DDX_Control(pDX, IDC_TEXT, m_text);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CPatDlg, CDialog)
			//{{AFX_MSG_MAP(CPatDlg)
			ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
			ON_EN_UPDATE(IDC_EDIT2, OnUpdateNumLines)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CPatDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_spinlines.SetRange(1,MAX_LINES);
			m_patname.SetWindowText(patName);
			m_patname.SetLimitText(30);
			char buffer[16];
			itoa(patLines,buffer,10);
			m_numlines.SetWindowText(buffer);
			UDACCEL acc;
			acc.nSec = 4;
			acc.nInc = 16;
			m_spinlines.SetAccel(1, &acc);
			// Pass the focus to the texbox
			m_patname.SetFocus();
			m_patname.SetSel(0,-1);
			bInit = true;
			OnUpdateNumLines();
			return false;
		}

		void CPatDlg::OnOK() 
		{
			char buffer[32];
			m_numlines.GetWindowText(buffer,16);
			
			int nlines = atoi(buffer);

			if (nlines < 1)
				{ nlines = 1; }
			else if (nlines > MAX_LINES)
				{ nlines = MAX_LINES; }

			patLines=nlines;

			m_patname.GetWindowText(buffer,31);
			buffer[31]='\0';

			strcpy(patName,buffer);
			
			CDialog::OnOK();
		}

		void CPatDlg::OnCheck1() 
		{
			m_adaptsize = m_adaptsizeCheck.GetCheck();
		}

		void CPatDlg::OnUpdateNumLines() 
		{
			char buffer[256];
			if (bInit)
			{
				m_numlines.GetWindowText(buffer,16);
				int val=atoi(buffer);

				if (val < 0)
				{
					val = 0;
				}
				else if(val > MAX_LINES)
				{
					val = MAX_LINES-1;
				}
				sprintf(buffer,"HEX: %x",val);
				m_text.SetWindowText(buffer);
			}
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
