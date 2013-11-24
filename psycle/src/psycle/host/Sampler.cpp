///\file
///\brief implementation file for psycle::host::Sampler.

#include <psycle/host/detail/project.private.hpp>
#include "Sampler.hpp"
#include "Song.hpp"
#include "XMInstrument.hpp"
#include "Player.hpp"
#include "FileIO.hpp"
#include "Configuration.hpp"
#include <psycle/helpers/value_mapper.hpp>
#include <psycle/helpers/math.hpp>
namespace psycle
{
	namespace host
	{
		char* Sampler::_psName = "Sampler";

		Voice::Voice():resampler_data(NULL)
		{
		}
		Voice::~Voice()
		{
		}

		Sampler::Sampler(int index)
		{
			_macIndex = index;
			_numPars=0;
			_type = MACH_SAMPLER;
			_mode = MACHMODE_GENERATOR;
			strncpy(_editName, _psName, sizeof(_editName)-1);
			_editName[sizeof(_editName)-1]='\0';
			InitializeSamplesVector();
			
			baseC = 60;
			_resampler.quality(helpers::dsp::resampler::quality::spline);
			for (int i=0; i<SAMPLER_MAX_POLYPHONY; i++)
			{
				_voices[i]._envelope._stage = ENV_OFF;
				_voices[i]._envelope._sustain = 0;
				_voices[i]._filterEnv._stage = ENV_OFF;
				_voices[i]._filterEnv._sustain = 0;
				_voices[i]._cutoff = 0;
				_voices[i]._sampleCounter = 0;
				_voices[i]._triggerNoteOff = 0;
				_voices[i]._triggerNoteDelay = 0;
				_voices[i]._channel = -1;
				_voices[i]._wave._lVolCurr = 0;
				_voices[i]._wave._rVolCurr = 0;

				_voices[i].effCmd = SAMPLER_CMD_NONE;
			}
			for (int i = 0; i < MAX_TRACKS; i++)
			{
				lastInstrument[i]=255;
			}
		}

		Sampler::~Sampler() {
			for (int i=0; i<SAMPLER_MAX_POLYPHONY; i++)
			{
				if (_voices[i].resampler_data != NULL) _resampler.DisposeResamplerData(_voices[i].resampler_data);
			}
		}
		void Sampler::Init(void)
		{
			Machine::Init();

			_numVoices = SAMPLER_DEFAULT_POLYPHONY;
			for (int i=0; i<_numVoices; i++)
			{
				_voices[i]._envelope._stage = ENV_OFF;
				_voices[i]._envelope._sustain = 0;
				_voices[i]._filterEnv._stage = ENV_OFF;
				_voices[i]._filterEnv._sustain = 0;
				_voices[i]._filter.Init(44100);
				_voices[i]._triggerNoteOff = 0;
				_voices[i]._triggerNoteDelay = 0;
			}
		}

		void Sampler::Stop(void)
		{
			for (int i=0; i<_numVoices; i++)
			{
				NoteOffFast(i);
			}
		}

		void Sampler::StopInstrument(int insIdx) {
			for (int i=0; i<_numVoices; i++)
			{
				Voice* pVoice = &_voices[insIdx];
				if (pVoice->_envelope._stage != ENV_OFF && pVoice->_instrument == insIdx)
				{
					pVoice->_envelope._stage = ENV_OFF;
				}
			}
		}
		const char* Sampler::AuxColumnName(int idx) const {
			SampleList &m_Samples = Global::song().samples;
			return m_Samples.Exists(idx)?m_Samples[idx].WaveName().c_str():"";
		}

		void Sampler::SetSampleRate(int sr)
		{
			Machine::SetSampleRate(sr);
			for (int i=0; i<_numVoices; i++)
			{
				_voices[i]._envelope._stage = ENV_OFF;
				_voices[i]._envelope._sustain = 0;
				_voices[i]._filterEnv._stage = ENV_OFF;
				_voices[i]._filterEnv._sustain = 0;
				_voices[i]._filter.SampleRate(sr);
				_voices[i]._triggerNoteOff = 0;
				_voices[i]._triggerNoteDelay = 0;
			}
		}

