///\file
///\brief implementation file for psycle::host::Plugin
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/engine/FileIO.hpp>
#include <psycle/engine/plugin.hpp>
#include <universalis/operating_system/exceptions/code_description.hpp>
#include <psycle/engine/song.hpp>
#include <cctype>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstdlib> // for environment variables functions
#include <string>
#include <sstream>
//todo:
#include <psycle/host/uiglobal.hpp>
#include <psycle/host/cacheddllfinder.hpp>
#include <psycle/host/uiconfiguration.hpp>
namespace psycle
{
	namespace host
	{
		typedef CMachineInfo * (* GETINFO) ();
		typedef CMachineInterface * (* CREATEMACHINE) ();
		typedef CMachineInterface * (* CREATEGMACHINE) (int);	//dw00t
		typedef CMachineGuiParameter ** (* GETPARAMS) (int);	//dw00t

		PluginFxCallback Plugin::callback_;

		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma warning(push)
				#pragma warning(disable:4355) // 'this' : used in base member initializer list
		#endif

		Plugin::Plugin(Machine::id_type id)
			:Machine(MACH_PLUGIN, MACHMODE_UNDEFINED, id)
			,_dll(0)
			,proxy_(*this)
			,_psAuthor("")
			,_psDllName("")
			,_psName("")
			{
				_audiorange=32768.0f;
			}

		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma warning(pop)
		#endif

		Plugin::~Plugin() throw()
		{
			Free();
		}
		Machine* Plugin::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			Plugin* plug = new Plugin(_id);
			if (plug->LoadDll(_dllname) )
			{
				plug->Init();
				return plug;
			}
			else
			{
				delete plug;
				return 0;
			}
		}

		#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
			namespace boost
			{
				namespace filesystem
				{
					using namespace ::boost::filesystem;
					/// blergh, dos/win9x needs a work around for boost::filesystem::equivalent
					bool equivalent(path const & path1, path const & path2)
					{
						class unique
						{
						public:
							path operator()(path const & input)
							{
								std::string s((input.is_complete() ? input : current_path() / input).string());
								std::transform(s.begin(), s.end(), s.begin(), std::tolower);
								return path(s, no_check);
							}
						} unique;
						return unique(path1) == unique(path2);
					}
				}
			}
		#endif

		bool Plugin::LoadDll(std::string const & base_name_)
		{
			std::string fullpath = base_name_;
			if(!Global::dllfinder().LookupDllPath(fullpath,MACH_PLUGIN)) 
			{
				// Check Compatibility Table.
				// Probably could be done with the dllNames lockup.
				//GetCompatible(base_name,path) // If no one found, it will return a null string.
				fullpath = base_name_;
			}
			try
			{
				char const static path_env_var_name[] =
				{
				#if defined DIVERSALIS__OPERATING_SYSTEM__LINUX
					"LD_LIBRARY_PATH"
				#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					"PATH"
				#else
					#error unknown dynamic linker
				#endif
				};
				//_dll = ::LoadLibraryEx(file_name.c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
				// Or better, add all intermediate dirs from the root plugin dir down to this plugin's dir to the search path
				// ::SetDllDirectory only exists since XP SP1 and doesn't let us add more than one path.. it's hence as poor as LOAD_WITH_ALTERED_SEARCH_PATH.
				// So, we simply locally alter the PATH environment variable, it's more sane than wapi.
				{
					boost::filesystem::path path(fullpath, boost::filesystem::native);
					// save the original path env var
					std::string old_path;
					{
						char const * const env(std::getenv(path_env_var_name));
						if(env) old_path = env;
					}
					std::ostringstream new_path;
					// grows the search path with intermediate dirs between the configured root dir for plugins and the dir of this plugin
					{
						// configured root dir for plugins
						boost::filesystem::path root_path(Global::configuration().GetPluginDir(), boost::filesystem::native);
						// grow the search path with each compound of the sub dir until we reached the root dir
						// first, normalize the path so we don't have
						// dirs that are not intermediate because of things like foo/../bar
						// or useless repetitions because of things like foo/./bar
						path.normalize();
						// then, loop
						do
						{
							// go to the parent dir (in the first iteration of the loop, it removes the file leaf)
							path = path.branch_path();
							/*if(loggers::trace()())*/ loggers::trace("path: " + path.string());
							// the following test is necessary in case the user has changed the configured root dir but not rescanned the plugins.
							// the loop would never exit because boost::filesystem::equivalent returns false if any of the directory doesn't exist.
							if(path.empty()) throw exceptions::library_errors::loading_error("Directory does not exits.");
							// appends the intermediate dir to the list of paths
							new_path << path.native_directory_string() << ";";
						}
						while(!boost::filesystem::equivalent(path, root_path));
						// append the old path value, at the end so it's searched last
						new_path << old_path;
					}
					// set the new path env var
					if(::putenv((path_env_var_name + ("="+ new_path.str())).c_str())) throw exceptions::library_errors::loading_error("Could not alter PATH env var.");
					/*if(loggers::trace()())*/ loggers::trace(path_env_var_name + (" env var: " + new_path.str()));
					// load the library passing just the base file name and relying on the search path env var
					_dll = ::LoadLibrary(base_name_.c_str());
					// set the path env var back to its original value
					if(::putenv((path_env_var_name + ("=" + old_path)).c_str())) throw exceptions::library_errors::loading_error("Could not set PATH env var back to its original value.");

					Instance(fullpath);

				}
				if(!_dll)
				{
					std::ostringstream s; s
						<< "could not load library: " << fullpath << std::endl
						<< universalis::operating_system::exceptions::code_description();
					throw exceptions::library_errors::loading_error(s.str());
				}
			}
			catch(std::exception const & e)
			{
				std::ostringstream s; s
					<< "Exception while instanciating: " << fullpath << std::endl
					<< "Replacing with dummy." << std::endl
					<< universalis::compiler::typenameof(e) << std::endl
					<< e.what();
				MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
				return false;
			}
			catch(...)
			{
				std::ostringstream s; s
					<< "Exception while instanciating: " << fullpath << std::endl
					<< "Replacing with dummy." << std::endl
					<< "Unkown type of exception" << std::endl
					<< universalis::compiler::exceptions::ellipsis();
				MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
				return false;
			}
			return true;
		};

