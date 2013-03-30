///\file
///\brief implementation file for psycle::host::CEnvDialog.
#include <psycle/host/detail/project.private.hpp>
#include "EnvDialog.hpp"
#include "Song.hpp"

namespace psycle { namespace host {

		CEnvDialog::CEnvDialog(Song& song, CWnd* pParent)
			: CDialog(CEnvDialog::IDD, pParent)
			, thesong(song)
		{
		}

		void CEnvDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_A_A_LABEL7, m_envelope_label);
			DDX_Control(pDX, IDC_ENVELOPE, m_envelope_slider);
			DDX_Control(pDX, IDC_COMBO1, m_filtercombo);
			DDX_Control(pDX, IDC_LABELQ, m_q_label);
			DDX_Control(pDX, IDC_CUTOFF_LBL, m_cutoff_label);
			DDX_Control(pDX, IDC_SLIDER10, m_q_slider);
			DDX_Control(pDX, IDC_SLIDER9, m_cutoff_slider);
			DDX_Control(pDX, IDC_F_S_LABEL, m_f_s_label);
			DDX_Control(pDX, IDC_F_R_LABEL, m_f_r_label);
			DDX_Control(pDX, IDC_F_D_LABEL, m_f_d_label);
			DDX_Control(pDX, IDC_F_A_LABEL, m_f_a_label);
			DDX_Control(pDX, IDC_F_SLIDER4, m_f_release_slider);
			DDX_Control(pDX, IDC_F_SLIDER3, m_f_sustain_slider);
			DDX_Control(pDX, IDC_F_SLIDER2, m_f_decay_slider);
			DDX_Control(pDX, IDC_F_SLIDER1, m_f_attack_slider);
			DDX_Control(pDX, IDC_A_A_LABEL, m_a_a_label);
			DDX_Control(pDX, IDC_AMPFRAME2, m_filframe);
			DDX_Control(pDX, IDC_A_S_LABEL, m_a_s_label);
			DDX_Control(pDX, IDC_A_R_LABEL, m_a_r_label);
			DDX_Control(pDX, IDC_A_D_LABEL, m_a_d_label);
			DDX_Control(pDX, IDC_AMPFRAME, m_ampframe);
			DDX_Control(pDX, IDC_SLIDER4, m_a_release_slider);
			DDX_Control(pDX, IDC_SLIDER3, m_a_sustain_slider);
			DDX_Control(pDX, IDC_SLIDER2, m_a_decay_slider);
			DDX_Control(pDX, IDC_SLIDER1, m_a_attack_slider);
		}

		BEGIN_MESSAGE_MAP(CEnvDialog, CDialog)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnDrawAmpAttackSlider)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnDrawAmpDecaySlider)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnDrawAmpSustainSlider)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnDrawAmpReleaseSlider)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER1, OnCustomdrawFSlider1)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER2, OnCustomdrawFSlider2)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER3, OnCustomdrawFSlider3)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER4, OnCustomdrawFSlider4)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER9, OnCustomdrawSliderCutoff)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER10, OnCustomdrawSliderQ)
			ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_ENVELOPE, OnCustomdrawEnvelope)
		END_MESSAGE_MAP()

		BOOL CEnvDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			// Set slider ranges
			m_a_attack_slider.SetRange(1,65536);
			m_a_decay_slider.SetRange(1,65536);
			m_a_sustain_slider.SetRange(0,100);
			m_a_release_slider.SetRange(16,65536);

			m_f_attack_slider.SetRange(1,65536);
			m_f_decay_slider.SetRange(1,65536);
			m_f_sustain_slider.SetRange(0,128);
			m_f_release_slider.SetRange(16,65536);
				
			m_cutoff_slider.SetRange(0,127);
			m_q_slider.SetRange(0,127);
			
			m_envelope_slider.SetRange(-128,128);
			//Hack to fix "0 placed on leftmost on start".
			m_envelope_slider.SetPos(-128);
			
			m_filtercombo.AddString("LowPass");
			m_filtercombo.AddString("HiPass");
			m_filtercombo.AddString("BandPass");
			m_filtercombo.AddString("NotchBand");
			m_filtercombo.AddString("None");
			m_filtercombo.AddString("LowPass/IT");

			m_filtercombo.SetCurSel(pins->ENV_F_TP);

			// Update sliders
			m_a_attack_slider.SetPos(pins->ENV_AT);
			m_a_decay_slider.SetPos(pins->ENV_DT);
			m_a_sustain_slider.SetPos(pins->ENV_SL);
			m_a_release_slider.SetPos(pins->ENV_RT);

			m_f_attack_slider.SetPos(pins->ENV_F_AT);
			m_f_decay_slider.SetPos(pins->ENV_F_DT);
			m_f_sustain_slider.SetPos(pins->ENV_F_SL);
			m_f_release_slider.SetPos(pins->ENV_F_RT);
			
			m_cutoff_slider.SetPos(pins->ENV_F_CO);
			m_q_slider.SetPos(pins->ENV_F_RQ);
			m_envelope_slider.SetPos(pins->ENV_F_EA);
			
			return TRUE;
		}

		void CEnvDialog::OnDrawAmpAttackSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_AT = m_a_attack_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)pins->ENV_AT*0.0226757f);
			m_a_a_label.SetWindowText(buffer);

			// Update ADSR
			DrawADSR(
				pins->ENV_AT,
				pins->ENV_DT,
				pins->ENV_SL,
				pins->ENV_RT);

			*pResult = 0;

		}

		void CEnvDialog::OnDrawAmpDecaySlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_DT = m_a_decay_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)pins->ENV_DT*0.0226757f);
			m_a_d_label.SetWindowText(buffer);

			// Update ADSR
			DrawADSR(
				pins->ENV_AT,
				pins->ENV_DT,
				pins->ENV_SL,
				pins->ENV_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnDrawAmpSustainSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_SL = m_a_sustain_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%d%%",pins->ENV_SL);
			m_a_s_label.SetWindowText(buffer);

			// Update ADSR
			DrawADSR(
				pins->ENV_AT,
				pins->ENV_DT,
				pins->ENV_SL,
				pins->ENV_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnDrawAmpReleaseSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_RT = m_a_release_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)pins->ENV_RT*0.0226757f);
			m_a_r_label.SetWindowText(buffer);
			
			// Update ADSR
			DrawADSR(
					pins->ENV_AT,
					pins->ENV_DT,
					pins->ENV_SL,
					pins->ENV_RT);

			*pResult = 0;
		}

		//////////////////////////////////////////////////////////////////////
		// Filter envelope

		void CEnvDialog::OnCustomdrawFSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_F_AT = m_f_attack_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)pins->ENV_F_AT*0.0226757f);
			m_f_a_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(pins->ENV_F_AT,
						pins->ENV_F_DT,
						pins->ENV_F_SL,
						pins->ENV_F_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawFSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_F_DT=m_f_decay_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)pins->ENV_F_DT*0.0226757f);
			m_f_d_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(pins->ENV_F_AT,
						pins->ENV_F_DT,
						pins->ENV_F_SL,
						pins->ENV_F_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawFSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			char buffer[12];
			pins->ENV_F_SL=m_f_sustain_slider.GetPos();
			sprintf(buffer,"%.0f%%",pins->ENV_F_SL*0.78125);
			m_f_s_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(pins->ENV_F_AT,
						pins->ENV_F_DT,
						pins->ENV_F_SL,
						pins->ENV_F_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawFSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_F_RT = m_f_release_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)pins->ENV_F_RT*0.0226757f);
			m_f_r_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(pins->ENV_F_AT,
						pins->ENV_F_DT,
						pins->ENV_F_SL,
						pins->ENV_F_RT);
			
			*pResult = 0;
		}


		//////////////////////////////////////////////////////////////////////
		// GDI Envelope Drawing Members

		void CEnvDialog::DrawADSR(int AX,int BX,int CX,int DX)
		{
			CClientDC dc(&m_ampframe);
			CRect rect;
			m_ampframe.GetClientRect(&rect);
			CPoint pol[5];
			int CH=rect.Height();
			int CW=rect.Width();

			int samplesperpixel = 65536*3/rect.Width();
			AX/=samplesperpixel;
			BX/=samplesperpixel;
			CX*=CH;
			CX/=100;
			DX/=samplesperpixel;

			//dc.FillSolidRect(0,0,494,CH+1,0xCC7788);
			dc.FillSolidRect(0,0,CW,CH,RGB(50, 50, 50));
			pol[0].x=0;
			pol[0].y=CH;

			pol[1].x=AX;
			pol[1].y=0;
			
			pol[2].x=AX+BX;
			pol[2].y=CH-CX;
			
			pol[3].x=AX+BX;
			pol[3].y=CH-CX;
			
			pol[4].x=AX+BX+DX;
			pol[4].y=CH;
			
			//CBrush newBrush(RGB(255, 128, 64));
			//dc.SelectObject(&newBrush);	

			dc.Polygon(&pol[0],5);

			dc.MoveTo(AX+BX,0);
			dc.LineTo(AX+BX,CH);
		}

		void CEnvDialog::DrawADSRFil(int AX,int BX,int CX,int DX)
		{
			CClientDC dc(&m_filframe);
			CRect rect;
			m_filframe.GetClientRect(&rect);
			CPoint pol[5];
			int CH=rect.Height()-1;
			int CW=rect.Width();

			int samplesperpixel = 65536*3/rect.Width();
			AX/=samplesperpixel;
			BX/=samplesperpixel;
			CX*=CH;
			CX/=128;
			DX/=samplesperpixel;


			//dc.FillSolidRect(0,0,494,CH+1,0x4422CC);
			dc.FillSolidRect(0,0,CW,CH+1,RGB(50, 50, 50));
			pol[0].x=0;
			pol[0].y=CH;

			pol[1].x=AX;
			pol[1].y=0;
			
			pol[2].x=AX+BX;
			pol[2].y=CH-CX;
			
			pol[3].x=AX+BX;
			pol[3].y=CH-CX;
			
			pol[4].x=AX+BX+DX;
			pol[4].y=CH;
			
			//CBrush newBrush(RGB(255, 128, 64));
			//dc.SelectObject(&newBrush);		

			dc.Polygon(&pol[0],5);

			dc.MoveTo(AX+BX,0);
			dc.LineTo(AX+BX,CH);
		}

		void CEnvDialog::OnCustomdrawSliderCutoff(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_F_CO = m_cutoff_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%d",pins->ENV_F_CO);
			m_cutoff_label.SetWindowText(buffer);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawSliderQ(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];

			pins->ENV_F_RQ = m_q_slider.GetPos();

			char buffer[12];
			sprintf(buffer,"%.0f%%",pins->ENV_F_RQ*0.78740);
			m_q_label.SetWindowText(buffer);

			*pResult = 0;
		}

		void CEnvDialog::OnSelchangeCombo1() 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];
			pins->ENV_F_TP = static_cast<dsp::FilterType>(m_filtercombo.GetCurSel());
		}

		void CEnvDialog::OnCustomdrawEnvelope(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			Instrument* pins = thesong._pInstrument[thesong.instSelected];
			pins->ENV_F_EA = m_envelope_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%.0f",(float)pins->ENV_F_EA*0.78125f);
			m_envelope_label.SetWindowText(buffer);
			
			*pResult = 0;
		}

	}   // namespace
}   // namespace
