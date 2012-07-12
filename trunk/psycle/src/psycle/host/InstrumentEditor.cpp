///\file
///\brief implementation file for psycle::host::CInstrumentEditor.
#include <psycle/host/detail/project.private.hpp>
#include "InstrumentEditor.hpp"
#include "XMInstrument.hpp"
#include "MainFrm.hpp"
#include "EnvDialog.hpp"
#include "Song.hpp"
namespace psycle { namespace host {

		CInstrumentEditor::CInstrumentEditor(CWnd* pParent)
			: CDialog(CInstrumentEditor::IDD, pParent)
		{
		}

		void CInstrumentEditor::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_NOTETUNE, m_notelabel);
			DDX_Control(pDX, IDC_PANLABEL, m_panlabel);
			DDX_Control(pDX, IDC_VOLABEL2, m_finelabel);
			DDX_Control(pDX, IDC_SLIDER2, m_finetune);
			DDX_Control(pDX, IDC_LOOPEDIT, m_loopedit);
			DDX_Control(pDX, IDC_CHECK4, m_loopcheck);
			DDX_Control(pDX, IDC_RRES, m_rres_check);
			DDX_Control(pDX, IDC_PANSLIDER, m_panslider);			
			DDX_Control(pDX, IDC_LOCKINST, m_lockinst);
			DDX_Control(pDX, IDC_RPAN, m_rpan_check);
			DDX_Control(pDX, IDC_RCUT, m_rcut_check);
			DDX_Control(pDX, IDC_NNA_COMBO, m_nna_combo);			
			DDX_Control(pDX, IDC_LOCKINSTNUMBER, m_lockinstnumber);
			DDX_Control(pDX, IDC_INSTNAME, m_instname);
			DDX_Control(pDX, IDC_VOLABEL, m_volabel);
			DDX_Control(pDX, IDC_SLIDER1, m_volumebar);
			DDX_Control(pDX, IDC_WAVELENGTH, m_wlen);
			DDX_Control(pDX, IDC_LOOPSTART, m_loopstart);
			DDX_Control(pDX, IDC_LOOPEND, m_loopend);
			DDX_Control(pDX, IDC_LOOPTYPE, m_looptype);
			DDX_Control(pDX, IDC_INSTNUMBER, m_instlabel);
		}

		BEGIN_MESSAGE_MAP(CInstrumentEditor, CDialog)
			ON_BN_CLICKED(IDC_LOOPOFF, OnLoopoff)
			ON_BN_CLICKED(IDC_LOOPFORWARD, OnLoopforward)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)			
			ON_EN_CHANGE(IDC_LOCKINSTNUMBER, OnChangeLockInst)
			ON_EN_CHANGE(IDC_INSTNAME, OnChangeInstname)
			ON_CBN_SELCHANGE(IDC_NNA_COMBO, OnSelchangeNnaCombo)
			ON_BN_CLICKED(IDC_PREV_INSTRUMENT, OnPrevInstrument)
			ON_BN_CLICKED(IDC_NEXT_INSTRUMENT, OnNextInstrument)
			ON_BN_CLICKED(IDC_ENVEL_INSTRUMENT, OnEnvButton)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_PANSLIDER, OnCustomdrawPanslider)			
			ON_BN_CLICKED(IDC_LOCKINST, OnLockinst)
			ON_BN_CLICKED(IDC_RPAN, OnRpan)
			ON_BN_CLICKED(IDC_RCUT, OnRcut)
			ON_BN_CLICKED(IDC_RRES, OnRres)
			ON_BN_CLICKED(IDC_CHECK4, OnLoopCheck)
			ON_EN_CHANGE(IDC_LOOPEDIT, OnChangeLoopedit)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
			ON_BN_CLICKED(IDC_KILL_INSTRUMENT, OnKillInstrument)
			ON_BN_CLICKED(IDC_INS_DECOCTAVE, OnInsDecoctave)
			ON_BN_CLICKED(IDC_INS_DECNOTE, OnInsDecnote)
			ON_BN_CLICKED(IDC_INS_INCNOTE, OnInsIncnote)
			ON_BN_CLICKED(IDC_INS_INCOCTAVE, OnInsIncoctave)
		END_MESSAGE_MAP()

		BOOL CInstrumentEditor::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			cando=false;
			
			m_volumebar.SetRange(0,512);
			m_finetune.SetRange(0,512);	// Don't use (-,+) range. It fucks up with the "0"

			m_instname.SetLimitText(31);
			
			m_panslider.SetRange(0,256);
			
			m_nna_combo.AddString("Note Cut");
			m_nna_combo.AddString("Note Release");
			m_nna_combo.AddString("None");

			WaveUpdate();
			cando=true;
			return TRUE;
		}

		//////////////////////////////////////////////////////////////////////
		// Auxiliar members

		void CInstrumentEditor::WaveUpdate()
		{
			const int si = pSong->instSelected;
			Instrument *pins = pSong->_pInstrument[si];
			XMInstrument::WaveData wavetmp;
			XMInstrument::WaveData& wave = (pSong->samples.IsEnabled(si)) ? pSong->samples[si] : wavetmp;

			char buffer[64];
			// Set instrument current selected label
			sprintf(buffer, "%.2X", si);
			m_instlabel.SetWindowText(buffer);

			char buffer2[64];

			if (pins->_lock_instrument_to_machine < 0)
			{
				m_lockinstnumber.SetWindowText("");
			}
			else
			{
				sprintf(buffer2, "%.2X", pins->_lock_instrument_to_machine);
				m_lockinstnumber.SetWindowText(buffer2);
			}

			if (pins->_LOCKINST)
			{
				m_lockinst.SetCheck(BST_CHECKED);
				m_lockinstnumber.EnableWindow(true);
			}
			else
			{
				m_lockinst.SetCheck(BST_UNCHECKED);
				m_lockinstnumber.EnableWindow(false);
			}

			initializingDialog=true;
			// Set instrument current selected name
			m_instname.SetWindowText(pins->_sName);
			initializingDialog=false; // This prevents that "OnChangeInstname()", calls "UpdateComboIns()"

			UpdateCombo();

			m_panslider.SetPos(wave.PanFactor()*256);
			m_rpan_check.SetCheck(pins->_RPAN);
			m_rcut_check.SetCheck(pins->_RCUT);
			m_rres_check.SetCheck(pins->_RRES);
			
			sprintf(buffer,"%d",m_panslider.GetPos());
			m_panlabel.SetWindowText(buffer);
			

			bool const ils = pins->_loop;

			m_loopcheck.SetCheck(ils);
			sprintf(buffer,"%d",pins->_lines);
			m_loopedit.EnableWindow(ils);
			m_loopedit.SetWindowText(buffer);

			// Volume bar
			m_volumebar.SetPos(wave.WaveGlobVolume()*100);
			m_finetune.SetPos(wave.WaveFineTune()+256);

			UpdateNoteLabel();	
			
			
			// Set looptype
			if(wave.WaveLoopType() == XMInstrument::WaveData::LoopType::NORMAL)
			sprintf(buffer,"Forward");
			else
			sprintf(buffer,"Off");
			
			m_looptype.SetWindowText(buffer);

			// Display Loop Points & Wave Length
			
			sprintf(buffer,"%d",wave.WaveLoopStart());
			m_loopstart.SetWindowText(buffer);

			sprintf(buffer,"%d",wave.WaveLoopEnd());
			m_loopend.SetWindowText(buffer);

			sprintf(buffer,"%d",wave.WaveLength());
			m_wlen.SetWindowText(buffer);

		}

		//////////////////////////////////////////////////////////////////////
		// Loop setting GUI Handlers

		void CInstrumentEditor::OnLoopoff() 
		{
			int si = pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				if(wave.WaveLoopType() == XMInstrument::WaveData::LoopType::NORMAL)
				{
					wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
					WaveUpdate();
				}
			}
		}

		void CInstrumentEditor::OnLoopforward() 
		{
			int si=pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				if(wave.WaveLoopType() == XMInstrument::WaveData::LoopType::DO_NOT)
				{
					wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
					WaveUpdate();
				}
			}
		}

		//////////////////////////////////////////////////////////////////////
		// Volume bar handler

		void CInstrumentEditor::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=pSong->instSelected;
			char buffer[8];
			if (pSong->samples.IsEnabled(si)) {
				pSong->samples[si].WaveGlobVolume(m_volumebar.GetPos()*0.01f);
				
				sprintf(buffer,"%d%%",m_volumebar.GetPos());
				m_volabel.SetWindowText(buffer);
			}
			*pResult = 0;
		}

		void CInstrumentEditor::OnChangeLockInst()
		{
			char buffer[32];
			sprintf(buffer,"\0");
			m_lockinstnumber.GetWindowText(buffer, 16);

			int si = pSong->instSelected;
			
			using helpers::hexstring_to_integer;

			if (buffer[0] == '\0')
			{
				pSong->_pInstrument[si]->_lock_instrument_to_machine = -1;
			}
			else
			{
				int macNum = hexstring_to_integer(buffer);
				if (macNum >= MAX_BUSES)
					macNum = MAX_BUSES-1;
				else if (macNum < 0)
					macNum = 0;
				if ( ! pSong->_pMachine[macNum] )
					macNum = -1;
				pSong->_pInstrument[si]->_lock_instrument_to_machine = macNum;
			}
		}

		//////////////////////////////////////////////////////////////////////
		// Change instrument & layer name

		void CInstrumentEditor::OnChangeInstname() 
		{
			int si = pSong->instSelected;
			m_instname.GetWindowText(pSong->_pInstrument[si]->_sName, 32);
			if ( !initializingDialog ) 
			{
				pParentMain->UpdateComboIns();
				pParentMain->RedrawGearRackList();
			}
		}

		void CInstrumentEditor::OnSelchangeNnaCombo() 
		{
			pSong->_pInstrument[pSong->instSelected]->_NNA = m_nna_combo.GetCurSel();	
		}

		void CInstrumentEditor::UpdateCombo() 
		{
			switch(pSong->_pInstrument[pSong->instSelected]->_NNA)
			{
			case 0:m_nna_combo.SelectString(0,"Note Cut");break;
			case 1:m_nna_combo.SelectString(0,"Note Release");break;
			case 2:m_nna_combo.SelectString(0,"None");break;
			}
		}

		void CInstrumentEditor::OnPrevInstrument() 
		{
			const int si=pSong->instSelected;

			if(si>0)
			{
				pParentMain->ChangeIns(si-1);
			}

		}

		void CInstrumentEditor::OnNextInstrument() 
		{
			const int si=pSong->instSelected;

			if(si<254)
			{
				pParentMain->ChangeIns(si+1);
			}
		}

		void CInstrumentEditor::OnEnvButton() 
		{
			CEnvDialog dlg(*pSong);
			dlg.DoModal();
		}

		void CInstrumentEditor::OnCustomdrawPanslider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			char buffer[8];
			int si=pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				wave.PanFactor(m_panslider.GetPos()/256.f);
				
				sprintf(buffer,"%d%",m_panslider.GetPos());
				m_panlabel.SetWindowText(buffer);
			}
			*pResult = 0;
		}

		void CInstrumentEditor::OnLockinst()
		{
			int si = pSong->instSelected;
			if (m_lockinst.GetCheck())
			{
				pSong->_pInstrument[si]->_LOCKINST = true;				
			}
			else
			{
				pSong->_pInstrument[si]->_LOCKINST = false;				
			}

			if (pSong->_pInstrument[si]->_LOCKINST)
			{
				m_lockinstnumber.EnableWindow(true);
			}
			else
			{
				m_lockinstnumber.EnableWindow(false);
			}
		}

		void CInstrumentEditor::OnRpan() 
		{
			int si = pSong->instSelected;
			
			if (m_rpan_check.GetCheck())
			{
				pSong->_pInstrument[si]->_RPAN = true;
			}
			else
			{
				pSong->_pInstrument[si]->_RPAN = false;
			}
		}

		void CInstrumentEditor::OnRcut() 
		{
			int si=pSong->instSelected;
			
			if (m_rcut_check.GetCheck())
			{
				pSong->_pInstrument[si]->_RCUT = true;
			}
			else
			{
				pSong->_pInstrument[si]->_RCUT = false;
			}
		}

		void CInstrumentEditor::OnRres() 
		{
			int si=pSong->instSelected;
			
			if(m_rres_check.GetCheck())
			{
			pSong->_pInstrument[si]->_RRES = true;
			}
			else
			{
			pSong->_pInstrument[si]->_RRES = false;
			}
		}

		void CInstrumentEditor::Validate()
		{
			pParentMain=(CMainFrame*)GetParentFrame();
		}

		void CInstrumentEditor::OnLoopCheck() 
		{
			int si=pSong->instSelected;
			
			if(m_loopcheck.GetCheck())
			{
				pSong->_pInstrument[si]->_loop = true;
				m_loopedit.EnableWindow(true);
			}
			else
			{
				pSong->_pInstrument[si]->_loop = false;
				m_loopedit.EnableWindow(false);
			}
		}

		void CInstrumentEditor::OnChangeLoopedit() 
		{
			int si = pSong->instSelected;
			CString buffer;
			m_loopedit.GetWindowText(buffer);
			pSong->_pInstrument[si]->_lines = atoi(buffer);

			if (pSong->_pInstrument[si]->_lines < 1)
			{
				pSong->_pInstrument[si]->_lines = 1;
			}
		}

		void CInstrumentEditor::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=pSong->instSelected;
			char buffer[8];
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				
				if(cando)
					wave.WaveFineTune(m_finetune.GetPos()-256);
				
				sprintf(buffer,"%d",wave.WaveFineTune());
				m_finelabel.SetWindowText(buffer);
			}
			*pResult = 0;
		}

		void CInstrumentEditor::OnKillInstrument() 
		{
			{
				CExclusiveLock lock(&Global::song().semaphore, 2, true);
				pSong->DeleteInstrument(pSong->instSelected);
			}
			pParentMain->UpdateComboIns();
			pParentMain->WaveEditorBackUpdate();
			WaveUpdate();
			pParentMain->RedrawGearRackList();
		}

		void CInstrumentEditor::OnInsDecoctave() 
		{
			const int si=pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];

				if ( wave.WaveTune()>-37)
					wave.WaveTune(wave.WaveTune()-12);
				else wave.WaveTune(-48);
				UpdateNoteLabel();
			}
		}

		void CInstrumentEditor::OnInsDecnote() 
		{
			const int si=pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				if ( wave.WaveTune()>-47)
					wave.WaveTune(wave.WaveTune()-1);
				else wave.WaveTune(-48);
				UpdateNoteLabel();	
			}
		}

		void CInstrumentEditor::OnInsIncnote() 
		{
			const int si=pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				if ( wave.WaveTune()<70)
					wave.WaveTune(wave.WaveTune()+1);
				else wave.WaveTune(71);
				UpdateNoteLabel();	
			}
		}

		void CInstrumentEditor::OnInsIncoctave() 
		{
			const int si=pSong->instSelected;
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				if ( wave.WaveTune()<60)
					wave.WaveTune(wave.WaveTune()+12);
				else wave.WaveTune(71);
				UpdateNoteLabel();	
			}
		}

		void CInstrumentEditor::UpdateNoteLabel()
		{
			const int si = pSong->instSelected;
			char buffer[64];
			if (pSong->samples.IsEnabled(si)) {
				XMInstrument::WaveData& wave = pSong->samples[si];
				
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

	}   // namespace
}   // namespace
