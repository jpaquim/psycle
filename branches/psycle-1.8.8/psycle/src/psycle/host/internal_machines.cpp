
#include <psycle/host/detail/project.private.hpp>
#include "internal_machines.hpp"
#include "Configuration.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "AudioDriver.hpp"
#include "cpu_time_clock.hpp"

namespace psycle
{
	namespace host
	{

		char* Dummy::_psName = "DummyPlug";
		char* DuplicatorMac::_psName = "Dupe it!";
		char* AudioRecorder::_psName = "Recorder";
		char* Mixer::_psName = "Mixer";

		//////////////////////////////////////////////////////////////////////////
		// Dummy
		Dummy::Dummy(int index)
		{
			_macIndex = index;
			_type = MACH_DUMMY;
			_mode = MACHMODE_FX;
			sprintf(_editName, _psName);
			wasVST = false;
		}
		Dummy::Dummy(Machine *mac)
			:Machine(mac)
		{
			_type = MACH_DUMMY;
			if (mac->_type == MACH_VST || mac->_type == MACH_VSTFX)
				wasVST = true;
			else
				wasVST = false;
		}
		int Dummy::GenerateAudio(int numSamples, bool measure_cpu_usage)
		{
			UpdateVuAndStanbyFlag(numSamples);
			recursive_processed_ = true;
			return numSamples;
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
		}
		//////////////////////////////////////////////////////////////////////////
		// NoteDuplicator
		MultiMachine::MultiMachine(int index)
		{
			bisTicking = false;
			for (int i=0;i<NUMMACHINES;i++)
			{
				macOutput[i]=-1;
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
		MultiMachine::~MultiMachine()
		{
		}

		void MultiMachine::Init()
		{
			Machine::Init();
			for (int i=0;i<NUMMACHINES;i++)
			{
				macOutput[i]=-1;
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
		void MultiMachine::Stop()
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


		void MultiMachine::Tick( int channel,PatternEntry* pData)
		{
			if ( !_mute && !bisTicking) // Prevent possible loops of MultiMachines.
			{
				bisTicking=true;
				for (int i=0;i<NUMMACHINES;i++)
				{
					if (macOutput[i] != -1 && Global::_pSong->_pMachine[macOutput[i]] != NULL )
					{
						AllocateVoice(channel,i);
						PatternEntry pTemp = *pData;
						CustomTick(channel,i, pTemp);
						// this can happen if the parameter is the machine itself.
						if (Global::_pSong->_pMachine[macOutput[i]] != this) 
						{
							Global::_pSong->_pMachine[macOutput[i]]->Tick(allocatedchans[channel][i],&pTemp);
							if (pTemp._note >= notecommands::release )
							{
								DeallocateVoice(channel,i);
							}
						}
						else
						{
							DeallocateVoice(channel,i);
						}
					}
				}
			}
			bisTicking=false;
		}
		void MultiMachine::AllocateVoice(int channel,int machine)
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
		void MultiMachine::DeallocateVoice(int channel, int machine)
		{
			if ( allocatedchans[channel][machine] == -1 )
				return;
			availablechans[macOutput[machine]][allocatedchans[channel][machine]]= true;
			allocatedchans[channel][machine]=-1;
		}
		bool MultiMachine::playsTrack(const int track) const
		{
			return Machine::playsTrack(track);
		}

		//////////////////////////////////////////////////////////////////////////
		// NoteDuplicator
		DuplicatorMac::DuplicatorMac(int index):
			MultiMachine(index)
		{
			_macIndex = index;
			_numPars = NUMMACHINES*2;
			_nCols = 2;
			_type = MACH_DUPLICATOR;
			_mode = MACHMODE_GENERATOR;
			sprintf(_editName, _psName);

			for (int i=0;i<NUMMACHINES;i++)
			{
				noteOffset[i]=0;
			}
		}
		void DuplicatorMac::Init()
		{
			MultiMachine::Init();
			for (int i=0;i<NUMMACHINES;i++)
			{

				noteOffset[i]=0;
			}
		}

		void DuplicatorMac::Tick()
		{

		}

		void DuplicatorMac::CustomTick(int channel,int i, PatternEntry& pData)
		{
			if ( pData._note < notecommands::release )
			{
				int note = pData._note+noteOffset[i];
				if ( note>=notecommands::release) note=119;
				else if (note<0 ) note=0;
				pData._note = static_cast<std::uint8_t>(note);
			}
		}
		bool DuplicatorMac::playsTrack(const int track) const
		{
			for (int i=0;i<NUMMACHINES;i++)
			{
				if (macOutput[i] != -1 && Global::_pSong->_pMachine[macOutput[i]] != NULL )
				{
					if(allocatedchans[track][i] != -1 &&
						Global::_pSong->_pMachine[macOutput[i]]->playsTrack(allocatedchans[track][i]))
					{
							return true;
					}
				}
			}
			return false;
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

		int DuplicatorMac::GenerateAudio(int numSamples, bool measure_cpu_usage)
		{
			recursive_processed_ = true;
			Standby(true);
			return numSamples;
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

		//////////////////////////////////////////////////////////////////////////
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
			_gainvol=1.0f;
		}
		AudioRecorder::~AudioRecorder()
		{
			AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
			if (_initialized) mydriver.RemoveCapturePort(_captureidx);
		}
		void AudioRecorder::Init(void)
		{
			Machine::Init();
			if (!_initialized)
			{
				AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
				_initialized = mydriver.AddCapturePort(_captureidx);
				strncpy(drivername,mydriver.settings().GetInfo()._psName,32);
			}
		}
		void AudioRecorder::ChangePort(int newport)
		{
			AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
			if ( _initialized )
			{
				mydriver.Enable(false);
				mydriver.RemoveCapturePort(_captureidx);
				_initialized=false;
			}
			_initialized = mydriver.AddCapturePort(newport);
			_captureidx = newport;
			strncpy(drivername,mydriver.settings().GetInfo()._psName,32);
			mydriver.Enable(true);
		}
		int AudioRecorder::GenerateAudio(int numSamples, bool measure_cpu_usage)
		{
			if (!_mute &&_initialized)
			{
				AudioDriver &mydriver = *Global::pConfig->_pOutputDriver;
				AudioDriverInfo &myinfo = mydriver.settings().GetInfo();
				if(strcmp(myinfo._psName, drivername)) {
					_initialized = false;
					return numSamples;
				}
				mydriver.GetReadBuffers(_captureidx,&pleftorig,&prightorig,numSamples);
				if(pleftorig == NULL) {
					helpers::dsp::Clear(_pSamplesL,numSamples);
					helpers::dsp::Clear(_pSamplesR,numSamples);
				}
				else {
					helpers::dsp::MovMul(pleftorig,_pSamplesL,numSamples,_gainvol);
					helpers::dsp::MovMul(prightorig,_pSamplesR,numSamples,_gainvol);
				}
				UpdateVuAndStanbyFlag(numSamples);
			}
			else Standby(true);
			recursive_processed_ = true;
			return numSamples;
		}
		bool AudioRecorder::LoadSpecificChunk(RiffFile * pFile, int version)
		{
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Read(&_captureidx,sizeof _captureidx);
			pFile->Read(&_gainvol,sizeof _gainvol);
			return true;
		}
		void AudioRecorder::SaveSpecificChunk(RiffFile * pFile)
		{
			UINT size = sizeof _captureidx+ sizeof _gainvol;
			pFile->Write(&size, sizeof size); // size of this part params to save
			pFile->Write(&_captureidx,sizeof _captureidx);
			pFile->Write(&_gainvol,sizeof _gainvol);
		}


		//////////////////////////////////////////////////////////////////////////
		// Mixer
		Mixer::Mixer(int id)
		{
			_macIndex = id;
			_numPars = 0x100;
			_type = MACH_MIXER;
			_mode = MACHMODE_FX;
			sprintf(_editName, _psName);
			mixed=true;
			sched_returns_processed_curr=0;
			sched_returns_processed_prev=0;
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

		void Mixer::Tick( int channel,PatternEntry* pData)
		{
			if(pData->_note == notecommands::tweak)
			{
				int nv = (pData->_cmd<<8)+pData->_parameter;
				SetParameter(pData->_inst,nv);
			}
			else if(pData->_note == notecommands::tweakslide)
			{
				//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
				// doing simply "tweak" for now..
				int nv = (pData->_cmd<<8)+pData->_parameter;
				SetParameter(pData->_inst,nv);
			}
		}
		void Mixer::recursive_process(unsigned int frames, bool measure_cpu_usage) {
			if(_mute || Bypass()) {
				recursive_process_deps(frames, true, measure_cpu_usage);
				return;
			}

			// Step One, do the usual work, except mixing all the inputs to a single stream.
			recursive_process_deps(frames, false, measure_cpu_usage);
			// Step Two, prepare input signals for the Send Fx, and make them work
			sched_returns_processed_curr=0;
			FxSend(frames, true, measure_cpu_usage);
			{ // Step Three, Mix the returns of the Send Fx's with the leveled input signal
				cpu_time_clock::time_point t0;
				if(measure_cpu_usage) t0 = cpu_time_clock::now();
				Mix(frames);
				helpers::dsp::Undenormalize(_pSamplesL, _pSamplesR, frames);
				Machine::UpdateVuAndStanbyFlag(frames);
				if(measure_cpu_usage) {
					cpu_time_clock::time_point const t1(cpu_time_clock::now());
					accumulate_processing_time(t1 - t0);
				}
			}

			recursive_processed_ = true;
		}

		void Mixer::FxSend(int numSamples, bool recurse, bool measure_cpu_usage)
		{
			// Note: Since mixer allows to route returns to other sends, this needs
			// to stop in non-recurse mode when the first of such routes is found
			sched_returns_processed_prev=sched_returns_processed_curr;
			int i;
			for (i=sched_returns_processed_curr; i<numsends(); i++)
			{
				if (sends_[i].IsValid())
				{
					Machine* pSendMachine = Global::song()._pMachine[sends_[i].machine_];
					assert(pSendMachine);
					if (!pSendMachine->recursive_processed_ && !pSendMachine->recursive_is_processing_)
					{ 
						cpu_time_clock::time_point t0;
						if(measure_cpu_usage) t0 = cpu_time_clock::now();
						bool soundready=false;
						// Mix all the inputs and route them to the send fx.
						{
							if ( solocolumn_ >=0 && solocolumn_ < MAX_CONNECTIONS)
							{
								int j = solocolumn_;
								if (_inputCon[j] && !Channel(j).Mute() && !Channel(j).DryOnly() && (_sendvolpl[j][i] != 0.0f || _sendvolpr[j][i] != 0.0f ))
								{
									Machine* pInMachine = Global::song()._pMachine[_inputMachines[j]];
									assert(pInMachine);
									if(!pInMachine->_mute && !pInMachine->Standby())
									{
										helpers::dsp::Add(pInMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pInMachine->_lVol*_sendvolpl[j][i]);
										helpers::dsp::Add(pInMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pInMachine->_rVol*_sendvolpr[j][i]);
										soundready=true;
									}
								}
							}
							else for (int j=0; j<numinputs(); j++)
							{
								if (_inputCon[j] && !Channel(j).Mute() && !Channel(j).DryOnly() && (_sendvolpl[j][i] != 0.0f || _sendvolpr[j][i] != 0.0f ))
								{
									Machine* pInMachine = Global::song()._pMachine[_inputMachines[j]];
									assert(pInMachine);
									if(!pInMachine->_mute && !pInMachine->Standby())
									{
										helpers::dsp::Add(pInMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pInMachine->_lVol*_sendvolpl[j][i]);
										helpers::dsp::Add(pInMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pInMachine->_rVol*_sendvolpr[j][i]);
										soundready=true;
									}
								}
							}
							for (int j=0; j<i; j++)
							{
								if (Return(j).IsValid() && Return(j).Send(i) && !Return(j).Mute() && (mixvolretpl[j][i] != 0.0f || mixvolretpr[j][i] != 0.0f ))
								{
									Machine* pRetMachine = Global::song()._pMachine[Return(j).Wire().machine_];
									assert(pRetMachine);
									// Note: Since mixer allows to route returns to other sends, this needs
									// to stop when the first of such routes is found.
									if(!recurse) 
									{
										if(!pRetMachine->sched_processed_) {
											sched_returns_processed_curr=i;
											return;
										}
									}
									if(!pRetMachine->_mute && !pRetMachine->Standby())
									{
										helpers::dsp::Add(pRetMachine->_pSamplesL, pSendMachine->_pSamplesL, numSamples, pRetMachine->_lVol*mixvolretpl[j][i]);
										helpers::dsp::Add(pRetMachine->_pSamplesR, pSendMachine->_pSamplesR, numSamples, pRetMachine->_rVol*mixvolretpr[j][i]);
										soundready=true;
									}
								}
							}
							if (soundready) pSendMachine->Standby(false);
						}

						// tell the FX to work, now that the input is ready.
						if(recurse){
							//Time is only accumulated in recurse mode, because in shed mode the sched_process method already does it.
							if(measure_cpu_usage) {
								cpu_time_clock::time_point const t1(cpu_time_clock::now());
								accumulate_processing_time(t1 - t0);
							}
							Machine* pRetMachine = Global::song()._pMachine[Return(i).Wire().machine_];
							pRetMachine->recursive_process(numSamples, measure_cpu_usage);
							/// pInMachines are verified in Machine::WorkNoMix, so we only check the returns.
							if(!pRetMachine->Standby())Standby(false);
						}
					}
				}
			}
			sched_returns_processed_curr=i;
		}

		void Mixer::Mix(int numSamples)
		{
			if ( master_.DryWetMix() > 0.0f)
			{
				if ( solocolumn_ >= MAX_CONNECTIONS)
				{
					int i= solocolumn_-MAX_CONNECTIONS;
					if (ReturnValid(i) && !Return(i).Mute() && Return(i).MasterSend() && (mixvolretpl[i][MAX_CONNECTIONS] != 0.0f || mixvolretpr[i][MAX_CONNECTIONS] != 0.0f ))
					{
						Machine* pRetMachine = Global::song()._pMachine[Return(i).Wire().machine_];
						assert(pRetMachine);
						if(!pRetMachine->_mute && !pRetMachine->Standby())
						{
							helpers::dsp::Add(pRetMachine->_pSamplesL, _pSamplesL, numSamples, pRetMachine->_lVol*mixvolretpl[i][MAX_CONNECTIONS]);
							helpers::dsp::Add(pRetMachine->_pSamplesR, _pSamplesR, numSamples, pRetMachine->_rVol*mixvolretpr[i][MAX_CONNECTIONS]);
						}
					}
				}
				else for (int i=0; i<numreturns(); i++)
				{
					if (Return(i).IsValid() && !Return(i).Mute() && Return(i).MasterSend() && (mixvolretpl[i][MAX_CONNECTIONS] != 0.0f || mixvolretpr[i][MAX_CONNECTIONS] != 0.0f ))
					{
						Machine* pRetMachine = Global::song()._pMachine[Return(i).Wire().machine_];
						assert(pRetMachine);
						if(!pRetMachine->_mute && !pRetMachine->Standby())
						{
							helpers::dsp::Add(pRetMachine->_pSamplesL, _pSamplesL, numSamples, pRetMachine->_lVol*mixvolretpl[i][MAX_CONNECTIONS]);
							helpers::dsp::Add(pRetMachine->_pSamplesR, _pSamplesR, numSamples, pRetMachine->_rVol*mixvolretpr[i][MAX_CONNECTIONS]);
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
						Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
						assert(pInMachine);
						if(!pInMachine->_mute && !pInMachine->Standby())
						{
							helpers::dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*mixvolpl[i]);
							helpers::dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*mixvolpr[i]);
						}
					}
				}
				else for (int i=0; i<numinputs(); i++)
				{
					if (_inputCon[i] && !Channel(i).Mute() && !Channel(i).WetOnly() && (mixvolpl[i] != 0.0f || mixvolpr[i] != 0.0f ))
					{
						Machine* pInMachine = Global::song()._pMachine[_inputMachines[i]];
						assert(pInMachine);
						if(!pInMachine->_mute && !pInMachine->Standby())
						{
							helpers::dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*mixvolpl[i]);
							helpers::dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*mixvolpr[i]);
						}
					}
				}
			}
		}

		/// tells the scheduler which machines to process before this one
		void Mixer::sched_inputs(sched_deps & result) const {
			if(mixed) {
				// step 1: receive all standard inputs
				Machine::sched_inputs(result);
			} else {
				// step 2: get the output from return fx that have been processed
				for(unsigned int i = sched_returns_processed_prev; i < sched_returns_processed_curr; ++i) {
					if(Return(i).IsValid()) {
						Machine & returned(*Global::_pSong->_pMachine[Return(i).Wire().machine_]);
						result.push_back(&returned);
					}
				}
			}
		}

		/// tells the scheduler which machines may be processed after this one
		void Mixer::sched_outputs(sched_deps & result) const {
			if(!mixed) {
				// step 1: signal sent to sends. Identify them.
				for (unsigned int i=sched_returns_processed_prev; i<sched_returns_processed_curr; i++) if (Send(i).IsValid()) {
					Machine & input(*Global::_pSong->_pMachine[Send(i).machine_]);
					result.push_back(&input);
				}
			} else {
				// step 2: everything done, our outputs will be the next connections
				Machine::sched_outputs(result);
			}
		}

		/// called by the scheduler to ask for the actual processing of the machine
		bool Mixer::sched_process(unsigned int frames, bool measure_cpu_usage) {
			cpu_time_clock::time_point t0;
			if(measure_cpu_usage) t0 = cpu_time_clock::now();

			if( sched_returns_processed_curr < numreturns()) {
				// Step 1: send audio to the effects
				mixed = false;
				if(!_mute && !Bypass()) {
					FxSend(frames, false, measure_cpu_usage);
				}
				else {
					sched_returns_processed_prev = sched_returns_processed_curr;
					sched_returns_processed_curr = numreturns();
				}
			} else {
				// step 2: mix input with return fx to generate output
				if(Bypass()) {
					for (int i=0; i<numinputs(); i++) if (_inputCon[i])	{
						Machine & input_node(*Global::song()._pMachine[_inputMachines[i]]);
						if(!input_node._mute && !input_node.Standby()) {
							helpers::dsp::Add(input_node._pSamplesL, _pSamplesL, frames, input_node._lVol);
							helpers::dsp::Add(input_node._pSamplesR, _pSamplesR, frames, input_node._rVol);
						}
					}
				}
				else if(!_mute) {
					Mix(frames);
				}
				helpers::dsp::Undenormalize(_pSamplesL, _pSamplesR, frames);
				Machine::UpdateVuAndStanbyFlag(frames);
				mixed = true;
				sched_returns_processed_curr=0;
				++processing_count_;
			}
			if(measure_cpu_usage){ 
				cpu_time_clock::time_point const t1(cpu_time_clock::now());
				accumulate_processing_time(t1 - t0);
			}
			return mixed;
		}

		float Mixer::GetWireVolume(int wireIndex)
		{
			if (wireIndex< MAX_CONNECTIONS)
				return Machine::GetWireVolume(wireIndex);
			else if ( ReturnValid(wireIndex-MAX_CONNECTIONS) )
				return Return(wireIndex-MAX_CONNECTIONS).Wire().volume_;
			return 0;
		}
		void Mixer::SetWireVolume(int wireIndex,float value)
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
		void Mixer::InsertInputWireIndex(Song* pSong,int wireIndex, int srcmac, float wiremultiplier,float initialvol)
		{
			if (wireIndex< MAX_CONNECTIONS)
			{
				// If we're replacing an existing connection, keep the sends
				if (ChannelValid(wireIndex))
				{
					InputChannel chan = Channel(wireIndex);
					InsertChannel(wireIndex,&chan);
				} else {
					InsertChannel(wireIndex);
				}
				Machine::InsertInputWireIndex(pSong,wireIndex,srcmac,wiremultiplier,initialvol);
				RecalcChannel(wireIndex);
				for (int i(0);i<numsends();++i)
				{
					RecalcSend(wireIndex,i);
				}
			}
			else
			{
				wireIndex-=MAX_CONNECTIONS;
				SetMixerSendFlag(pSong,pSong->_pMachine[srcmac]);
				MixerWire wire(srcmac,0);
				// If we're replacing an existing connection
				if ( ReturnValid(wireIndex))
				{
					ReturnChannel ret = Return(wireIndex);
					ret.Wire() = wire;
					InsertReturn(wireIndex,&ret);
					InsertSend(wireIndex,wire);

				} else {
					InsertReturn(wireIndex,wire);
					InsertSend(wireIndex,wire);
				}
				Return(wireIndex).Wire().volume_ = initialvol;
				Return(wireIndex).Wire().normalize_ = wiremultiplier;
				sends_[wireIndex].volume_ = 1.0f;
				sends_[wireIndex].normalize_ = 1.0f/wiremultiplier;
				RecalcReturn(wireIndex);
				for(int c(0) ; c < numinputs() ; ++c)
				{
					RecalcSend(c,wireIndex);	
				}
			}
		}

		int Mixer::FindInputWire(int macIndex)
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
		int Mixer::GetFreeInputWire(int slottype)
		{
			if ( slottype == 0) return Machine::GetFreeInputWire(0);
			else 
			{
				// Get a free sendfx slot
				for(int c(0) ; c < MAX_CONNECTIONS ; ++c)
				{
					if(!ReturnValid(c)) return c+MAX_CONNECTIONS;
				}
				return -1;
			}
		}

		void Mixer::DeleteInputWireIndex(Song* pSong,int wireIndex)
		{
			if ( wireIndex < MAX_CONNECTIONS)
			{
				Machine::DeleteInputWireIndex(pSong,wireIndex);
				DiscardChannel(wireIndex);
			}
			else
			{
				wireIndex-=MAX_CONNECTIONS;
				DeleteMixerSendFlag(pSong,pSong->_pMachine[Return(wireIndex).Wire().machine_]);
				Return(wireIndex).Wire().machine_=-1;
				sends_[wireIndex].machine_ = -1;
				DiscardReturn(wireIndex);
				DiscardSend(wireIndex);
			}
		}
		void Mixer::NotifyNewSendtoMixer(Song* pSong,int callerMac,int senderMac)
		{
			// Mixer reached, set flags upwards.
			SetMixerSendFlag(pSong,pSong->_pMachine[callerMac]);
			bool found=false;
			int i=0;
			for (;i < MAX_CONNECTIONS; i++)
			{
				if ( ReturnValid(i))
				{
					//If the caller is any of the ends of the chain, found.
					if( Send(i).machine_ == callerMac || Return(i).Wire().machine_ == callerMac) {
						found=true;
						break;
					}
					//Else, try to find it inside this chain.
					Machine* pMac = pSong->_pMachine[Return(i).Wire().machine_];
					for(int c=0; c<MAX_CONNECTIONS; c++)
					{
						if(pMac->_inputCon[c])	{
							pMac = pSong->_pMachine[pMac->_inputMachines[c]];
							if ( callerMac == pMac->_macIndex) {
								found = true;
								goto endfor;
							}
							c=0;
							continue;
						}
					}
				}
			}
endfor:
			if (found)
			{
				sends_[i].machine_ = senderMac;
				sends_[i].normalize_ = GetAudioRange()/pSong->_pMachine[senderMac]->GetAudioRange();
				for (int ch(0);ch<numinputs();ch++)
				{
					RecalcSend(ch,i);
				}
			}
		}
		void Mixer::SetMixerSendFlag(Song* pSong,Machine* mac)
		{
			for (int i(0);i<MAX_CONNECTIONS;++i)
			{
				if (mac->_inputCon[i]) SetMixerSendFlag(pSong,pSong->_pMachine[mac->_inputMachines[i]]);
			}
			mac->_isMixerSend=true;
		}
		void Mixer::DeleteMixerSendFlag(Song* pSong,Machine* mac)
		{
			for (int i(0);i<MAX_CONNECTIONS;++i)
			{
				if (mac->_inputCon[i]) DeleteMixerSendFlag(pSong,pSong->_pMachine[mac->_inputMachines[i]]);
			}
			mac->_isMixerSend=false;
		}

		void Mixer::DeleteWires(Song* pSong)
		{
			Machine::DeleteWires(pSong);
			Machine *iMac;
			for(int w=0; w<numreturns(); w++)
			{
				// Checking send/return Wires
				if(Return(w).IsValid())
				{
					iMac = pSong->_pMachine[Return(w).Wire().machine_];
					if (iMac)
					{
						int wix = iMac->FindOutputWire(_macIndex);
						if (wix >=0)
						{
							iMac->DeleteOutputWireIndex(pSong,wix);
						}
					}
					DeleteInputWireIndex(pSong,w+MAX_CONNECTIONS);
				}
			}
		}
		std::string Mixer::GetAudioInputName(int port)
		{
			std::string rettxt;
			if (port < chanmax )
			{	
				int i = port-chan1;
				rettxt = "Input ";
				if ( i < 9 ) rettxt += ('1'+i);
				else { rettxt += '1'; rettxt += ('0'+i-9); }
				return rettxt;
			}
			else if ( port < returnmax)
			{
				int i = port-return1;
				rettxt = "Return ";
				if ( i < 9 ) rettxt += ('1'+i);
				else { rettxt += '1'; rettxt += ('0'+i-9); }
				return rettxt;
			}
			rettxt = "-";
			return rettxt;
		}

		int Mixer::GetNumCols()
		{
			return 2+numinputs()+numreturns();
		}
		void Mixer::GetParamRange(int numparam, int &minval, int &maxval)
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
		void Mixer::GetParamName(int numparam,char *name)
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

		int Mixer::GetParamValue(int numparam)
		{
			int channel=numparam/16;
			int param=numparam%16;
			if ( channel == 0)
			{
				if (param == 0)
				{
					float dbs = helpers::dsp::dB(master_.Volume());
					return (dbs+96.0f)*42.67; // *(0x1000 / 96.0f)
				}
				else if (param <= 12)
				{
					if (!ChannelValid(param-1)) return 0;
					else {
						float dbs = helpers::dsp::dB(Channel(param-1).Volume());
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
					float dbs = helpers::dsp::dB(Return(param-1).Volume());
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

		void Mixer::GetParamValue(int numparam, char *parVal)
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
						float dbs = helpers::dsp::dB(master_.Volume());
						sprintf(parVal,"%.01fdB",dbs);
					}
				}
				else if (param <= 12)
				{ 
					if (!ChannelValid(param-1)) return;
					else if (Channel(param-1).Volume() < 0.00002f ) strcpy(parVal,"-inf");
					else
					{
						float dbs = helpers::dsp::dB(Channel(param-1).Volume());
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
					float dbs = (((val>0.0f)?helpers::dsp::dB(val):-100.0f));
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
					float dbs = (((val>0.0f)?helpers::dsp::dB(val):-100.0f));
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
					float dbs = helpers::dsp::dB(Return(param-1).Volume());
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
						master_.Volume() = helpers::dsp::dB2Amp(dbs);
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
						Channel(param-1).Volume() = helpers::dsp::dB2Amp(dbs);
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
					RecalcReturn(param-1);
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
						Return(param-1).Volume() = helpers::dsp::dB2Amp(dbs);
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

		float Mixer::VuChan(int idx)
		{
			if ( _inputCon[idx] ) 
			{
				//Note that since volumeDisplay is integer, when using positive gain,
				//the result can visually differ from the calculated one
				float vol;
				GetWireVolume(idx,vol);
				vol*=Channel(idx).Volume();
				int temp(lround<int>(50.0f * std::log10(vol)));
				return (Global::song()._pMachine[_inputMachines[idx]]->_volumeDisplay+temp)/97.0f;
			}
			return 0.0f;
		}

		float Mixer::VuSend(int idx)
		{
			if ( SendValid(idx) )
			{
				//Note that since volumeDisplay is integer, when using positive gain,
				// the result can visually differ from the calculated one
				float vol;
				GetWireVolume(idx+MAX_CONNECTIONS,vol);
				vol *= Return(idx).Volume();
				int temp(lround<int>(50.0f * std::log10(vol)));
				return (Global::song()._pMachine[Return(idx).Wire().machine_]->_volumeDisplay+temp)/97.0f;
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
			ExchangeInputWires(chann1,chann2);
			InputChannel tmp = inputs_[chann1];
			inputs_[chann1] = inputs_[chann2];
			inputs_[chann2] = tmp;
			RecalcChannel(chann1);
			RecalcChannel(chann2);
			//The following line autocleans the left-out channels at the end.
			DiscardChannel(numinputs()-1); 
		}
		void Mixer::ExchangeReturns(int chann1,int chann2)
		{
			ReturnChannel tmp = returns_[chann1];
			returns_[chann1] = returns_[chann2];
			returns_[chann2] = tmp;
			RecalcReturn(chann1);
			RecalcReturn(chann2);
			ExchangeSends(chann1,chann2);
			//The following lines autoclean the left-out send/returns at the end.
			DiscardReturn(numreturns()-1);
			DiscardSend(numsends()-1);
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

			for(int send=0; send < numsends(); ++send)
			{
				if (Return(idx).Send(send))
				{
					mixvolretpl[idx][send] = mixvolretpr[idx][send] = Return(idx).Volume()*( Return(idx).Wire().normalize_/ Send(send).normalize_);
					if (Return(idx).Panning() >= 0.5f )
					{
						mixvolretpl[idx][send] *= (1.0f-Return(idx).Panning())*2.0f;
					}
					else mixvolretpr[idx][send] *= (Return(idx).Panning())*2.0f;
				}
			}
			float wet = master_.Volume()*master_.Gain();
			if (master_.DryWetMix() < 0.5f )
			{
				wet *= (master_.DryWetMix())*2.0f;
			}

			mixvolretpl[idx][MAX_CONNECTIONS] = mixvolretpr[idx][MAX_CONNECTIONS] = Return(idx).Volume()*val*wet/Return(idx).Wire().normalize_;
			if (Return(idx).Panning() >= 0.5f )
			{
				mixvolretpl[idx][MAX_CONNECTIONS] *= (1.0f-Return(idx).Panning())*2.0f;
			}
			else mixvolretpr[idx][MAX_CONNECTIONS] *= (Return(idx).Panning())*2.0f;
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
		bool Mixer::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t filesize;
			pFile->Read(&filesize,sizeof(filesize));

			pFile->Read(&solocolumn_,sizeof(solocolumn_));
			pFile->Read(&master_.Volume(),sizeof(float));
			pFile->Read(&master_.Gain(),sizeof(float));
			pFile->Read(&master_.DryWetMix(),sizeof(float));

			int numins(0),numrets(0);
			pFile->Read(&numins,sizeof(int));
			pFile->Read(&numrets,sizeof(int));
			if ( numins >0 ) InsertChannel(numins-1);
			if ( numrets >0 ) InsertReturn(numrets-1);
			if ( numrets >0 ) InsertSend(numrets-1,MixerWire());
			for (int i(0);i<numinputs();i++)
			{
				for (int j(0);j<numsends();j++)
				{
					float send(0.0f);
					pFile->Read(&send,sizeof(float));
					Channel(i).Send(j)=send;
				}
				pFile->Read(&Channel(i).Volume(),sizeof(float));
				pFile->Read(&Channel(i).Panning(),sizeof(float));
				pFile->Read(&Channel(i).DryMix(),sizeof(float));
				pFile->Read(&Channel(i).Mute(),sizeof(bool));
				pFile->Read(&Channel(i).DryOnly(),sizeof(bool));
				pFile->Read(&Channel(i).WetOnly(),sizeof(bool));
			}
			for (int i(0);i<numreturns();i++)
			{
				pFile->Read(&Return(i).Wire().machine_,sizeof(int));
				pFile->Read(&Return(i).Wire().volume_,sizeof(float));
				pFile->Read(&Return(i).Wire().normalize_,sizeof(float));
				pFile->Read(&sends_[i].machine_,sizeof(int));
				pFile->Read(&sends_[i].volume_,sizeof(float));
				pFile->Read(&sends_[i].normalize_,sizeof(float));
				for (int j(0);j<numsends();j++)
				{
					bool send(false);
					pFile->Read(&send,sizeof(bool));
					Return(i).Send(j,send);
				}
				pFile->Read(&Return(i).MasterSend(),sizeof(bool));
				pFile->Read(&Return(i).Volume(),sizeof(float));
				pFile->Read(&Return(i).Panning(),sizeof(float));
				pFile->Read(&Return(i).Mute(),sizeof(bool));
			}
			RecalcMaster();
			for (int i(0);i<numinputs();i++)
				for(int j(0);j<numsends();j++)
					RecalcSend(i,j);
			return true;
		}

		void Mixer::SaveSpecificChunk(RiffFile* pFile)
		{
			std::uint32_t size(sizeof(solocolumn_)+sizeof(master_)+2*sizeof(int));
			size+=(3*sizeof(float)+3*sizeof(bool)+numsends()*sizeof(float))*numinputs();
			size+=(2*sizeof(float)+2*sizeof(bool)+numsends()*sizeof(bool)+2*sizeof(float)+sizeof(int))*numreturns();
			size+=(2*sizeof(float)+sizeof(int))*numsends();
			pFile->Write(&size,sizeof(size));

			pFile->Write(&solocolumn_,sizeof(solocolumn_));
			pFile->Write(&master_.Volume(),sizeof(float));
			pFile->Write(&master_.Gain(),sizeof(float));
			pFile->Write(&master_.DryWetMix(),sizeof(float));

			const int numins = numinputs();
			const int numrets = numreturns();
			pFile->Write(&numins,sizeof(int));
			pFile->Write(&numrets,sizeof(int));
			for (int i(0);i<numinputs();i++)
			{
				for (int j(0);j<numsends();j++)
				{
					pFile->Write(&Channel(i).Send(j),sizeof(float));
				}
				pFile->Write(&Channel(i).Volume(),sizeof(float));
				pFile->Write(&Channel(i).Panning(),sizeof(float));
				pFile->Write(&Channel(i).DryMix(),sizeof(float));
				pFile->Write(&Channel(i).Mute(),sizeof(bool));
				pFile->Write(&Channel(i).DryOnly(),sizeof(bool));
				pFile->Write(&Channel(i).WetOnly(),sizeof(bool));
			}
			for (int i(0);i<numreturns();i++)
			{
				pFile->Write(&Return(i).Wire().machine_,sizeof(int));
				pFile->Write(&Return(i).Wire().volume_,sizeof(float));
				pFile->Write(&Return(i).Wire().normalize_,sizeof(float));
				pFile->Write(&sends_[i].machine_,sizeof(int));
				pFile->Write(&sends_[i].volume_,sizeof(float));
				pFile->Write(&sends_[i].normalize_,sizeof(float));
				for (int j(0);j<numsends();j++)
				{
					bool send(Return(i).Send(j));
					pFile->Write(&send,sizeof(bool));
				}
				pFile->Write(&Return(i).MasterSend(),sizeof(bool));
				pFile->Write(&Return(i).Volume(),sizeof(float));
				pFile->Write(&Return(i).Panning(),sizeof(float));
				pFile->Write(&Return(i).Mute(),sizeof(bool));
			}
		}
	}
}
