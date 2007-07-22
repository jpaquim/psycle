#include <packageneric/pre-compiled.private.hpp>

#include <diversalis/compiler.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#if defined _DEBUG
		#define new DEBUG_NEW
	#endif
#endif

#include "internal_machines.hpp"

//\todo: These three includes need to be replaced by a "host" callback which gives such information.
#include <psycle/engine/song.hpp>
#include <psycle/engine/player.hpp>
#include <psycle/host/uiconfiguration.hpp>

//\todo : The information required from this include should go to constants.hpp
//#include <psycle/host/InputHandler.hpp>

namespace psycle {
	namespace host {

		//////////////////////////////////////////////////////////////////////////
		// How to create a new Internal Machine / Host
		//////////////////////////////////////////////////////////////////////////
		// Create a new subclass of Machine ( you might want to take DuplicatorMac as an example, or Plugin for host example)
		// Define yours in the MachineClass struct. ( right now located in machine.hpp ),
		//	Machine::class_type is the machine type identifier ( master, plugin, sampler, ... )
		//  Machine::mode_type is the working mode of the machine ( effect, generator,... )
		// and then add the minfo to InternalMachinePackage, ( internal_machine_package.cpp, creator function ).
		// That should be all for creating/loading and saving. Might need more work for the GUI.

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy

		Dummy::Dummy(Machine::id_type id)
		: Machine(MACH_DUMMY, MACHMODE_FX, id)
		{
//			DefineStereoInput(1);
//			DefineStereoOutput(1);
			_audiorange = 32768.0f;
		}

		Dummy::~Dummy() throw()
		{
//			DestroyInputs();
//			DestroyOutputs();
		}
		Machine* Dummy::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			return new Dummy(_id);
		}

		void Dummy::Work(int numSamples)
		{
			Machine::Work(numSamples);
			cpu::cycles_type cost(cpu::cycles());
			Machine::SetVolumeCounter(numSamples);
			cost = cpu::cycles() - cost;
			work_cpu_cost(work_cpu_cost() + cost);
			_worked = true;
		}

