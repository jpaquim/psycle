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
			if(psOutputDll) delete psOutputDll;
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
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			imgList.Create(IDB_MACHINETYPE,16,2,1);
			m_browser.SetImageList(&imgList,TVSIL_NORMAL);
			bAllowChanged = false;
			LoadPluginInfo();
			UpdateList();
			return TRUE;
		}

		void CNewMachine::OnDestroy() 
		{
			CDialog::OnDestroy();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
		}

		void CNewMachine::UpdateList(bool bInit)
		{
			int nodeindex;
			m_browser.DeleteAllItems();
			HTREEITEM intFxNode;
			if(!pluginOrder)
			{
				hNodes[0] = m_browser.InsertItem("Internal Plugins",0,0 , TVI_ROOT, TVI_LAST);
				hNodes[1] = m_browser.InsertItem("Native plug-ins",2,2,TVI_ROOT,TVI_LAST);
				hNodes[2] = m_browser.InsertItem("VST2 plug-ins",4,4,TVI_ROOT,TVI_LAST);
				intFxNode = hNodes[0];
				nodeindex = 3;
				//The following is unfinished. It is for nested branches.
				/*
				int i=_numPlugins;	// I Search from the end because when creating the array, the deepest dir is saved first.
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

				}
				*/
				for(int i(_numPlugins - 1) ; i >= 0 ; --i)
				{
					if(!_pPlugsInfo[i]->error)
					{
						int imgindex;
						HTREEITEM hitem;
						if( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if( _pPlugsInfo[i]->type == MACH_PLUGIN) 
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
							if( _pPlugsInfo[i]->type == MACH_PLUGIN) 
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
						if(pluginName)
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name, imgindex, imgindex, hitem, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname, imgindex, imgindex, hitem, TVI_SORT);
					}
				}
				hInt[0] = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
				hInt[1] = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				m_browser.Select(hNodes[LastType0],TVGN_CARET);
			}
			else
			{
				hNodes[0] = m_browser.InsertItem("Generators",0,0 , TVI_ROOT, TVI_LAST);
				hNodes[1] = m_browser.InsertItem("Effects",1,1,TVI_ROOT,TVI_LAST);
				intFxNode = hNodes[1];
				nodeindex=2;
				for(int i(_numPlugins - 1) ; i >= 0 ; --i) // I Search from the end because when creating the array, the deepest dir comes first.
				{
					if(!_pPlugsInfo[i]->error)
					{
						int imgindex;
						HTREEITEM hitem;
						if(_pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
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
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
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
						if(pluginName)
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name, imgindex, imgindex, hitem, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname, imgindex, imgindex, hitem, TVI_SORT);
					}

				}
				hInt[0] = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
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
			DestroyPluginInfo();
			{
				char cache[1 << 10];
				GetModuleFileName(0, cache, sizeof cache);
				char * last(std::strrchr(cache,'\\'));
				std::strcpy(last, "\\plugin-scan.cache");
				DeleteFile(cache);
			}
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
				host::logger(host::logger::info, "Scanning plugins ...");
				::AfxGetApp()->DoWaitCursor(1); 
				int plugsCount(0);
				int badPlugsCount(0);
				_numPlugins = 0;
				bool progressOpen = !LoadCacheFile(plugsCount, badPlugsCount);
				CProgressDialog Progress;
				int plugin_count(0);
				if(progressOpen)
				{
					host::logger(host::logger::info, "Scanning plugins ... Counting ...");
					Progress.Create();
					Progress.SetWindowText("Scanning plugins ... Counting ...");
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
					plugin_count =
						plugin_counter(Global::pConfig->GetPluginDir()) +
						plugin_counter(Global::pConfig->GetVstDir());
					Progress.m_Progress.SetStep(16384 / plugin_count);
					{
						std::ostringstream s; s << "Scanning plugins ... Counted " << plugin_count << " plugins.";
						host::logger(host::logger::info, s.str().c_str());
						Progress.SetWindowText(s.str().c_str());
					}
				}
				std::ofstream out;
				{
					std::string module_directory;
					{
						char module_file_name[MAX_PATH];
						::GetModuleFileName(0, module_file_name, sizeof module_file_name);
						module_directory = module_file_name;
						module_directory = module_directory.substr(0, module_directory.rfind('\\'));
					}
					out.open((module_directory + "/plugin-scan.log.txt").c_str());
				}
				out
					<< "==========================================" << std::endl
					<< "=== Psycle Plugin Scan Enumeration Log ===" << std::endl
					<< std::endl
					<< "If psycle is crashing on load, chances are it's a bad plugin, "
					<< "specifically the last item listed, if it has no comment after the library file name." << std::endl;
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Natives ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::logger(host::logger::info, "Scanning plugins ... Natives");
				out
					<< std::endl
					<< "======================" << std::endl
					<< "=== Native Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPluginsInDir(plugsCount, badPlugsCount, ::CString(Global::pConfig->GetPluginDir()), MACH_PLUGIN, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... VST ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::logger(host::logger::info, "Scanning plugins ... VST");
				out
					<< std::endl
					<< "===================" << std::endl
					<< "=== VST Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPluginsInDir(plugsCount, badPlugsCount, ::CString(Global::pConfig->GetVstDir()), MACH_VST, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanned " << plugin_count << " plugins.";
					host::logger(host::logger::info, s.str().c_str());
					Progress.SetWindowText(s.str().c_str());
				}
				out.close();
				_numPlugins = plugsCount;
				if(progressOpen)
				{
					Progress.m_Progress.SetPos(16384);
					Progress.SetWindowText("Saving scan cache file ...");
				}
				host::logger(host::logger::info, "Saving scan cache file ...");
				SaveCacheFile();
				if(progressOpen) Progress.OnCancel();
				::AfxGetApp()->DoWaitCursor(-1); 
				host::logger(host::logger::info, "Done.");
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
				out << finder.GetFilePath() << " ... ";
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
							if(std::strcmp(finder.GetFilePath(), _pPlugsInfo[i]->dllname) == 0)
							{
								exists = true;
								const std::string * error(_pPlugsInfo[i]->error);
								std::stringstream s;
								if(!error) s << "cached.";
								else s << "cache says it has previously been disabled because:" << std::endl << *error << std::endl;
								out << s.str().c_str();
								out.flush();
								host::logger(host::logger::info, std::string(finder.GetFilePath()) + '\n' + s.str().c_str());
								break;
							}
						}
					}
				}
				if(!exists)
				{
					try
					{
						out << "new plugin added to cache ; ";
						out.flush();
						host::logger(host::logger::info, std::string(finder.GetFilePath()) + "\nnew plugin added to cache");
						_pPlugsInfo[currentPlugsCount]= new PluginInfo;
						::ZeroMemory(_pPlugsInfo[currentPlugsCount], sizeof(PluginInfo));
						_pPlugsInfo[currentPlugsCount]->dllname = new char[finder.GetFilePath().GetLength()+1];
						std::strcpy(_pPlugsInfo[currentPlugsCount]->dllname,finder.GetFilePath());
						FILETIME time;
						finder.GetLastWriteTime(&time);
						_pPlugsInfo[currentPlugsCount]->FileTime = time;
						if(type == MACH_PLUGIN)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;
							Plugin plug(0);
							try
							{
								 plug.Instance((char*)(const char*)finder.GetFilePath());
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s
									<< typeid(e).name() << std::endl
									<< e.what() << std::endl;
								out << s.str();
								out.flush();
								_pPlugsInfo[currentPlugsCount]->error = new std::string(s.str());
							}
							catch(...)
							{
								std::ostringstream s; s
									<< "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = new std::string(s.str());
							}
							if(_pPlugsInfo[currentPlugsCount]->error)
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << *_pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << finder.GetFilePath();
								host::logger(host::logger::crash, title.str() + '\n' + *_pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								std::sprintf(_pPlugsInfo[currentPlugsCount]->name, "???");
								std::sprintf(_pPlugsInfo[currentPlugsCount]->desc, "???");
								std::sprintf(_pPlugsInfo[currentPlugsCount]->version, "???");
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								std::strcpy(_pPlugsInfo[currentPlugsCount]->name,plug.GetName());
								std::sprintf
									(
										_pPlugsInfo[currentPlugsCount]->desc, "%s by %s",
										plug.IsSynth() ? "Psycle instrument" : "Psycle effect",
										plug.GetAuthor()
									);
								std::strcpy(_pPlugsInfo[currentPlugsCount]->version, "could be any");
								if(plug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
								char str2[1 << 10];
								::CString str = _pPlugsInfo[currentPlugsCount]->dllname;
								str.MakeLower();
								std::strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
								dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);
								out << plug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// <bohan> plug is a stack object, so its destructor is called
							// <bohan> at the end of its scope (this cope actually).
							// <bohan> The problem with destructors of any object of any class is that
							// <bohan> they are never allowed to throw any exception.
							// <bohan> So, we catch exceptions here by calling plug.Free(); explicitly.
							try
							{
								plug.Free();
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl
									<< e.what();
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << finder.GetFilePath();
								host::logger(host::logger::crash, title.str() + '\n' + s.str());
							}
							catch(...)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< "Type of exception is unknown, no further information available.";
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << finder.GetFilePath();
								host::logger(host::logger::crash, title.str() + '\n' + s.str());
							}
						}
						else if(type == MACH_VST)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_VST;
							vst::plugin vstPlug;
							try
							{
								 vstPlug.Instance((char*)(const char*)finder.GetFilePath());
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s
									<< typeid(e).name() << std::endl
									<< e.what() << std::endl;
								out << s.str();
								out.flush();
								_pPlugsInfo[currentPlugsCount]->error = new std::string(s.str());
							}
							catch(...)
							{
								std::ostringstream s; s
									<< "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = new std::string(s.str());
							}
							if(_pPlugsInfo[currentPlugsCount]->error)
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << *_pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << finder.GetFilePath();
								host::logger(host::logger::crash, title.str() + '\n' + *_pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								std::sprintf(_pPlugsInfo[currentPlugsCount]->name, "???");
								std::sprintf(_pPlugsInfo[currentPlugsCount]->desc, "???");
								std::sprintf(_pPlugsInfo[currentPlugsCount]->version, "???");
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								std::strcpy(_pPlugsInfo[currentPlugsCount]->name,vstPlug.GetName());
								std::sprintf
									(
										_pPlugsInfo[currentPlugsCount]->desc, "%s by %s",
										vstPlug.IsSynth() ? "VST2 instrument" : "VST2 effect",
										vstPlug.GetVendorName()
									);
								std::sprintf(_pPlugsInfo[currentPlugsCount]->version,"%d", vstPlug.GetVersion());
								_pPlugsInfo[currentPlugsCount]->version[15]='\0';
								if(vstPlug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
								char str2[1 << 10];
								::CString str = _pPlugsInfo[currentPlugsCount]->dllname;
								str.MakeLower();
								std::strcpy(str2, str.Mid(str.ReverseFind('\\') + 1));
								dllNames.SetAt(str2, _pPlugsInfo[currentPlugsCount]->dllname);
								out << vstPlug.GetName() << " - successfully instanciated";
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
								// <bohan> phatmatik crashes here...
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl
									<< e.what();
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << finder.GetFilePath();
								host::logger(host::logger::crash, title.str() + '\n' + s.str());
							}
							catch(...)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< "Type of exception is unknown, no further information available.";
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << finder.GetFilePath();
								host::logger(host::logger::crash, title.str() + '\n' + s.str());
							}
						}
					}
					catch(...)
					{
						out
							<< std::endl
							<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################";
						out.flush();
						host::logger(host::logger::crash, "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################");
					}
				}
				out << std::endl;
				out.flush();
			}
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
			strcpy(last,"\\plugin-scan.cache");

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
				{
					UINT size;
					file.Read(&size, sizeof size);
					if(size)
					{
						char * const chars(new char[size] + 1);
						file.Read(chars, size);
						chars[size] = '\0';
						p.error = new std::string(chars);
					}
				}
				file.Read(&p.allow,sizeof(p.allow));
				file.Read(&p.mode,sizeof(p.mode));
				file.Read(&p.type,sizeof(p.type));
				file.ReadString(p.name,sizeof(p.name));
				file.ReadString(p.desc,sizeof(p.desc));
				file.ReadString(p.version,sizeof(p.version));
				if(finder.FindFile(Temp))
				{
					FILETIME time;
					finder.FindNextFile();
					if (finder.GetLastWriteTime(&time))
					{
						if
							(
								p.FileTime.dwHighDateTime == time.dwHighDateTime &&
								p.FileTime.dwLowDateTime == time.dwLowDateTime
							)
						{
							_pPlugsInfo[currentPlugsCount]= new PluginInfo;
							ZeroMemory(_pPlugsInfo[currentPlugsCount],sizeof(PluginInfo));

							_pPlugsInfo[currentPlugsCount]->dllname = new char[strlen(Temp)+1];
							strcpy(_pPlugsInfo[currentPlugsCount]->dllname,Temp);

							_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;

							///\todo this cool be better handled
							if(_pPlugsInfo[currentPlugsCount]->error)
							{
								delete _pPlugsInfo[currentPlugsCount]->error;
								_pPlugsInfo[currentPlugsCount]->error = 0;
							}
							if(p.error)
							{
								_pPlugsInfo[currentPlugsCount]->error = new std::string(*p.error);
							}

							_pPlugsInfo[currentPlugsCount]->allow = p.allow;

							_pPlugsInfo[currentPlugsCount]->mode = p.mode;
							_pPlugsInfo[currentPlugsCount]->type = p.type;
							strcpy(_pPlugsInfo[currentPlugsCount]->name,p.name);
							strcpy(_pPlugsInfo[currentPlugsCount]->desc,p.desc);
							strcpy(_pPlugsInfo[currentPlugsCount]->version,p.version);

							if(!p.error)
							{
								char str2[MAX_PATH];
								CString str = _pPlugsInfo[currentPlugsCount]->dllname;
								str.MakeLower();
								strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
								dllNames.SetAt(str2,_pPlugsInfo[currentPlugsCount]->dllname);
							}
							++currentPlugsCount;
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
			char * last = strrchr(cache,'\\');
			strcpy(last,"\\plugin-scan.cache");
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
				{
					const std::string * const error(_pPlugsInfo[i]->error);
					UINT size(error ? error->size() : 0);
					file.Write(&size, sizeof size);
					if(size) file.Write(error->data(), size);
				}
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
			for(int i(0) ; i < _numPlugins ; ++i)
			{
				if(!std::strcmp(name, _pPlugsInfo[i]->dllname))
				{
					// bad plugins always have allow = false
					if(_pPlugsInfo[i]->allow) return true;
					std::ostringstream s; s
						<< "Plugin " << name << " is disabled because:" << std::endl
						<< _pPlugsInfo[i]->error << std::endl
						<< "Try to load anyway?";
					return ::MessageBox(0, s.str().c_str(), "Plugin Warning!", MB_YESNO | MB_ICONWARNING) == IDYES;
				}
			}
			return false;
		}
	}
}
