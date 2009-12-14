#include "project.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/player.h>
#include <psycle/audiodrivers/audiodriver.h>

#include "Psy3Saver.hpp"
using namespace psycle::core;
#else
#include "Player.hpp"
#endif

#include "ChildView.hpp"
#include "Configuration.hpp"
#include "PatternView.hpp"
#include "MachineView.hpp"
#include "MainFrm.hpp"
#include "MidiInput.hpp"
#include "ProjectData.hpp"
#include "SongpDlg.hpp"
#include "XMSongExport.hpp"
#include "XMSongLoader.hpp"
#include "ITModule2.h"

namespace psycle {
	namespace host {

		Project::Project(ProjectData* parent, PatternView* pat_view, MachineView* mac_view)
			: parent_(parent),
			  pat_view_(pat_view),
			  mac_view_(mac_view),
			  beat_zoom_(4)
		{
			assert(pat_view_);
			assert(mac_view_);
			song_.New();			
		}

		Project::~Project()
		{
		}

		PatternView* Project::pat_view()
		{
			return pat_view_;
		}

		MachineView* Project::mac_view()
		{
			return mac_view_;
		}

		ProjectData* Project::parent()
		{
			return parent_;
		}

		void Project::SetActive() {
			pat_view()->SetSong(&song());
			mac_view()->SetSong(&song());
		}

		void Project::Clear()
		{
			pat_view()->KillUndo();
			pat_view()->KillRedo();
			mac_view()->LockVu();
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			Player & player(Player::singleton());
			player.stop();
			///\todo lock/unlock
			Sleep(256);
			player.driver().Enable(false);

			song().New();

			player.driver().Enable(true);
			player.setBpm(song().BeatsPerMin());
			player.timeInfo().setTicksSpeed(song().LinesPerBeat(), song().isTicks());
			CMainFrame* pParentMain = mac_view()->main();
			pParentMain->m_wndSeq.UpdateSequencer();
			mac_view()->Rebuild();
			mac_view()->UnlockVu();
#else
			Global::pPlayer->stop();
			///\todo lock/unlock
			Sleep(256);
			mac_view()->child_view()->_outputActive = false;
			Global::pConfig->_pOutputDriver->Enable(false);
			// midi implementation
			Global::pConfig->_pMidiInput->Close();
			///\todo lock/unlock
			Sleep(256);

			song().New();

			mac_view()->child_view()->_outputActive = true;
			if (!Global::pConfig->_pOutputDriver->Enable(true))
			{
				mac_view()->child_view()->_outputActive = false;
			}
			else
			{
				// midi implementation
				Global::pConfig->_pMidiInput->Open();
			}
			Global::pPlayer->SetBPM(song().BeatsPerMin(),song().LinesPerBeat());
			mac_view()->child_view()->SetTitleBarText();
			pat_view()->editPosition=0;
			song().seqBus=0;
			CMainFrame* pParentMain = mac_view()->main();
			pParentMain->PsybarsUpdate(); // Updates all values of the bars
			pParentMain->WaveEditorBackUpdate();
			pParentMain->m_wndInst.WaveUpdate();
			pParentMain->RedrawGearRackList();
			pParentMain->m_wndSeq.UpdateSequencer();
			pParentMain->m_wndSeq.UpdatePlayOrder(false); // should be done always after updatesequencer
			//pParentMain->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
			pat_view()->RecalculateColourGrid();
			mac_view()->child_view()->Repaint();
			mac_view()->Rebuild();
			mac_view()->UnlockVu();
#endif
		}

		void Project::OnFileLoadsongNamed(const std::string& fName, int fType)
		{
			if( fType == 2 )
			{
				FILE* hFile=fopen(fName.c_str(),"rb");
				pat_view()->LoadBlock(hFile);
				fclose(hFile);
			}
			else
			{
				if (CheckUnsavedSong("Load Song"))
				{
					FileLoadsongNamed(fName);
				}
			}
		}