		bool Dummy::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Skip(size);
			return true;
		};


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// NoteDuplicator

		DuplicatorMac::DuplicatorMac(Machine::id_type id)
		: Machine(MACH_DUPLICATOR, MACHMODE_GENERATOR, id)
		{
			_nCols = 2;
			bisTicking = false;
			for (int i=0;i<8;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
			}
		}

		DuplicatorMac::~DuplicatorMac() throw()
		{
		}

		Machine* DuplicatorMac::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			return new DuplicatorMac(_id);
		}

		void DuplicatorMac::Init()
		{
			Machine::Init();
			for (int i=0;i<8;i++)
			{
				macOutput[i]=-1;
				noteOffset[i]=0;
			}
		}

		void DuplicatorMac::Tick( int channel,PatternEntry* pData)
		{
			if ( !_mute && !bisTicking)
			{
				bisTicking=true;
				for (int i=0;i<8;i++)
				{
					PatternEntry pTemp = *pData;
					if ( pTemp._note <= notecommands::b9 )
					{
						pTemp._note+=noteOffset[i];
					}
					if (macOutput[i] != -1 && Global::song()._pMachine[macOutput[i]] != NULL 
						&& Global::song()._pMachine[macOutput[i]] != this) Global::song()._pMachine[macOutput[i]]->Tick(channel,&pTemp);
				}
			}
			bisTicking=false;
		}

		void DuplicatorMac::GetParamName(int numparam,char *name)
		{
			if (numparam >=0 && numparam<8)
			{
				sprintf(name,"Output Machine %d",numparam);
			} else if (numparam >=8 && numparam<16) {
				sprintf(name,"Note Offset %d",numparam-8);
			}
			else name[0] = '\0';
		}

		void DuplicatorMac::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if ( numparam < 8) { minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if ( numparam < 16) { minval = -48; maxval = 48; }
		}

		int DuplicatorMac::GetParamValue(int numparam)
		{
			if (numparam >=0 && numparam<8)
			{
				return macOutput[numparam];
			} else if (numparam >=8 && numparam <16) {
				return noteOffset[numparam-8];
			}
			else return 0;
		}

		void DuplicatorMac::GetParamValue(int numparam, char *parVal)
		{
			if (numparam >=0 && numparam <8)
			{
				if ((macOutput[numparam] != -1 ) &&( Global::song()._pMachine[macOutput[numparam]] != NULL))
				{
					sprintf(parVal,"%X -%s", macOutput[numparam],Global::song()._pMachine[macOutput[numparam]]->GetEditName().c_str());
				}
				else if (macOutput[numparam] != -1) sprintf(parVal,"%X (none)",macOutput[numparam]);
				else sprintf(parVal,"(disabled)");

			} else if (numparam >=8 && numparam <16) {
				char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
				sprintf(parVal,"%s%d",notes[(noteOffset[numparam-8]+60)%12],(noteOffset[numparam-8]+60)/12);
			}
			else parVal[0] = '\0';
		}

		bool DuplicatorMac::SetParameter(int numparam, int value)
		{
			if (numparam >=0 && numparam<8)
			{
				macOutput[numparam]=value;
				return true;
			} else if (numparam >=8 && numparam<16) {
				noteOffset[numparam-8]=value;
				return true;
			}
			else return false;
		}

		void DuplicatorMac::Work(int numSamples)
		{
			_worked = true;
			_stopped = false;
		}

		bool DuplicatorMac::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(macOutput);
			pFile->Read(noteOffset);
			return true;
		}

		void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof macOutput + sizeof noteOffset);
			pFile->Write(size);
			pFile->Write(macOutput);
			pFile->Write(noteOffset);
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Master

		float * Master::_pMasterSamples = 0;

		Master::Master(Machine::id_type id)
		: Machine(MACH_MASTER, MACHMODE_MASTER, id)
		,sampleCount(0)
		,_outDry(256)
		,decreaseOnClip(false)
		{
			_audiorange = 32768.0f;
			DefineStereoInput(1);
		}

		Master::~Master() throw()
		{
		}
		Machine* Master::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			return new Master(_id);
		}

		void Master::Stop()
		{
			_clip = false;
			sampleCount = 0;
		}

		void Master::Init()
		{
			Machine::Init();
			sampleCount = 0;
			//_LMAX = 1; // Min value should NOT be zero, because we use a log10() to calculate the vu-meter's value.
			//_RMAX = 1;
			currentpeak=0.0f;
			peaktime=1;
			_lMax = 1;
			_rMax = 1;
			vuupdated = false;
			_clip = false;
		}
		void Master::Tick(PatternEntry *pEntry)
		{
			if ( pEntry->_note == PatternCmd::SET_VOLUME )
			{
				_outDry = pEntry->_parameter;
			}
		}

		void Master::Work(int numSamples)
		{
			#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
				universalis::processor::exceptions::fpu::mask fpu_exception_mask(this->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
			#endif
			Machine::Work(numSamples);
			cpu::cycles_type cost(cpu::cycles());
			//if(!_mute)
			//{
			float mv = CValueMapper::Map_255_1(_outDry);

			float *pSamples = _pMasterSamples;
			float *pSamplesL = _pSamplesL;
			float *pSamplesR = _pSamplesR;

			//_lMax -= numSamples*8;
			//_rMax -= numSamples*8;
			//_lMax *= 0.5;
			//_rMax *= 0.5;
			if(vuupdated) 
			{ 
				_lMax *= 0.5; 
				_rMax *= 0.5; 
			}
			int i = numSamples;
			if(decreaseOnClip)
			{
				do
				{
					// Left channel
					if(std::fabs(*pSamples = *pSamplesL = *pSamplesL * mv) > _lMax)
					{
						_lMax = fabsf(*pSamplesL);
					}
					if(*pSamples > 32767.0f)
					{
						_outDry = f2i((float)_outDry * 32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesL = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = f2i((float)_outDry * -32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesL = -32767.0f; 
					}
					pSamples++;
					pSamplesL++;
					// Right channel
					if(std::fabs(*pSamples = *pSamplesR = *pSamplesR * mv) > _rMax)
					{
						_rMax = fabsf(*pSamplesR);
					}
					if(*pSamples > 32767.0f)
					{
						_outDry = f2i((float)_outDry * 32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesR = 32767.0f; 
					}
					else if (*pSamples < -32767.0f)
					{
						_outDry = f2i((float)_outDry * -32767.0f / (*pSamples));
						mv = CValueMapper::Map_255_1(_outDry);
						*pSamples = *pSamplesR = -32767.0f; 
					}
					pSamples++;
					pSamplesR++;
				}
				while (--i);
			}
			else
			{
				do
				{
					// Left channel
					if(std::fabs( *pSamples++ = *pSamplesL = *pSamplesL * mv) > _lMax)
					{
						_lMax = fabsf(*pSamplesL);
					}
					pSamplesL++;
					// Right channel
					if(std::fabs(*pSamples++ = *pSamplesR = *pSamplesR * mv) > _rMax)
					{
						_rMax = fabsf(*pSamplesR);
					}
					pSamplesR++;
				}
				while (--i);
			}
			if(_lMax > 32767.0f)
			{
				_clip=true;
				_lMax = 32767.0f; //_LMAX = 32768;
			}
			else if (_lMax < 1.0f) { _lMax = 1.0f; /*_LMAX = 1;*/ }
			//else _LMAX = Dsp::F2I(_lMax);
			if(_rMax > 32767.0f)
			{
				_clip=true;
				_rMax = 32767.0f; //_RMAX = 32768;
			}
			else if(_rMax < 1.0f) { _rMax = 1.0f; /*_RMAX = 1;*/ }
			//else _RMAX = Dsp::F2I(_rMax);
			if( _lMax > currentpeak ) currentpeak = _lMax;
			if( _rMax > currentpeak ) currentpeak = _rMax;
			//}
			sampleCount+=numSamples;
			cost = cpu::cycles() - cost;
			work_cpu_cost(work_cpu_cost() + cost);
			_worked = true;
		}

		bool Master::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(_outDry);
			pFile->Read(decreaseOnClip);
			return true;
		};

		void Master::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof _outDry + sizeof decreaseOnClip);
			pFile->Write(size);
			pFile->Write(_outDry);
			pFile->Write(decreaseOnClip);
		};


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mixer


		Mixer::Mixer(Machine::id_type id)
		: Machine(MACH_MIXER, MACHMODE_FX, id)
		{
			_audiorange = 32768.0f;
			DefineStereoInput(24);
			DefineStereoOutput(1);
		}

		Mixer::~Mixer() throw()
		{
		}
		Machine* Mixer::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			return new Mixer(_id);
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
			cpu::cycles_type cost(cpu::cycles());
			if(!_mute && !_stopped )
			{
				Mix(numSamples);
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
				cost = cpu::cycles() - cost;
				work_cpu_cost(work_cpu_cost() + cost);
			}

			cost = cpu::cycles();
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			cost = cpu::cycles() - cost;
			wire_cpu_cost(wire_cpu_cost() + cost);

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
								cpu::cycles_type cost(cpu::cycles());
								for (int j=0; j<MAX_CONNECTIONS; j++)
								{
									if (_inputCon[j])
									{
										Machine* pInMachine = Global::song()._pMachine[_inputMachines[j]];
										if (pInMachine)
										{
											if(!_mute && !_stopped && _sendGrid[j][send0+i]!= 0.0f)
											{
												dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[j]*_sendGrid[j][send0+i]);
												dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[j]*_sendGrid[j][send0+i]);
											}
										}
									}
								}
								cost = cpu::cycles() - cost;
								work_cpu_cost(work_cpu_cost() + cost);
							}

							// tell the FX to work, now that the input is ready.
							{
								#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
									universalis::processor::exceptions::fpu::mask fpu_exception_mask(pSendMachine->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
								#endif
								pSendMachine->Work(numSamples);
							}

							{
								cpu::cycles_type cost(cpu::cycles());
								pSendMachine->_waitingForSound = false;
								dsp::Clear(_pSamplesL, numSamples);
								dsp::Clear(_pSamplesR, numSamples);
								cost = cpu::cycles() - cost;
								work_cpu_cost(work_cpu_cost() + cost);
							}

						}
						if(!pSendMachine->_stopped) _stopped = false;
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
						if(!_mute && !_stopped)
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
						if(!_mute && !_stopped && _sendGrid[i][mix] != 0.0f)
						{
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]*_sendGrid[i][mix]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]*_sendGrid[i][mix]);
						}
					}
				}
			}
		}

		bool Mixer::ConnectTo(Machine & dst_machine, InPort::id_type dstport, OutPort::id_type outport, float volume)
		{
			//
			// \todo ?
			//
			return Machine::ConnectTo(dst_machine,dstport,outport,volume);
		}

		std::string Mixer::GetAudioInputName(InPort::id_type port)
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
			Wire::id_type send(numparam % 16); // 0 is for channel mix, others are send.
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
			pFile->Read(size);
			pFile->Read(_sendGrid);
			pFile->Read(_send);
			pFile->Read(_sendVol);
			pFile->Read(_sendVolMulti);
			pFile->Read(_sendValid);
			return true;
		}

		void Mixer::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof _sendGrid + sizeof _send + sizeof _sendVol + sizeof _sendVolMulti + sizeof _sendValid);
			pFile->Write(size);
			pFile->Write(_sendGrid);
			pFile->Write(_send);
			pFile->Write(_sendVol);
			pFile->Write(_sendVolMulti);
			pFile->Write(_sendValid);
		}

		float Mixer::VuChan(Wire::id_type idx)
		{
			float vol;
			GetWireVolume(idx,vol);
			if ( _inputCon[idx] ) return (Global::song()._pMachine[_inputMachines[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}

		float Mixer::VuSend(Wire::id_type idx)
		{
			float vol = _sendVol[idx] * _sendVolMulti[idx];
			if ( _sendValid[idx] ) return (Global::song()._pMachine[_send[idx]]->_volumeDisplay/97.0f)*vol;
			return 0.0f;
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LFO

#if 0 // don't worry, msvc is the weird
		int const LFO::LFO_SIZE;
		int const LFO::MAX_PHASE;
		int const LFO::MAX_SPEED;
		int const LFO::MAX_DEPTH;
		int const LFO::NUM_CHANS;
#endif

		LFO::LFO(Machine::id_type id)
		: Machine(MACH_LFO, MACHMODE_GENERATOR, id)
		{
			_nCols = 3;
			bisTicking = false;
		}

		LFO::~LFO() throw()
		{
		}
		Machine* LFO::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			return new LFO(_id);
		}

		void LFO::Init()
		{
			Machine::Init();
			for (int i=0;i<NUM_CHANS;i++)
			{
				macOutput[i]=-1;
				paramOutput[i]=-1;
				level[i]=100;
				phase[i]=MAX_PHASE/2.0f;
				prevVal[i]=centerVal[i]=0.0;
			}
			lfoPos=0.0;
			lSpeed=MAX_SPEED/10;
			waveform=lfo_types::sine;
			FillTable();
		}

		void LFO::Tick( int channel,PatternEntry* pData)
		{
			if(!bisTicking)
			{
				bisTicking=true;
				if(pData->_cmd==0x01)
					lfoPos=0.0;
			}
			bisTicking=false;
		}

		void LFO::GetParamName(int numparam,char *name)
		{
			if(numparam==prms::wave)
				sprintf(name,"Waveform");
			else if(numparam==prms::speed)
				sprintf(name,"Speed");
			else if (numparam<prms::prm0)
				sprintf(name,"Output Machine %d",numparam-prms::mac0);
			else if (numparam<prms::level0)
				sprintf(name,"Output Param %d",numparam-prms::prm0);
			else if (numparam<prms::phase0)
				sprintf(name,"Output Level %d",numparam-prms::level0);
			else if (numparam<prms::num_params)
				sprintf(name,"Output Phase %d",numparam-prms::phase0);
			else name[0] = '\0';
		}

		void LFO::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if(numparam==prms::wave) { minval = 0; maxval = lfo_types::num_lfos-1;}
			else if (numparam==prms::speed) {minval = 0; maxval = MAX_SPEED;}
			else if (numparam <prms::prm0) {minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if (numparam <prms::level0)
			{
				minval = -1;
				if(macOutput[numparam-prms::prm0]==-1 || Global::song()._pMachine[macOutput[numparam-prms::prm0]] == NULL)
					maxval = -1;
				else
					maxval =  Global::song()._pMachine[macOutput[numparam-prms::prm0]]->GetNumParams()-1;
			}

			else if (numparam <prms::phase0){minval = 0; maxval = MAX_DEPTH*2;}
			else if (numparam <prms::num_params){minval = 0; maxval = MAX_PHASE; }
			else {minval=0;maxval=0; }

		}

		int LFO::GetParamValue(int numparam)
		{
			if(numparam==prms::wave)			return waveform;
			else if(numparam==prms::speed)	return lSpeed;
			else if(numparam <prms::prm0)	return macOutput[numparam-prms::mac0];
			else if(numparam <prms::level0)	return paramOutput[numparam-prms::prm0];
			else if(numparam <prms::phase0)	return level[numparam-prms::level0];
			else if(numparam <prms::num_params)	return phase[numparam-prms::phase0];
			else return 0;
		}

		void LFO::GetParamValue(int numparam, char *parVal)
		{
			if(numparam==prms::wave)
			{
				switch(waveform)
				{
				case lfo_types::sine: sprintf(parVal, "sine"); break;
				case lfo_types::tri: sprintf(parVal, "triangle"); break;
				case lfo_types::saw: sprintf(parVal, "saw"); break;
				case lfo_types::square: sprintf(parVal, "square"); break;
				default: throw;
				}
			} 
			else if(numparam==prms::speed)
			{
				if(lSpeed==0)
					sprintf(parVal, "inf.");
				else
				{
					float speedInMs = 100.0f / (float)(lSpeed/(float)(MAX_SPEED));
					if(speedInMs<1000.0f)
						sprintf(  parVal, "%.1f ms", speedInMs);		
					else
						sprintf( parVal, "%.3f secs", speedInMs/1000.0f);
				}
			} 
			else if(numparam<prms::prm0)
			{
				if ((macOutput[numparam-prms::mac0] != -1 ) &&( Global::song()._pMachine[macOutput[numparam-prms::mac0]] != NULL))
					sprintf(parVal,"%X -%s",macOutput[numparam-prms::mac0],Global::song()._pMachine[macOutput[numparam-prms::mac0]]->GetEditName().c_str());
				else if (macOutput[numparam-prms::mac0] != -1)
					sprintf(parVal,"%X (none)",macOutput[numparam-prms::mac0]);
				else 
					sprintf(parVal,"(disabled)");
			}
			else if(numparam<prms::level0)
			{
				if(		(macOutput[numparam-prms::prm0] != -1) 
					&&	(Global::song()._pMachine[macOutput[numparam-prms::prm0]] != NULL)
					&&  (paramOutput[numparam-prms::prm0] >= 0)	)
				{
					if		(paramOutput[numparam-prms::prm0] < Global::song()._pMachine[macOutput[numparam-prms::prm0]]->GetNumParams())
					{
						char name[128];
						Global::song()._pMachine[macOutput[numparam-prms::prm0]]->GetParamName(paramOutput[numparam-prms::prm0], name);
						sprintf(parVal,"%X -%s", paramOutput[numparam-prms::prm0], name);
					}
					else
						sprintf(parVal,"%X -none", paramOutput[numparam-prms::prm0]);					
				} 
				else 
					sprintf(parVal,"(disabled)");

			}
			else if(numparam<prms::phase0)
				sprintf(parVal,"%i%%", level[numparam-prms::level0]-MAX_DEPTH);
			else if(numparam<prms::num_params)
				sprintf(parVal,"%.1f deg.", (phase[numparam-prms::phase0]-MAX_PHASE/2.0f)/float(MAX_PHASE/2.0f) * 180.0f);
			else
				parVal[0] = '\0';
		}

		bool LFO::SetParameter(int numparam, int value)
		{
			if(numparam==prms::wave)
			{
				waveform = value;
				FillTable();
				return true;
			}
			else if(numparam==prms::speed)
			{
				lSpeed = value;
				return true;
			}
			else if(numparam <prms::prm0)
			{
				if(value!=macOutput[numparam-prms::mac0])
				{
					int newMac(0);

					//if we're increasing, increase until we hit an active machine
					if(value>macOutput[numparam-prms::mac0])
					{
						for(newMac=value; newMac<MAX_MACHINES && Global::song()._pMachine[newMac]==NULL; ++newMac)
							;
					}
					//if we're decreasing, or if we're increasing but didn't find anything, decrease until we find an active machine
					if(value<macOutput[numparam-prms::mac0] || newMac>=MAX_MACHINES)
					{
						for(newMac=value;newMac>-1 && Global::song()._pMachine[newMac]==NULL; --newMac)
							;
					}

					ParamEnd(numparam-prms::mac0);
					macOutput[numparam-prms::mac0] = newMac;
					paramOutput[numparam-prms::mac0] = -1;
				}
				return true;
			}
			else if(numparam <prms::level0)
			{
				if( macOutput[numparam-prms::prm0]>-1 && Global::song()._pMachine[macOutput[numparam-prms::prm0]] )
				{
					if(value<Global::song()._pMachine[macOutput[numparam-prms::prm0]]->GetNumParams())
					{
						ParamEnd(numparam-prms::prm0);
						paramOutput[numparam-prms::prm0] = value;
						ParamStart(numparam-prms::prm0);
					}
					else
					{
						ParamEnd(numparam-prms::prm0);
						paramOutput[numparam-prms::prm0] = Global::song()._pMachine[macOutput[numparam-prms::prm0]]->GetNumParams()-1;
						ParamStart(numparam-prms::prm0);
					}
				}
				else
					paramOutput[numparam-prms::prm0] = -1;

				return true;
			}
			else if(numparam <prms::phase0)
			{
				level[numparam-prms::level0] = value;
				return true;
			}
			else if(numparam <prms::num_params)
			{
				phase[numparam-prms::phase0] = value;
				return true;
			}
			else return false;
		}

		void LFO::PreWork(int numSamples)
		{
			Machine::PreWork(numSamples);
			cpu::cycles_type cost(cpu::cycles());

			int maxVal=0, minVal=0;
			int curVal=0, newVal=0;
			float curLFO=0.0;
			float lfoAmt=0.0;
			bool bRedraw=false;

			for(int j(0);j<NUM_CHANS;++j)
			{
				if	(	macOutput[j] != -1		&&	Global::song()._pMachine[macOutput[j]] != NULL
					&&  paramOutput[j] != -1	&&	paramOutput[j] < Global::song()._pMachine[macOutput[j]]->GetNumParams() )
				{
					Global::song()._pMachine[macOutput[j]]->GetParamRange(paramOutput[j], minVal, maxVal);
					curVal = Global::song()._pMachine[macOutput[j]]->GetParamValue(paramOutput[j]);
					curLFO = waveTable[	int(lfoPos+phase[j]+(MAX_PHASE/2.0f)) % LFO_SIZE];
					lfoAmt = (level[j]-MAX_DEPTH)/(float)MAX_DEPTH;

					centerVal[j] -= prevVal[j] - curVal;  //compensate for external movement

					newVal = curLFO * ((maxVal-minVal)/2.0f) * lfoAmt + centerVal[j];

					if(newVal>maxVal) newVal=maxVal;
					else if(newVal<minVal) newVal=minVal;

					Global::song()._pMachine[macOutput[j]]->SetParameter(paramOutput[j], newVal);	//make it happen!
					bRedraw=true;
					prevVal[j] = newVal;
				}
			}

			if(bRedraw)
				Global::player().Tweaker=true;
			bRedraw=false;

			float minms = Global::player().SampleRate() /1000.0f * 100.0f;	//100ms in samples
			lfoPos += (lSpeed/ float(MAX_SPEED)) * (LFO_SIZE/float(minms/float(numSamples)));
			if(lfoPos>LFO_SIZE) lfoPos-=LFO_SIZE;

			cost = cpu::cycles() - cost;
			work_cpu_cost(work_cpu_cost() + cost);
		}
		
		void LFO::Work(int numSamples)
		{
			_worked=true;
		}

		bool LFO::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(waveform);
			pFile->Read(lSpeed);
			pFile->Read(macOutput);
			pFile->Read(paramOutput);
			pFile->Read(level);
			pFile->Read(phase);
			return true;
		}

		void LFO::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t const size(sizeof waveform + sizeof lSpeed + sizeof macOutput + sizeof paramOutput + sizeof level + sizeof phase);
			pFile->Write(size);
			pFile->Write(waveform);
			pFile->Write(lSpeed);
			pFile->Write(macOutput);
			pFile->Write(paramOutput);
			pFile->Write(level);
			pFile->Write(phase);
		}

		void LFO::FillTable()
		{
			switch(waveform)
			{
			case lfo_types::sine:
				for (int i(0);i<LFO_SIZE;++i)
				{
					waveTable[i]=sin(i/float(LFO_SIZE-1) * 6.2831853071795864769252);
				}
				break;
			case lfo_types::tri:
				for (int i(0);i<LFO_SIZE/2;++i)
				{
					waveTable[i] = waveTable[LFO_SIZE-i-1] = i/float(LFO_SIZE/4) - 1;
				}
				break;
			case lfo_types::saw:
				for (int i(0);i<LFO_SIZE;++i)
				{
					waveTable[i] = i/float((LFO_SIZE-1)/2) - 1;
				}
				break;
			case lfo_types::square:
				for (int i(0);i<LFO_SIZE/2;++i)
				{
					waveTable[i] = 1;
					waveTable[LFO_SIZE-1-i]=-1;
				}
				break;
			default:
				for(int i(0);i<LFO_SIZE;++i)	//????
				{
					waveTable[i] = 0;
				}
				break;
			}
		}

		void LFO::ParamStart(int which)
		{
			if(which<0 || which>=NUM_CHANS) return;  //jic
			int destMac = macOutput[which];
			int destParam = paramOutput[which];

			if	(	destMac		!= -1	&&	Global::song()._pMachine[destMac] != NULL
				&&  destParam	!= -1	&&	destParam < Global::song()._pMachine[destMac]->GetNumParams())
			{
				int minVal, maxVal;
				float curLFO, lfoAmt;

				Global::song()._pMachine[destMac]->GetParamRange(destParam, minVal, maxVal);
				curLFO = waveTable[	int(lfoPos+phase[which]+(MAX_PHASE/2.0f)) % LFO_SIZE];
				lfoAmt = (level[which]-MAX_DEPTH)/(float)MAX_DEPTH;

				//bad! bad!
				//prevVal[which] = Global::song()._pMachine[macOutput[which]]->GetParamValue(paramOutput[which]);
				//centerVal[which] = prevVal[which] - (curLFO * ((maxVal-minVal)/2.0f) * lfoAmt);

				centerVal[which] = Global::song()._pMachine[destMac]->GetParamValue(destParam);
				prevVal[which] = centerVal[which];

				// the way i've set this up, a control will 'jump' if the lfo is at a peak or dip when a control is first selected.
				// that may seem like a bad thing, but it means that the center of the lfo does -not- depend on the lfo position
				// when a control is selected.. and i think that this would be the preferred behavior.

			}
			else
			{
				prevVal[which]=0;
				centerVal[which]=0;
			}
		}


		/// currently, this function resets the value of an output parameter to where it would be at lfo==0.  this behavior deserves
		/// some consideration, because it is conceivable that some people would want turning off an output to leave the parameter where it is
		/// instead of jerking it back to its original position..  on the other hand, without this code, sweeping through a list of parameters
		/// carelessly can wreak havoc on an entire plugin's settings.  i may decide just to let the user choose which s/he prefers..
		void LFO::ParamEnd(int which)
		{
			if(which<0 || which>=NUM_CHANS) return;
			id_type destMac(macOutput[which]);
			int destParam = paramOutput[which];

			if	(	destMac		!= -1	&&	Global::song()._pMachine[destMac] != NULL
				&&  destParam	!= -1	&&	destParam < Global::song()._pMachine[destMac]->GetNumParams())
			{
				int minVal, maxVal;
				int newVal;
				Global::song()._pMachine[destMac]->GetParamRange(destParam, minVal, maxVal);
				newVal = centerVal[which];
				if(newVal<minVal) newVal=minVal;
				else if(newVal>maxVal) newVal=maxVal;

				if(destMac != this->id()) // craziness may ensue without this check.. folks routing the lfo to itself are on their own
					Global::song()._pMachine[destMac]->SetParameter(destParam, newVal); //set to value at lfo==0
			}
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Automator Machine
		
		// still preliminary, but relatively functional
		// (it can only be triggered from the pattern currently-- if you want to test it: 01-- is the trigger command, aux/ins column is dest
		// machine, command parameter is dest param.. so, for example, if automator machine is 40, the command to modulate parameter 2E of
		// mach 03 would be  --- 03 40 012E		)

		//todo: -the map for the continuous table's been replaced with a sorted vector, but i'm not convinced the pros outweigh the cons..
		//       decide if there's a better way.
		//      -implement control points, for curving segments :)  maybe some kind of simplified bezier thing? i need to consider the 
		//       geometry of it a bit more..
		//      -come up with a cooler name :)
		//      -decide if we really want this thing to be two machines combined (meaning the discrete/continuous modes).. and if so,
		//       decide if there's a cleaner way of dealing with the machine's dual nature
		//      -option to stretch/squeeze envelope to match length changes (probably just for continuous mode)

		Automator::Automator(Machine::id_type id)
		: Machine(MACH_AUTOMATOR, MACHMODE_GENERATOR, id)
		,dLength(64)
		,cLength(10000)
		,dStepSize(1.0f)
		,bDiscrete(true)
		,bRelative(false)
		{
			_nCols = 0;
			bisTicking = false;
		}

		Automator::~Automator() throw()
		{
			for(std::vector<Track*>::iterator iter = tracks.begin(); iter!=tracks.end(); ++iter)
				delete *iter;
		}
		Machine* Automator::CreateFromType(Machine::id_type _id, std::string _dllname)
		{
			return new Automator(_id);
		}

		void Automator::Init()
		{
			Machine::Init();
			dTable.resize(dLength);
			for(int i(0);i<dLength/2;++i)
			{
				dTable[i]=i/float(dLength*2) + 0.375;
				dTable[dLength-1-i]=dTable[i];
			}
			Node start(0, 1.0f);
			Node middle(cLength/2, 0.0f);
			Node end(cLength, 1.0f);
			cTable.push_back(start);
			cTable.push_back(middle);
			cTable.push_back(end);
			std::sort(cTable.begin(), cTable.end());
		}

		void Automator::Tick( int channel,PatternEntry* pData)
		{
			if(!bisTicking)
			{
				bisTicking=true;
				if(pData->_cmd==0x01)
				{
					if(Global::song()._pMachine[pData->_inst] && pData->_parameter < Global::song()._pMachine[pData->_inst]->GetNumParams())
					{
						tracks.push_back(new Track(pData->_inst, pData->_parameter));

						int min, max;
						Global::song()._pMachine[pData->_inst]->GetParamRange(pData->_parameter, min, max);

						std::vector<Track*>::iterator iter = tracks.end()-1;
						(*iter)->prevVal = (*iter)->centerVal = Global::song()._pMachine[pData->_inst]->GetParamValue(pData->_parameter);
						(*iter)->minVal=min;
						(*iter)->maxVal=max;
						
					}
				}
			}
			bisTicking=false;
		}

		void Automator::Tick()
		{
			sampsPerStep = Global::player().SamplesPerRow()*dStepSize;
			sampsPerMs = Global::player().SampleRate()/1000;
		}

		void Automator::GetParamName(int numparam,char *name)
		{
			switch(numparam)
			{
			case prms::discrete:
				sprintf(name, "Sequence Type");
				break;
			case prms::relative:
				sprintf(name, "Sequence Mode");
				break;
			case prms::clength:
			case prms::dlength:
				sprintf(name, "Sequence Length");
				break;
			case prms::dstep:
				sprintf(name, "Step Length");
				break;
			}
		}

		void Automator::GetParamRange(int numparam,int &minval,int &maxval)
		{
			switch(numparam)
			{
			case prms::discrete:
			case prms::relative:
				minval = 0; maxval = 1;
				break;
			case prms::clength:
				minval = 0; maxval = CLENGTH_MAX;
				break;
			case prms::dlength:
				minval = 0; maxval = DLENGTH_MAX;
				break;
			case prms::dstep:
				minval = 0; maxval = DSTEP_MAX;
				break;
			}
		}

		int Automator::GetParamValue(int numparam)
		{
			switch(numparam)
			{
			case prms::discrete:
				return (int)bDiscrete;
			case prms::relative:
				return (int)bRelative;
			case prms::clength:
				return cLength;
			case prms::dlength:
				return dLength;
			case prms::dstep:
			default:
				return (int)(dStepSize * DSTEP_SCALER);
			}
		}

		void Automator::GetParamValue(int numparam, char *parVal)
		{
			switch(numparam)
			{
			case prms::discrete:
				if(bDiscrete)		sprintf(parVal, "Discrete");
				else				sprintf(parVal, "Continuous");
				break;
			case prms::relative:
				if(bRelative)		sprintf(parVal, "Relative");
				else				sprintf(parVal, "Absolute");
				break;
			case prms::clength:
				if(cLength<1000)	sprintf(parVal, "%i ms", cLength);
				else				sprintf(parVal, "%.3f secs", cLength/1000.0f);
				break;
			case prms::dlength:
				sprintf(parVal, "%.2f ticks", dLength*dStepSize);
				break;
			case prms::dstep:
				sprintf(parVal, "%.2f ticks", dStepSize);
				break;
			}
		}

		bool Automator::SetParameter(int numparam, int value)
		{
			std::vector<Node>::iterator iter;
			switch(numparam)
			{
			case prms::discrete:
				if(bDiscrete != (bool) value)
					Stop();
				bDiscrete = (bool)value;
				return true;
			case prms::relative:
				if(bRelative != (bool) value)
					Stop();
				bRelative = (bool)value;
				return true;
			case prms::clength:
				iter = std::upper_bound(cTable.begin(), cTable.end(), value);
				//here, iter will either point to the first element greater than the new length (if we're decreasing), or cTable.end()
				//(if we're increasing..) if it's cTable.end(), decrement so it points to a valid index.
				if(iter == cTable.end()) iter--;
				else
				{	//if we're decreasing, we want to erase anything that may be after iter in the table.. this way, if we shorten the length past
					//two nodes, we ditch the last one and keep the value of the second-to-last.
					cTable.erase(iter+1, cTable.end());
				}
				//update the last element to match the new cLength
				iter->time=value;
				cLength=value;
				return true;
			case prms::dlength:
				dLength=value;
				if(dLength>dTable.size())
					dTable.resize(dLength);
				return true;
			case prms::dstep:
				dStepSize=value/(float)DSTEP_SCALER;
				return true;
			}
			return false;
		}

		void Automator::PreWork(int numSamples)
		{
			Machine::PreWork(numSamples);
			cpu::cycles_type cost(cpu::cycles());
			bool somethinghappened=false;
			for(std::vector<Track*>::iterator iter = tracks.begin(); iter != tracks.end(); ++iter)
			{
				if( (*iter)->pos >= (bDiscrete? dLength: cLength) )
				{
					if(bResetWhenDone)
					{
						if(bRelative)
							Global::song()._pMachine[(*iter)->mach]->SetParameter( (*iter)->param, (*iter)->centerVal );
						else
							Global::song()._pMachine[(*iter)->mach]->SetParameter( (*iter)->param, (*iter)->prevVal );
					}
					delete *iter;
					iter = tracks.erase(iter);
					--iter;
				}
				else
				{
					int setto;
					int range((*iter)->maxVal-(*iter)->minVal);

					if(bRelative)
					{
						int newval = Global::song()._pMachine[(*iter)->mach]->GetParamValue((*iter)->param);
						(*iter)->centerVal += newval - (*iter)->prevVal;	//adjust for outside changes
						float temp = EvaluatePos((*iter)->pos);
						int temp2 = (*iter)->centerVal;
						setto = temp2 + range * temp; //EvaluatePos((*iter)->pos);
						if(setto>(*iter)->maxVal)		setto=(*iter)->maxVal;
						else if(setto<(*iter)->minVal)	setto=(*iter)->minVal;
						(*iter)->prevVal=setto;
					}
					else	//absolute mode
						setto = (*iter)->minVal + EvaluatePos((*iter)->pos)*range;

					Global::song()._pMachine[(*iter)->mach]->SetParameter( (*iter)->param, setto );
					(*iter)->pos += numSamples/(float) (bDiscrete? sampsPerStep: sampsPerMs);
					somethinghappened=true;
				}
			}

			if(somethinghappened) Global::player().Tweaker=true;

			cost = cpu::cycles() - cost;
			work_cpu_cost(work_cpu_cost() + cost);
		}
		
		void Automator::Work(int numSamples)
		{
			_worked=true;
		}

		bool Automator::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			int tablelength;
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(bRelative);
			pFile->Read(bDiscrete);
			pFile->Read(dStepSize);
			pFile->Read(dLength);
			dTable.clear();
			for(int i(0);i<dLength;++i)
			{
				float value;
				pFile->Read(value);
				dTable.push_back(value);
			}
			pFile->Read(tablelength);
			cTable.clear();
			int key(0);
			for(int i(0);i<tablelength;++i)
			{
				float value;
				pFile->Read(key);
				pFile->Read(value);
				Node node(key, value);
				cTable.push_back(node);
			}
			std::sort(cTable.begin(), cTable.end());	//really shouldn't be necessary, but just in case..
			cLength = key;

			return true;
		}

		void Automator::SaveSpecificChunk(RiffFile* pFile)
		{
			int cTabSize = cTable.size();
			std::uint32_t const size(	  sizeof bDiscrete + sizeof bRelative + sizeof dStepSize 
										+ sizeof dLength + dLength * std::numeric_limits<float>::digits
										+ sizeof cTabSize + cTabSize*std::numeric_limits<int>::digits + cTabSize*std::numeric_limits<float>::digits);
			pFile->Write(size);
			pFile->Write(bRelative);
			pFile->Write(bDiscrete);
			pFile->Write(dStepSize);
			pFile->Write(dLength);
			dTable.resize(dLength);
			for(std::vector<float>::iterator iter = dTable.begin(); iter != dTable.end(); iter++)
				pFile->Write(*iter);
			pFile->Write(cTabSize);
			for(std::vector<Node>::iterator iter = cTable.begin(); iter != cTable.end(); ++iter)
			{
				pFile->Write(iter->time);
				pFile->Write(iter->value);
			}
			
		}

		void Automator::Stop()
		{
			for(std::vector<Track*>::iterator iter = tracks.begin(); iter!=tracks.end(); ++iter)
			{
				delete *iter;	//todo: check bResetOnStop
			}
			tracks.clear();
		}

		float Automator::EvaluatePos(float pos)
		{
			float val;
			if(bDiscrete)
				val = dTable.at((int)floor(pos));
			else
			{
				std::vector<Node>::iterator first = std::upper_bound(cTable.begin(), cTable.end(), (int)pos);
				std::vector<Node>::iterator second = first;
				first--;
				float weight = ( pos - first->time ) / (float)(second->time - first->time);	//position between the two neighboring nodes
				val = (1-weight) * first->value + weight * second->value;					//interpolate linearly between the value of each
			}
			if(bRelative)
				val = val*2-1;

			return val;
		}
	}
}
