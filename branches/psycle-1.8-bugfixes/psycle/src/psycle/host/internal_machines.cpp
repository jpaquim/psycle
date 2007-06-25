#include <project.private.hpp>
#include "internal_machines.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "AudioDriver.hpp"

namespace psycle
{
	namespace host
	{

		char* Dummy::_psName = "DummyPlug";
		char* DuplicatorMac::_psName = "Dupe it!";
		char* AudioRecorder::_psName = "Recorder";
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
						if ( pTemp._note < notecommands::release )
						{
							int note = pTemp._note+noteOffset[i];
							if ( note>=notecommands::release) note=119;
							else if (note<0 ) note=0;
							pTemp._note=(compiler::uint8)note;
						}

						// the first part can happen if the parameter is the machine itself.
						if (Global::_pSong->_pMachine[macOutput[i]] != this) 
						{
							Global::_pSong->_pMachine[macOutput[i]]->Tick(allocatedchans[channel][i],&pTemp);
							if (pTemp._note >= notecommands::release )
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
		// AudioRecorder

		AudioRecorder::AudioRecorder(int index)
		{
			_macIndex = index;
			_numPars = 0;
			_type = MACH_RECORDER;
			_mode = MACHMODE_GENERATOR;
			sprintf(_editName, _psName);
			_initialized=false;
			_captureidx=0;
			pleftorig=_pSamplesL;
			prightorig=_pSamplesR;
			_gainvol=1.0f;
		}
		AudioRecorder::~AudioRecorder()
		{
			AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
			if (_initialized) mydriver.RemoveCapturePort(_captureidx);
			_pSamplesL=pleftorig;
			_pSamplesR=prightorig;
		}
		void AudioRecorder::Init(void)
		{
			Machine::Init();
			if (!_initialized)
			{
				AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
				mydriver.AddCapturePort(_captureidx);
				_initialized=true;
			}
		}
		void AudioRecorder::Work(int numSamples)
		{
			AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
			mydriver.GetReadBuffers(_captureidx,&_pSamplesL,&_pSamplesR,numSamples);
			// prevent crashing if the audio driver is not working.
			if ( _pSamplesL == 0 ) { _pSamplesL=pleftorig; _pSamplesR=prightorig; }
			dsp::Mul(_pSamplesL,numSamples,_gainvol);
			dsp::Mul(_pSamplesR,numSamples,_gainvol);
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
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			_cpuCost = 1;
			_worked = true;
		}
		bool AudioRecorder::LoadSpecificChunk(RiffFile * pFile, int version)
		{
			return false;
		}
		void AudioRecorder::SaveSpecificChunk(RiffFile * pFile)
		{
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mixer

		Mixer::Mixer(int id)
		{
			_macIndex = id;
			_numPars = 0xff;
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
			thegrid.Init(_inputCon);





			solocolumn_=-1;
			for (int j=0;j<MAX_CONNECTIONS;j++)
			{
				_sendGrid[j][mix]=1.0f;
				for (int i=send0;i<sendmax;i++)
				{
					_sendGrid[j][i]=0.0f;
				}
				_send[j]=0;
				_sendValid[j]=false;
				_returnVol[j]=1.0f;
				_returnVolMulti[j]=1.0f;
				_mutestate[j*2]=false;
				_mutestate[j*2+1]=false;
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
			if ( _mute || Standby() || Bypass())
			{
				Machine::Work(numSamples);
				return;
			}

			// Step One, do the usual work, except mixing all the inputs to a single stream.
			Machine::WorkNoMix(numSamples);
			// Step Two, prepare input signals for the Send Fx, and make them work
			FxSend(numSamples);
			// Step Three, Mix the returns of the Send Fx's with the leveled input signal
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
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			_cpuCost += cpu::cycles() - cost;

			_worked = true;
		}

		void Mixer::FxSend(int numSamples)
		{
			for (int i=0; i<MAX_CONNECTIONS; i++)
			{
				if (SendValid(i))
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
											if(!pInMachine->_mute && !pInMachine->Standby() && _sendGrid[j][send1+i]!= 0.0f)
											{
												dsp::Add(pInMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[j]*_sendGrid[j][send1+i]);
												dsp::Add(pInMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[j]*_sendGrid[j][send1+i]);
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
				if (SendValid(i))
				{
					Machine* pSendMachine = Global::song()._pMachine[_send[i]];
					if (pSendMachine)
					{
						if(!pSendMachine->_mute && !pSendMachine->Standby() && !_mutestate[i+MAX_CONNECTIONS])
						{
							dsp::Add(pSendMachine->_pSamplesL, _pSamplesL, numSamples, pSendMachine->_lVol*_returnVol[i]);
							dsp::Add(pSendMachine->_pSamplesR, _pSamplesR, numSamples, pSendMachine->_rVol*_returnVol[i]);
						}
					}
				}
			}
			if ( !_wetmix)
			{
				for (int i=0; i<MAX_CONNECTIONS; i++)
				{
					if (_inputCon[i])
					{
						Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
						if (pInMachine)
						{
							if(!pInMachine->_mute && !pInMachine->Standby() && _sendGrid[i][mix] != 0.0f && !_mutestate[i])
							{
								dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]*_sendGrid[i][mix]);
								dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]*_sendGrid[i][mix]);
							}
						}
					}
				}
			}
		}
		int Mixer::InsertFx(Machine* mac)
		{
			int sendbus=-1;
			int origbus=-1;
			bool error=false;

			// Get a free sendfx slot
			for(int c(MAX_CONNECTIONS - 1) ; c >= 0 ; --c)
			{
				if(!SendValid(c)) sendbus = c;
				// Checking that there's not a slot to the dest. machine already
				else if(GetSend(c) == mac->_macIndex) error = true;
			}
			if(sendbus == -1 || error) return -1;
			// Get a free output slot on the source machine
			for(int c(MAX_CONNECTIONS - 1) ; c >= 0 ; --c)
			{
				if(!mac->_connection[c]) origbus = c;
				// Checking that there's not an slot to the dest. machine already
				else if(mac->_outputMachines[c] == _macIndex) error = true;
			}
			if(origbus == -1 || error) return false;

			// Calibrating in/out properties
			mac->_outputMachines[origbus] = _macIndex;
			mac->_connection[origbus] = true;
			mac->_numOutputs++;
			_send[sendbus]=mac->_macIndex;
			_sendValid[sendbus]=true;
			InitWireVolume(mac->_type,MAX_CONNECTIONS+sendbus,1.0f);

			return sendbus;
		}

