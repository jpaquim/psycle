// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007 psycledelics http://psycle.sourceforge.net
///\file
///\implementation psycle::file_format_upgrade::detail::version3::root
namespace psycle {
	namespace file_format_upgrade {
		namespace detail {
			namespace version3 {
				namespace {
					/// maximum num of patterns
					int const MAX_PATTERNS = 256;
					/// maximum num of tracks
					int const MAX_TRACKS = 64;
					/// maximum num of machines (+1 for master)
					int const MAX_MACHINES = 129;
					/// maximum num of machines (+1 for master)
					int const MAX_INTRUMENTS = 129;
				}

				void root::read(input & in)
				{
					CProgressDialog Progress;
					Progress.Create();
					Progress.SetWindowText("Loading old format...");
					Progress.ShowWindow(SW_SHOW);
					int i;
					int num,sampR;
					bool _machineActive[128];
					unsigned char busEffect[64];
					unsigned char busMachine[64];
					char name_[129]; char author_[65]; char comments_[65536];
					pFile->Read(name_, 32);
					pFile->Read(author_, 32);
					pFile->Read(comments_,128);
					name = name_;
					author = author_;
					comments = comments_;

					pFile->Read(&m_BeatsPerMin, sizeof m_BeatsPerMin);
					pFile->Read(&sampR, sizeof sampR);
					if( sampR <= 0)
					{
						// Shouldn't happen but has happened.
						m_LinesPerBeat= 4;
					}
					// The old format assumes we output at 44100 samples/sec, so...
					else m_LinesPerBeat = 44100 * 60 / (sampR * m_BeatsPerMin);
					Global::pPlayer->SetBPM(m_BeatsPerMin,m_LinesPerBeat);
	//				Global::pPlayer->bpm = m_BeatsPerMin;
	//				Global::pPlayer->tpb = m_LinesPerBeat;
	//				Global::pPlayer->SamplesPerRow(sampR * Global::pConfig->_pOutputDriver->_samplesPerSec / 44100);
					pFile->Read(&currentOctave, sizeof(char));
					pFile->Read(busMachine, 64);
					pFile->Read(playOrder, 128);
					pFile->Read(&playLength, sizeof(int));
					pFile->Read(&SONGTRACKS, sizeof(int));
					// Patterns
					pFile->Read(&num, sizeof num);
					for(i =0 ; i < num; ++i)
					{
						pFile->Read(&patternLines[i], sizeof *patternLines);
						pFile->Read(&patternName[i][0], sizeof *patternName);
						if(patternLines[i] > 0) {
							unsigned char * pData(CreateNewPattern(i));
							for(int c(0) ; c < patternLines[i] ; ++c) {
								pFile->Read(reinterpret_cast<char*>(pData), OLD_MAX_TRACKS * sizeof(PatternEntry));
								pData += MAX_TRACKS * sizeof(PatternEntry);
							}
						} else {
							patternLines[i] = 64;
							RemovePattern(i);
						}
					}

					// Instruments
					pFile->Read(&instSelected, sizeof instSelected);
					for(i=0 ; i < OLD_MAX_INSTRUMENTS ; ++i)
						pFile->Read(&_pInstrument[i]->_sName, sizeof(_pInstrument[0]->_sName));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_NNA, sizeof(_pInstrument[0]->_NNA));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_AT, sizeof(_pInstrument[0]->ENV_AT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_DT, sizeof(_pInstrument[0]->ENV_DT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_SL, sizeof(_pInstrument[0]->ENV_SL));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_RT, sizeof(_pInstrument[0]->ENV_RT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_AT, sizeof(_pInstrument[0]->ENV_F_AT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_DT, sizeof(_pInstrument[0]->ENV_F_DT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_SL, sizeof(_pInstrument[0]->ENV_F_SL));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_RT, sizeof(_pInstrument[0]->ENV_F_RT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_CO, sizeof(_pInstrument[0]->ENV_F_CO));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_RQ, sizeof(_pInstrument[0]->ENV_F_RQ));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_EA, sizeof(_pInstrument[0]->ENV_F_EA));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->ENV_F_TP, sizeof(_pInstrument[0]->ENV_F_TP));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_pan, sizeof(_pInstrument[0]->_pan));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_RPAN, sizeof(_pInstrument[0]->_RPAN));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_RCUT, sizeof(_pInstrument[0]->_RCUT));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_RRES, sizeof(_pInstrument[0]->_RRES));
					
					// Waves
					//
					int tmpwvsl;
					pFile->Read(&tmpwvsl, sizeof(int));

					for (i=0; i<OLD_MAX_INSTRUMENTS; i++) {
						for (int w=0; w<OLD_MAX_WAVES; w++) {
							int wltemp;
							pFile->Read(&wltemp, sizeof(_pInstrument[0]->waveLength));
							if (wltemp > 0) {
								if ( w == 0 ) {
									short tmpFineTune;
									_pInstrument[i]->waveLength=wltemp;
									pFile->Read(&_pInstrument[i]->waveName, 32);
									pFile->Read(&_pInstrument[i]->waveVolume, sizeof(_pInstrument[0]->waveVolume));
									pFile->Read(&tmpFineTune, sizeof(short));
									_pInstrument[i]->waveFinetune=(int)tmpFineTune;
									pFile->Read(&_pInstrument[i]->waveLoopStart, sizeof(_pInstrument[0]->waveLoopStart));
									pFile->Read(&_pInstrument[i]->waveLoopEnd, sizeof(_pInstrument[0]->waveLoopEnd));
									pFile->Read(&_pInstrument[i]->waveLoopType, sizeof(_pInstrument[0]->waveLoopType));
									pFile->Read(&_pInstrument[i]->waveStereo, sizeof(_pInstrument[0]->waveStereo));
									_pInstrument[i]->waveDataL = new signed short[_pInstrument[i]->waveLength];
									pFile->Read(_pInstrument[i]->waveDataL, _pInstrument[i]->waveLength*sizeof(short));
									if (_pInstrument[i]->waveStereo) {
										_pInstrument[i]->waveDataR = new signed short[_pInstrument[i]->waveLength];
										pFile->Read(_pInstrument[i]->waveDataR, _pInstrument[i]->waveLength*sizeof(short));
									}
								}
								else {
									bool stereo;
									char *junk =new char[42+sizeof(bool)];
									pFile->Read(junk,sizeof(junk));
									delete junk;
									pFile->Read(&stereo,sizeof(bool));
									short *junk2 = new signed short[wltemp];
									pFile->Read(junk2, sizeof(junk2));
									if ( stereo )
										pFile->Read(junk2, sizeof(junk2));
									delete junk2;
								}
							}
						}
					}
					
					// VST DLLs
					//

					VSTLoader vstL[OLD_MAX_PLUGINS]; 
					for (i=0; i<OLD_MAX_PLUGINS; i++)
					{
						pFile->Read(&vstL[i].valid,sizeof(bool));
						if( vstL[i].valid )
						{
							pFile->Read(vstL[i].dllName,sizeof(vstL[i].dllName));
							_strlwr(vstL[i].dllName);
							pFile->Read(&(vstL[i].numpars), sizeof(int));
							vstL[i].pars = new float[vstL[i].numpars];

							for (int c=0; c<vstL[i].numpars; c++)
							{
								pFile->Read(&(vstL[i].pars[c]), sizeof(float));
							}
						}
					}
					
					// Machines
					//
					pFile->Read(&_machineActive[0], sizeof(_machineActive));
					Machine* pMac[128];
					memset(pMac,0,sizeof(pMac));

					convert_internal_machines::Converter converter;

					for (i=0; i<128; i++)
					{
						Sampler* pSampler;
						XMSampler* pXMSampler;
						Plugin* pPlugin;
						vst::plugin * pVstPlugin(0);
						int x,y,type;
						if (_machineActive[i])
						{
							Progress.m_Progress.SetPos(8192+i*(4096/128));
							::Sleep(1);

							pFile->Read(&x, sizeof(x));
							pFile->Read(&y, sizeof(y));

							pFile->Read(&type, sizeof(type));

							if(converter.plugin_names().exists(type))
								pMac[i] = &converter.redirect(i, type, *pFile);
							else switch (type)
							{
							case MACH_MASTER:
								pMac[i] = _pMachine[MASTER_INDEX];
								pMac[i]->Init();
								pMac[i]->Load(pFile);
								break;
							case MACH_SAMPLER:
								pMac[i] = pSampler = new Sampler(i);
								pMac[i]->Init();
								pMac[i]->Load(pFile);
								break;
							case MACH_PLUGIN:
								{
								pMac[i] = pPlugin = new Plugin(i);
								// Should the "Init()" function go here? -> No. Needs to load the dll first.
								if (!pMac[i]->Load(pFile))
								{
									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(*((Dummy*)pOldMachine));
									// dummy name goes here
									sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
									pMac[i]->_type = MACH_DUMMY;
									pOldMachine->_pSamplesL = NULL;
									pOldMachine->_pSamplesR = NULL;
									zapObject(pOldMachine);
								}
								break;
								}
							case MACH_VST:
							case MACH_VSTFX:
								{
									std::string temp;
									char sPath[_MAX_PATH];
									char sError[128];
									bool berror=false;
									vst::plugin* pTempMac = new vst::plugin(0);
									unsigned char program;
									int instance;
									// The trick: We need to load the information from the file in order to know the "instance" number
									// and be able to create a plugin from the corresponding dll. Later, we will set the loaded settings to
									// the newly created plugin.
									pTempMac->PreLoad(pFile,program,instance);
									assert(instance < OLD_MAX_PLUGINS);
									int shellIdx=0;
									if((!vstL[instance].valid) || (!CNewMachine::lookupDllName(vstL[instance].dllName,temp,shellIdx)))
									{
										berror=true;
										sprintf(sError,"VST plug-in missing, or erroneous data in song file \"%s\"",vstL[instance].dllName);
									}
									else
									{
										strcpy(sPath,temp.c_str());
										if (!CNewMachine::TestFilename(sPath,shellIdx))
										{
											berror=true;
											sprintf(sError,"This VST plug-in is Disabled \"%s\" - replacing with Dummy.",sPath);
										}
										else
										{
											try
											{
												pMac[i] = pVstPlugin = dynamic_cast<vst::plugin*>(Global::vsthost().LoadPlugin(sPath,shellIdx));

												if (pVstPlugin)
												{
													pVstPlugin->LoadFromMac(pTempMac);
													pVstPlugin->SetProgram(program);
													const int numpars = vstL[instance].numpars;
													for (int c(0) ; c < numpars; ++c)
													{
														try
														{
															pVstPlugin->SetParameter(c, vstL[instance].pars[c]);
														}
														catch(const std::exception &)
														{
															// o_O`
														}
													}
												}
											}
											catch(...)
											{
												berror=true;
												sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath);
											}
										}
									}
									if (berror)
									{
										MessageBox(NULL,sError, "Loading Error", MB_OK);

										Machine* pOldMachine = pTempMac;
										pMac[i] = new Dummy(*((Dummy*)pOldMachine));
										pOldMachine->_pSamplesL = NULL;
										pOldMachine->_pSamplesR = NULL;
										// dummy name goes here
										sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
										zapObject(pOldMachine);
										pMac[i]->_type = MACH_DUMMY;
										((Dummy*)pMac[i])->wasVST = true;
									}
								break;
								}
							case MACH_SCOPE:
							case MACH_DUMMY:
								pMac[i] = new Dummy(i);
								pMac[i]->Init();
								pMac[i]->Load(pFile);
								break;
							default:
								{
									char buf[MAX_PATH];
									sprintf(buf,"unkown machine type: %i",type);
									MessageBox(0, buf, "Loading old song", MB_ICONERROR);
								}
								pMac[i] = new Dummy(i);
								pMac[i]->Init();
								pMac[i]->Load(pFile);
							}

							switch (pMac[i]->_mode)
							{
							case MACHMODE_GENERATOR:
								if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
								if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
								break;
							case MACHMODE_FX:
								if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
								if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
								break;

							case MACHMODE_MASTER:
								if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
								if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;
								break;
							}

							pMac[i]->_x = x;
							pMac[i]->_y = y;
						}
					}

					// Since the old file format stored volumes on each output
					// rather than on each input, we must convert
					//
					float volMatrix[128][MAX_CONNECTIONS];
					for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
					{
						if (!_machineActive[i])
							zapObject(pMac[i]);
						else if (!pMac[i])
							_machineActive[i] = FALSE;
						else 
							for (int c=0; c<MAX_CONNECTIONS; c++)
								volMatrix[i][c] = pMac[i]->_inputConVol[c];
					}
					
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
										val*=0.000030517578125f; // BugFix
									else if ( val < 0.00004f) 
										val*=32768.0f; // BugFix
									pMac[i]->InitWireVolume(pOrigMachine->_type,c,val);
								}
							}
						}
					}
					
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_loop, sizeof(_pInstrument[0]->_loop));
					for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
						pFile->Read(&_pInstrument[i]->_lines, sizeof(_pInstrument[0]->_lines));

					// Patch 1: BusEffects (twf)
					if ( pFile->Read(&busEffect[0],sizeof(busEffect)) == false ) {
						int j=0;
						for ( i=0;i<128;i++ ) {
							if (_machineActive[i] && pMac[i]->_mode != MACHMODE_GENERATOR ) {
								busEffect[j]=i;	
								j++;
							}
						}
						for (j; j < 64; j++)
							busEffect[j] = 255;
					}
					
					// Patch 1.2: Fixes inconsistence when deleting a machine which couldn't be loaded
					// (.dll not found, or Load failed), which is, then, replaced by a DUMMY machine.
					int j=0;
					for ( i=0;i<64;i++ ) {
						// If there's a machine in the generators' bus that it is not a generator:
						if (busMachine[i] != 255 && _machineActive[busMachine[i]]) {
							if (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR ) 
								pMac[busMachine[i]]->_mode = MACHMODE_GENERATOR;
						}
					}
					for ( i=0;i<64;i++ ) {
						if ((busMachine[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)) 
							busMachine[i] = 255;
						if ((busEffect[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busEffect[i]]->_mode != MACHMODE_FX)) 
							busEffect[i] = 255;
					}

					bool chunkpresent=false;

					// Patch 2: VST's Chunk.
					pFile->Read(&chunkpresent,sizeof(chunkpresent));

					if ( fullopen ) for ( i=0;i<128;i++ ) {
						if (_machineActive[i]) {
							if ( pMac[i]->_type == MACH_DUMMY ) {
								if (((Dummy*)pMac[i])->wasVST && chunkpresent ) {
									// Since we don't know if the plugin saved it or not, 
									// we're stuck on letting the loading crash/behave incorrectly.
									// There should be a flag, like in the VST loading Section to be correct.
									MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
								}
							}
							else if (( pMac[i]->_type == MACH_VST ) || 
									( pMac[i]->_type == MACH_VSTFX))
							{
								bool chunkread = false;
								try {
									vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
									if(chunkpresent) chunkread = plugin.LoadChunk(pFile);
									//plugin.SetProgram(plugin._program);
								} catch(const std::exception &) {
									// o_O`
								}
							}
						}
					}
					// Clean "pars" array.
					for (i=0; i<OLD_MAX_PLUGINS; i++)
						if( vstL[i].valid )
							zapObject(vstL[i].pars);

					// move machines around to where they really should go
					// now we have to remap all the inputs and outputs again... ouch
					for (i = 0; i < 64; i++) {
						if ((busMachine[i] < MAX_MACHINES-1) && (busMachine[i] > 0)) {
							if (_machineActive[busMachine[i]]) {
								if (pMac[busMachine[i]]->_mode == MACHMODE_GENERATOR) {
									_pMachine[i] = pMac[busMachine[i]];
									_machineActive[busMachine[i]] = FALSE; // don't update this twice;
									for (int c=0; c<MAX_CONNECTIONS; c++) {
										if (_pMachine[i]->_inputCon[c]) {
											for (int x=0; x<64; x++) {
												if (_pMachine[i]->_inputMachines[c] == busMachine[x]) {
													_pMachine[i]->_inputMachines[c] = x;
													break;
												} else if (_pMachine[i]->_inputMachines[c] == busEffect[x]) {
													_pMachine[i]->_inputMachines[c] = x+MAX_BUSES;
													break;
												}
											}
										}
										if (_pMachine[i]->_connection[c]) {
											if (_pMachine[i]->_outputMachines[c] == 0) {
												_pMachine[i]->_outputMachines[c] = MASTER_INDEX;
											} else {
												for (int x=0; x<64; x++) {
													if (_pMachine[i]->_outputMachines[c] == busMachine[x]) {
														_pMachine[i]->_outputMachines[c] = x;
														break;
													} else if (_pMachine[i]->_outputMachines[c] == busEffect[x]) {
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
						if ((busEffect[i] < MAX_MACHINES-1) && (busEffect[i] > 0)) {
							if (_machineActive[busEffect[i]]) {
								if (pMac[busEffect[i]]->_mode == MACHMODE_FX) {
									_pMachine[i+MAX_BUSES] = pMac[busEffect[i]];
									_machineActive[busEffect[i]] = FALSE; // don't do this again
									for (int c=0; c<MAX_CONNECTIONS; c++) {
										if (_pMachine[i+MAX_BUSES]->_inputCon[c]) {
											for (int x=0; x<64; x++) {
												if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busMachine[x]) {
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
										if (_pMachine[i+MAX_BUSES]->_connection[c]) {
											if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == 0) {
												_pMachine[i+MAX_BUSES]->_outputMachines[c] = MASTER_INDEX;
											} else {
												for (int x=0; x<64; x++) {
													if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busMachine[x]) {
														_pMachine[i+MAX_BUSES]->_outputMachines[c] = x;
														break;
													} else if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busEffect[x]) {
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
					for (i = 0; i < MAX_MACHINES-1; i++) {
						if (_pMachine[i]) {
							_pMachine[i]->_macIndex = i;
							for (j = i+1; j < MAX_MACHINES-1; j++) {
								if (_pMachine[i] == _pMachine[j]) {
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
					// test all connections
					for (int c=0; c<MAX_CONNECTIONS; c++) {
						if (_pMachine[MASTER_INDEX]->_inputCon[c]) {
							for (int x=0; x<64; x++) {
								if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busMachine[x]) {
									_pMachine[MASTER_INDEX]->_inputMachines[c] = x;
									break;
								} else if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busEffect[x]) {
									_pMachine[MASTER_INDEX]->_inputMachines[c] = x+MAX_BUSES;
									break;
								}
							}
						}
					}
					// test all connections for invalid machines. disconnect invalid machines.
					for (i = 0; i < MAX_MACHINES; i++) {
						if (_pMachine[i]) {
							_pMachine[i]->_numInputs = 0;
							_pMachine[i]->_numOutputs = 0;
							for (int c = 0; c < MAX_CONNECTIONS; c++) {
								if (_pMachine[i]->_connection[c]) {
									if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES) {
										_pMachine[i]->_connection[c]=FALSE;
										_pMachine[i]->_outputMachines[c]=-1;
									} else if (!_pMachine[_pMachine[i]->_outputMachines[c]]) {
										_pMachine[i]->_connection[c]=FALSE;
										_pMachine[i]->_outputMachines[c]=-1;
									} else {
										_pMachine[i]->_numOutputs++;
									}
								} else {
									_pMachine[i]->_outputMachines[c]=-1;
								}
								if (_pMachine[i]->_inputCon[c]) {
									if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES-1) {
										_pMachine[i]->_inputCon[c]=FALSE;
										_pMachine[i]->_inputMachines[c]=-1;
									} else if (!_pMachine[_pMachine[i]->_inputMachines[c]]) {
										_pMachine[i]->_inputCon[c]=FALSE;
										_pMachine[i]->_inputMachines[c]=-1;
									} else {
										_pMachine[i]->_numInputs++;
									}
								} else {
									_pMachine[i]->_inputMachines[c]=-1;
								}
							}
						}
					}
					if(fullopen) converter.retweak(*this);
					_machineLock = false;
					seqBus=0;

					if (!pFile->Close()) {
						std::ostringstream s;
						s << "Error reading from file '" << pFile->szName << "'" << std::endl;
						MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
						return false;
					}
					return true;
				}
				return false;
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			void root::write(output & out)
			{
				int chunkcount = 3; // 3 chunks plus:
				for (int i = 0; i < MAX_PATTERNS; i++) if (IsPatternUsed(i)) ++chunkcount;
				for (int i = 0; i < MAX_MACHINES; i++) if (_pMachine[i]) ++chunkcount;
				for (int i = 0; i < MAX_INSTRUMENTS; i++) if (!_pInstrument[i]->Empty()) ++chunkcount;

				/*
				===================
				FILE HEADER
				===================
				id = "PSY3SONG"; // PSY2 was 1.66
				*/

				pFile->Write("PSY3SONG", 8);

				UINT version = CURRENT_FILE_VERSION;
				UINT size = sizeof(chunkcount);
				UINT index = 0;
				int temp;

				pFile->Write(&version,sizeof version);
				pFile->Write(&size,sizeof size);
				pFile->Write(&chunkcount,sizeof chunkcount);

				// the rest of the modules can be arranged in any order

				/*
				===================
				SONG INFO TEXT
				===================
				id = "INFO"; 
				*/

				pFile->Write("INFO",4);
				version = CURRENT_FILE_VERSION_INFO;
				size = name.length() + author.length() + comments.length() + 3*sizeof(int) + 3; // +3 for \0

				pFile->Write(&version,sizeof(version));
				pFile->Write(&size,sizeof(size));

				pFile->Write(name.c_str(),name.length()+1);
				pFile->Write(name.c_str(),name.length()+1);
				pFile->Write(comments.c_str(),comments.length()+1);

				/*
				===================
				SONG INFO
				===================
				id = "SNGI"; 
				*/

				pFile->Write("SNGI",4);
				version = CURRENT_FILE_VERSION_SNGI;
				size = (11*sizeof(temp))+(SONGTRACKS*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])));
				pFile->Write(&version,sizeof(version));
				pFile->Write(&size,sizeof(size));

				temp = SONGTRACKS;
				pFile->Write(&temp,sizeof(temp));
				temp = m_BeatsPerMin;
				pFile->Write(&temp,sizeof(temp));
				temp = m_LinesPerBeat;
				pFile->Write(&temp,sizeof(temp));
				temp = currentOctave;
				pFile->Write(&temp,sizeof(temp));
				temp = machineSoloed;
				pFile->Write(&temp,sizeof(temp));
				temp = _trackSoloed;
				pFile->Write(&temp,sizeof(temp));

				temp = seqBus;
				pFile->Write(&temp,sizeof(temp));

				temp = midiSelected;
				pFile->Write(&temp,sizeof(temp));
				temp = auxcolSelected;
				pFile->Write(&temp,sizeof(temp));
				temp = instSelected;
				pFile->Write(&temp,sizeof(temp));

				temp = 1; // sequence width
				pFile->Write(&temp,sizeof(temp));

				for (int i = 0; i < SONGTRACKS; i++) {
					pFile->Write(&_trackMuted[i],sizeof(_trackMuted[i]));
					pFile->Write(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
				}

				/*
				===================
				SEQUENCE DATA
				===================
				id = "SEQD"; 
				*/
				index = 0; // index
				for (index=0;index<MAX_SEQUENCES;index++) {
					char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequence.
					pFile->Write("SEQD",4);
					version = CURRENT_FILE_VERSION_SEQD;
					size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
					pFile->Write(&version,sizeof(version));
					pFile->Write(&size,sizeof(size));
					pFile->Write(&index,sizeof(index)); // Sequence Track number
					temp = playLength;
					pFile->Write(&temp,sizeof(temp)); // Sequence length
					pFile->Write(pSequenceName,strlen(pSequenceName)+1); // Sequence Name
					for (int i = 0; i < playLength; i++) {
						temp = playOrder[i];
						pFile->Write(&temp,sizeof(temp));	// Sequence data.
					}
				}

				/*
				===================
				PATTERN DATA
				===================
				id = "PATD"; 
				*/

				for (int i = 0; i < MAX_PATTERNS; i++) {
					// check every pattern for validity
					if (IsPatternUsed(i)) {
						// ok save it
						byte* pSource=new byte[SONGTRACKS*patternLines[i]*EVENT_SIZE];
						byte* pCopy = pSource;
						for (int y = 0; y < patternLines[i]; y++) {
							unsigned char* pData = ppPatternData[i]+(y*MULTIPLY);
							memcpy(pCopy,pData,EVENT_SIZE*SONGTRACKS);
							pCopy+=EVENT_SIZE*SONGTRACKS;
						}
						int sizez77 = BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[i]*EVENT_SIZE);
						zapArray(pSource);

						pFile->Write("PATD",4);
						version = CURRENT_FILE_VERSION_PATD;

						pFile->Write(&version,sizeof(version));
						size = sizez77+(4*sizeof(temp))+strlen(patternName[i])+1;
						pFile->Write(&size,sizeof(size));

						index = i; // index
						pFile->Write(&index,sizeof(index));
						temp = patternLines[i];
						pFile->Write(&temp,sizeof(temp));
						temp = SONGTRACKS; // eventually this may be variable per pattern
						pFile->Write(&temp,sizeof(temp));

						pFile->Write(&patternName[i],strlen(patternName[i])+1);

						pFile->Write(&sizez77,sizeof(sizez77));
						pFile->Write(pCopy,sizez77);
						zapArray(pCopy);
					}
				}

				/*
				===================
				MACHINE DATA
				===================
				id = "MACD"; 
				*/
				// machine and instruments handle their save and load in their respective classes

				for (int i = 0; i < MAX_MACHINES; i++) {
					if (_pMachine[i]) {
						pFile->Write("MACD",4);
						version = CURRENT_FILE_VERSION_MACD;
						pFile->Write(&version,sizeof(version));
						long pos = pFile->GetPos();
						size = 0;
						pFile->Write(&size,sizeof(size));

						index = i; // index
						pFile->Write(&index,sizeof(index));

						_pMachine[i]->SaveFileChunk(pFile);

						long pos2 = pFile->GetPos(); 
						size = pos2-pos-sizeof(size);
						pFile->Seek(pos);
						pFile->Write(&size,sizeof(size));
						pFile->Seek(pos2);
					}
				}

				/*
				===================
				Instrument DATA
				===================
				id = "INSD"; 
				*/
				for (int i = 0; i < MAX_INSTRUMENTS; i++) {
					if (!_pInstrument[i]->Empty()) {
						pFile->Write("INSD",4);
						version = CURRENT_FILE_VERSION_INSD;
						pFile->Write(&version,sizeof(version));
						long pos = pFile->GetPos();
						size = 0;
						pFile->Write(&size,sizeof(size));

						index = i; // index
						pFile->Write(&index,sizeof(index));

						_pInstrument[i]->SaveFileChunk(pFile);

						long pos2 = pFile->GetPos(); 
						size = pos2-pos-sizeof(size);
						pFile->Seek(pos);
						pFile->Write(&size,sizeof(size));
						pFile->Seek(pos2);
					}
				}

				if (!pFile->Close()) {
					std::ostringstream s;
					s << "Error writing to file '" << pFile->szName << "'" << std::endl;
					MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
				}
				return true;
			}
		}
	}
}
