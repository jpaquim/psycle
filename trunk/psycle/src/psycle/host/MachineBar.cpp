/** @file 
 *  @brief MachineBar dialog
 *  $Date: 2010-08-15 18:18:35 +0200 (dg., 15 ag 2010) $
 *  $Revision: 9831 $
 */
#include <psycle/host/detail/project.private.hpp>
#include "MachineBar.hpp"
#include "ChildView.hpp"
#include "InputHandler.hpp"
#include "MainFrm.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Plugin.hpp"
#include "XMSongLoader.hpp"
#include "ITModule2.h"
#include "WavFileDlg.hpp"
#include "GearRackDlg.hpp"
#include "WaveEdFrame.hpp"

namespace psycle{ namespace host{
	extern CPsycleApp theApp;
IMPLEMENT_DYNAMIC(MachineBar, CDialogBar)

	MachineBar::MachineBar()
	{
	}

	MachineBar::~MachineBar()
	{
	}

	void MachineBar::DoDataExchange(CDataExchange* pDX)
	{
		CDialogBar::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_COMBOSTEP, m_stepcombo);
		DDX_Control(pDX, IDC_BAR_COMBOGEN, m_gencombo);
		DDX_Control(pDX, IDC_AUXSELECT, m_auxcombo);
		DDX_Control(pDX, IDC_BAR_COMBOINS, m_inscombo);
	}

	//Message Maps are defined in CMainFrame, since this isn't a window, but a DialogBar.
	BEGIN_MESSAGE_MAP(MachineBar, CDialogBar)
		ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
	END_MESSAGE_MAP()

	void MachineBar::InitializeValues(CMainFrame* frame, CChildView* view, Song& song)
	{
		m_pParentMain = frame;
		m_pWndView = view;
		m_pSong = &song;
	}


	// MachineBar message handlers
	LRESULT MachineBar::OnInitDialog ( WPARAM wParam, LPARAM lParam)
	{
		BOOL bRet = HandleInitDialog(wParam, lParam);

		if (!UpdateData(FALSE))
		{
		   TRACE0("Warning: UpdateData failed during dialog init.\n");
		}

		macComboInitialized = false;

		((CButton*)GetDlgItem(IDC_B_DECGEN))->SetIcon(PsycleGlobal::conf().iconless);
		((CButton*)GetDlgItem(IDC_B_INCGEN))->SetIcon(PsycleGlobal::conf().iconmore);
		((CButton*)GetDlgItem(IDC_B_DECWAV))->SetIcon(PsycleGlobal::conf().iconless);
		((CButton*)GetDlgItem(IDC_B_INCWAV))->SetIcon(PsycleGlobal::conf().iconmore);

		m_stepcombo.SetCurSel(m_pWndView->patStep);

		m_auxcombo.SetCurSel(AUX_PARAMS);

		return bRet;
	}


	void MachineBar::OnSelchangeCombostep()
	{
		int sel=m_stepcombo.GetCurSel();
		m_pWndView->patStep=sel;
		m_pWndView->SetFocus();
	}

	void MachineBar::OnCloseupCombostep()
	{
		m_pWndView->SetFocus();
	}

	void MachineBar::EditQuantizeChange(int diff) // User Called (Hotkey)
	{
		const int total = m_stepcombo.GetCount();
		const int nextsel = (total + m_stepcombo.GetCurSel() + diff) % total;
		m_stepcombo.SetCurSel(nextsel);
		m_pWndView->patStep=nextsel;
	}

