/***************************************************************************
	*   Copyright (C) 2007 Psycledelics   *
	*   psycle.sf.net   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#include "plugin.h"
//#include "configuration.h"
//#include "inputhandler.h"
#ifdef __unix__
#include <dlfcn.h>
#else
	#include <windows.h>
	#ifdef _MSC_VER
	#undef min 
	#undef max
	#endif
#endif
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <cctype>

// win32 note : mingw cannot load our shipped 1.8.2 binary plugins due c++ this and std calling convention

namespace psy {
	namespace core {

typedef CMachineInfo * (* GETINFO) ();
typedef CMachineInterface * (* CREATEMACHINE) ();


PluginFxCallback Plugin::_callback;

Plugin::Plugin(Machine::id_type id , Song* song)
			:
				Machine(MACH_PLUGIN, MACHMODE_FX, id, song),
				_dll(0),
				proxy_(*this)
			{
				_audiorange=32768.0f;
				_editName = "native plugin";
			}

Plugin::~ Plugin( ) throw()
{
		#ifdef __unix__
		#else
		if  ( _dll ) {
				::FreeLibrary( (HINSTANCE) _dll ) ;
		}
		#endif          
}

bool Plugin::Instance( const std::string & file_name )
{      
		try {
		#ifdef __unix__
		_dll = dlopen(file_name.c_str(), RTLD_LAZY);
		#else
		if ( file_name.find(".dll") == std::string::npos) return false;
		// Set error mode to disable system error pop-ups (for LoadLibrary)
		UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		_dll = LoadLibraryA( file_name.c_str() );
		// Restore previous error mode
		SetErrorMode( uOldErrorMode );
		#endif
		if (!_dll) {
				#ifdef __unix__
//				std::cerr << "Cannot load library: " << dlerror() << '\n';
				#else
				#endif
				return false;
		} else {
			GETINFO GetInfo  = 0;
			#ifdef __unix__
			GetInfo = (GETINFO) dlsym( _dll, "GetInfo");
			#else
			GetInfo = (GETINFO) GetProcAddress( static_cast<HINSTANCE>( _dll ), "GetInfo" );
			#endif
			if (!GetInfo) {
				#ifdef __unix__
//				std::cerr << "Cannot load symbols: " << dlerror() << '\n';
				#else
				#endif
				return false;
			} else {
				_pInfo = GetInfo();
//				std::cout << _pInfo->Author << std::endl;

				if(_pInfo->Version < MI_VERSION) std::cerr << "plugin format is too old" << _pInfo->Version << file_name <<std::endl;
				fflush(stdout);
				_isSynth = _pInfo->Flags == 3;
				if(_isSynth) _mode = MACHMODE_GENERATOR;
			strncpy(_psShortName,_pInfo->ShortName,15);
			_psShortName[15]='\0';
			char buf[32];
			strncpy(buf, _pInfo->ShortName,31);
			buf[31]='\0';
			_editName = buf;
			_psAuthor = _pInfo->Author;
			_psName = _pInfo->Name;
			CREATEMACHINE GetInterface = 0;
			#ifdef __unix__
			GetInterface =  (CREATEMACHINE) dlsym(_dll, "CreateMachine");
			#else
			GetInterface = (CREATEMACHINE) GetProcAddress( (HINSTANCE)_dll, "CreateMachine" );
			#endif
			if(!GetInterface) {
					#ifdef __unix__
//					std::cerr << "Cannot load symbol: " << dlerror() << "\n";
					#else
					#endif
					return false;
			} else {
					proxy()(GetInterface());
			}
		}
		}
	return true;
	} catch (...) {
		return false;
	}
}


void Plugin::Init( )
{
	Machine::Init();
	
	if(proxy()())
	{
		proxy().Init();
		for(int gbp(0) ; gbp < GetInfo()->numParameters ; ++gbp)
		{
			proxy().ParameterTweak(gbp, _pInfo->Parameters[gbp]->DefValue);
		}
	}
}

int Plugin::GenerateAudioInTicks(int startSample,  int numSamples )
{
	int ns = numSamples;
	int us = startSample;
	if(_mode == MACHMODE_GENERATOR)
	{
		if (!_mute) _stopped = false;
		else _stopped = true;
	}

	if (!_mute) {
		if((_mode == MACHMODE_GENERATOR) || (!_bypass && !_stopped)) {
			proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, song()->tracks());
		}
	}
	return numSamples;
/*
	if (!_mute) {
		if ((_mode == MACHMODE_GENERATOR) || (!_bypass && !_stopped)) {
			int ns = numSamples;
			int us = startSample;
					while (ns)
					{
						int nextevent = (TWSActive)?TWSSamples:ns+1;
						for (int i=0; i < song()->tracks(); i++)
						{
							if (TriggerDelay[i]._cmd) {
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
								for (int i=0; i < song()->tracks(); i++)
								{
									// come back to this
									if (TriggerDelay[i]._cmd)
									{
										TriggerDelayCounter[i] -= ns;
									}
								}
								try
								{
									proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, song()->tracks());
								}
								catch(const std::exception &)
								{
								}
								ns = 0;
						} else {
										if(nextevent) {
												ns -= nextevent;
												try
												{
													proxy().Work(_pSamplesL+us, _pSamplesR+us, nextevent, song()->tracks());
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
															if (((TWSDelta[i] > 0) && (TWSCurrent[i] >= TWSDestination[i])) || ((TWSDelta[i] < 0) && (TWSCurrent[i] <= TWSDestination[i])))
															{
																	TWSCurrent[i] = TWSDestination[i];
																	TWSDelta[i] = 0;
															} else
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
										for (int i=0; i < song()->tracks(); i++)
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
														TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer()->SamplesPerRow())/256;
													} else
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
														TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer()->SamplesPerRow())/256;
														int parameter = TriggerDelay[i]._parameter&0x0f;
														if (parameter < 9)
														{
																RetriggerRate[i]+= 4*parameter;
														} else
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
												} else if (TriggerDelay[i]._cmd == PatternCmd::ARPEGGIO)
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
																TriggerDelayCounter[i] = Global::pPlayer()->SamplesPerRow()*Global::pPlayer()->tpb/24;
																} else {
																		TriggerDelayCounter[i] -= nextevent;
																}
															}
													}
												}

						}
						Machine::SetVolumeCounter(numSamples);
						if ( Global::pConfig()->autoStopMachines )
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
*/	  
			//CPUCOST_CALC(cost, numSamples);
			//_cpuCost += cost;
			_worked = true;
}

