// -*- mode:c++; indent-tabs-mode:t -*-
#include <psycle/core/psycleCorePch.hpp>

#include "internal_machines.h"

///\todo: These two includes need to be replaced by a "host" callback which gives such information.
#include "commands.h"
#include "player.h"
#include "song.h"
#include "dsp.h"
#include "fileio.h"

namespace psy {
	namespace core {

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dummy

		std::string Dummy::_psName = "DummyPlug";

		Dummy::Dummy(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song)
		:
			Machine(callbacks, MACH_DUMMY, MACHMODE_FX, id, song)
		{
			SetEditName("Dummy");
			//DefineStereoInput(1);
			//DefineStereoOutput(1);
			SetAudioRange(32768.0f);
			wasVST=false;
		}
		Dummy::Dummy(Machine* oldmac)
		:
			Machine(oldmac,MACH_DUMMY,oldmac->mode())
		{
			SetAudioRange(32768.0f);

			if (oldmac->type() == MACH_VST || oldmac->type() == MACH_VSTFX)
			{
				// we need to redo this, because SetAudioRange cannot be called before the Machine constructor.
				for (int i=0; i<MAX_CONNECTIONS; i++)
				{
					_wireMultiplier[i]=oldmac->_wireMultiplier[i]*(oldmac->GetAudioRange()/GetAudioRange());
				}
				wasVST = true;
			}
			else
				wasVST = false;
		}
		
		Dummy::~Dummy() throw()
		{
			//DestroyInputs();
			//DestroyOutputs();
		}

		int Dummy::GenerateAudio(int numSamples)
		{
			//cpu::cycles_type cost(cpu::cycles());
			Machine::UpdateVuAndStanbyFlag(numSamples);
			//cost = cpu::cycles() - cost;
			//work_cpu_cost(work_cpu_cost() + cost);
			_worked = true;
			return numSamples;
		}

		bool Dummy::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Skip(size);
			return true;
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// NoteDuplicator

		std::string DuplicatorMac::_psName = "Dupe it!";

