///\file
///\brief implementation file for psycle::host::CNewMachine.
#include <project.private.hpp>
#include "psycle.hpp"
#include "NewMachine.hpp"
#include "Plugin.hpp"
#include "VstHost.hpp"
#include "ProgressDialog.hpp"
#undef min // ???
#undef max // ???
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm> //std::transform
#include <cctype>	// std::tolower
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		int CNewMachine::pluginOrder = 1;
		bool CNewMachine::pluginName = 1;
		int CNewMachine::_numPlugins = -1;
		int CNewMachine::LastType0=0;
		int CNewMachine::LastType1=0;

		PluginInfo* CNewMachine::_pPlugsInfo[MAX_BROWSER_PLUGINS];

		std::map<std::string,std::string> CNewMachine::dllNames;


		void CNewMachine::learnDllName(const std::string & fullname)
		{
			std::string str=fullname;
			// strip off path
			std::string::size_type pos=str.rfind('\\');
			if(pos != std::string::npos)
				str=str.substr(pos+1);

			// transform string to lower case
			std::transform(str.begin(),str.end(),str.begin(),std::tolower);

			dllNames[str]=fullname;
		}

		bool CNewMachine::lookupDllName(const std::string & name, std::string & result)
		{
			std::map<std::string,std::string>::iterator iterator
				= dllNames.find(name);
			if(iterator != dllNames.end())
			{
				result=iterator->second;
				return true;
			}
			return false;
		}

		CNewMachine::CNewMachine(CWnd* pParent)
			: CDialog(CNewMachine::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CNewMachine)
			m_orderby = pluginOrder;
			m_showdllName = pluginName;
			//}}AFX_DATA_INIT
			OutBus = false;
			//pluginOrder = 0; Do NOT uncomment. It would cause the variable to be reseted each time.
				// It is initialized above, where it is declared.
		}

		CNewMachine::~CNewMachine()
		{
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
					if(_pPlugsInfo[i]->error.empty())
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
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hitem, TVI_SORT);
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
					if(_pPlugsInfo[i]->error.empty())
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
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hitem, TVI_SORT);
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
					std::string str = _pPlugsInfo[i]->dllname;
					std::string::size_type pos = str.rfind('\\');
					if(pos != std::string::npos)
						str=str.substr(pos+1);
					m_dllnameLabel.SetWindowText(str.c_str());
					m_nameLabel.SetWindowText(_pPlugsInfo[i]->name.c_str());
					m_descLabel.SetWindowText(_pPlugsInfo[i]->desc.c_str());
					m_versionLabel.SetWindowText(_pPlugsInfo[i]->version.c_str());
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

					psOutputDll = _pPlugsInfo[i]->dllname;

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
				host::loggers::info("Scanning plugins ...");

				::AfxGetApp()->DoWaitCursor(1); 
				int plugsCount(0);
				int badPlugsCount(0);
				_numPlugins = 0;
				bool progressOpen = !LoadCacheFile(plugsCount, badPlugsCount);

				class populate_plugin_list
				{
				public:
					populate_plugin_list(std::vector<std::string> &result,
						std::string directory)
					{
						::CFileFind finder;
						int loop = finder.FindFile(::CString((directory + "\\*").c_str()));
						while(loop)
						{
							loop = finder.FindNextFile();
							if(finder.IsDirectory()) {
								if(!finder.IsDots())
									populate_plugin_list(result,(LPCSTR)finder.GetFilePath());
							}
							else
							{
								CString filePath=finder.GetFilePath();
								filePath.MakeLower();
								if(filePath.Right(4) == ".dll")
									result.push_back((LPCSTR)filePath);
							}
						}
						finder.Close();
					}
				};

				std::vector<std::string> nativePlugs;
				std::vector<std::string> vstPlugs;

				CProgressDialog Progress;
				host::loggers::info("Scanning plugins ... Listing ...");
				if(progressOpen)
				{
					Progress.Create();
					Progress.SetWindowText("Scanning plugins ... Listing ...");
					Progress.ShowWindow(SW_SHOW);
				}

				populate_plugin_list(nativePlugs,Global::pConfig->GetPluginDir());
				populate_plugin_list(vstPlugs,Global::pConfig->GetVstDir());

				int plugin_count = nativePlugs.size() + vstPlugs.size();

				std::ostringstream s; s << "Scanning plugins ... Counted " << plugin_count << " plugins.";
				host::loggers::info(s.str());
				if(progressOpen) {
					Progress.m_Progress.SetStep(16384 / std::max(1,plugin_count));
					Progress.SetWindowText(s.str().c_str());
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
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing Natives ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::loggers::info("Scanning plugins ... Testing Natives ...");
				out
					<< std::endl
					<< "======================" << std::endl
					<< "=== Native Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPlugins(plugsCount, badPlugsCount, nativePlugs, MACH_PLUGIN, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing VSTs ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::loggers::info("Scanning plugins ... Testing VSTs ...");
				out
					<< std::endl
					<< "===================" << std::endl
					<< "=== VST Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPlugins(plugsCount, badPlugsCount, vstPlugs, MACH_VST, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanned " << plugin_count << " plugins.";
					host::loggers::info(s.str().c_str());
					Progress.SetWindowText(s.str().c_str());
				}
				out.close();
				_numPlugins = plugsCount;
				if(progressOpen)
				{
					Progress.m_Progress.SetPos(16384);
					Progress.SetWindowText("Saving scan cache file ...");
				}
				host::loggers::info("Saving scan cache file ...");
				SaveCacheFile();
				if(progressOpen)
					Progress.OnCancel();
				::AfxGetApp()->DoWaitCursor(-1); 
				host::loggers::info("Done.");
			}
		}

		void CNewMachine::FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress)
		{
			for(unsigned fileIdx=0;fileIdx<list.size();fileIdx++)
			{
				if(pProgress)
				{
					pProgress->m_Progress.StepIt();
					::Sleep(1);
				}
				std::string fileName = list[fileIdx];

				out << fileName << " ... ";
				out.flush();
				FILETIME time;
				ZeroMemory(&time,sizeof FILETIME);
				HANDLE hFile=CreateFile(fileName.c_str(),
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if(hFile!=INVALID_HANDLE_VALUE) {
					GetFileTime(hFile,0,0,&time);
					CloseHandle(hFile);
				}
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
							if(_pPlugsInfo[i]->dllname == fileName)
							{
								exists = true;
								const std::string error(_pPlugsInfo[i]->error);
								std::stringstream s;
								if(error.empty())
									s << "found in cache.";
								else
									s << "cache says it has previously been disabled because:" << std::endl << error << std::endl;
								out << s.str();
								out.flush();
								host::loggers::info(fileName + '\n' + s.str());
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
						host::loggers::info(fileName + "\nnew plugin added to cache.");
						_pPlugsInfo[currentPlugsCount]= new PluginInfo;
						_pPlugsInfo[currentPlugsCount]->dllname = fileName;
						_pPlugsInfo[currentPlugsCount]->FileTime = time;
						if(type == MACH_PLUGIN)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;
							Plugin plug(0);
							try
							{
								 plug.Instance(fileName);
								 plug.Init(); // [bohan] hmm, we should get rid of two-stepped constructions.
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s << typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							catch(...)
							{
								std::ostringstream s; s
									<< "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << _pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								_pPlugsInfo[currentPlugsCount]->name = "???";
								_pPlugsInfo[currentPlugsCount]->desc = "???";
								_pPlugsInfo[currentPlugsCount]->version = "???";
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = plug.GetName();
								{
									std::ostringstream s; s << (plug.IsSynth() ? "Psycle instrument" : "Psycle effect") << " by " << plug.GetAuthor();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s; s << plug.GetInfo()->Version; // API VERSION
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								if(plug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
								out << plug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// [bohan] plug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling plug.Free(); explicitly.
							try
							{
								plug.Free();
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
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
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
						}
						else if(type == MACH_VST)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_VST;
							vst::plugin vstPlug;
							try
							{
								 vstPlug.Instance(fileName);
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s << typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							catch(...)
							{
								std::ostringstream s; s << "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << _pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								_pPlugsInfo[currentPlugsCount]->name = "???";
								_pPlugsInfo[currentPlugsCount]->desc = "???";
								_pPlugsInfo[currentPlugsCount]->version = "???";
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = vstPlug.GetName();
								{
									std::ostringstream s;
									s << (vstPlug.IsSynth() ? "VST2 instrument" : "VST2 effect") << " by " << vstPlug.GetVendorName();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s;
									s << vstPlug.GetVersion();
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								
								if(vstPlug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;

								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
								out << vstPlug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// [bohan] vstPlug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling vstPlug.Free(); explicitly.
							try
							{
								vstPlug.Free();
								// [bohan] phatmatik crashes here...
								// <magnus> so does PSP Easyverb, in FreeLibrary
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
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
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
						}
					}
					catch(const std::exception & e)
					{
						std::stringstream s; s
							<< std::endl
							<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl
							<< typeid(e).name() << std::endl;
							if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
						out
							<< s.str().c_str();
						out.flush();
						host::loggers::crash(s.str());
					}
					catch(...)
					{
						std::stringstream s; s
							<< std::endl
							<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl
							<< "Type of exception is unknown, no further information available.";
						out
							<< s.str().c_str();
						out.flush();
						host::loggers::crash(s.str());
					}
				}
				out << std::endl;
				out.flush();
			}
			out.flush();
		}

		void CNewMachine::DestroyPluginInfo()
		{
			for (int i=0; i<_numPlugins; i++)
			{
				zapObject(_pPlugsInfo[i]);
			}
			dllNames.clear();
			_numPlugins = -1;
		}

		bool CNewMachine::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount)
		{
			char modulefilename[_MAX_PATH];
			GetModuleFileName(NULL,modulefilename,_MAX_PATH);
			std::string path=modulefilename;
			std::string::size_type pos=path.rfind('\\');
			if(pos != std::string::npos)
				path=path.substr(0,pos);
			std::string cache=path + "\\plugin-scan.cache";

			RiffFile file;
			CFileFind finder;

			if (!file.Open(cache.c_str())) 
			{
				return false;
			}

			char Temp[MAX_PATH];
			file.Read(Temp,8);
			Temp[8]=0;
			if (strcmp(Temp,"PSYCACHE")!=0)
			{
				file.Close();
				DeleteFile(cache.c_str());
				return false;
			}

			UINT version;
			file.Read(&version,sizeof(version));
			if (version > CURRENT_CACHE_MAP_VERSION)
			{
				file.Close();
				DeleteFile(cache.c_str());
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
						char *chars(new char[size + 1]);
						file.Read(chars, size);
						chars[size] = '\0';
						p.error = chars;
						zapArray(chars);
					}
				}
				file.Read(&p.allow,sizeof(p.allow));
				file.Read(&p.mode,sizeof(p.mode));
				file.Read(&p.type,sizeof(p.type));
				file.ReadString(p.name);
				file.ReadString(p.desc);
				file.ReadString(p.version);
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

							_pPlugsInfo[currentPlugsCount]->dllname = Temp;
							_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;

							///\todo this could be better handled
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								_pPlugsInfo[currentPlugsCount]->error = "";
							}
							if(!p.error.empty())
							{
								_pPlugsInfo[currentPlugsCount]->error = p.error;
							}

							_pPlugsInfo[currentPlugsCount]->allow = p.allow;

							_pPlugsInfo[currentPlugsCount]->mode = p.mode;
							_pPlugsInfo[currentPlugsCount]->type = p.type;
							_pPlugsInfo[currentPlugsCount]->name = p.name;
							_pPlugsInfo[currentPlugsCount]->desc = p.desc;
							_pPlugsInfo[currentPlugsCount]->version = p.version;

							if(p.error.empty())
							{
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
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
				file.Write(_pPlugsInfo[i]->dllname.c_str(),_pPlugsInfo[i]->dllname.length()+1);
				file.Write(&_pPlugsInfo[i]->FileTime,sizeof(_pPlugsInfo[i]->FileTime));
				{
					const std::string error(_pPlugsInfo[i]->error);
					UINT size(error.size());
					file.Write(&size, sizeof size);
					if(size) file.Write(error.data(), size);
				}
				file.Write(&_pPlugsInfo[i]->allow,sizeof(_pPlugsInfo[i]->allow));
				file.Write(&_pPlugsInfo[i]->mode,sizeof(_pPlugsInfo[i]->mode));
				file.Write(&_pPlugsInfo[i]->type,sizeof(_pPlugsInfo[i]->type));
				file.Write(_pPlugsInfo[i]->name.c_str(),_pPlugsInfo[i]->name.length()+1);
				file.Write(_pPlugsInfo[i]->desc.c_str(),_pPlugsInfo[i]->desc.length()+1);
				file.Write(_pPlugsInfo[i]->version.c_str(),_pPlugsInfo[i]->version.length()+1);
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

		bool CNewMachine::TestFilename(const std::string & name)
		{
			for(int i(0) ; i < _numPlugins ; ++i)
			{
				if(name == _pPlugsInfo[i]->dllname)
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
	NAMESPACE__END
NAMESPACE__END
