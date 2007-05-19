///\file
///\brief implementation file for psycle::host::Machine
#include <project.private.hpp>
#include "global.hpp"
#include "player.hpp"
#include "psycle.hpp"
#include "MainFrm.hpp"
#include "VstEditorDlg.hpp"

///\todo:  When inserting a note in a pattern, set the correct samplePos and ppqPos corresponding to the place the note is being put.
//		(LiveSlice is a good example of what happens if it isn't correct)


#include "vsthost24.hpp"

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		namespace vst
		{
			float plugin::junk[STREAM_SIZE];
			using namespace seib::vst;

/*			Machine* host::CreateFromType(int _id, std::string _dllname)
			{
				//\todo;
				//return new;
			}
*/
			HWND host::MainWindow() { return ((CMainFrame *) theApp.m_pMainWnd)->m_hWnd; }
			void host::CalcTimeInfo(long lMask)
			{
				///\todo: cycleactive and recording to a "Start()" function.
				// create(?) a "tick" function called each work cycle in order to reset transportchanged,
				// automationwriting and automationreading.
				//
				/*
				kVstTransportChanged 		= 1,		// Indicates that Playing, Cycle or Recording has changed
				kVstTransportPlaying 		= 1 << 1,
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

					//||(!strcmp(ptr, canDoReportConnectionChanges )) // "reportConnectionChanges",
					//||	(!strcmp(ptr, canDoAcceptIOChanges ))	// "acceptIOChanges",
					(!strcmp(ptr, canDoSizeWindow ))		// "sizeWindow",

					//||	(!strcmp(ptr, canDoAsyncProcessing ))	// DEPRECATED
					//||	(!strcmp(ptr, canDoOffline ))			// "offline",
					//||	(!strcmp(ptr, "supportShell" ))		// DEPRECATED
					//||	(!strcmp(ptr, canDoEditFile ))			// "editFile",
					//||	(!strcmp(ptr, canDoStartStopProcess ))	// "startStopProcess"
					//||	(!strcmp(ptr, canDoSendVstMidiEventFlagIsRealtime ))
					)
					return true;
				return false;                           /* per default, no.                  */
			}

			long host::DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos)
			{
				//  pos in Sample frames, return bpm* 10000
				return 0;
			}
			long host::OnGetOutputLatency(CEffect &pEffect)
			{
				//\todo : return Global::pConfig->_pOutputDriver->LatencyInSamples();
				AudioDriver* pdriver = Global::pConfig->_pOutputDriver;

				return (pdriver->_numBlocks*pdriver->_blockSize)/4;
			}
			long host::OnGetInputLatency(CEffect &pEffect)
			{
				//\todo : return Global::pConfig->_pOutputDriver->LatencyInSamples();
				AudioDriver* pdriver = Global::pConfig->_pOutputDriver;

				return (pdriver->_numBlocks*pdriver->_blockSize)/4;
			}


			///\todo: Get information about the following five functions, and especially on how the host automates the plugins
			// (i.e. the inverse step)
			long host::OnGetAutomationState(CEffect &pEffect) { return kVstAutomationUnsupported; }
			// Accepting BeginEdit and EndEdit.
			bool host::OnBeginEdit(CEffect &pEffect,long index) { return true; }
			bool host::OnEndEdit(CEffect &pEffect,long index) { return true; }
			void host::OnSetParameterAutomated(CEffect &pEffect, long index, float value)
			{
				Global::player().Tweaker = true;
				if(index<0 || index >= pEffect.numParams()) {
					psycle::host::loggers::info("error audioMasterAutomate: index<0 || index >= pEffect.numParams()");
					return;
				}
				// Send the event to the IOhandler. It will know what to do. It's not the host's work.
				if(Global::configuration()._RecordTweaks)
				{
					if(Global::configuration()._RecordMouseTweaksSmooth)
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(((plugin*)&pEffect)->_macIndex, index, f2i(value * quantization));
					else
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(((plugin*)&pEffect)->_macIndex, index, f2i(value * quantization));
				}
			}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			plugin::plugin(LoadedAEffect &loadstruct)
				:CEffect(loadstruct)
				,queue_size(0)
				,requiresRepl(0)
				,requiresProcess(0)
				,editorwin(0)
			{
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
					_pOutSamplesL = new float[STREAM_SIZE];
					_pOutSamplesR = new float[STREAM_SIZE];
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
				char temp[64];
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
			bool plugin::OnSizeEditorWindow(long width, long height)
			{
				if (editorwin)
				{	
					editorwin->Resize(width,height);
					return true;
				}
				return false;
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
				if(parameter < numParams())
				{
					char par_display[64]={0};
					char par_label[64]={0};
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
					pFile->Read(&_program, sizeof _program);
					SetProgram(_program);

					UINT count;
					pFile->Read(&count, sizeof count);
					size -= sizeof _program + sizeof count + sizeof(float) * count;
					if(!size)
					{
						for(UINT i(0) ; i < count ; ++i)
						{
							float temp;
							pFile->Read(&temp, sizeof temp);
							SetParameter(i, temp);
						}
					}
					else
					{
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
					}
				}
				return true;
			};

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
					count=0;
					chunksize = GetChunk((void**)&pData);
					size+=chunksize;
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
			};

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


			bool plugin::AddMIDI(unsigned char data0, unsigned char data1, unsigned char data2)
			{
				VstMidiEvent * pevent(reserveVstMidiEvent());
				if(!pevent) return false;
				pevent->type = kVstMidiType;
				pevent->byteSize = 24;
				pevent->deltaFrames = 0;
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
			bool plugin::AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel)
			{
				if(trackNote[channel].key != 255)
					AddNoteOff(channel, trackNote[channel].key, true);

				if(AddMIDI(0x90 | midichannel /*Midi On*/, key, velocity)) {
					note thisnote;
					thisnote.key = key;
					thisnote.midichan = midichannel;
					trackNote[channel] = thisnote;
					return true;
				}
				return false;
			}

			bool plugin::AddNoteOff(unsigned char channel, unsigned char midichannel, bool addatStart)
			{
				if(trackNote[channel].key == 255)
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
				pevent->deltaFrames = 0;
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

					queue_size = 0;
					//Finally Send the events.
					ProcessEvents(reinterpret_cast<VstEvents*>(&mevents));
				}
			}

			void plugin::Tick(int channel, PatternEntry * pData)
			{
					const int note = pData->_note;
					if(pData->_note == cdefMIDICC) // Mcm (MIDI CC) Command
					{
						AddMIDI(pData->_inst, pData->_cmd, pData->_parameter);
					}
					else if(note < 120) // Note on
					{
						if(pData->_cmd == 0x10) // _OLD_ MIDI Command
						{
							if((pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)
							{
								AddMIDI(pData->_inst, note, pData->_parameter);
							}
							else AddMIDI(pData->_inst,pData->_parameter);
						}
						else if(pData->_cmd == 0x0C) 
						{
							if(pData->_inst == 0xFF) AddNoteOn(channel, note, pData->_parameter / 2);
							else AddNoteOn(channel,note,pData->_parameter/2,pData->_inst&0x0F);
						}
						else 
						{
							if(pData->_inst == 0xFF) AddNoteOn(channel, note, 127); // should be 100, but previous host used 127
							else AddNoteOn(channel, note, 127, pData->_inst & 0x0F);
						}
					}
					else if(note == 120) // Note Off.
					{
						if(pData->_inst == 0xFF) AddNoteOff(channel);
						else AddNoteOff(channel, pData->_inst & 0x0F);
					}
					else if(note == cdefTweakM || note == cdefTweakE) // Tweak Command
					{
						const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
						SetParameter(pData->_inst, value);
						Global::pPlayer->Tweaker = true;
					}
					else if(note == cdefTweakS)
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
							TWSCurrent[i] = 0;
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
			}

			void plugin::Stop()
			{
					for(int i(0) ; i < MAX_TRACKS ; ++i) AddNoteOff(i);
					// <alk>
					// has been commented out because it crashes Z3ta+
					// and doesnt seem to be needed.
					/*
					for(int i(0) ; i < 16 ; ++i) AddMIDI(0xb0 + i, 0x7b); // Reset all controllers
					*/
			}

			void plugin::Work(int numSamples)
			{
				if(_mode != MACHMODE_GENERATOR) Machine::Work(numSamples);
				else
				{
					if (!_mute) _stopped = false;
					else _stopped = true;
				}
				CPUCOST_INIT(cost);
				if((!_mute) && (!_stopped) && (!_bypass))
				{
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
					SendMidi();
					try
					{
						if(numInputs() == 1)
						{
							///\todo MIX input0 and input1!
						}
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					try
					{
						if(!WillProcessReplace())
						{
							dsp::Clear(_pOutSamplesL, numSamples);
							dsp::Clear(_pOutSamplesR, numSamples);
						}
					}
					catch(const std::exception &)
					{
						// o_O`
					}
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
							if(TriggerDelay[i]._cmd) if(TriggerDelayCounter[i] < nextevent) nextevent = TriggerDelayCounter[i];
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
						if(numOutputs() == 1) std::memcpy(outputs[1], outputs[0], numSamples * sizeof(float));
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					if (!WillProcessReplace())
					{
						// This is an inversion of the pointers
						// so that _pOutSamples doesn't need to
						// be copied to _pSamples.
						float* const tempSamplesL = inputs[0];
						float* const tempSamplesR = inputs[1];	
						_pSamplesL = inputs[0] = outputs[0];
						_pSamplesR = inputs[1] = outputs[1];
						_pOutSamplesL = outputs[0] = tempSamplesL;
						_pOutSamplesR = outputs[1] = tempSamplesR;
						/*
						memcpy(inputs[0],outputs[0],numSamples);
						memcpy(inputs[1],outputs[1],numSamples);
						*/
					}
				}
				// volume "counter"
				{
					_volumeCounter = dsp::GetMaxVSTVol(_pSamplesL, _pSamplesR,numSamples) * 32768.0f;
					if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
					int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3))); // * 2; // not 100% accurate, but looks as it sounds
					// prevent downward jerkiness
					if(temp > 97) temp = 97;
					if(temp > _volumeDisplay) _volumeDisplay = temp;
					--_volumeDisplay;
					if(Global::pConfig->autoStopMachines)
					{
						if(_volumeCounter < 8.0f)
						{
							_volumeCounter = 0.0f;
							_volumeDisplay = 0;
							_stopped = true;
						}
					}
				}
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
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
