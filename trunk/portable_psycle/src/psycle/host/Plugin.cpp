#include "stdafx.h"
#include "FileIO.h"
#include "Plugin.h"
#include "InputHandler.h"
#include <operating_system/exceptions/code_description.h>
#include <algorithm>
#include <cctype>
#if !defined _WINAMP_PLUGIN_
	#include "psycle.h"
	#include "NewMachine.h"
#endif
///\file
///\brief implementation file for psycle::host::Plugin
namespace psycle
{
	namespace host
	{
		typedef CMachineInfo * (* GETINFO) ();
		typedef CMachineInterface * (* CREATEMACHINE) ();

		PluginFxCallback Plugin::_callback;

		#pragma warning(push)
			#pragma warning(disable:4355) // 'this' : used in base member initializer list
			Plugin::Plugin(int index)
				: _dll(0)
				, proxy_(*this)
				, _psAuthor(0)
				, _psDllName(0)
				, _psName(0)
			{
				_macIndex = index;
				_type = MACH_PLUGIN;
				_mode = MACHMODE_FX;
				std::sprintf(_editName, "native plugin");
			}
		#pragma warning(pop)

		Plugin::~Plugin() throw()
		{
			Free();
			zapArray(_psAuthor);
			zapArray(_psDllName);
			zapArray(_psName);
		}

