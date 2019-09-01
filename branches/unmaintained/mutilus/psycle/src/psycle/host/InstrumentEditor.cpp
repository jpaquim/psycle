///\file
///\brief implementation file for psycle::host::CInstrumentEditor.
#include <psycle/project.private.hpp>
#include "InstrumentEditor.hpp"
#include "Psycle.hpp"
#include "MainFrm.hpp"
#include "EnvDialog.hpp"
#include "constants.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		CInstrumentEditor::CInstrumentEditor(CWnd* pParent)
			: CDialog(CInstrumentEditor::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CInstrumentEditor)
			//}}AFX_DATA_INIT
		}

		void CInstrumentEditor::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CInstrumentEditor)
			DDX_Control(pDX, IDC_NOTETUNE, m_notelabel);
			DDX_Control(pDX, IDC_PANLABEL, m_panlabel);
			DDX_Control(pDX, IDC_VOLABEL2, m_finelabel);
			DDX_Control(pDX, IDC_SLIDER2, m_finetune);
			DDX_Control(pDX, IDC_LOOPEDIT, m_loopedit);
			DDX_Control(pDX, IDC_CHECK4, m_loopcheck);
			DDX_Control(pDX, IDC_RRES, m_rres_check);
			DDX_Control(pDX, IDC_PANSLIDER, m_panslider);
			DDX_Control(pDX, IDC_RPAN, m_rpan_check);
			DDX_Control(pDX, IDC_RCUT, m_rcut_check);
			DDX_Control(pDX, IDC_NNA_COMBO, m_nna_combo);
			DDX_Control(pDX, IDC_INSTNAME, m_instname);
			DDX_Control(pDX, IDC_VOLABEL, m_volabel);
			DDX_Control(pDX, IDC_SLIDER1, m_volumebar);
			DDX_Control(pDX, IDC_WAVELENGTH, m_wlen);
			DDX_Control(pDX, IDC_LOOPSTART, m_loopstart);
			DDX_Control(pDX, IDC_LOOPEND, m_loopend);
			DDX_Control(pDX, IDC_LOOPTYPE, m_looptype);
			DDX_Control(pDX, IDC_INSTNUMBER, m_instlabel);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CInstrumentEditor, CDialog)
			//{{AFX_MSG_MAP(CInstrumentEditor)
			ON_BN_CLICKED(IDC_LOOPOFF, OnLoopoff)
			ON_BN_CLICKED(IDC_LOOPFORWARD, OnLoopforward)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
			ON_EN_CHANGE(IDC_INSTNAME, OnChangeInstname)
			ON_CBN_SELCHANGE(IDC_NNA_COMBO, OnSelchangeNnaCombo)
			ON_BN_CLICKED(IDC_BUTTON12, OnPrevInstrument)
			ON_BN_CLICKED(IDC_BUTTON13, OnNextInstrument)
			ON_BN_CLICKED(IDC_BUTTON8, OnEnvButton)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_PANSLIDER, OnCustomdrawPanslider)
			ON_BN_CLICKED(IDC_RPAN, OnRpan)
			ON_BN_CLICKED(IDC_RCUT, OnRcut)
			ON_BN_CLICKED(IDC_RRES, OnRres)
			ON_BN_CLICKED(IDC_CHECK4, OnLoopCheck)
			ON_EN_CHANGE(IDC_LOOPEDIT, OnChangeLoopedit)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
			ON_BN_CLICKED(IDC_BUTTON7, OnKillInstrument)
			ON_BN_CLICKED(IDC_INS_DECOCTAVE, OnInsDecoctave)
			ON_BN_CLICKED(IDC_INS_DECNOTE, OnInsDecnote)
			ON_BN_CLICKED(IDC_INS_INCNOTE, OnInsIncnote)
			ON_BN_CLICKED(IDC_INS_INCOCTAVE, OnInsIncoctave)
			//}}AFX_MSG_MAP
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

		//	_pSong->waveSelected=0;

			WaveUpdate();
			cando=true;
			return TRUE;
		}

		//////////////////////////////////////////////////////////////////////
		// Auxiliar members

		void CInstrumentEditor::WaveUpdate()
		{
			const int si = _pSong->instSelected;

			char buffer[64];
			// Set instrument current selected label
			sprintf(buffer, "%.2X", si);
			m_instlabel.SetWindowText(buffer);

			initializingDialog=true;
			// Set instrument current selected name
			m_instname.SetWindowText(_pSong->_pInstrument[si]->_sName);
			initializingDialog=false; // This prevents that "OnChangeInstname()", calls "UpdateComboIns()"

			UpdateCombo();

			m_panslider.SetPos(_pSong->_pInstrument[si]->_pan);
			m_rpan_check.SetCheck(_pSong->_pInstrument[si]->_RPAN);
			m_rcut_check.SetCheck(_pSong->_pInstrument[si]->_RCUT);
			m_rres_check.SetCheck(_pSong->_pInstrument[si]->_RRES);
			
			sprintf(buffer,"%d",_pSong->_pInstrument[si]->_pan);
			m_panlabel.SetWindowText(buffer);
			

			bool const ils = _pSong->_pInstrument[si]->_loop;

			m_loopcheck.SetCheck(ils);
			sprintf(buffer,"%d",_pSong->_pInstrument[si]->_lines);
			m_loopedit.EnableWindow(ils);
			m_loopedit.SetWindowText(buffer);

			// Volume bar
			m_volumebar.SetPos(_pSong->_pInstrument[si]->waveVolume);
			m_finetune.SetPos(_pSong->_pInstrument[si]->waveFinetune+256);

			UpdateNoteLabel();	
			
			
			// Set looptype
			if(_pSong->_pInstrument[si]->waveLoopType)
			sprintf(buffer,"Forward");
			else
			sprintf(buffer,"Off");
			
			m_looptype.SetWindowText(buffer);

			// Display Loop Points & Wave Length
			
			sprintf(buffer,"%d",_pSong->_pInstrument[si]->waveLoopStart);
			m_loopstart.SetWindowText(buffer);

			sprintf(buffer,"%d",_pSong->_pInstrument[si]->waveLoopEnd);
			m_loopend.SetWindowText(buffer);

			sprintf(buffer,"%d",_pSong->_pInstrument[si]->waveLength);
			m_wlen.SetWindowText(buffer);

		}

		//////////////////////////////////////////////////////////////////////
		// Loop setting GUI Handlers

		void CInstrumentEditor::OnLoopoff() 
		{
		int si = _pSong->instSelected;

			if(_pSong->_pInstrument[si]->waveLoopType)
			{
			_pSong->_pInstrument[si]->waveLoopType=0;
			WaveUpdate();
			}
		}

		void CInstrumentEditor::OnLoopforward() 
		{
		int si=_pSong->instSelected;

			if(!_pSong->_pInstrument[si]->waveLoopType)
			{
			_pSong->_pInstrument[si]->waveLoopType=1;
			WaveUpdate();
			}
		}

		//////////////////////////////////////////////////////////////////////
		// Volume bar handler

		void CInstrumentEditor::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
		int si=_pSong->instSelected;
		char buffer[8];

			_pSong->_pInstrument[si]->waveVolume=m_volumebar.GetPos();
			
			sprintf(buffer,"%d%%",_pSong->_pInstrument[si]->waveVolume);
			m_volabel.SetWindowText(buffer);

			*pResult = 0;
		}

		//////////////////////////////////////////////////////////////////////
		// Change instrument & layer name

		void CInstrumentEditor::OnChangeInstname() 
		{
			int si = _pSong->instSelected;
			m_instname.GetWindowText(_pSong->_pInstrument[si]->_sName, 32);
			if ( !initializingDialog ) 
			{
				pParentMain->UpdateComboIns();
				pParentMain->RedrawGearRackList();
			}
		}


		void CInstrumentEditor::OnSelchangeNnaCombo() 
		{
			_pSong->_pInstrument[_pSong->instSelected]->_NNA = m_nna_combo.GetCurSel();	
		}

		void CInstrumentEditor::UpdateCombo() 
		{
			switch(_pSong->_pInstrument[_pSong->instSelected]->_NNA)
			{
			case 0:m_nna_combo.SelectString(0,"Note Cut");break;
			case 1:m_nna_combo.SelectString(0,"Note Release");break;
			case 2:m_nna_combo.SelectString(0,"None");break;
			}
		}

		void CInstrumentEditor::OnPrevInstrument() 
		{
			const int si=_pSong->instSelected;

			if(si>0)
			{
				pParentMain->ChangeIns(si-1);
			}

		}

		void CInstrumentEditor::OnNextInstrument() 
		{
			const int si=_pSong->instSelected;

			if(si<254)
			{
				pParentMain->ChangeIns(si+1);
			}
		}

		void CInstrumentEditor::OnEnvButton() 
		{
			CEnvDialog dlg;
			dlg._pSong=_pSong;
			dlg.DoModal();
		}

		void CInstrumentEditor::OnCustomdrawPanslider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			char buffer[8];
			int si=_pSong->instSelected;
			_pSong->_pInstrument[si]->_pan = m_panslider.GetPos();
			
			sprintf(buffer,"%d%",_pSong->_pInstrument[si]->_pan);
			m_panlabel.SetWindowText(buffer);
			*pResult = 0;
		}

		void CInstrumentEditor::OnRpan() 
		{
			int si = _pSong->instSelected;
			
			if (m_rpan_check.GetCheck())
			{
				_pSong->_pInstrument[si]->_RPAN = true;
			}
			else
			{
				_pSong->_pInstrument[si]->_RPAN = false;
			}
		}

		void CInstrumentEditor::OnRcut() 
		{
			int si=_pSong->instSelected;
			
			if (m_rcut_check.GetCheck())
			{
				_pSong->_pInstrument[si]->_RCUT = true;
			}
			else
			{
				_pSong->_pInstrument[si]->_RCUT = false;
			}
		}

		void CInstrumentEditor::OnRres() 
		{
			int si=_pSong->instSelected;
			
			if(m_rres_check.GetCheck())
			{
			_pSong->_pInstrument[si]->_RRES = true;
			}
			else
			{
			_pSong->_pInstrument[si]->_RRES = false;
			}
		}

		void CInstrumentEditor::Validate()
		{
			pParentMain=(CMainFrame*)GetParentFrame();
		}

		void CInstrumentEditor::OnLoopCheck() 
		{
			int si=_pSong->instSelected;
			
			if(m_loopcheck.GetCheck())
			{
				_pSong->_pInstrument[si]->_loop = true;
				m_loopedit.EnableWindow(true);
			}
			else
			{
				_pSong->_pInstrument[si]->_loop = false;
				m_loopedit.EnableWindow(false);
			}
		}

		void CInstrumentEditor::OnChangeLoopedit() 
		{
			int si = _pSong->instSelected;
			CString buffer;
			m_loopedit.GetWindowText(buffer);
			_pSong->_pInstrument[si]->_lines = atoi(buffer);

			if (_pSong->_pInstrument[si]->_lines < 1)
			{
				_pSong->_pInstrument[si]->_lines = 1;
			}
		}

		void CInstrumentEditor::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			int si=_pSong->instSelected;
			char buffer[8];
			
			if(cando)
				_pSong->_pInstrument[si]->waveFinetune=m_finetune.GetPos()-256;
			
			sprintf(buffer,"%d",_pSong->_pInstrument[si]->waveFinetune);
			m_finelabel.SetWindowText(buffer);
			
			*pResult = 0;
		}

		void CInstrumentEditor::OnKillInstrument() 
		{
			_pSong->DeleteInstrument(_pSong->instSelected);
			WaveUpdate();
			pParentMain->UpdateComboIns();
			pParentMain->RedrawGearRackList();
		}

		void CInstrumentEditor::OnInsDecoctave() 
		{
			const int si=_pSong->instSelected;
			if ( _pSong->_pInstrument[si]->waveTune>-37)
				_pSong->_pInstrument[si]->waveTune-=12;
			else _pSong->_pInstrument[si]->waveTune=-48;
			UpdateNoteLabel();	
		}

		void CInstrumentEditor::OnInsDecnote() 
		{
			const int si=_pSong->instSelected;
			if ( _pSong->_pInstrument[si]->waveTune>-47)
				_pSong->_pInstrument[si]->waveTune-=1;
			else _pSong->_pInstrument[si]->waveTune=-48;
			UpdateNoteLabel();	
		}

		void CInstrumentEditor::OnInsIncnote() 
		{
			const int si=_pSong->instSelected;
			if ( _pSong->_pInstrument[si]->waveTune < 71)
				_pSong->_pInstrument[si]->waveTune+=1;
			else _pSong->_pInstrument[si]->waveTune=71;
			UpdateNoteLabel();	
		}

		void CInstrumentEditor::OnInsIncoctave() 
		{
			const int si=_pSong->instSelected;
			if ( _pSong->_pInstrument[si]->waveTune < 60)
				_pSong->_pInstrument[si]->waveTune+=12;
			else _pSong->_pInstrument[si]->waveTune=71;
			UpdateNoteLabel();	
		}

		void CInstrumentEditor::UpdateNoteLabel()
		{
			const int si = _pSong->instSelected;
			char buffer[64];
			
			const int octave= ((_pSong->_pInstrument[si]->waveTune+48)/12);
			switch ((_pSong->_pInstrument[si]->waveTune+48)%12)
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

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
