#include <project.private.hpp>
#include "internal_machines.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "Player.hpp"

namespace psycle
{
	namespace host
	{

		char* Dummy::_psName = "DummyPlug";
		char* DuplicatorMac::_psName = "Dupe it!";
		char* Mixer::_psName = "Mixer";

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy



		Dummy::Dummy(int index)
		{
			_macIndex = index;
			_type = MACH_DUMMY;
			_mode = MACHMODE_FX;
			sprintf(_editName, _psName);
			wasVST = false;
		}
		void Dummy::Work(int numSamples)
		{
			Machine::Work(numSamples);
			cpu::cycles_type cost = cpu::cycles();
			Machine::SetVolumeCounter(numSamples);
			if ( Global::pConfig->autoStopMachines )
			{
				//Machine::SetVolumeCounterAccurate(numSamples);
				if (_volumeCounter < 8.0f)	{
					_volumeCounter = 0.0f;
					_volumeDisplay = 0;
					Standby(true);
				}
			}
			//else Machine::SetVolumeCounter(numSamples);
			_cpuCost += cpu::cycles() - cost;
			_worked = true;
		}

		// Since Dummy is used by the loader to load broken/missing plugins, 
		// its "LoadSpecificChunk" skips the data of the chunk so that the
		// song loader can continue the sequence.
		bool Dummy::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Skip(size);
			return true;
		};


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// NoteDuplicator
		DuplicatorMac::DuplicatorMac(int index)
		{
			_macIndex = index;
			_numPars = NUMMACHINES*2;
			_nCols = 2;
			_type = MACH_DUPLICATOR;
			_mode = MACHMODE_GENERATOR;
			bisTicking = false;
			sprintf(_editName, _psName);
			for (int i=0;i<NUMMACHINES;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
				for (int j=0;j<MAX_TRACKS;j++)
				{
					allocatedchans[j][i] = -1;
				}
			}
			for (int i=0;i<MAX_MACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					availablechans[i][j] = true;
				}
			}
		}
		void DuplicatorMac::Init()
		{
			Machine::Init();
			for (int i=0;i<NUMMACHINES;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
				for (int j=0;j<MAX_TRACKS;j++)
				{
					allocatedchans[j][i] = -1;
				}
			}
			for (int i=0;i<MAX_MACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					availablechans[i][j] = true;
				}
			}
		}
		void DuplicatorMac::Stop()
		{
			for (int i=0;i<NUMMACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					allocatedchans[j][i] = -1;
				}
			}
			for (int i=0;i<MAX_MACHINES;i++)
			{
				for (int j=0;j<MAX_TRACKS;j++)
				{
					availablechans[i][j] = true;
				}
			}
		}
		void DuplicatorMac::Tick()
		{

		}

		void DuplicatorMac::Tick( int channel,PatternEntry* pData)
		{
			if ( !_mute && !bisTicking) // Prevent possible loops of dupe machines.
			{
				bisTicking=true;
				for (int i=0;i<NUMMACHINES;i++)
				{
					if (macOutput[i] != -1 && Global::_pSong->_pMachine[macOutput[i]] != NULL )
					{
						AllocateVoice(channel,i);
						PatternEntry pTemp = *pData;
						if ( pTemp._note < 120 )
						{
							int note = pTemp._note+noteOffset[i];
							if ( note>=120) note=119;
							else if (note<0 ) note=0;
							pTemp._note=(compiler::uint8)note;
						}

						// the first part can happen if the parameter is the machine itself.
						if (Global::_pSong->_pMachine[macOutput[i]] != this) 
						{
							Global::_pSong->_pMachine[macOutput[i]]->Tick(allocatedchans[channel][i],&pTemp);
							if (pTemp._note >= 120 )
							{
								DeallocateVoice(channel,i);
							}
						}
					}
				}
			}
			bisTicking=false;
		}
		void DuplicatorMac::AllocateVoice(int channel,int machine)
		{
			// If this channel already has allocated channels, use them.
			if ( allocatedchans[channel][machine] != -1 )
				return;
			// If not, search an available channel
			int j=channel;
			while (j<MAX_TRACKS && !availablechans[macOutput[machine]][j]) j++;
			if ( j == MAX_TRACKS)
			{
				j=0;
				while (j<MAX_TRACKS && !availablechans[macOutput[machine]][j]) j++;
				if (j == MAX_TRACKS)
				{
					j= MAX_TRACKS * static_cast<unsigned int>(rand())/((RAND_MAX+1)*2);
				}
			}
			allocatedchans[channel][machine]=j;
			availablechans[macOutput[machine]][j]=false;
		}
		void DuplicatorMac::DeallocateVoice(int channel, int machine)
		{
			if ( allocatedchans[channel][machine] == -1 )
				return;
			availablechans[macOutput[machine]][allocatedchans[channel][machine]]= true;
			allocatedchans[channel][machine]=-1;
		}
		void DuplicatorMac::GetParamName(int numparam,char *name)
		{
			if (numparam >=0 && numparam<NUMMACHINES)
			{
				sprintf(name,"Output Machine %d",numparam);
			} else if (numparam >=NUMMACHINES && numparam<NUMMACHINES*2) {
				sprintf(name,"Note Offset %d",numparam-NUMMACHINES);
			}
			else name[0] = '\0';
		}
		void DuplicatorMac::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if ( numparam < NUMMACHINES) { minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if ( numparam < NUMMACHINES*2) { minval = -48; maxval = 48; }
		}
		int DuplicatorMac::GetParamValue(int numparam)
		{
			if (numparam >=0 && numparam<NUMMACHINES)
			{
				return macOutput[numparam];
			} else if (numparam >=NUMMACHINES && numparam <NUMMACHINES*2) {
				return noteOffset[numparam-NUMMACHINES];
			}
			else return 0;
		}
		void DuplicatorMac::GetParamValue(int numparam, char *parVal)
		{
			if (numparam >=0 && numparam <NUMMACHINES)
			{
				if ((macOutput[numparam] != -1 ) &&( Global::_pSong->_pMachine[macOutput[numparam]] != NULL))
				{
					sprintf(parVal,"%X -%s",macOutput[numparam],Global::_pSong->_pMachine[macOutput[numparam]]->_editName);
				}else if (macOutput[numparam] != -1) sprintf(parVal,"%X (none)",macOutput[numparam]);
				else sprintf(parVal,"(disabled)");

			} else if (numparam >= NUMMACHINES && numparam <NUMMACHINES*2) {
				char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
				sprintf(parVal,"%s%d",notes[(noteOffset[numparam-NUMMACHINES]+60)%12],(noteOffset[numparam-NUMMACHINES]+60)/12);
			}
			else parVal[0] = '\0';
		}
		bool DuplicatorMac::SetParameter(int numparam, int value)
		{
			if (numparam >=0 && numparam<NUMMACHINES)
			{
				macOutput[numparam]=value;
				return true;
			} else if (numparam >=NUMMACHINES && numparam<NUMMACHINES*2) {
				noteOffset[numparam-NUMMACHINES]=value;
				return true;
			}
			else return false;
		}

		void DuplicatorMac::Work(int numSamples)
		{
			_worked = true;
			Standby(true);
		}
		bool DuplicatorMac::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Read(&macOutput,sizeof macOutput);
			pFile->Read(&noteOffset,sizeof noteOffset);
			return true;
		}

		void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile)
		{
			UINT size = sizeof macOutput+ sizeof noteOffset;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&macOutput,sizeof macOutput);
			pFile->Write(&noteOffset,sizeof noteOffset);
		}





		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mixer

		Mixer::Mixer(int id)
		{
			_macIndex = id;
			_type = MACH_MIXER;
			_mode = MACHMODE_FX;
			sprintf(_editName, _psName);

		}

		Mixer::~Mixer() throw()
		{
		}

		void Mixer::Init()
		{
			Machine::Init();
			for (int j=0;j<MAX_CONNECTIONS;j++)
			{
				_sendGrid[j][mix]=1.0f;
				for (int i=send0;i<sendmax;i++)
				{
					_sendGrid[j][i]=0.0f;
				}
				_send[j]=0;
				_sendVol[j]=1.0f;
				_sendVolMulti[j]=1.0f;
				_sendValid[j]=false;
			}
		}

		void Mixer::Tick( int channel,PatternEntry* pData)
		{
			if(pData->_note == notecommands::tweak)
			{
				int nv = (pData->_cmd<<8)+pData->_parameter;
				SetParameter(pData->_inst,nv);
				Global::player().Tweaker = true;
			}
			else if(pData->_note == notecommands::tweakslide)
			{
				//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
			}
		}

		void Mixer::Work(int numSamples)
		{
			// Step One, do the usual work, except mixing all the inputs to a single stream.
			Machine::WorkNoMix(numSamples);
			// Step Two, prepare input signals for the Send Fx, and make them work
			FxSend(numSamples);
			// Step Three, Mix the returns of the Send Fx's with the leveled input signal
			if(!_mute && !Standby() )
			{
				cpu::cycles_type cost = cpu::cycles();
				Mix(numSamples);
				Machine::SetVolumeCounter(numSamples);
				if ( Global::configuration().autoStopMachines )
				{
					if (_volumeCounter < 8.0f)
					{
						_volumeCounter = 0.0f;
						_volumeDisplay = 0;
						Standby(true);
					}
					else Standby(false);
				}
				_cpuCost += cpu::cycles() - cost;
			}

			cpu::cycles_type cost = cpu::cycles();
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			_cpuCost += cpu::cycles() - cost;

			_worked = true;
		}

		void Mixer::FxSend(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::song()._pMachine[_send[i]];
					if (pSendMachine)
					{
						if (!pSendMachine->_worked && !pSendMachine->_waitingForSound)
						{ 
							// Mix all the inputs and route them to the send fx.
							{
								cpu::cycles_type cost = cpu::cycles();
								for (int j=0; j<MAX_CONNECTIONS; j++)
								{
									if (_inputCon[j])
									{
										Machine* pInMachine = Global::song()._pMachine[_inputMachines[j]];
										if (pInMachine)
										{
											if(!pInMachine->_mute && !pInMachine->Standby() && _sendGrid[j][send0+i]!= 0.0f)
											{
												dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[j]*_sendGrid[j][send0+i]);
												dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[j]*_sendGrid[j][send0+i]);
											}
										}
									}
								}
								_cpuCost += cpu::cycles() - cost ;
							}

							// tell the FX to work, now that the input is ready.
							{
#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
								universalis::processor::exceptions::fpu::mask fpu_exception_mask(pSendMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
#endif
								pSendMachine->Work(numSamples);
							}

							{
								cpu::cycles_type cost = cpu::cycles();
								pSendMachine->_waitingForSound = false;
								dsp::Clear(_pSamplesL, numSamples);
								dsp::Clear(_pSamplesR, numSamples);
								_cpuCost += cpu::cycles() - cost;
							}

						}
						if(!pSendMachine->Standby())Standby(false);
					}
				}
			}
		}

		void Mixer::Mix(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i])
				{
					Machine* pSendMachine = Global::song()._pMachine[_send[i]];
					if (pSendMachine)
					{
						if(!pSendMachine->_mute && !pSendMachine->Standby())
						{
							dsp::Add(pSendMachine->_pSamplesL, _pSamplesL, numSamples, pSendMachine->_lVol*_sendVol[i]);
							dsp::Add(pSendMachine->_pSamplesR, _pSamplesR, numSamples, pSendMachine->_rVol*_sendVol[i]);
						}
					}
				}
			}
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_inputCon[i])
				{
					Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
					if (pInMachine)
					{
						if(!pInMachine->_mute && !pInMachine->Standby() && _sendGrid[i][mix] != 0.0f)
						{
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]*_sendGrid[i][mix]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]*_sendGrid[i][mix]);
						}
					}
				}
			}
		}

		/*		std::string Mixer::GetAudioInputName(int port)
		{
		std::string rettxt;
		if (port < return1 )
		{	
		rettxt = "Input ";
		rettxt += ('0'+port-chan1);
		return rettxt;
		}
		else if ( port <= return12)
		{
		rettxt = "Return ";
		rettxt += ('0'+port-return1);
		return rettxt;
		}
		rettxt = "-";
		return rettxt;
		}
		*/
		std::string Mixer::GetAudioInputName(int port)
		{
			std::string rettxt;
			if (port < return1 )
			{	
				rettxt = "Input ";
				rettxt += ('0'+port-chan1);
				return rettxt;
			}
			else if ( port <= return12)
			{
				rettxt = "Return ";
				rettxt += ('0'+port-return1);
				return rettxt;
			}
			rettxt = "-";
			return rettxt;
		}

		int Mixer::GetNumCols()
		{
			int cols=0;
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_inputCon[i]) cols++;
			}
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (_sendValid[i]) cols++;
			}
			return cols==0?1:cols;
		}

		void Mixer::GetParamName(int numparam,char *name)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) { name[0] = '\0'; return; }
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					if ( send == mix )sprintf(name,"Channel %d - Mix",channel+1);
					else sprintf(name,"Channel %d - Send %d",channel+1,send);
				}
				else name[0] = '\0';
			}
			else if  ( send == 0){ name[0] = '\0'; return; }
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) sprintf(name,"Input level Ch %d",send+send0);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && _sendValid[send]) sprintf(name,"Input level Fx %d",send+send0);
				else name[0] = '\0';
			}
		}

		int Mixer::GetParamValue(int numparam)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) return 0;
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					return (int)(_sendGrid[channel][send]*100.0f);
				}
				else return 0;
			}
			else if  ( send == 0) return 0;
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) return (int)(_inputConVol[send]*_wireMultiplier[send]*100.0f);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && _sendValid[send]) return (int)(_sendVol[send]*_sendVolMulti[send]*100.0f);
				else return 0;
			}
		}

		void Mixer::GetParamValue(int numparam, char *parVal)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0) { parVal[0] = '\0'; return; }
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					sprintf(parVal,"%.0f%%",_sendGrid[channel][send]*100.0f);
				}
				else  parVal[0] = '\0';
			}
			else if  ( send == 0) { parVal[0] = '\0'; return; }
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) sprintf(parVal,"%.0f%%",_inputConVol[send]*_wireMultiplier[send]*100.0f);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && _sendValid[send]) sprintf(parVal,"%.0f%%",_sendVol[send]*_sendVolMulti[send]*100.0f);
				else parVal[0] = '\0';
			}
		}

		bool Mixer::SetParameter(int numparam, int value)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send(numparam % 16); // 0 is for channel mix, others are send.
			if ( channel == 0) return false;
			if ( value>100 ) value=100;
			if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || _sendValid[send-send0]))
				{
					_sendGrid[channel][send]=value/100.0f;
					return true;
				}
				else return false;
			}
			else if  ( send == 0) return false;
			else
			{
				send-=send0;
				if ( channel == 0x0E && send < MAX_CONNECTIONS)
				{
					SetWireVolume(send,value/100.0f);
					return true;
				}
				else if ( channel == 0x0F && send < MAX_CONNECTIONS) 
				{
					_sendVol[send]= value / (_sendVolMulti[send] * 100.0f);
					return true;
				}
				else return false;
			}
		}

		bool Mixer::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(&size,sizeof(size));
			pFile->Read(_sendGrid,sizeof(_sendGrid));
			pFile->Read(_send,sizeof(_send));
			pFile->Read(_sendVol,sizeof(_sendVol));
			pFile->Read(_sendVolMulti,sizeof(_sendVolMulti));
			pFile->Read(_sendValid,sizeof(_sendValid));
			return true;
		}

		void Mixer::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof _sendGrid + sizeof _send + sizeof _sendVol + sizeof _sendVolMulti + sizeof _sendValid);
			pFile->Write(&size,sizeof(size));
			pFile->Write(_sendGrid,sizeof(_sendGrid));
			pFile->Write(_send,sizeof(_send));
			pFile->Write(_sendVol,sizeof(_sendVol));
			pFile->Write(_sendVolMulti,sizeof(_sendVolMulti));
			pFile->Write(_sendValid,sizeof(_sendValid));
		}

		float Mixer::VuChan(int idx)
		{
			float vol;
			GetWireVolume(idx,vol);
			if ( _inputCon[idx] ) return (Global::song()._pMachine[_inputMachines[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}

		float Mixer::VuSend(int idx)
		{
			float vol = _sendVol[idx] * _sendVolMulti[idx];
			if ( _sendValid[idx] ) return (Global::song()._pMachine[_send[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}
	}
}