		void Plugin::Instance(const char file_name[]) throw(...)
		{
			_dll = ::LoadLibrary(file_name);
			if(!_dll)
			{
				std::ostringstream s; s
					<< "could not load library: " << file_name << std::endl
					<< operating_system::exceptions::code_description();
				throw exceptions::library_errors::loading_error(s.str());
			}
			GETINFO GetInfo = (GETINFO) GetProcAddress(_dll, "GetInfo");
			if(!GetInfo)
			{
				std::ostringstream s; s
					<< "could not resolve symbol 'GetInfo' in library: " << file_name << std::endl
					<< operating_system::exceptions::code_description();
				throw exceptions::library_errors::symbol_resolving_error(s.str());
			}
			try
			{
				_pInfo = GetInfo();
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(*this, "GetInfo", &e); }
			catch(const char * const e) { exceptions::function_errors::rethrow(*this, "GetInfo", &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(*this, "GetInfo", &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(*this, "GetInfo", &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(*this, "GetInfo"); }
			if(_pInfo->Version < MI_VERSION) throw std::runtime_error("plugin format is too old");
			_isSynth = _pInfo->Flags == 3;
			if(_isSynth) _mode = MACHMODE_GENERATOR;
			strncpy(_psShortName,_pInfo->ShortName,15);
			_psShortName[15]='\0';
			strncpy(_editName, _pInfo->ShortName,31);
			_editName[31]='\0';
			_psAuthor = new char[strlen(_pInfo->Author)+1];
			strcpy(_psAuthor,_pInfo->Author);
			_psName = new char[strlen(_pInfo->Name)+1];
			strcpy(_psName,_pInfo->Name);
			_psDllName = new char[strlen(file_name)+1];
			strcpy(_psDllName, file_name);
			CREATEMACHINE GetInterface = (CREATEMACHINE) GetProcAddress(_dll, "CreateMachine");
			if(!GetInterface)
			{
				std::ostringstream s; s
					<< "could not resolve symbol 'CreateMachine' in library: " << file_name << std::endl
					<< operating_system::exceptions::code_description();
				throw exceptions::library_errors::symbol_resolving_error(s.str());
			}
			try
			{
				proxy()(GetInterface());
			}
			catch(const std::exception & e) { exceptions::function_errors::rethrow(*this, "CreateMachine", &e); }
			catch(const char * const e) { exceptions::function_errors::rethrow(*this, "CreateMachine", &e); }
			catch(const long int & e) { exceptions::function_errors::rethrow(*this, "CreateMachine", &e); }
			catch(const unsigned long int & e) { exceptions::function_errors::rethrow(*this, "CreateMachine", &e); }
			catch(...) { exceptions::function_errors::rethrow<void*>(*this, "CreateMachine"); }
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

		bool Plugin::LoadDll(std::string psFileName)
		{
			std::transform(psFileName.begin(),psFileName.end(),psFileName.begin(),std::tolower);
			std::string sPath2;
			#if defined _WINAMP_PLUGIN_
				CString sPath;
				sPath = Global::pConfig->GetPluginDir();
				if( FindFileinDir(psFileName,sPath) )
				{
					return Instance(sPath2);
				}
				return false;
			#else
				std::string sPath;
				if(!CNewMachine::lookupDllName(psFileName,sPath)) 
				{
					// Check Compatibility Table.
					// Probably could be done with the dllNames lockup.
					//GetCompatible(psFileName,sPath2) // If no one found, it will return a null string.
					sPath = psFileName;
				}

				if(!CNewMachine::TestFilename(sPath) ) 
				{
					return false;
				}
				try
				{
					Instance(sPath.c_str());
				}
				catch(const std::exception & e)
				{
					std::ostringstream s; s
						<< "Exception while instanciating: " << sPath << std::endl
						<< "Replacing with dummy." << std::endl
						<< typeid(e).name() << std::endl
						<< e.what();
					::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
					return false;
				}
				catch(...)
				{
					std::ostringstream s; s
						<< "Exception while instanciating: " << sPath2 << std::endl
						<< "Replacing with dummy." << std::endl
						<< "Unkown type of exception";
					::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
					return false;
				}
				return true;
			#endif
		};

		void Plugin::Free() throw(...)
		{
			const std::exception * exception(0);
			try
			{
				proxy()(0);
			}
			catch(const std::exception & e)
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
			CString str = _psDllName;
			char str2[256];
			strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
			pFile->Write(&str2,strlen(str2)+1);
		};

		bool Plugin::LoadSpecificFileChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size,sizeof(size)); // size of whole structure
			if(size)
			{
				if(version > CURRENT_FILE_VERSION_MACD)
				{
					pFile->Skip(size);
					std::ostringstream s; s
						<< version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
						<< "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
					::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
					return false;
				}
				else
				{
					UINT count;
					pFile->Read(&count,sizeof(count));  // size of vars
					/*
					if (count)
					{
						pFile->Read(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
					}
					*/
					for (UINT i = 0; i < count; i++)
					{
						int temp;
						pFile->Read(&temp,sizeof(temp));
						SetParameter(i,temp);
					}
					size -= sizeof(count) + sizeof(int)*count;
					if(size)
					{
						byte* pData = new byte[size];
						pFile->Read(pData, size); // Number of parameters
						try
						{
							proxy().PutData(pData); // Internal load
						}
						catch(const std::exception &)
						{
							return false;
						}
						return true;
					}
				}
			}
			return true;
		};

		#if !defined _WINAMP_PLUGIN_
			void Plugin::SaveSpecificChunk(RiffFile* pFile)
			{
				UINT count = GetNumParams();
				UINT size2(0);
				try
				{
					size2 = proxy().GetDataSize();
				}
				catch(const std::exception &)
				{
					// data won't be saved
				}
				UINT size = size2 + sizeof(count) + sizeof(int)*count;
				pFile->Write(&size,sizeof(size));
				pFile->Write(&count,sizeof(count));
				//pFile->Write(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
				for (UINT i = 0; i < count; i++)
				{
					int temp = GetParamValue(i);
					pFile->Write(&temp,sizeof temp);
				}
				if(size2)
				{
					byte * pData = new byte[size2];
					try
					{
						proxy().GetData(pData); // Internal save
					}
					catch(const std::exception &)
					{
						// this sucks because we already wrote the size,
						// so now we have to write the data, even if they are corrupted.
					}
					pFile->Write(pData, size2); // Number of parameters
					zapArray(pData);
				}
			};
		#endif

		void Plugin::Work(int numSamples)
		{
			if(_mode != MACHMODE_GENERATOR) Machine::Work(numSamples);
			#if !defined(_WINAMP_PLUGIN_)
				CPUCOST_INIT(cost);
			#endif
			if (!_mute) 
			{
				if ((_mode == MACHMODE_GENERATOR) || (!_bypass && !_stopped))
				{
					int ns = numSamples;
					int us = 0;
					while (ns)
					{
						int nextevent;
						nextevent = ns+1;
						if (TWSActive)
						{
							nextevent = TWSSamples;
						}
						else
						{
							nextevent = ns+1;
						}
						for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
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
							for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
							{
								// come back to this
								if (TriggerDelay[i]._cmd)
								{
									TriggerDelayCounter[i] -= ns;
								}
							}
							try
							{
								proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, Global::_pSong->SONGTRACKS);
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
									proxy().Work(_pSamplesL+us, _pSamplesR+us, nextevent, Global::_pSong->SONGTRACKS);
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
							for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
							{
								// come back to this
								if (TriggerDelay[i]._cmd == 0xfd)
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
								else if (TriggerDelay[i]._cmd == 0xfb)
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
										TriggerDelayCounter[i] = (RetriggerRate[i]*Global::_pSong->SamplesPerTick)/256;
									}
									else
									{
										TriggerDelayCounter[i] -= nextevent;
									}
								}
								else if (TriggerDelay[i]._cmd == 0xfa)
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
										TriggerDelayCounter[i] = (RetriggerRate[i]*Global::_pSong->SamplesPerTick)/256;
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
							}
						}
					}
					#ifndef _WINAMP_PLUGIN_
						Machine::SetVolumeCounter(numSamples);
						if ( Global::pConfig->autoStopMachines )
						{
							if (_volumeCounter < 8.0f)
							{
								_volumeCounter = 0.0f;
								_volumeDisplay = 0;
								_stopped = true;
							}
							else _stopped = false;
						}
					#endif
				}
			}
			#ifndef _WINAMP_PLUGIN_
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
			#endif // ndef _WINAMP_PLUGIN_
			_worked = true;
		}

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

