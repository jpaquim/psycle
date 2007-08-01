///\file
///\brief implementation file for psycle::host::Machine
#include <psycle/project.private.hpp>
#include "vsthost24.hpp"
#include "global.hpp"
#include "psycle.hpp"
#include "player.hpp"
#include "zap.hpp"
#include <psycle/host/loggers.hpp>

///\todo: these are required by the GetIn/OutLatency() functions. They should instead ask the player.
#include "Configuration.hpp"
#include "AudioDriver.hpp"

//This is so wrong. It's needed because of the loop inside the code for retrigger inside the Work() function.
#include "song.hpp"

///\todo:  When inserting a note in a pattern (editing), set the correct samplePos and ppqPos corresponding to the place the note is being put.
//		(LiveSlice is a good example of what happens if it isn't correct)

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		namespace loggers = psycle::loggers;

		namespace vst
		{
			float plugin::junk[STREAM_SIZE];
			using namespace seib::vst;
			int plugin::pitchWheelCentre(8191);

/*			Machine* host::CreateFromType(int _id, std::string _dllname)
			{
				//\todo;
				//return new;
			}
*/
			void host::CalcTimeInfo(long lMask)
			{
				///\todo: cycleactive and recording to a "Start()" function.
				// automationwriting and automationreading.
				//
				/*
				kVstTransportCycleActive	= 1 << 2,
				kVstTransportRecording		= 1 << 3,

				kVstAutomationWriting		= 1 << 6,
				kVstAutomationReading		= 1 << 7,
				*/

				//kVstCyclePosValid			= 1 << 12,	// start and end
				//	cyclestart // locator positions in quarter notes.
				//	cycleend   // locator positions in quarter notes.

				CVSTHost::CalcTimeInfo(lMask);
			}


			bool host::OnCanDo(CEffect &pEffect, const char *ptr)
			{
				using namespace seib::vst::HostCanDos;
				bool value =  CVSTHost::OnCanDo(pEffect,ptr);
				if (value) return value;
				else if (
					//||	(!strcmp(ptr, canDoReceiveVstEvents))	// "receiveVstEvents",
					//||	(!strcmp(ptr, canDoReceiveVstMidiEvent ))// "receiveVstMidiEvent",
					//||	(!strcmp(ptr, "receiveVstTimeInfo" ))// DEPRECATED

					(!strcmp(ptr, canDoReportConnectionChanges )) // "reportConnectionChanges",
					//||	(!strcmp(ptr, canDoAcceptIOChanges ))	// "acceptIOChanges",
					||(!strcmp(ptr, canDoSizeWindow ))		// "sizeWindow",

					//||	(!strcmp(ptr, canDoAsyncProcessing ))	// DEPRECATED
					//||	(!strcmp(ptr, canDoOffline ))			// "offline",
					//||	(!strcmp(ptr, "supportShell" ))		// DEPRECATED
					//||	(!strcmp(ptr, canDoEditFile ))			// "editFile",
					//||	(!strcmp(ptr, canDoSendVstMidiEventFlagIsRealtime ))
					)
					return true;
				return false;                           /* per default, no.                  */
			}

			long host::DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos)
			{
				//\todo: return the real tempo in the future, not always the current one
				// pos in Sample frames, return bpm* 10000
				return vstTimeInfo.tempo * 10000;
			}
			long host::OnGetOutputLatency(CEffect &pEffect)
			{
				//\todo : return Global::pPlayer->->LatencyInSamples();
				AudioDriver* pdriver = Global::pConfig->_pOutputDriver;
				return pdriver->GetOutputLatency();
			}
			long host::OnGetInputLatency(CEffect &pEffect)
			{
				//\todo : return Global::pPlayer->->LatencyInSamples();
				AudioDriver* pdriver = Global::pConfig->_pOutputDriver;
				return pdriver->GetInputLatency();
			}
			void host::Log(std::string message)
			{
				loggers::info(message);
			}

			///\todo: Get information about this function
			long host::OnGetAutomationState(CEffect &pEffect) { return kVstAutomationUnsupported; }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			plugin::plugin(LoadedAEffect &loadstruct)
				:CEffect(loadstruct)
				,queue_size(0)
				,requiresRepl(0)
				,requiresProcess(0)
				,rangeInSemis(2)
			{
				for (int midiChannel=0; midiChannel<16; midiChannel++)
				{
					NSActive[midiChannel] = false;
				    NSCurrent[midiChannel] = pitchWheelCentre;
					NSDestination[midiChannel] = pitchWheelCentre;
					NSTargetDistance[midiChannel] = 0;
					NSSamples[midiChannel] = 0;
					NSDelta[midiChannel] = 0;
					oldNote[midiChannel] = -1;
				}

				_nCols=0;
				if ( IsSynth())
				{
					_mode=MACHMODE_GENERATOR; _type=MACH_VST;
				}
				else 
				{
					_mode=MACHMODE_FX; _type=MACH_VSTFX;
				}
				// Compatibility hacks
				{
					if(uniqueId() == 0x41446c45 ) //"sc-101"
					{
						requiresRepl = true;
					}
				}

				std::memset(junk, 0, STREAM_SIZE * sizeof(float));
				for(int i(2) ; i < vst::max_io ; ++i)
				{
					inputs[i]=junk;
					outputs[i]=junk;
				}
				inputs[0] = _pSamplesL;
				inputs[1] = _pSamplesR;
				if (WillProcessReplace())
				{
					_pOutSamplesL = _pOutSamplesR = junk;
					outputs[0] = inputs[0];
					outputs[1] = inputs[1];
				}
				else
				{
				#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
					_pOutSamplesL = static_cast<float*>(_aligned_malloc(STREAM_SIZE*sizeof(float),16));
					_pOutSamplesR = static_cast<float*>(_aligned_malloc(STREAM_SIZE*sizeof(float),16));
				#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__GNU
					posix_memalign(_pSamplesL,16,STREAM_SIZE*sizeof(float));
					posix_memalign(_pSamplesR,16,STREAM_SIZE*sizeof(float));
				#else
					_pOutSamplesL = new float[STREAM_SIZE];
					_pOutSamplesR = new float[STREAM_SIZE];
				#endif
					dsp::Clear(_pOutSamplesL, STREAM_SIZE);
					dsp::Clear(_pOutSamplesR, STREAM_SIZE);
					outputs[0] = _pOutSamplesL;
					outputs[1] = _pOutSamplesR;
				}
				for(int i(0) ; i < MAX_TRACKS; ++i)
				{
					trackNote[i].key = 255; // No Note.
					trackNote[i].midichan = 0;
				}
				_sDllName= (char*)(loadstruct.pluginloader->sFileName);
				char temp[kVstMaxVendorStrLen];
				memset(temp,0,sizeof(temp));
				if ( GetPlugCategory() != kPlugCategShell )
				{
					// GetEffectName is the better option to GetProductString.
					// To the few that they show different values in these,
					// synthedit plugins show only "SyntheditVST" in GetProductString()
					// and others like battery 1 or psp-nitro, don't have GetProductString(),
					// so it's almost a no-go.
					if (GetEffectName(temp) && temp[0])_sProductName=temp;
					else if(GetProductString(temp) && temp[0]) _sProductName=temp;
					else
					{
						std::string temp;
						std::string::size_type pos;
						pos = _sDllName.rfind('\\');
						if(pos==std::string::npos)
							temp=_sDllName;
						else
							temp=_sDllName.substr(pos+1);
						_sProductName=temp.substr(0,temp.rfind('.'));
					}
				}
				else
				{
					std::string temp;
					std::string::size_type pos;
					pos = _sDllName.rfind('\\');
					if(pos==std::string::npos)
						temp=_sDllName;
					else
						temp=_sDllName.substr(pos+1);
					_sProductName=temp.substr(0,temp.rfind('.'));
				}
				if(GetVendorString(temp) && temp[0]) _sVendorName = temp;
				else _sVendorName = "Unknown vendor";
				std::strcpy(_editName,_sProductName.c_str());

			}

			plugin::~plugin()
			{
				if (aEffect)
				{
					if (!WillProcessReplace())
					{
					#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
						_aligned_free(_pOutSamplesL);
						_aligned_free(_pOutSamplesR);
					#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__GNU
						free(_pOutSamplesL);
						free(_pOutSamplesR);
					#else
						delete [] _pOutSamplesL;
						delete [] _pOutSamplesR;
					#endif
						_pOutSamplesL = _pOutSamplesR=0;
					}
				}
			}

			void plugin::GetParamValue(int numparam, char * parval)
			{
				try
				{
					if(numparam < numParams())
					{
						if(!DescribeValue(numparam, parval))
						{
							std::sprintf(parval,"%.0f",GetParameter(numparam) * quantization);
						}
					}
					else std::strcpy(parval,"Out of Range");
				}
				catch(const std::exception &)
				{
					// [bohan]
					// exception blocked here for now,
					// but we really should do something...
					//throw;
					std::strcpy(parval, "fucked up");
				}
			}

			bool plugin::DescribeValue(int parameter, char * psTxt)
			{
				if(parameter >= 0 && parameter < numParams())
				{
					char par_display[kVstMaxProgNameLen+1]={0}; 
					char par_label[kVstMaxProgNameLen+1]={0};
					GetParamDisplay(parameter,par_display);
					GetParamLabel(parameter,par_label);
					std::sprintf(psTxt, "%s(%s)", par_display, par_label);
					return true;
				}
				else std::sprintf(psTxt, "Invalid NumParams Value");
				return false;
			}
			bool plugin::LoadSpecificChunk(RiffFile * pFile, int version)
			{
				UINT size;
				unsigned char _program;
				pFile->Read(&size, sizeof size );
				if(size)
				{
					if(version > CURRENT_FILE_VERSION_MACD)
					{
						pFile->Skip(size);
						std::ostringstream s; s
							<< version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
							<< "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
						MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
					UINT count;
					pFile->Read(&_program, sizeof _program);
					pFile->Read(&count, sizeof count);
					size -= sizeof _program + sizeof count + sizeof(float) * count;
					if(!size)
					{
						BeginSetProgram();
						SetProgram(_program);
						for(UINT i(0) ; i < count ; ++i)
						{
							float temp;
							pFile->Read(&temp, sizeof temp);
							SetParameter(i, temp);
						}
						EndSetProgram();
					}
					else
					{
						BeginSetProgram();
						SetProgram(_program);
						EndSetProgram();
						MainsChanged(true);
						pFile->Skip(sizeof(float) *count);
						if(ProgramIsChunk())
						{
							char * data(new char[size]);
							pFile->Read(data, size); // Number of parameters
							SetChunk(data,size);
							zapArray(data);
						}
						else
						{
							// there is a data chunk, but this machine does not want one.
							pFile->Skip(size);
							return false;
						}
						/// Do it again for VST's that "forget" the program.
						BeginSetProgram();
						SetProgram(_program);
						EndSetProgram();
						MainsChanged(true);
					}
				}
				return true;
			}

			void plugin::SaveSpecificChunk(RiffFile * pFile) 
			{
				UINT count(numParams());
				unsigned char _program=0;
				UINT size(sizeof _program + sizeof count);
				UINT chunksize(0);
				char * pData(0);
				bool b = ProgramIsChunk();
				if(b)
				{
					// can't do this! we have  a thing called "autosave" and everything is stopped then.
//					MainsChanged(false);
					count=0;
					chunksize = GetChunk((void**)&pData);
					size+=chunksize;
//					MainsChanged(true);
				}
				else
				{
					 size+=sizeof(float) * count;
				}
				pFile->Write(&size, sizeof size);
				_program = static_cast<unsigned char>(GetProgram());
				pFile->Write(&_program, sizeof _program);
				pFile->Write(&count, sizeof count);

				if(b)
				{
					pFile->Write(pData, chunksize);
				}
				else
				{
					for(UINT i(0); i < count; ++i)
					{
						float temp = GetParameter(i);
						pFile->Write(&temp, sizeof temp);
					}
				}
			}

			VstMidiEvent* plugin::reserveVstMidiEvent() {
				assert(queue_size>=0 && queue_size <= MAX_VST_EVENTS);
				if(queue_size >= MAX_VST_EVENTS) {
					loggers::info("vst::plugin warning: event buffer full, midi message could not be sent to plugin");
					return NULL;
				}
				return &midievent[queue_size++];
			}

			VstMidiEvent* plugin::reserveVstMidiEventAtFront() {
				assert(queue_size>=0 && queue_size <= MAX_VST_EVENTS);
				if(queue_size >= MAX_VST_EVENTS) {
					loggers::info("vst::plugin warning: event buffer full, midi message could not be sent to plugin");
					return NULL;
				}
				for(int i=queue_size; i > 0 ; --i) midievent[i] = midievent[i - 1];
				queue_size++;
				return &midievent[0];
			}


			bool plugin::AddMIDI(unsigned char data0, unsigned char data1, unsigned char data2,unsigned int sampleoffset)
			{
				VstMidiEvent * pevent(reserveVstMidiEvent());
				if(!pevent) return false;
				pevent->type = kVstMidiType;
				pevent->byteSize = 24;
				pevent->deltaFrames = sampleoffset;
				pevent->flags = 0;
				pevent->detune = 0;
				pevent->noteLength = 0;
				pevent->noteOffset = 0;
				pevent->reserved1 = 0;
				pevent->reserved2 = 0;
				pevent->noteOffVelocity = 0;
				pevent->midiData[0] = data0;
				pevent->midiData[1] = data1;
				pevent->midiData[2] = data2;
				pevent->midiData[3] = 0;
				return true;
			}
			bool plugin::AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel,unsigned int sampleoffset, bool slide)
			{
				if(trackNote[channel].key != notecommands::empty && !slide)
					AddNoteOff(channel, trackNote[channel].key, true);

				if(AddMIDI(0x90 | midichannel /*Midi On*/, key, velocity,sampleoffset)) {
					note thisnote;
					thisnote.key = key;
					thisnote.midichan = midichannel;
					trackNote[channel] = thisnote;
					return true;
				}
				return false;
			}

			bool plugin::AddNoteOff(unsigned char channel, unsigned char midichannel, bool addatStart,unsigned int sampleoffset)
			{
				if(trackNote[channel].key == notecommands::empty)
					return false;
				VstMidiEvent * pevent;
				if( addatStart)
				{
					// PATCH:
					// When a new note enters, it adds a note-off for the previous note playing in
					// the track (this is ok). But if you have like: A-4 C-5 and in the next line
					// C-5 E-5 , you will only hear E-5.
					// Solution: Move the NoteOffs at the beginning.
					pevent = reserveVstMidiEventAtFront();
				}
				else 
				{
					pevent = reserveVstMidiEvent();
				}
				if(!pevent)
					return false;
				pevent->type = kVstMidiType;
				pevent->byteSize = 24;
				pevent->deltaFrames = sampleoffset;
				pevent->flags = 0;
				pevent->detune = 0;
				pevent->noteLength = 0;
				pevent->noteOffset = 0;
				pevent->reserved1 = 0;
				pevent->reserved2 = 0;
				pevent->noteOffVelocity = 0;
				pevent->midiData[0] = 0x80 | static_cast<unsigned char>(trackNote[channel].midichan); //midichannel; // Midi Off
				pevent->midiData[1] = trackNote[channel].key;
				pevent->midiData[2] = 0;
				pevent->midiData[3] = 0;

				note thisnote;
				thisnote.key = 255;
				thisnote.midichan = 0;
				trackNote[channel] = thisnote;
				return true;
			}
			void plugin::SendMidi()
			{
				assert(queue_size >= 0);
				assert(queue_size <= MAX_VST_EVENTS);


				if(queue_size > 0)
				{
					// Prepare MIDI events and free queue dispatching all events
					mevents.numEvents = queue_size;
					mevents.reserved = 0;
					for(int q(0) ; q < queue_size ; ++q) {
#ifndef NDEBUG

						// assert that events are sent in order.
						// although the standard doesn't require this,
						// many synths rely on this.
						if(q>0) {
							assert(midievent[q-1].deltaFrames <= 
								midievent[q].deltaFrames);
						}

/*						// assert that the note sequence is well-formed,
						// which means, no note-offs happen without a
						// corresponding preceding note-on.
						switch(midievent[q].midiData[0]&0xf0) {
						case 0x90: // note-on
							note_checker_.note_on(midievent[q].midiData[1],
								midievent[q].midiData[0]&0x0f);
							break;
						case 0x80: // note-off
							note_checker_.note_off(midievent[q].midiData[1],
								midievent[q].midiData[0]&0x0f);
							break;
						}
*/
#endif

						mevents.events[q] = (VstEvent*) &midievent[q];
					}
					//Finally Send the events.
					queue_size = 0;
//					WantsMidi(ProcessEvents(reinterpret_cast<VstEvents*>(&mevents)));
					ProcessEvents(reinterpret_cast<VstEvents*>(&mevents));
				}
			}
			void plugin::PreWork(int numSamples,bool clear)
			{
				Machine::PreWork(numSamples,clear);
				if(!WillProcessReplace())
				{
					dsp::Clear(_pOutSamplesL, numSamples);
					dsp::Clear(_pOutSamplesR, numSamples);
				}

				for (int midiChannel=0; midiChannel<16; midiChannel++)
				{
					if(NSActive[midiChannel])
					{
						NSSamples[midiChannel]=NSSamples[midiChannel]%TWEAK_SLIDE_SAMPLES;
						int ns = numSamples + NSSamples[midiChannel] - TWEAK_SLIDE_SAMPLES;
						while(ns >= 0)
						{
							NSCurrent[midiChannel] += NSDelta[midiChannel];
							if(
								(NSDelta[midiChannel] > 0 && NSCurrent[midiChannel] >= NSDestination[midiChannel]) ||
								(NSDelta[midiChannel] < 0 && NSCurrent[midiChannel] <= NSDestination[midiChannel]))
							{
								NSCurrent[midiChannel] = NSDestination[midiChannel];
								NSDelta[midiChannel] = 0;
								NSActive[midiChannel] = false;
							}
							//currently just working with midichannel 0
							//TRACE( "semislide of channel %i = %d\n", midiChannel, NSCurrent[midiChannel]);
							AddMIDI(0xE0 + midiChannel,LSB(NSCurrent[midiChannel]),MSB(NSCurrent[midiChannel]),NSSamples[midiChannel]);
							 
							NSSamples[midiChannel]+=min(TWEAK_SLIDE_SAMPLES,ns);
							ns-=TWEAK_SLIDE_SAMPLES;
						}
					}
				}
			}
			void plugin::Tick(int channel, PatternEntry * pData)
			{
				const int note = pData->_note;
				int midiChannel;
				if (pData->_inst == 0xFF) midiChannel = 0;
				else midiChannel = pData->_inst & 0x0F;

				if(note == notecommands::tweak || note == notecommands::tweakeffect) // Tweak Command
				{
					const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
					SetParameter(pData->_inst, value);
					Global::pPlayer->Tweaker = true;
				}
				else if(note == notecommands::tweakslide)
				{
					int i;
					if(TWSActive)
					{
						for(i = 0 ; i < MAX_TWS; ++i) if(TWSInst[i] == pData->_inst && TWSDelta[i]) break;
						if(i == MAX_TWS) for(i = 0 ; i < MAX_TWS; ++i) if(!TWSDelta[i]) break;
					}
					else for(i = MAX_TWS - 1 ; i > 0 ; --i) TWSDelta[i] = 0;
					if(i < MAX_TWS)
					{
						TWSDestination[i] = ((pData->_cmd * 256) + pData->_parameter) / 65535.0f;
						TWSInst[i] = pData->_inst;
						TWSCurrent[i] = GetParameter(TWSInst[i]);
						TWSDelta[i] = ((TWSDestination[i] - TWSCurrent[i]) * TWEAK_SLIDE_SAMPLES) / Global::pPlayer->SamplesPerRow();
						TWSSamples = 0;
						TWSActive = true;
					}
					else
					{
						// we have used all our slots, just send a twk
						const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
						SetParameter(pData->_inst, value);
					}
					Global::pPlayer->Tweaker = true;
				}
				else if(pData->_note == notecommands::midicc) // Mcm (MIDI CC) Command
				{
					AddMIDI(pData->_inst, pData->_cmd, pData->_parameter);
				}
				else
				{
					if(pData->_cmd == 0xC1) //set the pitch bend range
					{
						rangeInSemis = pData->_parameter;
					}
					else if(pData->_cmd == 0xC2) //Panning
					{
						AddMIDI(0xB0 | midiChannel, 0x0A,pData->_parameter*0.5f);
					}

					if(note < notecommands::release) // Note on
					{
						int semisToSlide(0);

						if((pData->_cmd == 0x10) && ((pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)) // _OLD_ MIDI Command
						{
							AddMIDI(pData->_inst, note, pData->_parameter);
						}
						else if((pData->_cmd & 0xF0) == 0xD0 || (pData->_cmd & 0xF0) == 0xE0) //semislide
						{
							if (NSCurrent[midiChannel] != pitchWheelCentre)
							{
								AddMIDI(0xE0 + midiChannel,LSB(pitchWheelCentre),MSB(pitchWheelCentre));
							}
							///\todo: sorry???
							currentSemi[midiChannel] = 0;

							if ((pData->_cmd & 0xF0) == 0xD0) //pitch slide down
							{
								semisToSlide = -(pData->_cmd & 0x0F);
								if (semisToSlide < (-rangeInSemis - currentSemi[midiChannel]))
									semisToSlide = (-rangeInSemis - currentSemi[midiChannel]);
							}
							else							  //pitch slide up
							{
								semisToSlide = pData->_cmd & 0x0F;
								if (semisToSlide > (rangeInSemis - currentSemi[midiChannel]))
									semisToSlide = (rangeInSemis - currentSemi[midiChannel]);
							}

							int speedToSlide = pData->_parameter;
							
							NSCurrent[midiChannel] = pitchWheelCentre + (currentSemi[midiChannel] * (pitchWheelCentre / rangeInSemis));
							NSDestination[midiChannel] = NSCurrent[midiChannel] + ((pitchWheelCentre / rangeInSemis) * semisToSlide);						
							NSDelta[midiChannel] = ((NSDestination[midiChannel] - NSCurrent[midiChannel]) * (speedToSlide*2)) / Global::pPlayer->SamplesPerRow();
							NSSamples[midiChannel] = 0;
							NSActive[midiChannel] = true;
							currentSemi[midiChannel] = currentSemi[midiChannel] + semisToSlide;
							AddNoteOn(channel, note, 127, midiChannel);
						}
						else if((pData->_cmd == 0xC3) && (oldNote[midiChannel]!=-1))//slide to note
						{
							semisToSlide = note - oldNote[midiChannel];
							int speedToSlide = pData->_parameter;
							NSDestination[midiChannel] = NSDestination[midiChannel] + ((pitchWheelCentre / rangeInSemis) * semisToSlide);
							NSTargetDistance[midiChannel] = (NSDestination[midiChannel] - NSCurrent[midiChannel]);
							NSDelta[midiChannel] = (NSTargetDistance[midiChannel] * (speedToSlide*2)) / Global::pPlayer->SamplesPerRow();
							NSSamples[midiChannel] = 0;
							NSActive[midiChannel] = true;
						}
						else //basic note on
						{
							if (NSCurrent[midiChannel] != pitchWheelCentre)
							{
								NSActive[midiChannel] = false;
								AddMIDI(0xE0 | midiChannel,LSB(pitchWheelCentre),MSB(pitchWheelCentre));
								NSCurrent[midiChannel] = pitchWheelCentre;
								NSDestination[midiChannel] = NSCurrent[midiChannel];
								currentSemi[midiChannel] = 0;
							}
							//AddMIDI(0xB0 | midiChannel,0x07,127); // channel volume. Reset it for the new note.
							AddNoteOn(channel,note,(pData->_cmd == 0x0C)?pData->_parameter/2:127,midiChannel,0,(pData->_mach==0xFF));
						}
						if (((pData->_cmd & 0xF0) == 0xD0) || ((pData->_cmd & 0xF0) == 0xE0))
							oldNote[midiChannel] = note + semisToSlide;								
						else
							oldNote[midiChannel] = note;
					}
					else if(note == notecommands::release) // Note Off. 
					{
						AddNoteOff(channel, midiChannel);
						oldNote[midiChannel] = -1;
					}
					else if(pData->_note == notecommands::empty)
					{
						int semisToSlide(0);

						if (pData->_cmd == 0x10) // _OLD_ MIDI Command
						{
							AddMIDI(pData->_inst,pData->_parameter);
						}
//						else if (pData->_cmd == 0x0C) // channel volume.
//						{
///							AddMIDI(0xB0 | midiChannel,0x07,pData->_parameter*0.5f);
//						}
						else if (pData->_cmd == 0x0C) // channel aftertouch.
						{
							AddMIDI(0xD0 | midiChannel,pData->_parameter*0.5f);
						}
						else if(pData->_cmd == 0xC3) //slide to note . Used to change the speed.
						{
							int speedToSlide = pData->_parameter;
							NSDelta[midiChannel] = (NSTargetDistance[midiChannel] * (speedToSlide*2)) / Global::pPlayer->SamplesPerRow();
						}
						else if((pData->_cmd & 0xF0) == 0xD0 || (pData->_cmd & 0xF0) == 0xE0) //semislide
						{
							if (NSCurrent[midiChannel] != NSDestination[midiChannel])
							{
								AddMIDI(0xE0 | midiChannel,LSB(NSDestination[midiChannel]),MSB(NSDestination[midiChannel]));
							}

							if ((pData->_cmd & 0xF0) == 0xD0) //pitch slide down
							{
								semisToSlide = -(pData->_cmd & 0x0F);
								if (semisToSlide < (-rangeInSemis - currentSemi[midiChannel]))
									semisToSlide = (-rangeInSemis - currentSemi[midiChannel]);
							}
							else							  //pitch slide up
							{
								semisToSlide = pData->_cmd & 0x0F;
								if (semisToSlide > (rangeInSemis - currentSemi[midiChannel]))
									semisToSlide = (rangeInSemis - currentSemi[midiChannel]);
							}

							int speedToSlide = pData->_parameter;
							
							NSCurrent[midiChannel] = pitchWheelCentre + (currentSemi[midiChannel] * (pitchWheelCentre / rangeInSemis));
							NSDestination[midiChannel] = NSCurrent[midiChannel] + ((pitchWheelCentre / rangeInSemis) * semisToSlide);						
							NSDelta[midiChannel] = ((NSDestination[midiChannel] - NSCurrent[midiChannel]) * (speedToSlide*2)) / Global::pPlayer->SamplesPerRow();
							NSSamples[midiChannel] = 0;
							NSActive[midiChannel] = true;
							currentSemi[midiChannel] = currentSemi[midiChannel] + semisToSlide;
						}
						if (((pData->_cmd & 0xF0) == 0xD0) || ((pData->_cmd & 0xF0) == 0xE0))
							oldNote[midiChannel] = oldNote[midiChannel] + semisToSlide;								
					}
				}
			}

			void plugin::Stop()
			{
				for(int i(0) ; i < 16 ; ++i) AddMIDI(0xb0 + i, 0x7b); // All Notes Off
				for(int i(0) ; i < MAX_TRACKS ; ++i) AddNoteOff(i);
			}

			void plugin::Work(int numSamples)
			{
				if(_mode != MACHMODE_GENERATOR) Machine::Work(numSamples);
				else
				{
					if (!_mute) Standby(false);
					else Standby(true);
				}
				cpu::cycles_type cost = cpu::cycles();
				if(!_mute && ((!Standby() && !Bypass()) || bCanBypass))
				{
/*					The following is now being done in the OnTimer() thread of the UI.
					if(bNeedIdle) 
					{
						try
						{
							Idle();
						}
						catch (...)
						{
							// o_O`
						}

					}
*/
					try
					{
						if (WantsMidi()) SendMidi();
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					if(numInputs() == 1)
					{
						dsp::Add(inputs[1],inputs[0],numSamples,0.5f);
					}

					///\todo: Move all this messy retrigger code to somewhere else. (it is repeated in each machine subclass)
					// Store temporary pointers so that we can increase the address in the retrigger code
					float * tempinputs[vst::max_io];
					float * tempoutputs[vst::max_io];
					for(int i(0) ; i < vst::max_io; ++i)
					{
						tempinputs[i] = inputs[i];
						tempoutputs[i] = outputs[i];
					}
					int ns(numSamples);
					while(ns)
					{
						int nextevent;
						if(TWSActive) nextevent = TWSSamples; else nextevent = ns + 1;
						for(int i(0) ; i < Global::_pSong->SONGTRACKS ; ++i)
						{
							if(TriggerDelay[i]._cmd && TriggerDelayCounter[i] < nextevent) nextevent = TriggerDelayCounter[i];
						}
						if(nextevent > ns)
						{
							if(TWSActive) TWSSamples -= ns;
							for(int i(0) ; i < Global::_pSong->SONGTRACKS; ++i)
							{
								// come back to this
								if(TriggerDelay[i]._cmd) TriggerDelayCounter[i] -= ns;
							}
							try
							{
								if(WillProcessReplace())
									ProcessReplacing(tempinputs, tempoutputs, ns);
								else
									Process(tempinputs, tempoutputs, ns);
							}
							catch(const std::exception &)
							{
								// o_O`
							}
							ns = 0;
						}
						else
						{
							if(nextevent)
							{
								ns -= nextevent;
								try
								{
									if(WillProcessReplace())
										ProcessReplacing(tempinputs, tempoutputs, nextevent);
									else
										Process(tempinputs, tempoutputs, nextevent);
								}
								catch(const std::exception &)
								{
									// o_O`
								}
								for(int i(0) ; i < vst::max_io ; ++i)
								{
									tempinputs[i]+=nextevent;
									tempoutputs[i]+=nextevent;
								}
							}
							if(TWSActive)
							{
								if(TWSSamples == nextevent)
								{
									int activecount = 0;
									TWSSamples = TWEAK_SLIDE_SAMPLES;
									for(int i(0) ; i < MAX_TWS; ++i)
									{
										if(TWSDelta[i])
										{
											TWSCurrent[i] += TWSDelta[i];
											if(
												(TWSDelta[i] > 0 && TWSCurrent[i] >= TWSDestination[i]) ||
												(TWSDelta[i] < 0 && TWSCurrent[i] <= TWSDestination[i]))
											{
												TWSCurrent[i] = TWSDestination[i];
												TWSDelta[i] = 0;
											}
											else ++activecount;
											SetParameter(TWSInst[i],TWSCurrent[i]);
										}
									}
									if(activecount == 0) TWSActive = false;
								}
							}
							for(int i(0) ; i < Global::_pSong->SONGTRACKS; ++i)
							{
								// come back to this
								if(TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelay[i]._cmd = 0;
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if(TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelayCounter[i] = (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if(TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelayCounter[i] = (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
										int parameter(TriggerDelay[i]._parameter & 0x0f);
										if(parameter < 9) RetriggerRate[i] += 4 * parameter;
										else
										{
											RetriggerRate[i] -= 2 * (16 - parameter);
											if(RetriggerRate[i] < 16) RetriggerRate[i] = 16;
										}
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
							}
						}
					}
					try
					{
						if(numOutputs() == 1) dsp::Mov(outputs[0],outputs[1], numSamples);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					if (!WillProcessReplace())
					{
						// We need the output in _pSamples, so we invert theThis is an inversion of the pointers
						// pointers to avoid copying _pOutSamples into _pSamples
						float* const tempSamplesL = inputs[0];
						float* const tempSamplesR = inputs[1];	
						_pSamplesL = inputs[0] = outputs[0];
						_pSamplesR = inputs[1] = outputs[1];
						_pOutSamplesL = outputs[0] = tempSamplesL;
						_pOutSamplesR = outputs[1] = tempSamplesR;
						/*
						memcpy(inputs[0],outputs[0],numSamples*sizeof(float));
						memcpy(inputs[1],outputs[1],numSamples*sizeof(float));
						*/
					}
				}
				// volume "counter"
				{
					_volumeCounter = dsp::GetMaxVSTVol(_pSamplesL, _pSamplesR,numSamples) * 32768.0f;
					if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
					int temp((dsp::F2I(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3))); // * 2; // not 100% accurate, but looks as it sounds
					// prevent downward jerkiness
					if(temp > 97) temp = 97;
					if(temp > _volumeDisplay) _volumeDisplay = temp;
					--_volumeDisplay;
					///\todo: move autoStopMachines to player
					if(Global::pConfig->autoStopMachines)
					{
						if(_volumeCounter < 8.0f)
						{
							_volumeCounter = 0.0f;
							_volumeDisplay = 0;
							Standby(true);
						}
					}
				}
				
				_cpuCost += cpu::cycles() - cost;
				_worked = true;
			}



			/// old file format vomit, don't look at it.
			///////////////////////////////////////////////
			bool plugin::PreLoad(RiffFile * pFile, unsigned char &_program, int &_instance)
			{
				char junkdata[256];
				std::memset(&junkdata, 0, sizeof(junkdata));
				Machine::Init();

				pFile->Read(&_editName, 16);	//Remove when changing the fileformat.
				_editName[15]='\0';
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
				pFile->Read(&junkdata[0], 8*sizeof(int)); // SubTrack[]
				pFile->Read(&junkdata[0], sizeof(int)); // numSubtracks
				pFile->Read(&junkdata[0], sizeof(int)); // interpol

				pFile->Read(&junkdata[0], sizeof(int)); // outwet
				pFile->Read(&junkdata[0], sizeof(int)); // outdry

				pFile->Read(&junkdata[0], sizeof(int)); // distPosThreshold
				pFile->Read(&junkdata[0], sizeof(int)); // distPosClamp
				pFile->Read(&junkdata[0], sizeof(int)); // distNegThreshold
				pFile->Read(&junkdata[0], sizeof(int)); // distNegClamp

				pFile->Read(&junkdata[0], sizeof(char)); // sinespeed
				pFile->Read(&junkdata[0], sizeof(char)); // sineglide
				pFile->Read(&junkdata[0], sizeof(char)); // sinevolume
				pFile->Read(&junkdata[0], sizeof(char)); // sinelfospeed
				pFile->Read(&junkdata[0], sizeof(char)); // sinelfoamp

				pFile->Read(&junkdata[0], sizeof(int)); // delayTimeL
				pFile->Read(&junkdata[0], sizeof(int)); // delayTimeR
				pFile->Read(&junkdata[0], sizeof(int)); // delayFeedbackL
				pFile->Read(&junkdata[0], sizeof(int)); // delayFeedbackR

				pFile->Read(&junkdata[0], sizeof(int)); // filterCutoff
				pFile->Read(&junkdata[0], sizeof(int)); // filterResonance
				pFile->Read(&junkdata[0], sizeof(int)); // filterLfospeed
				pFile->Read(&junkdata[0], sizeof(int)); // filterLfoamp
				pFile->Read(&junkdata[0], sizeof(int)); // filterLfophase
				pFile->Read(&junkdata[0], sizeof(int)); // filterMode

				bool old;
				pFile->Read(&old, sizeof old); // old format
				pFile->Read(&_instance, sizeof _instance); // ovst.instance
				if(old)
				{
					char mch;
					pFile->Read(&mch, sizeof mch);
					_program = 0;
				}
				else
				{
					pFile->Read(&_program, sizeof _program);
				}
				return true;
			}
			bool plugin::LoadFromMac(vst::plugin *pMac)
			{
				Machine::Init();
				strcpy(_editName,pMac->_editName);
				memcpy(_inputMachines,pMac->_inputMachines,sizeof(_inputMachines));
				memcpy(_outputMachines,pMac->_outputMachines,sizeof(_outputMachines));
				memcpy(_inputConVol,pMac->_inputConVol,sizeof(_inputConVol));
				memcpy(_connection,pMac->_connection,sizeof(_connection));
				memcpy(_inputCon,pMac->_inputCon,sizeof(_inputCon));
				memcpy(_connectionPoint,pMac->_connectionPoint,sizeof(_connectionPoint));
				_numInputs= pMac->_numInputs;
				_numOutputs= pMac->_numOutputs;
				
				Machine::SetPan(pMac->_panning);
				return true;
			}
			// Load for Old Psycle fileformat
			bool plugin::LoadChunk(RiffFile * pFile)
			{
				bool b;
				try
				{
					b = ProgramIsChunk();
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(!b) return false;
				// read chunk size
				long chunk_size;
				pFile->Read(&chunk_size, sizeof chunk_size);
				// read chunk data
				char * chunk(new char[chunk_size]);
				pFile->Read(chunk, chunk_size);
				try
				{
					SetChunk(chunk,chunk_size);
				}
				catch(const std::exception &)
				{
					// [bohan] hmm, so, data just gets lost?
					zapArray(chunk);
					return false;
				}
				zapArray(chunk);
				return true;
			}
		}
	}
}
