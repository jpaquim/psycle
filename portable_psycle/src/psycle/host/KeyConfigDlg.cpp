///\file
///\brief implementation file for psycle::host::CKeyConfigDlg.
#include <project.private.hpp>
#include "psycle.hpp"
#include "KeyConfigDlg.hpp"
#include "inputhandler.hpp"
#include "Configuration.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		IMPLEMENT_DYNCREATE(CKeyConfigDlg, CPropertyPage)

		CKeyConfigDlg::CKeyConfigDlg() : CPropertyPage(CKeyConfigDlg::IDD)
		{
			m_prvIdx = 0;
			//{{AFX_DATA_INIT(CKeyConfigDlg)
			//}}AFX_DATA_INIT
			bInit = FALSE;
		}

		void CKeyConfigDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CKeyConfigDlg)
			DDX_Control(pDX, IDC_AUTOSAVE_MINS_SPIN, m_autosave_spin);
			DDX_Control(pDX, IDC_AUTOSAVE_MINS, m_autosave_mins);
			DDX_Control(pDX, IDC_AUTOSAVE_CURRENT_SONG, m_autosave);
			DDX_Control(pDX, IDC_FILE_SAVE_REMINDERS, m_save_reminders);
			DDX_Control(pDX, IDC_TWEAK_SMOOTH, m_tweak_smooth);
			DDX_Control(pDX, IDC_RECORD_UNARMED, m_record_unarmed);
			DDX_Control(pDX, IDC_MOVE_CURSOR_PASTE, m_move_cursor_paste);
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
		END_MESSAGE_MAP()

		void CKeyConfigDlg::DoCommandList()
		{
		}

		void CKeyConfigDlg::FillCmdList()
		{
			// clear list box
			m_lstCmds.ResetContent();

			// add command definitions
			InputHandler* pinp = Global::pInputHandler;
			int j,i;

			for(j=0;j<MOD_MAX;j++)
			{
				for(i=0;i<256;i++)
				{
					if(pinp->cmdLUT[j][i].IsValid())
					{
						m_lstCmds.AddString(LPCTSTR(pinp->cmdLUT[j][i].GetName()));
					}
				}
			}
		}

		BOOL CKeyConfigDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			m_cmdCtrlPlay.SetCheck(Global::pInputHandler->bCtrlPlay?1:0);
			m_cmdNewHomeBehaviour.SetCheck(Global::pInputHandler->bFT2HomeBehaviour?1:0);
			m_cmdFT2Del.SetCheck(Global::pInputHandler->bFT2DelBehaviour?1:0);
			m_cmdShiftArrows.SetCheck(Global::pInputHandler->bShiftArrowsDoSelect?1:0);

			m_save_reminders.SetCheck(Global::pConfig->bFileSaveReminders?1:0);
			m_tweak_smooth.SetCheck(Global::pConfig->_RecordMouseTweaksSmooth?1:0);
			m_record_unarmed.SetCheck(Global::pConfig->_RecordUnarmed?1:0);
			m_move_cursor_paste.SetCheck(Global::pConfig->_MoveCursorPaste?1:0);
			m_navigation_ignores_step.SetCheck(Global::pConfig->_NavigationIgnoresStep?1:0);
			m_show_info.SetCheck(Global::pConfig->bShowSongInfoOnLoad?1:0);
			m_autosave.SetCheck(Global::pConfig->autosaveSong?1:0);
			
			// prevent ALT in hotkey	

			//TRACE("%d %d %d %d %d %d\n",HKCOMB_A,HKCOMB_C,HKCOMB_S,HKCOMB_CA,HKCOMB_SA,HKCOMB_SCA);

			WORD rules=HKCOMB_A|HKCOMB_CA|HKCOMB_SA|HKCOMB_SCA;
			WORD subst=0;

			m_hotkey0.SetRules(rules,subst);

			m_wrap.SetCheck(Global::pConfig->_wrapAround?1:0);
			m_centercursor.SetCheck(Global::pConfig->_centerCursor?1:0);
			m_cursordown.SetCheck(Global::pConfig->_cursorAlwaysDown?1:0);

			m_spinlines.SetRange(1,MAX_LINES);
			m_autosave_spin.SetRange(1,60);
			
			char buffer[16];
			itoa(Global::pConfig->defaultPatLines,buffer,10);
			m_numlines.SetWindowText(buffer);
			itoa(Global::pConfig->autosaveSongTime,buffer,10);
			m_autosave_mins.SetWindowText(buffer);
			
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

		/// update key on show
		void CKeyConfigDlg::OnSelchangeCmdlist() 
		{	
			
			WORD key = 0;
			WORD mods = 0;	
			
			// save key settings for key we've just moved from
			FindKey(m_prvIdx,key,mods);
			SaveHotKey(m_prvIdx,key,mods);
			
			// update display for new key
			m_prvIdx = m_lstCmds.GetCurSel();
			FindKey(m_prvIdx,key,mods);
			m_hotkey0.SetHotKey(key,mods);
		}

		void CKeyConfigDlg::SaveHotKey(long idx,WORD&key,WORD&mods)
		{
			// what key is selected in the hotkey?
			WORD hotkey_key = 0;
			WORD hotkey_mods = 0;
			m_hotkey0.GetHotKey(hotkey_key,hotkey_mods);

			// nothing selected? abort
			if(!hotkey_key)
				return;		
			
			UINT nKey=hotkey_key;
			UINT nMod=0;
			if(hotkey_mods&HOTKEYF_SHIFT)
				nMod|=MOD_S;
			if(hotkey_mods&HOTKEYF_CONTROL)
				nMod|=MOD_C;
			if(hotkey_mods&HOTKEYF_EXT)
				nMod|=MOD_E;

			// same as previous? do nothing
			if(nKey==key && nMod==mods)
				return;

			// what command is selected?
			CmdDef cmd = FindCmd(idx);
			
			// save key definition
			if(cmd.IsValid())
				Global::pInputHandler->SetCmd(cmd,nKey,nMod);
		}

		void CKeyConfigDlg::FindKey(long idx,WORD&key,WORD&mods)
		{
			// what command is selected?
			CmdDef cmd = FindCmd(idx);
			
			// locate which key is that command
			if(cmd.IsValid())
				Global::pInputHandler->CmdToKey(cmd,key,mods);
		}

		CmdDef CKeyConfigDlg::FindCmd(long idx)
		{	
			// init
			CmdDef cmd;

			// sanity check
			if(idx<0) 
				return cmd;				

			// get command string
			CString cmdDesc;
			m_lstCmds.GetText(idx,cmdDesc);

			// convert string to cmd
			cmd = Global::pInputHandler->StringToCmd(cmdDesc);
			return cmd;
		}

		void CKeyConfigDlg::OnCancel() 
		{
			// user cancelled,
			// restore from saved settings
			Global::pInputHandler->ConfigRestore();
			CDialog::OnCancel();
		}

		void CKeyConfigDlg::OnOK() 
		{
			// update last key
			OnSelchangeCmdlist();

			Global::pInputHandler->bCtrlPlay  = m_cmdCtrlPlay.GetCheck()?true:false;
			Global::pInputHandler->bFT2HomeBehaviour = m_cmdNewHomeBehaviour.GetCheck()?true:false;
			Global::pInputHandler->bFT2DelBehaviour = m_cmdFT2Del.GetCheck()?true:false;
			Global::pInputHandler->bShiftArrowsDoSelect = m_cmdShiftArrows.GetCheck()?true:false;
			
			// save settings
			Global::pInputHandler->ConfigSave();
			
			Global::pConfig->_wrapAround = m_wrap.GetCheck()?true:false;
			Global::pConfig->_centerCursor = m_centercursor.GetCheck()?true:false;
			Global::pConfig->_cursorAlwaysDown = m_cursordown.GetCheck()?true:false;

			Global::pConfig->bFileSaveReminders = m_save_reminders.GetCheck()?true:false;
			Global::pConfig->_RecordMouseTweaksSmooth = m_tweak_smooth.GetCheck()?true:false;
			Global::pConfig->_RecordUnarmed = m_record_unarmed.GetCheck()?true:false;
			Global::pConfig->_MoveCursorPaste = m_move_cursor_paste.GetCheck()?true:false;
			Global::pConfig->_NavigationIgnoresStep = m_navigation_ignores_step.GetCheck()?true:false;
			
			Global::pConfig->bShowSongInfoOnLoad = m_show_info.GetCheck()?true:false;
			Global::pConfig->autosaveSong = m_autosave.GetCheck()?true:false;
			
			char buffer[32];
			m_numlines.GetWindowText(buffer,16);
			
			int nlines = atoi(buffer);

			if (nlines < 1)
				{ nlines = 1; }
			else if (nlines > MAX_LINES)
				{ nlines = MAX_LINES; }

			Global::pConfig->defaultPatLines=nlines;

			m_autosave_mins.GetWindowText(buffer,16);
			nlines = atoi(buffer);
			if (nlines < 1)
			{ nlines = 1; }
			else if (nlines > 60)
			{ nlines = 60; }
			Global::pConfig->autosaveSongTime = nlines;
			
			CDialog::OnOK();
		}

		void CKeyConfigDlg::OnImportreg() 
		{
			// restore key config data
			// returns true on success, false on fail 
			// TODO: Add your control notification handler code here
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
			ofn.lpstrInitialDir = _skinPathBuf.c_str();
			// Display the Open dialog box. 
			
			if (GetOpenFileName(&ofn)==TRUE)
			{
				FILE* hfile;
				if ((hfile=fopen(szFile,"rw")) == NULL )
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
									Global::pInputHandler->SetCmd(ID,i,j);
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

			if (GetSaveFileName(&ofn)==TRUE)
			{
				FILE* hfile;

				CString str = szFile;
				CString str2 = str.Right(4);
				if ( str2.CompareNoCase(".psk") != 0 ) str.Insert(str.GetLength(),".psk");
				sprintf(szFile,str);
				DeleteFile(szFile);

				if ((hfile=fopen(szFile,"wa")) == NULL ) // file does not exist.
				{
					MessageBox("Couldn't open File for Writing. Operation Aborted","File Save Error",MB_OK);
					return;
				}

				fprintf(hfile,"[Psycle Keymap Presets v1.0]\n\n");
				for(UINT j=0;j<MOD_MAX;j++)
				{
					for(UINT i=0;i<256;i++)
					{
						if(Global::pInputHandler->cmdLUT[j][i].IsValid())
						{
							fprintf(hfile,"Key[%d]%03d=%03d     ; cmd = '%s'\n",j,i,Global::pInputHandler->cmdLUT[j][i].ID,Global::pInputHandler->cmdLUT[j][i].GetName());
						}
					}
				}
				fclose(hfile);
			}
		}

		void CKeyConfigDlg::OnDefaults() 
		{
			Global::pInputHandler->BuildCmdLUT();
			WORD key = 0;
			WORD mods = 0;	
			// update display for new key
			m_prvIdx = m_lstCmds.GetCurSel();
			FindKey(m_prvIdx,key,mods);
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
			Global::pInputHandler->SetCmd(cdefNull,nKey,nMod);
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
	NAMESPACE__END
NAMESPACE__END
