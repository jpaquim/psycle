// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psy::core::Sampler. based on psyclemfc revision 5903
#include <psycle/core/psycleCorePch.hpp>

#include "sampler.h"

#include "fileio.h"
#include "song.h"

namespace psy
{
	namespace core
	{
		std::string Sampler::_psName = "Sampler";
		InstPreview Sampler::wavprev;
		InstPreview Sampler::waved;

		static inline int alteRand(int x) {
			return (x*rand())/RAND_MAX;
		}


		Sampler::Sampler(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song)
		:
			Machine(callbacks, MACH_SAMPLER, MACHMODE_GENERATOR, id, song)
		{
			SetEditName("Sampler");
			SetAudioRange(32768.0f);
			//DefineStereoOutput(1);

			_resampler.SetQuality(dsp::R_LINEAR);
			for (int i=0; i<SAMPLER_MAX_POLYPHONY; i++)
			{
				_voices[i]._envelope._stage = ENV_OFF;
				_voices[i]._envelope._sustain = 0;
				_voices[i]._filterEnv._stage = ENV_OFF;
				_voices[i]._filterEnv._sustain = 0;
				_voices[i]._filter.Init(44100);
				_voices[i]._cutoff = 0;
				_voices[i]._sampleCounter = 0;
				_voices[i]._triggerNoteOff = 0;
				_voices[i]._triggerNoteDelay = 0;
				_voices[i]._channel = -1;
				_voices[i]._wave._lVolCurr = 0;
				_voices[i]._wave._rVolCurr = 0;

				_voices[i].effCmd = SAMPLER_CMD_NONE;
			}
			for (Instrument::id_type i(0); i < MAX_TRACKS; i++) lastInstrument[i]=255;
		}

		void Sampler::Init()
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