		int Plugin::GetParamValue(int numparam)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					return proxy().Vals()[numparam];
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
			if(pData->_note == cdefTweakM || pData->_note == cdefTweakE)
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
					#if !defined _WINAMP_PLUGIN_
						Global::_pSong->Tweaker = true;
					#endif
				}
			}
			else if(pData->_note == cdefTweakS)
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
						TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/Global::_pSong->SamplesPerTick;
						TWSSamples = 0;
						TWSActive = TRUE;
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
				#if !defined(_WINAMP_PLUGIN_)
							Global::_pSong->Tweaker = true;
				#endif // ndef _WINAMP_PLUGIN_
			}
		}


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// old file format vomit. don't look at it.



		/// old file format vomit. don't look at it.
		bool Plugin::Load(RiffFile* pFile)
		{
			bool result = true;
			char junk[256];
			memset(&junk, 0, sizeof(junk));

			char sDllName[256];
			int numParameters;


			pFile->Read(sDllName, sizeof(sDllName)); // Plugin dll name
			_strlwr(sDllName);

			//Patch: Automatically replace old AS's by AS2F.
			bool wasAB=false;
			bool wasAS1=false;
			if (strcmp(sDllName,"arguru bass.dll" ) == 0)
			{
				strcpy(sDllName,"arguru synth 2f.dll");
				wasAB=true;
			}
			if (strcmp(sDllName,"arguru synth.dll" ) == 0)
			{
				strcpy(sDllName,"arguru synth 2f.dll");
				wasAS1=true;
			}
			if (!strcmp(sDllName,"arguru synth 2.dll" ))
				strcpy(sDllName,"arguru synth 2f.dll");
			if (!strcmp(sDllName,"synth21.dll" ))
				strcpy(sDllName,"arguru synth 2f.dll");
			std::string sPath2;
			CString sPath;
			#if defined _WINAMP_PLUGIN_
				sPath = Global::pConfig->GetPluginDir();
				if ( FindFileinDir(sDllName,sPath) )
				{
					try
					{
						Instance(sPath);
					}
					catch(...)
					{
						result = false;
					}
				}
				else 
				{
					result = false;
				}
			#else
				if ( !CNewMachine::lookupDllName(sDllName,sPath2) ) 
				{
					// Check Compatibility Table.
					// Probably could be done with the dllNames lockup.
					//GetCompatible(sDllName,sPath2) // If no one found, it will return a null string.
					sPath2 = sDllName;
				}
				
				if ( !CNewMachine::TestFilename(sPath2) ) 
				{
					result = false;
				}
				else 
				{
					try
					{
						Instance(sPath2.c_str());
					}
					catch(...)
					{
						char sError[_MAX_PATH];
						sprintf(sError,"Missing or corrupted native Plug-in \"%s\" - replacing with Dummy.",sDllName);
						::MessageBox(NULL,sError, "Error", MB_OK);
						result = false;
					}
				}
			#endif
			Init();
			pFile->Read(&_editName,16);
			_editName[15] = 0;
			pFile->Read(&numParameters, sizeof(numParameters));
			if(result)
			{
				int *Vals = new int[numParameters];
				pFile->Read(Vals, numParameters*sizeof(int));
				try
				{
					if ( wasAB ) // Patch to replace Arguru Bass by Arguru Synth 2f
					{
						proxy().ParameterTweak(0,Vals[0]);
						for (int i=1;i<15;i++)
						{
							proxy().ParameterTweak(i+4,Vals[i]);
						}
						proxy().ParameterTweak(19,0);
						proxy().ParameterTweak(20,Vals[15]);
						if (numParameters>16)
						{
							proxy().ParameterTweak(24,Vals[16]);
							proxy().ParameterTweak(25,Vals[17]);
						}
					}
					else for (int i=0; i<numParameters; i++)
					{
						proxy().ParameterTweak(i,Vals[i]);
					}
				}
				catch(const std::exception &)
				{
				}
				try
				{
					int size = proxy().GetDataSize();
					//pFile->Read(&size,sizeof(int));	// This SHOULD be the right thing to do
					if(size)
					{
						byte* pData = new byte[size];
						pFile->Read(pData, size); // Number of parameters
						try
						{
							proxy().PutData(pData); // Internal load
						}
						catch(const std::exception &)
						{
						}
						zapArray(pData);
					}
				}
				catch(const std::exception &)
				{
				}
				if(wasAS1) // Patch to replace Synth1 by Arguru Synth 2f
				{
					try
					{
						proxy().ParameterTweak(17,Vals[17]+10);
					}
					catch(const std::exception &)
					{
					}
				}
				zapArray(Vals);
			}
			else
			{
				for (int i=0; i<numParameters; i++)
				{
					pFile->Read(&junk[0], sizeof(int));			
				}
				/*int size;		// This SHOULD be done, but it breaks the fileformat.
				pFile->Read(&size,sizeof(int));
				if (size)
				{
					byte* pData = new byte[size];
					pFile->Read(pData, size); // Number of parameters
					zapArray(pData);
				}*/
			}

			pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
			pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
			pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
			pFile->Read(&_connection[0], sizeof(_connection));
			pFile->Read(&_inputCon[0], sizeof(_inputCon));
			#if defined (_WINAMP_PLUGIN_)
				pFile->Skip(96) ; // sizeof(CPoint) = 8.
			#else
				pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
			#endif
			pFile->Read(&_numInputs, sizeof(_numInputs));
			pFile->Read(&_numOutputs, sizeof(_numOutputs));

			pFile->Read(&_panning, sizeof(_panning));
			Machine::SetPan(_panning);
			pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
			pFile->Read(&junk[0], sizeof(int)); // numSubtracks
			pFile->Read(&junk[0], sizeof(int)); // interpol

			pFile->Read(&junk[0], sizeof(int)); // outwet
			pFile->Read(&junk[0], sizeof(int)); // outdry

			pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
			pFile->Read(&junk[0], sizeof(int)); // distPosClamp
			pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
			pFile->Read(&junk[0], sizeof(int)); // distNegClamp

			pFile->Read(&junk[0], sizeof(char)); // sinespeed
			pFile->Read(&junk[0], sizeof(char)); // sineglide
			pFile->Read(&junk[0], sizeof(char)); // sinevolume
			pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
			pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

			pFile->Read(&junk[0], sizeof(int)); // delayTimeL
			pFile->Read(&junk[0], sizeof(int)); // delayTimeR
			pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
			pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

			pFile->Read(&junk[0], sizeof(int)); // filterCutoff
			pFile->Read(&junk[0], sizeof(int)); // filterResonance
			pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
			pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
			pFile->Read(&junk[0], sizeof(int)); // filterLfophase
			pFile->Read(&junk[0], sizeof(int)); // filterMode

			return result;
		}
	}
}
