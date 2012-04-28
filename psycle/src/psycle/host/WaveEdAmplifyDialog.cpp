///\file
///\brief implementation file for psycle::host::CWaveEdAmplifyDialog.

#include <psycle/host/detail/project.private.hpp>
#include "WaveEdAmplifyDialog.hpp"

namespace psycle { namespace host {

		CWaveEdAmplifyDialog::CWaveEdAmplifyDialog(CWnd* pParent)
			: CDialog(CWaveEdAmplifyDialog::IDD, pParent)
		{
		}

		void CWaveEdAmplifyDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_EDIT2, m_dbedit);
			DDX_Control(pDX, IDC_SLIDER3, m_slider);
		}

		BEGIN_MESSAGE_MAP(CWaveEdAmplifyDialog, CDialog)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider)
		END_MESSAGE_MAP()

		BOOL CWaveEdAmplifyDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_slider.SetRange(0, 14400); 	// Don't use (-,+) range. It fucks up with the "0"
			m_slider.SetPos(9600);
			return TRUE;
		}

		void CWaveEdAmplifyDialog::OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			char tmp[10];
			float db;
			db =  (float) (m_slider.GetPos()-9600)*0.01f;
			_gcvt(db ,4 ,tmp);
			if (tmp[strlen(tmp) -1] == '.') tmp[strlen(tmp) -1] = 0; //Remove dot if last.
			
			m_dbedit.SetWindowText(tmp);
			*pResult = 0;
		}

		void CWaveEdAmplifyDialog::OnOK() 
		{
			
			char db_t[10];
			int db_i = 0;		
			m_dbedit.GetWindowText(db_t,9);
			db_i = (int)(100*atof(db_t));
			if (db_i) EndDialog( db_i );
			else EndDialog( AMP_DIALOG_CANCEL);
			//	CDialog::OnOK();
		}

		void CWaveEdAmplifyDialog::OnCancel() 
		{
			EndDialog( AMP_DIALOG_CANCEL );
			//CDialog::OnCancel();
		}

	}   // namespace
}   // namespace
