#include "stdafx.h"
#include "psycle.h"
#include "NewMachine.h"
#include "Plugin.h"
#include "VstHost.h"
#include "ProgressDialog.h"
#include <string>
#include <sstream>
#include <fstream>
///\file
///\brief implementation file for psycle::host::CNewMachine.
namespace psycle
{
	namespace host
	{
		int CNewMachine::pluginOrder = 1;
		bool CNewMachine::pluginName = 1;
		int CNewMachine::_numPlugins = -1;
		int CNewMachine::LastType0=0;
		int CNewMachine::LastType1=0;

		PluginInfo* CNewMachine::_pPlugsInfo[MAX_BROWSER_PLUGINS];

		CMapStringToString CNewMachine::dllNames(MAX_BROWSER_PLUGINS);

		CNewMachine::CNewMachine(CWnd* pParent)
			: CDialog(CNewMachine::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CNewMachine)
			m_orderby = pluginOrder;
			m_showdllName = pluginName;
			//}}AFX_DATA_INIT
			OutBus = false;
			psOutputDll = 0;
			//pluginOrder = 0; Do NOT uncomment. It would cause the variable to be reseted each time.
				// It is initialized above, where it is declared.
		}

		CNewMachine::~CNewMachine()
		{
			if (psOutputDll != NULL)
			{
				delete psOutputDll;
			}
		}

