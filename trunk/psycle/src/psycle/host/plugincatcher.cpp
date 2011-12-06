
#include <psycle/host/detail/project.private.hpp>
#include "plugincatcher.hpp"
#if !defined WINAMP_PLUGIN
	#include "ProgressDialog.hpp"
#else
	#include "player_plugins/winamp/fake_progressDialog.hpp"
#endif //!defined WINAMP_PLUGIN

#include <psycle/host/Plugin.hpp>
#include <psycle/host/VstHost24.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower
#include <direct.h>

namespace psycle
{
	namespace host
	{
		PluginCatcher::PluginCatcher()
			:_numPlugins(-1)
		{}
		PluginCatcher::~PluginCatcher() {
			DestroyPluginInfo();
		}

		bool PluginCatcher::lookupDllName(const std::string name, std::string & result, MachineType type,int& shellidx)
		{
			std::string tmp = name;
			std::string extension = name.substr(name.length()-4,4);
			if ( extension != ".dll")
			{
				shellidx =  extension[0] + extension[1]*256 + extension[2]*65536 + extension[3]*16777216;
				tmp = name.substr(0,name.length()-4);
			}
			else shellidx = 0;

			tmp = preprocessName(tmp);

			switch(type)
			{
			case MACH_PLUGIN:
				{
					std::map<std::string,std::string>::iterator iterator = NativeNames.find(tmp);
					if(iterator != NativeNames.end())
					{
						result=iterator->second;
						return true;
					}
					break;
				}
			case MACH_VST:
			case MACH_VSTFX:
				{
					std::map<std::string,std::string>::iterator iterator = VstNames.find(tmp);
					if(iterator != VstNames.end())
					{
						result=iterator->second;
						return true;
					}
					break;
				}
			default:
				break;
			}
			return false;
		}

		bool PluginCatcher::TestFilename(const std::string & name, const int shellIdx)
		{
			for(int i(0) ; i < _numPlugins ; ++i)
			{
				if ((name == _pPlugsInfo[i]->dllname) &&
					(shellIdx == 0 || shellIdx == _pPlugsInfo[i]->identifier))
				{
					// bad plugins always have allow = false
					if(_pPlugsInfo[i]->allow) return true;
					std::ostringstream s; s
						<< "Plugin " << name << " is disabled because:" << std::endl
						<< _pPlugsInfo[i]->error << std::endl
						<< "Try to load anyway?";
					return ::MessageBox(0,s.str().c_str(), "Plugin Warning!", MB_YESNO | MB_ICONWARNING) == IDYES;
				}
			}
			return false;
		}

		void PluginCatcher::LoadPluginInfo(bool verify)
		{
			if(_numPlugins == -1)
			{
				loggers::information()("Scanning plugins ...");

				//DWORD dwThreadId;
				LoadPluginInfoParams params;
				params.verify = verify;
				params.theCatcher = this;
				//This almost works.. except for the fact that when closing the progress dialog, the focus is lost.
				//CreateThread( NULL, 0, ProcessLoadPlugInfo, &params, 0, &dwThreadId );
				//CSingleLock event(&params.theEvent, TRUE);
				ProcessLoadPlugInfo(&params);

				loggers::information()("Saving scan cache file ...");
				SaveCacheFile();
				loggers::information()("Done.");
			}
		}

		void PluginCatcher::ReScan(bool regenerate)
		{
			DestroyPluginInfo();

			if (regenerate) {
#if defined _WIN64
			DeleteFile((Global::configuration().cacheDir() / "psycle64.plugin-scan.cache").native_file_string().c_str());
#elif defined _WIN32
			DeleteFile((Global::configuration().cacheDir() / "psycle.plugin-scan.cache").native_file_string().c_str());
#else
#error unexpected platform
#endif
			}
			LoadPluginInfo();
		}

