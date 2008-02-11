/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief implementation file for psycle::host::Sampler.
#include <psycle/project.private.hpp>
#include "Sampler.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "FileIO.hpp"
#include "Configuration.hpp"
#include "global.hpp"
namespace psycle
{
	namespace host
	{
		char* Sampler::_psName = "Sampler";

		Sampler::Sampler(int index)
		{
			_macIndex = index;
			_numPars=0;
			_type = MACH_SAMPLER;
			_mode = MACHMODE_GENERATOR;
			sprintf(_editName, "Sampler");

			_resampler.SetQuality(helpers::dsp::R_LINEAR);
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
			for (int i = 0; i < MAX_TRACKS; i++)
			{
				lastInstrument[i]=255;
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

		void Sampler::Work(
			int numSamples)
		{
			cpu::cycles_type cost = cpu::cycles();
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
					for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
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
						for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
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
						for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
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
									TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer->SamplesPerRow())/256;
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
									TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer->SamplesPerRow())/256;
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
			_cpuCost += cpu::cycles() - cost;
			_worked = true;
		}

		void Sampler::Stop(void)
		{
			for (int i=0; i<_numVoices; i++)
			{
				NoteOffFast(i);
			}
		}

		bool Sampler::Load(RiffFile* pFile)
		{
			int i;
			char junk[256];
			memset(&junk, 0, sizeof(junk));

			pFile->Read(&_editName,16);
			_editName[15] = 0;

			pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
			pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
			pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
			pFile->Read(&_connection[0], sizeof(_connection));
			pFile->Read(&_inputCon[0], sizeof(_inputCon));
			pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
			pFile->Read(&_numInputs, sizeof(_numInputs));
			pFile->Read(&_numOutputs, sizeof(_numOutputs));

			pFile->Read(&_panning, sizeof(_panning));
			Machine::SetPan(_panning);
			pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
			pFile->Read(&_numVoices, sizeof(_numVoices)); // numSubtracks

			if (_numVoices < 4)
			{
				// Psycle versions < 1.1b2 had polyphony per channel,not per machine.
				_numVoices = 8;
			}

			pFile->Read(&i, sizeof(int)); // interpol
			switch (i)
			{
			case 2:
				_resampler.SetQuality(helpers::dsp::R_SPLINE);
				break;
			case 0:
				_resampler.SetQuality(helpers::dsp::R_NONE);
				break;
			default:
			case 1:
				_resampler.SetQuality(helpers::dsp::R_LINEAR);
				break;
			}

			pFile->Read(&junk[0], sizeof(int)); // outwet
			pFile->Read(&junk[0], sizeof(int)); // outdry

			pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
			pFile->Read(&junk[0], sizeof(int)); // distPosClamp
			pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
			pFile->Read(&junk[0], sizeof(int)); // distNegClamp

			pFile->Read(&junk[0], sizeof(char)); // sinespeed
			pFile->Read(&junk[0], sizeof(char)); // sineglide
			pFile->Read(&junk[0], sizeof(char)); // sinevolume
			pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
			pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

			pFile->Read(&junk[0], sizeof(int)); // delayTimeL
			pFile->Read(&junk[0], sizeof(int)); // delayTimeR
			pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
			pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

			pFile->Read(&junk[0], sizeof(int)); // filterCutoff
			pFile->Read(&junk[0], sizeof(int)); // filterResonance
			pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
			pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
			pFile->Read(&junk[0], sizeof(int)); // filterLfophase
			pFile->Read(&junk[0], sizeof(int)); // filterMode

			return true;
		}