		int Sampler::GenerateAudioInTicks( int startSample, int numSamples )
		{
			assert(numSamples >= 0);
			const PlayerTimeInfo & timeInfo = callbacks->timeInfo();
			//cpu::cycles_type cost = cpu::cycles();
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
					for (int i=0; i < song()->tracks(); i++)
					{
						if (TriggerDelay[i].command() )
						{
							if (TriggerDelayCounter[i] < nextevent)
							{
								nextevent = TriggerDelayCounter[i];
							}
						}
					}
					if (nextevent > ns)
					{
						for (int i=0; i < song()->tracks(); i++)
						{
							// come back to this
							if (TriggerDelay[i].command() )
							{
								TriggerDelayCounter[i] -= ns;
							}
						}
						for (int voice=0; voice<_numVoices; voice++)
						{
							VoiceWork(startSample, ns, voice );
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
								VoiceWork(startSample, nextevent, voice );
							}
						}
						for (int i=0; i < song()->tracks(); i++)
						{
							// come back to this
							if (TriggerDelay[i].command() == PatternCmd::NOTE_DELAY)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i, TriggerDelay[i] );
									TriggerDelay[i].setCommand( 0 );
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i].command() == PatternCmd::RETRIGGER)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i, TriggerDelay[i] );
									TriggerDelayCounter[i] = static_cast<int>( (RetriggerRate[i]*timeInfo.samplesPerTick())/256 );
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i].command() == PatternCmd::RETR_CONT)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i, TriggerDelay[i] );
									TriggerDelayCounter[i] = static_cast<int>( (RetriggerRate[i]*timeInfo.samplesPerTick())/256 );
									int parameter = TriggerDelay[i].parameter() & 0x0f;
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

			//_cpuCost += cpu::cycles() - cost;
			_worked = true;
			return numSamples;
		}

		void Sampler::SetSampleRate(int sr)
		{
			Machine::SetSampleRate(sr);
			for (int i=0; i<_numVoices; i++) {
				_voices[i]._envelope._stage = ENV_OFF;
				_voices[i]._envelope._sustain = 0;
				_voices[i]._filterEnv._stage = ENV_OFF;
				_voices[i]._filterEnv._sustain = 0;
				_voices[i]._filter.Init(sr);
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
			Machine::Stop();
		}

		void Sampler::VoiceWork(int startSample, int numsamples, int voice )
		{
			assert(numsamples >= 0);
			const PlayerTimeInfo & timeInfo = callbacks->timeInfo();
			dsp::PRESAMPLERFN pResamplerWork;
			Voice* pVoice = &_voices[voice];
			float* pSamplesL = _pSamplesL+startSample;
			float* pSamplesR = _pSamplesR+startSample;
			float left_output;
			float right_output;

			pVoice->_sampleCounter += numsamples;

			if ( song()->IsInvalided() )
			{
				pVoice->_envelope._stage = ENV_OFF;
				return;
			}
			else if ((pVoice->_triggerNoteDelay) && (pVoice->_sampleCounter >= pVoice->_triggerNoteDelay))
			{
				if ( pVoice->effCmd == SAMPLER_CMD_RETRIG && pVoice->effretTicks)
				{
					pVoice->_triggerNoteDelay = pVoice->_sampleCounter+ pVoice->effVal;
					pVoice->_envelope._step = (1.0f/song()->_pInstrument[pVoice->_instrument]->ENV_AT)*(44100.0f/timeInfo.sampleRate());
					pVoice->_filterEnv._step = (1.0f/song()->_pInstrument[pVoice->_instrument]->ENV_F_AT)*(44100.0f/timeInfo.sampleRate());
					pVoice->effretTicks--;
					pVoice->_wave._pos = 0;
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
				NoteOff( voice );
			}

			pResamplerWork = _resampler._pWorkFn;
			while (numsamples)
			{
				left_output=0;
				right_output=0;

				if (pVoice->_envelope._stage != ENV_OFF)
				{
					left_output = pResamplerWork(pVoice->_wave._pL + (pVoice->_wave._pos >> 32),
										pVoice->_wave._pos>>32,
										pVoice->_wave._pos & 0xFFFFFFFF,
										pVoice->_wave._length);
					if (pVoice->_wave._stereo)
					{
						right_output = pResamplerWork(pVoice->_wave._pR + (pVoice->_wave._pos >> 32),
											pVoice->_wave._pos >> 32,
											pVoice->_wave._pos & 0xFFFFFFFF,
											pVoice->_wave._length);
					}

					// Filter section
					//
					if (pVoice->_filter._type < dsp::F_NONE)
					{
						TickFilterEnvelope( voice );
						pVoice->_filter._cutoff = pVoice->_cutoff + common::math::rounded(pVoice->_filterEnv._value*pVoice->_coModify);
						if (pVoice->_filter._cutoff < 0)
						{
							pVoice->_filter._cutoff = 0;
						}
						if (pVoice->_filter._cutoff > 127)
						{
							pVoice->_filter._cutoff = 127;
						}

						pVoice->_filter.Update();
						if (pVoice->_wave._stereo)
						{
							pVoice->_filter.WorkStereo(left_output, right_output);
						}
						else
						{
							left_output = pVoice->_filter.Work(left_output);
						}
					}

					TickEnvelope( voice );

					// calculate volume
					
					if(pVoice->_wave._lVolCurr<0)
						pVoice->_wave._lVolCurr=pVoice->_wave._lVolDest;
					if(pVoice->_wave._rVolCurr<0)
						pVoice->_wave._rVolCurr=pVoice->_wave._rVolDest;

					if(pVoice->_wave._lVolCurr>pVoice->_wave._lVolDest)
						pVoice->_wave._lVolCurr-=0.005f;
					if(pVoice->_wave._lVolCurr<pVoice->_wave._lVolDest)
						pVoice->_wave._lVolCurr+=0.005f;
					if(pVoice->_wave._rVolCurr>pVoice->_wave._rVolDest)
						pVoice->_wave._rVolCurr-=0.005f;
					if(pVoice->_wave._rVolCurr<pVoice->_wave._rVolDest)
						pVoice->_wave._rVolCurr+=0.005f;

					if(!pVoice->_wave._stereo)
						right_output=left_output;
					right_output *= pVoice->_wave._rVolCurr*pVoice->_envelope._value;
					left_output *= pVoice->_wave._lVolCurr*pVoice->_envelope._value;



					pVoice->_wave._pos += pVoice->_wave._speed;

					// Loop handler
					//
					if ((pVoice->_wave._loop) && ((pVoice->_wave._pos>>32) >= pVoice->_wave._loopEnd))
					{
						pVoice->_wave._pos -= (std::int64_t)(pVoice->_wave._loopEnd - pVoice->_wave._loopStart) << 32;
					}
					if ((pVoice->_wave._pos>>32) >= pVoice->_wave._length)
					{
						pVoice->_envelope._stage = ENV_OFF;
					}
				}
					
				*pSamplesL++ = *pSamplesL+left_output;
				*pSamplesR++ = *pSamplesR+right_output;
				numsamples--;
			}
		}

		void Sampler::Tick( )
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

		void Sampler::Tick( int channel, const PatternEvent & event )
		{
			if ( event.note() > psy::core::commands::release ) // don't process twk , twf of Mcm Commands
			{
				if ( event.command() == 0 || event.note() != 255) return; // Return in everything but commands!
			}
			if ( _mute ) return; // Avoid new note entering when muted.

			int voice;
			int useVoice = -1;


			PatternEvent data = event;

			if (data.instrument() >= 255)
			{
				data.setInstrument( lastInstrument[channel] );
				if (data.instrument() >= 255)
				{
					return;  // no previous sample
				}
			}
			else
			{
				data.setInstrument( lastInstrument[channel] = event.instrument() );
			}


			if ( data.note() < psy::core::commands::release ) // Handle Note On.
			{
				if ( song()->_pInstrument[data.instrument()]->waveLength == 0 ) return; // if no wave, return.

				for (voice=0; voice<_numVoices; voice++) // Find a voice to apply the new note
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
						switch (song()->_pInstrument[_voices[voice]._instrument]->_NNA)
						{
						case 0:
							NoteOffFast( voice );
							break;
						case 1:
							NoteOff( voice );
							break;
						}
						if ( useVoice == -1 ) { useVoice = voice; }
					}
				}
				if ( useVoice == -1 ) // No free voices. Assign first one.
				{ ///\todo This algorithm should be replace by a LRU lookup
					useVoice=0;
				}
				_voices[useVoice]._channel=channel;
			}
			else {
				for (voice=0;voice<_numVoices; voice++)  // Find the...
				{
					if (( _voices[voice]._channel == channel ) && // ...playing voice on current channel.
						(_voices[voice]._envelope._stage != ENV_OFF ) &&
						//(_voices[voice]._envelope._stage != ENV_RELEASE ) &&
						// Effects can STILL apply in this case.
						// Think on a slow fadeout and changing panning
						(_voices[voice]._envelope._stage != ENV_FASTRELEASE )) 
					{
						if ( data.note() == psy::core::commands::release ) NoteOff( voice );//  Handle Note Off
						useVoice=voice;
					}
				}
				if ( useVoice == -1 )
					// No playing note on this channel. Just go out.
					// Change it if you have channel commands.
					return;
			}
			// If you want to make a command that controls more than one voice (the entire channel, for
			// example) you'll need to change this. Otherwise, add it to VoiceTick().
			VoiceTick( useVoice, data ); 
		}


		bool Sampler::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size);
			if (size) {
				if (version > CURRENT_FILE_VERSION_MACD) {
					// data is from a newer format of psycle, it might be unsafe to load.
					pFile->Skip(size);
					return false;
				}
				else {
					std::int32_t temp;
					pFile->Read(temp); // numSubtracks
					_numVoices=temp;
					pFile->Read(temp); // quality
					
					switch (temp) {
					case 2:
						_resampler.SetQuality(dsp::R_SPLINE);
						break;
					case 3:
						_resampler.SetQuality(dsp::R_BANDLIM);
						break;
					case 0:
						_resampler.SetQuality(dsp::R_NONE);
						break;
					default:
					case 1:
						_resampler.SetQuality(dsp::R_LINEAR);
						break;
					}
				}
			}
			return true;
		}

		void Sampler::SaveSpecificChunk(RiffFile* pFile) const
		{
			std::int32_t temp;
			std::uint32_t size = 2 * sizeof temp;
			pFile->Write(size);
			temp = _numVoices;
			pFile->Write(temp); // numSubtracks
			switch (_resampler.GetQuality()) {
			case dsp::R_NONE:
				temp = 0;
				break;
			case dsp::R_LINEAR:
				temp = 1;
				break;
			case dsp::R_SPLINE:
				temp = 2;
				break;
			case dsp::R_BANDLIM:
				temp = 3;
				break;
			}
			pFile->Write(temp); // quality
		}

		int Sampler::VoiceTick( int voice, const PatternEvent & entry )
		{
			const PlayerTimeInfo & timeInfo = callbacks->timeInfo();
			PatternEvent pEntry = entry;

			Voice* pVoice = &_voices[voice];
			int triggered = 0;
			std::uint64_t w_offset = 0;

			if (song()->IsInvalided()) return 0;

			pVoice->_sampleCounter=0;
			pVoice->effCmd= pEntry.command();

			switch( pEntry.command() ) // DO NOT ADD here those commands that REQUIRE a note.
			{
				case SAMPLER_CMD_EXTENDED:
					if ((pEntry.parameter() & 0xf0) == SAMPLER_CMD_EXT_NOTEOFF)
					{
						pVoice->_triggerNoteOff = static_cast<int>( (timeInfo.samplesPerTick()/6)*(pEntry.parameter() & 0x0f) );
					}
					else if (((pEntry.parameter() & 0xf0) == SAMPLER_CMD_EXT_NOTEDELAY) && ((pEntry.parameter() & 0x0f) == 0 ))
					{
						pEntry.setCommand(0); 
						pEntry.setParameter(0);
					}
					break;
				case SAMPLER_CMD_PORTAUP:
					pVoice->effVal=pEntry.parameter();
					break;
				case SAMPLER_CMD_PORTADOWN:
					pVoice->effVal=pEntry.parameter();
					break;
			}
			

		//  All this mess should be really changed with classes using the "operator=" to "copy" values.

			int twlength = song()->_pInstrument[pEntry.instrument()]->waveLength;
			
			if (pEntry.note() < psy::core::commands::release && twlength > 0)
			{
				pVoice->_triggerNoteOff=0;
				pVoice->_instrument = pEntry.instrument();
				
				// Init filter synthesizer
				//
				pVoice->_filter.Init(timeInfo.sampleRate());

				if (song()->_pInstrument[pVoice->_instrument]->_RCUT)
				{
					pVoice->_cutoff = alteRand( song()->_pInstrument[pVoice->_instrument]->ENV_F_CO);
				}
				else
				{
					pVoice->_cutoff = song()->_pInstrument[pVoice->_instrument]->ENV_F_CO;
				}
				
				if (song()->_pInstrument[pVoice->_instrument]->_RRES)
				{
					pVoice->_filter._q = alteRand( song()->_pInstrument[pVoice->_instrument]->ENV_F_RQ);
				}
				else
				{
					pVoice->_filter._q = song()->_pInstrument[pVoice->_instrument]->ENV_F_RQ;
				}

				pVoice->_filter._type = (dsp::FilterType)song()->_pInstrument[pVoice->_instrument]->ENV_F_TP;
				pVoice->_coModify = (float) song()->_pInstrument[pVoice->_instrument]->ENV_F_EA;
				pVoice->_filterEnv._sustain = (float)song()->_pInstrument[pVoice->_instrument]->ENV_F_SL*0.0078125f;

				if (( pEntry.command() != SAMPLER_CMD_EXTENDED) || ((pEntry.parameter() & 0xf0) != SAMPLER_CMD_EXT_NOTEDELAY))
				{
					pVoice->_filterEnv._stage = ENV_ATTACK;
				}
				pVoice->_filterEnv._step = (1.0f/song()->_pInstrument[pVoice->_instrument]->ENV_F_AT)*(44100.0f/timeInfo.sampleRate());
				pVoice->_filterEnv._value = 0;
				
				// Init Wave
				//
				pVoice->_wave._pL = song()->_pInstrument[pVoice->_instrument]->waveDataL;
				pVoice->_wave._pR = song()->_pInstrument[pVoice->_instrument]->waveDataR;
				pVoice->_wave._stereo = song()->_pInstrument[pVoice->_instrument]->waveStereo;
				pVoice->_wave._length = twlength;
				
				// Init loop
				if (song()->_pInstrument[pVoice->_instrument]->waveLoopType)
				{
					pVoice->_wave._loop = true;
					pVoice->_wave._loopStart = song()->_pInstrument[pVoice->_instrument]->waveLoopStart;
					pVoice->_wave._loopEnd = song()->_pInstrument[pVoice->_instrument]->waveLoopEnd;
				}
				else
				{
					pVoice->_wave._loop = false;
				}
				
				// Init Resampler
				//
				if ( song()->_pInstrument[pVoice->_instrument]->_loop)
				{
					double const totalsamples = double(timeInfo.samplesPerTick()*song()->_pInstrument[pVoice->_instrument]->_lines);
					pVoice->_wave._speed = (std::int64_t)((pVoice->_wave._length/totalsamples)*4294967296.0f);
				}
				else
				{
					float const finetune = CValueMapper::Map_255_1(song()->_pInstrument[pVoice->_instrument]->waveFinetune);
					pVoice->_wave._speed = (std::int64_t)(pow(2.0f, ((pEntry.note()+song()->_pInstrument[pVoice->_instrument]->waveTune)-48 +finetune)/12.0f)*4294967296.0f*(44100.0f/timeInfo.sampleRate()));
				}
				

				// Handle wave_start_offset cmd
				//
				if (pEntry.command() == SAMPLER_CMD_OFFSET)
				{
					w_offset = pEntry.parameter()*pVoice->_wave._length;
					pVoice->_wave._pos = w_offset << 24;
				}
				else
				{
					pVoice->_wave._pos = 0;
				}

				// Calculating volume coef ---------------------------------------
				//
				pVoice->_wave._vol = (float)song()->_pInstrument[pVoice->_instrument]->waveVolume*0.01f;

				if (pEntry.command() == SAMPLER_CMD_VOLUME)
				{
					pVoice->_wave._vol *= CValueMapper::Map_255_1( pEntry.parameter() );
				}
				
				// Panning calculation -------------------------------------------
				//
				float panFactor;
				
				if (song()->_pInstrument[pVoice->_instrument]->_RPAN)
				{
					panFactor = (float)rand()/RAND_MAX;
				}
				else if ( pEntry.command() == SAMPLER_CMD_PANNING )
				{
					panFactor = CValueMapper::Map_255_1( pEntry.parameter() );
				}
				else {
					panFactor = CValueMapper::Map_255_1(song()->_pInstrument[pVoice->_instrument]->_pan);
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
				pVoice->_envelope._step = (1.0f/song()->_pInstrument[pVoice->_instrument]->ENV_AT)*(44100.0f/timeInfo.sampleRate());
				pVoice->_envelope._value = 0.0f;
				pVoice->_envelope._sustain = (float)song()->_pInstrument[pVoice->_instrument]->ENV_SL*0.01f;
				if (( pEntry.command() == SAMPLER_CMD_EXTENDED) && ((pEntry.parameter() & 0xf0) == SAMPLER_CMD_EXT_NOTEDELAY))
				{
					pVoice->_triggerNoteDelay = static_cast<int>( (timeInfo.samplesPerTick()/6)*(pEntry.parameter() & 0x0f) );
					pVoice->_envelope._stage = ENV_OFF;
				}
				else
				{
					if (pEntry.command() == SAMPLER_CMD_RETRIG && (pEntry.parameter() & 0x0f) > 0)
					{
						pVoice->effretTicks=(pEntry.parameter() & 0x0f); // number of Ticks.
						pVoice->effVal= static_cast<int>( (timeInfo.samplesPerTick()/(pVoice->effretTicks+1)) );
						
						int volmod = (pEntry.parameter() & 0xf0)>>4; // Volume modifier.
						switch (volmod) 
						{
							case 0:
							case 8: pVoice->effretVol = 0; pVoice->effretMode=0; break;
							case 1:
							case 2:
							case 3:
							case 4:
							case 5: pVoice->effretVol = (float)(std::pow(2.,volmod-1)/64); pVoice->effretMode=1; break;
							case 6: pVoice->effretVol = 0.66666666f; pVoice->effretMode=2; break;
							case 7: pVoice->effretVol = 0.5f; pVoice->effretMode=2; break;
							case 9:
							case 10:
							case 11:
							case 12:
							case 13: pVoice->effretVol = (float)(std::pow(2.,volmod-9)*(-1))/64; pVoice->effretMode=1; break;
							case 14: pVoice->effretVol = 1.5f; pVoice->effretMode=2; break;
							case 15: pVoice->effretVol = 2.0f; pVoice->effretMode=2; break;
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

			else if ((pEntry.command() == SAMPLER_CMD_VOLUME) || ( pEntry.command() == SAMPLER_CMD_PANNING ) )
			{
				// Calculating volume coef ---------------------------------------
				//
				pVoice->_wave._vol = (float)song()->_pInstrument[pVoice->_instrument]->waveVolume*0.01f;

				if ( pEntry.command() == SAMPLER_CMD_VOLUME ) pVoice->_wave._vol *= CValueMapper::Map_255_1(pEntry.parameter() );
				
				// Panning calculation -------------------------------------------
				//
				float panFactor;
				
				if (song()->_pInstrument[pVoice->_instrument]->_RPAN)
				{
					panFactor = (float)rand()/RAND_MAX;
				}
				else if ( pEntry.command() == SAMPLER_CMD_PANNING )
				{
					panFactor = CValueMapper::Map_255_1( pEntry.parameter() );
				}
				else
				{
					panFactor = CValueMapper::Map_255_1(song()->_pInstrument[pVoice->_instrument]->_pan);
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

		void Sampler::TickFilterEnvelope( int voice )
		{
			const PlayerTimeInfo & timeInfo = callbacks->timeInfo();

			Voice* pVoice = &_voices[voice];
			switch (pVoice->_filterEnv._stage)
			{
			case ENV_ATTACK:
				pVoice->_filterEnv._value += pVoice->_filterEnv._step;

				if (pVoice->_filterEnv._value > 1.0f)
				{
					pVoice->_filterEnv._stage = ENV_DECAY;
					pVoice->_filterEnv._value = 1.0f;
					pVoice->_filterEnv._step = ((1.0f - pVoice->_filterEnv._sustain) / song()->_pInstrument[pVoice->_instrument]->ENV_F_DT) * (44100.0f/timeInfo.sampleRate());
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

		void Sampler::TickEnvelope( int voice )
		{
			const PlayerTimeInfo & timeInfo = callbacks->timeInfo();

			Voice* pVoice = &_voices[voice];
			switch (pVoice->_envelope._stage)
			{
			case ENV_ATTACK:
				pVoice->_envelope._value += pVoice->_envelope._step;
				if (pVoice->_envelope._value > 1.0f)
				{
					pVoice->_envelope._value = 1.0f;
					pVoice->_envelope._stage = ENV_DECAY;
					pVoice->_envelope._step = ((1.0f - pVoice->_envelope._sustain)/song()->_pInstrument[pVoice->_instrument]->ENV_DT)*(44100.0f/timeInfo.sampleRate());
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

		void Sampler::NoteOff( int voice )
		{
			Voice* pVoice = &_voices[voice];
			if (pVoice->_envelope._stage != ENV_OFF)
			{
				const PlayerTimeInfo & timeInfo = callbacks->timeInfo();

				pVoice->_envelope._stage = ENV_RELEASE;
				pVoice->_filterEnv._stage = ENV_RELEASE;
				pVoice->_envelope._step = (pVoice->_envelope._value/song()->_pInstrument[pVoice->_instrument]->ENV_RT)*(44100.0f/timeInfo.sampleRate());
				pVoice->_filterEnv._step = (pVoice->_filterEnv._value/song()->_pInstrument[pVoice->_instrument]->ENV_F_RT)*(44100.0f/timeInfo.sampleRate());
			}
		}

		void Sampler::NoteOffFast( int voice )
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

		void Sampler::PerformFx( int voice )
		{
		std::int64_t shift;
			switch(_voices[voice].effCmd)
			{
				// 0x01 : Pitch Up
				case 0x01:
					shift=_voices[voice].effVal*4294967;
					_voices[voice]._wave._speed+=shift;
				break;

				// 0x02 : Pitch Down
				case 0x02:
					shift=_voices[voice].effVal*4294967;
					_voices[voice]._wave._speed-=shift;
					if ( _voices[voice]._wave._speed < 0 ) _voices[voice]._wave._speed=0;
				break;
				
				default:
				break;
			}
		}


		void Sampler::DoPreviews(int amount, float* pLeft, float* pRight)
		{
			//todo do better.. use a vector<InstPreview*> or something instead
			if(wavprev.IsEnabled())
			{
				wavprev.Work(pLeft, pRight, amount);
			}
			if(waved.IsEnabled())
			{
				waved.Work(pLeft, pRight, amount);
			}
		}
}
}