		float Mixer::GetWireVolume(int wireIndex)
		{
			if (wireIndex< MAX_CONNECTIONS)
				return Machine::GetWireVolume(wireIndex);
			else
				return _returnVol[wireIndex-MAX_CONNECTIONS] * _returnVolMulti[wireIndex-MAX_CONNECTIONS];
		}
		void Mixer::SetWireVolume(int wireIndex,float value)
		{
			if (wireIndex< MAX_CONNECTIONS)
				Machine::SetWireVolume(wireIndex,value);
			else
				_returnVol[wireIndex-MAX_CONNECTIONS] = value / _returnVolMulti[wireIndex-MAX_CONNECTIONS];
		}
		void Mixer::InitWireVolume(MachineType mType,int wireIndex,float value)
		{
			if (wireIndex< MAX_CONNECTIONS)
				Machine::InitWireVolume(mType,wireIndex,value);
			else
			{
				wireIndex-=MAX_CONNECTIONS;
				if (mType== MACH_VST || mType == MACH_VSTFX)
				{
					_returnVol[wireIndex]=value *32768.0f;
					_returnVolMulti[wireIndex]=0.000030517578125f;
				}
				else
				{
					_returnVol[wireIndex]=value;
					_returnVolMulti[wireIndex]=1.0f;
				}

				for(int c(MAX_CONNECTIONS - 1) ; c >= 0 ; --c)
				{
					_sendGrid[c][send1+wireIndex]*=_returnVolMulti[wireIndex];
				}
			}
		}
		int Mixer::FindInputWire(int macIndex)
		{
			int ret=Machine::FindInputWire(macIndex);
			if ( ret == -1)
			{
				for (int c=0; c<MAX_CONNECTIONS; c++)
				{
					if (SendValid(c))
					{
						if (GetSend(c) == macIndex)
						{
							ret = c+MAX_CONNECTIONS;
							break;
						}
					}
				}
			}
			return ret;
		}
		void Mixer::DeleteInputWireIndex(int wireIndex)
		{
			if ( wireIndex < MAX_CONNECTIONS)
				Machine::DeleteInputWireIndex(wireIndex);
			else
			{
				wireIndex-=MAX_CONNECTIONS;
				_send[wireIndex]=-1;
				_sendValid[wireIndex]=false;
			}
		}
		void Mixer::DeleteWires()
		{
			Machine::DeleteWires();
			Machine *iMac;
			for(int w=0; w<MAX_CONNECTIONS; w++)
			{
				// Checking send/return Wires
				if(_sendValid[w])
				{
					if((_send[w] >= 0) && (_send[w] < MAX_MACHINES))
					{
						iMac = Global::_pSong->_pMachine[_send[w]];
						if (iMac)
						{
							int wix = iMac->FindOutputWire(_macIndex);
							if (wix >=0)
							{
								iMac->DeleteOutputWireIndex(wix);
							}
						}
					}
				}
			}
		}
		std::string Mixer::GetAudioInputName(int port)
		{
			std::string rettxt;
			if (port < return1 )
			{	
				rettxt = "Input ";
				rettxt += ('0'+port-chan1);
				return rettxt;
			}
			else if ( port < returnmax)
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
				if (SendValid(i)) cols++;
			}
			return cols==0?1:cols;
		}
		void Mixer::GetParamRange(int numparam, int &minval, int &maxval)
		{
		}
		void Mixer::GetParamName(int numparam,char *name)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0)
			{
				if (send == 0)
				{
					strcpy(name,"Master Output");
				}
				else
					name[0] = '\0';
			}
			else if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || SendValid(send-send1)))
				{
					if ( send == mix )sprintf(name,"Channel %d - Mix",channel+1);
					else sprintf(name,"Channel %d - Send %d",channel+1,send);
				}
				else name[0] = '\0';
			}
			else if  ( send == 0){ name[0] = '\0'; return; }
			else
			{
				send-=send1;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) sprintf(name,"Input level Chn %02d",send+send1);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && SendValid(send)) sprintf(name,"Input level Ret %02d",send+send1);
				else name[0] = '\0';
			}
		}

		int Mixer::GetParamValue(int numparam)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0)
			{
				if (send == 0) return static_cast<int>(_outGain*256.0f);
				else return 0;
			}
			else if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || SendValid(send-send1)))
				{
					return (int)(_sendGrid[channel][send]*100.0f/_returnVolMulti[send-send1]);
				}
				else return 0;
			}
			else if  ( send == 0) return 0;
			else
			{
				send-=send1;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) return (int)(_inputConVol[send]*_wireMultiplier[send]*100.0f);
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && SendValid(send)) return (int)(_returnVol[send]*_returnVolMulti[send]*100.0f);
				else return 0;
			}
		}

		void Mixer::GetParamValue(int numparam, char *parVal)
		{
			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send=numparam%16; // 0 is for channel mix, others are send.
			if ( channel == 0)
			{
				if (send == 0) sprintf(parVal,"%.01fdB",dsp::dB(_outGain));
				else parVal[0] = '\0';
			}
			else if ( channel <= MAX_CONNECTIONS && send <= MAX_CONNECTIONS)
			{
				channel--;
				if ( _inputCon[channel] && (send==mix || SendValid(send-send1)))
				{
					sprintf(parVal,"%.01fdB",dsp::dB(_sendGrid[channel][send]/_returnVolMulti[send-send1]));
				}
				else  parVal[0] = '\0';
			}
			else if  ( send == 0) { parVal[0] = '\0'; return; }
			else
			{
				send-=send1;
				if ( channel == 0x0E && send < MAX_CONNECTIONS && _inputCon[send]) sprintf(parVal,"%.01fdB",dsp::dB(_inputConVol[send]*_wireMultiplier[send]));
				else if ( channel == 0x0F && send < MAX_CONNECTIONS && SendValid(send)) sprintf(parVal,"%.01fdB",dsp::dB(_returnVol[send]*_returnVolMulti[send]));
				else parVal[0] = '\0';
			}
		}

		bool Mixer::SetParameter(int numparam, int value)
		{	//channels:
			//0  -> volume colum -> master volume , channels volume and master gain.
			//1..12 -> channel column -> mix, sends , mute, solo and gain
			//14 -> return column -> volume of each return
			//15 -> return column -> gain of each return

			int channel=numparam/16; // channel E is input level and channel F is "fx's" level.
			int send(numparam % 16); // 0 is for channel mix, others are send, mute, solo and gain.
			if ( channel == 0)
			{
				if (send == 0) _masterVolume = value;
				else if (send == 15) _masterGain = value>4?4:value;
				else
				{
					send-=send1;
					if ( send < MAX_CONNECTIONS)
					{
						if ( value>100 ) value=100;
						SetWireVolume(send,value/100.0f);
						return true;
					}
					return false;
				}
				_outGain=_masterVolume/255.0f*_masterGain;
				return true;
			}
			if ( channel <= MAX_CONNECTIONS && send-send1 < MAX_CONNECTIONS)
			{
				channel--;
				if (!_inputCon[channel]) return false;
				
				if (send==mix) _sendGrid[channel][mix]=value/100.0f;
				else if (SendValid(send-send1))
				{
					_sendGrid[channel][send]=value/100.0f*_returnVolMulti[send-send1];
				}
				else if (send == solo)
				{
					solocolumn_=channel+1;
				}
				else if (send == mute)
				{
					_mutestate[channel]=value>0?true:false;
				}
				else if (send == gain)
				{

				}
				else return false;
				return true;
			}
			else if ( channel == 0x0E && send < MAX_CONNECTIONS) // vol
			{
				_returnVol[send]= value / (_returnVolMulti[send] * 100.0f);
				return true;
			}
			else if ( channel == 0x0F && send < MAX_CONNECTIONS) // gain
			{
				;
				return true;
			}
			return false;
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
			float vol = _returnVol[idx] * _returnVolMulti[idx];
			if ( SendValid(idx) ) return (Global::song()._pMachine[_send[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}

		bool Mixer::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(&size,sizeof(size));
			pFile->Read(&_outGain,sizeof(_outGain));
			pFile->Read(_sendValid,sizeof(_sendValid));
			pFile->Read(_send,sizeof(_send));
			pFile->Read(_sendGrid,sizeof(_sendGrid));
			///\todo: not store returnvolmulti and store a 0..1.0 scale for _returnVol.
			/// returnvolmulti should be calculated by the loader.
			pFile->Read(_returnVol,sizeof(_returnVol));
			pFile->Read(_returnVolMulti,sizeof(_returnVolMulti));
			for (int i=0;i<MAX_CONNECTIONS;i++) for (int j=0;j<MAX_CONNECTIONS;j++)
			{
				_sendGrid[j][i+send1]*=_returnVolMulti[i];
			}
			return true;
		}

		void Mixer::SaveSpecificChunk(RiffFile* pFile)
		{
			float sendGridTmp[MAX_CONNECTIONS][MAX_CONNECTIONS+1];
			memcpy(sendGridTmp,_sendGrid,sizeof(_sendGrid));
			for (int i=0;i<MAX_CONNECTIONS;i++) for (int j=0;j<MAX_CONNECTIONS;j++)
			{
				sendGridTmp[j][i+send1]/=_returnVolMulti[i];
			}
			std::uint32_t const size(sizeof _sendGrid + sizeof _send + sizeof _returnVol + sizeof _returnVolMulti + sizeof _sendValid + sizeof _outGain);
			pFile->Write(&size,sizeof(size));
			pFile->Write(&_outGain,sizeof(_outGain));
			pFile->Write(_sendValid,sizeof(_sendValid));
			pFile->Write(_send,sizeof(_send));
			pFile->Write(sendGridTmp,sizeof(sendGridTmp));
			///\todo: not store returnvolmulti and store a 0..1.0 scale for _returnVol.
			/// returnvolmulti should be calculated by the loader.
			pFile->Write(_returnVol,sizeof(_returnVol));
			pFile->Write(_returnVolMulti,sizeof(_returnVolMulti));
		}


	}
}