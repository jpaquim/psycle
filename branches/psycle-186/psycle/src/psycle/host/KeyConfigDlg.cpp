///\file
///\brief implementation file for psycle::host::CKeyConfigDlg.

#include "KeyConfigDlg.hpp"
#include "KeyPresetIO.hpp"
#include "SpecialKeys.hpp"

namespace psycle { namespace host {

		IMPLEMENT_DYNCREATE(CKeyConfigDlg, CPropertyPage)

		CKeyConfigDlg::CKeyConfigDlg() : CPropertyPage(CKeyConfigDlg::IDD)
			, handler(Global::psycleconf().inputHandler())
		{
			
			m_prvIdx = 0;
			bInit = FALSE;
		}

		void CKeyConfigDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_CMDLIST, m_lstCmds);
			DDX_Control(pDX, IDC_KEY0, m_hotkey0);
			DDX_Control(pDX, IDC_PLAY_R_CTRL, m_cmdCtrlPlay);
			DDX_Control(pDX, IDC_FT2_HOME_END, m_cmdNewHomeBehaviour);
			DDX_Control(pDX, IDC_FT2DEL, m_cmdFT2Del);
			DDX_Control(pDX, IDC_WINDOWSBLOCKS, m_windowsblocks);
			DDX_Control(pDX, IDC_SHIFTARROWS, m_cmdShiftArrows);
			DDX_Control(pDX, IDC_WRAP, m_wrap);
			DDX_Control(pDX, IDC_CURSORDOWN, m_cursordown);
			DDX_Control(pDX, IDC_TWEAK_SMOOTH, m_tweak_smooth);
			DDX_Control(pDX, IDC_RECORD_UNARMED, m_record_unarmed);			
			DDX_Control(pDX, IDC_NAVIGATION_IGNORES_STEP, m_navigation_ignores_step);
			DDX_Control(pDX, IDC_PAGEUPSTEPS, m_pageupsteps);

			DDX_Control(pDX, IDC_AUTOSAVE_CURRENT_SONG, m_autosave);
			DDX_Control(pDX, IDC_AUTOSAVE_MINS, m_autosave_mins);
			DDX_Control(pDX, IDC_AUTOSAVE_MINS_SPIN, m_autosave_spin);
			DDX_Control(pDX, IDC_FILE_SAVE_REMINDERS, m_save_reminders);
			DDX_Control(pDX, IDC_SHOW_INFO_ON_LOAD, m_show_info);
			DDX_Control(pDX, IDC_MULTIPLEINSTANCES, m_allowinstances);
			DDX_Control(pDX, IDC_SAVESETTINGS_ON, m_storeplaces);