		void Plugin::Instance(std::string const & file_name)
		{
			GETINFO GetInfoFunc = (GETINFO) GetProcAddress(_dll, "GetInfo");
			if(!GetInfoFunc)
			{
				std::ostringstream s; s
					<< "library is not a psycle native plugin:" << std::endl
					<< "could not resolve symbol 'GetInfo' in library: " << file_name << std::endl
					<< universalis::operating_system::exceptions::code_description();
				throw exceptions::library_errors::symbol_resolving_error(s.str());
			}
			try
			{
				_pInfo = GetInfoFunc();
			}
			PSYCLE__HOST__CATCH_ALL(*this)
			if(_pInfo->Version < plugin_interface::MI_VERSION) throw std::runtime_error("plugin format is too old");

			_isSynth = _pInfo->Flags & 3; //dw00t //this was an == instead of an &, but the newgui uses the Flags field to identify itself
			
			if(_isSynth)
			{
				DefineStereoOutput(1);
				_mode = MACHMODE_GENERATOR;
			}
			else
			{
				DefineStereoInput(1);
				DefineStereoOutput(1);
				_mode = MACHMODE_FX;
			}
			_editName = _pInfo->ShortName;
			_psAuthor = _pInfo->Author;
			_psName = _pInfo->Name;
			_psDllName = file_name;

			CREATEMACHINE GetInterface = (CREATEMACHINE) GetProcAddress(_dll, "CreateMachine");
			if(!GetInterface)
			{
				std::ostringstream s; s
					<< "could not resolve symbol 'CreateMachine' in library: " << file_name << std::endl
					<< universalis::operating_system::exceptions::code_description();
				throw exceptions::library_errors::symbol_resolving_error(s.str());
			}
			try
			{
				proxy()(GetInterface());
			}
			PSYCLE__HOST__CATCH_ALL(*this)

			if(_pInfo->Flags & plugin_interface::CUSTOM_GUI)
			{
				GETPARAMS GetParams = (GETPARAMS) GetProcAddress(_dll, "GetParams");
				if(!GetParams)
				{
					std::ostringstream s; s
						<< "could not resolve symbol 'GetParams' in library: " << file_name << std::endl
						<< universalis::operating_system::exceptions::code_description();
					throw exceptions::library_errors::symbol_resolving_error(s.str());
				}
				try
				{
					_pParams = GetParams(id());
				}
				PSYCLE__HOST__CATCH_ALL(*this)
			}
		}