void Plugin::Tick( )
{
	try {
			proxy().SequencerTick();
	}
	catch(const std::exception &) {
	}
}

void Plugin::Tick( int channel, const PatternEvent & pData )
{
	const PlayerTimeInfo & timeInfo = Player::Instance()->timeInfo();

	try
	{
			proxy().SeqTick(channel, pData.note(), pData.instrument(), pData.command(), pData.parameter());
	}
	catch(const std::exception &)
	{
			return;
	}
	if(pData.note() == notecommands::tweak || pData.note() == notecommands::tweakeffect)
	{
			if( pData.instrument() < _pInfo->numParameters)
			{
				int nv = (pData.command() << 8) +pData.parameter();
					int const min = _pInfo->Parameters[pData.instrument()]->MinValue;
					int const max = _pInfo->Parameters[pData.instrument()]->MaxValue;
					nv += min;
					if(nv > max) nv = max;
					try
					{
						proxy().ParameterTweak(pData.instrument(), nv);
					}
					catch(const std::exception &)
					{
					}
					Player::Instance()->Tweaker = true;
				}
			}
	else if(pData.note() == notecommands::tweakslide)
			{
				if(pData.instrument() < _pInfo->numParameters)
				{
					int i;
					if(TWSActive)
					{
						// see if a tweak slide for this parameter is already happening
						for(i = 0; i < MAX_TWS; i++)
						{
							if((TWSInst[i] == pData.instrument()) && (TWSDelta[i] != 0))
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
						TWSDestination[i] = float(pData.command() << 8)+pData.parameter();
						float min = float(_pInfo->Parameters[pData.instrument()]->MinValue);
						float max = float(_pInfo->Parameters[pData.instrument()]->MaxValue);
						TWSDestination[i] += min;
						if (TWSDestination[i] > max)
						{
							TWSDestination[i] = max;
						}
						TWSInst[i] = pData.instrument();
						try
						{
							TWSCurrent[i] = float(proxy().Vals()[TWSInst[i]]);
						}
						catch(const std::exception &)
						{
						}
						TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/ timeInfo.samplesPerRow();
						TWSSamples = 0;
						TWSActive = true;
					}
					else
					{
						// we have used all our slots, just send a twk
						int nv = (pData.command() << 8)+pData.parameter();
						int const min = _pInfo->Parameters[pData.instrument()]->MinValue;
						int const max = _pInfo->Parameters[pData.instrument()]->MaxValue;
						nv += min;
						if (nv > max) nv = max;
						try
						{
							proxy().ParameterTweak(pData.instrument(), nv);
						}
						catch(const std::exception &)
						{
						}
					}
				}
					Player::Instance()->Tweaker = true;
			}
}

void Plugin::Stop( )
{
	try {
			proxy().Stop();
	}
	catch(const std::exception &)
	{
	}
	Machine::Stop();
}

struct ToLower
		{
			char operator() (char c) const  { return std::tolower(c); }
		};

bool Plugin::LoadDll( std::string psFileName ) // const is here not possible cause we modify it
{
	#ifdef __unix__        
	std::transform(psFileName.begin(),psFileName.end(),psFileName.begin(),ToLower());
	if (psFileName.find(".so")== std::string::npos) {
		_psDllName = psFileName;
		int i = psFileName.find(".dll");
		std::string withoutSuffix = psFileName.substr(0,i);
		std::string soName = withoutSuffix + ".so";
		psFileName = "lib-xpsycle.plugin."+soName;
		psFileName = Global::configuration().pluginPath() + psFileName; 
		int pos;
		while((pos = psFileName.find(' ')) != std::string::npos) psFileName[pos] = '_';
	} else {
			int i = psFileName.find("lib-xpsycle.plugin.");
			if (i!=std::string::npos) {
					int j = psFileName.find(".so");
					if (j!=0) {
						_psDllName = psFileName.substr(0,j);
						_psDllName.erase(0,std::string("lib-xpsycle.plugin.").length());
						_psDllName = _psDllName + ".dll";
					} else {
						_psDllName = psFileName;
						_psDllName.erase(0,std::string("lib-xpsycle.plugin.").length());
						_psDllName = _psDllName + ".dll";
					}
			} else _psDllName = psFileName;

    		psFileName = Global::configuration().pluginPath() + psFileName; 
	}
	#else
	_psDllName = psFileName;
	psFileName = Global::pConfig()->pluginPath() + psFileName;
	#endif   
	return Instance(psFileName);
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
		catch (...) {
		}
			}
			else std::strcpy(parval,"Out of Range");
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


		void Plugin::SaveDllName(RiffFile * pFile) 
		{
			pFile->WriteChunk(_psDllName.c_str(), _psDllName.length() + 1);
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
//					MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
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
							delete pData;
							return false;
						}
						delete pData;
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
}
}
