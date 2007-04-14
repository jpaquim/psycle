///\file
///\brief implementation file for psycle::host::CKeyConfigDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/uiglobal.hpp>
#include <psycle/host/KeyConfigDlg.hpp>
#include <psycle/host/inputhandler.hpp>
#include <psycle/host/uiconfiguration.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		IMPLEMENT_DYNCREATE(CKeyConfigDlg, CPropertyPage)

		CKeyConfigDlg::CKeyConfigDlg() : CPropertyPage(CKeyConfigDlg::IDD)
		{
			m_prvIdx = 0;
			//{{AFX_DATA_INIT(CKeyConfigDlg)
			//}}AFX_DATA_INIT
			bInit = false;
		}

		void CKeyConfigDlg::DoDataExchange(CDataExchange* pDX)
		{
			//removed by alk
			//DDX_Control(pDX, IDC_MOVE_CURSOR_PASTE, m_move_cursor_paste);
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CKeyConfigDlg)
			DDX_Control(pDX, IDC_AUTOSAVE_MINS_SPIN, m_autosave_spin);
			DDX_Control(pDX, IDC_AUTOSAVE_MINS, m_autosave_mins);
			DDX_Control(pDX, IDC_AUTOSAVE_CURRENT_SONG, m_autosave);
			DDX_Control(pDX, IDC_FILE_SAVE_REMINDERS, m_save_reminders);
			DDX_Control(pDX, IDC_TWEAK_SMOOTH, m_tweak_smooth);
			DDX_Control(pDX, IDC_RECORD_UNARMED, m_record_unarmed);			
			DDX_Control(pDX, IDC_NAVIGATION_IGNORES_STEP, m_navigation_ignores_step);
			DDX_Control(pDX, IDC_SHOW_INFO_ON_LOAD, m_show_info);
			DDX_Control(pDX, IDC_SHIFTARROWS, m_cmdShiftArrows);
			DDX_Control(pDX, IDC_FT2DEL, m_cmdFT2Del);
			DDX_Control(pDX, IDC_FT2_HOME_END, m_cmdNewHomeBehaviour);
			DDX_Control(pDX, IDC_PLAY_R_CTRL, m_cmdCtrlPlay);
			DDX_Control(pDX, IDC_CMDLIST, m_lstCmds);
			DDX_Control(pDX, IDC_KEY0, m_hotkey0);
			DDX_Control(pDX, IDC_WRAP, m_wrap);
			DDX_Control(pDX, IDC_CENTERCURSOR, m_centercursor);
			DDX_Control(pDX, IDC_CURSORDOWN, m_cursordown);
			DDX_Control(pDX, IDC_EDIT_DEFLINES, m_numlines);
			DDX_Control(pDX, IDC_SPIN_DEFLINES, m_spinlines);
			DDX_Control(pDX, IDC_TEXT_DEFLINES, m_textlines);
			//}}AFX_DATA_MAP
			DDX_Control(pDX, IDC_COMBO1, m_timesig);
		}

		BEGIN_MESSAGE_MAP(CKeyConfigDlg, CDialog)
			//{{AFX_MSG_MAP(CKeyConfigDlg)
			ON_LBN_SELCHANGE(IDC_CMDLIST, OnSelchangeCmdlist)
			ON_BN_CLICKED(IDC_IMPORTREG, OnImportreg)
			ON_BN_CLICKED(IDC_EXPORTREG, OnExportreg)
			ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
			ON_BN_CLICKED(IDC_NONE, OnNone)
			ON_EN_UPDATE(IDC_EDIT_DEFLINES, OnUpdateNumLines)
			//}}AFX_MSG_MAP
			//ON_BN_CLICKED(IDC_MOVE_CURSOR_PASTE, OnBnClickedMoveCursorPaste)
		END_MESSAGE_MAP()

		void CKeyConfigDlg::FillCmdList()
		{
			// add command definitions
			InputHandler* pinp = UIGlobal::pInputHandler;
			bool written[max_cmds];
			int j,i,pos;

			// clear list box
			m_lstCmds.ResetContent();
			for (i=0;i<max_cmds;i++)written[i]=false;

			for(j=0;j<MOD_MAX;j++)
			{
				for(i=0;i<256;i++)
				{
					if(pinp->cmdLUT[j][i].IsValid())
					{
						pos = m_lstCmds.AddString(LPCTSTR(pinp->cmdLUT[j][i].GetName()));
						m_lstCmds.SetItemData(pos,j*256+i);
						written[int(pinp->cmdLUT[j][i].ID)]=true;
					}
				}
			}
			for (i=0;i<max_cmds;i++) // This second loop adds the strings which don't have a hotkey assigned.
			{
				if ( !written[i] )
				{
					CmdDef cmd;
					CString cmdDefn;
					char bla[64];
					cmd.ID = CmdSet(i);
					cmdDefn = cmd.GetName();
					if(cmdDefn!="Invalid")
					{
						strncpy(bla,cmdDefn,64);
						pos = m_lstCmds.AddString(LPCTSTR(bla));
						m_lstCmds.SetItemData(pos,0);
					}
				}
			}
		}

		BOOL CKeyConfigDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			m_cmdCtrlPlay.SetCheck(UIGlobal::pInputHandler->bCtrlPlay?1:0);
			m_cmdNewHomeBehaviour.SetCheck(UIGlobal::pInputHandler->bFT2HomeBehaviour?1:0);
			m_cmdFT2Del.SetCheck(UIGlobal::pInputHandler->bFT2DelBehaviour?1:0);
			m_cmdShiftArrows.SetCheck(UIGlobal::pInputHandler->bShiftArrowsDoSelect?1:0);

			m_save_reminders.SetCheck(Global::configuration().bFileSaveReminders?1:0);
			m_tweak_smooth.SetCheck(Global::configuration()._RecordMouseTweaksSmooth?1:0);
			m_record_unarmed.SetCheck(Global::configuration()._RecordUnarmed?1:0);
			//m_move_cursor_paste.SetCheck(Global::configuration()._MoveCursorPaste?1:0);
			m_navigation_ignores_step.SetCheck(Global::configuration()._NavigationIgnoresStep?1:0);
			m_show_info.SetCheck(Global::configuration().bShowSongInfoOnLoad?1:0);
			m_autosave.SetCheck(Global::configuration().autosaveSong?1:0);
			
			// prevent ALT in hotkey	

			//TRACE("%d %d %d %d %d %d\n",HKCOMB_A,HKCOMB_C,HKCOMB_S,HKCOMB_CA,HKCOMB_SA,HKCOMB_SCA);

			WORD rules=HKCOMB_A|HKCOMB_CA|HKCOMB_SA|HKCOMB_SCA;
			WORD subst=0;

			m_hotkey0.SetRules(rules,subst);

			m_wrap.SetCheck(Global::configuration()._wrapAround?1:0);
			m_centercursor.SetCheck(Global::configuration()._centerCursor?1:0);
			m_cursordown.SetCheck(Global::configuration()._cursorAlwaysDown?1:0);

			m_spinlines.SetRange(1,MAX_LINES);
			m_autosave_spin.SetRange(1,60);
			
			char buffer[16];
			itoa(Global::configuration().defaultPatLines,buffer,10);
			m_numlines.SetWindowText(buffer);
			itoa(Global::configuration().autosaveSongTime,buffer,10);
			m_autosave_mins.SetWindowText(buffer);
			m_timesig.SetCurSel(Global::configuration().pv_timesig-1);
			
			UDACCEL acc;
			acc.nSec = 4;
			acc.nInc = 16;
			m_spinlines.SetAccel(1, &acc);
			
			bInit = true;
			OnUpdateNumLines();

			FillCmdList();
			
			return true;  // return true unless you set the focus to a control
						// EXCEPTION: OCX Property Pages should return false
		}

		/// update key on show
		void CKeyConfigDlg::OnSelchangeCmdlist() 
		{	
			WORD key = 0;
			WORD mods = 0;	
			
			// save key settings for key we've just moved from
			m_hotkey0.GetHotKey(key,mods);
			SaveHotKey(m_prvIdx,key,mods);
			key=0;
			mods=0;
			// update display for new key
			m_prvIdx = m_lstCmds.GetCurSel();
			FindKey(m_prvIdx,key,mods);
			m_hotkey0.SetHotKey(key,mods);
		}

		void CKeyConfigDlg::SaveHotKey(long idx,WORD new_key,WORD new_mods)
		{
			WORD old_key = 0;
			WORD old_mods = 0;

			// do we have a new key?
			if(!new_key)
				return;		

			// Locate the current values
			FindKey(idx,old_key,old_mods);
			// same as previous? do nothing
			if(old_key==new_key && old_mods==new_mods)
				return;
			UINT nMod=0;
			if(new_mods&HOTKEYF_SHIFT)
				nMod|=MOD_S;
			if(new_mods&HOTKEYF_CONTROL)
				nMod|=MOD_C;	
			if(new_mods&HOTKEYF_EXT)
				nMod|=MOD_E;

			// what command is selected?
			CmdDef cmd = FindCmd(idx);

			m_lstCmds.SetItemData(idx,nMod*256+new_key);
			
			// save key definition
			if(cmd.IsValid())
				UIGlobal::pInputHandler->SetCmd(cmd,new_key,nMod);
		}

		void CKeyConfigDlg::FindKey(long idx,WORD&key,WORD&mods)
		{
			// what command is selected?
//			CmdDef cmd = FindCmd(idx);
			// locate which key is that command
//			if(cmd.IsValid())
//				UIGlobal::pInputHandler->CmdToKey(cmd,key,mods);

			int j = m_lstCmds.GetItemData(idx)/256;
			key = m_lstCmds.GetItemData(idx)%256;
			if(j&MOD_S)
				mods|=HOTKEYF_SHIFT;
			if(j&MOD_C)
				mods|=HOTKEYF_CONTROL;				
			if(j&MOD_E)
				mods|=HOTKEYF_EXT;

		}

		CmdDef CKeyConfigDlg::FindCmd(long idx)
		{	
			CmdDef cmd;
			// sanity check
			if(idx<0) 
				return cmd;				

			// init
			InputHandler* pinp = UIGlobal::pInputHandler;
			int j = m_lstCmds.GetItemData(idx)/256;
			int i = m_lstCmds.GetItemData(idx)%256;

			if ( i != 0 || j != 0)
				cmd = pinp->cmdLUT[j][i];
			else
			{
				// get command string
				CString cmdDesc;
				m_lstCmds.GetText(idx,cmdDesc);

				// convert string to cmd
				cmd = UIGlobal::pInputHandler->StringToCmd(cmdDesc);
			}
			return cmd;
		}

		void CKeyConfigDlg::OnCancel() 
		{
			// user cancelled,
			// restore from saved settings
			UIGlobal::pInputHandler->ConfigRestore();
			CDialog::OnCancel();
		}

		void CKeyConfigDlg::OnOK() 
		{
			// update last key
			OnSelchangeCmdlist();

			UIGlobal::pInputHandler->bCtrlPlay  = m_cmdCtrlPlay.GetCheck()?true:false;
			UIGlobal::pInputHandler->bFT2HomeBehaviour = m_cmdNewHomeBehaviour.GetCheck()?true:false;
			UIGlobal::pInputHandler->bFT2DelBehaviour = m_cmdFT2Del.GetCheck()?true:false;
			UIGlobal::pInputHandler->bShiftArrowsDoSelect = m_cmdShiftArrows.GetCheck()?true:false;
			
			// save settings
			UIGlobal::pInputHandler->ConfigSave();
			
			Global::configuration()._wrapAround = m_wrap.GetCheck()?true:false;
			Global::configuration()._centerCursor = m_centercursor.GetCheck()?true:false;
			Global::configuration()._cursorAlwaysDown = m_cursordown.GetCheck()?true:false;

			Global::configuration().bFileSaveReminders = m_save_reminders.GetCheck()?true:false;
			Global::configuration()._RecordMouseTweaksSmooth = m_tweak_smooth.GetCheck()?true:false;
			Global::configuration()._RecordUnarmed = m_record_unarmed.GetCheck()?true:false;
			//Global::configuration()._MoveCursorPaste = m_move_cursor_paste.GetCheck()?true:false;
			Global::configuration()._NavigationIgnoresStep = m_navigation_ignores_step.GetCheck()?true:false;
			
			Global::configuration().bShowSongInfoOnLoad = m_show_info.GetCheck()?true:false;
			Global::configuration().autosaveSong = m_autosave.GetCheck()?true:false;
			Global::configuration().pv_timesig = m_timesig.GetCurSel()+1;
			
			char buffer[32];
			m_numlines.GetWindowText(buffer,16);
			
			int nlines = atoi(buffer);

			if (nlines < 1)
				{ nlines = 1; }
			else if (nlines > MAX_LINES)
				{ nlines = MAX_LINES; }

			Global::configuration().defaultPatLines=nlines;

			m_autosave_mins.GetWindowText(buffer,16);
			nlines = atoi(buffer);
			if (nlines < 1)
			{ nlines = 1; }
			else if (nlines > 60)
			{ nlines = 60; }
			Global::configuration().autosaveSongTime = nlines;
			
			CDialog::OnOK();
		}

		/// restores key config data.
		///\return true on success, false on failure.
		//\todo : "return" ?? it's void...
		void CKeyConfigDlg::OnImportreg() 
		{
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
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			ofn.lpstrInitialDir = _skinPathBuf.c_str();
			// Display the Open dialog box. 
			
			if (GetOpenFileName(&ofn)==true)
			{
				FILE* hfile;
				if ((hfile=fopen(szFile,"r")) == NULL )
				{
					MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
					return;
				}
				char buf[512];
				while (fgets(buf, 512, hfile))
				{
					UINT i,j;
					if (strstr(buf,"Key["))
					{
						char *q = strchr(buf,91); // [
						if (q)
						{
							q++;
							j = atoi(q);
							char *p = strchr(q,93); // ]
							if (p)
							{
								p++;
								i = atoi(p);
								q = strchr(p,61); // =
								if (q)
								{
									q++;
									int cmddata = atoi(q);
									CmdSet ID = CmdSet(cmddata);
									UIGlobal::pInputHandler->SetCmd(ID,i,j);
								}
							}
						}
					}
				}
				fclose(hfile);
			}
			//FillCmdList();
			//OnSelchangeCmdlist();
			WORD key = 0;
			WORD mods = 0;	
			
			// update display for new key
			m_prvIdx = m_lstCmds.GetCurSel();
			FindKey(m_prvIdx,key,mods);
			m_hotkey0.SetHotKey(key,mods);
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
			ofn.lpstrInitialDir = _skinPathBuf.c_str();

			if (GetSaveFileName(&ofn)==true)
			{
				FILE* hfile;

				CString str = szFile;
				CString str2 = str.Right(4);
				if ( str2.CompareNoCase(".psk") != 0 ) str.Insert(str.GetLength(),".psk");
				sprintf(szFile,str);

				if(!(hfile=fopen(szFile,"wb")))
				{
					MessageBox("Couldn't open File for Writing. Operation Aborted","File Save Error",MB_OK);
					return;
				}

				fprintf(hfile,"[Psycle Keymap Presets v1.0]\n\n");
				for(UINT j=0;j<MOD_MAX;j++)
				{
					for(UINT i=0;i<256;i++)
					{
						if(UIGlobal::pInputHandler->cmdLUT[j][i].IsValid())
						{
							fprintf(hfile,"Key[%d]%03d=%03d     ; cmd = '%s'\n",j,i,UIGlobal::pInputHandler->cmdLUT[j][i].ID,UIGlobal::pInputHandler->cmdLUT[j][i].GetName());
						}
					}
				}
				fclose(hfile);
			}
		}

		void CKeyConfigDlg::OnDefaults() 
		{
			WORD key = 0;
			WORD mods = 0;
			UIGlobal::pInputHandler->BuildCmdLUT();
			FillCmdList();
			m_prvIdx = 0;
			m_lstCmds.SetCurSel(0);
			FindKey(0,key,mods);
			m_hotkey0.SetHotKey(key,mods);
		}

		void CKeyConfigDlg::OnNone() 
		{
			WORD key = 0;
			WORD mods = 0;	
			// save key settings for key we've just moved from
			FindKey(m_prvIdx,key,mods);
			UINT nKey=key;
			UINT nMod=0;
			if(mods&HOTKEYF_SHIFT)
				nMod|=MOD_S;
			if(mods&HOTKEYF_CONTROL)
				nMod|=MOD_C;
			if(mods&HOTKEYF_EXT)
				nMod|=MOD_E;
			UIGlobal::pInputHandler->SetCmd(cdefNull,nKey,nMod);
			m_hotkey0.SetHotKey(0,0);
		}

		void CKeyConfigDlg::OnUpdateNumLines() 
		{
			char buffer[256];
			if (bInit)
			{
				m_numlines.GetWindowText(buffer,16);
				int val=atoi(buffer);

				if (val < 0)
				{
					val = 0;
				}
				else if(val > MAX_LINES)
				{
					val = MAX_LINES-1;
				}
				sprintf(buffer,"HEX: %x",val);
				m_textlines.SetWindowText(buffer);
			}
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