		void Sampler::VoiceWork(int numsamples, int voice)
		{
			helpers::dsp::PRESAMPLERFN pResamplerWork;
			Voice* pVoice = &_voices[voice];
			float* pSamplesL = _pSamplesL;
			float* pSamplesR = _pSamplesR;
			float left_output;
			float right_output;

			pVoice->_sampleCounter += numsamples;

			if (Global::_pSong->Invalided)
			{
				pVoice->_envelope._stage = ENV_OFF;
				return;
			}
			else if ((pVoice->_triggerNoteDelay) && (pVoice->_sampleCounter >= pVoice->_triggerNoteDelay))
			{
				if ( pVoice->effCmd == SAMPLER_CMD_RETRIG && pVoice->effretTicks)
				{
					pVoice->_triggerNoteDelay = pVoice->_sampleCounter+ pVoice->effVal;
					pVoice->_envelope._step = (1.0f/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_AT)*(44100.0f/Global::pPlayer->SampleRate());
					pVoice->_filterEnv._step = (1.0f/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_AT)*(44100.0f/Global::pPlayer->SampleRate());
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

			pResamplerWork = _resampler._pWorkFn;
			while (numsamples)
			{
				left_output=0;
				right_output=0;

				if (pVoice->_envelope._stage != ENV_OFF)
				{
					left_output = pResamplerWork(
						pVoice->_wave._pL + pVoice->_wave._pos.HighPart,
						pVoice->_wave._pos.HighPart, pVoice->_wave._pos.LowPart, pVoice->_wave._length);
					if (pVoice->_wave._stereo)
					{
						right_output = pResamplerWork(
							pVoice->_wave._pR + pVoice->_wave._pos.HighPart,
							pVoice->_wave._pos.HighPart, pVoice->_wave._pos.LowPart, pVoice->_wave._length);
					}

					// Filter section
					//
					if (pVoice->_filter._type < dsp::F_NONE)
					{
						TickFilterEnvelope(voice);
						pVoice->_filter._cutoff = pVoice->_cutoff + helpers::math::rounded(pVoice->_filterEnv._value*pVoice->_coModify);
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

					TickEnvelope(voice);

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
					
				*pSamplesL++ = *pSamplesL+left_output;
				*pSamplesR++ = *pSamplesR+right_output;
				numsamples--;
			}
		}

		void Sampler::Tick()
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
			if ( pData->_note > notecommands::release ) // don't process twk , twf of Mcm Commands
			{
				if ( pData->_cmd == 0 || pData->_note != 255) return; // Return in everything but commands!
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
				if ( Global::_pSong->_pInstrument[data._inst]->waveLength == 0 ) return; // if no wave, return.

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
						switch (Global::_pSong->_pInstrument[_voices[voice]._instrument]->_NNA)
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
			unsigned __int64 w_offset = 0;

			if (Global::_pSong->Invalided) return 0;

			pVoice->_sampleCounter=0;
			pVoice->effCmd=pEntry->_cmd;

			switch(pEntry->_cmd) // DO NOT ADD here those commands that REQUIRE a note.
			{
				case SAMPLER_CMD_EXTENDED:
					if ((pEntry->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEOFF)
					{
						pVoice->_triggerNoteOff = (Global::pPlayer->SamplesPerRow()/6)*(pEntry->_parameter & 0x0f);
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

			int twlength = Global::_pSong->_pInstrument[pEntry->_inst]->waveLength;
			
			if (pEntry->_note < notecommands::release && twlength > 0)
			{
				pVoice->_triggerNoteOff=0;
				pVoice->_instrument = pEntry->_inst;
				
				// Init filter synthesizer
				//
				pVoice->_filter.Init(Global::pPlayer->SampleRate());

				if (Global::_pSong->_pInstrument[pVoice->_instrument]->_RCUT)
				{
					pVoice->_cutoff = alteRand(Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_CO);
				}
				else
				{
					pVoice->_cutoff = Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_CO;
				}
				
				if (Global::_pSong->_pInstrument[pVoice->_instrument]->_RRES)
				{
					pVoice->_filter._q = alteRand(Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_RQ);
				}
				else
				{
					pVoice->_filter._q = Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_RQ;
				}

				pVoice->_filter._type = (dsp::FilterType)Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_TP;
				pVoice->_coModify = (float)Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_EA;
				pVoice->_filterEnv._sustain = (float)Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_SL*0.0078125f;

				if (( pEntry->_cmd != SAMPLER_CMD_EXTENDED) || ((pEntry->_parameter & 0xf0) != SAMPLER_CMD_EXT_NOTEDELAY))
				{
					pVoice->_filterEnv._stage = ENV_ATTACK;
				}
				pVoice->_filterEnv._step = (1.0f/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_AT)*(44100.0f/Global::pPlayer->SampleRate());
				pVoice->_filterEnv._value = 0;
				
				// Init Wave
				//
				pVoice->_wave._pL = Global::_pSong->_pInstrument[pVoice->_instrument]->waveDataL;
				pVoice->_wave._pR = Global::_pSong->_pInstrument[pVoice->_instrument]->waveDataR;
				pVoice->_wave._stereo = Global::_pSong->_pInstrument[pVoice->_instrument]->waveStereo;
				pVoice->_wave._length = twlength;
				
				// Init loop
				if (Global::_pSong->_pInstrument[pVoice->_instrument]->waveLoopType)
				{
					pVoice->_wave._loop = true;
					pVoice->_wave._loopStart = Global::_pSong->_pInstrument[pVoice->_instrument]->waveLoopStart;
					pVoice->_wave._loopEnd = Global::_pSong->_pInstrument[pVoice->_instrument]->waveLoopEnd;
				}
				else
				{
					pVoice->_wave._loop = false;
				}
				
				// Init Resampler
				//
				if (Global::_pSong->_pInstrument[pVoice->_instrument]->_loop)
				{
					double const totalsamples = double(Global::pPlayer->SamplesPerRow()*Global::_pSong->_pInstrument[pVoice->_instrument]->_lines);
					pVoice->_wave._speed = (__int64)((pVoice->_wave._length/totalsamples)*4294967296.0f);
				}	
				else
				{
					float const finetune = helpers::CValueMapper::Map_255_1(Global::_pSong->_pInstrument[pVoice->_instrument]->waveFinetune);
					pVoice->_wave._speed = (__int64)(pow(2.0f, ((pEntry->_note+Global::_pSong->_pInstrument[pVoice->_instrument]->waveTune)-48 +finetune)/12.0f)*4294967296.0f*(44100.0f/Global::pPlayer->SampleRate()));
				}
				

				// Handle wave_start_offset cmd
				//
				if (pEntry->_cmd == SAMPLER_CMD_OFFSET)
				{
					w_offset = pEntry->_parameter*pVoice->_wave._length;
					pVoice->_wave._pos.QuadPart = (w_offset << 24);
				}
				else
				{
					pVoice->_wave._pos.QuadPart = 0;
				}

				// Calculating volume coef ---------------------------------------
				//
				pVoice->_wave._vol = (float)Global::_pSong->_pInstrument[pVoice->_instrument]->waveVolume*0.01f;

				if (pEntry->_cmd == SAMPLER_CMD_VOLUME)
				{
					pVoice->_wave._vol *= helpers::CValueMapper::Map_255_1(pEntry->_parameter);
				}
				
				// Panning calculation -------------------------------------------
				//
				float panFactor;
				
				if (Global::_pSong->_pInstrument[pVoice->_instrument]->_RPAN)
				{
					panFactor = (float)rand()*0.000030517578125f;
				}
				else if ( pEntry->_cmd == SAMPLER_CMD_PANNING )
				{
					panFactor = helpers::CValueMapper::Map_255_1(pEntry->_parameter);
				}
				else {
					panFactor = helpers::CValueMapper::Map_255_1(Global::_pSong->_pInstrument[pVoice->_instrument]->_pan);
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
				pVoice->_envelope._step = (1.0f/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_AT)*(44100.0f/Global::pPlayer->SampleRate());
				pVoice->_envelope._value = 0.0f;
				pVoice->_envelope._sustain = (float)Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_SL*0.01f;
				if (( pEntry->_cmd == SAMPLER_CMD_EXTENDED) && ((pEntry->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEDELAY))
				{
					pVoice->_triggerNoteDelay = (Global::pPlayer->SamplesPerRow()/6)*(pEntry->_parameter & 0x0f);
					pVoice->_envelope._stage = ENV_OFF;
				}
				else
				{
					if (pEntry->_cmd == SAMPLER_CMD_RETRIG && (pEntry->_parameter&0x0f) > 0)
					{
						pVoice->effretTicks=(pEntry->_parameter&0x0f); // number of Ticks.
						pVoice->effVal= (Global::pPlayer->SamplesPerRow()/(pVoice->effretTicks+1));
						
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
				pVoice->_wave._vol = (float)Global::_pSong->_pInstrument[pVoice->_instrument]->waveVolume*0.01f;

				if ( pEntry->_cmd == SAMPLER_CMD_VOLUME ) pVoice->_wave._vol *= helpers::CValueMapper::Map_255_1(pEntry->_parameter);
				
				// Panning calculation -------------------------------------------
				//
				float panFactor;
				
				if (Global::_pSong->_pInstrument[pVoice->_instrument]->_RPAN)
				{
					panFactor = (float)rand()*0.000030517578125f;
				}
				else if ( pEntry->_cmd == SAMPLER_CMD_PANNING )
				{
					panFactor = helpers::CValueMapper::Map_255_1(pEntry->_parameter);
				}
				else
				{
					panFactor = helpers::CValueMapper::Map_255_1(Global::_pSong->_pInstrument[pVoice->_instrument]->_pan);
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
					pVoice->_filterEnv._step = ((1.0f - pVoice->_filterEnv._sustain) / Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_DT) * (44100.0f/Global::pPlayer->SampleRate());
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
					pVoice->_envelope._step = ((1.0f - pVoice->_envelope._sustain)/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_DT)*(44100.0f/Global::pPlayer->SampleRate());
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
				pVoice->_envelope._step = (pVoice->_envelope._value/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_RT)*(44100.0f/Global::pPlayer->SampleRate());
				pVoice->_filterEnv._step = (pVoice->_filterEnv._value/Global::_pSong->_pInstrument[pVoice->_instrument]->ENV_F_RT)*(44100.0f/Global::pPlayer->SampleRate());
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
			__int64 shift;
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
	}
}