		DuplicatorMac::DuplicatorMac(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song)
		:
			Machine(callbacks, MACH_DUPLICATOR, MACHMODE_GENERATOR, id, song)
		{
			SetEditName("Dupe it!");
			_numPars = NUM_MACHINES*2;
			_nCols = 2;
			bisTicking = false;
			for (int i=0;i<NUM_MACHINES;i++)
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
		
		DuplicatorMac::~DuplicatorMac() throw()
		{
		}
		
		void DuplicatorMac::Init()
		{
			Machine::Init();
			for (int i=0;i<NUM_MACHINES;i++)
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
			for (int i=0;i<NUM_MACHINES;i++)
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
		
		void DuplicatorMac::Tick( int /*channel*/, const PatternEvent & /*pData*/ )
		{
			//const PlayerTimeInfo & timeInfo = callbacks->timeInfo();
		}
		void DuplicatorMac::PreWork(int numSamples)
		{
			Machine::PreWork(numSamples);
			for(; !workEvents.empty(); workEvents.pop_front()) {
			
				WorkEvent & workEvent = workEvents.front();
				if ( !_mute && !bisTicking)
				{
					bisTicking=true;
					for (int i=0;i<NUM_MACHINES;i++)
					{
						if (macOutput[i] != -1 && song()->machine(macOutput[i]) != NULL )
						{
							AllocateVoice(workEvent.track(),i);
							PatternEvent temp = workEvent.event();
							if ( temp.note() < notetypes::release )
							{
								int note = temp.note()+noteOffset[i];
								if ( note>= notetypes::release) note=119;
								else if (note<0 ) note=0;
								temp.setNote(static_cast<std::uint8_t>(note));
							}
							if (song()->machine(macOutput[i]) != this)
							{
								song()->machine(macOutput[i])->AddEvent(workEvent.beatOffset(),workEvent.track(),temp);
								if (temp.note() >= notetypes::release )
								{
									DeallocateVoice(workEvent.track(),i);
								}
							}
						}
					}
				}
				bisTicking=false;
			}
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
					j = (unsigned int) (  (double)rand() * MAX_TRACKS /(((double)RAND_MAX) + 1.0 ));
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
		void DuplicatorMac::GetParamName(int numparam,char *name) const
		{
			if (numparam >=0 && numparam<NUM_MACHINES)
			{
				sprintf(name,"Output Machine %d",numparam);
			} else if (numparam >=NUM_MACHINES && numparam<NUM_MACHINES*2) {
				sprintf(name,"Note Offset %d",numparam-NUM_MACHINES);
			}
			else name[0] = '\0';
		}

		void DuplicatorMac::GetParamRange(int numparam,int &minval,int &maxval) const
		{
			if ( numparam < NUM_MACHINES) { minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if ( numparam < NUM_MACHINES*2) { minval = -48; maxval = 48; }
		}

		int DuplicatorMac::GetParamValue(int numparam) const
		{
			if (numparam >=0 && numparam<NUM_MACHINES)
			{
				return macOutput[numparam];
			} else if (numparam >=NUM_MACHINES && numparam <NUM_MACHINES*2) {
				return noteOffset[numparam-NUM_MACHINES];
			}
			else return 0;
		}

		void DuplicatorMac::GetParamValue(int numparam, char *parVal) const
		{
			if (numparam >=0 && numparam <NUM_MACHINES)
			{
				if ((macOutput[numparam] != -1 ) &&( song()->machine(macOutput[numparam]) != NULL))
				{
					sprintf(parVal,"%X -%s", macOutput[numparam], song()->machine(macOutput[numparam])->GetEditName().c_str());
				}
				else if (macOutput[numparam] != -1) sprintf(parVal,"%X (none)",macOutput[numparam]);
				else sprintf(parVal,"(disabled)");

			} else if (numparam >= NUM_MACHINES && numparam <NUM_MACHINES*2) {
				char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
				sprintf(parVal,"%s%d",notes[(noteOffset[numparam-NUM_MACHINES]+60)%12],(noteOffset[numparam-NUM_MACHINES]+60)/12);
			}
			else parVal[0] = '\0';
		}

		bool DuplicatorMac::SetParameter(int numparam, int value)
		{
			if (numparam >=0 && numparam<NUM_MACHINES)
			{
				macOutput[numparam]=value;
				return true;
			} else if (numparam >=NUM_MACHINES && numparam<NUM_MACHINES*2) {
				noteOffset[numparam-NUM_MACHINES]=value;
				return true;
			}
			else return false;
		}

		int DuplicatorMac::GenerateAudio( int numSamples )
		{
			_worked = true;
			Standby(true);
			return numSamples;
		}

		bool DuplicatorMac::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->ReadArray(macOutput,NUM_MACHINES);
			pFile->ReadArray(noteOffset,NUM_MACHINES);
			return true;
		}

		void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile) const
		{
			std::uint32_t const size(sizeof macOutput + sizeof noteOffset);
			pFile->Write(size);
			pFile->WriteArray(macOutput,NUM_MACHINES);
			pFile->WriteArray(noteOffset,NUM_MACHINES);
		}
		//////////////////////////////////////////////////////////////////////////
		// AudioRecorder


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Master

		std::string Master::_psName = "Master";

		float * Master::_pMasterSamples = 0;

		Master::Master(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song)
		:
			Machine(callbacks, MACH_MASTER, MACHMODE_MASTER, id, song),
			sampleCount(0),
			decreaseOnClip(false),
			_lMax(0),
			_rMax(0),
			_outDry(256)
		{
			SetEditName("Master");
			_outDry = 256;
			SetAudioRange(32768.0f);
			//DefineStereoInput(1);
		}
		
		Master::~Master() throw()
		{
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
		
		void Master::Tick(int /*channel*/, const PatternEvent & data )
		{
			if ( data.note() == commandtypes::SET_VOLUME )
			{
				_outDry = data.parameter();
			}
		}

		int Master::GenerateAudio( int numSamples )
		{
			#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
				universalis::processor::exceptions::fpu::mask fpu_exception_mask(this->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
			#endif

			//cpu::cycles_type cost(cpu::cycles());
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
					if(std::fabs( *pSamples = (*pSamplesL = *pSamplesL * mv)) > _lMax)
					{
						_lMax = fabsf(*pSamplesL);
					}
					pSamples++;
					pSamplesL++;
					// Right channel
					if(std::fabs(*pSamples = (*pSamplesR = *pSamplesR * mv)) > _rMax)
					{
						_rMax = fabsf(*pSamplesR);
					}
					pSamples++;
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
			//cost = cpu::cycles() - cost;
			//work_cpu_cost(work_cpu_cost() + cost);
			_worked = true;
			return numSamples;
		}

		bool Master::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(_outDry);
			pFile->Read(decreaseOnClip);
			return true;
		}

		void Master::SaveSpecificChunk(RiffFile* pFile) const
		{
			std::uint32_t const size(sizeof _outDry + sizeof decreaseOnClip);
			pFile->Write(size);
			pFile->Write(_outDry);
			pFile->Write(decreaseOnClip);
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mixer

		std::string Mixer::_psName = "Mixer";

		Mixer::Mixer(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song )
		:
			Machine(callbacks, MACH_MIXER, MACHMODE_FX, id, song)
		{
			SetEditName("Mixer");
			_numPars = 255;
			SetAudioRange(32768.0f);
			//DefineStereoInput(24);
			//DefineStereoOutput(1);
		}
		
		Mixer::~Mixer() throw()
		{
		}
		
		void Mixer::Init()
		{
			Machine::Init();

			if (inputs_.size() != 0) inputs_.resize(0);
			if (returns_.size() != 0) returns_.resize(0);
			master_.Init();

			solocolumn_=-1;
		}

		void Mixer::Tick( int /*channel*/, const PatternEvent & pData )
		{
			if(pData.note() == notetypes::tweak)
			{
				int nv = (pData.command() << 8)+pData.parameter();
				SetParameter(pData.instrument(), nv);
				Player::Instance()->Tweaker = true;
			}
			else if( pData.note() == notetypes::tweak_slide)
			{
				///\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
				// doing simply "tweak" for now..
				int nv = (pData.command() << 8)+pData.parameter();
				SetParameter(pData.instrument(), nv);
				Player::Instance()->Tweaker = true;
			}
		}

		void Mixer::Work( int numSamples )
		{
			if ( _mute || Bypass())
			{
				WorkWires(numSamples);
				return;
			}

			// Step One, do the usual work, except mixing all the inputs to a single stream.
			WorkWires( numSamples, false );
			// Step Two, prepare input signals for the Send Fx, and make them work
			FxSend( numSamples );
			// Step Three, Mix the returns of the Send Fx's with the leveled input signal
			//cpu::cycles_type cost(cpu::cycles());
			Mix(numSamples);
			dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			Machine::UpdateVuAndStanbyFlag(numSamples);
			//cost = cpu::cycles() - cost;
			
			_worked = true;
			return;
		}

		void Mixer::FxSend(int numSamples )
		{
			//const PlayerTimeInfo & timeInfo = callbacks->timeInfo();

			for (int i=0; i<numsends(); i++)
			{
				if (sends_[i].IsValid())
				{
					Machine* pSendMachine = song()->machine(sends_[i].machine_);
					assert(pSendMachine);
					if (!pSendMachine->_worked && !pSendMachine->_waitingForSound)
					{
						bool soundready=false;
						// Mix all the inputs and route them to the send fx.
						{
							//cpu::cycles_type cost(cpu::cycles());
							if ( solocolumn_ >=0 && solocolumn_ < MAX_CONNECTIONS )
							{
								int j = solocolumn_;
								if (_inputCon[j] && !Channel(j).Mute() && !Channel(j).DryOnly() && (_sendvolpl[j][i] != 0.0f || _sendvolpr[j][i] != 0.0f))
								{
									Machine* pInMachine = song()->machine(_inputMachines[j]);
									assert(pInMachine);
									if(!pInMachine->_mute && !pInMachine->Standby())
									{
										dsp::Add(pInMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pInMachine->lVol()*_sendvolpl[j][i]);
										dsp::Add(pInMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pInMachine->rVol()*_sendvolpr[j][i]);
										soundready=true;
									}
								}
							}
							else for (int j=0; j<numinputs(); j++)
							{
								if (_inputCon[j] && !Channel(j).Mute() && !Channel(j).DryOnly() && (_sendvolpl[j][i] != 0.0f || _sendvolpr[j][i] != 0.0f ))
								{
									Machine* pInMachine = song()->machine(_inputMachines[j]);
									assert(pInMachine);
									if(!pInMachine->_mute && !pInMachine->Standby())
									{
										dsp::Add(pInMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pInMachine->lVol()*_sendvolpl[j][i]);
										dsp::Add(pInMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pInMachine->rVol()*_sendvolpr[j][i]);
										soundready=true;
									}
								}
							}
							for (int j=0; j<i; j++)
							{
								if (Return(j).IsValid() && Return(j).Send(i) && !Return(j).Mute() && (mixvolretpl[j] != 0.0f || mixvolretpr[j] != 0.0f ))
								{
									Machine* pRetMachine = song()->machine(Return(j).Wire().machine_);
									assert(pRetMachine);
									if(!pRetMachine->_mute && !pRetMachine->Standby())
									{
										dsp::Add(pRetMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pRetMachine->lVol()*mixvolretpl[j]);
										dsp::Add(pRetMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pRetMachine->rVol()*mixvolretpr[j]);
										soundready=true;
									}
								}
							}
							if (soundready) pSendMachine->Standby(false);
							//cost = cpu::cycles() - cost;
							//work_cpu_cost(work_cpu_cost() + cost);
						}
	
						// tell the FX to work, now that the input is ready.
						{
							Machine* pRetMachine = song()->machine(Return(i).Wire().machine_);
							pRetMachine->Work(numSamples);
							/// pInMachines are verified in Machine::WorkNoMix, so we only check the returns.
							if(!pRetMachine->Standby())Standby(false);
						}
						///todo: why was this here? It is already done in PreWork()
						#if 0
						{
							//cpu::cycles_type cost = cpu::cycles();
							pSendMachine->_waitingForSound = false;
							dsp::Clear(_pSamplesL, numSamples);
							dsp::Clear(_pSamplesR, numSamples);
							//_cpuCost += cpu::cycles() - cost;
						}
						#endif
					}
				}
			}
		}

		void Mixer::Mix(int numSamples)
		{
			if ( master_.DryWetMix() > 0.0f)
			{
				if ( solocolumn_ >= MAX_CONNECTIONS)
				{
					int i= solocolumn_-MAX_CONNECTIONS;
					if (ReturnValid(i) && !Return(i).Mute() && Return(i).MasterSend() && (mixvolretpl[i] != 0.0f || mixvolretpr[i] != 0.0f ))
					{
						Machine* pRetMachine = song()->machine(Return(i).Wire().machine_);
						assert(pRetMachine);
						if(!pRetMachine->_mute && !pRetMachine->Standby())
						{
							dsp::Add(pRetMachine->_pSamplesL, _pSamplesL, numSamples, pRetMachine->lVol()*mixvolretpl[i]);
							dsp::Add(pRetMachine->_pSamplesR, _pSamplesR, numSamples, pRetMachine->rVol()*mixvolretpr[i]);
						}
					}
				}
				else for (int i=0; i<numreturns(); i++)
				{
					if (Return(i).IsValid() && !Return(i).Mute() && Return(i).MasterSend() && (mixvolretpl[i] != 0.0f || mixvolretpr[i] != 0.0f ))
					{
						Machine* pRetMachine = song()->machine(Return(i).Wire().machine_);
						assert(pRetMachine);
						if(!pRetMachine->_mute && !pRetMachine->Standby())
						{
							dsp::Add(pRetMachine->_pSamplesL, _pSamplesL, numSamples, pRetMachine->lVol()*mixvolretpl[i]);
							dsp::Add(pRetMachine->_pSamplesR, _pSamplesR, numSamples, pRetMachine->rVol()*mixvolretpr[i]);
						}
					}
				}
			}
			if ( master_.DryWetMix() < 1.0f && solocolumn_ < MAX_CONNECTIONS)
			{
				if ( solocolumn_ >= 0)
				{
					int i = solocolumn_;
					if (ChannelValid(i) && !Channel(i).Mute() && !Channel(i).WetOnly() && (mixvolpl[i] != 0.0f || mixvolpr[i] != 0.0f ))
					{
						Machine* pInMachine = song()->machine(_inputMachines[i]);
						assert(pInMachine);
						if(!pInMachine->_mute && !pInMachine->Standby())
						{
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->lVol()*mixvolpl[i]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->rVol()*mixvolpr[i]);
						}
					}
				}
				else for (int i=0; i<numinputs(); i++)
				{
					if (_inputCon[i] && !Channel(i).Mute() && !Channel(i).WetOnly() && (mixvolpl[i] != 0.0f || mixvolpr[i] != 0.0f ))
					{
						Machine* pInMachine = song()->machine(_inputMachines[i]);
						assert(pInMachine);
						if(!pInMachine->_mute && !pInMachine->Standby())
						{
							dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->lVol()*mixvolpl[i]);
							dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->rVol()*mixvolpr[i]);
						}
					}
				}
			}
		}
		void Mixer::InsertInputWire(Machine& srcMac, Wire::id_type dstWire,InPort::id_type dstType, float initialVol) 
		{
			if (dstWire< MAX_CONNECTIONS)
			{
				Machine::InsertInputWire(srcMac,dstWire,dstType,initialVol);
				InsertChannel(dstWire);
				RecalcChannel(dstWire);
				for (int i(0);i<numsends();++i)
				{
					RecalcSend(dstWire,i);
				}
			}
			else
			{
				dstWire-=MAX_CONNECTIONS;
				srcMac.SetMixerSendFlag();
				MixerWire wire(srcMac.id(),0);
				InsertReturn(dstWire,wire);
				InsertSend(dstWire,wire);
				Return(dstWire).Wire().volume_ = initialVol;
				Return(dstWire).Wire().normalize_ = srcMac.GetAudioRange()/GetAudioRange();
				sends_[dstWire].volume_ = 1.0f;
				sends_[dstWire].normalize_ = 1.0f/(srcMac.GetAudioRange()/GetAudioRange());
				RecalcReturn(dstWire);
				for(int c(0) ; c < numinputs() ; ++c)
				{
					RecalcSend(c,dstWire);
				}
			}
		}
		bool Mixer::MoveWireSourceTo(Machine& srcMac, InPort::id_type dsttype, Wire::id_type dstwire, OutPort::id_type srctype)
		{
			if ( dstwire < MAX_CONNECTIONS)
			{
				return Machine::MoveWireSourceTo(srcMac,dsttype,dstwire,srctype);
			}
			if (srctype >= srcMac.GetOutPorts() || dsttype >= GetInPorts())
				return false;
			if (!_inputCon[dstwire])
				return false;
			if (!ReturnValid(dstwire-MAX_CONNECTIONS))
				return false;
			
			Machine* oldSrc = song()->machine(Return(dstwire-MAX_CONNECTIONS).Wire().machine_);
			if (oldSrc)
			{
				Wire::id_type oldwire;
				float volume = 1.0f;
				if ((oldwire =oldSrc->FindOutputWire(id())) == -1)
					return false;
				///\todo: Error srctype may not be the correct type. FindOutputWire should give that info to us.
				oldSrc->DeleteOutputWire(oldwire,srctype);
				srcMac.InsertOutputWire(*this,dstwire,dsttype);
				GetWireVolume(dstwire,volume);
				InsertInputWire(srcMac,dstwire,dsttype,volume);
				return true;
			}
			return false;
		}
		
		void Mixer::ExchangeInputWires(Wire::id_type first,Wire::id_type second, InPort::id_type /*firstType*/, InPort::id_type /*secondType*/)
		{
			// When correctly implemented, we should make use of the InPort Type,
			if ( first  < MAX_CONNECTIONS && second < MAX_CONNECTIONS)
			{
				ExchangeChans(first,second);
			} else if ( first >= MAX_CONNECTIONS && second >= MAX_CONNECTIONS) {
				ExchangeReturns(first-MAX_CONNECTIONS,second-MAX_CONNECTIONS);
			}
		}

		Wire::id_type Mixer::FindInputWire(Machine::id_type macIndex) const
		{
			int ret=Machine::FindInputWire(macIndex);
			if ( ret == -1)
			{
				for (int c=0; c<numreturns(); c++)
				{
					if (Return(c).Wire().machine_ == macIndex)
					{
						ret = c+MAX_CONNECTIONS;
						break;
					}
				}
			}
			return ret;
		}
		Wire::id_type Mixer::GetFreeInputWire(InPort::id_type slotType) const
		{
			if ( slotType == 0) return Machine::GetFreeInputWire(0);
			else 
			{
				// Get a free sendfx slot
				for(int c(0) ; c < MAX_CONNECTIONS ; ++c)
				{
					if(!ReturnValid(c)) return c+MAX_CONNECTIONS;
				}
				return Wire::id_type(-1);
			}
		}

		void Mixer::DeleteInputWire(Wire::id_type wireIndex, InPort::id_type dstType)
		{
			// When correctly implemented, we should make use of the InPort Type,
			if ( wireIndex < MAX_CONNECTIONS)
			{
				Machine::DeleteInputWire(wireIndex,dstType);
				DiscardChannel(wireIndex);
			}
			else
			{
				wireIndex-=MAX_CONNECTIONS;
				song()->machine(Return(wireIndex).Wire().machine_)->ClearMixerSendFlag();
				Return(wireIndex).Wire().machine_=-1;
				sends_[wireIndex].machine_ = -1;
				DiscardReturn(wireIndex);
				DiscardSend(wireIndex);
			}
		}
		void Mixer::NotifyNewSendtoMixer(Machine &caller, Machine& sender)
		{
			// Mixer reached, set flags upwards.
			caller.SetMixerSendFlag();
			for (int i(0); i < MAX_CONNECTIONS; i++)
			{
				if ( ReturnValid(i))
				{
					if (Return(i).Wire().machine_ == caller.id())
					{
						sends_[i].machine_ = sender.id();
						sends_[i].normalize_ = GetAudioRange()/sender.GetAudioRange();
						for (int ch(0);ch<numinputs();ch++)
						{
							RecalcSend(ch,i);
						}
					}
				}
			}
		}


		void Mixer::DeleteWires()
		{
			Machine::DeleteWires();
			Machine *iMac;
			for(int w=0; w<numreturns(); w++)
			{
				// Checking send/return Wires
				if(Return(w).IsValid())
				{
					iMac = song()->machine(Return(w).Wire().machine_);
					if (iMac)
					{
						int wix = iMac->FindOutputWire(id());
						if (wix >=0)
						{
							///\todo: get the correct port type.
							iMac->DeleteOutputWire(wix,0);
						}
					}
					///\todo: get the correct port type.
					DeleteInputWire(w+MAX_CONNECTIONS,0);
				}
			}
		}
		float Mixer::GetWireVolume(Wire::id_type wireIndex) const
		{
			if (wireIndex< MAX_CONNECTIONS)
				return Machine::GetWireVolume(wireIndex);
			else if ( ReturnValid(wireIndex-MAX_CONNECTIONS) )
				return Return(wireIndex-MAX_CONNECTIONS).Wire().volume_;
			return 0;
		}
		void Mixer::SetWireVolume(Wire::id_type wireIndex,float value)
		{
			if (wireIndex < MAX_CONNECTIONS)
			{
				Machine::SetWireVolume(wireIndex,value);
				if (ChannelValid(wireIndex))
				{
					RecalcChannel(wireIndex);
				}
			}
			else if (ReturnValid(wireIndex-MAX_CONNECTIONS))
			{
				Return(wireIndex-MAX_CONNECTIONS).Wire().volume_ = value;
				RecalcReturn(wireIndex-MAX_CONNECTIONS);
			}
		}
		std::string Mixer::GetAudioInputName(Wire::id_type wire) const
		{
			std::string rettxt;
			if (wire < chanmax)
			{
				int i = wire-chan1;
				rettxt = "Input ";
				if ( i < 9 ) rettxt += ('1'+i);
				else { rettxt += '1'; rettxt += ('0'+i-9); }
				return rettxt;
			}
			else if ( wire < returnmax)
			{
				int i = wire-return1;
				rettxt = "Return ";
				if ( i < 9 ) rettxt += ('1'+i);
				else { rettxt += '1'; rettxt += ('0'+i-9); }
				return rettxt;
			}
			rettxt = "-";
			return rettxt;
		}
		std::string Mixer:: GetPortInputName(InPort::id_type port) const
		{
			return (port==0)?"Input Port":"Send/Return Port";
		}
		
		int Mixer::GetNumCols() const
		{
			return 2+numinputs()+numreturns();
		}
		void Mixer::GetParamRange(int numparam, int &minval, int &maxval) const
		{
			int channel=numparam/16;
			int param=numparam%16;
			if ( channel == 0)
			{
				if (param == 0){ minval=0; maxval=0x1000; }
				else if (param <= 12)  {
					if (!ChannelValid(param-1)) { minval=0; maxval=0; }
					else { minval=0; maxval=0x1000; }
				}
				else if (param == 13) { minval=0; maxval=0x100; }
				else if (param == 14) { minval=0; maxval=0x400; }
				else  { minval=0; maxval=0x100; }
			}
			else if (channel <= 12 )
			{
				if (!ChannelValid(channel-1)) { minval=0; maxval=0; }
				else if (param == 0) { minval=0; maxval=0x100; }
				else if (param <= 12) {
					if(!ReturnValid(param-1)) { minval=0; maxval=0; }
					else { minval=0; maxval=0x100; }
				}
				else if (param == 13) { minval=0; maxval=3; }
				else if (param == 14) { minval=0; maxval=0x400; }
				else  { minval=0; maxval=0x100; }
			}
			else if ( channel == 13)
			{
				if ( param > 12) { minval=0; maxval=0; }
				else if ( param == 0 ) { minval=0; maxval=24; }
				else if (!ReturnValid(param-1)) { minval=0; maxval=0; }
				else { minval=0; maxval=(1<<14)-1; }
			}
			else if ( channel == 14)
			{
				if ( param == 0 || param > 12) { minval=0; maxval=0; }
				else if (!ReturnValid(param-1)) { minval=0; maxval=0; }
				else { minval=0; maxval=0x1000; }
			}
			else if ( channel == 15)
			{
				if ( param == 0 || param > 12) { minval=0; maxval=0; }
				else if (!ReturnValid(param-1)) { minval=0; maxval=0; }
				else { minval=0; maxval=0x100; }
			}
			else { minval=0; maxval=0; }
		}
		void Mixer::GetParamName(int numparam,char *name) const
		{
			int channel=numparam/16;
			int param=numparam%16;
			if ( channel == 0)
			{
				if (param == 0) strcpy(name,"Master - Output");
				else if (param <= 12) sprintf(name,"Channel %d - Volume",param);
				else if (param == 13) strcpy(name,"Master - Mix");
				else if (param == 14) strcpy(name,"Master - Gain");
				else strcpy(name,"Master - Panning");
			}
			else if (channel <= 12 )
			{
				if (param == 0) sprintf(name,"Channel %d - Dry mix",channel);
				else if (param <= 12) sprintf(name,"Chan %d Send %d - Amount",channel,param);
				else if (param == 13) sprintf(name,"Channel %d  - Mix type",channel);
				else if (param == 14) sprintf(name,"Channel %d - Gain",channel);
				else sprintf(name,"Channel %d - Panning",channel);
			}
			else if ( channel == 13)
			{
				if (param > 12) strcpy(name,"");
				else if (param == 0) strcpy(name,"Set Channel Solo");
				else sprintf(name,"Return %d - Route Array",param);
			}
			else if ( channel == 14)
			{
				if ( param == 0 || param > 12) strcpy(name,"");
				else sprintf(name,"Return %d - Volume",param);
			}
			else if ( channel == 15)
			{
				if ( param == 0 || param > 12) strcpy(name,"");
				else sprintf(name,"Return %d - Panning",param);
			}
			else strcpy(name,"");
		}

		int Mixer::GetParamValue(int numparam) const
		{
			int channel=numparam/16;
			int param=numparam%16;
			if ( channel == 0)
			{
				if (param == 0)
				{
					float dbs = dsp::dB(master_.Volume());
					return (dbs+96.0f)*42.67; // *(0x1000 / 96.0f)
				}
				else if (param <= 12)
				{
					if (!ChannelValid(param-1)) return 0;
					else {
						float dbs = dsp::dB(Channel(param-1).Volume());
						return (dbs+96.0f)*42.67; // *(0x1000 / 96.0f)
					}
				}
				else if (param == 13) return master_.DryWetMix()*0x100;
				else if (param == 14) return master_.Gain()*0x100;
				else return _panning*2;
			}
			else if (channel <= 12 )
			{
				if (!ChannelValid(channel-1)) return 0;
				else if (param == 0) return Channel(channel-1).DryMix()*0x100;
				else if (param <= 12) 
				{
					if ( !ReturnValid(param-1)) return 0;
					else return Channel(channel-1).Send(param-1)*0x100;
				}
				else if (param == 13)
				{
					if (Channel(channel-1).Mute()) return 3;
					else if (Channel(channel-1).WetOnly()) return 2;
					else if (Channel(channel-1).DryOnly()) return 1;
					return 0;
				}
				else if (param == 14) { float val; GetWireVolume(channel-1,val); return val*0x100; }
				else return Channel(channel-1).Panning()*0x100;
			}
			else if ( channel == 13)
			{
				if ( param > 12) return 0;
				else if (param == 0 ) return solocolumn_+1;
				else if ( !ReturnValid(param-1)) return 0;
				else
				{
					int val(0);
					if (Return(param-1).Mute()) val|=1;
					for (int i(0);i<numreturns();i++)
					{
						if (Return(param-1).Send(i)) val|=(2<<i);
					}
					if (Return(param-1).MasterSend()) val|=(1<<13);
					return val;
				}
			}
			else if ( channel == 14)
			{
				if ( param == 0 || param > 12) return 0;
				else if ( !ReturnValid(param-1)) return 0;
				else
				{
					float dbs = dsp::dB(Return(param-1).Volume());
					return (dbs+96.0f)*42.67; // *(0x1000 / 96.0f)
				}
			}
			else if ( channel == 15)
			{
				if ( param == 0 || param > 12) return 0;
				else if ( !ReturnValid(param-1)) return 0;
				else return Return(param-1).Panning()*0x100;
			}
			else return 0;
		}

		void Mixer::GetParamValue(int numparam, char *parVal) const
		{
			int channel=numparam/16;
			int param=numparam%16;
			parVal[0]='\0';
			if ( channel == 0)
			{
				if (param == 0)
				{ 
					if (master_.Volume() < 0.00002f ) strcpy(parVal,"-inf");
					else
					{
						float dbs = dsp::dB(master_.Volume());
						sprintf(parVal,"%.01fdB",dbs);
					}
				}
				else if (param <= 12)
				{
					if (!ChannelValid(param-1)) return;
					else if (Channel(param-1).Volume() < 0.00002f ) strcpy(parVal,"-inf");
					else
					{
						float dbs = dsp::dB(Channel(param-1).Volume());
						sprintf(parVal,"%.01fdB",dbs);
					}
				}
				else if (param == 13)
				{
					if (master_.DryWetMix() == 0.0f) strcpy(parVal,"Dry");
					else if (master_.DryWetMix() == 1.0f) strcpy(parVal,"Wet");
					else sprintf(parVal,"%.0f%%",master_.DryWetMix()*100.0f);
				}
				else if (param == 14)
				{
					float val = master_.Gain();
					float dbs = (((val>0.0f)?dsp::dB(val):-100.0f));
					sprintf(parVal,"%.01fdB",dbs);
				}
				else 
				{
					if (_panning == 0) strcpy(parVal,"left");
					else if (_panning == 128) strcpy(parVal,"right");
					else if (_panning == 64) strcpy(parVal,"center");
					else sprintf(parVal,"%.0f%%",_panning*0.78125f);
				}
			}
			else if (channel <= 12 )
			{
				if (!ChannelValid(channel-1)) return;
				else if (param == 0)
				{
					if (Channel(channel-1).DryMix() == 0.0f) strcpy(parVal,"Off");
					else sprintf(parVal,"%.0f%%",Channel(channel-1).DryMix()*100.0f);
				}
				else if (param <= 12)
				{
					if ( !ReturnValid(param-1)) return;
					else if (Channel(channel-1).Send(param-1) == 0.0f) strcpy(parVal,"Off");
					else sprintf(parVal,"%.0f%%",Channel(channel-1).Send(param-1)*100.0f);
				}
				else if (param == 13)
				{
					parVal[0]= (Channel(channel-1).DryOnly())?'D':' ';
					parVal[1]= (Channel(channel-1).WetOnly())?'W':' ';
					parVal[2]= (Channel(channel-1).Mute())?'M':' ';
					parVal[3]='\0';
				}
				else if (param == 14) 
				{
					float val;
					GetWireVolume(channel-1,val);
					float dbs = (((val>0.0f)?dsp::dB(val):-100.0f));
					sprintf(parVal,"%.01fdB",dbs);
				}
				else
				{
					if (Channel(channel-1).Panning()== 0.0f) strcpy(parVal,"left");
					else if (Channel(channel-1).Panning()== 1.0f) strcpy(parVal,"right");
					else if (Channel(channel-1).Panning()== 0.5f) strcpy(parVal,"center");
					else sprintf(parVal,"%.0f%%",Channel(channel-1).Panning()*100.0f);
				}
			}
			else if ( channel == 13)
			{
				if ( param > 12) return;
				else if (param == 0 ){ sprintf(parVal,"%d",solocolumn_+1); }
				else if ( !ReturnValid(param-1))  return;
				else
				{
					parVal[0]= (Return(param-1).Mute())?'M':' ';
					parVal[1]= (Return(param-1).Send(0))?'1':' ';
					parVal[2]= (Return(param-1).Send(1))?'2':' ';
					parVal[3]= (Return(param-1).Send(2))?'3':' ';
					parVal[4]= (Return(param-1).Send(3))?'4':' ';
					parVal[5]= (Return(param-1).Send(4))?'5':' ';
					parVal[6]= (Return(param-1).Send(5))?'6':' ';
					parVal[7]= (Return(param-1).Send(6))?'7':' ';
					parVal[8]= (Return(param-1).Send(7))?'8':' ';
					parVal[9]= (Return(param-1).Send(8))?'9':' ';
					parVal[10]= (Return(param-1).Send(9))?'A':' ';
					parVal[11]= (Return(param-1).Send(10))?'B':' ';
					parVal[12]= (Return(param-1).Send(11))?'C':' ';
					parVal[13]= (Return(param-1).MasterSend())?'O':' ';
					parVal[14]= '\0';
				}
			}
			else if ( channel == 14)
			{
				if ( param == 0 || param > 12) return;
				else if ( !ReturnValid(param-1)) return;
				else if (Return(param-1).Volume() < 0.00002f ) strcpy(parVal,"-inf");
				else
				{
					float dbs = dsp::dB(Return(param-1).Volume());
					sprintf(parVal,"%.01fdB",dbs);
				}
			}
			else if ( channel == 15)
			{
				if ( param == 0 || param > 12) return;
				else if ( !ReturnValid(param-1)) return;
				else
				{
					if (Return(param-1).Panning()== 0.0f) strcpy(parVal,"left");
					else if (Return(param-1).Panning()== 1.0f) strcpy(parVal,"right");
					else if (Return(param-1).Panning()== 0.5f) strcpy(parVal,"center");
					else sprintf(parVal,"%.0f%%",Return(param-1).Panning()*100.0f);
				}
			}
		}

		bool Mixer::SetParameter(int numparam, int value)
		{
			int channel=numparam/16;
			int param=numparam%16;
			if ( channel == 0)
			{
				if (param == 0)
				{
					if ( value >= 0x1000) master_.Volume()=1.0f;
					else if ( value == 0) master_.Volume()=0.0f;
					else
					{
						float dbs = (value/42.67f)-96.0f;
						master_.Volume() = dsp::dB2Amp(dbs);
					}
					RecalcMaster();
				}
				else if (param <= 12)
				{
					if (!ChannelValid(param-1)) return false;
					else if ( value >= 0x1000) Channel(param-1).Volume()=1.0f;
					else if ( value == 0) Channel(param-1).Volume()=0.0f;
					else
					{
						float dbs = (value/42.67f)-96.0f;
						Channel(param-1).Volume() = dsp::dB2Amp(dbs);
					}
					RecalcChannel(param-1);
				}
				else if (param == 13) { master_.DryWetMix() = (value==256)?1.0f:((value&0xFF)/256.0f); RecalcMaster(); }
				else if (param == 14) { master_.Gain() = (value>=1024)?4.0f:((value&0x3FF)/256.0f); RecalcMaster(); }
				else SetPan(value>>1);
				return true;
			}
			else if (channel <= 12 )
			{
				if (!ChannelValid(channel-1)) return false;
				if (param == 0) { Channel(channel-1).DryMix() = (value==256)?1.0f:((value&0xFF)/256.0f); RecalcChannel(channel-1); }
				else if (param <= 12) { Channel(channel-1).Send(param-1) = (value==256)?1.0f:((value&0xFF)/256.0f); RecalcSend(channel-1,param-1); } 
				else if (param == 13)
				{
					Channel(channel-1).Mute() = (value == 3)?true:false;
					Channel(channel-1).WetOnly() = (value==2)?true:false;
					Channel(channel-1).DryOnly() = (value==1)?true:false;
				}
				else if (param == 14) { float val=(value>=1024)?4.0f:((value&0x3FF)/256.0f); SetWireVolume(channel-1,val); RecalcChannel(channel-1); }
				else { Channel(channel-1).Panning() = (value==256)?1.0f:((value&0xFF)/256.0f); RecalcChannel(channel-1); }
				return true;
			}
			else if ( channel == 13)
			{
				if ( param > 12) return false;
				else if (param == 0) solocolumn_ = (value<24)?value-1:23;
				else if (!ReturnValid(param-1)) return false;
				else 
				{
					Return(param-1).Mute() = (value&1)?true:false;
					for (int i(param);i<numreturns();i++)
					{
						Return(param-1).Send(i,(value&(2<<i))?true:false);
					}
					Return(param-1).MasterSend() = (value&(1<<13))?true:false;
				}
				return true;
			}
			else if ( channel == 14)
			{
				if ( param == 0 || param > 12) return false;
				else if (!ReturnValid(param-1)) return false;
				else
				{
					if ( value >= 0x1000) Return(param-1).Volume()=1.0f;
					else if ( value == 0) Return(param-1).Volume()=0.0f;
					else
					{
						float dbs = (value/42.67f)-96.0f;
						Return(param-1).Volume() = dsp::dB2Amp(dbs);
					}
					RecalcReturn(param-1);
				}
				return true;
			}
			else if ( channel == 15)
			{
				if ( param == 0 || param > 12) return false;
				else if (!ReturnValid(param-1)) return false;
				else { Return(param-1).Panning() = (value==256)?1.0f:((value&0xFF)/256.0f); RecalcReturn(param-1); }
				return true;
			}
			return false;
		}



		float Mixer::VuChan(Wire::id_type idx) const
		{
			if ( _inputCon[idx] ) 
			{
				float vol;
				GetWireVolume(idx,vol);
				vol*=Channel(idx).Volume();
				///\todo: DANG! _volumeDisplay is not proportional to the volume, so this doesn't work as expected
				return (song()->machine(_inputMachines[idx])->_volumeDisplay/97.0f)*vol;
			}
			return 0.0f;
		}

		float Mixer::VuSend(Wire::id_type idx) const
		{
			if ( SendValid(idx) )
			{
				float vol;
				GetWireVolume(idx+MAX_CONNECTIONS,vol);
				vol *= Return(idx).Volume();
				///\todo: DANG! _volumeDisplay is not proportional to the volume, so this doesn't work as expected
				return (song()->machine(Return(idx).Wire().machine_)->_volumeDisplay/97.0f)*vol;
			}
			return 0.0f;
		}
		void Mixer::InsertChannel(int idx,InputChannel*input)
		{
			assert(idx<MAX_CONNECTIONS);
			if ( idx >= numinputs())
			{
				for(int i=numinputs(); i<idx; ++i)
				{
					inputs_.push_back(InputChannel(numsends()));
				}
				if (input) inputs_.push_back(*input);
				else inputs_.push_back(InputChannel(numsends()));
			}
			else if (input) inputs_[idx]=*input;
			else { inputs_[idx].Init(); inputs_[idx].ResizeTo(numsends()); }
		}
		void Mixer::InsertReturn(int idx,ReturnChannel* retchan)
		{
			assert(idx<MAX_CONNECTIONS);
			if ( idx >= numreturns())
			{
				for(int i=numreturns(); i<idx; ++i)
				{
					returns_.push_back(ReturnChannel(numsends()));
				}
				if (retchan) returns_.push_back(*retchan);
				else returns_.push_back(ReturnChannel(numsends()));
				for(int i=0; i<numinputs(); ++i)
				{
					Channel(i).ResizeTo(numsends());
				}
				for(int i=0; i<numreturns(); ++i)
				{
					Return(i).ResizeTo(numsends());
				}
			}
			else if (retchan) returns_[idx]=*retchan;
			else { returns_[idx].Init(); returns_[idx].ResizeTo(numsends());}
		}

		void Mixer::InsertSend(int idx,MixerWire swire)
		{
			assert(idx<MAX_CONNECTIONS);
			if ( idx >= numsends())
			{
				for(int i=numsends(); i<idx; ++i)
				{
					sends_.push_back(MixerWire());
				}
				sends_.push_back(swire);
			}
			else sends_[idx]=swire;
			for(int i=0; i<numinputs(); ++i)
			{
				Channel(i).ResizeTo(numsends());
			}
			for(int i=0; i<numreturns(); ++i)
			{
				Return(i).ResizeTo(numsends());
			}
		}
		void Mixer::DiscardChannel(int idx)
		{
			assert(idx<MAX_CONNECTIONS);
			if (idx!=numinputs()-1) return;
			int i;
			for (i = idx; i >= 0; i--)
			{
				if (_inputCon[i])
					break;
			}
			inputs_.resize(i+1);
		}
		void Mixer::DiscardReturn(int idx)
		{
			assert(idx<MAX_CONNECTIONS);
			if (idx!=numreturns()-1) return;
			int i;
			for (i = idx; i >= 0; i--)
			{
				if (Return(i).IsValid())
					break;
			}
			returns_.resize(i+1);
		}
		void Mixer::DiscardSend(int idx)
		{
			assert(idx<MAX_CONNECTIONS);
			if (idx!=numsends()-1) return;
			int i;
			for (i = idx; i >= 0; i--)
			{
				if (sends_[i].machine_ != -1)
					break;
			}
			sends_.resize(i+1);
		}


		void Mixer::ExchangeChans(int chann1,int chann2)
		{
			Machine::ExchangeInputWires(chann1,chann2);
			InputChannel tmp = inputs_[chann1];
			inputs_[chann1] = inputs_[chann2];
			inputs_[chann2] = tmp;
			RecalcChannel(chann1);
			RecalcChannel(chann2);

		}
		void Mixer::ExchangeReturns(int chann1,int chann2)
		{
			ReturnChannel tmp = returns_[chann1];
			returns_[chann1] = returns_[chann2];
			returns_[chann2] = tmp;
			RecalcReturn(chann1);
			RecalcReturn(chann2);
			ExchangeSends(chann1,chann2);
		}
		void Mixer::ExchangeSends(int send1,int send2)
		{
			MixerWire tmp = sends_[send1];
			sends_[send1] = sends_[send2];
			sends_[send2] = tmp;
			for (int i(0); i < numinputs(); ++i)
			{
				Channel(i).ExchangeSends(send1,send2);
				RecalcSend(i,send1);
				RecalcSend(i,send2);
			}
			for (int i(0); i < numreturns(); ++i)
			{
				Return(i).ExchangeSends(send1,send2);
			}
		}
		void Mixer::ResizeTo(int inputs, int sends)
		{
			inputs_.resize(inputs);
			returns_.resize(sends);
			sends_.resize(sends);
			for(int i=0; i<numinputs(); ++i)
			{
				Channel(i).ResizeTo(numsends());
			}
			for(int i=0; i<numreturns(); ++i)
			{
				Return(i).ResizeTo(numsends());
			}
		}
		void Mixer::RecalcMaster()
		{
			for (int i(0);i<numinputs();i++)
			{
				if (_inputCon[i]) RecalcChannel(i);
			}
			for (int i(0);i<numreturns();i++)
			{
				if (Return(i).IsValid()) RecalcReturn(i);
			}
		}
		void Mixer::RecalcReturn(int idx)
		{
			assert(idx<MAX_CONNECTIONS);
			float val;
			GetWireVolume(idx,val);

			float wet = master_.Volume()*master_.Gain();
			if (master_.DryWetMix() < 0.5f )
			{
				wet *= (master_.DryWetMix())*2.0f;
			}

			mixvolretpl[idx] = mixvolretpr[idx] = Return(idx).Volume()*val*wet/Return(idx).Wire().normalize_;
			if (Return(idx).Panning() >= 0.5f )
			{
				mixvolretpl[idx] *= (1.0f-Return(idx).Panning())*2.0f;
			}
			else mixvolretpr[idx] *= (Return(idx).Panning())*2.0f;
		}
		void Mixer::RecalcChannel(int idx)
		{
			assert(idx<MAX_CONNECTIONS);
			float val;
			GetWireVolume(idx,val);

			float dry = master_.Volume()*master_.Gain();
			if (master_.DryWetMix() > 0.5f )
			{
				dry *= (1.0f-master_.DryWetMix())*2.0f;
			}

			mixvolpl[idx] = mixvolpr[idx] = Channel(idx).Volume()*val*Channel(idx).DryMix()*dry/_wireMultiplier[idx];
			if (Channel(idx).Panning() >= 0.5f )
			{
				mixvolpl[idx] *= (1.0f-Channel(idx).Panning())*2.0f;
			}
			else mixvolpr[idx] *= (Channel(idx).Panning())*2.0f;
			for (int i(0);i<numsends();i++) RecalcSend(idx,i);
		}
		void Mixer::RecalcSend(int chan,int send)
		{
			assert(chan<MAX_CONNECTIONS);
			assert(send<MAX_CONNECTIONS);
			float val;
			GetWireVolume(chan,val);

			_sendvolpl[chan][send] =  _sendvolpr[chan][send] = Channel(chan).Volume()*val*Channel(chan).Send(send)/(sends_[send].normalize_*_wireMultiplier[chan]);
			if (Channel(chan).Panning() >= 0.5f )
			{
				_sendvolpl[chan][send] *= (1.0f-Channel(chan).Panning())*2.0f;
			}
			else _sendvolpr[chan][send] *= (Channel(chan).Panning())*2.0f;
		}
		bool Mixer::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
		{
			std::uint32_t filesize;
			pFile->Read(filesize);

			pFile->Read(solocolumn_);
			pFile->Read(master_.Volume());
			pFile->Read(master_.Gain());
			pFile->Read(master_.DryWetMix());

			int numins(0),numrets(0);
			pFile->Read(numins);
			pFile->Read(numrets);
			if ( numins >0 ) InsertChannel(numins-1);
			if ( numrets >0 ) InsertReturn(numrets-1);
			if ( numrets >0 ) InsertSend(numrets-1,MixerWire());
			for (int i(0);i<numinputs();i++)
			{
				for (int j(0);j<numsends();j++)
				{
					float send(0.0f);
					pFile->Read(send);
					Channel(i).Send(j)=send;
				}
				pFile->Read(Channel(i).Volume());
				pFile->Read(Channel(i).Panning());
				pFile->Read(Channel(i).DryMix());
				pFile->Read(Channel(i).Mute());
				pFile->Read(Channel(i).DryOnly());
				pFile->Read(Channel(i).WetOnly());
			}
			for (int i(0);i<numreturns();i++)
			{
				pFile->Read(Return(i).Wire().machine_);
				pFile->Read(Return(i).Wire().volume_);
				pFile->Read(Return(i).Wire().normalize_);
				pFile->Read(sends_[i].machine_);
				pFile->Read(sends_[i].volume_);
				pFile->Read(sends_[i].normalize_);
				for (int j(0);j<numsends();j++)
				{
					bool send(false);
					pFile->Read(send);
					Return(i).Send(j,send);
				}
				pFile->Read(Return(i).MasterSend());
				pFile->Read(Return(i).Volume());
				pFile->Read(Return(i).Panning());
				pFile->Read(Return(i).Mute());
			}
			RecalcMaster();
			for (int i(0);i<numinputs();i++)
				for(int j(0);j<numsends();j++)
					RecalcSend(i,j);
			return true;
		}

		void Mixer::SaveSpecificChunk(RiffFile* pFile) const
		{
			std::uint32_t size(sizeof(solocolumn_)+sizeof(master_)+2*sizeof(int));
			size+=(3*sizeof(float)+3*sizeof(bool)+numsends()*sizeof(float))*numinputs();
			size+=(2*sizeof(float)+2*sizeof(bool)+numsends()*sizeof(bool)+2*sizeof(float)+sizeof(int))*numreturns();
			size+=(2*sizeof(float)+sizeof(int))*numsends();
			pFile->Write(size);

			pFile->Write(solocolumn_);
			pFile->Write(master_.Volume());
			pFile->Write(master_.Gain());
			pFile->Write(master_.DryWetMix());

			const int numins = numinputs();
			const int numrets = numreturns();
			pFile->Write(numins);
			pFile->Write(numrets);
			for (int i(0);i<numinputs();i++)
			{
				for (int j(0);j<numsends();j++)
				{
					pFile->Write(Channel(i).Send(j));
				}
				pFile->Write(Channel(i).Volume());
				pFile->Write(Channel(i).Panning());
				pFile->Write(Channel(i).DryMix());
				pFile->Write(Channel(i).Mute());
				pFile->Write(Channel(i).DryOnly());
				pFile->Write(Channel(i).WetOnly());
			}
			for (int i(0);i<numreturns();i++)
			{
				pFile->Write(Return(i).Wire().machine_);
				pFile->Write(Return(i).Wire().volume_);
				pFile->Write(Return(i).Wire().normalize_);
				pFile->Write(sends_[i].machine_);
				pFile->Write(sends_[i].volume_);
				pFile->Write(sends_[i].normalize_);
				for (int j(0);j<numsends();j++)
				{
					bool send(Return(i).Send(j));
					pFile->Write(send);
				}
				pFile->Write(Return(i).MasterSend());
				pFile->Write(Return(i).Volume());
				pFile->Write(Return(i).Panning());
				pFile->Write(Return(i).Mute());
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LFO

		// todo:
		// - as is, control rate is proportional to MAX_BUFFER_LENGTH.. we update in work, which (at the moment) means once every 256
		//   samples. at 44k, this means a cr of 142hz.  this is probably good enough for most purposes, but i believe it also
		//   means that the lfo can and likely will be phased by 5.8ms depending on where it is placed in the machine view..
		//   if we want to take the idea of modulation machines much further, we should probably put together some kind of
		//   standard place in the processing chain where these machines will work, preferably -before- any audio
		//   <JosepMa> About the "before any audio", the player can support this right now in two different ways:
		//   One is in the "Machine::preWork" function, currently only used for buffer cleanup and generation of the wire visual data.
		//   The second one is in the "Player::NotifyNewLine" function or in "Player::ExecuteGlobalCommands"
		//   Also, note that currently, work does NOT mean 256 samples. It means *at much* 256, and quite frequently, it is a smaller
		//   value (each line). This will change with the event based player.
		//   processing.  this should also eliminate the need for the lfo to be connected to something to work.
		//  - respond to pulse width knob.. consider using it as a 'skew' control for sine/tri waves as in dw-tremolo?
		//  - now that we have a gui, keeping the 'position' display knob as an un-controllable control is just silly
		//  - prettify gui
		//  - vst support??

		std::string LFO::_psName = "LFO";

#if 0 // don't worry, msvc is the weird
		int const LFO::LFO_SIZE;
		int const LFO::MAX_PHASE;
		int const LFO::MAX_SPEED;
		int const LFO::MAX_DEPTH;
		int const LFO::NUM_CHANS;
#endif


		LFO::LFO(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song)
		:
			Machine(callbacks, MACH_LFO, MACHMODE_GENERATOR, id, song)
		{
			SetEditName("LFO");
			_numPars = prms::num_params;
			_nCols = 3;
			bisTicking = false;
		}

		LFO::~LFO() throw()
		{
		}

		void LFO::Init()
		{
			Machine::Init();
			for (int i=0;i<NUM_CHANS;i++)
			{
				macOutput[i]=-1;
				paramOutput[i]=-1;
				level[i]=100;
				phase[i]= static_cast<int>( MAX_PHASE / 2.0f );
				prevVal[i]   = 0;
								centerVal[i] = 0;
			}
			lfoPos = 0.0;
			lSpeed= MAX_SPEED / 10;
			waveform = lfo_types::sine;
			FillTable();
		}

		void LFO::Tick( int /*channel*/, const PatternEvent & pData )
		{
			if(!bisTicking) {
				bisTicking = true;
				// 0x01.. seems appropriate for a machine with exactly one command, but if this goes
				// against any established practices or something, let me know
				if(pData.command() == 0x01) lfoPos = 0;
			}
			bisTicking = false;
		}

		void LFO::GetParamName(int numparam,char *name) const
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

		void LFO::GetParamRange(int numparam,int &minval,int &maxval) const
		{
			if(numparam==prms::wave) { minval = 0; maxval = lfo_types::num_lfos-1;}
			else if (numparam==prms::speed) {minval = 0; maxval = MAX_SPEED;}
			else if (numparam <prms::prm0) {minval = -1; maxval = (MAX_BUSES*2)-1;}
			else if (numparam <prms::level0)
			{
				minval = -1;
				if(macOutput[numparam-prms::prm0]==-1 || song()->machine(macOutput[numparam-prms::prm0]) == NULL)
					maxval = -1;
				else
					maxval =  song()->machine(macOutput[numparam-prms::prm0])->GetNumParams()-1;
			}

			else if (numparam <prms::phase0){minval = 0; maxval = MAX_DEPTH*2;}
			else if (numparam <prms::num_params){minval = 0; maxval = MAX_PHASE; }
			else {minval=0;maxval=0; }

		}

		int LFO::GetParamValue(int numparam) const
		{
			if(numparam==prms::wave) return waveform;
			else if(numparam==prms::speed) return lSpeed;
			else if(numparam <prms::prm0) return macOutput[numparam-prms::mac0];
			else if(numparam <prms::level0) return paramOutput[numparam-prms::prm0];
			else if(numparam <prms::phase0) return level[numparam-prms::level0];
			else if(numparam <prms::num_params) return phase[numparam-prms::phase0];
			else return 0;
		}

		void LFO::GetParamValue(int numparam, char *parVal) const
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
				if ((macOutput[numparam-prms::mac0] != -1 ) &&( song()->machine(macOutput[numparam-prms::mac0]) != NULL))
					sprintf(parVal,"%X -%s",macOutput[numparam-prms::mac0],song()->machine(macOutput[numparam-prms::mac0])->GetEditName().c_str());
				else if (macOutput[numparam-prms::mac0] != -1)
					sprintf(parVal,"%X (none)",macOutput[numparam-prms::mac0]);
				else 
					sprintf(parVal,"(disabled)");
			}
			else if(numparam<prms::level0)
			{
				if((macOutput[numparam-prms::prm0] != -1) 
					&& (song()->machine(macOutput[numparam-prms::prm0]) != NULL)
					&& (paramOutput[numparam-prms::prm0] >= 0))
				{
					if(paramOutput[numparam-prms::prm0] < song()->machine(macOutput[numparam-prms::prm0])->GetNumParams())
					{
						char name[128];
						song()->machine(macOutput[numparam-prms::prm0])->GetParamName(paramOutput[numparam-prms::prm0], name);
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
						for(newMac=value; newMac<MAX_MACHINES && song()->machine(newMac)==NULL; ++newMac)
							;
					}
					//if we're decreasing, or if we're increasing but didn't find anything, decrease until we find an active machine
					if(value<macOutput[numparam-prms::mac0] || newMac>=MAX_MACHINES)
					{
						for(newMac=value;newMac>-1 && song()->machine(newMac)==NULL; --newMac)
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
				if( macOutput[numparam-prms::prm0]>-1 && song()->machine(macOutput[numparam-prms::prm0]) )
				{
					if(value<song()->machine(macOutput[numparam-prms::prm0])->GetNumParams())
					{
						ParamEnd(numparam-prms::prm0);
						paramOutput[numparam-prms::prm0] = value;
						ParamStart(numparam-prms::prm0);
					}
					else
					{
						ParamEnd(numparam-prms::prm0);
						paramOutput[numparam-prms::prm0] = song()->machine(macOutput[numparam-prms::prm0])->GetNumParams()-1;
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
			//cpu::cycles_type cost(cpu::cycles());

			int maxVal=0, minVal=0;
			int curVal=0, newVal=0;
			float curLFO=0.0;
			float lfoAmt=0.0;
			bool bRedraw=false;

			for(int j(0);j<NUM_CHANS;++j)
			{
				if( macOutput[j] != -1 && song()->machine(macOutput[j]) != NULL &&
					paramOutput[j] != -1 && paramOutput[j] < song()->machine(macOutput[j])->GetNumParams() )
				{
					song()->machine(macOutput[j])->GetParamRange(paramOutput[j], minVal, maxVal);
					curVal = song()->machine(macOutput[j])->GetParamValue(paramOutput[j]);
					curLFO = waveTable[int(lfoPos+phase[j]+(MAX_PHASE/2.0f)) % LFO_SIZE];
					lfoAmt = (level[j]-MAX_DEPTH)/(float)MAX_DEPTH;

					centerVal[j] -= prevVal[j] - curVal; // compensate for external movement

					newVal = (int) (curLFO * ((maxVal-minVal)/2.0f) * lfoAmt + centerVal[j]);

					if(newVal>maxVal) newVal=maxVal;
					else if(newVal<minVal) newVal=minVal;

					song()->machine(macOutput[j])->SetParameter(paramOutput[j], newVal); // make it happen!
					bRedraw=true;
					prevVal[j] = newVal;
				}
			}

			if(bRedraw)
				Player::Instance()->Tweaker=true;
			bRedraw=false;

			float minms = callbacks->timeInfo().sampleRate() /1000.0f * 100.0f; // 100ms in samples
			lfoPos += (lSpeed/ float(MAX_SPEED)) * (LFO_SIZE/float(minms/float(numSamples)));
			if(lfoPos>LFO_SIZE) lfoPos-=LFO_SIZE;

			//cost = cpu::cycles() - cost;
			//work_cpu_cost(work_cpu_cost() + cost);
		}
		
		int LFO::GenerateAudio( int numSamples )
		{
			_worked=true;
			return numSamples;
		}

		bool LFO::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
		{
			std::uint32_t size;
			pFile->Read(size);
			pFile->Read(waveform);
			pFile->Read(lSpeed);
			pFile->ReadArray(macOutput,NUM_CHANS);
			pFile->ReadArray(paramOutput,NUM_CHANS);
			pFile->ReadArray(level,NUM_CHANS);
			pFile->ReadArray(phase,NUM_CHANS);
			return true;
		}

		void LFO::SaveSpecificChunk(RiffFile* pFile) const
		{
			std::uint32_t const size(sizeof waveform + sizeof lSpeed + sizeof macOutput + sizeof paramOutput + sizeof level + sizeof phase);
			pFile->Write(size);
			pFile->Write(waveform);
			pFile->Write(lSpeed);
			pFile->WriteArray(macOutput,NUM_CHANS);
			pFile->WriteArray(paramOutput,NUM_CHANS);
			pFile->WriteArray(level,NUM_CHANS);
			pFile->WriteArray(phase,NUM_CHANS);
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
				for(int i(0);i<LFO_SIZE;++i) //????
				{
					waveTable[i] = 0;
				}
				break;
			}
		}

		void LFO::ParamStart( int which )
		{
			if(which<0 || which>=NUM_CHANS) return; // jic
			int destMac = macOutput[which];
			int destParam = paramOutput[which];

			if(destMac != -1 && song()->machine(destMac) != NULL
				&& destParam != -1 && destParam < song()->machine(destMac)->GetNumParams())
			{
				int minVal, maxVal;
				float curLFO, lfoAmt;

				song()->machine(destMac)->GetParamRange(destParam, minVal, maxVal);
				curLFO = waveTable[int(lfoPos+phase[which]+(MAX_PHASE/2.0f)) % LFO_SIZE];
				lfoAmt = (level[which]-MAX_DEPTH)/(float)MAX_DEPTH;

				//bad! bad!
				//prevVal[which] = Gloxxxxxxxxxxxxxxxxxbal::song().machine(macOutput[which])->GetParamValue(paramOutput[which]);
				//centerVal[which] = prevVal[which] - (curLFO * ((maxVal-minVal)/2.0f) * lfoAmt);

				centerVal[which] = song()->machine(destMac)->GetParamValue(destParam);
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


		//currently, this function resets the value of an output parameter to where it would be at lfo==0.  this behavior deserves
		// some consideration, because it is conceivable that some people would want turning off an output to leave the parameter where it is
		// instead of jerking it back to its original position..  on the other hand, without this code, sweeping through a list of parameters
		// carelessly can wreak havoc on an entire plugin's settings.  i may decide just to let the user choose which s/he prefers..
		void LFO::ParamEnd(int which)
		{
			if(which<0 || which>=NUM_CHANS) return;
			id_type destMac(macOutput[which]);
			int destParam = paramOutput[which];

			if(destMac != -1 && song()->machine(destMac) != NULL
				&& destParam != -1 && destParam < song()->machine(destMac)->GetNumParams())
			{
				int minVal, maxVal;
				int newVal;
				song()->machine(destMac)->GetParamRange(destParam, minVal, maxVal);
				newVal = centerVal[which];
				if(newVal<minVal) newVal=minVal;
				else if(newVal>maxVal) newVal=maxVal;

				if(destMac != this->id()) // craziness may ensue without this check.. folks routing the lfo to itself are on their own
					song()->machine(destMac)->SetParameter(destParam, newVal); //set to value at lfo==0
			}
		}
	}
}