		void Project::FileLoadsongNamed(const std::string& fName)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			mac_view()->LockVu();
			Player & player(Player::singleton());
			player.stop();
			///\todo lock/unlock
			Sleep(256);
			player.driver().Enable(false);
			pat_view()->editPosition = 0;
			if(!song().load(fName.c_str())) {
				mac_view_->child_view()->MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
				return;			
			}			
			player.song(song());
			AppendToRecent(fName);
			std::string::size_type index = fName.rfind('\\');
			if (index != std::string::npos)
			{
				Global::pConfig->SetCurrentSongDir(fName.substr(0,index));
				song().fileName = fName.substr(index+1);
			}
			else
			{
				song().fileName = fName;
			}
		//	set_lines_per_beat(song().ticksSpeed());
			set_beat_zoom(song().ticksSpeed());
			player.driver().Enable(true);
			CMainFrame* pParentMain = mac_view()->main();
			pParentMain->m_wndSeq.UpdateSequencer();
			pat_view()->RecalculateColourGrid();
			mac_view()->Rebuild();
			mac_view()->UnlockVu();
#else
			CMainFrame* pParentMain = mac_view()->main();

			mac_view()->LockVu();
			Global::pPlayer->stop();			
			///\todo lock/unlock
			Sleep(256);
			mac_view()->child_view()->_outputActive = false;
			Global::pConfig->_pOutputDriver->Enable(false);
			// MIDI IMPLEMENTATION
			Global::pConfig->_pMidiInput->Close();
			///\todo lock/unlock
			Sleep(256);
			
			OldPsyFile file;
			if (!file.Open(fName.c_str()))
			{
				mac_view_->child_view()->MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
				return;
			}
			pat_view()->editPosition = 0;
			song().Load(&file);
			//file.Close(); <- load handles this
			song()._saved=true;
			AppendToRecent(fName);
			std::string::size_type index = fName.rfind('\\');
			if (index != std::string::npos)
			{
				Global::pConfig->SetCurrentSongDir(fName.substr(0,index));
				song().fileName = fName.substr(index+1);
			}
			else
			{
				song().fileName = fName;
			}
			Global::pPlayer->SetBPM(song().BeatsPerMin(), song().LinesPerBeat());
			mac_view()->child_view()->_outputActive = true;
			if (!Global::pConfig->_pOutputDriver->Enable(true))
			{
				mac_view()->child_view()->_outputActive = false;
			}
			else
			{
				// MIDI IMPLEMENTATION
				
				Global::pConfig->_pMidiInput->Open();
			}

			pParentMain->PsybarsUpdate();
			pParentMain->WaveEditorBackUpdate();
			pParentMain->m_wndInst.WaveUpdate();
			pParentMain->RedrawGearRackList();
			pParentMain->m_wndSeq.UpdateSequencer();
			pParentMain->m_wndSeq.UpdatePlayOrder(false);
			//pParentMain->UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
			pat_view()->RecalculateColourGrid();
			mac_view()->child_view()->Repaint();
			pat_view()->KillUndo();
			pat_view()->KillRedo();
			mac_view()->child_view()->SetTitleBarText();
			mac_view()->Rebuild();
			mac_view()->UnlockVu();
#endif
			if (Global::pConfig->bShowSongInfoOnLoad)
			{
				CSongpDlg dlg(&song());
				dlg.SetReadOnly();
				dlg.DoModal();
/*				std::ostringstream songLoaded;
				songLoaded << '\'' << _pSong->name << '\'' << std::endl
					<< std::endl
					<< _pSong->author << std::endl
					<< std::endl
					<< _pSong->comments;
				MessageBox(songLoaded.str().c_str(), "Psycle song loaded", MB_OK);
*/
			}
		}

