#if !defined(_WINAMP_PLUGIN_)
		
		CProgressDialog Progress;
		Progress.Create();
		Progress.SetWindowText("Loading...");
		Progress.ShowWindow(SW_SHOW);
#endif

		UINT version = 0;
		UINT size = 0;
		UINT index = 0;
		int temp;
		int solo;
		int chunkcount;
		char Header[9];

		Header[4]=0;

		long filesize = pFile->FileSize();

		pFile->Read(&version,sizeof(version));
		pFile->Read(&size,sizeof(size));
		pFile->Read(&chunkcount,sizeof(chunkcount));

		if (version > CURRENT_FILE_VERSION)
		{
			// there is an error, this file is newer than this build of psycle
			MessageBox(NULL,"File is from a newer version of psycle! You should get a new one immediately!",NULL,NULL);
		}
		pFile->Skip(size-sizeof(chunkcount));
		/*
		else
		{
			// there is currently no data in this segment
		}
		*/
		DestroyAllMachines();
		_machineLock = true;
		DeleteInstruments();
		DeleteAllPatterns();

		while(pFile->Read(&Header, 4))
		{
#if !defined(_WINAMP_PLUGIN_)
			Progress.m_Progress.SetPos(f2i((pFile->GetPos()*16384.0f)/filesize));
			::Sleep(1);
#endif
			// we should use the size to update the index, but for now we will skip it
			if (strcmp(Header,"INFO")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_INFO)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Info Seqment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->ReadString(Name,sizeof(Name));
					pFile->ReadString(Author,sizeof(Author));
					pFile->ReadString(Comment,sizeof(Comment));
				}
			}
			else if (strcmp(Header,"SNGI")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_SNGI)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Song Segment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					// why all these temps?  to make sure if someone changes the defs of
					// any of these members, the rest of the file reads ok.  assume 
					// everything is an int, when we write we do the same thing.

					pFile->Read(&temp,sizeof(temp));  // # of tracks for whole song
					pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS = temp;
					pFile->Read(&temp,sizeof(temp));  // bpm
					BeatsPerMin = temp;
					pFile->Read(&temp,sizeof(temp));  // tpb
					_ticksPerBeat = temp;
					pFile->Read(&temp,sizeof(temp));  // current octave
					currentOctave = temp;
					pFile->Read(&temp,sizeof(temp));  // machineSoloed
					solo = temp;	// we need to buffer this because destroy machine will clear it

					pFile->Read(&temp,sizeof(temp));  // trackSoloed
					_trackSoloed=temp;

					pFile->Read(&temp,sizeof(temp));  
					seqBus=temp;

					pFile->Read(&temp,sizeof(temp));  
					midiSelected=temp;
					pFile->Read(&temp,sizeof(temp));  
					auxcolSelected=temp;
					pFile->Read(&temp,sizeof(temp));  
					instSelected=temp;

					pFile->Read(&temp,sizeof(temp));  // sequence width, for multipattern

					_trackArmedCount = 0;
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						pFile->Read(&_trackMuted[i],sizeof(_trackMuted[i]));
						pFile->Read(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
						if (_trackArmed[i])
						{
							_trackArmedCount++;
						}
					}

					Global::pPlayer->bpm = BeatsPerMin;
					Global::pPlayer->tpb = _ticksPerBeat;
					// calculate samples per tick
		
#if defined(_WINAMP_PLUGIN_)
					SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
#else
					SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