		bool Sampler::playsTrack(const int track) const
		{ 
			if (TriggerDelayCounter[track] > 0) return true;

			for ( int voice=0; voice<_numVoices; voice++)
			{
				if ( _voices[voice]._channel == track && _voices[voice]._envelope._stage != ENV_OFF)
				{
					return true;
				}
			}
			return false;
		}
		void Sampler::NewLine()
		{
			for (int voice=0;voice<_numVoices;voice++)
			{
				if ( _voices[voice].effCmd != SAMPLER_CMD_EXTENDED )
				{
					_voices[voice].effOld=_voices[voice].effCmd;
					_voices[voice].effCmd=SAMPLER_CMD_NONE;
				}
			}
		}
		void Sampler::Tick(
			int channel,
			PatternEntry* pData)
		{
			if(pData->_note==notecommands::midicc && (pData->_inst < MAX_TRACKS || pData->_inst == 0xFF)) {
				pData->_note=notecommands::empty;
				pData->_inst = 0xFF;
			}
			 // don't process twk , twf of Mcm Commands, or empty commands
			if ( pData->_note > notecommands::release && (pData->_note < notecommands::empty || pData->_cmd == 0) )
			{
				return;
			}
			if ( _mute ) return; // Avoid new note entering when muted.

			int voice;
			int useVoice = -1;

			PatternEntry data = *pData;
			if (data._inst >= 255)
			{
				data._inst = lastInstrument[channel];
				if (data._inst >= 255)
				{
					return;  // no previous sample
				}
			}
			else
			{
				data._inst = lastInstrument[channel] = pData->_inst;
			}


			if ( data._note < notecommands::release )	// Handle Note On.
			{
				if ( !Global::song().samples.IsEnabled(data._inst) ) return; // if no wave, return.

				for (voice=0; voice<_numVoices; voice++)	// Find a voice to apply the new note
				{
					switch(_voices[voice]._envelope._stage)
					{
						case ENV_OFF: 
							if ( _voices[voice]._triggerNoteDelay == 0 ) 
							{ 
								useVoice=voice; 
								voice=_numVoices; // Ok, we can go out from the loop already.
							}
							break;
						case ENV_FASTRELEASE: 
							useVoice = voice;
							break;
						case ENV_RELEASE:
							if ( useVoice == -1 ) 
								useVoice= voice;
							break;
						default:break;
					}
				}
				for ( voice=0; voice<_numVoices; voice++)
				{
					if ( _voices[voice]._channel == channel ) // NoteOff previous Notes in this channel.
					{
						switch (Global::song()._pInstrument[_voices[voice]._instrument]->_NNA)
						{
						case 0:
							NoteOffFast(voice);
							break;
						case 1:
							NoteOff(voice);
							break;
						}
						if ( useVoice == -1 ) { useVoice = voice; }
					}
				}
				if ( useVoice == -1 )	// No free voices. Assign first one.
				{						// This algorithm should be replace by a LRU lookup
					useVoice=0;
				}
				_voices[useVoice]._channel=channel;
			}
			else {
				for (voice=0;voice<_numVoices; voice++)  // Find the...
				{
					if (( _voices[voice]._channel == channel ) && // ...playing voice on current channel.
						(_voices[voice]._envelope._stage != ENV_OFF ) &&
		//				(_voices[voice]._envelope._stage != ENV_RELEASE ) &&
						// Effects can STILL apply in this case.
						// Think on a slow fadeout and changing panning
						(_voices[voice]._envelope._stage != ENV_FASTRELEASE )) 
					{
						if ( data._note == notecommands::release ) NoteOff(voice);//  Handle Note Off
						useVoice=voice;
					}
				}
				if ( useVoice == -1 ) return;   // No playing note on this channel. Just go out.
												// Change it if you have channel commands.
			}
			// If you want to make a command that controls more than one voice (the entire channel, for
			// example) you'll need to change this. Otherwise, add it to VoiceTick().

			VoiceTick(useVoice,&data); 
		}