		void CNewMachine::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CNewMachine)
			DDX_Control(pDX, IDC_CHECK_ALLOW, m_Allow);
			DDX_Control(pDX, IDC_NAMELABEL, m_nameLabel);
			DDX_Control(pDX, IDC_BROWSER, m_browser);
			DDX_Control(pDX, IDC_VERSIONLABEL, m_versionLabel);
			DDX_Control(pDX, IDC_DESCLABEL, m_descLabel);
			DDX_Radio(pDX, IDC_BYTYPE, m_orderby);
			DDX_Control(pDX, IDC_DLLNAMELABEL, m_dllnameLabel);
			DDX_Radio(pDX, IDC_SHOWDLLNAME, m_showdllName);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
			//{{AFX_MSG_MAP(CNewMachine)
			ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
			ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
			ON_BN_CLICKED(IDC_BYCLASS, OnByclass)
			ON_BN_CLICKED(IDC_BYTYPE, OnBytype)
			ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_SHOWDLLNAME, OnShowdllname)
			ON_BN_CLICKED(IDC_SHOWEFFNAME, OnShoweffname)
			ON_BN_CLICKED(IDC_CHECK_ALLOW, OnCheckAllow)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CNewMachine::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			if (imgList.GetSafeHandle() != 0)
			{
				imgList.DeleteImageList();
 			}
			imgList.Create(IDB_MACHINETYPE,16,2,1);
			m_browser.SetImageList(&imgList,TVSIL_NORMAL);

			bAllowChanged = FALSE;

			LoadPluginInfo();
			UpdateList();
			return TRUE;
		}

		void CNewMachine::OnDestroy() 
		{
			CDialog::OnDestroy();
			
			if (imgList.GetSafeHandle() != 0)
			{
				imgList.DeleteImageList();
			}
		}

		void CNewMachine::UpdateList(bool bInit)
		{
			int nodeindex;

			m_browser.DeleteAllItems();

			HTREEITEM intFxNode;

			if (pluginOrder == 0 )
			{
				hNodes[0] = m_browser.InsertItem("Internal Plugins",0,0 , TVI_ROOT, TVI_LAST);
				hNodes[1] = m_browser.InsertItem("Native plug-ins",2,2,TVI_ROOT,TVI_LAST);
				hNodes[2] = m_browser.InsertItem("VST2 plug-ins",4,4,TVI_ROOT,TVI_LAST);
				intFxNode = hNodes[0];
				nodeindex=3;

				//The following is unfinished. It is for nested branches.
		/*		int i=_numPlugins;	// I Search from the end because when creating the
									// array, the deepest dir is saved first.
				HTREEITEM curnode;
				int currdir = numDirs;
				while (i>=0)
				{
					if ( strcpy(_pPlugsInfo[i]->_pPlugsInfo[i]->dllname,dirArray[currdir]) != 0 )
					{
						currdir--:
						// check if you need to create a new node or what.
						// use m_browser.GetNextItem() to check where you are.
					}
					// do what it is under here, but with the correct "hNodes";

				}*/
				for (int i=_numPlugins-1; i>=0; i--)
				{
					if (_pPlugsInfo[i]->error_type == VSTINSTANCE_NO_ERROR)
					{
						int imgindex;
						HTREEITEM hitem;
						if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
						{
							if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
							{ 
								imgindex = 2; 
								hitem= hNodes[1]; 
							}
							else 
							{ 
								imgindex = 4; 
								hitem=hNodes[2]; 
							}
						}
						else
						{
							if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
							{ 
								imgindex = 3; 
								hitem= hNodes[1];
							}
							else 
							{ 
								imgindex = 5; 
								hitem=hNodes[2];
							}
						}
						if (pluginName)
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name, imgindex, imgindex, hitem, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname, imgindex, imgindex, hitem, TVI_SORT);
					}
				}
				hInt[0] = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
		//		hInt[1] = m_browser.InsertItem("'Dist!' Distortion",1,1,intFxNode,TVI_SORT);
		//		hInt[2] = m_browser.InsertItem("PsychOsc AM",1,1,intFxNode,TVI_SORT);
		//		hInt[3] = m_browser.InsertItem("Dalay Delay",1,1,intFxNode,TVI_SORT);
		//		hInt[4] = m_browser.InsertItem("2p Filter",1,1,intFxNode,TVI_SORT);
		//		hInt[5] = m_browser.InsertItem("Gainer",1,1,intFxNode,TVI_SORT);
		//		hInt[6] = m_browser.InsertItem("Flanger",1,1,intFxNode,TVI_SORT);
		//		hInt[7] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				hInt[1] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				m_browser.Select(hNodes[LastType0],TVGN_CARET);
			}
			else
			{
				hNodes[0] = m_browser.InsertItem("Generators",0,0 , TVI_ROOT, TVI_LAST);
				hNodes[1] = m_browser.InsertItem("Effects",1,1,TVI_ROOT,TVI_LAST);
				intFxNode = hNodes[1];
				nodeindex=2;
				for (int i=_numPlugins-1; i>=0; i--)// I Search from the end because when creating the
				{									// array, the deepest dir comes first.
					if (_pPlugsInfo[i]->error_type == VSTINSTANCE_NO_ERROR)
					{
						int imgindex;
						HTREEITEM hitem;
						if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
						{
							if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
							{ 
								imgindex = 2; 
								hitem= hNodes[0]; 
							}
							else 
							{ 
								imgindex = 4; 
								hitem=hNodes[0]; 
							}
						}
						else
						{
							if ( _pPlugsInfo[i]->type == MACH_PLUGIN ) 
							{ 
								imgindex = 3; 
								hitem= hNodes[1]; 
							}
							else 
							{ 
								imgindex = 5; 
								hitem=hNodes[1]; 
							}
						}
						if (pluginName)
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name, imgindex, imgindex, hitem, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname, imgindex, imgindex, hitem, TVI_SORT);
					}

				}
				hInt[0] = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
		//		hInt[1] = m_browser.InsertItem("'Dist!' Distortion",1,1,intFxNode,TVI_SORT);
		//		hInt[2] = m_browser.InsertItem("PsychOsc AM",1,1,intFxNode,TVI_SORT);
		//		hInt[3] = m_browser.InsertItem("Dalay Delay",1,1,intFxNode,TVI_SORT);
		//		hInt[4] = m_browser.InsertItem("2p Filter",1,1,intFxNode,TVI_SORT);
		//		hInt[5] = m_browser.InsertItem("Gainer",1,1,intFxNode,TVI_SORT);
		//		hInt[6] = m_browser.InsertItem("Flanger",1,1,intFxNode,TVI_SORT);
		//		hInt[7] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				hInt[1] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				m_browser.Select(hNodes[LastType1],TVGN_CARET);
			}

			Outputmachine = -1;

		}

		void CNewMachine::OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR; pNMTreeView; // not used

			tHand = m_browser.GetSelectedItem();

			Outputmachine = -1;
			OutBus = false;

			if (tHand == hInt[0])
			{
				m_nameLabel.SetWindowText("Sampler");
				m_descLabel.SetWindowText("Stereo Sampler Unit. Inserts new sampler.");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V0.5b");
				Outputmachine = MACH_SAMPLER;
				OutBus = true;
				LastType0 = 0;
				LastType1 = 0;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			
			/*
			// Effects
			if (tHand == hInt[1])
			{
				m_nameLabel.SetWindowText("'Dist!' Distortion");
				m_descLabel.SetWindowText("Threshold/Clamp digital distortion/booster machine");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V1.0");
				Outputmachine = MACH_DIST;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			
			if (tHand == hInt[2])
			{
				m_nameLabel.SetWindowText("PsychOsc Theremin/AM");
				m_descLabel.SetWindowText("Amplitude Modulator/Analog Theremin Emulator");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V0.3b");
				Outputmachine = MACH_SINE;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			
			if (tHand == hInt[3])
			{
				m_nameLabel.SetWindowText("Dalay Delay");
				m_descLabel.SetWindowText("Digital Delay Unit");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V1.1");
				Outputmachine = MACH_DELAY;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			
			if (tHand == hInt[4])
			{
				m_nameLabel.SetWindowText("2p Filter");
				m_descLabel.SetWindowText("Digital 2-Pole Filter");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V1.1");
				Outputmachine = MACH_2PFILTER;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			
			if (tHand == hInt[5])
			{
				m_nameLabel.SetWindowText("Gainer");
				m_descLabel.SetWindowText("Volume Gainer");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V1.0");
				Outputmachine = MACH_GAIN;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			
			if (tHand == hInt[6])
			{
				m_nameLabel.SetWindowText("Flanger");
				m_descLabel.SetWindowText("Variable delay flanger unit");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V1.0");
				Outputmachine = MACH_FLANGER;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			*/
			
			if (tHand == hInt[1])
			{
				m_nameLabel.SetWindowText("Dummy");
				m_descLabel.SetWindowText("Replaces inexistent plugins");
				m_dllnameLabel.SetWindowText("Internal Machine");
				m_versionLabel.SetWindowText("V1.0");
				Outputmachine = MACH_DUMMY;
				LastType0 = 0;
				LastType1 = 1;
				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}

			for (int i=0; i<_numPlugins; i++)
			{
				if (tHand == hPlug[i])
				{
					CString str = _pPlugsInfo[i]->dllname;
					char str2[256];
					strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
					m_dllnameLabel.SetWindowText(str2);
					m_nameLabel.SetWindowText(_pPlugsInfo[i]->name);
					m_descLabel.SetWindowText(_pPlugsInfo[i]->desc);
					m_versionLabel.SetWindowText(_pPlugsInfo[i]->version);
					if ( _pPlugsInfo[i]->type == MACH_PLUGIN )
					{
						Outputmachine = MACH_PLUGIN;
						LastType0 = 1;
						if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							OutBus = true;
							LastType1 = 0;
						}
						else
						{
							LastType1 = 1;
						}
					}
					else
					{
						LastType0 = 2;
						if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
						{
							Outputmachine = MACH_VST;
							OutBus = true;
							LastType1 = 0;
						}
						else
						{
							Outputmachine = MACH_VSTFX;
							LastType1 = 1;
						}
					}

					if (psOutputDll != NULL) delete psOutputDll;
					psOutputDll = new char[strlen(_pPlugsInfo[i]->dllname)+1];
					strcpy(psOutputDll,_pPlugsInfo[i]->dllname);

					m_Allow.SetCheck(!_pPlugsInfo[i]->allow);
					m_Allow.EnableWindow(TRUE);
				}
			}

			*pResult = 0;
		}

		void CNewMachine::OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			OnOK();	
			*pResult = 0;
		}

		void CNewMachine::OnRefresh() 
		{
			char cache[_MAX_PATH];
			GetModuleFileName(NULL,cache,_MAX_PATH);
			
			char *last = strrchr(cache,'\\');
			strcpy(last,"\\cache.map");

			DestroyPluginInfo();
			DeleteFile(cache);
			LoadPluginInfo();
			UpdateList();
			m_browser.Invalidate();
			SetFocus();
		}

		void CNewMachine::OnBytype() 
		{
			pluginOrder=0;
			UpdateList();
			m_browser.Invalidate();
		}
		void CNewMachine::OnByclass() 
		{
			pluginOrder=1;
			UpdateList();
			m_browser.Invalidate();
		}

		void CNewMachine::OnShowdllname() 
		{
			pluginName=false;	
			UpdateList();
			m_browser.Invalidate();
		}

		void CNewMachine::OnShoweffname() 
		{
			pluginName = true;
			UpdateList();
			m_browser.Invalidate();
		}

		void CNewMachine::LoadPluginInfo()
		{
			if(_numPlugins == -1)
			{
				::AfxGetApp()->DoWaitCursor(1); 
				int plugsCount(0);
				int badPlugsCount(0);
				_numPlugins = 0;
				bool progressOpen = !LoadCacheFile(plugsCount, badPlugsCount);
				CProgressDialog Progress;
				if(progressOpen)
				{
					Progress.Create();
					Progress.SetWindowText("Scanning plugins...");
					Progress.ShowWindow(SW_SHOW);
					class plugin_counter
					{
					public:
						const int operator()(const std::string & directory)
						{
							int count(0);
							::CFileFind finder;
							int loop = finder.FindFile(::CString((directory + "\\*").c_str()));
							while(loop)
							{								
								loop = finder.FindNextFile();
								if(finder.IsDirectory() && !finder.IsDots()) count += (*this)(std::string(finder.GetFilePath()));
								else
								{
									std::string s(finder.GetFilePath());
									if(s.substr(s.rfind('.')) == ".dll") ++count;
								}
							}
							finder.Close();
							return count;
						}
					};
					plugin_counter plugin_counter;
					Progress.m_Progress.SetStep
						(
							16384 /
							(
								plugin_counter(Global::pConfig->GetPluginDir()) +
								plugin_counter(Global::pConfig->GetVstDir())
							)
						);
					Progress.SetWindowText("Scanning plugins...."); // put a fourth dot
				}
				/* old code */	char logname[MAX_PATH];
				/* old code */	::GetModuleFileName(0, logname, sizeof logname);
				/* old code */	{
				/* old code */		char * last = strrchr(logname,'\\');
				/* old code */		std::strcpy(last, "\\plugin-scan.log.vanillia-c.txt");
				/* old code */	}
				/* old code */	::DeleteFile(logname);
				/* old code */	FILE * hfile(std::fopen(logname,"a"));
				/* old code */	std::fprintf(hfile,"[Psycle Plugin Enumeration Log]\n\nIf psycle is crashing on load, chances are it's a bad plugin, specifically the last item listed\n(if it has no comment after the -)\n\n");
				std::ofstream out;
				{
					std::string module_directory;
					{
						char module_file_name[MAX_PATH];
						::GetModuleFileName(0, module_file_name, sizeof module_file_name);
						module_directory = module_file_name;
						module_directory = module_directory.substr(0, module_directory.rfind('\\'));
					}
					out.open((module_directory + "/plugin-scan.log.c++-iostream.txt").c_str());
				}
				out << "[Psycle Plugin Enumeration Log]\n\nIf psycle is crashing on load, chances are it's a bad plugin, specifically the last item listed\n(if it has no comment after the -)" << std::endl;
				/* old code */	std::fprintf(hfile,"[Native Plugins]\n\n");
				/* old code */	std::fclose(hfile);
				out << std::endl << "[Native Plugins]" << std::endl << std::endl;
				out.flush();
				FindPluginsInDir(plugsCount, badPlugsCount, ::CString(Global::pConfig->GetPluginDir()), MACH_PLUGIN, out, progressOpen ? &Progress : 0);
				out.flush();
				/* old code */	hfile = std::fopen(logname,"a");
				/* old code */	std::fprintf(hfile, "\n[VST Plugins]\n\n");
				/* old code */	std::fclose(hfile);
				out << std::endl << "[VST Plugins]" << std::endl << std::endl;
				out.flush();
				FindPluginsInDir(plugsCount, badPlugsCount, ::CString(Global::pConfig->GetVstDir()), MACH_VST, out, progressOpen ? &Progress : 0);
				out.flush();
				out.close();
				_numPlugins = plugsCount;
				if(progressOpen) Progress.m_Progress.SetPos(16384);
				SaveCacheFile();
				if(progressOpen) Progress.OnCancel();
				::AfxGetApp()->DoWaitCursor(-1); 
			}
		}

		void CNewMachine::FindPluginsInDir(int & currentPlugsCount, int & currentBadPlugsCount, ::CString findDir, MachineType type, std::ostream & out, CProgressDialog * pProgress)
		{
			::CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*"); // check for subfolders.
			while(loop) 
			{								
				loop = finder.FindNextFile();
				if(finder.IsDirectory() && !finder.IsDots()) FindPluginsInDir(currentPlugsCount, currentBadPlugsCount, finder.GetFilePath(), type, out, pProgress);
			}
			finder.Close();

			/* old code */	char logname[MAX_PATH];
			/* old code */	::GetModuleFileName(0, logname, sizeof logname);
			/* old code */	{
			/* old code */		char * last = strrchr(logname,'\\');
			/* old code */		std::strcpy(last,"\\plugin-scan.log.vanillia-c.txt");
			/* old code */	}
			/* old code */	FILE * hfile(std::fopen(logname,"a"));

			int tmpCount = currentPlugsCount; tmpCount; // not used
			loop = finder.FindFile(findDir + "\\*.dll"); // check if the directory is empty
			while(loop)
			{
				if(pProgress)
				{
					pProgress->m_Progress.StepIt();
					::Sleep(1);
				}
				loop = finder.FindNextFile();

				::CString sDllName, tmpPath;
				sDllName = finder.GetFileName();
				sDllName.MakeLower();

				
				if(finder.IsDirectory()) continue; // Meaning : If it is not a directory and it is not in the cache ... <bohan> ???
				/* old code */	std::fprintf(hfile,finder.GetFilePath());
				/* old code */	std::fprintf(hfile," - ");
				/* old code */	std::fclose(hfile);
				/* old code */	hfile = std::fopen(logname, "a");
				out << finder.GetFilePath() << " - ";
				out.flush();

				FILETIME time;
				finder.GetLastWriteTime(&time);
				bool exists(false);
				for(int i(0) ; i < _numPlugins; ++i)
				{
					if(_pPlugsInfo[i])
					{
						if
							(
								_pPlugsInfo[i]->FileTime.dwHighDateTime == time.dwHighDateTime &&
								_pPlugsInfo[i]->FileTime.dwLowDateTime == time.dwLowDateTime
							)
						{
							if(std::strcmp(finder.GetFilePath(),_pPlugsInfo[i]->dllname) == 0)
							{
								exists = true;
								switch(_pPlugsInfo[i]->error_type)
								{
								case VSTINSTANCE_ERR_NO_VALID_FILE:
									/* old code */	std::fprintf(hfile,"* Previously Disabled - No File Error *");
									out << "* Previously Disabled - No File Error *";
									break;
								case VSTINSTANCE_ERR_NO_VST_PLUGIN:
									/* old code */	std::fprintf(hfile,"* Previously Disabled - Not Plugin Error *");
									out << "* Previously Disabled - Not Plugin Error *";
									break;
								case VSTINSTANCE_ERR_REJECTED:
									/* old code */	std::fprintf(hfile,"* Previously Disabled - Rejected Error *");
									out << "* Previously Disabled - Rejected Error *";
									break;
								case VSTINSTANCE_ERR_EXCEPTION:
									/* old code */	std::fprintf(hfile,"* Previously Disabled - Exception Error *");
									out << "* Previously Disabled - Exception Error *";
									break;
								case VSTINSTANCE_NO_ERROR:
									/* old code */	std::fprintf(hfile,"* Previously Mapped *");
									out << "* Previously Mapped *";
									break;
								default:
									/* old code */	std::fprintf(hfile,"* Disabled - Unknown Reason *");
									out << "* Disabled - Unknown Reason *";
									break;
								}
								out.flush();
								break;
							}
						}
					}
				}
				if(!exists)
				{
					if(type == MACH_PLUGIN )
					{
						Plugin plug(0);

						_pPlugsInfo[currentPlugsCount]= new PluginInfo;
						::ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

						_pPlugsInfo[currentPlugsCount]->dllname = new char[finder.GetFilePath().GetLength()+1];
						std::strcpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());

						FILETIME time;
						finder.GetLastWriteTime(&time);
						_pPlugsInfo[currentPlugsCount]->FileTime = time;

						_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;

						if(plug.Instance((char*)(const char*)finder.GetFilePath()))
						{
							_pPlugsInfo[currentPlugsCount]->error_type = VSTINSTANCE_NO_ERROR;
							_pPlugsInfo[currentPlugsCount]->allow = true;

							std::strcpy(_pPlugsInfo[currentPlugsCount]->name,plug.GetName());
							std::sprintf(_pPlugsInfo[currentPlugsCount]->desc, "%s by %s",
									(plug.IsSynth()) ? "Psycle instrument" : "Psycle effect",
									plug.GetAuthor());
							std::strcpy(_pPlugsInfo[currentPlugsCount]->version,"PsycleAPI 1.0");

							if( plug.IsSynth() ) 
							{
								_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
							}
							else 
							{
								_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
							}

							char str2[256];
							::CString str = _pPlugsInfo[currentPlugsCount]->dllname;
							str.MakeLower();
							std::strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
							dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

							/* old code */	std::fprintf(hfile,"*** NEW *** - ");
							/* old code */	std::fprintf(hfile,plug.GetName());
							out << "*** NEW *** - " << plug.GetName();
							out.flush();
						}
						else
						{
							_pPlugsInfo[currentPlugsCount]->error_type = VSTINSTANCE_ERR_NO_VST_PLUGIN;
							_pPlugsInfo[currentPlugsCount]->allow = false;

							sprintf(_pPlugsInfo[currentPlugsCount]->name,"???");
							sprintf(_pPlugsInfo[currentPlugsCount]->desc,"???");
							sprintf(_pPlugsInfo[currentPlugsCount]->version,"???");

							/* old code */	std::fprintf(hfile,"*** ERROR - NOT NATIVE PLUGIN OR BUGGED ***");
							/* old code */	out << "*** ERROR - NOT NATIVE PLUGIN OR BUGGED ***";
							out.flush();
						}
						currentPlugsCount++;
					}
					else if(type == MACH_VST)
					{
						VSTPlugin vstPlug;

						_pPlugsInfo[currentPlugsCount]= new PluginInfo;
						::ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

						_pPlugsInfo[currentPlugsCount]->dllname = new char[finder.GetFilePath().GetLength()+1];
						std::strcpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());

						FILETIME time;
						finder.GetLastWriteTime(&time);
						_pPlugsInfo[currentPlugsCount]->FileTime = time;

						_pPlugsInfo[currentPlugsCount]->type = MACH_VST;

						_pPlugsInfo[currentPlugsCount]->error_type = vstPlug.Instance((char*)(const char*)finder.GetFilePath());
						if(_pPlugsInfo[currentPlugsCount]->error_type == VSTINSTANCE_NO_ERROR)
						{
							_pPlugsInfo[currentPlugsCount]->allow = true;

							std::strcpy(_pPlugsInfo[currentPlugsCount]->name,vstPlug.GetName());
							std::sprintf(_pPlugsInfo[currentPlugsCount]->desc, "%s by %s",
								(vstPlug.IsSynth()) ? "VST2 instrument" : "VST2 effect",
								vstPlug.GetVendorName());
							std::sprintf(_pPlugsInfo[currentPlugsCount]->version,"%d",vstPlug.GetVersion());
							_pPlugsInfo[currentPlugsCount]->version[15]='\0';
							if(vstPlug.IsSynth()) 
							{
								_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
							}
							else 
							{
								_pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
							}

							char str2[256];
							::CString str = _pPlugsInfo[currentPlugsCount]->dllname;
							str.MakeLower();
							std::strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
							dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);

							/* old code */	std::fprintf(hfile,"*** NEW *** - ");
							/* old code */	std::fprintf(hfile,vstPlug.GetName());
							out << "*** NEW *** - " << vstPlug.GetName();
							out.flush();
						}
						else
						{
							_pPlugsInfo[currentPlugsCount]->allow = false;

							std::sprintf(_pPlugsInfo[currentPlugsCount]->name,"???");
							std::sprintf(_pPlugsInfo[currentPlugsCount]->desc,"???");
							std::sprintf(_pPlugsInfo[currentPlugsCount]->version,"???");

							switch (_pPlugsInfo[currentPlugsCount]->error_type)
							{
							case VSTINSTANCE_ERR_NO_VALID_FILE:
								/* old code */	std::fprintf(hfile,"*** ERROR - NO FILE ***");
								out << "*** ERROR - NO FILE ***";
								break;
							case VSTINSTANCE_ERR_NO_VST_PLUGIN:
								/* old code */	std::fprintf(hfile,"*** ERROR - NOT VST PLUGIN ***");
								out << "*** ERROR - NOT VST PLUGIN ***";
								break;
							case VSTINSTANCE_ERR_REJECTED:
								/* old code */	std::fprintf(hfile,"*** ERROR - REJECTED ***");
								out << "*** ERROR - REJECTED ***";
								break;
							case VSTINSTANCE_ERR_EXCEPTION:
								/* old code */	std::fprintf(hfile,"*** ERROR - EXCEPTION ***");
								out << "*** ERROR - EXCEPTION ***";
								break;
							default:
								/* old code */	std::fprintf(hfile,"*** ERROR - NOT VST PLUGIN OR BUGGED ***");
								out << "*** ERROR - NOT VST PLUGIN OR BUGGED ***";
								break;
							}
							out.flush();
						}
						++currentPlugsCount;
						// <bohan> vstPlug is a stack object, so its destructor is called
						// <bohan> at the end of its scope (this cope actually).
						// <bohan> The problem with destructors of any object of any class is that
						// <bohan> they are never allowed to throw any exception.
						// <bohan> So, we catch exceptions here by calling vstPlug.Free(); explicitly.
						try
						{
							vstPlug.Free();
						}
						catch(...)
						{
							/* old code */	std::fprintf(hfile, " ... ouch, exception when freeing the temporary instance");
							out << " ... ouch, exception when freeing the temporary instance";
							out.flush();
						}
					}
				}
				/* old code */	std::fprintf(hfile,"\n");
				out << std::endl;
				out.flush();
			}
			/* old code */	std::fclose(hfile);
			out.flush();
			finder.Close();
		}

		void CNewMachine::DestroyPluginInfo()
		{
			for (int i=0; i<_numPlugins; i++)
			{
				delete _pPlugsInfo[i];
			}
			dllNames.RemoveAll();
			_numPlugins = -1;
		}

		bool CNewMachine::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount)
		{
			char cache[_MAX_PATH];
			GetModuleFileName(NULL,cache,_MAX_PATH);
			
			char *last = strrchr(cache,'\\');
			strcpy(last,"\\cache.map");

			RiffFile file;
			CFileFind finder;

			if (!file.Open(cache)) 
			{
				return false;
			}

			char Temp[MAX_PATH];
			file.Read(Temp,8);
			Temp[8]=0;
			if (strcmp(Temp,"PSYCACHE")!=0)
			{
				file.Close();
				DeleteFile(cache);
				return false;
			}

			UINT version;
			file.Read(&version,sizeof(version));
			if (version > CURRENT_CACHE_MAP_VERSION)
			{
				file.Close();
				DeleteFile(cache);
				return false;
			}
			file.Read(&_numPlugins,sizeof(_numPlugins));

			for (int i = 0; i < _numPlugins; i++)
			{
				PluginInfo p;

				file.ReadString(Temp,sizeof(Temp));
				file.Read(&p.FileTime,sizeof(_pPlugsInfo[currentPlugsCount]->FileTime));
				file.Read(&p.error_type,sizeof(p.error_type));
				file.Read(&p.allow,sizeof(p.allow));
				file.Read(&p.mode,sizeof(p.mode));
				file.Read(&p.type,sizeof(p.type));
				file.ReadString(p.name,sizeof(p.name));
				file.ReadString(p.desc,sizeof(p.desc));
				file.ReadString(p.version,sizeof(p.version));

				if ( finder.FindFile(Temp) )
				{
					FILETIME time;
					finder.FindNextFile();
					if (finder.GetLastWriteTime(&time))
					{
						if ((p.FileTime.dwHighDateTime == time.dwHighDateTime)
							&& (p.FileTime.dwLowDateTime == time.dwLowDateTime))
						{
							_pPlugsInfo[currentPlugsCount]= new PluginInfo;
							ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

							_pPlugsInfo[currentPlugsCount]->dllname = new char[strlen(Temp)+1];
							strcpy(_pPlugsInfo[currentPlugsCount]->dllname,Temp);

							_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;

							_pPlugsInfo[currentPlugsCount]->error_type = p.error_type;
							_pPlugsInfo[currentPlugsCount]->allow = p.allow;

							_pPlugsInfo[currentPlugsCount]->mode = p.mode;
							_pPlugsInfo[currentPlugsCount]->type = p.type;
							strcpy(_pPlugsInfo[currentPlugsCount]->name,p.name);
							strcpy(_pPlugsInfo[currentPlugsCount]->desc,p.desc);
							strcpy(_pPlugsInfo[currentPlugsCount]->version,p.version);

							if (p.error_type == VSTINSTANCE_NO_ERROR)
							{
								char str2[MAX_PATH];
								CString str = _pPlugsInfo[currentPlugsCount]->dllname;
								str.MakeLower();
								strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
								dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);
							}

							currentPlugsCount++;
						}
					}
				}
			}

			file.Close();
			
			return true;
		}

		bool CNewMachine::SaveCacheFile()
		{
			char cache[_MAX_PATH];
			GetModuleFileName(NULL,cache,_MAX_PATH);
			
			char *last = strrchr(cache,'\\');
			strcpy(last,"\\cache.map");
			DeleteFile(cache);

			RiffFile file;
			if (!file.Create(cache,true)) 
			{
				return false;
			}

			file.Write("PSYCACHE",8);
			UINT version = CURRENT_CACHE_MAP_VERSION;
			file.Write(&version,sizeof(version));
			file.Write(&_numPlugins,sizeof(_numPlugins));

			for (int i=0; i<_numPlugins; i++ )
			{
				file.Write(_pPlugsInfo[i]->dllname,strlen(_pPlugsInfo[i]->dllname)+1);
				file.Write(&_pPlugsInfo[i]->FileTime,sizeof(_pPlugsInfo[i]->FileTime));
				file.Write(&_pPlugsInfo[i]->error_type,sizeof(_pPlugsInfo[i]->error_type));
				file.Write(&_pPlugsInfo[i]->allow,sizeof(_pPlugsInfo[i]->allow));
				file.Write(&_pPlugsInfo[i]->mode,sizeof(_pPlugsInfo[i]->mode));
				file.Write(&_pPlugsInfo[i]->type,sizeof(_pPlugsInfo[i]->type));
				file.Write(_pPlugsInfo[i]->name,strlen(_pPlugsInfo[i]->name)+1);
				file.Write(_pPlugsInfo[i]->desc,strlen(_pPlugsInfo[i]->desc)+1);
				file.Write(_pPlugsInfo[i]->version,strlen(_pPlugsInfo[i]->version)+1);
			}

			file.Close();
			return true;
		}

		void CNewMachine::OnOK() 
		{
			if (Outputmachine > -1) // Necessary so that you cannot doubleclick a Node
			{
				if (bAllowChanged)
				{
					SaveCacheFile();
				}
				CDialog::OnOK();
			}
		}

		void CNewMachine::OnCheckAllow() 
		{
			for (int i=0; i<_numPlugins; i++)
			{
				if (tHand == hPlug[i])
				{
					_pPlugsInfo[i]->allow = !m_Allow.GetCheck();
					bAllowChanged = TRUE;
				}
			}
		}

		bool CNewMachine::TestFilename(char* name)
		{
			for (int i=0; i<_numPlugins; i++)
			{
				if (strcmp(name,_pPlugsInfo[i]->dllname)==0)
				{
					// bad plugins always have allow = false
					if (_pPlugsInfo[i]->allow)
					{
						return TRUE;
					}
					char buf[256];
					switch (_pPlugsInfo[i]->error_type)
					{
					case VSTINSTANCE_ERR_NO_VALID_FILE:
						sprintf(buf,"%s is Disabled - No File Error.  Try to Load Anyway?",name);
						break;
					case VSTINSTANCE_ERR_NO_VST_PLUGIN:
						sprintf(buf,"%s is Disabled - Not Plugin Error.  Try to Load Anyway?",name);
						break;
					case VSTINSTANCE_ERR_REJECTED:
						sprintf(buf,"%s is Disabled - Rejected Error  Try to Load Anyway?",name);
						break;
					case VSTINSTANCE_ERR_EXCEPTION:
						sprintf(buf,"%s is Disabled - Exception Error  Try to Load Anyway?",name);
						break;
					case VSTINSTANCE_NO_ERROR:
						sprintf(buf,"%s is Disabled by User.  Try to Load Anyway?",name);
						break;
					default:
						sprintf(buf,"%s is Disabled - Unknown Reason  Try to Load Anyway?",name);
						break;
					}
					return (::MessageBox(NULL,buf,"Plugin Warning!",MB_YESNO) == IDYES);

				}
			}
			return FALSE;
		}
	}
}