		DWORD PluginCatcher::ProcessLoadPlugInfo(void* pParam ) {
			LoadPluginInfoParams* param = (LoadPluginInfoParams*)pParam;
			int plugsCount(0);
			int badPlugsCount(0);
			param->theCatcher->_numPlugins = 0;
			bool cacheValid = param->theCatcher->LoadCacheFile(plugsCount, badPlugsCount, param->verify);
			// If cache found&loaded and no verify, we're ready, else start scan.
			if (cacheValid && !param->verify) {
				param->theEvent.SetEvent();
				return 0;
			}

			class populate_plugin_list
			{
			public:
				populate_plugin_list(std::vector<std::string> & result, std::string directory)
				{
					::CFileFind finder;
					int loop = finder.FindFile(::CString((directory + "\\*").c_str()));
					while(loop)
					{
						loop = finder.FindNextFile();
						if(finder.IsDirectory()) {
							if(!finder.IsDots())
							{
								std::string sfilePath = finder.GetFilePath();
								populate_plugin_list(result,sfilePath);
							}
						}
						else
						{
							CString filePath=finder.GetFilePath();
							filePath.MakeLower();
							if(filePath.Right(4) == ".dll")
							{
								std::string sfilePath = filePath;
								result.push_back(sfilePath);
							}
						}
					}
					finder.Close();
				}
			};

			std::vector<std::string> nativePlugs;
			std::vector<std::string> vstPlugs;

			CProgressDialog progress;
			{
				char c[1 << 10];
				::GetCurrentDirectory(sizeof c, c);
				std::string s(c);
				loggers::information()("Scanning plugins ... Current Directory: " + s);
			}
			loggers::information()("Scanning plugins ... Directory for Natives: " + Global::configuration().GetPluginDir());
			loggers::information()("Scanning plugins ... Directory for VSTs (32): " + Global::configuration().GetVst32Dir());
			loggers::information()("Scanning plugins ... Directory for VSTs (64): " + Global::configuration().GetVst64Dir());
			loggers::information()("Scanning plugins ... Listing ...");

			progress.SetWindowText("Scanning plugins ... Listing ...");
			progress.ShowWindow(SW_SHOW);

			populate_plugin_list(nativePlugs,Global::configuration().GetPluginDir());
#if	defined _WIN64
			populate_plugin_list(vstPlugs,Global::configuration().GetVst64Dir());
			if(Global::configuration().UsesJBridge() || Global::configuration().UsesPsycleVstBridge())
			{
				populate_plugin_list(vstPlugs,Global::configuration().GetVst32Dir());
			}
#elif defined _WIN32
			populate_plugin_list(vstPlugs,Global::configuration().GetVst32Dir());
			if(Global::configuration().UsesJBridge() || Global::configuration().UsesPsycleVstBridge())
			{
				populate_plugin_list(vstPlugs,Global::configuration().GetVst64Dir());
			}
#endif

			int plugin_count = (int)(nativePlugs.size() + vstPlugs.size());

			{
				std::ostringstream s; s << "Scanning plugins ... Counted " << plugin_count << " plugins.";
				loggers::information()(s.str());
				progress.m_Progress.SetStep(16384 / std::max(1,plugin_count));
				progress.SetWindowText(s.str().c_str());
			}
			std::ofstream out;
			{
				boost::filesystem::path log_dir(Global::configuration().cacheDir());
				// note mkdir is posix, not iso, on msvc, it's defined only #if !__STDC__ (in direct.h)
				mkdir(log_dir.native_directory_string().c_str());
#if defined _WIN64
				out.open((log_dir / "psycle64.plugin-scan.log.txt").native_file_string().c_str());
#elif defined _WIN32
				out.open((log_dir / "psycle.plugin-scan.log.txt").native_file_string().c_str());
#else
#error unexpected platform
#endif
			}
			out
				<< "==========================================" << std::endl
				<< "=== Psycle Plugin Scan Enumeration Log ===" << std::endl
				<< std::endl
				<< "If psycle is crashing on load, chances are it's a bad plugin, "
				<< "specifically the last item listed, if it has no comment after the library file name." << std::endl;

			std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing Natives ...";
			progress.SetWindowText(s.str().c_str());

			loggers::information()("Scanning plugins ... Testing Natives ...");
			out
				<< std::endl
				<< "======================" << std::endl
				<< "=== Native Plugins ===" << std::endl
				<< std::endl;
			out.flush();

			param->theCatcher->FindPlugins(plugsCount, badPlugsCount, nativePlugs, MACH_PLUGIN, out, cacheValid ? &progress : 0);

			out.flush();
			{
				std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing VSTs ...";
				progress.SetWindowText(s.str().c_str());
			}

			loggers::information()("Scanning plugins ... Testing VSTs ...");
			out
				<< std::endl
				<< "===================" << std::endl
				<< "=== VST Plugins ===" << std::endl
				<< std::endl;
			out.flush();

			param->theCatcher->FindPlugins(plugsCount, badPlugsCount, vstPlugs, MACH_VST, out, cacheValid ? &progress : 0);

			{
				std::ostringstream s; s << "Scanned " << plugin_count << " Files." << plugsCount << " plugins found";
				out << std::endl << s.str() << std::endl;
				out.flush();
				loggers::information()(s.str().c_str());
				progress.SetWindowText(s.str().c_str());
			}
			out.close();
			param->theCatcher->_numPlugins = plugsCount;
			progress.m_Progress.SetPos(16384);
			progress.SetWindowText("Saving scan cache file ...");

			loggers::information()("Saving scan cache file ...");
			progress.SendMessage(WM_CLOSE);
			param->theEvent.SetEvent();
			return 0;
		}