			DDX_Control(pDX, IDC_EDIT_DEFLINES, m_numlines);
			DDX_Control(pDX, IDC_SPIN_DEFLINES, m_spinlines);
			DDX_Control(pDX, IDC_TEXT_DEFLINES, m_textlines);
		}

		BEGIN_MESSAGE_MAP(CKeyConfigDlg, CDialog)
			ON_LBN_SELCHANGE(IDC_CMDLIST, OnSelchangeCmdlist)
			ON_BN_CLICKED(IDC_IMPORTREG, OnImportreg)
			ON_BN_CLICKED(IDC_EXPORTREG, OnExportreg)
			ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
			ON_BN_CLICKED(IDC_SPECIALKEYS, OnBnClickedSpecialKeys)
			ON_BN_CLICKED(IDC_NONE, OnNone)
			ON_EN_UPDATE(IDC_EDIT_DEFLINES, OnUpdateNumLines)
		END_MESSAGE_MAP()

		BOOL CKeyConfigDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			PsycleConfig& config = Global::psycleconf();
			
			// prevent ALT in hotkey	
			WORD rules=HKCOMB_A|HKCOMB_CA|HKCOMB_SA|HKCOMB_SCA;
			WORD subst=0;
			m_hotkey0.SetRules(rules,subst);

			m_cmdCtrlPlay.SetCheck(handler.bCtrlPlay?1:0);
			m_cmdNewHomeBehaviour.SetCheck(handler.bFT2HomeBehaviour?1:0);
			m_cmdFT2Del.SetCheck(handler.bFT2DelBehaviour?1:0);
			m_windowsblocks.SetCheck(handler._windowsBlocks?1:0);
			m_cmdShiftArrows.SetCheck(handler.bShiftArrowsDoSelect?1:0);
			m_wrap.SetCheck(handler._wrapAround?1:0);
			m_cursordown.SetCheck(handler._cursorAlwaysDown?1:0);
			m_tweak_smooth.SetCheck(handler._RecordMouseTweaksSmooth?1:0);
			m_record_unarmed.SetCheck(handler._RecordUnarmed?1:0);
			m_navigation_ignores_step.SetCheck(handler._NavigationIgnoresStep?1:0);
			m_pageupsteps.SetCurSel(handler._pageUpSteps);

			m_autosave.SetCheck(config.autosaveSong?1:0);
			m_autosave_spin.SetRange(1,60);
			char buffer[16];
			itoa(config.autosaveSongTime,buffer,10);
			m_autosave_mins.SetWindowText(buffer);

			m_save_reminders.SetCheck(config.bFileSaveReminders?1:0);
			m_show_info.SetCheck(config.bShowSongInfoOnLoad?1:0);
			m_spinlines.SetRange(1,MAX_LINES);
			m_allowinstances.SetCheck(config._allowMultipleInstances?1:0);
			m_storeplaces.SetCurSel(config.store_place_);

			itoa(Global::psycleconf().GetDefaultPatLines(),buffer,10);
			m_numlines.SetWindowText(buffer);
			UDACCEL acc;
			acc.nSec = 4;
			acc.nInc = 16;
			m_spinlines.SetAccel(1, &acc);

			bInit = TRUE;
			OnUpdateNumLines();
			FillCmdList();
			
			return TRUE;  // return TRUE unless you set the focus to a control
						// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CKeyConfigDlg::OnCancel() 
		{
			CDialog::OnCancel();
		}

		void CKeyConfigDlg::OnOK() 
		{
			PsycleConfig& config = Global::psycleconf();
			// update last key
			WORD mods=0;
			WORD key=0;

			m_hotkey0.GetHotKey(key,mods);
			SaveHotKey(m_prvIdx,key,mods);


			handler.bCtrlPlay  = m_cmdCtrlPlay.GetCheck()?true:false;
			handler.bFT2HomeBehaviour = m_cmdNewHomeBehaviour.GetCheck()?true:false;
			handler.bFT2DelBehaviour = m_cmdFT2Del.GetCheck()?true:false;
			handler._windowsBlocks = m_windowsblocks.GetCheck()?true:false;
			handler.bShiftArrowsDoSelect = m_cmdShiftArrows.GetCheck()?true:false;
			handler._wrapAround = m_wrap.GetCheck()?true:false;
			handler._cursorAlwaysDown = m_cursordown.GetCheck()?true:false;
			handler._RecordMouseTweaksSmooth = m_tweak_smooth.GetCheck()?true:false;
			handler._RecordUnarmed = m_record_unarmed.GetCheck()?true:false;
			handler._NavigationIgnoresStep = m_navigation_ignores_step.GetCheck()?true:false;
			handler._pageUpSteps= m_pageupsteps.GetCurSel();
			
			config.autosaveSong = m_autosave.GetCheck()?true:false;
			config.bFileSaveReminders = m_save_reminders.GetCheck()?true:false;
			config.bShowSongInfoOnLoad = m_show_info.GetCheck()?true:false;
			config._allowMultipleInstances = m_allowinstances.GetCheck()?true:false;
			config.store_place_ = static_cast<PsycleConfig::store_t>(m_storeplaces.GetCurSel());

			char buffer[32];
			m_autosave_mins.GetWindowText(buffer,16);
			int nlines = atoi(buffer);
			if (nlines < 1)
			{ nlines = 1; }
			else if (nlines > 60)
			{ nlines = 60; }
			config.autosaveSongTime = nlines;
			
			m_numlines.GetWindowText(buffer,16);
			nlines = atoi(buffer);
			if (nlines < 1) {
				nlines = 1;
			}
			else if (nlines > MAX_LINES) {
				nlines = MAX_LINES;
			}
			config.SetDefaultPatLines(nlines);

			//Check the modified keys.
			std::map<int, std::pair<int,int>>::const_iterator it;
			for(it = keyMap.begin(); it != keyMap.end(); ++it)
			{
				CmdSet set = (CmdSet) m_lstCmds.GetItemData(it->first);
				handler.SetCmd(set, it->second.first,it->second.second);
			}

			CDialog::OnOK();
		}

		/// restores key config data.
		void CKeyConfigDlg::OnImportreg() 
		{
			OPENFILENAME ofn;       // common dialog box structure
			char szFile[_MAX_PATH];       // buffer for file name
			
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Psycle Keymap Presets\0*.psk\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			ofn.lpstrInitialDir =  Global::psycleconf().GetSkinDir().c_str();
			// Display the Open dialog box. 
			
			if (GetOpenFileName(&ofn)==TRUE)
			{
				///\todo: Cannot cancel and need to refresh the UI.
				KeyPresetIO::LoadPreset(szFile, handler);
				keyMap.clear();
			}
			// update display for new key
			FillCmdList();
		}


		void CKeyConfigDlg::OnExportreg() 
		{
			// save key config data
			// returns true on success, false on fail
			OPENFILENAME ofn; // common dialog box structure
			char szFile[_MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Psycle Keymap Presets\0*.psk\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	
			ofn.lpstrInitialDir = Global::psycleconf().GetSkinDir().c_str();

			if (GetSaveFileName(&ofn)==TRUE)
			{
				///\todo: Will not export settings changed since dialog opened.
				KeyPresetIO::LoadPreset(szFile, handler);
			}
		}

		void CKeyConfigDlg::OnDefaults() 
		{
			///\todo: Cannot cancel and need to refresh the UI.
			handler.SetDefaultSettings();
			keyMap.clear();
			FillCmdList();
		}

		/// update key on show
		void CKeyConfigDlg::OnSelchangeCmdlist() 
		{	
			if(m_lstCmds.GetCurSel() == -1) return;
			WORD key = 0;
			WORD mods = 0;	
			
			// save key settings for key we've just moved from
			m_hotkey0.GetHotKey(key,mods);
			SaveHotKey(m_prvIdx,key,mods);
			key=0;
			mods=0;
			m_prvIdx = m_lstCmds.GetCurSel();
			FindKey(m_prvIdx,key,mods);
			m_hotkey0.SetHotKey(key,mods);
		}
		void CKeyConfigDlg::OnBnClickedSpecialKeys()
		{
			CSpecialKeys dlg;
			if ( dlg.DoModal() == IDOK )
			{
				int idx = m_lstCmds.GetCurSel();
				SaveHotKey(idx, dlg.mod, dlg.key);

				FillCmdList();
			}
		}
		void CKeyConfigDlg::OnNone() 
		{
			SaveHotKey(m_prvIdx, 0, 0);
			m_hotkey0.SetHotKey(0,0);
		}

		void CKeyConfigDlg::OnUpdateNumLines() 
		{
			char buffer[256];
			if (bInit)
			{
				m_numlines.GetWindowText(buffer,16);
				int val=atoi(buffer);
				if (val < 0) {
					val = 0;
				}
				else if(val > MAX_LINES) {
					val = MAX_LINES-1;
				}
				sprintf(buffer,"HEX: %x",val);
				m_textlines.SetWindowText(buffer);
			}
		}

		void CKeyConfigDlg::SaveHotKey(long idx,WORD new_key,WORD new_mods)
		{
			WORD old_key = 0;
			WORD old_mods = 0;
			UINT nMod=0;
			
			// Locate the current values
			FindKey(idx,old_key,old_mods);
			
			// same as previous? do nothing
			if(old_key==new_key && old_mods==new_mods)
				return;

			if(new_mods&HOTKEYF_SHIFT) {
				nMod|=MOD_S;
			}
			if(new_mods&HOTKEYF_CONTROL) {
				nMod|=MOD_C;
			}
			if(new_mods&HOTKEYF_EXT) {
				nMod|=MOD_E;
			}

			std::pair<int,int> pair(nMod,new_key);
			keyMap[idx]= pair;
		}

		void CKeyConfigDlg::FindKey(long idx,WORD&key,WORD&mods)
		{
			//Check if the key has been already modified.
			std::map<int, std::pair<int,int>>::const_iterator it;
			UINT j = 0;
			it = keyMap.find(idx);
			if (it != keyMap.end())
			{
				j = it->second.first;
				key = it->second.second;
			}
			else 
			{
				//If not modified, find it in the handler's map
				std::map<CmdSet, std::pair<int,int>>::const_iterator it2;
				CmdSet set = (CmdSet) m_lstCmds.GetItemData(idx);
				it2 = handler.setMap.find(set);
				if (it2 != handler.setMap.end()) {
					j = it2->second.first;
					key = it2->second.second;
				}
				else {
					//Safeguard.
					j = 0;
					key = 0;
				}
			}
			if(j&MOD_S) {
				mods|=HOTKEYF_SHIFT;
			}
			if(j&MOD_C) {
				mods|=HOTKEYF_CONTROL;
			}
			if(j&MOD_E) {
				mods|=HOTKEYF_EXT;
			}
		}

		void CKeyConfigDlg::FillCmdList()
		{
			WORD key = 0;
			WORD mods = 0;
			int pos;
			// clear list box
			m_lstCmds.ResetContent();
			std::map<CmdSet,std::pair<int,int>>::const_iterator it;
			for (it = handler.setMap.begin(); it != handler.setMap.end(); ++it)
			{
				pos = m_lstCmds.AddString(CmdDef::GetName(it->first));
				m_lstCmds.SetItemData(pos, it->first);
			}
			m_prvIdx = 0;
			m_lstCmds.SetCurSel(m_prvIdx);
			FindKey(m_prvIdx,key,mods);
			m_hotkey0.SetHotKey(key,mods);
		}

	}   // namespace
}   // namespace