		void Project::AppendToRecent(const std::string& fName)
		{
			int iCount;
			char* nameBuff;
			UINT nameSize;
			HMENU hFileMenu, hRootMenuBar;
			UINT ids[] =
				{
					ID_FILE_RECENT_01,
					ID_FILE_RECENT_02,
					ID_FILE_RECENT_03,
					ID_FILE_RECENT_04
				};
			MENUITEMINFO hNewItemInfo, hTempItemInfo;			
			hRootMenuBar = ::GetMenu(mac_view()->child_view()->GetParent()->m_hWnd);
			//pRootMenuBar = this->GetParent()->GetMenu();
			//hRootMenuBar = HMENU (*pRootMenuBar);
			hFileMenu = GetSubMenu(hRootMenuBar, 0);
			HMENU hRecentMenu = mac_view()->child_view()->hRecentMenu;
			hRecentMenu = GetSubMenu(hFileMenu, 11);
			// Remove initial empty element, if present.
			if(GetMenuItemID(hRecentMenu, 0) == ID_FILE_RECENT_NONE)
			{
				DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
			}
			// Check for duplicates and eventually remove.
			for(iCount = 0; iCount<GetMenuItemCount(hRecentMenu);iCount++)
			{
				nameSize = GetMenuString(hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
				nameBuff = new char[nameSize];
				GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
				if ( !strcmp(nameBuff, fName.c_str()) )
				{
					DeleteMenu(hRecentMenu, iCount, MF_BYPOSITION);
				}
				delete[] nameBuff;
			}
			// Ensure menu size doesn't exceed 4 positions.
			if (GetMenuItemCount(hRecentMenu) == 4)
			{
				DeleteMenu(hRecentMenu, 4-1, MF_BYPOSITION);
			}
			hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
			hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
			hNewItemInfo.fType		= MFT_STRING;
			hNewItemInfo.wID		= ids[0];
			hNewItemInfo.cch		= fName.length();
			hNewItemInfo.dwTypeData = (LPSTR)fName.c_str();
			InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);
			// Update identifiers.
			for(iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
			{
				hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
				hTempItemInfo.fMask		= MIIM_ID;
				hTempItemInfo.wID		= ids[iCount];
				SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
			}
			mac_view()->child_view()->hRecentMenu = hRecentMenu;
		}

		bool Project::CheckUnsavedSong(const std::string& title)
		{
			BOOL bChecked = pat_view()->CheckUnsavedSong() && mac_view()->CheckUnsavedSong();
			if (!bChecked)
			{
				if (Global::pConfig->bFileSaveReminders)
				{
					std::string filepath = Global::pConfig->GetCurrentSongDir();
					filepath += '\\';
					filepath += song().fileName;
					OldPsyFile file;
					std::ostringstream szText;
					szText << "Save changes to \"" << song().fileName
						<< "\"?";
					int result = mac_view_->child_view()->MessageBox(szText.str().c_str(),title.c_str(),MB_YESNOCANCEL | MB_ICONEXCLAMATION);
					switch (result)
					{
					case IDYES:
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						 //todo: fileformat selection in the filter selection.
						if ( ! song().save(filepath,3)) {
							std::ostringstream szText;
							szText << "Error writing to \"" << filepath << "\"!!!";
							mac_view_->child_view()->MessageBox(szText.str().c_str(),title.c_str(),MB_ICONEXCLAMATION);
							return FALSE;
						}
#else
						if (!file.Create((char*)filepath.c_str(), true))
						{
							std::ostringstream szText;
							szText << "Error writing to \"" << filepath << "\"!!!";
							mac_view_->child_view()->MessageBox(szText.str().c_str(),title.c_str(),MB_ICONEXCLAMATION);
							return FALSE;
						}
						song().Save(&file);
						//file.Close(); <- save handles this
#endif
						return TRUE;
						break;
					case IDNO:
						return TRUE;
						break;
					case IDCANCEL:
						return FALSE;
						break;
					}
				}
			}
			return TRUE;
		}

		void Project::FileImportModulefile()
		{
			CMainFrame* pParentMain = mac_view()->main();
			OPENFILENAME ofn; // common dialog box structure
			char szFile[_MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = mac_view()->child_view()->GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter =
				"All Module Songs (*.xm *.it *.s3m *.mod)" "\0" "*.xm;*.it;*.s3m;*.mod" "\0"
				"FastTracker II Songs (*.xm)"              "\0" "*.xm"                  "\0"
				"Impulse Tracker Songs (*.it)"             "\0" "*.it"                  "\0"
				"Scream Tracker Songs (*.s3m)"             "\0" "*.s3m"                 "\0"
				"Original Mod Format Songs (*.mod)"        "\0" "*.mod"                 "\0"
				"All (*)"                                  "\0" "*"                     "\0"
				;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn)==TRUE)
			{
				pat_view()->KillUndo();
				pat_view()->KillRedo();
				mac_view()->LockVu();
				Global::pPlayer->stop();
				///\todo lock/unlock
				Sleep(256);
				mac_view()->child_view()->_outputActive = false;
				Global::pConfig->_pOutputDriver->Enable(false);
				// MIDI IMPLEMENTATION
				Global::pConfig->_pMidiInput->Close();
				///\todo lock/unlock
				Sleep(256);

				CString str = ofn.lpstrFile;
				int index = str.ReverseFind('.');
				if (index != -1)
				{
					CString ext = str.Mid(index+1);
					if (ext.CompareNoCase("XM") == 0)
					{
						XMSongLoader xmfile;
						xmfile.Open(ofn.lpstrFile);
						song().New();
						pat_view()->editPosition=0;
						xmfile.Load(song());
						xmfile.Close();
						mac_view()->Rebuild();
						CSongpDlg dlg(&song());
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "XM file imported", MB_OK);
*/
					} else if (ext.CompareNoCase("IT") == 0)
					{
						ITModule2 it;
						it.Open(ofn.lpstrFile);
						song().New();
						pat_view()->editPosition=0;
						if(!it.LoadITModule(&song()))
						{			
							mac_view()->child_view()->MessageBox("Load failed");
							song().New();
							it.Close();
							return;
						}
						it.Close();
						mac_view()->Rebuild();
						CSongpDlg dlg(&song());
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "IT file imported", MB_OK);
*/
					} else if (ext.CompareNoCase("S3M") == 0)
					{
						ITModule2 s3m;
						s3m.Open(ofn.lpstrFile);
						song().New();
						pat_view()->editPosition=0;
						if(!s3m.LoadS3MModuleX(&song()))
						{			
							mac_view()->child_view()->MessageBox("Load failed");
							song().New();
							s3m.Close();
							return;
						}
						s3m.Close();
						mac_view()->Rebuild();
						CSongpDlg dlg(&song());
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "S3M file imported", MB_OK);
*/
					} else if (ext.CompareNoCase("MOD") == 0)
					{
						MODSongLoader modfile;
						modfile.Open(ofn.lpstrFile);
						song().New();
						pat_view()->editPosition=0;
						modfile.Load(song());
						modfile.Close();
						mac_view()->Rebuild();
						CSongpDlg dlg(&song());
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "MOD file imported", MB_OK);
*/
					}
				}

				str = ofn.lpstrFile;
				index = str.ReverseFind('\\');
				if (index != -1)
				{
					Global::pConfig->SetCurrentSongDir((LPCSTR)str.Left(index));
					song().fileName = str.Mid(index+1)+".psy";
				}
				else
				{
					song().fileName = str+".psy";
				}
				mac_view()->child_view()->_outputActive = true;
				if (!Global::pConfig->_pOutputDriver->Enable(true))
				{
					mac_view()->child_view()->_outputActive = false;
				}
				else
				{
					// MIDI IMPLEMENTATION
					Global::pConfig->_pMidiInput->Open();
				}
				pParentMain->PsybarsUpdate();
				pParentMain->WaveEditorBackUpdate();
				pParentMain->m_wndInst.WaveUpdate();
				pParentMain->RedrawGearRackList();
				pParentMain->m_wndSeq.UpdateSequencer();
				pParentMain->m_wndSeq.UpdatePlayOrder(false);
				pat_view()->RecalculateColourGrid();
				mac_view()->child_view()->Repaint();
				mac_view()->UnlockVu();
			}
			mac_view()->child_view()->SetTitleBarText();
		}

		bool Project::Export(UINT id)
		{
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = song().fileName.substr(0,song().fileName.length()-4) + ".xm";
			std::string if2 = ifile.substr(0,ifile.find_first_of("\\/:*\"<>|"));
			
			char szFile[_MAX_PATH];

			szFile[_MAX_PATH-1]=0;
			strncpy(szFile,if2.c_str(),_MAX_PATH-1);
			
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = mac_view()->child_view()->GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "FastTracker 2 Song (*.xm)\0*.xm\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = TRUE;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CString str = ofn.lpstrFile;

				CString str2 = str.Right(3);
				if ( str2.CompareNoCase(".xm") != 0 ) str.Insert(str.GetLength(),".xm");
				int index = str.ReverseFind('\\');
				XMSongExport file;

				if (index != -1)
				{
					Global::pConfig->SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
				}
				
				if (!file.Create(str.GetBuffer(1), true))
				{
					mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
					return FALSE;
				}
				file.exportsong(song());
				file.Close();
			}
			else
			{
				return FALSE;
			}
			return bResult;
		}


		bool Project::OnFileSave(UINT id)
		{
			//MessageBox("Saving Disabled");
			//return false;
			BOOL bResult = TRUE;
			if ( song()._saved )
			{
				if (mac_view()->child_view()->MessageBox("Proceed with Saving?","Song Save",MB_YESNO) == IDYES)
				{
					std::string filepath = Global::pConfig->GetCurrentSongDir();
					filepath += '\\';
					filepath += song().fileName;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					//todo: fileformat version
					// if (!song().save(filepath,3) ) {
					//	mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
					//	return FALSE;
					// }
					psycle::core::RiffFile file;
					if (!file.Create((char*)filepath.c_str(), true))
					{
						mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}
					Psy3Saver saver(song());
					if (!saver.Save(&file, true))
					{
						mac_view()->child_view()->MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}					
#else
					OldPsyFile file;
					if (!file.Create((char*)filepath.c_str(), true))
					{
						mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}
					if (!song().Save(&file))
					{
						mac_view()->child_view()->MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}
#endif
					else 
					{
						song()._saved=true;
						if (pat_view()->pUndoList)
						{
							pat_view()->UndoSaved = pat_view()->pUndoList->counter;
						}
						else
						{
							pat_view()->UndoSaved = 0;
						}
						mac_view()->child_view()->UndoMacSaved = mac_view()->child_view()->UndoMacCounter;
						mac_view()->child_view()->SetTitleBarText();
					}				
					//file.Close();  <- save handles this 
				}
				else 
				{
					return FALSE;
				}
			}
			else 
			{
				return OnFileSaveAs(0);
			}
			return bResult;

		}

		//////////////////////////////////////////////////////////////////////
		// "Save Song As" Function

		bool Project::OnFileSaveAs(UINT id)
		{
			//MessageBox("Saving Disabled");
			//return false;
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = song().fileName;
			std::string if2 = ifile.substr(0,ifile.find_first_of("\\/:*\"<>|"));
			
			char szFile[_MAX_PATH];

			szFile[_MAX_PATH-1]=0;
			strncpy(szFile,if2.c_str(),_MAX_PATH-1);
			
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = mac_view()->child_view()->GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Songs (*.psy)\0*.psy\0Psycle Pattern (*.psb)\0*.psb\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = TRUE;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CString str = ofn.lpstrFile;
				if ( ofn.nFilterIndex == 2 ) 
				{
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psb") != 0 )
						str.Insert(str.GetLength(),".psb");
					sprintf(szFile,str);
					FILE* hFile=fopen(szFile,"wb");
					pat_view()->SaveBlock(hFile);
					fflush(hFile);
					fclose(hFile);
				}
				else 
				{ 
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psy") != 0 )
						str.Insert(str.GetLength(),".psy");
					int index = str.ReverseFind('\\');
					OldPsyFile file;

					if (index != -1)
					{
						Global::pConfig->SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
						song().fileName = str.Mid(index+1);
					}
					else
					{
						song().fileName = str;
					}
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					//todo: fileformat version
					// if ( ! song().save(str.GetBuffer(1),3)){
					//	mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
					//	return FALSE;
					//}
					
						psycle::core::RiffFile file1;
						if (!file1.Create((char*)str.GetBuffer(1), true))
						{
							mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
							return FALSE;
						}
						Psy3Saver saver(song());
						if (!saver.Save(&file1, true))
						{
							mac_view()->child_view()->MessageBox("Error saving file!", "Error!", MB_OK);
							bResult = FALSE;
						}		
								
#else
					if (!file.Create(str.GetBuffer(1), true))
					{
						mac_view()->child_view()->MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}
					if (!song().Save(&file))
					{
						mac_view()->child_view()->MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}
#endif
					else 
					{
						song()._saved=true;
						AppendToRecent(str.GetBuffer(1));
						
						if (pat_view()->pUndoList)
						{
							pat_view()->UndoSaved = pat_view()->pUndoList->counter;
						}
						else
						{
							pat_view()->UndoSaved = 0;
						}
						mac_view()->child_view()->UndoMacSaved = mac_view()->child_view()->UndoMacCounter;
						mac_view()->child_view()->SetTitleBarText();
					}
					//file.Close(); <- save handles this
				}
			}
			else
			{
				return FALSE;
			}
			return bResult;
		}
	}
}
