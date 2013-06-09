///\file
///\brief implementation file for psycle::host::CInstrumentEditor.
#include <psycle/host/detail/project.private.hpp>
#include "InstrumentEditor.hpp"
#include "XMInstrument.hpp"
#include "MainFrm.hpp"
#include "Song.hpp"
namespace psycle { namespace host {

IMPLEMENT_DYNAMIC(CInstrumentEditor, CPropertyPage)

		CInstrumentEditor::CInstrumentEditor()
		: CPropertyPage(CInstrumentEditor::IDD)
		{
		}

		CInstrumentEditor::~CInstrumentEditor()
		{
		}

		void CInstrumentEditor::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_NOTETUNE, m_notelabel);
			DDX_Control(pDX, IDC_SLIDERFINE, m_finetune);
			DDX_Control(pDX, IDC_LOOPEDIT, m_loopedit);
			DDX_Control(pDX, IDC_CHECK4, m_loopcheck);
			DDX_Control(pDX, IDC_RRES, m_rres_check);
			DDX_Control(pDX, IDC_PANSLIDER, m_panslider);			
			DDX_Control(pDX, IDC_LOCKINST, m_lockinst);
			DDX_Control(pDX, IDC_RPAN, m_rpan_check);
			DDX_Control(pDX, IDC_RCUT, m_rcut_check);
			DDX_Control(pDX, IDC_NNA_COMBO, m_nna_combo);			
			DDX_Control(pDX, IDC_LOCKINSTNUMBER, m_lockinstnumber);
			DDX_Control(pDX, IDC_SLIDERVOL, m_volumebar);
			DDX_Control(pDX, IDC_WAVELENGTH, m_wlen);
			DDX_Control(pDX, IDC_LOOPSTART, m_loopstart);
			DDX_Control(pDX, IDC_LOOPEND, m_loopend);
			DDX_Control(pDX, IDC_LOOPTYPE, m_looptype);
			DDX_Control(pDX, IDC_INSTRUMENT_LABEL, m_instlabel);
			DDX_Control(pDX, IDC_COMBOFILTER, m_filtercombo);
			DDX_Control(pDX, IDC_SLIDER_FRES, m_q_slider);
			DDX_Control(pDX, IDC_SLIDER_FCUT, m_cutoff_slider);
			DDX_Control(pDX, IDC_SLIDER_AMPREL, m_a_release_slider);
			DDX_Control(pDX, IDC_SLIDER_AMPSUS, m_a_sustain_slider);
			DDX_Control(pDX, IDC_SLIDER_AMPDEC, m_a_decay_slider);
			DDX_Control(pDX, IDC_SLIDER_AMPATT, m_a_attack_slider);
			DDX_Control(pDX, IDC_SLIDER_FILREL, m_f_release_slider);
			DDX_Control(pDX, IDC_SLIDER_FILSUS, m_f_sustain_slider);
			DDX_Control(pDX, IDC_SLIDER_FILDEC, m_f_decay_slider);
			DDX_Control(pDX, IDC_SLIDER_FILATT, m_f_attack_slider);
			DDX_Control(pDX, IDC_SLIDER_FILAMT, m_f_amount_slider);
			DDX_Control(pDX, IDC_AMPFRAME, m_ampframe);
			DDX_Control(pDX, IDC_FILFRAME, m_filframe);
		}

		BEGIN_MESSAGE_MAP(CInstrumentEditor, CDialog)
			ON_WM_HSCROLL()
			ON_BN_CLICKED(IDC_LOOPOFF, OnLoopoff)
			ON_BN_CLICKED(IDC_LOOPFORWARD, OnLoopforward)
			ON_EN_CHANGE(IDC_LOCKINSTNUMBER, OnChangeLockInst)
			ON_EN_CHANGE(IDC_SAMPLERATE, OnEnChangeSamplerate)
			ON_CBN_SELCHANGE(IDC_NNA_COMBO, OnSelchangeNnaCombo)
			ON_BN_CLICKED(IDC_INST_DECTEN, OnPrevTenInstrument)
			ON_BN_CLICKED(IDC_INST_DECONE, OnPrevInstrument)
			ON_BN_CLICKED(IDC_INST_ADDONE, OnNextInstrument)
			ON_BN_CLICKED(IDC_INST_ADDTEN, OnNextTenInstrument)
			ON_BN_CLICKED(IDC_LOCKINST, OnLockinst)
			ON_BN_CLICKED(IDC_RPAN, OnRpan)
			ON_BN_CLICKED(IDC_RCUT, OnRcut)
			ON_BN_CLICKED(IDC_RRES, OnRres)
			ON_BN_CLICKED(IDC_CHECK4, OnLoopCheck)
			ON_EN_CHANGE(IDC_LOOPEDIT, OnChangeLoopedit)
			ON_BN_CLICKED(IDC_KILL_INSTRUMENT, OnKillInstrument)
			ON_BN_CLICKED(IDC_INS_DECOCTAVE, OnInsDecoctave)
			ON_BN_CLICKED(IDC_INS_DECNOTE, OnInsDecnote)
			ON_BN_CLICKED(IDC_INS_INCNOTE, OnInsIncnote)
			ON_BN_CLICKED(IDC_INS_INCOCTAVE, OnInsIncoctave)
			ON_CBN_SELCHANGE(IDC_COMBOFILTER, OnSelchangeFilterType)
			ON_NOTIFY_RANGE(NM_CUSTOMDRAW, IDC_SLIDERVOL, IDC_SLIDER_FRES, OnCustomdrawSliderm)
		END_MESSAGE_MAP()

		
		BOOL CInstrumentEditor::PreTranslateMessage(MSG* pMsg) 
		{
			InstrumentEditorUI* parent = dynamic_cast<InstrumentEditorUI*>(GetParent());
			BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
			if (res == FALSE ) return CPropertyPage::PreTranslateMessage(pMsg);
			return res;
		}

		BOOL CInstrumentEditor::OnInitDialog() 
		{
			CPropertyPage::OnInitDialog();
		
			m_nna_combo.AddString("Note Cut");
			m_nna_combo.AddString("Note Release");
			m_nna_combo.AddString("None");

			m_filtercombo.AddString("LowPass");
			m_filtercombo.AddString("HiPass");
			m_filtercombo.AddString("BandPass");
			m_filtercombo.AddString("NotchBand");
			m_filtercombo.AddString("None");

			m_cutoff_slider.SetRange(0,127);
			m_q_slider.SetRange(0,127);

			m_volumebar.SetRange(0,400);
			m_finetune.SetRange(-100,100);
			//Hack to fix "0 placed on leftmost on start".
			m_finetune.SetPos(-100);
			m_panslider.SetRange(0,128);

			// Set slider ranges
			m_a_attack_slider.SetRange(1,1024);
			m_a_decay_slider.SetRange(1,1024);
			m_a_sustain_slider.SetRange(0,100);
			m_a_release_slider.SetRange(16,2048);

			m_f_attack_slider.SetRange(1,1024);
			m_f_decay_slider.SetRange(1,1024);
			m_f_sustain_slider.SetRange(0,128);
			m_f_release_slider.SetRange(16,2048);
				
			m_f_amount_slider.SetRange(-128,128);
			//Hack to fix "0 placed on leftmost on start".
			m_f_amount_slider.SetPos(-128);
			
			m_ampframe.freeform(false);
			m_ampframe.envelopeIdx(0);
			m_ampframe.canSustain(false);
			m_ampframe.canLoop(false);
			m_ampEnv.Clear();
			m_ampEnv.Append(0, 0);
			m_ampEnv.Append(1, 1);
			m_ampEnv.Append(2, 0);
			m_ampEnv.Append(3, 0);
			m_ampEnv.SustainBegin(2);
			m_ampEnv.SustainEnd(2);
			m_ampframe.Initialize(m_ampEnv);

			m_filframe.freeform(false);
			m_filframe.envelopeIdx(1);
			m_filframe.canSustain(false);
			m_filframe.canLoop(false);
			m_filEnv.Clear();
			m_filEnv.Append(0, 0);
			m_filEnv.Append(1, 1);
			m_filEnv.Append(2, 0);
			m_filEnv.Append(3, 0);
			m_filEnv.SustainBegin(2);
			m_filEnv.SustainEnd(2);
			m_filframe.Initialize(m_filEnv);

			return TRUE;
		}

		//////////////////////////////////////////////////////////////////////
		// Auxiliary members
		BOOL CInstrumentEditor::OnSetActive()
		{
			TRACE("in setActive\n");
			WaveUpdate();
			return CPropertyPage::OnSetActive();
		}
		void CInstrumentEditor::WaveUpdate()
		{
			const int si = Global::song().instSelected;
			Instrument *pins = Global::song()._pInstrument[si];
			XMInstrument::WaveData wavetmp;
			bool enabled = Global::song().samples.IsEnabled(si);
			const XMInstrument::WaveData& wave = (enabled) ? Global::song().samples[si] : wavetmp;

			char buffer[64];
			// Set instrument current selected label
			sprintf(buffer, enabled?"%.2X*:%s":"%.2X :%s", si, wave.WaveName().c_str());
			m_instlabel.SetWindowText(buffer);

			char buffer2[64];
			if (pins->_lock_instrument_to_machine < 0) {
				m_lockinstnumber.SetWindowText("");
			} else {
				sprintf(buffer2, "%.2X", pins->_lock_instrument_to_machine);
				m_lockinstnumber.SetWindowText(buffer2);
			}

			if (pins->_LOCKINST) {
				m_lockinst.SetCheck(BST_CHECKED);
				m_lockinstnumber.EnableWindow(true);
			} else {
				m_lockinst.SetCheck(BST_UNCHECKED);
				m_lockinstnumber.EnableWindow(false);
			}


			UpdateComboNNA();

			bool const ils = pins->_loop;
			m_loopcheck.SetCheck(ils);
			sprintf(buffer,"%d",pins->_lines);
			m_loopedit.EnableWindow(ils);
			m_loopedit.SetWindowText(buffer);

			m_rpan_check.SetCheck(pins->_RPAN);
			m_rcut_check.SetCheck(pins->_RCUT);
			m_rres_check.SetCheck(pins->_RRES);

			m_filtercombo.SetCurSel(pins->ENV_F_TP);
			m_cutoff_slider.SetPos(pins->ENV_F_CO);
			m_q_slider.SetPos(pins->ENV_F_RQ);


			// Volume bar
			m_volumebar.SetPos(wave.WaveGlobVolume()*100);
			m_finetune.SetPos(wave.WaveFineTune());
			m_panslider.SetPos(wave.PanFactor()*128);

			UpdateNoteLabel();	
			
			// Set looptype
			if(wave.WaveLoopType() == XMInstrument::WaveData::LoopType::NORMAL) sprintf(buffer,"Forward");
			else sprintf(buffer,"Off");
			
			m_looptype.SetWindowText(buffer);

			// Display Loop Points & Wave Length
			sprintf(buffer,"%d",wave.WaveLoopStart());
			m_loopstart.SetWindowText(buffer);

			sprintf(buffer,"%d",wave.WaveLoopEnd());
			m_loopend.SetWindowText(buffer);

			sprintf(buffer,"%d",wave.WaveLength());
			m_wlen.SetWindowText(buffer);


			initializingDialog=true;
			CEdit* cedit = (CEdit*)GetDlgItem(IDC_SAMPLERATE);
			sprintf(buffer,"%d",wave.WaveSampleRate());
			cedit->SetWindowText(buffer);
			initializingDialog=false;


			RefreshEnvelopes();
		}

		void CInstrumentEditor::RefreshEnvelopes()
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			// Update sliders
			m_a_attack_slider.SetPos(pins->ENV_AT/256);//Reduced original range from samples to 5 milliseconds
			m_a_decay_slider.SetPos(pins->ENV_DT/256);//Reduced original range from samples to 5 milliseconds
			m_a_sustain_slider.SetPos(pins->ENV_SL);
			m_a_release_slider.SetPos(pins->ENV_RT/256);//Reduced original range from samples to 5 milliseconds

			m_f_attack_slider.SetPos(pins->ENV_F_AT/256);//Reduced original range from samples to 5 milliseconds
			m_f_decay_slider.SetPos(pins->ENV_F_DT/256);//Reduced original range from samples to 5 milliseconds
			m_f_sustain_slider.SetPos(pins->ENV_F_SL);
			m_f_release_slider.SetPos(pins->ENV_F_RT/256);//Reduced original range from samples to 5 milliseconds
			
			m_f_amount_slider.SetPos(pins->ENV_F_EA);
		}

		void CInstrumentEditor::OnEnvelopeChanged(WPARAM wParam, LPARAM lParam)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			int envIdx = wParam;
			if (envIdx == 0) { //amp
				pins->ENV_AT = m_ampframe.AttackTime()*256;
				pins->ENV_DT = m_ampframe.DecayTime()*256;
				pins->ENV_SL = m_ampframe.SustainValue()*100.f;
				pins->ENV_RT = m_ampframe.ReleaseTime()*256;
			}
			else { //filter
				pins->ENV_F_AT = m_filframe.AttackTime()*256;
				pins->ENV_F_DT = m_filframe.DecayTime()*256;
				pins->ENV_F_SL = m_filframe.SustainValue()*128.f;
				pins->ENV_F_RT = m_filframe.ReleaseTime()*256;
			}
			RefreshEnvelopes();
		}

		//////////////////////////////////////////////////////////////////////
		// Sliders handler
		void CInstrumentEditor::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
		{
			CSliderCtrl* the_slider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
			int uId = the_slider->GetDlgCtrlID();

			switch(nSBCode){
			case TB_BOTTOM: //fallthrough
			case TB_LINEDOWN: //fallthrough
			case TB_PAGEDOWN: //fallthrough
			case TB_TOP: //fallthrough
			case TB_LINEUP: //fallthrough
			case TB_PAGEUP: //fallthrough
			case TB_THUMBPOSITION: //fallthrough
			case TB_THUMBTRACK:
				if (uId == IDC_SLIDER_FCUT) { SliderFilterCut(*the_slider); }
				else if (uId == IDC_SLIDER_FRES) { SliderFilterRes(*the_slider); }
				else if (uId == IDC_SLIDERVOL) { SliderVolume(*the_slider); }
				else if (uId == IDC_PANSLIDER) { SliderPan(*the_slider); }
				else if (uId == IDC_SLIDERFINE) { SliderFinetune(*the_slider); }
				else if (uId == IDC_SLIDER_AMPATT) { SliderAmpAtt(*the_slider); }
				else if (uId == IDC_SLIDER_AMPDEC) { SliderAmpDec(*the_slider); }
				else if (uId == IDC_SLIDER_AMPSUS) { SliderAmpSus(*the_slider); }
				else if (uId == IDC_SLIDER_AMPREL) { SliderAmpRel(*the_slider); }
				else if (uId == IDC_SLIDER_FILATT) { SliderFilterAtt(*the_slider); }
				else if (uId == IDC_SLIDER_FILDEC) { SliderFilterDec(*the_slider); }
				else if (uId == IDC_SLIDER_FILSUS) { SliderFilterSus(*the_slider); }
				else if (uId == IDC_SLIDER_FILREL) { SliderFilterRel(*the_slider); }
				else if (uId == IDC_SLIDER_FILAMT) { SliderFilterMod(*the_slider); }
				break;
			case TB_ENDTRACK:
				break;
			}
			CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
		}

		void CInstrumentEditor::SliderFilterCut(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_CO = the_slider.GetPos();
		}
		void CInstrumentEditor::SliderFilterRes(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_RQ = the_slider.GetPos();
		}
		void CInstrumentEditor::SliderVolume(CSliderCtrl& the_slider)
		{
			int si = Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				Global::song().samples.get(si).WaveGlobVolume(the_slider.GetPos()*0.01f);
			}
		}
		void CInstrumentEditor::SliderPan(CSliderCtrl& the_slider)
		{
			int si = Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				 Global::song().samples.get(si).PanFactor(the_slider.GetPos()/128.f);
			}
		}
		void CInstrumentEditor::SliderFinetune(CSliderCtrl& the_slider)
		{
			int si = Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				 Global::song().samples.get(si).WaveFineTune(the_slider.GetPos());
			}
		}
		void CInstrumentEditor::SliderAmpAtt(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_AT = the_slider.GetPos()*256;//Reduced original range from samples to 5 milliseconds
		}
		void CInstrumentEditor::SliderAmpDec(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_DT = the_slider.GetPos()*256;//Reduced original range from samples to 5 milliseconds
		}
		void CInstrumentEditor::SliderAmpSus(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_SL = the_slider.GetPos();
		}
		void CInstrumentEditor::SliderAmpRel(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_RT = the_slider.GetPos()*256;//Reduced original range from samples to 5 milliseconds
		}
		void CInstrumentEditor::SliderFilterAtt(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_AT = the_slider.GetPos()*256;//Reduced original range from samples to 5 milliseconds
		}
		void CInstrumentEditor::SliderFilterDec(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_DT = the_slider.GetPos()*256;//Reduced original range from samples to 5 milliseconds
		}
		void CInstrumentEditor::SliderFilterSus(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_SL = the_slider.GetPos();
		}
		void CInstrumentEditor::SliderFilterRel(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_RT = the_slider.GetPos()*256;//Reduced original range from samples to 5 milliseconds
		}
		void CInstrumentEditor::SliderFilterMod(CSliderCtrl& the_slider)
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_EA = the_slider.GetPos();
		}


		//////////////////////////////////////////////////////////////////////
		// GUI Handlers
		void CInstrumentEditor::OnPrevTenInstrument() 
		{
			const int si=Global::song().instSelected;
			if(si>9)
			{
				CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
				win->ChangeIns(si-10);
			}
		}

		void CInstrumentEditor::OnPrevInstrument() 
		{
			const int si=Global::song().instSelected;
			if(si>0)
			{
				CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
				win->ChangeIns(si-1);
			}
		}

		void CInstrumentEditor::OnNextInstrument() 
		{
			const int si=Global::song().instSelected;
			if(si<254)
			{
				CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
				win->ChangeIns(si+1);
			}
		}
		void CInstrumentEditor::OnNextTenInstrument() 
		{
			const int si=Global::song().instSelected;
			if(si<245)
			{
				CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
				win->ChangeIns(si+10);
			}
		}
		
		void CInstrumentEditor::OnKillInstrument() 
		{
			{
				CExclusiveLock lock(&Global::song().semaphore, 2, true);
				Global::song().DeleteInstrument(Global::song().instSelected);
			}
			CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
			win->UpdateComboIns();
			win->WaveEditorBackUpdate();
			WaveUpdate();
			win->RedrawGearRackList();
		}


		void CInstrumentEditor::OnLockinst()
		{
			int si = Global::song().instSelected;
			Global::song()._pInstrument[si]->_LOCKINST = static_cast<bool>(m_lockinst.GetCheck());
			m_lockinstnumber.EnableWindow(Global::song()._pInstrument[si]->_LOCKINST);
		}
		void CInstrumentEditor::OnChangeLockInst()
		{
			int si = Global::song().instSelected;
			char buffer[32];
			sprintf(buffer,"\0");
			m_lockinstnumber.GetWindowText(buffer, 16);
			if (buffer[0] == '\0') {
				Global::song()._pInstrument[si]->_lock_instrument_to_machine = -1;
			} else {
				int macNum = helpers::hexstring_to_integer(buffer);
				if (macNum >= MAX_BUSES)
					macNum = MAX_BUSES-1;
				else if (macNum < 0)
					macNum = 0;
				if ( ! Global::song()._pMachine[macNum] )
					macNum = -1;
				Global::song()._pInstrument[si]->_lock_instrument_to_machine = macNum;
			}
		}

		void CInstrumentEditor::OnSelchangeNnaCombo() 
		{
			Global::song()._pInstrument[Global::song().instSelected]->_NNA = m_nna_combo.GetCurSel();	
		}

		void CInstrumentEditor::OnLoopCheck() 
		{
			int si=Global::song().instSelected;
			bool looped = static_cast<bool>(m_loopcheck.GetCheck());
			Global::song()._pInstrument[si]->_loop = looped;
			m_loopedit.EnableWindow(looped);
		}

		void CInstrumentEditor::OnChangeLoopedit() 
		{
			int si = Global::song().instSelected;
			CString buffer;
			m_loopedit.GetWindowText(buffer);
			Global::song()._pInstrument[si]->_lines = atoi(buffer);
			if (Global::song()._pInstrument[si]->_lines < 1)
			{
				Global::song()._pInstrument[si]->_lines = 1;
			}
		}

		void CInstrumentEditor::OnRpan() 
		{
			int si = Global::song().instSelected;
			Global::song()._pInstrument[si]->_RPAN = static_cast<bool>(m_rpan_check.GetCheck());
		}

		void CInstrumentEditor::OnRcut() 
		{
			int si=Global::song().instSelected;
			Global::song()._pInstrument[si]->_RCUT = static_cast<bool>(m_rcut_check.GetCheck());
		}

		void CInstrumentEditor::OnRres() 
		{
			int si=Global::song().instSelected;
			
			Global::song()._pInstrument[si]->_RRES = static_cast<bool>(m_rres_check.GetCheck());
		}


		void CInstrumentEditor::OnSelchangeFilterType() 
		{
			Instrument* pins = Global::song()._pInstrument[Global::song().instSelected];
			pins->ENV_F_TP = static_cast<dsp::FilterType>(m_filtercombo.GetCurSel());
		}

		void CInstrumentEditor::OnInsDecoctave() 
		{
			const int si=Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);

				if ( wave.WaveTune()>-37)
					wave.WaveTune(wave.WaveTune()-12);
				else wave.WaveTune(-48);
				UpdateNoteLabel();
			}
		}

		void CInstrumentEditor::OnInsDecnote() 
		{
			const int si=Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);
				if ( wave.WaveTune()>-47)
					wave.WaveTune(wave.WaveTune()-1);
				else wave.WaveTune(-48);
				UpdateNoteLabel();	
			}
		}

		void CInstrumentEditor::OnInsIncnote() 
		{
			const int si=Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);
				if ( wave.WaveTune()<70)
					wave.WaveTune(wave.WaveTune()+1);
				else wave.WaveTune(71);
				UpdateNoteLabel();	
			}
		}

		void CInstrumentEditor::OnInsIncoctave() 
		{
			const int si=Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);
				if ( wave.WaveTune()<60)
					wave.WaveTune(wave.WaveTune()+12);
				else wave.WaveTune(71);
				UpdateNoteLabel();	
			}
		}

		void CInstrumentEditor::OnLoopoff() 
		{
			int si = Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);
				if(wave.WaveLoopType() == XMInstrument::WaveData::LoopType::NORMAL)
				{
					wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
					WaveUpdate();
				}
			}
		}

		void CInstrumentEditor::OnLoopforward() 
		{
			int si=Global::song().instSelected;
			if (Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);
				if(wave.WaveLoopType() == XMInstrument::WaveData::LoopType::DO_NOT)
				{
					wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
					WaveUpdate();
				}
			}
		}
		void CInstrumentEditor::OnEnChangeSamplerate()
		{
			int si=Global::song().instSelected;
			char tmp[40];
			if ( !initializingDialog  && Global::song().samples.Exists(si)) {
				XMInstrument::WaveData& wave = Global::song().samples.get(si);
				CEdit* cedit = (CEdit*)GetDlgItem(IDC_SAMPLERATE);
				cedit->GetWindowText(tmp,40);
				int i = atoi(tmp);
				if (i==0) i=44100;
				wave.WaveSampleRate(i);
			}
		}
		
		void CInstrumentEditor::UpdateComboNNA() 
		{
			switch(Global::song()._pInstrument[Global::song().instSelected]->_NNA)
			{
			case 0:m_nna_combo.SelectString(0,"Note Cut");break;
			case 1:m_nna_combo.SelectString(0,"Note Release");break;
			case 2:m_nna_combo.SelectString(0,"None");break;
			}
		}

		void CInstrumentEditor::UpdateNoteLabel()
		{
			const int si = Global::song().instSelected;
			char buffer[64];
			if (Global::song().samples.Exists(si)) {
				const XMInstrument::WaveData& wave = Global::song().samples[si];
				
				const int octave= ((wave.WaveTune()+48)/12);
				switch ((wave.WaveTune()+48)%12)
				{
				case 0:  sprintf(buffer,"C-%i",octave);break;
				case 1:  sprintf(buffer,"C#%i",octave);break;
				case 2:  sprintf(buffer,"D-%i",octave);break;
				case 3:  sprintf(buffer,"D#%i",octave);break;
				case 4:  sprintf(buffer,"E-%i",octave);break;
				case 5:  sprintf(buffer,"F-%i",octave);break;
				case 6:  sprintf(buffer,"F#%i",octave);break;
				case 7:  sprintf(buffer,"G-%i",octave);break;
				case 8:  sprintf(buffer,"G#%i",octave);break;
				case 9:  sprintf(buffer,"A-%i",octave);break;
				case 10:  sprintf(buffer,"A#%i",octave);break;
				case 11:  sprintf(buffer,"B-%i",octave);break;
				}
				m_notelabel.SetWindowText(buffer);
			}
		}

		void CInstrumentEditor::OnCustomdrawSliderm(UINT idx, NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NMCUSTOMDRAW nmcd = *reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
			if (nmcd.dwDrawStage == CDDS_POSTPAINT)
			{
				char buffer[64];
				int label = 0;
				CSliderCtrl* slider = reinterpret_cast<CSliderCtrl*>(GetDlgItem(pNMHDR->idFrom));
				int si=Global::song().instSelected;
				Instrument *pins = Global::song()._pInstrument[si];
				int nPos = slider->GetPos();
				if (pNMHDR->idFrom == IDC_SLIDER_FCUT) {
					sprintf(buffer,"%.0f",2333.0*pow(6.0,nPos/127.0));
					label = IDC_CUTOFF_LBL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_FRES) {
					sprintf(buffer,"%.0f%%",nPos*0.78740);
					label = IDC_LABELQ;
				}
				else if (pNMHDR->idFrom == IDC_SLIDERVOL) {
					if (Global::song().samples.Exists(si)) {
						if(m_volumebar.GetPos()==0) {
							sprintf(buffer,"-inf dB");
						} else {
							float db = 20 * log10(nPos*0.01f);
							sprintf(buffer,"%.1f dB",db);
						}
					}
					else {
						buffer[0]='\0';
					}
					label = IDC_VOLABEL;
				}
				else if (pNMHDR->idFrom == IDC_PANSLIDER) {
					if (Global::song().samples.Exists(si)) {
						if ( nPos == 0) sprintf(buffer,"||%02d  ",nPos);
						else if ( nPos < 32) sprintf(buffer,"<<%02d  ",nPos);
						else if ( nPos < 64) sprintf(buffer," <%02d< ",nPos);
						else if ( nPos == 64) sprintf(buffer," |%02d| ",nPos);
						else if ( nPos <= 96) sprintf(buffer," >%02d> ",nPos);
						else if (nPos < 128) sprintf(buffer,"  %02d>>",nPos);
						else sprintf(buffer,"  %02d||",nPos);
					}
					else {
						buffer[0]='\0';
					}
					label = IDC_PANLABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDERFINE) {
					if (Global::song().samples.Exists(si)) {
						sprintf(buffer,"%d ct.",m_finetune.GetPos());
					}
					else {
						buffer[0]='\0';
					}
					label = IDC_FINELABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_AMPATT) {
					m_ampframe.AttackTime(pins->ENV_AT/256);
					m_ampframe.Invalidate();
					sprintf(buffer,"%.2f ms.",nPos*0.0226757f);
					label = IDC_A_A_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_AMPDEC) {
					m_ampframe.DecayTime(pins->ENV_DT/256);
					m_ampframe.Invalidate();
					sprintf(buffer,"%.2f ms.",nPos*5.8049792f);
					label = IDC_A_D_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_AMPSUS) {
					m_ampframe.SustainValue((float)pins->ENV_SL*0.01f);
					m_ampframe.Invalidate();
					sprintf(buffer,"%d%%",nPos);
					label = IDC_A_S_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_AMPREL) {
					m_ampframe.ReleaseTime(pins->ENV_RT/256);
					m_ampframe.Invalidate();
					sprintf(buffer,"%.2f ms.",nPos*5.8049792f);
					label = IDC_A_R_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_FILATT) {
					m_filframe.AttackTime(pins->ENV_F_AT/256);
					m_filframe.Invalidate();
					sprintf(buffer,"%.2f ms.",nPos*5.8049792f);
					label = IDC_F_A_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_FILDEC) {
					m_filframe.DecayTime(pins->ENV_F_DT/256);
					m_filframe.Invalidate();
					sprintf(buffer,"%.2f ms.",nPos*5.8049792f);
					label = IDC_F_D_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_FILSUS) {
					m_filframe.SustainValue((float)pins->ENV_F_SL*0.0078125f);
					m_filframe.Invalidate();
					sprintf(buffer,"%d%%",nPos);
					label = IDC_F_S_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_FILREL) {
					m_filframe.ReleaseTime(pins->ENV_F_RT/256);
					m_filframe.Invalidate();
					sprintf(buffer,"%.2f ms.",nPos*5.8049792);
					label = IDC_F_R_LABEL;
				}
				else if (pNMHDR->idFrom == IDC_SLIDER_FILAMT) {
					sprintf(buffer,"%.0f%%",nPos*0.78125f);
					label = IDC_F_AMOUNTLABEL;
				}
				if (label != 0) {
					((CStatic*)GetDlgItem(label))->SetWindowText(buffer);
				}
				*pResult = CDRF_DODEFAULT;
			}
			else if (nmcd.dwDrawStage == CDDS_PREPAINT ){
				*pResult = CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
			}
			else {
				*pResult = CDRF_DODEFAULT;
			}
		}
	}   // namespace
}   // namespace