	void MachineBar::OnBDecgen() // called by Button and Hotkey.
	{
		const int val = m_gencombo.GetCurSel();
		if ( val > 0 ) m_gencombo.SetCurSel(val-1);
		else m_gencombo.SetCurSel(m_gencombo.GetCount()-1);
		if ( m_gencombo.GetItemData(m_gencombo.GetCurSel()) == 65535 )
		{
			if ( val >1) m_gencombo.SetCurSel(val-2);
			else m_gencombo.SetCurSel(val);
		}
		OnSelchangeBarCombogen();
		((CButton*)GetDlgItem(IDC_B_DECGEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnBIncgen() // called by Button and Hotkey.
	{
		const int val = m_gencombo.GetCurSel();
		if ( val < m_gencombo.GetCount()-1 ) m_gencombo.SetCurSel(val+1);
		else m_gencombo.SetCurSel(0);
		if ( m_gencombo.GetItemData(m_gencombo.GetCurSel()) == 65535 )
		{
			if ( val < m_gencombo.GetCount()-2) m_gencombo.SetCurSel(val+2);
			else m_gencombo.SetCurSel(val);
		}
		OnSelchangeBarCombogen();
		((CButton*)GetDlgItem(IDC_B_INCGEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::UpdateComboGen(bool updatelist)
	{
		bool filled=false;
		bool found=false;
		int selected = -1;
		int line = -1;
		char buffer[64];
		
		if (m_pSong == NULL) 
		{
			return; // why should this happen?
		}
		
		macComboInitialized = false;
		if (updatelist) 
		{
			m_gencombo.ResetContent();
		}
		
		for (int b=0; b<MAX_BUSES; b++) // Check Generators
		{
			if( m_pSong->_pMachine[b])
			{
				if (updatelist)
				{	
					sprintf(buffer,"%.2X: %s",b,m_pSong->_pMachine[b]->_editName);
					m_gencombo.AddString(buffer);
					m_gencombo.SetItemData(m_gencombo.GetCount()-1,b);
				}
				if (!found) 
				{
					selected++;
				}
				if (m_pSong->seqBus == b) 
				{
					found = true;
				}
				filled = true;
			}
		}
		if ( updatelist) 
		{
			m_gencombo.AddString("----------------------------------------------------");
			m_gencombo.SetItemData(m_gencombo.GetCount()-1,65535);
		}
		if (!found) 
		{
			selected++;
			line = selected;
		}
		
		for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
		{
			if(m_pSong->_pMachine[b])
			{
				if (updatelist)
				{	
					sprintf(buffer,"%.2X: %s",b,m_pSong->_pMachine[b]->_editName);
					m_gencombo.AddString(buffer);
					m_gencombo.SetItemData(m_gencombo.GetCount()-1,b);
				}
				if (!found) 
				{
					selected++;
				}
				if (m_pSong->seqBus == b) 
				{
					found = true;
				}
				filled = true;
			}
		}
		if (!filled)
		{
			m_gencombo.ResetContent();
			m_gencombo.AddString("No Machines Loaded");
			selected = 0;
		}
		else if (!found) 
		{
			selected=line;
		}
		
		m_gencombo.SetCurSel(selected);

		// Select the appropiate Option in Aux Combobox.
		if (found) // If found (which also means, if it exists)
		{
			if (m_pSong->_pMachine[m_pSong->seqBus])
			{
				if ( m_pSong->seqBus < MAX_BUSES && m_pSong->_pMachine[m_pSong->seqBus]->NeedsAuxColumn())
				{
					m_auxcombo.SetCurSel(AUX_INSTRUMENT);
					if (m_pSong->_pMachine[m_pSong->seqBus]->_type == MACH_XMSAMPLER) {
						m_pSong->auxcolSelected = m_pSong->instSelected;
					}
					else if (m_pSong->_pMachine[m_pSong->seqBus]->_type == MACH_SAMPLER) {
						m_pSong->auxcolSelected = m_pSong->waveSelected;
					}
					else {
						m_pSong->auxcolSelected = m_pSong->_pMachine[m_pSong->seqBus]->AuxColumnIndex();
					}
				}
				else
				{
					m_auxcombo.SetCurSel(AUX_PARAMS);
					m_pSong->auxcolSelected = std::min(m_pSong->paramSelected,m_pSong->_pMachine[m_pSong->seqBus]->GetNumParams());
				}
			}
		}
		else
		{
			m_auxcombo.SetCurSel(AUX_INSTRUMENT); // WAVES
			m_pSong->auxcolSelected = m_pSong->waveSelected;
		}
		UpdateComboIns();
		macComboInitialized = true;
	}

	void MachineBar::OnSelchangeBarCombogen() 
	{
		if(macComboInitialized)
		{
			int nsb = GetNumFromCombo(&m_gencombo);

			if(m_pSong->seqBus!=nsb)
			{
				m_pSong->seqBus=nsb;
				UpdateComboGen(false);
			}
			
			m_pParentMain->RedrawGearRackList();
			//Added by J.Redfern, repaints main view after changing selection in combo
			m_pWndView->Repaint();

		}
	}

	void MachineBar::OnCloseupBarCombogen()
	{
		m_pWndView->SetFocus();
	}


	void MachineBar::ChangeGen(int i)	// Used to set an specific seqBus (used in "CChildView::SelectMachineUnderCursor")
	{
		if(i>=0 && i <(MAX_BUSES*2))
		{
			if ( (m_pSong->seqBus & MAX_BUSES) == (i & MAX_BUSES))
			{
				m_pSong->seqBus=i;
				UpdateComboGen(false);
			}
			else
			{
				m_pSong->seqBus=i;
				UpdateComboGen(true);
			}
		}
	}

	void MachineBar::OnGearRack() 
	{
		if (m_pParentMain->pGearRackDialog == NULL)
		{
			m_pParentMain->pGearRackDialog = new CGearRackDlg(m_pParentMain,m_pWndView, &m_pParentMain->pGearRackDialog);
			m_pParentMain->pGearRackDialog->ShowWindow(SW_SHOW);
		}
		else {

			m_pParentMain->pGearRackDialog->SetActiveWindow();
		}
		((CButton*)GetDlgItem(IDC_GEAR_RACK))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	void MachineBar::OnCloseupAuxselect() 
	{
		m_pWndView->SetFocus();
	}

	void MachineBar::OnSelchangeAuxselect() 
	{
		if ( m_auxcombo.GetCurSel() == AUX_INSTRUMENT )	// WAVES
		{
			m_pSong->auxcolSelected=m_pSong->instSelected;
		}
		UpdateComboIns();
	}
	void MachineBar::OnBDecAux() 
	{
		ChangeAux(m_pSong->auxcolSelected-1);
		((CButton*)GetDlgItem(IDC_B_DECWAV))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnBIncAux() 
	{
		Machine *tmac = m_pSong->_pMachine[m_pSong->seqBus];
		if (tmac) {
			if (tmac->_type == MACH_XMSAMPLER) {
				if (Global::song().xminstruments.size() <= m_pSong->auxcolSelected+1) {
					XMInstrument inst;
					inst.Init();
					Global::song().xminstruments.SetInst(inst,m_pSong->auxcolSelected+1);
					UpdateComboIns(true);
				}
			}
			else if (tmac->_type == MACH_SAMPLER) {
				if (Global::song().samples.size() <= m_pSong->auxcolSelected+1) {
					XMInstrument::WaveData<> wave;
					wave.Init();
					Global::song().samples.SetSample(wave,m_pSong->auxcolSelected+1);
					UpdateComboIns(true);
				}
			}
		}
		ChangeAux(m_pSong->auxcolSelected+1);
		((CButton*)GetDlgItem(IDC_B_INCWAV))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::UpdateComboIns(bool updatelist)
	{
		int listlen = 0;
		
		if (updatelist) 
		{
			m_inscombo.ResetContent();
		}

		if ( m_auxcombo.GetCurSel() == AUX_PARAMS)	// Params
		{
			int nmac = m_pSong->seqBus;
			Machine *tmac = m_pSong->_pMachine[nmac];
			if (tmac) 
			{
				int i=0;
				if (updatelist) 
				{
					for (i=0;i<tmac->GetNumParams();i++)
					{
						char buffer[64],buffer2[64];
						std::memset(buffer2,0,64);
						tmac->GetParamName(i,buffer2);
						bool label(false);
						if(tmac->_type == MACH_PLUGIN)
						{
							if(!(static_cast<Plugin*>(tmac)->GetInfo()->Parameters[i]->Flags & psycle::plugin_interface::MPF_STATE))
								label = true;
						}
						if(label)
							// just a label
							sprintf(buffer, "------ %s ------", buffer2);
						else
							sprintf(buffer, "%.2X:  %s", i, buffer2);
						m_inscombo.AddString(buffer);
						listlen++;
					}
				}
				else
				{
					listlen = m_inscombo.GetCount();
				}
			}
			else
			{
				if (updatelist) 
				{
					m_inscombo.AddString("No Machine");
				}
				listlen = 1;
			}
		}
		else
		{
			char buffer[64];
			if (updatelist) 
			{
				Machine *tmac = m_pSong->_pMachine[m_pSong->seqBus];
				if (tmac && tmac->NeedsAuxColumn()) 
				{
					listlen= tmac->NumAuxColumnIndexes();
					for (int i(0); i<listlen; i++)
					{
						sprintf(buffer, "%.2X: %s", i, tmac->AuxColumnName(i));
						m_inscombo.AddString(buffer);
					}
				}
				else
				{
					m_inscombo.AddString("No Machine");
				}
			}
			else
			{
				listlen = m_inscombo.GetCount();
			}
		}
		if (m_pSong->auxcolSelected >= listlen)
		{
			m_pSong->auxcolSelected = 0;
		}
		m_inscombo.SetCurSel(m_pSong->auxcolSelected);
	}

	void MachineBar::OnSelchangeBarComboins() 
	{
		if ( m_auxcombo.GetCurSel() == AUX_PARAMS ) {
			m_pSong->paramSelected=m_inscombo.GetCurSel();
		} else {
			m_pSong->instSelected=m_inscombo.GetCurSel();
			m_pParentMain->WaveEditorBackUpdate();
			m_pParentMain->UpdateInstrumentEditor();
			m_pParentMain->RedrawGearRackList();
		}

		m_pSong->auxcolSelected=m_inscombo.GetCurSel();
	}

	void MachineBar::OnCloseupBarComboins()
	{
		m_pWndView->SetFocus();
	}
	void MachineBar::ChangeWave(int i)
	{
		if ( m_pSong->waveSelected == i) return;
		if (i<0 || i >= MAX_INSTRUMENTS) return;
		m_pSong->waveSelected=i;
		m_pParentMain->UpdateInstrumentEditor();
		m_pParentMain->WaveEditorBackUpdate();
		m_pParentMain->RedrawGearRackList();
	}
	void MachineBar::ChangeIns(int i)
	{
		if ( m_pSong->instSelected == i) return;
		if (i<0 || i >= MAX_INSTRUMENTS) return;
		m_pSong->instSelected=i;
		m_pParentMain->UpdateInstrumentEditor();
		m_pParentMain->RedrawGearRackList();
	}
	void MachineBar::ChangeAux(int i)	// User Called (Hotkey, button or list change)
	{
		if ( m_inscombo.GetCurSel() == i) return;
		if (i<0 || i >= m_inscombo.GetCount()) return;

		if ( m_auxcombo.GetCurSel() == AUX_PARAMS ) {
			m_pSong->paramSelected=i;
		} else {
			Machine *tmac = m_pSong->_pMachine[m_pSong->seqBus];
			if (tmac) {
				if (tmac->_type == MACH_XMSAMPLER) {
					m_pSong->instSelected=i;
				}
				else if (tmac->_type == MACH_SAMPLER) {
					m_pSong->waveSelected=i;
				}
				else {
					tmac->AuxColumnIndex(i);
				}
			}
			else { m_pSong->waveSelected=i; }
			m_pParentMain->UpdateInstrumentEditor();
			m_pParentMain->WaveEditorBackUpdate();
			m_pParentMain->RedrawGearRackList();
		}
		m_pSong->auxcolSelected=i;
		m_inscombo.SetCurSel(m_pSong->auxcolSelected);
	}

	void MachineBar::OnLoadwave() 
	{
		bool update=false;
		int nmac = m_pSong->seqBus;
		Machine *tmac = m_pSong->_pMachine[nmac];
		bool found=false;
		if (!tmac || (tmac->_type != MACH_SAMPLER && tmac->_type != MACH_XMSAMPLER)) {
			for(int i=0;i<MAX_MACHINES;i++) {
				if (m_pSong->_pMachine[i] && (m_pSong->_pMachine[i]->_type == MACH_SAMPLER ||
						m_pSong->_pMachine[i]->_type == MACH_XMSAMPLER)	) {
					m_pSong->seqBus = i;
					m_pParentMain->UpdateComboGen();
					m_pWndView->Repaint();
					found=true;
					break;
				}
			}
		}
		else {
			found = true;
		}
		if(!found) {
			int i = m_pSong->GetFreeMachine();
			m_pSong->CreateMachine(MACH_SAMPLER,16,16,NULL, i);
			m_pSong->seqBus = i;
			m_pParentMain->UpdateComboGen();
			m_pWndView->Repaint();
		}
		tmac = m_pSong->_pMachine[m_pSong->seqBus];
		if (tmac && tmac->_type == MACH_XMSAMPLER) {
			int si = m_pSong->instSelected;
			if (m_pSong->xminstruments.IsEnabled(si)) {
				if (MessageBox("An instrument already exists in this slot. If you continue, it will be ovewritten. Continue?"
				,"Sample Loading",MB_YESNO|MB_ICONWARNING) == IDNO)  return;
			}
			update=LoadInstrument(si);

		}
		else {
			int si = m_pSong->waveSelected;
			if (m_pSong->samples.IsEnabled(si)) {
				if (MessageBox("A sample already exists in this slot. If you continue, it will be ovewritten. Continue?"
					,"Sample Loading",MB_YESNO|MB_ICONWARNING) == IDNO)  return;
			}
			update=LoadWave(si);
		}
		
		if (update){
			UpdateComboIns();
			m_pParentMain->m_wndStatusBar.SetWindowText("New wave loaded");
			m_pParentMain->WaveEditorBackUpdate();
			m_pParentMain->UpdateInstrumentEditor();
			m_pParentMain->RedrawGearRackList();
		}
		((CButton*)GetDlgItem(IDC_LOADWAVE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnSavewave()
	{
		Machine *tmac = m_pSong->_pMachine[m_pSong->seqBus];
		if (tmac && tmac->_type == MACH_XMSAMPLER) {
			if (m_pSong->xminstruments.IsEnabled(m_pSong->instSelected)) {
				SaveInstrument(m_pSong->instSelected);
			}
			else MessageBox("Nothing to save...\nSelect nonempty instrument first.", "Error", MB_ICONERROR);
		}
		else 
		{
			if (m_pSong->samples.IsEnabled(m_pSong->waveSelected)) {
				SaveWave(m_pSong->waveSelected);
			}
			else MessageBox("Nothing to save...\nSelect nonempty wave first.", "Error", MB_ICONERROR);
		}
	}

	void MachineBar::OnEditwave() 
	{
		bool found=false;
		for(int i=0;i<MAX_MACHINES;i++) {
			if (m_pSong->_pMachine[i] && (m_pSong->_pMachine[i]->_type == MACH_SAMPLER ||
					m_pSong->_pMachine[i]->_type == MACH_XMSAMPLER)	) {
				found=true;
				break;
			}
		}
		if(!found) {
			MessageBox(_T("Warning: To use samples, it is required to have a Sampler or a Sampulse Internal machine"),
				_T("Instrument editor"),MB_ICONWARNING);
		}
		m_pParentMain->ShowInstrumentEditor();
		((CButton*)GetDlgItem(IDC_EDITWAVE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	void MachineBar::OnWavebut() 
	{
		m_pParentMain->m_pWndWed->ShowWindow(SW_SHOWNORMAL);
		m_pParentMain->m_pWndWed->SetActiveWindow();
		((CButton*)GetDlgItem(IDC_WAVEBUT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	bool MachineBar::LoadWave(int waveIdx)
	{
		static char BASED_CODE szFilter[] = "Wav (PCM) Files (*.wav)|*.wav|Amiga IFF/SVX Samples (*.*)|*.*||";//TODO: its, s3i, aiff..
		CWavFileDlg dlg(true,"wav", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST| OFN_DONTADDTORECENT, szFilter);
		dlg.m_pSong = m_pSong;
		std::string tmpstr = PsycleGlobal::conf().GetCurrentInstrumentDir();
		dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
		bool update=false;
		if (dlg.DoModal() == IDOK)
		{
			PsycleGlobal::inputHandler().AddMacViewUndo();

			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			CString CurrExt=dlg.GetFileExt();
			CurrExt.MakeLower();
			if ( CurrExt == "wav" || dlg.GetOFN().nFilterIndex == 1 )
			{
				try {
					if (m_pSong->WavAlloc(waveIdx,dlg.GetPathName().GetString()))
					{
						update=true;
					}
					else {
						MessageBox("Could not load the file, unrecognized format","Load Error",MB_ICONERROR);
					}
				}
				catch(const std::runtime_error & e) {
					std::ostringstream os;
					os <<"Could not finish the operation: " << e.what();
					MessageBox(os.str().c_str(),"Load Error",MB_ICONERROR);
				}
			}
			else if ( CurrExt == "iff" || dlg.GetOFN().nFilterIndex == 2  )
			{
				try {
					if (m_pSong->IffAlloc(waveIdx,dlg.GetPathName().GetString()))
					{
						update=true;
					}
					else {
						MessageBox("Could not load the file, unrecognized format","Load Error",MB_ICONERROR);
					}
				}
				catch(const std::runtime_error & e) {
					std::ostringstream os;
					os <<"Could not finish the operation: " << e.what();
					MessageBox(os.str().c_str(),"Load Error",MB_ICONERROR);
				}
			}
			CString str = dlg.m_ofn.lpstrFile;
			int index = str.ReverseFind('\\');
			if (index != -1)
			{
				PsycleGlobal::conf().SetCurrentInstrumentDir(static_cast<char const *>(str.Left(index)));
			}
		}
		{
			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			// Stopping wavepreview if not stopped.
			m_pSong->wavprev.Stop();
		}
		return update;
	}

	void MachineBar::SaveWave(int waveIdx)
	{
		WaveFile output;
		static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";

		const XMInstrument::WaveData<> & wave = m_pSong->samples[waveIdx];
		CFileDialog dlg(FALSE, "wav", wave.WaveName().c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_DONTADDTORECENT, szFilter);
		std::string tmpstr = PsycleGlobal::conf().GetCurrentInstrumentDir();
		dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
		if (dlg.DoModal() == IDOK)
		{
			output.OpenForWrite(dlg.GetPathName(), wave.WaveSampleRate(), 16, (wave.IsWaveStereo()) ? 2 : 1 );
			if (wave.IsWaveStereo()) {
				for ( unsigned int c=0; c < wave.WaveLength(); c++)
				{
					output.WriteStereoSample( *(wave.pWaveDataL() + c), *(wave.pWaveDataR() + c) );
				}
			} else {
				output.WriteData(wave.pWaveDataL(), wave.WaveLength());
			}
			output.Close();
		}
	}

	bool MachineBar::LoadInstrument(int instIdx)
	{
		static char BASED_CODE szFilter[] = "Psycle Instrument (*.psins)|*.psins|XM Instruments (*.xi)|*.xi|IT Instruments (*.iti)|*.iti||";

		CFileDialog dlg(true,"psins", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST| OFN_DONTADDTORECENT, szFilter);
		std::string tmpstr = PsycleGlobal::conf().GetCurrentInstrumentDir();
		dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
		bool update=false;
		if (dlg.DoModal() == IDOK)
		{
			PsycleGlobal::inputHandler().AddMacViewUndo();

			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			CString CurrExt=dlg.GetFileExt();
			CurrExt.MakeLower();
			if ( CurrExt == "psins" ) {
				m_pSong->LoadPsyInstrument(dlg.GetPathName().GetString(),instIdx);
				update=true;
			}
			else if (CurrExt == "xi") {
				XMSongLoader xmsong;
				xmsong.Open(dlg.GetPathName().GetString());
				xmsong.LoadInstrumentFromFile(*m_pSong,instIdx);
				xmsong.Close();
				update=true;
			}
			else if (CurrExt == "iti") {
				ITModule2 itsong;
				itsong.Open(dlg.GetPathName().GetString());
				itsong.LoadInstrumentFromFile(*m_pSong,instIdx);
				itsong.Close();
				update=true;
			}

			CString str = dlg.m_ofn.lpstrFile;
			int index = str.ReverseFind('\\');
			if (index != -1) {
				PsycleGlobal::conf().SetCurrentInstrumentDir(static_cast<char const *>(str.Left(index)));
			}
		}
		return update;
	}
	void MachineBar::SaveInstrument(int instIdx)
	{
		static char BASED_CODE szFilter[] = "Psycle Instrument (*.psins)|*.psins||";//TODO: XM Instruments (*.xi)|*.xi|IT Instruments (*.iti)|*.iti||";

		const XMInstrument instr = m_pSong->xminstruments[instIdx];
		CFileDialog dlg(FALSE, "psins", instr.Name().c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_DONTADDTORECENT, szFilter);
		std::string tmpstr = PsycleGlobal::conf().GetCurrentInstrumentDir();
		dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
		if (dlg.DoModal() == IDOK)
		{
			CString CurrExt=dlg.GetFileExt();
			CurrExt.MakeLower();
			if ( CurrExt == "psins" ) {
				m_pSong->SavePsyInstrument(dlg.GetPathName().GetString(),instIdx);
			}
			else if (CurrExt == "xi") {
			}
			else if (CurrExt == "iti") {
			}
		}
	}

	int MachineBar::GetNumFromCombo(CComboBox *cb)
	{
		CString str;
		cb->GetWindowText(str);
		int result;
		helpers::hexstring_to_integer(static_cast<LPCTSTR>(str.Left(2)), result);
		return result;
	}
}}