		void Plugin::Init()
		{
			Machine::Init();
			if(proxy()()) 
			{
				try
				{
					proxy().Init();
				}
				catch(const std::exception &)
				{
					throw;
				}
				for(int gbp(0) ; gbp < GetInfo()->numParameters ; ++gbp)
				{
					try
					{
						proxy().ParameterTweak(gbp, _pInfo->Parameters[gbp]->DefValue);
					}
					catch(const std::exception &)
					{
						throw;
					}
				}
			}
		}

		void Plugin::Free()
		{
			const std::exception * exception(0);
			try
			{
				proxy()(0);
			}
			catch(std::exception const & e)
			{
				if(!exception) exception = &e;
			}
			if(_dll)
			{
				::FreeLibrary(_dll);
				_dll = 0;
			}
			if(exception) throw *exception;
		}

		void Plugin::SaveDllName(RiffFile * pFile) 
		{
			boost::filesystem::path path(this->GetDllName(), boost::filesystem::native);
			std::string pathstr = path.leaf();
			pFile->WriteChunk(pathstr.c_str(), pathstr.length() + 1);
		}

		bool Plugin::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size); // size of whole structure
			if(size)
			{
				if(version > CURRENT_FILE_VERSION_MACD)
				{
					pFile->Skip(size);
					std::ostringstream s; s
						<< version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
						<< "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
					MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
					return false;
				}
				else
				{
					std::uint32_t count;
					pFile->Read(count);  // size of vars
					/*
					if (count)
					{
						pFile->ReadChunk(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
					}
					*/
					for(unsigned int i(0) ; i < count ; ++i)
					{
						std::uint32_t temp;
						pFile->Read(temp);
						SetParameter(i, temp);
					}
					size -= sizeof(count) + sizeof(std::uint32_t) * count;
					if(size)
					{
						char * pData = new char[size];
						pFile->ReadChunk(pData, size); // Number of parameters
						try
						{
							proxy().PutData(pData); // Internal load
						}
						catch(std::exception const &)
						{
							return false;
						}
						return true;
					}
				}
			}
			return true;
		};

		void Plugin::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t count = GetNumParams();
			std::uint32_t size2(0);
			try
			{
				size2 = proxy().GetDataSize();
			}
			catch(std::exception const &)
			{
				// data won't be saved
			}
			std::uint32_t size = size2 + sizeof count  + sizeof(std::uint32_t) * count;
			pFile->Write(size);
			pFile->Write(count);
			for(unsigned int i(0) ; i < count ; ++i) pFile->Write<std::uint32_t>(GetParamValue(i));
			if(size2)
			{
				char * pData = new char[size2];
				try
				{
					proxy().GetData(pData); // Internal save
				}
				catch(std::exception const &)
				{
					// this sucks because we already wrote the size,
					// so now we have to write the data, even if they are corrupted.
				}
				pFile->WriteChunk(pData, size2); // Number of parameters
				delete[] pData;
			}
		};

		void Plugin::Work(int numSamples)
		{
			if(_mode != MACHMODE_GENERATOR) Machine::Work(numSamples);
			else
			{
				if (!_mute) _stopped = false;
				else _stopped = true;
			}
			cpu::cycles_type cost(cpu::cycles());
			if (!_mute) 
			{
				if ((_mode == MACHMODE_GENERATOR) || (!_bypass && !_stopped))
				{
					int ns = numSamples;
					int us = 0;
					while (ns)
					{
						int nextevent = (TWSActive)?TWSSamples:ns+1;
						for (int i=0; i < Global::song().tracks(); i++)
						{
							if (TriggerDelay[i]._cmd)
							{
								if (TriggerDelayCounter[i] < nextevent)
								{
									nextevent = TriggerDelayCounter[i];
								}
							}
						}
						if (nextevent > ns)
						{
							if (TWSActive)
							{
								TWSSamples -= ns;
							}
							for (int i=0; i < Global::song().tracks(); i++)
							{
								// come back to this
								if (TriggerDelay[i]._cmd)
								{
									TriggerDelayCounter[i] -= ns;
								}
							}
							try
							{
								proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, Global::song().tracks());
							}
							catch(const std::exception &)
							{
							}
							ns = 0;
						}
						else
						{
							if(nextevent)
							{
								ns -= nextevent;
								try
								{
									proxy().Work(_pSamplesL+us, _pSamplesR+us, nextevent, Global::song().tracks());
								}
								catch(const std::exception &)
								{
								}
								us += nextevent;
							}
							if (TWSActive)
							{
								if (TWSSamples == nextevent)
								{
									int activecount = 0;
									TWSSamples = TWEAK_SLIDE_SAMPLES;
									for (int i = 0; i < MAX_TWS; i++)
									{
										if (TWSDelta[i] != 0)
										{
											TWSCurrent[i] += TWSDelta[i];

											if (((TWSDelta[i] > 0) && (TWSCurrent[i] >= TWSDestination[i]))
												|| ((TWSDelta[i] < 0) && (TWSCurrent[i] <= TWSDestination[i])))
											{
												TWSCurrent[i] = TWSDestination[i];
												TWSDelta[i] = 0;
											}
											else
											{
												activecount++;
											}
											try
											{
												proxy().ParameterTweak(TWSInst[i], int(TWSCurrent[i]));
											}
											catch(const std::exception &)
											{
											}
										}
									}
									if(!activecount) TWSActive = false;
								}
							}
							for (int i=0; i < Global::song().tracks(); i++)
							{
								// come back to this
								if (TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
								{
									if (TriggerDelayCounter[i] == nextevent)
									{
										// do event
										try
										{
											proxy().SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
										}
										catch(const std::exception &)
										{
										}
										TriggerDelay[i]._cmd = 0;
									}
									else
									{
										TriggerDelayCounter[i] -= nextevent;
									}
								}
								else if (TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
								{
									if (TriggerDelayCounter[i] == nextevent)
									{
										// do event
										try
										{
											proxy().SeqTick(i, TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
										}
										catch(const std::exception &)
										{
										}
										TriggerDelayCounter[i] = (RetriggerRate[i]*Global::player().SamplesPerRow())/256;
									}
									else
									{
										TriggerDelayCounter[i] -= nextevent;
									}
								}
								else if (TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
								{
									if (TriggerDelayCounter[i] == nextevent)
									{
										// do event
										try
										{
											proxy().SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
										}
										catch(const std::exception &)
										{
										}
										TriggerDelayCounter[i] = (RetriggerRate[i]*Global::player().SamplesPerRow())/256;
										int parameter = TriggerDelay[i]._parameter&0x0f;
										if (parameter < 9)
										{
											RetriggerRate[i]+= 4*parameter;
										}
										else
										{
											RetriggerRate[i]-= 2*(16-parameter);
											if (RetriggerRate[i] < 16)
											{
												RetriggerRate[i] = 16;
											}
										}
									}
									else
									{
										TriggerDelayCounter[i] -= nextevent;
									}
								}
								else if (TriggerDelay[i]._cmd == PatternCmd::ARPEGGIO)
								{
									if (TriggerDelayCounter[i] == nextevent)
									{
										PatternEntry entry =TriggerDelay[i];
										switch(ArpeggioCount[i])
										{
										case 0: 
											try
											{
												proxy().SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
											}
											catch(const std::exception &)
											{
											}
											ArpeggioCount[i]++;
											break;
										case 1:
											entry._note+=((TriggerDelay[i]._parameter&0xF0)>>4);
											try
											{
												proxy().SeqTick(i ,entry._note, entry._inst, 0, 0);
											}
											catch(const std::exception &)
											{
											}
											ArpeggioCount[i]++;
											break;
										case 2:
											entry._note+=(TriggerDelay[i]._parameter&0x0F);
											try
											{
												proxy().SeqTick(i ,entry._note, entry._inst, 0, 0);
											}
											catch(const std::exception &)
											{
											}
											ArpeggioCount[i]=0;
											break;
										}
										TriggerDelayCounter[i] = Global::player().SamplesPerRow()*Global::player().tpb/24;
									}
									else
									{
										TriggerDelayCounter[i] -= nextevent;
									}
								}
							}
						}
					}
					Machine::SetVolumeCounter(numSamples);
					if ( Global::configuration().autoStopMachines )
					{
						if (_volumeCounter < 8.0f)
						{
							_volumeCounter = 0.0f;
							_volumeDisplay = 0;
							_stopped = true;
						}
						else _stopped = false;
					}
				}
			}
			cost = cpu::cycles() - cost;
			work_cpu_cost(work_cpu_cost() + cost);
			_worked = true;
		}

		//dw00t
		CMachineGuiParameter* Plugin::GetParam(int num)
		{
			if(num<GetNumParams() && num>=0)
				return _pParams[num];
			return 0;
		}
		//\dw00t

		bool Plugin::SetParameter(int numparam,int value)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					proxy().ParameterTweak(numparam,value);
				}
				catch(const std::exception &)
				{
					return false;
				}
				return true;
			}
			else return false;
		}

		void Plugin::GetParamName(int numparam, char * name)
		{
			if( numparam < _pInfo->numParameters ) std::strcpy(name,_pInfo->Parameters[numparam]->Name);
			else std::strcpy(name, "Out of Range");

		}
		void Plugin::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if(GetInfo()->Parameters[numparam]->Flags & MPF_STATE)
			{
				minval = GetInfo()->Parameters[numparam]->MinValue;
				maxval = GetInfo()->Parameters[numparam]->MaxValue;
			}
			else
			{
				minval = maxval = 0;
			}
		}
		int Plugin::GetParamValue(int numparam)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					return proxy().Val(numparam);
				}
				catch(const std::exception &)
				{
					return -1; // hmm
				}
			}
			else return -1; // hmm
		}

		void Plugin::GetParamValue(int numparam, char * parval)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					if(!proxy().DescribeValue(parval, numparam, proxy().Vals()[numparam]))
						std::sprintf(parval, "%i", proxy().Vals()[numparam]);
				}
				catch(const std::exception &)
				{
				}
			}
			else std::strcpy(parval,"Out of Range");
		}

		void Plugin::Stop()
		{
			try
			{
				proxy().Stop();
			}
			catch(const std::exception &)
			{
			}
		}

		void Plugin::Tick()
		{
			try
			{
				proxy().SequencerTick();
			}
			catch(const std::exception &)
			{
			}
		}

		void Plugin::Tick(int channel, PatternEntry * pData)
		{
			try
			{
				proxy().SeqTick(channel, pData->_note, pData->_inst, pData->_cmd, pData->_parameter);
			}
			catch(const std::exception &)
			{
				return;
			}
			if(pData->_note == notecommands::tweak || pData->_note == notecommands::tweakeffect)
			{
				if(pData->_inst < _pInfo->numParameters)
				{
					int nv = (pData->_cmd<<8)+pData->_parameter;
					int const min = _pInfo->Parameters[pData->_inst]->MinValue;
					int const max = _pInfo->Parameters[pData->_inst]->MaxValue;
					nv += min;
					if(nv > max) nv = max;
					try
					{
						proxy().ParameterTweak(pData->_inst, nv);
					}
					catch(const std::exception &)
					{
					}
					Global::player().Tweaker = true;
				}
			}
			else if(pData->_note == notecommands::tweakslide)
			{
				if(pData->_inst < _pInfo->numParameters)
				{
					int i;
					if(TWSActive)
					{
						// see if a tweak slide for this parameter is already happening
						for(i = 0; i < MAX_TWS; i++)
						{
							if((TWSInst[i] == pData->_inst) && (TWSDelta[i] != 0))
							{
								// yes
								break;
							}
						}
						if(i == MAX_TWS)
						{
							// nope, find an empty slot
							for (i = 0; i < MAX_TWS; i++)
							{
								if (TWSDelta[i] == 0)
								{
									break;
								}
							}
						}
					}
					else
					{
						// wipe our array for safety
						for (i = MAX_TWS-1; i > 0; i--)
						{
							TWSDelta[i] = 0;
						}
					}
					if (i < MAX_TWS)
					{
						TWSDestination[i] = float(pData->_cmd<<8)+pData->_parameter;
						float min = float(_pInfo->Parameters[pData->_inst]->MinValue);
						float max = float(_pInfo->Parameters[pData->_inst]->MaxValue);
						TWSDestination[i] += min;
						if (TWSDestination[i] > max)
						{
							TWSDestination[i] = max;
						}
						TWSInst[i] = pData->_inst;
						try
						{
							TWSCurrent[i] = float(proxy().Vals()[TWSInst[i]]);
						}
						catch(const std::exception &)
						{
						}
						TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/Global::player().SamplesPerRow();
						TWSSamples = 0;
						TWSActive = true;
					}
					else
					{
						// we have used all our slots, just send a twk
						int nv = (pData->_cmd<<8)+pData->_parameter;
						int const min = _pInfo->Parameters[pData->_inst]->MinValue;
						int const max = _pInfo->Parameters[pData->_inst]->MaxValue;
						nv += min;
						if (nv > max) nv = max;
						try
						{
							proxy().ParameterTweak(pData->_inst, nv);
						}
						catch(const std::exception &)
						{
						}
					}
				}
				Global::player().Tweaker = true;
			}
		}

		void proxy::operator()(plugin_interface::CMachineInterface * plugin) throw(exceptions::function_error)
		{
			delete this->plugin_; ///\todo call the plugin's exported deletion function instead
			this->plugin_ = plugin;
			if(plugin)
			{
				callback();
				//Init(); // can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
			}
		}
	}
}
