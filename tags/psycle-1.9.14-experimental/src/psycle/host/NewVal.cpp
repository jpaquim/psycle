///\file
///\brief implementation file for psycle::host::CNewVal.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/NewVal.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CNewVal::CNewVal(CWnd* pParent)
			: CDialog(CNewVal::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CNewVal)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
			m_Value=0;
		}

		void CNewVal::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CNewVal)
			DDX_Control(pDX, IDC_EDIT1, m_value);
			DDX_Control(pDX, IDC_TEXT, m_text);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CNewVal, CDialog)
			//{{AFX_MSG_MAP(CNewVal)
			ON_EN_UPDATE(IDC_EDIT1, OnUpdateEdit1)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CNewVal::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			SetWindowText(title);
			char buf[32];
			sprintf(buf,"%d",m_Value);
			m_value.SetWindowText(buf);
			m_value.SetSel(-1,-1,false);
			return true;
		}

		void CNewVal::OnOK() 
		{
			char buffer[16];
			m_value.GetWindowText(buffer,16);
			m_Value=atoi(buffer);
			CDialog::OnOK();
		}

		void CNewVal::OnUpdateEdit1() 
		{
			char buffer[256];
			m_value.GetWindowText(buffer,16);
			m_Value=atoi(buffer);

			if (m_Value < min)
			{
				m_Value = min;
				sprintf(buffer,"Below Range. Use this HEX value: twk %.2X %.2X %.4X",paramindex,macindex,m_Value-min);
			}
			else if(m_Value > max)
			{
				m_Value = max;
				sprintf(buffer,"Above Range. Use this HEX value: twk %.2X %.2X %.4X",paramindex,macindex,m_Value-min);
			}
			else
			{
				sprintf(buffer,"Use this HEX value: twk %.2X %.2X %.4X",paramindex,macindex,m_Value-min);
			}
			m_text.SetWindowText(buffer);
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
