///\file
///\brief implementation file for psycle::host::CMixerDlg.
#include <project.private.hpp>
#include "Psycle.hpp"
#include "Machine.hpp"
#include "Mixer.hpp"
#include "Helpers.hpp"
#include "ChildView.hpp"
#include "InputHandler.hpp"
#include "VolumeDlg.hpp"
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

		BOOL CMixerDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			for (int i = 1; i <= 16; i++)
			{			
				std::ostringstream oss;
				oss << i;
				m_Numbers[i-1].SetWindowText(oss.str().c_str());
			}
			
			return TRUE;
		}


		void CMixerDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CWireDlg)
			DDX_Control(pDX, IDC_EDIT17, m_Numbers[0]);
			DDX_Control(pDX, IDC_EDIT18, m_Numbers[1]);
			DDX_Control(pDX, IDC_EDIT19, m_Numbers[2]);
			DDX_Control(pDX, IDC_EDIT20, m_Numbers[3]);
			DDX_Control(pDX, IDC_EDIT21, m_Numbers[4]);
			DDX_Control(pDX, IDC_EDIT22, m_Numbers[5]);
			DDX_Control(pDX, IDC_EDIT23, m_Numbers[6]);
			DDX_Control(pDX, IDC_EDIT24, m_Numbers[7]);
			DDX_Control(pDX, IDC_EDIT25, m_Numbers[8]);
			DDX_Control(pDX, IDC_EDIT26, m_Numbers[9]);
			DDX_Control(pDX, IDC_EDIT27, m_Numbers[10]);
			DDX_Control(pDX, IDC_EDIT28, m_Numbers[11]);
			DDX_Control(pDX, IDC_EDIT29, m_Numbers[12]);
			DDX_Control(pDX, IDC_EDIT30, m_Numbers[13]);
			DDX_Control(pDX, IDC_EDIT31, m_Numbers[14]);
			DDX_Control(pDX, IDC_EDIT32, m_Numbers[15]);
			DDX_Control(pDX, IDC_MIXSLIDER1, m_slider[0]);
			DDX_Control(pDX, IDC_MIXSLIDER2, m_slider[1]);
			DDX_Control(pDX, IDC_MIXSLIDER3, m_slider[2]);
			DDX_Control(pDX, IDC_MIXSLIDER4, m_slider[3]);
			DDX_Control(pDX, IDC_MIXSLIDER5, m_slider[4]);
			DDX_Control(pDX, IDC_MIXSLIDER6, m_slider[5]);
			DDX_Control(pDX, IDC_MIXSLIDER7, m_slider[6]);
			DDX_Control(pDX, IDC_MIXSLIDER8, m_slider[7]);
			DDX_Control(pDX, IDC_MIXSLIDER9, m_slider[8]);
			DDX_Control(pDX, IDC_MIXSLIDER10, m_slider[9]);
			DDX_Control(pDX, IDC_MIXSLIDER11, m_slider[10]);
			DDX_Control(pDX, IDC_MIXSLIDER12, m_slider[11]);
			DDX_Control(pDX, IDC_MIXSLIDER13, m_slider[12]);
			DDX_Control(pDX, IDC_MIXSLIDER14, m_slider[13]);
			DDX_Control(pDX, IDC_MIXSLIDER15, m_slider[14]);
			DDX_Control(pDX, IDC_MIXSLIDER16, m_slider[15]);
			DDX_Control(pDX, IDC_VOLUME_DB, m_db[0]);
			DDX_Control(pDX, IDC_VOLUME_DB2, m_db[1]);
			DDX_Control(pDX, IDC_VOLUME_DB3, m_db[2]);
			DDX_Control(pDX, IDC_VOLUME_DB4, m_db[3]);
			DDX_Control(pDX, IDC_VOLUME_DB5, m_db[4]);
			DDX_Control(pDX, IDC_VOLUME_DB6, m_db[5]);
			DDX_Control(pDX, IDC_VOLUME_DB7, m_db[6]);
			DDX_Control(pDX, IDC_VOLUME_DB8, m_db[7]);
			DDX_Control(pDX, IDC_VOLUME_DB9, m_db[8]);
			DDX_Control(pDX, IDC_VOLUME_DB10, m_db[9]);
			DDX_Control(pDX, IDC_VOLUME_DB11, m_db[10]);
			DDX_Control(pDX, IDC_VOLUME_DB12, m_db[11]);
			DDX_Control(pDX, IDC_VOLUME_DB13, m_db[12]);
			DDX_Control(pDX, IDC_VOLUME_DB14, m_db[13]);
			DDX_Control(pDX, IDC_VOLUME_DB15, m_db[14]);
			DDX_Control(pDX, IDC_VOLUME_DB16, m_db[15]);
			DDX_Control(pDX, IDC_VOLUME_PER, m_percent[0]);
			DDX_Control(pDX, IDC_VOLUME_PER2, m_percent[1]);
			DDX_Control(pDX, IDC_VOLUME_PER3, m_percent[2]);
			DDX_Control(pDX, IDC_VOLUME_PER4, m_percent[3]);
			DDX_Control(pDX, IDC_VOLUME_PER5, m_percent[4]);
			DDX_Control(pDX, IDC_VOLUME_PER6, m_percent[5]);
			DDX_Control(pDX, IDC_VOLUME_PER7, m_percent[6]);
			DDX_Control(pDX, IDC_VOLUME_PER8, m_percent[7]);
			DDX_Control(pDX, IDC_VOLUME_PER9, m_percent[8]);
			DDX_Control(pDX, IDC_VOLUME_PER10, m_percent[9]);
			DDX_Control(pDX, IDC_VOLUME_PER11, m_percent[10]);
			DDX_Control(pDX, IDC_VOLUME_PER12, m_percent[11]);
			DDX_Control(pDX, IDC_VOLUME_PER13, m_percent[12]);
			DDX_Control(pDX, IDC_VOLUME_PER14, m_percent[13]);
			DDX_Control(pDX, IDC_VOLUME_PER15, m_percent[14]);
			DDX_Control(pDX, IDC_VOLUME_PER16, m_percent[15]);
			DDX_Control(pDX, IDC_EDIT1, m_Label[0]);
			DDX_Control(pDX, IDC_EDIT2, m_Label[1]);
			DDX_Control(pDX, IDC_EDIT3, m_Label[2]);
			DDX_Control(pDX, IDC_EDIT4, m_Label[3]);
			DDX_Control(pDX, IDC_EDIT5, m_Label[4]);
			DDX_Control(pDX, IDC_EDIT6, m_Label[5]);
			DDX_Control(pDX, IDC_EDIT7, m_Label[6]);
			DDX_Control(pDX, IDC_EDIT8, m_Label[7]);
			DDX_Control(pDX, IDC_EDIT9, m_Label[8]);
			DDX_Control(pDX, IDC_EDIT10, m_Label[9]);
			DDX_Control(pDX, IDC_EDIT11, m_Label[10]);
			DDX_Control(pDX, IDC_EDIT12, m_Label[11]);
			DDX_Control(pDX, IDC_EDIT13, m_Label[12]);
			DDX_Control(pDX, IDC_EDIT14, m_Label[13]);
			DDX_Control(pDX, IDC_EDIT15, m_Label[14]);
			DDX_Control(pDX, IDC_EDIT16, m_Label[15]);


			//}}AFX_DATA_MAP
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