		int Sampler::VoiceTick(int voice,PatternEntry* pEntry)
		{		

			Voice* pVoice = &_voices[voice];
			int triggered = 0;
			std::uint64_t w_offset = 0;

			pVoice->_sampleCounter=0;
			pVoice->effCmd=pEntry->_cmd;

			switch(pEntry->_cmd) // DO NOT ADD here those commands that REQUIRE a note.
			{
				case SAMPLER_CMD_EXTENDED:
					if ((pEntry->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEOFF)
					{
						pVoice->_triggerNoteOff = (Global::player().SamplesPerRow()/6)*(pEntry->_parameter & 0x0f);
					}
					else if (((pEntry->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEDELAY) && ((pEntry->_parameter & 0x0f) == 0 ))
					{
						pEntry->_cmd=0; pEntry->_parameter=0;
					}
					break;
				case SAMPLER_CMD_PORTAUP:
					pVoice->effVal=pEntry->_parameter;
					break;
				case SAMPLER_CMD_PORTADOWN:
					pVoice->effVal=pEntry->_parameter;
					break;
			}
			

		//  All this mess should be really changed with classes using the "operator=" to "copy" values.

			int twlength = 0;
			if (Global::song().samples.IsEnabled(pEntry->_inst)) twlength = Global::song().samples[pEntry->_inst].WaveLength();
			Instrument * pins = Global::song()._pInstrument[pEntry->_inst];

			if (pEntry->_note < notecommands::release && twlength > 0)
			{
				pVoice->_triggerNoteOff=0;
				pVoice->_instrument = pEntry->_inst;
				const XMInstrument::WaveData<>& wave = Global::song().samples[pVoice->_instrument];
				
				pVoice->_cutoff = (pins->_RCUT) ? alteRand(pins->ENV_F_CO) : pins->ENV_F_CO;
				pVoice->_filter.Ressonance((pins->_RRES) ? alteRand(pins->ENV_F_RQ) : pins->ENV_F_RQ);
				pVoice->_filter.Type(pins->ENV_F_TP);
				pVoice->_coModify = (float)pins->ENV_F_EA;
				pVoice->_filterEnv._sustain = (float)pins->ENV_F_SL*0.0078125f;

				if (( pEntry->_cmd != SAMPLER_CMD_EXTENDED) || ((pEntry->_parameter & 0xF0) != SAMPLER_CMD_EXT_NOTEDELAY))
				{
					pVoice->_filterEnv._stage = ENV_ATTACK;
				}
				pVoice->_filterEnv._step = (1.0f/pins->ENV_F_AT)*(44100.0f/Global::player().SampleRate());
				pVoice->_filterEnv._value = 0;
				
				// Init Wave
				//
				pVoice->_wave._pL = wave.pWaveDataL();
				pVoice->_wave._pR = wave.pWaveDataR();
				pVoice->_wave._stereo = wave.IsWaveStereo();
				pVoice->_wave._samplerate = wave.WaveSampleRate();
				pVoice->_wave._length = twlength;
				
				// Init loop
				if (wave.WaveLoopType() == XMInstrument::WaveData<>::LoopType::NORMAL)
				{
					pVoice->_wave._loop = true;
					pVoice->_wave._loopStart = wave.WaveLoopStart();
					pVoice->_wave._loopEnd = wave.WaveLoopEnd();
				}
				else { pVoice->_wave._loop = false; }
				
				// Init Resampler
				//
				double speeddouble;
				if (pins->_loop)
				{
					double const totalsamples = double(Global::player().SamplesPerRow()*pins->_lines);
					speeddouble = (pVoice->_wave._length/totalsamples);
				}	
				else
				{
					float const finetune = (float)wave.WaveFineTune()*0.01f;
					speeddouble = pow(2.0f, (pEntry->_note+wave.WaveTune()-baseC +finetune)/12.0f)*((float)wave.WaveSampleRate()/Global::player().SampleRate());
				}
				pVoice->_wave._speed = static_cast<std::int64_t>(speeddouble*4294967296.0f);

				if (pVoice->resampler_data != NULL) _resampler.DisposeResamplerData(pVoice->resampler_data);
				pVoice->resampler_data = _resampler.GetResamplerData();
				_resampler.UpdateSpeed(pVoice->resampler_data,speeddouble);

				// Handle wave_start_offset cmd
				//
				if (pEntry->_cmd == SAMPLER_CMD_OFFSET)
				{
					w_offset = pEntry->_parameter*pVoice->_wave._length;
					pVoice->_wave._pos.QuadPart = (w_offset << 24);
				}
				else { pVoice->_wave._pos.QuadPart = 0; }

				// Calculating volume coef ---------------------------------------
				//
				pVoice->_wave._vol = wave.WaveGlobVolume();

				if (pEntry->_cmd == SAMPLER_CMD_VOLUME)
				{
					pVoice->_wave._vol *= helpers::value_mapper::map_256_1(pEntry->_parameter);
				}
				
				// Panning calculation -------------------------------------------
				//
				float panFactor;
				
				if (pins->_RPAN)
				{
					panFactor = (float)rand()*0.000030517578125f;
				}
				else if ( pEntry->_cmd == SAMPLER_CMD_PANNING )
				{
					panFactor = helpers::value_mapper::map_256_1(pEntry->_parameter);
				}
				else {
					panFactor = wave.PanFactor();
				}

				pVoice->_wave._rVolDest = panFactor;
				pVoice->_wave._lVolDest = 1-panFactor;

				if (pVoice->_wave._rVolDest > 0.5f)
				{
					pVoice->_wave._rVolDest = 0.5f;
				}
				if (pVoice->_wave._lVolDest > 0.5f)
				{
					pVoice->_wave._lVolDest = 0.5f;
				}

				pVoice->_wave._lVolCurr = (pVoice->_wave._lVolDest *= pVoice->_wave._vol);
				pVoice->_wave._rVolCurr = (pVoice->_wave._rVolDest *= pVoice->_wave._vol);

				// Init Amplitude Envelope
				//
				pVoice->_envelope._step = (1.0f/pins->ENV_AT)*(44100.0f/Global::player().SampleRate());
				pVoice->_envelope._value = 0.0f;
				pVoice->_envelope._sustain = (float)pins->ENV_SL*0.01f;
				if (( pEntry->_cmd == SAMPLER_CMD_EXTENDED) && ((pEntry->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEDELAY))
				{
					pVoice->_triggerNoteDelay = (Global::player().SamplesPerRow()/6)*(pEntry->_parameter & 0x0f);
					pVoice->_envelope._stage = ENV_OFF;
				}
				else
				{
					if (pEntry->_cmd == SAMPLER_CMD_RETRIG && (pEntry->_parameter&0x0f) > 0)
					{
						pVoice->effretTicks=(pEntry->_parameter&0x0f); // number of Ticks.
						pVoice->effVal= (Global::player().SamplesPerRow()/(pVoice->effretTicks+1));
						
						int volmod = (pEntry->_parameter&0xf0)>>4; // Volume modifier.
						switch (volmod) 
						{
							case 0:
							case 8:	pVoice->effretVol = 0; pVoice->effretMode=0; break;

							case 1:
							case 2:
							case 3:
							case 4:
							case 5: pVoice->effretVol = (float)(std::pow(2.,volmod-1)/64); pVoice->effretMode=1; break;

							case 6: pVoice->effretVol = 0.66666666f;	 pVoice->effretMode=2; break;
							case 7: pVoice->effretVol = 0.5f;			 pVoice->effretMode=2; break;

							case 9:
							case 10:
							case 11:
							case 12:
							case 13: pVoice->effretVol = (float)(std::pow(2.,volmod-9)*(-1))/64; pVoice->effretMode=1; break;

							case 14: pVoice->effretVol = 1.5f;					pVoice->effretMode=2; break;
							case 15: pVoice->effretVol = 2.0f;					pVoice->effretMode=2; break;
						}
						pVoice->_triggerNoteDelay = pVoice->effVal;
					}
					else
					{
						pVoice->_triggerNoteDelay=0;
					}
					pVoice->_envelope._stage = ENV_ATTACK;
				}
				
				triggered = 1;
			}

			else if ((pEntry->_cmd == SAMPLER_CMD_VOLUME) || ( pEntry->_cmd == SAMPLER_CMD_PANNING ) )
			{
				// Calculating volume coef ---------------------------------------
				//
				pVoice->_wave._vol = 0 ;
				if (twlength > 0) { pVoice->_wave._vol = Global::song().samples[pVoice->_instrument].WaveGlobVolume(); }

				if ( pEntry->_cmd == SAMPLER_CMD_VOLUME ) pVoice->_wave._vol *= helpers::value_mapper::map_256_1(pEntry->_parameter);
				
				// Panning calculation -------------------------------------------
				//
				float panFactor;
				
				if (pins->_RPAN)
				{
					panFactor = (float)rand()*0.000030517578125f;
				}
				else if ( pEntry->_cmd == SAMPLER_CMD_PANNING )
				{
					panFactor = helpers::value_mapper::map_256_1(pEntry->_parameter);
				}
				else if (twlength > 0) 
				{
					panFactor = Global::song().samples[pVoice->_instrument].PanFactor();
				}
				else {
					panFactor = 0.5f;
				}

				pVoice->_wave._rVolDest = panFactor;
				pVoice->_wave._lVolDest = 1-panFactor;

				if (pVoice->_wave._rVolDest > 0.5f)
				{
					pVoice->_wave._rVolDest = 0.5f;
				}
				if (pVoice->_wave._lVolDest > 0.5f)
				{
					pVoice->_wave._lVolDest = 0.5f;
				}

				pVoice->_wave._lVolDest *= pVoice->_wave._vol;
				pVoice->_wave._rVolDest *= pVoice->_wave._vol;
			}

			return triggered;

		}

		int Sampler::GenerateAudioInTicks(int /*startSample*/,  int numSamples)
		{
			if (!_mute)
			{
				Standby(false);
				for (int voice=0; voice<_numVoices; voice++)
				{
					// A correct implementation needs to take numsamples into account.
					// This will not be fixed to leave sampler compatible with old songs.
					PerformFx(voice);
				}
				int ns = numSamples;
				while (ns)
				{
					int nextevent = ns+1;
					for (int i=0; i < Global::song().SONGTRACKS; i++)
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
						for (int i=0; i < Global::song().SONGTRACKS; i++)
						{
							// come back to this
							if (TriggerDelay[i]._cmd)
							{
								TriggerDelayCounter[i] -= ns;
							}
						}
						for (int voice=0; voice<_numVoices; voice++)
						{
							VoiceWork(ns, voice);
						}
						ns = 0;
					}
					else
					{
						if (nextevent)
						{
							ns -= nextevent;
							for (int voice=0; voice<_numVoices; voice++)
							{
								VoiceWork(nextevent, voice);
							}
						}
						for (int i=0; i < Global::song().SONGTRACKS; i++)
						{
							// come back to this
							if (TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
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
									Tick(i,&TriggerDelay[i]);
									TriggerDelayCounter[i] = (RetriggerRate[i]*Global::player().SamplesPerRow())/256;
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelayCounter[i] = (RetriggerRate[i]*Global::player().SamplesPerRow())/256;
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
				UpdateVuAndStanbyFlag(numSamples);
			}

			else Standby(true);
			return numSamples;
		}

		void Sampler::VoiceWork(int numsamples, int voice)
		{
			helpers::dsp::resampler::work_func_type pResamplerWork;
			Voice* pVoice = &_voices[voice];
			float* pSamplesL = samplesV[0];
			float* pSamplesR = samplesV[1];
			float left_output;
			float right_output;

			pVoice->_sampleCounter += numsamples;

			 if ((pVoice->_triggerNoteDelay) && (pVoice->_sampleCounter >= pVoice->_triggerNoteDelay))
			{
				if ( pVoice->effCmd == SAMPLER_CMD_RETRIG && pVoice->effretTicks)
				{
					pVoice->_triggerNoteDelay = pVoice->_sampleCounter+ pVoice->effVal;
					pVoice->_envelope._step = (1.0f/Global::song()._pInstrument[pVoice->_instrument]->ENV_AT)*(44100.0f/Global::player().SampleRate());
					pVoice->_filterEnv._step = (1.0f/Global::song()._pInstrument[pVoice->_instrument]->ENV_F_AT)*(44100.0f/Global::player().SampleRate());
					pVoice->effretTicks--;
					pVoice->_wave._pos.QuadPart = 0;
					if ( pVoice->effretMode == 1 )
					{
						pVoice->_wave._lVolDest += pVoice->effretVol;
						pVoice->_wave._rVolDest += pVoice->effretVol;
					}
					else if (pVoice->effretMode == 2 )
					{
						pVoice->_wave._lVolDest *= pVoice->effretVol;
						pVoice->_wave._rVolDest *= pVoice->effretVol;
					}
				}
				else 
				{
					pVoice->_triggerNoteDelay=0;
				}
				pVoice->_envelope._stage = ENV_ATTACK;
			}
			else if (pVoice->_envelope._stage == ENV_OFF)
			{
				pVoice->_wave._lVolCurr = 0;
				pVoice->_wave._rVolCurr = 0;
				return;
			}
			else if ((pVoice->_triggerNoteOff) && (pVoice->_sampleCounter >= pVoice->_triggerNoteOff))
			{
				pVoice->_triggerNoteOff = 0;
				NoteOff(voice);
			}

			// If the sample has been deleted while playing...
			if (!Global::song().samples.IsEnabled(pVoice->_instrument))
			{
				pVoice->_envelope._stage = ENV_OFF;
				pVoice->_wave._lVolCurr = 0;
				pVoice->_wave._rVolCurr = 0;
				return;
			}

			pResamplerWork = _resampler.work;
			while (numsamples)
			{
				left_output=0;
				right_output=0;

				if (pVoice->_envelope._stage != ENV_OFF)
				{
					left_output = pResamplerWork(
						pVoice->_wave._pL + pVoice->_wave._pos.HighPart,
						pVoice->_wave._pos.HighPart, pVoice->_wave._pos.LowPart, pVoice->_wave._length, pVoice->resampler_data);
					if (pVoice->_wave._stereo)
					{
						right_output = pResamplerWork(
							pVoice->_wave._pR + pVoice->_wave._pos.HighPart,
							pVoice->_wave._pos.HighPart, pVoice->_wave._pos.LowPart, pVoice->_wave._length, pVoice->resampler_data);
					}

					// Amplitude section
					{
						TickEnvelope(voice);

						// calculate volume  (volume ramped)
						if(pVoice->_wave._lVolCurr>pVoice->_wave._lVolDest) {
							pVoice->_wave._lVolCurr-=0.005f;
							if(pVoice->_wave._lVolCurr<pVoice->_wave._lVolDest)
								pVoice->_wave._lVolCurr=pVoice->_wave._lVolDest;

						}
						else if(pVoice->_wave._lVolCurr<pVoice->_wave._lVolDest) {
							pVoice->_wave._lVolCurr+=0.005f;
							if(pVoice->_wave._lVolCurr>pVoice->_wave._lVolDest)
								pVoice->_wave._lVolCurr=pVoice->_wave._lVolDest;
						}
						if(pVoice->_wave._rVolCurr>pVoice->_wave._rVolDest) {
							pVoice->_wave._rVolCurr-=0.005f;
							if(pVoice->_wave._rVolCurr<pVoice->_wave._rVolDest)
								pVoice->_wave._rVolCurr=pVoice->_wave._rVolDest;

						}
						else if(pVoice->_wave._rVolCurr<pVoice->_wave._rVolDest) {
							pVoice->_wave._rVolCurr+=0.005f;
							if(pVoice->_wave._rVolCurr>pVoice->_wave._rVolDest)
								pVoice->_wave._rVolCurr=pVoice->_wave._rVolDest;
						}

					}
					// Filter section
					//
					if (pVoice->_filter.Type() != dsp::F_NONE)
					{
						TickFilterEnvelope(voice);
						int newcutoff = pVoice->_cutoff + helpers::math::round<int, float>(pVoice->_filterEnv._value*pVoice->_coModify);
						if (newcutoff < 0) {
							newcutoff = 0;
						}
						else if (newcutoff > 127) {
							newcutoff = 127;
						}
						pVoice->_filter.Cutoff(newcutoff);
						if (pVoice->_wave._stereo)
						{
							pVoice->_filter.WorkStereo(left_output, right_output);
						}
						else
						{
							left_output = pVoice->_filter.Work(left_output);
						}
					}

					if(!pVoice->_wave._stereo)
						right_output=left_output;
					right_output *= pVoice->_wave._rVolCurr*pVoice->_envelope._value;
					left_output *= pVoice->_wave._lVolCurr*pVoice->_envelope._value;

					// Move sample position
					{
						pVoice->_wave._pos.QuadPart += pVoice->_wave._speed;

						// Loop handler
						//
						if ((pVoice->_wave._loop) && (pVoice->_wave._pos.HighPart >= pVoice->_wave._loopEnd))
						{
							pVoice->_wave._pos.HighPart -= (pVoice->_wave._loopEnd - pVoice->_wave._loopStart);
						}
						if (pVoice->_wave._pos.HighPart >= pVoice->_wave._length)
						{
							pVoice->_envelope._stage = ENV_OFF;
						}
					}
				}
					
				*pSamplesL++ = *pSamplesL+left_output;
				*pSamplesR++ = *pSamplesR+right_output;
				numsamples--;
			}
		}


		void Sampler::TickFilterEnvelope(
			int voice)
		{
			Voice* pVoice = &_voices[voice];
			switch (pVoice->_filterEnv._stage)
			{
			case ENV_ATTACK:
				pVoice->_filterEnv._value += pVoice->_filterEnv._step;

				if (pVoice->_filterEnv._value > 1.0f)
				{
					pVoice->_filterEnv._stage = ENV_DECAY;
					pVoice->_filterEnv._value = 1.0f;
					pVoice->_filterEnv._step = ((1.0f - pVoice->_filterEnv._sustain) / Global::song()._pInstrument[pVoice->_instrument]->ENV_F_DT) * (44100.0f/Global::player().SampleRate());
				}
				break;
			case ENV_DECAY:
				pVoice->_filterEnv._value -= pVoice->_filterEnv._step;

				if (pVoice->_filterEnv._value < pVoice->_filterEnv._sustain)
				{
					pVoice->_filterEnv._value = pVoice->_filterEnv._sustain;
					pVoice->_filterEnv._stage = ENV_SUSTAIN;
				}
				break;
			case ENV_RELEASE:
				pVoice->_filterEnv._value -= pVoice->_filterEnv._step;

				if (pVoice->_filterEnv._value < 0)
				{
					pVoice->_filterEnv._value = 0;
					pVoice->_filterEnv._stage = ENV_OFF;
				}
				break;
			}
		}

		void Sampler::TickEnvelope(
			int voice)
		{
			Voice* pVoice = &_voices[voice];
			switch (pVoice->_envelope._stage)
			{
			case ENV_ATTACK:
				pVoice->_envelope._value += pVoice->_envelope._step;
				if (pVoice->_envelope._value > 1.0f)
				{
					pVoice->_envelope._value = 1.0f;
					pVoice->_envelope._stage = ENV_DECAY;
					pVoice->_envelope._step = ((1.0f - pVoice->_envelope._sustain)/Global::song()._pInstrument[pVoice->_instrument]->ENV_DT)*(44100.0f/Global::player().SampleRate());
				}
				break;
			case ENV_DECAY:
				pVoice->_envelope._value -= pVoice->_envelope._step;
				if (pVoice->_envelope._value < pVoice->_envelope._sustain)
				{
					pVoice->_envelope._value = pVoice->_envelope._sustain;
					pVoice->_envelope._stage = ENV_SUSTAIN;
				}
				break;
			case ENV_RELEASE:
			case ENV_FASTRELEASE:
				pVoice->_envelope._value -= pVoice->_envelope._step;
				if (pVoice->_envelope._value <= 0)
				{
					pVoice->_envelope._value = 0;
					pVoice->_envelope._stage = ENV_OFF;
				}
				break;
			default:break;
			}
		}

		void Sampler::NoteOff(
			int voice)
		{
			Voice* pVoice = &_voices[voice];
			if (pVoice->_envelope._stage != ENV_OFF)
			{
				pVoice->_envelope._stage = ENV_RELEASE;
				pVoice->_filterEnv._stage = ENV_RELEASE;
				pVoice->_envelope._step = (pVoice->_envelope._value/Global::song()._pInstrument[pVoice->_instrument]->ENV_RT)*(44100.0f/Global::player().SampleRate());
				pVoice->_filterEnv._step = (pVoice->_filterEnv._value/Global::song()._pInstrument[pVoice->_instrument]->ENV_F_RT)*(44100.0f/Global::player().SampleRate());
			}
		}

		void Sampler::NoteOffFast(
			int voice)
		{
			Voice* pVoice = &_voices[voice];
			if (pVoice->_envelope._stage != ENV_OFF)
			{
				pVoice->_envelope._stage = ENV_FASTRELEASE;
				pVoice->_envelope._step = pVoice->_envelope._value/OVERLAPTIME;

				pVoice->_filterEnv._stage = ENV_RELEASE;
				pVoice->_filterEnv._step = pVoice->_filterEnv._value/OVERLAPTIME;
			}
		}

		void Sampler::PerformFx(int voice)
		{
			// 4294967 stands for (2^30/250), meaning that
			//value 250 = (inc)decreases the speed in in 1/4th of the original (wave) speed each PerformFx call.
			std::int64_t shift;
			switch(_voices[voice].effCmd)
			{
				// 0x01 : Pitch Up
				case 0x01:
					shift=_voices[voice].effVal*4294967 * _voices[voice]._wave._samplerate/Global::player().SampleRate();
					_voices[voice]._wave._speed+=shift;
					_resampler.UpdateSpeed(_voices[voice].resampler_data,_voices[voice]._wave._speed);
				break;

				// 0x02 : Pitch Down
				case 0x02:
					shift=_voices[voice].effVal*4294967 * _voices[voice]._wave._samplerate/Global::player().SampleRate();
					_voices[voice]._wave._speed-=shift;
					if ( _voices[voice]._wave._speed < 0 ) _voices[voice]._wave._speed=0;
					_resampler.UpdateSpeed(_voices[voice].resampler_data,_voices[voice]._wave._speed);
				break;
				
				default:
				break;
			}
		}

		bool Sampler::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			//Old version had default C4 as false
			DefaultC4(false);
			std::uint32_t size=0;
			pFile->Read(&size,sizeof(size));
			if (size)
			{
				/// Version 0
				int temp;
				pFile->Read(&temp, sizeof(temp)); // numSubtracks
				_numVoices=temp;
				pFile->Read(&temp, sizeof(temp)); // quality

				switch (temp)
				{
					case 2:	_resampler.quality(helpers::dsp::resampler::quality::spline); break;
					case 3:	_resampler.quality(helpers::dsp::resampler::quality::sinc); break;
					case 0:	_resampler.quality(helpers::dsp::resampler::quality::zero_order); break;
					case 1:
					default: _resampler.quality(helpers::dsp::resampler::quality::linear);
				}
				if(size > 3*sizeof(std::uint32_t))
				{
					std::uint32_t internalversion;
					pFile->Read(internalversion);
					if (internalversion >= SAMPLERVERSION) {
						bool defaultC4;
						pFile->Read(&defaultC4, sizeof(bool)); // correct A4 frequency.
						DefaultC4(defaultC4);
					}
				}
			}
			return TRUE;
		}

		void Sampler::SaveSpecificChunk(RiffFile* pFile) 
		{
			std::uint32_t temp;
			std::uint32_t size = 3*sizeof(temp) + 1*sizeof(bool);
			pFile->Write(&size,sizeof(size));
			temp = _numVoices;
			pFile->Write(&temp, sizeof(temp)); // numSubtracks
			switch (_resampler.quality())
			{
				case helpers::dsp::resampler::quality::zero_order: temp = 0; break;
				case helpers::dsp::resampler::quality::spline: temp = 2; break;
				case helpers::dsp::resampler::quality::sinc: temp = 3; break;
				case helpers::dsp::resampler::quality::linear: //fallthrough
				default: temp = 1;
			}
			pFile->Write(&temp, sizeof(temp)); // quality

			pFile->Write(SAMPLERVERSION);
			bool defaultC4 = isDefaultC4();
			pFile->Write(&defaultC4, sizeof(bool)); // correct A4
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// old file format vomit. don't look at it.

		bool Sampler::Load(RiffFile* pFile)
		{
			int i;

			pFile->Read(&_editName,16);
			_editName[15] = 0;

			legacyWires.resize(MAX_CONNECTIONS);
			for(int i = 0; i < MAX_CONNECTIONS; i++) {
				pFile->Read(&legacyWires[i]._inputMachine,sizeof(legacyWires[i]._inputMachine));	// Incoming connections Machine number
			}
			for(int i = 0; i < MAX_CONNECTIONS; i++) {
				pFile->Read(&legacyWires[i]._outputMachine,sizeof(legacyWires[i]._outputMachine));	// Outgoing connections Machine number
			}
			for(int i = 0; i < MAX_CONNECTIONS; i++) {
				pFile->Read(&legacyWires[i]._inputConVol,sizeof(legacyWires[i]._inputConVol));	// Incoming connections Machine vol
				legacyWires[i]._wireMultiplier = 1.0f;
			}
			for(int i = 0; i < MAX_CONNECTIONS; i++) {
				pFile->Read(&legacyWires[i]._connection,sizeof(legacyWires[i]._connection));      // Outgoing connections activated
			}
			for(int i = 0; i < MAX_CONNECTIONS; i++) {
				pFile->Read(&legacyWires[i]._inputCon,sizeof(legacyWires[i]._inputCon));		// Incoming connections activated
			}
			pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
			pFile->Skip(2*sizeof(int)); // numInputs and numOutputs

			pFile->Read(&_panning, sizeof(_panning));
			Machine::SetPan(_panning);
			pFile->Skip(8*sizeof(int)); // SubTrack[]
			pFile->Read(&_numVoices, sizeof(_numVoices)); // numSubtracks

			if (_numVoices < 4)
			{
				// Psycle versions < 1.1b2 had polyphony per channel,not per machine.
				_numVoices = 8;
			}

			pFile->Read(&i, sizeof(int)); // interpol
			switch (i)
			{
				case 2:	_resampler.quality(helpers::dsp::resampler::quality::spline); break;
				case 3:	_resampler.quality(helpers::dsp::resampler::quality::sinc); break;
				case 0:	_resampler.quality(helpers::dsp::resampler::quality::zero_order); break;
				case 1:
				default: _resampler.quality(helpers::dsp::resampler::quality::linear);
			}
			pFile->Skip(69);

			return true;
		}
	}
}
