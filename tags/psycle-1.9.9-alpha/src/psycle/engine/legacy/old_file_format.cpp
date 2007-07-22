///\file
///\brief implementation file for psycle::host::Global.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/engine/legacy/psy2filter.hpp>
#include <psycle/engine/FileIO.hpp>
#include <psycle/engine/song.hpp>

#include <psycle/engine/machine.hpp>
#include <psycle/engine/legacy/convert_internal_machines.hpp>
#include <psycle/engine/sampler.hpp>
#include <psycle/engine/XMSampler.hpp>
#include <psycle/engine/plugin.hpp>
#include <psycle/engine/VSTHost.hpp>
//todo:
#include <psycle/host/cacheddllfinder.hpp>

#include <cstdint>
namespace psycle
{
	namespace host
	{


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// old file format vomit, don't look at it.

		bool Song::LoadOldFileFormat(RiffFile* pFile, bool fullopen)
		{
			try
			{
				progress.emit(1,0,"");
				progress.emit(2,0,"Loading old song... psycle song fileformat version 2...");
				std::int32_t num,sampR;
				bool _machineActive[128];
				unsigned char busEffect[64];
				unsigned char busMachine[64];
				New();
				pFile->ReadChunk(Name, 32); Name[31]=0;
				pFile->ReadChunk(Author, 32); Author[31]=0;
				pFile->ReadChunk(Comment, 128); Comment[127]=0;
				unsigned int tmp;
				pFile->Read(tmp);
				m_BeatsPerMin=tmp;
				pFile->Read(sampR);
				if( sampR <= 0)
				{
					// Shouldn't happen but has happened.
					m_LinesPerBeat= 4; sampR = 4315;
				}
				else m_LinesPerBeat = 44100 * 15 * 4 / (sampR * m_BeatsPerMin);
				Global::player().bpm = m_BeatsPerMin;
				Global::player().tpb = m_LinesPerBeat;
				// The old (1.0..1.6) format assumes we output at 44100 samples/sec, so...
				Global::player().SamplesPerRow(sampR * Global::player().SampleRate() / 44100);
				pFile->Read(currentOctave);
				pFile->Read(busMachine);
				pFile->Read(playOrder);
				{ std::int32_t tmp; pFile->Read(tmp); playLength = tmp; }
				{ std::int32_t tmp; pFile->Read(tmp); tracks(tmp); }
				// Patterns
				pFile->Read(num);
				Machine::id_type i;
				for(i =0 ; i < num; ++i)
				{
					pFile->Read(patternLines[i]);
					pFile->ReadChunk(patternName[i], 32); patternName[i][31]=0;
					if(patternLines[i] > 0)
					{
						unsigned char * pData(CreateNewPattern(i));
						for(int c(0) ; c < patternLines[i] ; ++c)
						{
							pFile->ReadChunk(pData, OLD_MAX_TRACKS * 5); // OLD_MAX_TRACKS * sizeof(PatternEntry)
							pData += MAX_TRACKS * sizeof(PatternEntry);
						}
					}
					else
					{
						patternLines[i] = 64;
						RemovePattern(i);
					}
				}
				progress.emit(4,2048,"");
				::Sleep(1); ///< ???
				// Instruments
				pFile->Read(instSelected);
				for(i=0 ; i < OLD_MAX_INSTRUMENTS ; ++i)
				{
					pFile->ReadChunk(_pInstrument[i]->_sName,32); _pInstrument[i]->_sName[31]=0;
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_NNA);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_AT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_DT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_SL);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_RT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_AT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_DT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_SL);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_RT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_CO);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_RQ);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_EA);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->ENV_F_TP);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_pan);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_RPAN);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_RCUT);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_RRES);
				}
				
				progress.emit(4,4096,"");
				// Waves
				//
				std::int32_t tmpwvsl;
				pFile->Read(tmpwvsl);

				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					for (int w=0; w<OLD_MAX_WAVES; w++)
					{
						std::uint32_t wltemp;
						pFile->Read(wltemp);
						if (wltemp > 0)
						{
							if ( w == 0 )
							{
								std::int16_t tmpFineTune;
								_pInstrument[i]->waveLength=wltemp;
								pFile->ReadChunk(_pInstrument[i]->waveName, 32); _pInstrument[i]->waveName[31]=0;
								pFile->Read(_pInstrument[i]->waveVolume);
								pFile->Read(tmpFineTune);
								_pInstrument[i]->waveFinetune=tmpFineTune;
								pFile->Read(_pInstrument[i]->waveLoopStart);
								pFile->Read(_pInstrument[i]->waveLoopEnd);
								pFile->Read(_pInstrument[i]->waveLoopType);
								pFile->Read(_pInstrument[i]->waveStereo);
								_pInstrument[i]->waveDataL = new std::int16_t[_pInstrument[i]->waveLength];
								pFile->ReadChunk(_pInstrument[i]->waveDataL, _pInstrument[i]->waveLength * sizeof(std::int16_t));
								if (_pInstrument[i]->waveStereo)
								{
									_pInstrument[i]->waveDataR = new std::int16_t[_pInstrument[i]->waveLength];
									pFile->ReadChunk(_pInstrument[i]->waveDataR, _pInstrument[i]->waveLength * sizeof(std::int16_t));
								}
							}
							else 
							{
								char *junk =new char[42+sizeof(bool)];
								pFile->ReadChunk(junk,sizeof junk);
								delete junk;
								bool stereo;
								pFile->Read(stereo);
								std::int16_t *junk2 = new std::int16_t[wltemp];
								pFile->ReadChunk(junk2, sizeof junk2);
								if ( stereo )
								{
									pFile->ReadChunk(junk2, sizeof junk2);
								}
								delete junk2;
							}
						}
					}
				}
				
				progress.emit(4,4096+2048,"");
				// VST DLLs
				//

				VSTLoader vstL[OLD_MAX_PLUGINS]; 
				for (i=0; i<OLD_MAX_PLUGINS; i++)
				{
					pFile->Read(vstL[i].valid);
					if( vstL[i].valid )
					{
						pFile->ReadChunk(vstL[i].dllName,128); vstL[i].dllName[127]=0;
						_strlwr(vstL[i].dllName);
						pFile->Read(vstL[i].numpars);
						vstL[i].pars = new float[vstL[i].numpars];

						for (int c=0; c<vstL[i].numpars; c++)
						{
							pFile->Read(vstL[i].pars[c]);
						}
					}
				}
				
				progress.emit(4,8192,"");
				// Machines
				//
				_machineLock = true;

				pFile->Read(_machineActive);
				Machine* pMac[128];
				std::memset(pMac,0,sizeof pMac);

				convert_internal_machines::Converter converter;

				for (i=0; i<128; i++)
				{
					Sampler* pSampler;
					XMSampler* pXMSampler;
					Plugin* pPlugin;
					vst::plugin * pVstPlugin(0);
					std::int32_t x,y,type;
					if (_machineActive[i])
					{
						progress.emit(4,8192+i*(4096/128),"");

						pFile->Read(x);
						pFile->Read(y);

						pFile->Read(type);

						if(converter.plugin_names().exists(type))
							pMac[i] = &converter.redirect(i, type, *pFile);
						else switch (type)
						{
						case MACH_PLUGIN:
							{
								pMac[i] = pPlugin = new Plugin(i);
								// Should the "Init()" function go here? -> No. Needs to load the dll first.
								if (!pMac[i]->LoadOldFileFormat(pFile))
								{
									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(*((Dummy*)pOldMachine));
									// dummy name goes here
									std::stringstream s;
									s << "X!" << pOldMachine->GetEditName();
									pOldMachine->SetEditName(s.str());
									pMac[i]->_subclass = MACH_DUMMY;
									pOldMachine->_pSamplesL = NULL;
									pOldMachine->_pSamplesR = NULL;
									zapObject(pOldMachine);
								}
								break;
							}
						case MACH_VST:
						case MACH_VSTFX:
							if (type == MACH_VST) pMac[i] = pVstPlugin = new vst::instrument(i);
							else if (type == MACH_VSTFX)	pMac[i] = pVstPlugin = new vst::fx(i);
							goto init_and_load_VST;
						case MACH_MASTER:
							pMac[i] = _pMachine[MASTER_INDEX];
							goto init_and_load;
						case MACH_SAMPLER:
							pMac[i] = pSampler = new Sampler(i);
							goto init_and_load;
						case MACH_XMSAMPLER:
							pMac[i] = pXMSampler = new XMSampler(i);
							goto init_and_load;
						case MACH_SCOPE:
						case MACH_DUMMY:
							pMac[i] = new Dummy(i);
							goto init_and_load;
						default:
							{
								std::ostringstream s;
								s << "unkown machine type: " << type;
								MessageBox(0, s.str().c_str(), "Loading old song", MB_ICONERROR);
							}
							pMac[i] = new Dummy(i);

						init_and_load:
							pMac[i]->Init();
							pMac[i]->LoadOldFileFormat(pFile);
							break;

						init_and_load_VST:
							if ((pMac[i]->LoadOldFileFormat(pFile)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
							{
								std::string path = vstL[pVstPlugin->_instance].dllName;
								if(!Global::dllfinder().LookupDllPath(path,MACH_VST)) 
								{
									try
									{
										pVstPlugin->LoadDll(path,false);
									}
									catch(...)
									{
										std::ostringstream ss;
										ss << "Plugin instancation threw an exception " << path << " - replacing with Dummy.";
										MessageBox(NULL,ss.str().c_str(), "Loading Error", MB_OK);

										Machine* pOldMachine = pMac[i];
										pMac[i] = new Dummy(*((Dummy*)pOldMachine));
										pOldMachine->_pSamplesL = NULL;
										pOldMachine->_pSamplesR = NULL;
										// dummy name goes here
										std::stringstream ss2;
										ss2 << "X!" << pOldMachine->GetEditName();
										pOldMachine->SetEditName(ss2.str());
										zapObject(pOldMachine);
										pMac[i]->_subclass = MACH_DUMMY;
										((Dummy*)pMac[i])->wasVST = true;
									}
								}
								else
								{
									std::ostringstream ss;
									ss << "Missing  or disabled VST plug-in: " << vstL[pVstPlugin->_instance].dllName;
									MessageBox(NULL,ss.str().c_str(), "Loading Error", MB_OK);

									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(*((Dummy*)pOldMachine));
									pOldMachine->_pSamplesL = NULL;
									pOldMachine->_pSamplesR = NULL;
									// dummy name goes here
									std::stringstream ss2;
									ss2 << "X!" << pOldMachine->GetEditName();
									pOldMachine->SetEditName(ss2.str());
									zapObject(pOldMachine);
									pMac[i]->_subclass = MACH_DUMMY;
									((Dummy*)pMac[i])->wasVST = true;
								}
							}
							else
							{
								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								// dummy name goes here
								std::stringstream ss;
								ss << "X!" << pOldMachine->GetEditName();
								pOldMachine->SetEditName(ss.str());
								zapObject(pOldMachine);
								pMac[i]->_subclass = MACH_DUMMY;
								((Dummy*)pMac[i])->wasVST = true;
							}

						}
						pMac[i]->SetPosX(x);
						pMac[i]->SetPosY(y);
					}
				}

				progress.emit(4,8192+4096,"");

				// Since the old file format stored volumes on each output
				// rather than on each input, we must convert
				//
				float volMatrix[128][MAX_CONNECTIONS];
				for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
				{
					if (!_machineActive[i])
					{
						zapObject(pMac[i]);
					}
					else if (!pMac[i])
					{
						_machineActive[i] = false;
					}
					else 
					{
						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							volMatrix[i][c] = pMac[i]->_inputConVol[c];
						}
					}
				}
				
				progress.emit(4,8192+4096+1024,"");

				for (i=0; i<128; i++) // Next, we go to fix this for each
				{
					if (_machineActive[i])		// valid machine (important, since we have to navigate!)
					{
						for (int c=0; c<MAX_CONNECTIONS; c++) // all of its input connections.
						{
							if (pMac[i]->_inputCon[c])	// If there's a valid machine in this inputconnection,
							{
								Machine* pOrigMachine = pMac[pMac[i]->_inputMachines[c]]; // We get that machine
								int d = pOrigMachine->FindOutputWire(i);

								float val = volMatrix[pMac[i]->_inputMachines[c]][d];
								if( val >= 4.000001f ) 
								{
									val*=0.000030517578125f; // BugFix
								}
								else if ( val < 0.00004f) 
								{
									val*=32768.0f; // BugFix
								}

								pMac[i]->InitWireVolume(pOrigMachine->subclass(),c,val);
							}
						}
					}
				}
				
				progress.emit(4,8192+4096+2048,"");

				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_loop);
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(_pInstrument[i]->_lines);
				}

				if ( pFile->Read(busEffect) == false ) // Patch 1: BusEffects (twf)
				{
					int j=0;
					for ( i=0;i<128;i++ ) 
					{
						if (_machineActive[i] && pMac[i]->_mode != MACHMODE_GENERATOR )
						{
							busEffect[j]=i;	
							j++;
						}
					}
					for (j; j < 64; j++)
					{
						busEffect[j] = 255;
					}
				}
				// Patch 1.2: Fixes crash/inconsistence when deleting a machine which couldn't be loaded
				// (.dll not found, or Load failed), which is, then, replaced by a DUMMY machine.
				int j=0;
				for ( i=0;i<64;i++ ) 
				{
					if (busMachine[i] != 255 && _machineActive[busMachine[i]]) 
					{ // If there's a machine in the generators' bus that it is not a generator:
						if (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR ) 
						{
							pMac[busMachine[i]]->_mode = MACHMODE_FX;
							while (busEffect[j] != 255 && j<MAX_BUSES) 
							{
								j++;
							}
							busEffect[j]=busMachine[i];
							busMachine[i]=255;
						}
					}
				}
				for ( i=0;i<64;i++ ) 
				{
					if ((busMachine[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)) 
					{
						busMachine[i] = 255;
					}
					if ((busEffect[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busEffect[i]]->_mode != MACHMODE_FX)) 
					{
						busEffect[i] = 255;
					}
				}

				bool chunkpresent=false;
				pFile->Read(chunkpresent); // Patch 2: VST's Chunk.

				if ( fullopen ) for ( i=0;i<128;i++ ) 
				{
					if (_machineActive[i])
					{
						if ( pMac[i]->subclass() == MACH_DUMMY ) 
						{
							if (((Dummy*)pMac[i])->wasVST && chunkpresent )
							{
								// Since we don't know if the plugin saved it or not, 
								// we're stuck on letting the loading crash/behave incorrectly.
								// There should be a flag, like in the VST loading Section to be correct.
								MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
							}
						}
						else if (( pMac[i]->subclass() == MACH_VST ) || 
								( pMac[i]->subclass() == MACH_VSTFX))
						{
							bool chunkread = false;
							try
							{
								vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
								if(chunkpresent) chunkread = plugin.LoadChunkOldFileFormat(pFile);
								plugin.proxy().dispatcher(effSetProgram, 0, plugin._program);
							}
							catch(const std::exception &)
							{
								// o_O`
							}
							if(!chunkpresent || !chunkread)
							{
								vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
								const int vi = plugin._instance;
								const int numpars = vstL[vi].numpars;
								for (int c(0) ; c < numpars; ++c)
								{
									try
									{
										plugin.proxy().setParameter(c, vstL[vi].pars[c]);
									}
									catch(const std::exception &)
									{
										// o_O`
									}
								}
							}
						}
					}
				}
				for (i=0; i<OLD_MAX_PLUGINS; i++) // Clean "pars" array.
				{
					if( vstL[i].valid )
					{
						zapObject(vstL[i].pars);
					}
				}

				// move machines around to where they really should go
				// now we have to remap all the inputs and outputs again... ouch
				
				progress.emit(4,8192+4096+2048+1024,"");

				for (i = 0; i < 64; i++)
				{
					if ((busMachine[i] < MAX_MACHINES-1) && (busMachine[i] > 0))
					{
						if (_machineActive[busMachine[i]])
						{
							if (pMac[busMachine[i]]->_mode == MACHMODE_GENERATOR)
							{
								_pMachine[i] = pMac[busMachine[i]];
								_machineActive[busMachine[i]] = false; // don't update this twice;

								for (int c=0; c<MAX_CONNECTIONS; c++)
								{
									if (_pMachine[i]->_inputCon[c])
									{
										for (int x=0; x<64; x++)
										{
											if (_pMachine[i]->_inputMachines[c] == busMachine[x])
											{
												_pMachine[i]->_inputMachines[c] = x;
												break;
											}
											else if (_pMachine[i]->_inputMachines[c] == busEffect[x])
											{
												_pMachine[i]->_inputMachines[c] = x+MAX_BUSES;
												break;
											}
										}
									}

									if (_pMachine[i]->_connection[c])
									{
										if (_pMachine[i]->_outputMachines[c] == 0)
										{
											_pMachine[i]->_outputMachines[c] = MASTER_INDEX;
										}
										else
										{
											for (int x=0; x<64; x++)
											{
												if (_pMachine[i]->_outputMachines[c] == busMachine[x])
												{
													_pMachine[i]->_outputMachines[c] = x;
													break;
												}
												else if (_pMachine[i]->_outputMachines[c] == busEffect[x])
												{
													_pMachine[i]->_outputMachines[c] = x+MAX_BUSES;
													break;
												}
											}
										}
									}
								}
							}
						}
					}
					if ((busEffect[i] < MAX_MACHINES-1) && (busEffect[i] > 0))
					{
						if (_machineActive[busEffect[i]])
						{
							if (pMac[busEffect[i]]->_mode == MACHMODE_FX)
							{
								_pMachine[i+MAX_BUSES] = pMac[busEffect[i]];
								_machineActive[busEffect[i]] = false; // don't do this again

								for (int c=0; c<MAX_CONNECTIONS; c++)
								{
									if (_pMachine[i+MAX_BUSES]->_inputCon[c])
									{
										for (int x=0; x<64; x++)
										{
											if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busMachine[x])
											{
												_pMachine[i+MAX_BUSES]->_inputMachines[c] = x;
												break;
											}
											else if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busEffect[x])
											{
												_pMachine[i+MAX_BUSES]->_inputMachines[c] = x+MAX_BUSES;
												break;
											}
										}
									}
									if (_pMachine[i+MAX_BUSES]->_connection[c])
									{
										if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == 0)
										{
											_pMachine[i+MAX_BUSES]->_outputMachines[c] = MASTER_INDEX;
										}
										else
										{
											for (int x=0; x<64; x++)
											{
												if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busMachine[x])
												{
													_pMachine[i+MAX_BUSES]->_outputMachines[c] = x;
													break;
												}
												else if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busEffect[x])
												{
													_pMachine[i+MAX_BUSES]->_outputMachines[c] = x+MAX_BUSES;
													break;
												}
											}
										}
									}
								}
							}
						}
					}
				}

				// fix machine #s

				for (i = 0; i < MAX_MACHINES-1; i++)
				{
					if (_pMachine[i])
					{
						_pMachine[i]->_macIndex = i;
						for (j = i+1; j < MAX_MACHINES-1; j++)
						{
							if (_pMachine[i] == _pMachine[j])
							{
								assert(false);
								// we have duplicate machines...
								// this should NEVER happen
								// delete the second one :(
								_pMachine[j] = NULL;
								// and we should remap anything that had wires to it to the first one
							}
						}
					}
				}
				
				progress.emit(4,8192+4096+2048+1024+512,"");
				// test all connections

				for (int c=0; c<MAX_CONNECTIONS; c++)
				{
					if (_pMachine[MASTER_INDEX]->_inputCon[c])
					{
						for (int x=0; x<64; x++)
						{
							if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busMachine[x])
							{
								_pMachine[MASTER_INDEX]->_inputMachines[c] = x;
								break;
							}
							else if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busEffect[x])
							{
								_pMachine[MASTER_INDEX]->_inputMachines[c] = x+MAX_BUSES;
								break;
							}
						}
					}
				}
				
				// test all connections for invalid machines. disconnect invalid machines.
				for (i = 0; i < MAX_MACHINES; i++)
				{
					if (_pMachine[i])
					{
						_pMachine[i]->_connectedInputs = 0;
						_pMachine[i]->_connectedOutputs = 0;

						for (int c = 0; c < MAX_CONNECTIONS; c++)
						{
							if (_pMachine[i]->_connection[c])
							{
								if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_connection[c]=false;
									_pMachine[i]->_outputMachines[c]=-1;
								}
								else if (!_pMachine[_pMachine[i]->_outputMachines[c]])
								{
									_pMachine[i]->_connection[c]=false;
									_pMachine[i]->_outputMachines[c]=-1;
								}
								else 
								{
									_pMachine[i]->_connectedOutputs++;
								}
							}
							else
							{
								_pMachine[i]->_outputMachines[c]=255;
							}

							if (_pMachine[i]->_inputCon[c])
							{
								if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES-1)
								{
									_pMachine[i]->_inputCon[c]=false;
									_pMachine[i]->_inputMachines[c]=-1;
								}
								else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
								{
									_pMachine[i]->_inputCon[c]=false;
									_pMachine[i]->_inputMachines[c]=-1;
								}
								else
								{
									_pMachine[i]->_connectedInputs++;
								}
							}
							else
							{
								_pMachine[i]->_inputMachines[c]=255;
							}
						}
					}
				}
				if(fullopen) converter.retweak(*this);
				_machineLock = false;
				seqBus=0;
				
				progress.emit(5,0,"");
			}
			catch(...)
			{
				std::ostringstream s;
				s << "Error reading from " << pFile->file_name() << " !!!";
				MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
				progress.emit(5,0,"");
				return false;
			}
			return true;
		}

		bool Machine::LoadOldFileFormat(RiffFile* pFile)
		{
			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			pFile->Skip(4); // outwet
			pFile->Skip(4); // outdry

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Master::LoadOldFileFormat(RiffFile* pFile)
		{
			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;
			
			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);
			
			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			/////////////
			_outDry = 0; pFile->Read(_outDry); // outdry
			/////////////

			pFile->Skip(4); // outwet
			
			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Sampler::LoadOldFileFormat(RiffFile* pFile)
		{
			int i;

			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);
			pFile->Skip(4*8); // SubTrack[]
			pFile->Read(_numVoices); // numSubtracks

			if (_numVoices < 4)
			{
				// Psycle versions < 1.1b2 had polyphony per channel,not per machine.
				_numVoices = 8;
			}

			pFile->Read(i); // interpol
			switch (i)
			{
			case 2:
				_resampler.SetQuality(dsp::R_SPLINE);
				break;
			case 0:
				_resampler.SetQuality(dsp::R_NONE);
				break;
			default:
			case 1:
				_resampler.SetQuality(dsp::R_LINEAR);
				break;
			}

			pFile->Skip(4); // outdry
			pFile->Skip(4); // outwet

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Plugin::LoadOldFileFormat(RiffFile* pFile)
		{
			bool result = true;
			char junk[256];
			std::memset(junk, 0, sizeof junk);

			char sDllName[256];
			int numParameters;

			pFile->Read(sDllName); // Plugin dll name
			sDllName[255]='\0';
			std::string strname = sDllName;
			std::transform(strname.begin(),strname.end(),strname.begin(),std::tolower);

			//Patch: Automatically replace old AS's by AS2F.
			bool wasAB=false;
			bool wasAS1=false;
			bool wasAS2=false;
			if (strname == "arguru bass.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAB=true;
			}
			else if (strname == "arguru synth.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAS1=true;
			}
			else if (strname == "arguru synth 2.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAS2=true;
			}
			else if (strname == "synth21.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAS2=true;
			}

			Global::dllfinder().LookupDllPath(strname,MACH_PLUGIN);
			try
			{
				LoadDll(strname);
			}
			catch(...)
			{
				char sError[_MAX_PATH];
				sprintf(sError,"Missing or corrupted native Plug-in \"%s\" - replacing with Dummy.",sDllName);
				MessageBox(NULL,sError, "Error", MB_OK);
				result = false;
			}

			Init();

			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;

			pFile->Read(numParameters);
			if(result)
			{
				std::int32_t * Vals = new std::int32_t[numParameters];
				pFile->ReadChunk(Vals, numParameters * sizeof(std::int32_t));
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
						else
						{
							proxy().ParameterTweak(24,0);
							proxy().ParameterTweak(25,0);
						}
					}
					else for (int i=0; i<numParameters; i++)
					{
						proxy().ParameterTweak(i,Vals[i]);
					}
				}
				catch(const std::exception &)
				{
					loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				try
				{
					int size = proxy().GetDataSize();
					//pFile->Read(size);	// This would have been the right thing to do
					if(size)
					{
						char * pData = new char[size];
						pFile->ReadChunk(pData, size); // Number of parameters
						try
						{
							proxy().PutData(pData); // Internal load
						}
						catch(const std::exception &)
						{
						}
						delete[] pData;
					}
				}
				catch(std::exception const &)
				{
					loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				if(wasAS1) // Patch to replace Synth1 by Arguru Synth 2f
				{
					try
					{
						proxy().ParameterTweak(17,Vals[17]+10);
						proxy().ParameterTweak(24,0);
						proxy().ParameterTweak(25,0);
					}
					catch(const std::exception &)
					{
					}
				}
				if(wasAS2)
				{
					try
					{
						proxy().ParameterTweak(24,0);
						proxy().ParameterTweak(25,0);
					}
					catch(const std::exception&)
					{
					}
				}
				delete[] Vals;
			}
			else
			{
				for (int i=0; i<numParameters; i++)
				{
					pFile->ReadChunk(junk, 4);
				}
			}

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			pFile->Skip(4); // outwet
			pFile->Skip(4); // outdry

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return result;
		}

		namespace vst
		{
			bool plugin::LoadChunkOldFileFormat(RiffFile * pFile)
			{
				bool b;
				try
				{
					b = proxy().flags() & effFlagsProgramChunks;
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(!b) return false;

				// read chunk size
				std::uint32_t chunk_size;
				pFile->Read(chunk_size);

				// read chunk data
				char * chunk(new char[chunk_size]);
				pFile->ReadChunk(chunk, chunk_size);

				try
				{
					proxy().dispatcher(effSetChunk, 0, chunk_size, chunk);
				}
				catch(const std::exception &)
				{
					// [bohan] hmm, so, data just gets lost?
					delete[] chunk;
					return false;
				}

				delete[] chunk;
				return true;
			}

			bool plugin::LoadOldFileFormat(RiffFile * pFile)
			{
				Machine::Init();

				char edName[32];
				pFile->ReadChunk(edName, 16); edName[15] = 0;
				_editName=edName;

				pFile->Read(_inputMachines);
				pFile->Read(_outputMachines);
				pFile->Read(_inputConVol);
				pFile->Read(_connection);
				pFile->Read(_inputCon);
				pFile->Read(_connectionPoint);
				pFile->Read(_connectedInputs);
				pFile->Read(_connectedOutputs);

				pFile->Read(_panning);
				Machine::SetPan(_panning);

				pFile->Skip(4*8); // SubTrack[]
				pFile->Skip(4); // numSubtracks
				pFile->Skip(4); // interpol

				pFile->Skip(4); // outwet
				pFile->Skip(4); // outdry

				pFile->Skip(4); // distPosThreshold
				pFile->Skip(4); // distPosClamp
				pFile->Skip(4); // distNegThreshold
				pFile->Skip(4); // distNegClamp

				pFile->Skip(1); // sinespeed
				pFile->Skip(1); // sineglide
				pFile->Skip(1); // sinevolume
				pFile->Skip(1); // sinelfospeed
				pFile->Skip(1); // sinelfoamp

				pFile->Skip(4); // delayTimeL
				pFile->Skip(4); // delayTimeR
				pFile->Skip(4); // delayFeedbackL
				pFile->Skip(4); // delayFeedbackR

				pFile->Skip(4); // filterCutoff
				pFile->Skip(4); // filterResonance
				pFile->Skip(4); // filterLfospeed
				pFile->Skip(4); // filterLfoamp
				pFile->Skip(4); // filterLfophase
				pFile->Skip(4); // filterMode

				bool old;
				pFile->Read(old); // old format
				pFile->Read(_instance); // ovst.instance
				if(old)
				{
					char mch;
					pFile->Read(mch);
					_program = 0;
				}
				else
				{
					pFile->Read(_program);
				}
				return true;
			}
		}
	}
}