	void PluginCatcher::FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress)
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
			// Verify if the plugin is already in the cache.
			for(int i(0) ; i < _numPlugins; ++i)
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
						{
							currentBadPlugsCount++;
							s << "cache says it has previously been disabled because:" << std::endl << error << std::endl;
						}
						out << s.str();
						out.flush();
						loggers::information()(fileName + '\n' + s.str());
						break;
					}
				}
			}
			if(!exists)
			{
				try
				{
					out << "new plugin added to cache ; ";
					out.flush();
					loggers::information()(fileName + "\nnew plugin added to cache.");
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
							loggers::exception()(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
							_pPlugsInfo[currentPlugsCount]->allow = false;
							_pPlugsInfo[currentPlugsCount]->name = "???";
							_pPlugsInfo[currentPlugsCount]->identifier = 0;
							_pPlugsInfo[currentPlugsCount]->vendor = "???";
							_pPlugsInfo[currentPlugsCount]->desc = "???";
							_pPlugsInfo[currentPlugsCount]->version = "???";
							_pPlugsInfo[currentPlugsCount]->APIversion = 0;
							++currentBadPlugsCount;
						}
						else
						{
							_pPlugsInfo[currentPlugsCount]->allow = true;
							_pPlugsInfo[currentPlugsCount]->name = plug.GetName();
							_pPlugsInfo[currentPlugsCount]->identifier = 0;
							_pPlugsInfo[currentPlugsCount]->vendor = plug.GetAuthor();
							if(plug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
							else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
							{
								std::ostringstream s; s << (plug.IsSynth() ? "Psycle instrument" : "Psycle effect") << " by " << plug.GetAuthor();
								_pPlugsInfo[currentPlugsCount]->desc = s.str();
							}
								_pPlugsInfo[currentPlugsCount]->APIversion = plug.GetInfo()->APIVersion;
							{
									std::ostringstream s; s << std::setfill('0') << std::setw(3) << std::hex << plug.GetInfo()->PlugVersion;
								_pPlugsInfo[currentPlugsCount]->version = s.str();
							}
							out << plug.GetName() << " - successfully instanciated";
							out.flush();
						}
						learnDllName(fileName,type);
						// [bohan] plug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this scope actually).
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
							loggers::exception()(title.str() + '\n' + s.str());
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
							loggers::exception()(title.str() + '\n' + s.str());
						}
					}
					else if(type == MACH_VST)
					{
						_pPlugsInfo[currentPlugsCount]->type = MACH_VST;
						vst::plugin *vstPlug=0;
						try
						{
							vstPlug = dynamic_cast<vst::plugin*>(Global::vsthost().LoadPlugin(fileName.c_str()));
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
							loggers::exception()(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
							_pPlugsInfo[currentPlugsCount]->allow = false;
							_pPlugsInfo[currentPlugsCount]->identifier = 0;
							_pPlugsInfo[currentPlugsCount]->name = "???";
							_pPlugsInfo[currentPlugsCount]->vendor = "???";
							_pPlugsInfo[currentPlugsCount]->desc = "???";
							_pPlugsInfo[currentPlugsCount]->version = "???";
							_pPlugsInfo[currentPlugsCount]->APIversion = 0;
							++currentBadPlugsCount;
							if (vstPlug) delete vstPlug;
						}
						else
						{
							if (vstPlug->IsShellMaster())
							{
								char tempName[64] = {0}; 
								VstInt32 plugUniqueID = 0;
								bool firstrun = true;
								while ((plugUniqueID = vstPlug->GetNextShellPlugin(tempName)) != 0)
								{ 
									if (tempName[0] != 0)
									{
										if ( !firstrun )
										{
											++currentPlugsCount;
											_pPlugsInfo[currentPlugsCount]= new PluginInfo;
											_pPlugsInfo[currentPlugsCount]->dllname = fileName;
											_pPlugsInfo[currentPlugsCount]->FileTime = time;
										}

										_pPlugsInfo[currentPlugsCount]->allow = true;
										{
											std::ostringstream s;
											s << vstPlug->GetVendorName() << " " << tempName;
											_pPlugsInfo[currentPlugsCount]->name = s.str();
										}
										_pPlugsInfo[currentPlugsCount]->identifier = plugUniqueID;
										_pPlugsInfo[currentPlugsCount]->vendor = vstPlug->GetVendorName();
										if(vstPlug->IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
										else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
										{
											std::ostringstream s;
											s << (vstPlug->IsSynth() ? "VST Shell instrument" : "VST Shell effect") << " by " << vstPlug->GetVendorName();
											_pPlugsInfo[currentPlugsCount]->desc = s.str();
										}
										{
											std::ostringstream s;
											s << vstPlug->GetVersion();
											_pPlugsInfo[currentPlugsCount]->version = s.str();
										}
										_pPlugsInfo[currentPlugsCount]->APIversion = vstPlug->GetVstVersion();
										firstrun=false;
									}
								}
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = vstPlug->GetName();
								_pPlugsInfo[currentPlugsCount]->identifier = vstPlug->uniqueId();
								_pPlugsInfo[currentPlugsCount]->vendor = vstPlug->GetVendorName();
								if(vstPlug->IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
								{
									std::ostringstream s;
									s << (vstPlug->IsSynth() ? "VST instrument" : "VST effect") << " by " << vstPlug->GetVendorName();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s;
									s << vstPlug->GetVersion();
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								_pPlugsInfo[currentPlugsCount]->APIversion = vstPlug->GetVstVersion();
							}
							out << vstPlug->GetName() << " - successfully instanciated";
							out.flush();

							// [bohan] vstPlug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling vstPlug.Free(); explicitly.
							try
							{
								delete vstPlug;
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
								loggers::exception()(title.str() + '\n' + s.str());
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
								loggers::exception()(title.str() + '\n' + s.str());
							}
						}
						learnDllName(fileName,type);
					}
					++currentPlugsCount;
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
					loggers::crash()(s.str());
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
					loggers::crash()(s.str());
				}
			}
			out << std::endl;
			out.flush();
			}
			out.flush();
		}

		void PluginCatcher::DestroyPluginInfo()
		{
			for (int i=0; i<_numPlugins; i++)
			{
				delete _pPlugsInfo[i];
				_pPlugsInfo[i] = 0;
			}
			NativeNames.clear();
			VstNames.clear();
			_numPlugins = -1;
		}

		bool PluginCatcher::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount, bool verify)
		{
#if defined _WIN64
			std::string cache((Global::configuration().cacheDir() / "psycle64.plugin-scan.cache").native_file_string());
#elif defined _WIN32
			std::string cache((Global::configuration().cacheDir() / "psycle.plugin-scan.cache").native_file_string());
#else
#error unexpected platform
#endif
			RiffFile file;
			CFileFind finder;

			if (!file.Open(cache.c_str()))
			{
#if defined _WIN32
				/// try old location
				std::string cache2((boost::filesystem::path(Global::configuration().appPath()) / "psycle.plugin-scan.cache").native_file_string());
				if (!file.Open(cache2.c_str())) return false;
#else
				return false;
#endif
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
			if (version != CURRENT_CACHE_MAP_VERSION)
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
						p.error = (const char*)chars;
						delete [] chars;
					}
				}
				file.Read(&p.allow,sizeof(p.allow));
				file.Read(&p.mode,sizeof(p.mode));
				file.Read(&p.type,sizeof(p.type));
				file.ReadString(p.name);
				file.Read(&p.identifier,sizeof(p.identifier));
				file.ReadString(p.vendor);
				file.ReadString(p.desc);
				file.ReadString(p.version);
				file.Read(&p.APIversion,sizeof(p.APIversion));

				// Temp here contains the full path to the .dll
				if(finder.FindFile(Temp))
				{
					FILETIME time;
					finder.FindNextFile();
					if (finder.GetLastWriteTime(&time))
					{
						// Only add the information to the cache if the dll hasn't been modified (say, a new version)
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
							_pPlugsInfo[currentPlugsCount]->identifier = p.identifier;
							_pPlugsInfo[currentPlugsCount]->vendor = p.vendor;
							_pPlugsInfo[currentPlugsCount]->desc = p.desc;
							_pPlugsInfo[currentPlugsCount]->version = p.version;
							_pPlugsInfo[currentPlugsCount]->APIversion = p.APIversion;

							if(p.error.empty())
							{
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname,_pPlugsInfo[currentPlugsCount]->type);
							}
							++currentPlugsCount;
						}
					}
				}
			}

			_numPlugins = currentPlugsCount;
			file.Close();
			return true;
		}

		bool PluginCatcher::SaveCacheFile()
		{
#if defined _WIN64
			boost::filesystem::path cache(Global::configuration().cacheDir() / "psycle64.plugin-scan.cache");
#elif defined _WIN32
			boost::filesystem::path cache(Global::configuration().cacheDir() / "psycle.plugin-scan.cache");
#else
	#error unexpected platform
#endif

			DeleteFile(cache.native_file_string().c_str());
			RiffFile file;
			if (!file.Create(cache.native_file_string().c_str(),true)) 
			{
				// note mkdir is posix, not iso, on msvc, it's defined only #if !__STDC__ (in direct.h)
				mkdir(cache.branch_path().native_directory_string().c_str());
				if (!file.Create(cache.native_file_string().c_str(),true)) return false;
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
					UINT size((UINT)error.size());
					file.Write(&size, sizeof size);
					if(size) file.Write(error.data(), size);
				}
				file.Write(&_pPlugsInfo[i]->allow,sizeof(_pPlugsInfo[i]->allow));
				file.Write(&_pPlugsInfo[i]->mode,sizeof(_pPlugsInfo[i]->mode));
				file.Write(&_pPlugsInfo[i]->type,sizeof(_pPlugsInfo[i]->type));
				file.Write(_pPlugsInfo[i]->name.c_str(),_pPlugsInfo[i]->name.length()+1);
				file.Write(&_pPlugsInfo[i]->identifier,sizeof(_pPlugsInfo[i]->identifier));
				file.Write(_pPlugsInfo[i]->vendor.c_str(),_pPlugsInfo[i]->vendor.length()+1);
				file.Write(_pPlugsInfo[i]->desc.c_str(),_pPlugsInfo[i]->desc.length()+1);
				file.Write(_pPlugsInfo[i]->version.c_str(),_pPlugsInfo[i]->version.length()+1);
				file.Write(&_pPlugsInfo[i]->APIversion,sizeof(_pPlugsInfo[i]->APIversion));
			}
			file.Close();
			return true;
		}

		std::string PluginCatcher::preprocessName(std::string dllName) {
			{ // 1) remove extension
				std::string::size_type const pos(dllName.find(".dll"));
				if(pos != std::string::npos) dllName = dllName.substr(0, pos);
			}

			// 2) ensure lower case
			std::transform(dllName.begin(),dllName.end(),dllName.begin(),std::tolower);

			// 3) replace spaces and underscores with dash.
			std::replace(dllName.begin(),dllName.end(),' ','-');
			std::replace(dllName.begin(),dllName.end(),'_','-');

			return dllName;
		}

		void PluginCatcher::learnDllName(const std::string & fullname,MachineType type)
		{
			std::string str=fullname;
			// strip off path
			std::string::size_type pos=str.rfind('\\');
			if(pos != std::string::npos)
				str=str.substr(pos+1);

			str = preprocessName(str);

			switch(type)
			{
			case MACH_PLUGIN: NativeNames[str]=fullname;
				break;
			case MACH_VST:
			case MACH_VSTFX:VstNames[str]=fullname;
				break;
			default:
				break;
			}
		}

	}
}
