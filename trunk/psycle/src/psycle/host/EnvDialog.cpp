///\file
///\brief implementation file for psycle::host::CEnvDialog.

#include "EnvDialog.hpp"
#include "Psycle.hpp"
#include "Song.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		CEnvDialog::CEnvDialog(CWnd* pParent)
			: CDialog(CEnvDialog::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CEnvDialog)
			//}}AFX_DATA_INIT
		}

		void CEnvDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CEnvDialog)
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
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CEnvDialog, CDialog)
			//{{AFX_MSG_MAP(CEnvDialog)
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
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CEnvDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			int si = _pSong->instSelected;

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
			
			m_envelope_slider.SetRange(0,256); // Don't use (-,+) range. It fucks up with the "0"
			
			m_filtercombo.AddString("LowPass");
			m_filtercombo.AddString("HiPass");
			m_filtercombo.AddString("BandPass");
			m_filtercombo.AddString("NotchBand");
			m_filtercombo.AddString("None");

			m_filtercombo.SetCurSel(_pSong->_pInstrument[si]->ENV_F_TP);

			// Update sliders
			m_a_attack_slider.SetPos(_pSong->_pInstrument[si]->ENV_AT);
			m_a_decay_slider.SetPos(_pSong->_pInstrument[si]->ENV_DT);
			m_a_sustain_slider.SetPos(_pSong->_pInstrument[si]->ENV_SL);
			m_a_release_slider.SetPos(_pSong->_pInstrument[si]->ENV_RT);

			m_f_attack_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_AT);
			m_f_decay_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_DT);
			m_f_sustain_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_SL);
			m_f_release_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_RT);
			
			m_cutoff_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_CO);
			m_q_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_RQ);
			m_envelope_slider.SetPos(_pSong->_pInstrument[si]->ENV_F_EA+128);
			
			return TRUE;
		}

		void CEnvDialog::OnDrawAmpAttackSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si = _pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_AT = m_a_attack_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)_pSong->_pInstrument[si]->ENV_AT*0.0226757f);
			m_a_a_label.SetWindowText(buffer);

			// Update ADSR
			DrawADSR(
				_pSong->_pInstrument[si]->ENV_AT,
				_pSong->_pInstrument[si]->ENV_DT,
				_pSong->_pInstrument[si]->ENV_SL,
				_pSong->_pInstrument[si]->ENV_RT);

			*pResult = 0;

		}

		void CEnvDialog::OnDrawAmpDecaySlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_DT = m_a_decay_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)_pSong->_pInstrument[si]->ENV_DT*0.0226757f);
			m_a_d_label.SetWindowText(buffer);

			// Update ADSR
			DrawADSR(
				_pSong->_pInstrument[si]->ENV_AT,
				_pSong->_pInstrument[si]->ENV_DT,
				_pSong->_pInstrument[si]->ENV_SL,
				_pSong->_pInstrument[si]->ENV_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnDrawAmpSustainSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_SL = m_a_sustain_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%d%%",_pSong->_pInstrument[si]->ENV_SL);
			m_a_s_label.SetWindowText(buffer);

			// Update ADSR
			DrawADSR(
				_pSong->_pInstrument[si]->ENV_AT,
				_pSong->_pInstrument[si]->ENV_DT,
				_pSong->_pInstrument[si]->ENV_SL,
				_pSong->_pInstrument[si]->ENV_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnDrawAmpReleaseSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_RT = m_a_release_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)_pSong->_pInstrument[si]->ENV_RT*0.0226757f);
			m_a_r_label.SetWindowText(buffer);
			
			// Update ADSR
			DrawADSR(
					_pSong->_pInstrument[si]->ENV_AT,
					_pSong->_pInstrument[si]->ENV_DT,
					_pSong->_pInstrument[si]->ENV_SL,
					_pSong->_pInstrument[si]->ENV_RT);

			*pResult = 0;
		}

		//////////////////////////////////////////////////////////////////////
		// Filter envelope

		void CEnvDialog::OnCustomdrawFSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_F_AT = m_f_attack_slider.GetPos();
			
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)_pSong->_pInstrument[si]->ENV_F_AT*0.0226757f);
			m_f_a_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(_pSong->_pInstrument[si]->ENV_F_AT,
						_pSong->_pInstrument[si]->ENV_F_DT,
						_pSong->_pInstrument[si]->ENV_F_SL,
						_pSong->_pInstrument[si]->ENV_F_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawFSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_F_DT=m_f_decay_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)_pSong->_pInstrument[si]->ENV_F_DT*0.0226757f);
			m_f_d_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(_pSong->_pInstrument[si]->ENV_F_AT,
						_pSong->_pInstrument[si]->ENV_F_DT,
						_pSong->_pInstrument[si]->ENV_F_SL,
						_pSong->_pInstrument[si]->ENV_F_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawFSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			char buffer[12];
			_pSong->_pInstrument[si]->ENV_F_SL=m_f_sustain_slider.GetPos();
			sprintf(buffer,"%.0f%%",_pSong->_pInstrument[si]->ENV_F_SL*0.78125);
			m_f_s_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(_pSong->_pInstrument[si]->ENV_F_AT,
						_pSong->_pInstrument[si]->ENV_F_DT,
						_pSong->_pInstrument[si]->ENV_F_SL,
						_pSong->_pInstrument[si]->ENV_F_RT);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawFSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_F_RT = m_f_release_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%.2f ms.",(float)_pSong->_pInstrument[si]->ENV_F_RT*0.0226757f);
			m_f_r_label.SetWindowText(buffer);

			// Update filter ADSR
			DrawADSRFil(_pSong->_pInstrument[si]->ENV_F_AT,
						_pSong->_pInstrument[si]->ENV_F_DT,
						_pSong->_pInstrument[si]->ENV_F_SL,
						_pSong->_pInstrument[si]->ENV_F_RT);
			
			*pResult = 0;
		}


		//////////////////////////////////////////////////////////////////////
		// GDI Envelope Drawing Members

		void CEnvDialog::DrawADSR(int AX,int BX,int CX,int DX)
		{
			AX/=420;
			BX/=420;
			DX/=420;

			int CH=100;
		//	int CW=512;

			CClientDC dc(&m_ampframe);
			CPoint pol[5];

			//dc.FillSolidRect(0,0,494,CH+1,0xCC7788);
			dc.FillSolidRect(0,0,494,CH+1,RGB(50, 50, 50));
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
			AX/=420;
			BX/=420;
			CX*=100;
			CX/=128;
			DX/=420;

			int CH=100;
		//	int CW=512;

			CClientDC dc(&m_filframe);
			CPoint pol[5];

			//dc.FillSolidRect(0,0,494,CH+1,0x4422CC);
			dc.FillSolidRect(0,0,494,CH+1,RGB(50, 50, 50));
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
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_F_CO = m_cutoff_slider.GetPos();
			char buffer[12];
			sprintf(buffer,"%d",_pSong->_pInstrument[si]->ENV_F_CO);
			m_cutoff_label.SetWindowText(buffer);

			*pResult = 0;
		}

		void CEnvDialog::OnCustomdrawSliderQ(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;

			_pSong->_pInstrument[si]->ENV_F_RQ = m_q_slider.GetPos();

			char buffer[12];
			sprintf(buffer,"%.0f%%",_pSong->_pInstrument[si]->ENV_F_RQ*0.78740);
			m_q_label.SetWindowText(buffer);

			*pResult = 0;
		}

		void CEnvDialog::OnSelchangeCombo1() 
		{
			int si=_pSong->instSelected;
			_pSong->_pInstrument[si]->ENV_F_TP = m_filtercombo.GetCurSel();
		}

		void CEnvDialog::OnCustomdrawEnvelope(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;
			_pSong->_pInstrument[si]->ENV_F_EA = m_envelope_slider.GetPos()-128;
			char buffer[12];
			sprintf(buffer,"%.0f",(float)_pSong->_pInstrument[si]->ENV_F_EA*0.78125f);
			m_envelope_label.SetWindowText(buffer);
			
			*pResult = 0;
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