#endif
				}
			}
			else if (strcmp(Header,"SEQD")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_SEQD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index)); // index, for multipattern - for now always 0
					if (index < MAX_SEQUENCES)
					{
						char pTemp[256];
						pFile->Read(&temp,sizeof(temp)); // play length for this sequence
						pSequencer->startLine = temp;
						pFile->Read(&temp,sizeof(temp)); // play length for this sequence
						pSequencer->endLine = temp;
						pFile->Read(&temp,sizeof(temp)); // play length for this sequence
						unsigned int seqCount = temp;
						pFile->Read(&temp,sizeof(temp)); // play length for this sequence
						unsigned int trackCount = temp;
						pFile->ReadString(pTemp,sizeof(pTemp)); // name, for multipattern, for now unused

						//for (int i = 0; i < playLength; i++)
						//{
						//	pFile->Read(&temp,sizeof(temp));
						//	playOrder[i] = temp;
						//}
						pSequencer->machineList.clear();
						for ( unsigned int track = 0; track < trackCount; track++ )
						{
							pFile->Read(&temp,sizeof(temp));
							int trackID = temp;
							pSequencer->CreateMachineSequenceTrack(trackID);
							// pattern colour 
							pFile->Read(&temp,sizeof(temp));
							pSequencer->pSequenceData[trackID]->patColor = temp;
						}
						for ( unsigned int seq = 0; seq < seqCount; seq++ )
						{
							pFile->Read(&temp,sizeof(temp));
							int row = temp;
							pFile->Read(&temp,sizeof(temp));
							int col = temp;
							pFile->Read(&temp,sizeof(temp));
							int patID = temp;
							pSequencer->pSequenceData[col]->pSequences[row]->patternID = patID;
						}
					}
					else
					{
//						MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"PATD")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_PATD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&temp,sizeof(temp)); // num tracks
					unsigned int trackCount = temp;

					for ( unsigned int track = 0; track < trackCount; track++ )
					{
						pFile->Read(&temp,sizeof(temp)); // num patterns in track
						int patCount = temp;
						pFile->Read(&temp,sizeof(temp)); // trackID in machine list 
						int trackID = temp;	
						pSequencer->pSequenceData[trackID]->patCount = patCount;

						for (int i = 0; i < patCount; i++)
						{
							pFile->Read(&index,sizeof(index)); // index
							if (index < MAX_PATTERNS)
							{
								pFile->Read(&temp,sizeof(temp)); // num lines
								//RemovePattern(index); // clear it out if it already exists
								pSequencer->pSequenceData[trackID]->patternLines[index] = temp;
								pFile->Read(&temp,sizeof(temp)); // num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
								pSequencer->pSequenceData[trackID]->SONGTRACKS = temp;

								pFile->ReadString(pSequencer->pSequenceData[trackID]->patternName[index], 
									sizeof(pSequencer->pSequenceData[trackID]->patternName[index]));

								pFile->Read(&size,sizeof(size));
								byte* pSource = new byte[size];
								pFile->Read(pSource,size);
								byte* pDest;

								BEERZ77Decomp2(pSource, &pDest);
								delete pSource;
								pSource = pDest;

								for (int y = 0; y < pSequencer->pSequenceData[trackID]->patternLines[index]; y++)
								{
									unsigned char* pData = pSequencer->pSequenceData[trackID]->_ppattern(index)+(y*MULTIPLY);
									memcpy(pData,pSource,pSequencer->pSequenceData[trackID]->SONGTRACKS*EVENT_SIZE);
									pSource += pSequencer->pSequenceData[trackID]->SONGTRACKS*EVENT_SIZE;
								}
								delete pDest;
							}
							else
							{
		//						MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
								pFile->Skip(size-sizeof(index));
							}
						}
					}
				}
			}
			else if (strcmp(Header,"MACD")==0)
			{
				int curpos=0;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				chunkcount--;
				if ( !fullopen )
				{
					curpos=pFile->GetPos();
				}
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Machine section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index));
					if (index < MAX_MACHINES)
					{
						// we had better load it
						//DestroyMachine(index);
						_pMachine[index] = Machine::LoadFileChunk(pFile,index,version,fullopen);
						if ( !fullopen ) pFile->Seek(curpos+size); // skips specific chunk.
					}
					else
					{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"INSD")==0)
			{
				int curpos=0;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				chunkcount--;
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index));
					if (index < MAX_INSTRUMENTS)
					{
						_pInstrument[index]->LoadFileChunk(pFile,version,fullopen);
					}
					else
					{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else 
			{
				// we are not at a valid header for some weird reason.  
				// probably there is some extra data.
				// shift back 3 bytes and try again
				pFile->Skip(-3);
			}
		}
		// now that we have loaded all the modules, time to prepare them.
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(16384);
		::Sleep(1);
#endif
		// test all connections for invalid machines. disconnect invalid machines.
		for (int i = 0; i < MAX_MACHINES; i++)
		{
			if (_pMachine[i])
			{
				_pMachine[i]->_numInputs = 0;
				_pMachine[i]->_numOutputs = 0;

				for (int c = 0; c < MAX_CONNECTIONS; c++)
				{
					if (_pMachine[i]->_connection[c])
					{
						if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_outputMachines[c]])
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else 
						{
							_pMachine[i]->_numOutputs++;
						}
					}
					else
					{
						_pMachine[i]->_outputMachines[c]=255;
					}

					if (_pMachine[i]->_inputCon[c])
					{
						if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else
						{
							_pMachine[i]->_numInputs++;
						}
					}
					else
					{
						_pMachine[i]->_inputMachines[c]=255;
					}
				}
			}
		}

		// translate any data that is required
#ifndef _WINAMP_PLUGIN_
		((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen();
		machineSoloed = solo;
#endif
		// allow stuff to work again
		_machineLock = false;

#if !defined(_WINAMP_PLUGIN_)
		Progress.OnCancel();
#endif
		if ((!pFile->Close()) || (chunkcount))
		{
			char error[MAX_PATH];
			sprintf(error,"Error reading from \"%s\"!!!",pFile->szName);
			MessageBox(NULL,error,"File Error!!!",0);
			return false;
		}

		return true;
