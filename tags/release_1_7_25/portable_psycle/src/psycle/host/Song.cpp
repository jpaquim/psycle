#include "stdafx.h"
#if !defined _WINAMP_PLUGIN_
	#include "psycle.h"
	#include "NewMachine.h"
	#include "MainFrm.h"
	#include "ChildView.h"
	#include "ProgressDialog.h"
#endif
#include "Song.h"
#include "Machine.h" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "DataCompression.h"
#include <psycle/host/convert_internal_machines.h>
///\file
///\brief implementation file for psycle::host::Song.

#if !defined _WINAMP_PLUGIN_
	#include "Riff.h" // for Wave file loading.
	namespace psycle
	{
		namespace host
		{
			extern CPsycleApp theApp;

			/// the riff WAVE/fmt chunk.
			class WavHeader
			{
			public:
				char chunkID[4];
				long chunkSize;
				short wFormatTag;
				unsigned short wChannels;
				unsigned long  dwSamplesPerSec;
				unsigned long  dwAvgBytesPerSec;
				unsigned short wBlockAlign;
				unsigned short wBitsPerSample;
			};
		}
	}
#endif

namespace psycle
{
	namespace host
	{
		/// ???.
		class VSTLoader
		{
		public:
			bool valid;
			char dllName[128];
			int numpars;
			float * pars;
		};

		bool Song::CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int index)
		{
			Machine* pMachine;
			Master* pMaster;
			Sampler* pSampler;
			Plugin* pPlugin;
			vst::plugin* pVstPlugin;
			switch (type)
			{
			case MACH_MASTER:
				if(_pMachine[MASTER_INDEX]) return false;
				pMachine = pMaster = new Master(index);
				index = MASTER_INDEX;
				break;
			case MACH_SAMPLER:
				pMachine = pSampler = new Sampler(index);
				break;
			case MACH_PLUGIN:
				{
					pMachine = pPlugin = new Plugin(index);
					#if !defined _WINAMP_PLUGIN_
						if(!CNewMachine::TestFilename(psPluginDll))
						{
							zapObject(pMachine);
							return false;
						}
					#endif
					try
					{
						pPlugin->Instance(psPluginDll);
					}
					catch(...)
					{
						zapObject(pMachine); 
						return false;
					}
					break;
				}
			case MACH_VST:
				{
					pMachine = pVstPlugin = new vst::instrument(index);
					#if !defined _WINAMP_PLUGIN_
						if(!CNewMachine::TestFilename(psPluginDll)) 
						{
							zapObject(pMachine);
							return false;
						}
					#endif
					try
					{
						pVstPlugin->Instance(psPluginDll); // <bohan> why not using Load?
					}
					catch(...)
					{
						zapObject(pMachine);
						return false;
					}
					break;
				}
			case MACH_VSTFX:
				{
					pMachine = pVstPlugin = new vst::fx(index);
					#if !defined _WINAMP_PLUGIN_
						if(!CNewMachine::TestFilename(psPluginDll)) 
						{
							zapObject(pMachine);
							return false;
						}
					#endif
					try
					{
						pVstPlugin->Instance(psPluginDll); // <bohan> why not using Load?
					}
					catch(...)
					{
						zapObject(pMachine);
						return false;
					}
					break;
				}
			case MACH_DUMMY:
				pMachine = new Dummy(index);
				break;
			default:
				return false; ///< hmm?
			}
			if(index < 0)
			{
				index =	GetFreeMachine();
				if(index < 0) return false;
			}
			if(_pMachine[index]) DestroyMachine(index);
			if(pMachine->_type == MACH_VSTFX || pMachine->_type == MACH_VST )
			{
				// Do not call VST Init() function after Instance.
				pMachine->Machine::Init();
			}
			else pMachine->Init();
			pMachine->_x = x;
			pMachine->_y = y;
			// Finally, activate the machine
			_pMachine[index] = pMachine;
			return true;
		}

		#if !defined _WINAMP_PLUGIN_
			int Song::FindBusFromIndex(int smac)
			{
				if(!_pMachine[smac])  return 255;
				return smac;
			}
		#endif

		Song::Song()
		{
			#if defined(_WINAMP_PLUGIN_)
				filesize=0;
			#else
				_machineLock = false;
				Invalided = false;
				Tweaker = false;
				PW_Phase = 0;
				PW_Stage = 0;
				PW_Length = 0;
				// setting the preview wave volume to 25%
				preview_vol = 0.25f;
				#if !defined _CYRIX_PROCESSOR_
					ULONG cpuHz;
					__asm rdtsc ///< read time stamp to EAX
					__asm mov cpuHz, eax
					Sleep(1000);
					__asm rdtsc
					__asm sub eax, cpuHz ///< Find the difference
					__asm mov cpuHz, eax
					Global::_cpuHz = cpuHz;
				#else
					Global::_cpuHz = 1;
				#endif
			#endif
			for(int i(0) ; i < MAX_PATTERNS; ++i) ppPatternData[i] = NULL;
			for(int i(0) ; i < MAX_MACHINES; ++i) _pMachine[i] = NULL;
			CreateNewPattern(0);
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument;
			Reset();
		}

		Song::~Song()
		{
			DestroyAllMachines();
			DestroyAllInstruments();
			DeleteAllPatterns();
		}

		void Song::DestroyAllMachines()
		{
			#if !defined _WINAMP_PLUGIN_
				_machineLock = true;
			#endif
			for(int c(0) ;  c < MAX_MACHINES; ++c)
			{
				if(_pMachine[c])
				{
					for(int j(c + 1) ; j < MAX_MACHINES; ++j)
					{
						if(_pMachine[c] == _pMachine[j])
						{
							///\todo wtf? duplicate machine? could happen if loader messes up?
							char buf[128];
							std::sprintf(buf,"%d and %d have duplicate pointers", c, j);
							::MessageBox(0, buf, "Duplicate Machine", 0);
							_pMachine[j] = 0;
						}
					}
					DestroyMachine(c);
				}
				_pMachine[c] = 0;
			}
			#if !defined _WINAMP_PLUGIN_
				_machineLock = false;
			#endif
		}

		void Song::DeleteLayer(int i,int c)
		{
			_pInstrument[i]->DeleteLayer(c);
		}

		void Song::DeleteInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) DeleteInstrument(i);
		}

		void Song::DestroyAllInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) zapObject(_pInstrument[i]);
		}

		void Song::DeleteInstrument(int i)
		{
			#if !defined _WINAMP_PLUGIN_
				Invalided=true;
			#endif
			_pInstrument[i]->Delete();
			#if !defined _WINAMP_PLUGIN_
				Invalided=false;
			#endif
		}

		void Song::Reset()
		{
			#if !defined _WINAMP_PLUGIN_
				cpuIdle=0;
				_sampCount=0;
			#endif
			// Cleaning pattern allocation info
			for(int i(0) ; i < MAX_INSTRUMENTS; ++i) for(int c(0) ; c < MAX_WAVES; ++c) _pInstrument[i]->waveLength[c]=0;
			for(int i(0) ; i < MAX_MACHINES ; ++i)
			{
					zapObject(_pMachine[i]);
			}
			#if !defined _WINAMP_PLUGIN_
				for(int i(0) ; i < MAX_PATTERNS; ++i)
				{
					// All pattern reset
					if(Global::pConfig) patternLines[i]=Global::pConfig->defaultPatLines;
					else patternLines[i]=64;
					std::sprintf(patternName[i], "Untitled"); 
				}
			#endif
			_trackArmedCount = 0;
			for(int i(0) ; i < MAX_TRACKS; ++i)
			{
				_trackMuted[i] = false;
				_trackArmed[i] = false;
			}
			#if defined _WINAMP_PLUGIN_
				for(int i(0) ; i < MAX_SONG_POSITIONS; ++i) playOrder[i]=0; // All pattern reset
			#else
				machineSoloed = -1;
				_trackSoloed = -1;
				playLength=1;
				for(int i(0) ; i < MAX_SONG_POSITIONS; ++i)
				{
					playOrder[i]=0; // All pattern reset
					playOrderSel[i]=false;
				}
				playOrderSel[0]=true;
			#endif
		}

		void Song::New()
		{
			#if !defined(_WINAMP_PLUGIN_)
				CSingleLock lock(&door,TRUE);
			#endif
			seqBus=0;
			// Song reset
			std::memset(&Name, 0, sizeof Name);
			std::memset(&Author, 0, sizeof Author);
			std::memset(&Comment, 0, sizeof Comment);
			std::sprintf(Name, "Untitled");
			std::sprintf(Author, "Unnamed");
			std::sprintf(Comment, "No Comments");
			currentOctave=4;
			// General properties
			SetBPM(125, 4, 44100);
//			LineCounter=0;
//			LineChanged=false;
			//::MessageBox(0, "Machines", 0, 0);
			// Clean up allocated machines.
			DestroyAllMachines();
			//::MessageBox(0, "Insts", 0, 0);
			// Cleaning instruments
			DeleteInstruments();
			//::MessageBox(0, "Pats", 0, 0);
			// Clear patterns
			DeleteAllPatterns();
			// Clear sequence
			Reset();
			waveSelected = 0;
			instSelected = 0;
			midiSelected = 0;
			auxcolSelected = 0;
			_saved=false;
			#if defined _WINAMP_PLUGIN_
				std::strcpy(fileName,"Untitled.psy");
				CreateMachine(MACH_MASTER, 320, 200, NULL,MASTER_INDEX);
			#else
				fileName ="Untitled.psy";
				if((CMainFrame *)theApp.m_pMainWnd)
				{
					CreateMachine
						(
							MACH_MASTER, 
							(viewSize.x - static_cast<CMainFrame*>(theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width) / 2, 
							(viewSize.y - static_cast<CMainFrame*>(theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height) / 2, 
							0,
							MASTER_INDEX
						);
				}
				else
				{
					CreateMachine(MACH_MASTER, 320, 200, 0, MASTER_INDEX);
				}
			#endif
		}

		int Song::GetFreeMachine()
		{
			int tmac = 0;
			for(;;)
			{
				if(!_pMachine[tmac]) return tmac;
				if(tmac++ >= MAX_MACHINES) return -1;
			}
		}


		#if !defined _WINAMP_PLUGIN_
			bool Song::InsertConnection(int src, int dst, float value)
			{
				int freebus=-1;
				int dfreebus=-1;
				bool error=false;
				Machine *srcMac = _pMachine[src];
				Machine *dstMac = _pMachine[dst];
				if(!srcMac || !dstMac) return false;
				if(dstMac->_mode == MACHMODE_GENERATOR) return false;
				// Get a free output slot on the source machine
				for(int c(MAX_CONNECTIONS - 1) ; c >= 0 ; --c)
				{
					if(!srcMac->_connection[c]) freebus = c;
					// Checking that there's not an slot to the dest. machine already
					else if(srcMac->_outputMachines[c] == dst) error = true;
				}
				if(freebus == -1 || error) return false;
				// Get a free input slot on the destination machine
				error=false;
				for(int c=MAX_CONNECTIONS-1; c>=0; c--)
				{
					if(!dstMac->_inputCon[c]) dfreebus = c;
					// Checking if the destination machine is connected with the source machine to avoid a loop.
					else if((dstMac->_outputMachines[c] == src) && (dstMac->_connection[c])) error = true;
				}
				if(dfreebus == -1 || error) return false;
				// Calibrating in/out properties
				srcMac->_outputMachines[freebus] = dst;
				srcMac->_connection[freebus] = true;
				srcMac->_numOutputs++;
				dstMac->_inputMachines[dfreebus] = src;
				dstMac->_inputCon[dfreebus] = true;
				dstMac->_numInputs++;
				dstMac->InitWireVolume(srcMac->_type,dfreebus,value);
				return true;
			}
			int Song::ChangeWireDestMac(int wiresource, int wiredest, int wireindex)
			{
				int w;
				float volume = 1.0f;

				if (_pMachine[wiresource])
				{
					Machine *dmac = _pMachine[_pMachine[wiresource]->_outputMachines[wireindex]];

					if (dmac)
					{
						w = dmac->FindInputWire(wiresource);
						dmac->GetWireVolume(w,volume);
						if (InsertConnection(wiresource, wiredest,volume)) //\todo this needs to be checked. It wouldn't allow a machine with MAXCONNECTIONS to move any wire.
						{
							// delete the old wire
							_pMachine[wiresource]->_connection[wireindex] = FALSE;
							_pMachine[wiresource]->_numOutputs--;

							dmac->_inputCon[w] = FALSE;
							dmac->_numInputs--;
						}
/*						else
						{
							MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
						}*/
					}
				}
				return 0;
			}
			int Song::ChangeWireSourceMac(int wiresource,int wiredest, int wireindex)
			{
				float volume = 1.0f;

				if (_pMachine[wiredest])
				{					
					Machine *smac = _pMachine[_pMachine[wiredest]->_inputMachines[wireindex]];

					if (smac)
					{
						_pMachine[wiredest]->GetWireVolume(wireindex,volume);
						if (InsertConnection(wiresource, wiredest,volume)) //\todo this needs to be checked. It wouldn't allow a machine with MAXCONNECTIONS to move any wire.
						{
							// delete the old wire
							smac->_connection[smac->FindOutputWire(wiredest)] = FALSE;
							smac->_numOutputs--;

							_pMachine[wiredest]->_inputCon[wireindex] = FALSE;
							_pMachine[wiredest]->_numInputs--;
						}
/*						else
						{
							MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
						}*/
					}
				}
				return 0;
			}

		#endif

		void Song::DestroyMachine(int mac)
		{
			#if !defined _WINAMP_PLUGIN_
				CSingleLock lock(&door, TRUE);
			#endif
			Machine *iMac = _pMachine[mac];
			Machine *iMac2;
			if(iMac)
			{
				// Deleting the connections to/from other machines
				for(int w=0; w<MAX_CONNECTIONS; w++)
				{
					// Checking In-Wires
					if(iMac->_inputCon[w])
					{
						if((iMac->_inputMachines[w] >= 0) && (iMac->_inputMachines[w] < MAX_MACHINES))
						{
							iMac2 = _pMachine[iMac->_inputMachines[w]];
							if(iMac2)
							{
								for(int x=0; x<MAX_CONNECTIONS; x++)
								{
									if( iMac2->_connection[x] && iMac2->_outputMachines[x] == mac)
									{
										iMac2->_connection[x] = false;
										iMac2->_numOutputs--;
										break;
									}
								}
							}
						}
					}
					// Checking Out-Wires
					if(iMac->_connection[w])
					{
						if((iMac->_outputMachines[w] >= 0) && (iMac->_outputMachines[w] < MAX_MACHINES))
						{
							iMac2 = _pMachine[iMac->_outputMachines[w]];
							if(iMac2)
							{
								for(int x=0; x<MAX_CONNECTIONS; x++)
								{
									if(iMac2->_inputCon[x] && iMac2->_inputMachines[x] == mac)
									{
										iMac2->_inputCon[x] = false;
										iMac2->_numInputs--;
										break;
									}
								}
							}
						}
					}
				}
			}
			#if !defined _WINAMP_PLUGIN_
				if(mac == machineSoloed) machineSoloed = -1;
			#endif
			// If it's a (Vst)Plugin, the destructor calls to release the underlying library
			zapObject(_pMachine[mac]);
		}

		void Song::DeleteAllPatterns()
		{
			SONGTRACKS = 16;
			for(int i=0; i<MAX_PATTERNS; i++) RemovePattern(i);
		}

		void Song::RemovePattern(int ps)
		{
			zapObject(ppPatternData[ps]);
		}

		unsigned char * Song::CreateNewPattern(int ps)
		{
			RemovePattern(ps);
			ppPatternData[ps] = new unsigned char[MULTIPLY2];
			unsigned char blank[5]={255,255,255,0,0};
			unsigned char * pData = ppPatternData[ps];
			for(int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
			{
				memcpy(pData,blank,5*sizeof(unsigned char));
				pData+= EVENT_SIZE;
			}
			return ppPatternData[ps];
		}

		#if !defined _WINAMP_PLUGIN_
			bool Song::AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
			{
				unsigned char blank[5]={255,255,255,0,0};
				unsigned char *toffset;
				if(adaptsize)
				{
					float step;
					if( patternLines[pattern] > lines ) 
					{
						step= (float)patternLines[pattern]/lines;
						for(int t=0;t<SONGTRACKS;t++)
						{
							toffset=_ptrack(pattern,t);
							int l;
							for(l = 1 ; l < lines; ++l)
							{
								std::memcpy(toffset + l * MULTIPLY, toffset + f2i(l * step) * MULTIPLY,EVENT_SIZE);
							}
							while(l < patternLines[pattern])
							{
								// This wouldn't be necessary if we really allocate a new pattern.
								std::memcpy(toffset + (l * MULTIPLY), blank, EVENT_SIZE);
								++l;
							}
						}
						patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
					}
					else if(patternLines[pattern] < lines)
					{
						step= (float)lines/patternLines[pattern];
						int nl= patternLines[pattern];
						for(int t=0;t<SONGTRACKS;t++)
						{
							toffset=_ptrack(pattern,t);
							for(int l=nl-1;l>0;l--)
							{
								std::memcpy(toffset + f2i(l * step) * MULTIPLY, toffset + l * MULTIPLY,EVENT_SIZE);
								int tz(f2i(l * step) - 1);
								while (tz > (l - 1) * step)
								{
									std::memcpy(toffset + tz * MULTIPLY, blank, EVENT_SIZE);
									--tz;
								}
							}
						}
						patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
					}
				}
				else
				{
					int l(patternLines[pattern]);
					while(l < lines)
					{
						// This wouldn't be necessary if we really allocate a new pattern.
						for(int t(0) ; t < SONGTRACKS ; ++t)
						{
							toffset=_ptrackline(pattern,t,l);
							memcpy(toffset,blank,EVENT_SIZE);
							l++;
						}
					}
					patternLines[pattern] = lines;
				}
				std::sprintf(patternName[pattern], name);
				return true;
			}
		#endif

		void Song::SetBPM(int bpm, int tpb, int srate)
		{
			static int sr = 0;
			m_BeatsPerMin = bpm;
			_ticksPerBeat = tpb;
			m_SamplesPerTick = (srate*15*4)/(bpm*tpb);
			///\todo update the source code of the plugins... hey if our rate has changed, let everybody know!
			if(sr != srate)
			{
				sr = srate;
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(_pMachine[i]) _pMachine[i]->SetSampleRate(srate);
				}
			}
		}

		int Song::GetNumPatternsUsed()
		{
			int rval(0);
			for(int c(0) ; c < playLength ; ++c) if(rval < playOrder[c]) rval = playOrder[c];
			++rval;
			if(rval > MAX_PATTERNS - 1) rval = MAX_PATTERNS - 1;
			return rval;
		}

		#if !defined _WINAMP_PLUGIN_

			int Song::GetBlankPatternUnused(int rval)
			{
				for(int i(0) ; i < MAX_PATTERNS; ++i) if(!IsPatternUsed(i)) return i;
				const unsigned char blank[5] = {255,255,255,0,0};
				bool bTryAgain(true);
				while(bTryAgain && rval < MAX_PATTERNS - 1)
				{
					for(int c(0) ; c < playLength ; ++c)
					{
						if(rval == playOrder[c]) 
						{
							++rval;
							c = -1;
						}
					}
					// now test to see if data is really blank
					bTryAgain = false;
					if(rval < MAX_PATTERNS - 1)
					{
						unsigned char *offset_source(_ppattern(rval));
						for(int t(0) ; t < MULTIPLY2 ; t += EVENT_SIZE)
						{
							for(int i(0) ; i < EVENT_SIZE; ++i)
							{
								if(offset_source[i] != blank[i])
								{
									++rval;
									bTryAgain = true;
									t = MULTIPLY2;
									i = EVENT_SIZE;
								}
								offset_source += EVENT_SIZE;
							}
						}
					}
				}
				if(rval > MAX_PATTERNS - 1)
				{
					rval = 0;
					for(int c(0) ; c < playLength ; ++c)
					{
						if(rval == playOrder[c]) 
						{
							++rval;
							c = -1;
						}
					}
					if(rval > MAX_PATTERNS - 1) rval = MAX_PATTERNS - 1;
				}
				return rval;
			}

			int Song::GetFreeBus()
			{
				for(int c(0) ; c < MAX_BUSES ; ++c) if(!_pMachine[c]) return c;
				return -1; 
			}

			int Song::GetFreeFxBus()
			{
				for(int c(MAX_BUSES) ; c < MAX_BUSES * 2 ; ++c) if(!_pMachine[c]) return c;
				return -1; 
			}

			// IFF structure ripped by krokpitr
			// Current Code Extremely modified by [JAZ] ( RIFF based )
			// Advise: IFF files use Big Endian byte ordering. That's why I use
			// the following structure.
			//
			// typedef struct {
			//   unsigned char hihi;
			//   unsigned char hilo;
			//   unsigned char lohi;
			//   unsigned char lolo;
			// } ULONGINV;
			// 
			//
			/*
			** IFF Riff Header
			** ----------------

			char Id[4]			// "FORM"
			ULONGINV hlength	// of the data contained in the file (except Id and length)
			char type[4]		// "16SV" == 16bit . 8SVX == 8bit

			char name_Id[4]		// "NAME"
			ULONGINV hlength	// of the data contained in the header "NAME". It is 22 bytes
			char name[22]		// name of the sample.

			char vhdr_Id[4]		// "VHDR"
			ULONGINV hlength	// of the data contained in the header "VHDR". it is 20 bytes
			ULONGINV Samplength	// Lenght of the sample. It is in bytes, not in Samples.
			ULONGINV loopstart	// Start point for the loop. It is in bytes, not in Samples.
			ULONGINV loopLength	// Length of the loop (so loopEnd = loopstart+looplenght) In bytes.
			unsigned char unknown2[5]; //Always $20 $AB $01 $00 //
			unsigned char volumeHiByte;
			unsigned char volumeLoByte;
			unsigned char unknown3;

			char body_Id[4]		// "BODY"
			ULONGINV hlength	// of the data contained in the file. It is the sample length as well (in bytes)
			char *data			// the sample.

			*/

			int Song::IffAlloc(int instrument,int layer,const char * str)
			{
				if(instrument != PREV_WAV_INS)
				{
					Invalided = true;
					::Sleep(LOCK_LATENCY); ///< ???
				}
				RiffFile file;
				RiffChunkHeader hd;
				ULONG data;
				ULONGINV tmp;
				int bits = 0;
				// opens the file and reads the "FORM" header.
				if(!file.Open(const_cast<char*>(str)))
				{
					Invalided = false;
					return 0;
				}
				DeleteLayer(instrument,layer);
				file.Read(&data,4);
				if( data == file.FourCC("16SV")) bits = 16;
				else if(data == file.FourCC("8SVX")) bits = 8;
				file.Read(&hd,sizeof hd);
				if(hd._id == file.FourCC("NAME"))
				{
					file.Read(_pInstrument[instrument]->waveName[layer], 22); ///\todo should be hd._size instead of "22", but it is incorrectly read.
					if( strcmp(_pInstrument[instrument]->_sName,"empty") == 0)
					{
						strncpy(_pInstrument[instrument]->_sName,str,31);
						_pInstrument[instrument]->_sName[31]='\0';
					}
					file.Read(&hd,sizeof hd);
				}
				if ( hd._id == file.FourCC("VHDR"))
				{
					unsigned int Datalen, ls, le;
					file.Read(&tmp,sizeof tmp);
					Datalen = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
					file.Read(&tmp,sizeof tmp);
					ls = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
					file.Read(&tmp,sizeof tmp);
					le = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
					if(bits == 16)
					{
						Datalen >>= 1;
						ls >>= 1;
						le >>= 1;
					}
					_pInstrument[instrument]->waveLength[layer]=Datalen;
					if(ls != le)
					{
						_pInstrument[instrument]->waveLoopStart[layer] = ls;
						_pInstrument[instrument]->waveLoopEnd[layer] = ls + le;
						_pInstrument[instrument]->waveLoopType[layer] = true;
					}
					file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
					file.Read(&hd,sizeof hd);
				}
				if(hd._id == file.FourCC("BODY"))
				{
					short * csamples;
					const unsigned int Datalen(_pInstrument[instrument]->waveLength[layer]);
					_pInstrument[instrument]->waveStereo[layer] = false;
					_pInstrument[instrument]->waveDataL[layer] = new signed short[Datalen];
					csamples = _pInstrument[instrument]->waveDataL[layer];
					if(bits == 16)
					{
						for(unsigned int smp(0) ; smp < Datalen; ++smp)
						{
							file.Read(&tmp, 2);
							*csamples = tmp.hilo * 256 + tmp.hihi;
							++csamples;
						}
					}
					else
					{
						for(unsigned int smp(0) ; smp < Datalen; ++smp)
						{
							file.Read(&tmp, 1);
							*csamples = tmp.hihi * 256 + tmp.hihi;
							++csamples;
						}
					}
				}
				file.Close();
				Invalided = false;
				return 1;
			}

			int Song::WavAlloc(int iInstr, int iLayer, bool bStereo, long iSamplesPerChan, const char * sName)
			{
				///\todo what is ASSERT? some msicrosoft thingie?
				ASSERT(iSamplesPerChan<(1<<30)); ///< Since in some places, signed values are used, we cannot use the whole range.
				DeleteLayer(iInstr,iLayer);
				if(bStereo)
				{
					_pInstrument[iInstr]->waveDataL[iLayer] = new signed short[iSamplesPerChan];
					_pInstrument[iInstr]->waveDataR[iLayer] = new signed short[iSamplesPerChan];
					_pInstrument[iInstr]->waveStereo[iLayer] = true;
				}
				else
				{
					_pInstrument[iInstr]->waveDataL[iLayer] = new signed short[iSamplesPerChan];
					_pInstrument[iInstr]->waveStereo[iLayer] = false;
				}
				_pInstrument[iInstr]->waveLength[iLayer] = iSamplesPerChan;
				std::strncpy(_pInstrument[iInstr]->waveName[iLayer], sName, 31);
				_pInstrument[iInstr]->waveName[iLayer][31] = '\0';
				if(iLayer == 0)
				{
					std::strncpy(_pInstrument[iInstr]->_sName,sName,31);
					_pInstrument[iInstr]->_sName[31]='\0';
				}
				return true;
			}

			int Song::WavAlloc(int instrument,int layer,const char * Wavfile)
			{ 
				///\todo what is ASSERT? some msicrosoft thingie?
				ASSERT(Wavfile != 0);
				WaveFile file;
				ExtRiffChunkHeader hd;
				// opens the file and read the format Header.
				DDCRET retcode(file.OpenForRead((char*)Wavfile));
				if(retcode != DDC_SUCCESS) 
				{
					Invalided = false;
					return 0; 
				}
				Invalided = true;
				::Sleep(LOCK_LATENCY); ///< ???
				// sample type	
				int st_type(file.NumChannels());
				int bits(file.BitsPerSample());
				long Datalen(file.NumSamples());
				// Initializes the layer.
				WavAlloc(instrument, layer, st_type == 2, Datalen, Wavfile);
				// Reading of Wave data.
				// We don't use the WaveFile "ReadSamples" functions, because there are two main differences:
				// We need to convert 8bits to 16bits, and stereo channels are in different arrays.
				short * sampL(_pInstrument[instrument]->waveDataL[layer]);

				///\todo use template code for all this semi-repetitive code.

				long io; ///< \todo why is this declared here?
				// mono
				if(st_type == 1)
				{
					UINT8 smp8;
					switch(bits)
					{
						case 8:
							for(io = 0 ; io < Datalen ; ++io)
							{
								file.ReadData(&smp8, 1);
								*sampL = (smp8 << 8) - 32768;
								++sampL;
							}
							break;
						case 16:
								file.ReadData(sampL, Datalen);
							break;
						case 24:
							for(io = 0 ; io < Datalen ; ++io)
							{
								file.ReadData(&smp8, 1);
								file.ReadData(sampL, 1);
								++sampL;
							}
							break;
						default:
							break;
					}
				}
				// stereo
				else
				{
					short *sampR(_pInstrument[instrument]->waveDataR[layer]);
					UINT8 smp8;
					switch(bits)
					{
						case 8:
							for(io = 0 ; io < Datalen ; ++io)
							{
								file.ReadData(&smp8, 1);
								*sampL = (smp8 << 8) - 32768;
								++sampL;
								file.ReadData(&smp8, 1);
								*sampR = (smp8 << 8) - 32768;
								++sampR;
							}
							break;
						case 16:
							for(io = 0 ; io < Datalen ; ++io)
							{
								file.ReadData(sampL, 1);
								file.ReadData(sampR, 1);
								++sampL;
								++sampR;
							}
							break;
						case 24:
							for(io = 0 ; io < Datalen ; ++io)
							{
								file.ReadData(&smp8, 1);
								file.ReadData(sampL, 1);
								++sampL;
								file.ReadData(&smp8, 1);
								file.ReadData(sampR, 1);
								++sampR;
							}
							break;
						default:
							break; ///< \todo should throw an exception
					}
				}
				retcode = file.Read(static_cast<void*>(&hd), 8);
				while(retcode == DDC_SUCCESS)
				{
					if(hd.ckID == FourCC("smpl"))
					{
						char pl(0);
						file.Skip(28);
						file.Read(static_cast<void*>(&pl), 1);
						if(pl == 1)
						{
							file.Skip(15);
							unsigned int ls(0);
							unsigned int le(0);
							file.Read(static_cast<void*>(&ls), 4);
							file.Read(static_cast<void*>(&le), 4);
							_pInstrument[instrument]->waveLoopStart[layer] = ls;
							_pInstrument[instrument]->waveLoopEnd[layer] = le;
							// only for my bad sample collection
							//if(!((ls <= 0) && (le >= Datalen - 1)))
							{
								_pInstrument[instrument]->waveLoopType[layer] = true;
							}
						}
						file.Skip(9);
					}
					else if(hd.ckSize > 0)
						file.Skip(hd.ckSize);
					else
						file.Skip(1);
					retcode = file.Read(static_cast<void*>(&hd), 8);
				}
				file.Close();
				Invalided = false;
				return 1;
			}

		#endif // ndef _WINAMP_PLUGIN_

		bool Song::Load(RiffFile* pFile, bool fullopen)
		{
			char Header[9];
			pFile->Read(&Header, 8);
			Header[8]=0;

			if (strcmp(Header,"PSY3SONG")==0)
			{
				#if !defined _WINAMP_PLUGIN_
					CProgressDialog Progress;
					Progress.Create();
					Progress.SetWindowText("Loading...");
					Progress.ShowWindow(SW_SHOW);
				#endif
				UINT version = 0;
				UINT size = 0;
				UINT index = 0;
				int temp;
				int solo(0);
				int chunkcount;
				Header[4]=0;
				long filesize = pFile->FileSize();
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				pFile->Read(&chunkcount,sizeof(chunkcount));
				if(version > CURRENT_FILE_VERSION)
				{
					// there is an error, this file is newer than this build of psycle
					::MessageBox(0,"This file is from a newer version of Psycle! This process will try to load it anyway.", 0, 0);
				}
				pFile->Skip(size - sizeof chunkcount); // This ensures that any extra data is skipped.
				/*
				else
				{
					// This is left here if someday, extra data is added to the file version chunk.
				}
				*/
				DestroyAllMachines();
				_machineLock = true;
				DeleteInstruments();
				DeleteAllPatterns();
				Reset(); //added by sampler mainly to reset current pattern showed.
				while(pFile->Read(&Header, 4))
				{
					#if !defined _WINAMP_PLUGIN_
						Progress.m_Progress.SetPos(f2i((pFile->GetPos()*16384.0f)/filesize));
						::Sleep(1); ///< Allow screen refresh.
					#endif
					// we should use the size to update the index, but for now we will skip it
					if(std::strcmp(Header,"INFO") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						if(version > CURRENT_FILE_VERSION_INFO)
						{
							// there is an error, this file is newer than this build of psycle
							//::MessageBox(0, "Info Seqment of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->ReadString(Name, sizeof Name);
							pFile->ReadString(Author, sizeof Author);
							pFile->ReadString(Comment,sizeof Comment);
						}
					}
					else if(std::strcmp(Header,"SNGI")==0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						if(version > CURRENT_FILE_VERSION_SNGI)
						{
							// there is an error, this file is newer than this build of psycle
							//::MessageBox(0, "Song Segment of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// why all these temps?  to make sure if someone changes the defs of
							// any of these members, the rest of the file reads ok.  assume 
							// everything is an int, when we write we do the same thing.

							// # of tracks for whole song
							pFile->Read(&temp, sizeof temp);
							SONGTRACKS = temp;
							// bpm
							pFile->Read(&temp, sizeof temp);
							m_BeatsPerMin = temp;
							// tpb
							pFile->Read(&temp, sizeof temp);
							_ticksPerBeat = temp;
							// current octave
							pFile->Read(&temp, sizeof temp);
							currentOctave = temp;
							// machineSoloed
							// we need to buffer this because destroy machine will clear it
							pFile->Read(&temp, sizeof temp);
							solo = temp;
							// trackSoloed
							pFile->Read(&temp, sizeof temp);
							_trackSoloed = temp;
							pFile->Read(&temp, sizeof temp);  
							seqBus = temp;
							pFile->Read(&temp, sizeof temp);  
							midiSelected = temp;
							pFile->Read(&temp, sizeof temp);  
							auxcolSelected = temp;
							pFile->Read(&temp, sizeof temp);  
							instSelected = temp;
							// sequence width, for multipattern
							pFile->Read(&temp,sizeof(temp));
							_trackArmedCount = 0;
							for(int i(0) ; i < SONGTRACKS; ++i)
							{
								pFile->Read(&_trackMuted[i],sizeof(_trackMuted[i]));
								// remember to count them
								pFile->Read(&_trackArmed[i],sizeof(_trackArmed[i]));
								if(_trackArmed[i]) ++_trackArmedCount;
							}
							Global::pPlayer->bpm = m_BeatsPerMin;
							Global::pPlayer->tpb = _ticksPerBeat;
							// calculate samples per tick
							///\todo there's a unified call for samples per seconds
							#if defined _WINAMP_PLUGIN_
								m_SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
							#else
								m_SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
							#endif
						}
					}
					else if(std::strcmp(Header,"SEQD")==0)
					{
						--chunkcount;
						pFile->Read(&version,sizeof version);
						pFile->Read(&size,sizeof size);
						if(version > CURRENT_FILE_VERSION_SEQD)
						{
							// there is an error, this file is newer than this build of psycle
							//::MessageBox(0, "Sequence section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// index, for multipattern - for now always 0
							pFile->Read(&index, sizeof index);
							if (index < MAX_SEQUENCES)
							{
								char pTemp[256];
								// play length for this sequence
								pFile->Read(&temp, sizeof temp);
								playLength = temp;
								// name, for multipattern, for now unused
								pFile->ReadString(pTemp, sizeof pTemp);
								for (int i(0) ; i < playLength; ++i)
								{
									pFile->Read(&temp, sizeof temp);
									playOrder[i] = temp;
								}
							}
							else
							{
								//::MessageBox(0, "Sequence section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"PATD") == 0)
					{
						--chunkcount;
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						if(version > CURRENT_FILE_VERSION_PATD)
						{
							// there is an error, this file is newer than this build of psycle
							//::MessageBox(0, "Pattern section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// index
							pFile->Read(&index, sizeof index);
							if(index < MAX_PATTERNS)
							{
								// num lines
								pFile->Read(&temp, sizeof temp );
								// clear it out if it already exists
								RemovePattern(index);
								patternLines[index] = temp;
								// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
								pFile->Read(&temp, sizeof temp );
								pFile->ReadString(patternName[index], sizeof *patternName);
								pFile->Read(&size, sizeof size);
								byte* pSource = new byte[size];
								pFile->Read(pSource, size);
								byte* pDest;
								BEERZ77Decomp2(pSource, &pDest);
								zapObject(pSource,pDest);
								for(int y(0) ; y < patternLines[index] ; ++y)
								{
									unsigned char* pData(_ppattern(index) + (y * MULTIPLY));
									std::memcpy(pData, pSource, SONGTRACKS * EVENT_SIZE);
									pSource += SONGTRACKS * EVENT_SIZE;
								}
								zapObject(pDest);
							}
							else
							{
								//::MessageBox(0, "Pattern section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"MACD") == 0)
					{
						int curpos(0);
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						--chunkcount;
						if(!fullopen)
						{
							curpos = pFile->GetPos();
						}
						if(version > CURRENT_FILE_VERSION_MACD)
						{
							// there is an error, this file is newer than this build of psycle
							//::MessageBox(0, "Machine section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(&index, sizeof index);
							if(index < MAX_MACHINES)
							{
								// we had better load it
								DestroyMachine(index);
								_pMachine[index] = Machine::LoadFileChunk(pFile, index, version, fullopen);
								// skips specific chunk.
								if(!fullopen) pFile->Seek(curpos + size);
							}
							else
							{
								//::MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"INSD") == 0)
					{
						pFile->Read(&version, sizeof version);
						pFile->Read(&size, sizeof size);
						--chunkcount;
						if(version > CURRENT_FILE_VERSION_INSD)
						{
							// there is an error, this file is newer than this build of psycle
							//::MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(&index, sizeof index);
							if(index < MAX_INSTRUMENTS)
							{
								_pInstrument[index]->LoadFileChunk(pFile, version, fullopen);
							}
							else
							{
								//::MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else 
					{
						// we are not at a valid header for some weird reason.  
						// probably there is some extra data.
						// shift back 3 bytes and try again
						pFile->Skip(-3);
					}
				}
				// now that we have loaded all the modules, time to prepare them.
				#if !defined _WINAMP_PLUGIN_
					Progress.m_Progress.SetPos(16384);
					::Sleep(1); ///< ???
				#endif
				// test all connections for invalid machines. disconnect invalid machines.
				for(int i(0) ; i < MAX_MACHINES ; ++i)
				{
					if(_pMachine[i])
					{
						_pMachine[i]->_numInputs = 0;
						_pMachine[i]->_numOutputs = 0;
						for (int c(0) ; c < MAX_CONNECTIONS ; ++c)
						{
							if(_pMachine[i]->_connection[c])
							{
								if(_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_connection[c] = FALSE;
									_pMachine[i]->_outputMachines[c] = 255;
								}
								else if(!_pMachine[_pMachine[i]->_outputMachines[c]])
								{
									_pMachine[i]->_connection[c] = FALSE;
									_pMachine[i]->_outputMachines[c] = 255;
								}
								else 
								{
									_pMachine[i]->_numOutputs++;
								}
							}
							else
							{
								_pMachine[i]->_outputMachines[c] = 255;
							}

							if (_pMachine[i]->_inputCon[c])
							{
								if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_inputCon[c] = FALSE;
									_pMachine[i]->_inputMachines[c] = 255;
								}
								else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
								{
									_pMachine[i]->_inputCon[c] = FALSE;
									_pMachine[i]->_inputMachines[c] = 255;
								}
								else
								{
									_pMachine[i]->_numInputs++;
								}
							}
							else
							{
								_pMachine[i]->_inputMachines[c] = 255;
							}
						}
					}
				}

				// translate any data that is required
				#if !defined _WINAMP_PLUGIN_
					static_cast<CMainFrame*>(theApp.m_pMainWnd)->UpdateComboGen();
					machineSoloed = solo;
				#endif
				// allow stuff to work again
				_machineLock = false;
				#if !defined _WINAMP_PLUGIN_
					Progress.OnCancel();
				#endif
				if((!pFile->Close()) || (chunkcount))
				{
					char error[MAX_PATH];
					std::sprintf(error,"Error reading from \"%s\"!!!", pFile->szName);
					::MessageBox(0, error, "File Error!!!", 0);
					return false;
				}
				return true;
			}
			else if(std::strcmp(Header, "PSY2SONG") == 0)
			{
				#if !defined _WINAMP_PLUGIN_
					CProgressDialog Progress;
					Progress.Create();
					Progress.SetWindowText("Loading old format...");
					Progress.ShowWindow(SW_SHOW);
				#endif
				int i;
				int num;
				bool _machineActive[128];
				unsigned char busEffect[64];
				unsigned char busMachine[64];
				#if !defined _WINAMP_PLUGIN_
					New();
				#endif
				pFile->Read(&Name, 32);
				pFile->Read(&Author, 32);
				pFile->Read(&Comment, 128);
				pFile->Read(&m_BeatsPerMin, sizeof m_BeatsPerMin);
				pFile->Read(&m_SamplesPerTick, sizeof m_SamplesPerTick);
				if( m_SamplesPerTick <= 0)
				{
					// Shouldn't happen but has happened.
					_ticksPerBeat= 4; m_SamplesPerTick = 4315;
				}
				else _ticksPerBeat = 44100 * 15 * 4 / (m_SamplesPerTick * m_BeatsPerMin);
				Global::pPlayer->bpm = m_BeatsPerMin;
				Global::pPlayer->tpb = _ticksPerBeat;
				// The old format assumes we output at 44100 samples/sec, so...
				#if defined _WINAMP_PLUGIN_
					m_SamplesPerTick = m_SamplesPerTick * Global::pConfig->_samplesPerSec / 44100;
				#else
					m_SamplesPerTick = m_SamplesPerTick * Global::pConfig->_pOutputDriver->_samplesPerSec / 44100;
				#endif
				pFile->Read(&currentOctave, sizeof currentOctave);
				pFile->Read(&busMachine[0], sizeof busMachine);
				pFile->Read(&playOrder, sizeof playOrder);
				pFile->Read(&playLength, sizeof playLength);
				pFile->Read(&SONGTRACKS, sizeof SONGTRACKS);
				// Patterns
				pFile->Read(&num, sizeof num);
				for(i =0 ; i < num; ++i)
				{
					pFile->Read(&patternLines[i], sizeof *patternLines);
					pFile->Read(&patternName[i][0], sizeof *patternName);
					if(patternLines[i] > 0)
					{
						unsigned char * pData(CreateNewPattern(i));
						for(int c(0) ; c < patternLines[i] ; ++c)
						{
							pFile->Read(reinterpret_cast<char*>(pData), OLD_MAX_TRACKS * sizeof(PatternEntry));
							pData += MAX_TRACKS * sizeof(PatternEntry);
						}
					}
					else
					{
						patternLines[i] = 64;
						RemovePattern(i);
					}
				}
				#if !defined _WINAMP_PLUGIN_
					Progress.m_Progress.SetPos(2048);
					::Sleep(1); ///< ???
				#endif
				// Instruments
				pFile->Read(&instSelected, sizeof instSelected);
				for(int i(0) ; i < OLD_MAX_INSTRUMENTS ; ++i)
				{
					pFile->Read(&_pInstrument[i]->_sName, sizeof(_pInstrument[0]->_sName));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_NNA, sizeof(_pInstrument[0]->_NNA));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_AT, sizeof(_pInstrument[0]->ENV_AT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_DT, sizeof(_pInstrument[0]->ENV_DT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_SL, sizeof(_pInstrument[0]->ENV_SL));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_RT, sizeof(_pInstrument[0]->ENV_RT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_AT, sizeof(_pInstrument[0]->ENV_F_AT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_DT, sizeof(_pInstrument[0]->ENV_F_DT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_SL, sizeof(_pInstrument[0]->ENV_F_SL));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_RT, sizeof(_pInstrument[0]->ENV_F_RT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_CO, sizeof(_pInstrument[0]->ENV_F_CO));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_RQ, sizeof(_pInstrument[0]->ENV_F_RQ));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_EA, sizeof(_pInstrument[0]->ENV_F_EA));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->ENV_F_TP, sizeof(_pInstrument[0]->ENV_F_TP));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_pan, sizeof(_pInstrument[0]->_pan));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_RPAN, sizeof(_pInstrument[0]->_RPAN));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_RCUT, sizeof(_pInstrument[0]->_RCUT));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_RRES, sizeof(_pInstrument[0]->_RRES));
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(4096);
				::Sleep(1);
		#endif
				// Waves
				//
				pFile->Read(&waveSelected, sizeof(waveSelected));

				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					for (int w=0; w<OLD_MAX_WAVES; w++)
					{
						pFile->Read(&_pInstrument[i]->waveLength[w], sizeof(_pInstrument[0]->waveLength[0]));
						if (_pInstrument[i]->waveLength[w] > 0)
						{
							short tmpFineTune;
							pFile->Read(&_pInstrument[i]->waveName[w], 32);
							pFile->Read(&_pInstrument[i]->waveVolume[w], sizeof(_pInstrument[0]->waveVolume[0]));
							pFile->Read(&tmpFineTune, sizeof(short));
							_pInstrument[i]->waveFinetune[w]=(int)tmpFineTune;
							pFile->Read(&_pInstrument[i]->waveLoopStart[w], sizeof(_pInstrument[0]->waveLoopStart[0]));
							pFile->Read(&_pInstrument[i]->waveLoopEnd[w], sizeof(_pInstrument[0]->waveLoopEnd[0]));
							pFile->Read(&_pInstrument[i]->waveLoopType[w], sizeof(_pInstrument[0]->waveLoopType[0]));
							pFile->Read(&_pInstrument[i]->waveStereo[w], sizeof(_pInstrument[0]->waveStereo[0]));
							_pInstrument[i]->waveDataL[w] = new signed short[_pInstrument[i]->waveLength[w]];
							pFile->Read(_pInstrument[i]->waveDataL[w], _pInstrument[i]->waveLength[w]*sizeof(short));
							if (_pInstrument[i]->waveStereo[w])
							{
								_pInstrument[i]->waveDataR[w] = new signed short[_pInstrument[i]->waveLength[w]];
								pFile->Read(_pInstrument[i]->waveDataR[w], _pInstrument[i]->waveLength[w]*sizeof(short));
							}
						}
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(4096+2048);
				::Sleep(1);
		#endif
				// VST DLLs
				//

				VSTLoader vstL[OLD_MAX_PLUGINS]; 
				for (i=0; i<OLD_MAX_PLUGINS; i++)
				{
					pFile->Read(&vstL[i].valid,sizeof(bool));
					if( vstL[i].valid )
					{
						pFile->Read(vstL[i].dllName,sizeof(vstL[i].dllName));
						_strlwr(vstL[i].dllName);
						pFile->Read(&(vstL[i].numpars), sizeof(int));
						vstL[i].pars = new float[vstL[i].numpars];

						for (int c=0; c<vstL[i].numpars; c++)
						{
							pFile->Read(&(vstL[i].pars[c]), sizeof(float));
						}
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(8192);
				::Sleep(1);
		#endif
				// Machines
				//
				_machineLock = true;

				pFile->Read(&_machineActive[0], sizeof(_machineActive));
				Machine* pMac[128];
				memset(pMac,0,sizeof(pMac));

				convert_internal_machines::Converter converter;

				for (i=0; i<128; i++)
				{
					Sampler* pSampler;
					Plugin* pPlugin;
					vst::plugin * pVstPlugin(0);
					int x,y,type;
					if (_machineActive[i])
					{
		#if !defined(_WINAMP_PLUGIN_)
						Progress.m_Progress.SetPos(8192+i*(4096/128));
						::Sleep(1);
		#endif

						pFile->Read(&x, sizeof(x));
						pFile->Read(&y, sizeof(y));

						pFile->Read(&type, sizeof(type));

						if(converter.plugin_names().exists(type))
							pMac[i] = &converter.redirect(i, type, *pFile);
						else switch (type)
						{
						case MACH_MASTER:
							pMac[i] = _pMachine[MASTER_INDEX];
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						case MACH_SAMPLER:
							pMac[i] = pSampler = new Sampler(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						case MACH_PLUGIN:
							{
							pMac[i] = pPlugin = new Plugin(i);
							// Should the "Init()" function go here? -> No. Needs to load the dll first.
							if (!pMac[i]->Load(pFile))
							{
								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								// dummy name goes here
								sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
								pMac[i]->_type = MACH_DUMMY;
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								zapObject(pOldMachine);
							}
							break;
							}
						case MACH_VST:
						case MACH_VSTFX:
							{
							
							if ( type == MACH_VST ) 
							{
								pMac[i] = pVstPlugin = new vst::instrument(i);
							}
							else if ( type == MACH_VSTFX ) 
							{
								pMac[i] = pVstPlugin = new vst::fx(i);
							}
							if ((pMac[i]->Load(pFile)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
							{
								char sPath2[_MAX_PATH];
								CString sPath;
			#if defined(_WINAMP_PLUGIN_)
								sPath = Global::pConfig->GetVstDir();
								if ( fullopen && FindFileinDir(vstL[pVstPlugin->_instance].dllName,sPath) )
								{
									strcpy(sPath2,sPath);

									if (pVstPlugin->Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
									{
										Machine* pOldMachine = pMac[i];
										pMac[i] = new Dummy(*((Dummy*)pOldMachine));
										pOldMachine->_pSamplesL = NULL;
										pOldMachine->_pSamplesR = NULL;
										// dummy name goes here
										sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
										zapObject(pOldMachine);
										pMac[i]->_type = MACH_DUMMY;
										((Dummy*)pMac[i])->wasVST = true;
									}
								}
								else
								{
									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(*((Dummy*)pOldMachine));
									pOldMachine->_pSamplesL = NULL;
									pOldMachine->_pSamplesR = NULL;
									// dummy name goes here
									sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
									zapObject(pOldMachine);
									pMac[i]->_type = MACH_DUMMY;
									((Dummy*)pMac[i])->wasVST = true;
								}
			#else // if !_WINAMP_PLUGIN_
								std::string temp;
								if(CNewMachine::lookupDllName(vstL[pVstPlugin->_instance].dllName,temp))
								{
									sPath=temp.c_str();
									strcpy(sPath2,sPath);
									if (!CNewMachine::TestFilename(sPath2))
									{
										char sError[128];
										sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath2);
										::MessageBox(NULL,sError, "Loading Error", MB_OK);

										Machine* pOldMachine = pMac[i];
										pMac[i] = new Dummy(*((Dummy*)pOldMachine));
										pOldMachine->_pSamplesL = NULL;
										pOldMachine->_pSamplesR = NULL;
										// dummy name goes here
										sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
										zapObject(pOldMachine);
										pMac[i]->_type = MACH_DUMMY;
										((Dummy*)pMac[i])->wasVST = true;
									}
									else
									{
										try
										{
											pVstPlugin->Instance(sPath2, false); // <bohan> why not using Load?
										}
										catch(...)
										{
											char sError[128];
											sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath2);
											::MessageBox(NULL,sError, "Loading Error", MB_OK);

											Machine* pOldMachine = pMac[i];
											pMac[i] = new Dummy(*((Dummy*)pOldMachine));
											pOldMachine->_pSamplesL = NULL;
											pOldMachine->_pSamplesR = NULL;
											// dummy name goes here
											sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
											zapObject(pOldMachine);
											pMac[i]->_type = MACH_DUMMY;
											((Dummy*)pMac[i])->wasVST = true;
										}
									}
								}
								else
								{
									char sError[128];
									sprintf(sError,"Missing VST plug-in \"%s\"",vstL[pVstPlugin->_instance].dllName);
									::MessageBox(NULL,sError, "Loading Error", MB_OK);

									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(*((Dummy*)pOldMachine));
									pOldMachine->_pSamplesL = NULL;
									pOldMachine->_pSamplesR = NULL;
									// dummy name goes here
									sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
									zapObject(pOldMachine);
									pMac[i]->_type = MACH_DUMMY;
									((Dummy*)pMac[i])->wasVST = true;
								}
			#endif // _WINAMP_PLUGIN_
							}
							else
							{
								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								// dummy name goes here
								sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
								zapObject(pOldMachine);
								pMac[i]->_type = MACH_DUMMY;
								((Dummy*)pMac[i])->wasVST = true;
							}
							break;
							}
						case MACH_SCOPE:
						case MACH_DUMMY:
							pMac[i] = new Dummy(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
							break;
						default:
							{
								char buf[MAX_PATH];
								sprintf(buf,"unkown machine type: %i",type);
								::MessageBox(0, buf, "Loading old song", MB_ICONERROR);
							}
							pMac[i] = new Dummy(i);
							pMac[i]->Init();
							pMac[i]->Load(pFile);
						}

		#if !defined(_WINAMP_PLUGIN_)
						switch (pMac[i]->_mode)
						{
						case MACHMODE_GENERATOR:
							if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
							if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
							break;
						case MACHMODE_FX:
							if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
							if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
							break;

						case MACHMODE_MASTER:
							if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
							if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;
							break;
						}
		#endif // _WINAMP_PLUGIN_

						pMac[i]->_x = x;
						pMac[i]->_y = y;
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				Progress.m_Progress.SetPos(8192+4096);
				::Sleep(1);
		#endif

				// Since the old file format stored volumes on each output
				// rather than on each input, we must convert
				//
				float volMatrix[128][MAX_CONNECTIONS];
				for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
				{
					if (!_machineActive[i])
					{
						zapObject(pMac[i]);
					}
					else if (!pMac[i])
					{
						_machineActive[i] = FALSE;
					}
					else 
					{
						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							volMatrix[i][c] = pMac[i]->_inputConVol[c];
						}
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(8192+4096+1024);
				::Sleep(1);
		#endif
				for (i=0; i<128; i++) // Next, we go to fix this for each
				{
					if (_machineActive[i])		// valid machine (important, since we have to navigate!)
					{
						for (int c=0; c<MAX_CONNECTIONS; c++) // all of its input connections.
						{
							if (pMac[i]->_inputCon[c])	// If there's a valid machine in this inputconnection,
							{
								Machine* pOrigMachine = pMac[pMac[i]->_inputMachines[c]]; // We get that machine
								int d = pOrigMachine->FindOutputWire(i);

								float val = volMatrix[pMac[i]->_inputMachines[c]][d];
								if( val >= 4.000001f ) 
								{
									val*=0.000030517578125f; // BugFix
								}
								else if ( val < 0.00004f) 
								{
									val*=32768.0f; // BugFix
								}

								pMac[i]->InitWireVolume(pOrigMachine->_type,c,val);
							}
						}
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(8192+4096+2048);
				::Sleep(1);
		#endif
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_loop, sizeof(_pInstrument[0]->_loop));
				}
				for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
				{
					pFile->Read(&_pInstrument[i]->_lines, sizeof(_pInstrument[0]->_lines));
				}

				if ( pFile->Read(&busEffect[0],sizeof(busEffect)) == false ) // Patch 1: BusEffects (twf)
				{
					int j=0;
					for ( i=0;i<128;i++ ) 
					{
						if (_machineActive[i] && pMac[i]->_mode != MACHMODE_GENERATOR )
						{
							busEffect[j]=i;	
							j++;
						}
					}
					for (j; j < 64; j++)
					{
						busEffect[j] = 255;
					}
				}
				// Patch 1.2: Fixes crash/inconsistence when deleting a machine which couldn't be loaded
				// (.dll not found, or Load failed), which is, then, replaced by a DUMMY machine.
				int j=0;
				for ( i=0;i<64;i++ ) 
				{
					if (busMachine[i] != 255 && _machineActive[busMachine[i]]) 
					{ // If there's a machine in the generators' bus that it is not a generator:
						if (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR ) 
						{
							pMac[busMachine[i]]->_mode = MACHMODE_FX;
							while (busEffect[j] != 255 && j<MAX_BUSES) 
							{
								j++;
							}
							busEffect[j]=busMachine[i];
							busMachine[i]=255;
						}
					}
				}
				for ( i=0;i<64;i++ ) 
				{
					if ((busMachine[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)) 
					{
						busMachine[i] = 255;
					}
					if ((busEffect[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busEffect[i]]->_mode != MACHMODE_FX)) 
					{
						busEffect[i] = 255;
					}
				}

				bool chunkpresent=false;
				pFile->Read(&chunkpresent,sizeof(chunkpresent)); // Patch 2: VST's Chunk.

				if ( fullopen ) for ( i=0;i<128;i++ ) 
				{
					if (_machineActive[i])
					{
						if ( pMac[i]->_type == MACH_DUMMY ) 
						{
							if (((Dummy*)pMac[i])->wasVST && chunkpresent )
							{
								// Since we don't know if the plugin saved it or not, 
								// we're stuck on letting the loading crash/behave incorrectly.
								// There should be a flag, like in the VST loading Section to be correct.
								::MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
							}
						}
						else if (( pMac[i]->_type == MACH_VST ) || 
								( pMac[i]->_type == MACH_VSTFX))
						{
							bool chunkread = false;
							try
							{
								vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
								if(chunkpresent) chunkread = plugin.LoadChunk(pFile);
								plugin.proxy().dispatcher(effSetProgram, 0, plugin._program);
							}
							catch(const std::exception &)
							{
								// o_O`
							}
							if(!chunkpresent || !chunkread)
							{
								vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
								const int vi = plugin._instance;
								const int numpars = vstL[vi].numpars;
								for (int c(0) ; c < numpars; ++c)
								{
									try
									{
										plugin.proxy().setParameter(c, vstL[vi].pars[c]);
									}
									catch(const std::exception &)
									{
										// o_O`
									}
								}
							}
						}
					}
				}
				for (i=0; i<OLD_MAX_PLUGINS; i++) // Clean "pars" array.
				{
					if( vstL[i].valid )
					{
						zapObject(vstL[i].pars);
					}
				}

				// move machines around to where they really should go
				// now we have to remap all the inputs and outputs again... ouch
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(8192+4096+2048+1024);
				::Sleep(1);
		#endif

				for (i = 0; i < 64; i++)
				{
					if ((busMachine[i] < MAX_MACHINES-1) && (busMachine[i] > 0))
					{
						if (_machineActive[busMachine[i]])
						{
							if (pMac[busMachine[i]]->_mode == MACHMODE_GENERATOR)
							{
								_pMachine[i] = pMac[busMachine[i]];
								_machineActive[busMachine[i]] = FALSE; // don't update this twice;

								for (int c=0; c<MAX_CONNECTIONS; c++)
								{
									if (_pMachine[i]->_inputCon[c])
									{
										for (int x=0; x<64; x++)
										{
											if (_pMachine[i]->_inputMachines[c] == busMachine[x])
											{
												_pMachine[i]->_inputMachines[c] = x;
												break;
											}
											else if (_pMachine[i]->_inputMachines[c] == busEffect[x])
											{
												_pMachine[i]->_inputMachines[c] = x+MAX_BUSES;
												break;
											}
										}
									}

									if (_pMachine[i]->_connection[c])
									{
										if (_pMachine[i]->_outputMachines[c] == 0)
										{
											_pMachine[i]->_outputMachines[c] = MASTER_INDEX;
										}
										else
										{
											for (int x=0; x<64; x++)
											{
												if (_pMachine[i]->_outputMachines[c] == busMachine[x])
												{
													_pMachine[i]->_outputMachines[c] = x;
													break;
												}
												else if (_pMachine[i]->_outputMachines[c] == busEffect[x])
												{
													_pMachine[i]->_outputMachines[c] = x+MAX_BUSES;
													break;
												}
											}
										}
									}
								}
							}
						}
					}
					if ((busEffect[i] < MAX_MACHINES-1) && (busEffect[i] > 0))
					{
						if (_machineActive[busEffect[i]])
						{
							if (pMac[busEffect[i]]->_mode == MACHMODE_FX)
							{
								_pMachine[i+MAX_BUSES] = pMac[busEffect[i]];
								_machineActive[busEffect[i]] = FALSE; // don't do this again

								for (int c=0; c<MAX_CONNECTIONS; c++)
								{
									if (_pMachine[i+MAX_BUSES]->_inputCon[c])
									{
										for (int x=0; x<64; x++)
										{
											if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busMachine[x])
											{
												_pMachine[i+MAX_BUSES]->_inputMachines[c] = x;
												break;
											}
											else if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busEffect[x])
											{
												_pMachine[i+MAX_BUSES]->_inputMachines[c] = x+MAX_BUSES;
												break;
											}
										}
									}
									if (_pMachine[i+MAX_BUSES]->_connection[c])
									{
										if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == 0)
										{
											_pMachine[i+MAX_BUSES]->_outputMachines[c] = MASTER_INDEX;
										}
										else
										{
											for (int x=0; x<64; x++)
											{
												if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busMachine[x])
												{
													_pMachine[i+MAX_BUSES]->_outputMachines[c] = x;
													break;
												}
												else if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busEffect[x])
												{
													_pMachine[i+MAX_BUSES]->_outputMachines[c] = x+MAX_BUSES;
													break;
												}
											}
										}
									}
								}
							}
						}
					}
				}

				// fix machine #s

				for (i = 0; i < MAX_MACHINES-1; i++)
				{
					if (_pMachine[i])
					{
						_pMachine[i]->_macIndex = i;
						for (j = i+1; j < MAX_MACHINES-1; j++)
						{
							if (_pMachine[i] == _pMachine[j])
							{
								assert(false);
								// we have duplicate machines...
								// this should NEVER happen
								// delete the second one :(
								_pMachine[j] = NULL;
								// and we should remap anything that had wires to it to the first one
							}
						}
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(8192+4096+2048+1024+512);
				::Sleep(1);
		#endif
				// test all connections

				for (int c=0; c<MAX_CONNECTIONS; c++)
				{
					if (_pMachine[MASTER_INDEX]->_inputCon[c])
					{
						for (int x=0; x<64; x++)
						{
							if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busMachine[x])
							{
								_pMachine[MASTER_INDEX]->_inputMachines[c] = x;
								break;
							}
							else if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busEffect[x])
							{
								_pMachine[MASTER_INDEX]->_inputMachines[c] = x+MAX_BUSES;
								break;
							}
						}
					}
				}
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.m_Progress.SetPos(16384);
				::Sleep(1);
		#endif
				// test all connections for invalid machines. disconnect invalid machines.
				for (i = 0; i < MAX_MACHINES; i++)
				{
					if (_pMachine[i])
					{
						_pMachine[i]->_numInputs = 0;
						_pMachine[i]->_numOutputs = 0;

						for (int c = 0; c < MAX_CONNECTIONS; c++)
						{
							if (_pMachine[i]->_connection[c])
							{
								if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_connection[c]=FALSE;
									_pMachine[i]->_outputMachines[c]=255;
								}
								else if (!_pMachine[_pMachine[i]->_outputMachines[c]])
								{
									_pMachine[i]->_connection[c]=FALSE;
									_pMachine[i]->_outputMachines[c]=255;
								}
								else 
								{
									_pMachine[i]->_numOutputs++;
								}
							}
							else
							{
								_pMachine[i]->_outputMachines[c]=255;
							}

							if (_pMachine[i]->_inputCon[c])
							{
								if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES-1)
								{
									_pMachine[i]->_inputCon[c]=FALSE;
									_pMachine[i]->_inputMachines[c]=255;
								}
								else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
								{
									_pMachine[i]->_inputCon[c]=FALSE;
									_pMachine[i]->_inputMachines[c]=255;
								}
								else
								{
									_pMachine[i]->_numInputs++;
								}
							}
							else
							{
								_pMachine[i]->_inputMachines[c]=255;
							}
						}
					}
				}
				if(fullopen) converter.retweak(*this);
				_machineLock = false;
				seqBus=0;
		#if !defined(_WINAMP_PLUGIN_)
				
				Progress.OnCancel();
		#endif
				if (!pFile->Close())
				{
					char error[MAX_PATH];
					sprintf(error,"Error reading from \"%s\"!!!",pFile->szName);
					MessageBox(NULL,error,"File Error!!!",0);
					return false;
				}

				return true;
			}

			// load did not work
			MessageBox(NULL,"Incorrect file format","Error",MB_OK);
			return false;
		}


		#if !defined(_WINAMP_PLUGIN_)
		bool Song::Save(RiffFile* pFile,bool autosave)
		{
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			// header, this has to be at the top of the file

			CProgressDialog Progress;
			if ( !autosave ) 
			{
				Progress.Create();
				Progress.SetWindowText("Saving...");
				Progress.ShowWindow(SW_SHOW);
			}

			int chunkcount = 3; // 3 chunks plus:
			for (int i = 0; i < MAX_PATTERNS; i++)
			{
				// check every pattern for validity
				if (IsPatternUsed(i))
				{
					chunkcount++;
				}
			}

			for (int i = 0; i < MAX_MACHINES; i++)
			{
				// check every pattern for validity
				if (_pMachine[i])
				{
					chunkcount++;
				}
			}

			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!_pInstrument[i]->Empty())
				{
					chunkcount++;
				}
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.SetRange(0,chunkcount);
				Progress.m_Progress.SetStep(1);
			}

			/*
			===================
			FILE HEADER
			===================
			id = "PSY3SONG"; // PSY2 was 1.66
			*/

			pFile->Write("PSY3SONG", 8);

			UINT version = CURRENT_FILE_VERSION;
			UINT size = sizeof(chunkcount);
			UINT index = 0;
			int temp;

			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));
			pFile->Write(&chunkcount,sizeof(chunkcount));

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			// the rest of the modules can be arranged in any order

			/*
			===================
			SONG INFO TEXT
			===================
			id = "INFO"; 
			*/

			pFile->Write("INFO",4);
			version = CURRENT_FILE_VERSION_INFO;
			size = strlen(Name)+strlen(Author)+strlen(Comment)+3;
			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));

			pFile->Write(&Name,strlen(Name)+1);
			pFile->Write(&Author,strlen(Author)+1);
			pFile->Write(&Comment,strlen(Comment)+1);

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			/*
			===================
			SONG INFO
			===================
			id = "SNGI"; 
			*/

			pFile->Write("SNGI",4);
			version = CURRENT_FILE_VERSION_SNGI;
			size = (6*sizeof(temp));
			pFile->Write(&version,sizeof(version));
			pFile->Write(&size,sizeof(size));

			temp = SONGTRACKS;
			pFile->Write(&temp,sizeof(temp));
			temp = m_BeatsPerMin;
			pFile->Write(&temp,sizeof(temp));
			temp = _ticksPerBeat;
			pFile->Write(&temp,sizeof(temp));
			temp = currentOctave;
			pFile->Write(&temp,sizeof(temp));
			temp = machineSoloed;
			pFile->Write(&temp,sizeof(temp));
			temp = _trackSoloed;
			pFile->Write(&temp,sizeof(temp));

			temp = seqBus;
			pFile->Write(&temp,sizeof(temp));

			temp = midiSelected;
			pFile->Write(&temp,sizeof(temp));
			temp = auxcolSelected;
			pFile->Write(&temp,sizeof(temp));
			temp = instSelected;
			pFile->Write(&temp,sizeof(temp));

			temp = 1; // sequence width
			pFile->Write(&temp,sizeof(temp));

			for (int i = 0; i < SONGTRACKS; i++)
			{
				pFile->Write(&_trackMuted[i],sizeof(_trackMuted[i]));
				pFile->Write(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			/*
			===================
			SEQUENCE DATA
			===================
			id = "SEQD"; 
			*/
			index = 0; // index
			for (index=0;index<MAX_SEQUENCES;index++)
			{
				char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequence.

				pFile->Write("SEQD",4);
				version = CURRENT_FILE_VERSION_SEQD;
				size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
				pFile->Write(&version,sizeof(version));
				pFile->Write(&size,sizeof(size));
				
				pFile->Write(&index,sizeof(index)); // Sequence Track number
				temp = playLength;
				pFile->Write(&temp,sizeof(temp)); // Sequence length

				pFile->Write(pSequenceName,strlen(pSequenceName)+1); // Sequence Name

				for (int i = 0; i < playLength; i++)
				{
					temp = playOrder[i];
					pFile->Write(&temp,sizeof(temp));	// Sequence data.
				}
			}
			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}

			/*
			===================
			PATTERN DATA
			===================
			id = "PATD"; 
			*/

			for (int i = 0; i < MAX_PATTERNS; i++)
			{
				// check every pattern for validity
				if (IsPatternUsed(i))
				{
					// ok save it
					byte* pSource=new byte[SONGTRACKS*patternLines[i]*EVENT_SIZE];
					byte* pCopy = pSource;

					for (int y = 0; y < patternLines[i]; y++)
					{
						unsigned char* pData = ppPatternData[i]+(y*MULTIPLY);
						memcpy(pCopy,pData,EVENT_SIZE*SONGTRACKS);
						pCopy+=EVENT_SIZE*SONGTRACKS;
					}

					size = BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[i]*EVENT_SIZE)+(3*sizeof(temp))+strlen(patternName[i])+1;
					zapObject(pSource);

					pFile->Write("PATD",4);
					version = CURRENT_FILE_VERSION_PATD;

					pFile->Write(&version,sizeof(version));
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));
					temp = patternLines[i];
					pFile->Write(&temp,sizeof(temp));
					temp = SONGTRACKS; // eventually this may be variable per pattern
					pFile->Write(&temp,sizeof(temp));

					pFile->Write(&patternName[i],strlen(patternName[i])+1);

					size -= (3*sizeof(temp))+strlen(patternName[i])+1;
					pFile->Write(&size,sizeof(size));
					pFile->Write(pCopy,size);
					zapObject(pCopy);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}

			// machine and instruments handle their save and load in their respective classes

			for (int i = 0; i < MAX_MACHINES; i++)
			{
				if (_pMachine[i])
				{
					pFile->Write("MACD",4);
					version = CURRENT_FILE_VERSION_MACD;
					pFile->Write(&version,sizeof(version));
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));

					_pMachine[i]->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(&size,sizeof(size));
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}

			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!_pInstrument[i]->Empty())
				{
					pFile->Write("INSD",4);
					version = CURRENT_FILE_VERSION_INSD;
					pFile->Write(&version,sizeof(version));
					long pos = pFile->GetPos();
					size = 0;
					pFile->Write(&size,sizeof(size));

					index = i; // index
					pFile->Write(&index,sizeof(index));

					_pInstrument[i]->SaveFileChunk(pFile);

					long pos2 = pFile->GetPos(); 
					size = pos2-pos-sizeof(size);
					pFile->Seek(pos);
					pFile->Write(&size,sizeof(size));
					pFile->Seek(pos2);

					if ( !autosave ) 
					{
						Progress.m_Progress.StepIt();
						::Sleep(1);
					}
				}
			}

			if ( !autosave ) 
			{
				Progress.m_Progress.SetPos(chunkcount);
				::Sleep(1);

				Progress.OnCancel();
			}

			if (!pFile->Close())
			{
				char error[MAX_PATH];
				sprintf(error,"Error writing to \"%s\"!!!",pFile->szName);
				MessageBox(NULL,error,"File Error!!!",0);
				return false;
			}

			return true;
		}

		void Song::PW_Play()
		{
			if (PW_Stage==0)
			{
				PW_Length=_pInstrument[PREV_WAV_INS]->waveLength[0];
				if (PW_Length>0 )
				{
					PW_Stage=1;
					PW_Phase=0;
				}
			}
		}

		void Song::PW_Work(float *pInSamplesL, float *pInSamplesR, int numSamples)
		{
			float *pSamplesL = pInSamplesL;
			float *pSamplesR = pInSamplesR;
			--pSamplesL;
			--pSamplesR;
			
			signed short *wl=_pInstrument[PREV_WAV_INS]->waveDataL[0];
			signed short *wr=_pInstrument[PREV_WAV_INS]->waveDataR[0];
			bool const stereo=_pInstrument[PREV_WAV_INS]->waveStereo[0];
			float ld=0;
			float rd=0;
				
			do
			{
				ld=(*(wl+PW_Phase))*preview_vol;
				
				if(stereo)
					rd=(*(wr+PW_Phase))*preview_vol;
				else
					rd=ld;
					
				*++pSamplesL+=ld;
				*++pSamplesR+=rd;
					
				if(++PW_Phase>=PW_Length)
				{
					PW_Stage=0;
					return;
				}
				
			}while(--numSamples);
		}

		bool Song::CloneMac(int src,int dst)
		{
			// src has to be occupied and dst must be empty
			if (_pMachine[src] && _pMachine[dst])
			{
				return false;
			}
			if (_pMachine[dst])
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (!_pMachine[src])
			{
				return false;
			}
			// check to see both are same type
			if (((dst < MAX_BUSES) && (src >= MAX_BUSES))
				|| ((dst >= MAX_BUSES) && (src < MAX_BUSES)))
			{
				return false;
			}

			if ((src >= MAX_MACHINES-1) || (dst >= MAX_MACHINES-1))
			{
				return false;
			}

			// save our file
			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();

			CString filepath = Global::pConfig->GetInitialSongDir().c_str();
			filepath += "\\psycle.tmp";
			DeleteFile(filepath);
			OldPsyFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.Write("MACD",4);
			UINT version = CURRENT_FILE_VERSION_MACD;
			file.Write(&version,sizeof(version));
			long pos = file.GetPos();
			UINT size = 0;
			file.Write(&size,sizeof(size));

			int index = dst; // index
			file.Write(&index,sizeof(index));

			_pMachine[src]->SaveFileChunk(&file);

			long pos2 = file.GetPos(); 
			size = pos2-pos-sizeof(size);
			file.Seek(pos);
			file.Write(&size,sizeof(size));
			file.Close();

			// now load it

			if (!file.Open(filepath.GetBuffer(1)))
			{
				DeleteFile(filepath);
				return false;
			}
			char Header[5];
			file.Read(&Header, 4);
			Header[4] = 0;
			if (strcmp(Header,"MACD")==0)
			{
				file.Read(&version,sizeof(version));
				file.Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(&index,sizeof(index));
					index = dst;
					if (index < MAX_MACHINES)
					{
						// we had better load it
						DestroyMachine(index);
						_pMachine[index] = Machine::LoadFileChunk(&file,index,version);
					}
					else
					{
						file.Close();
						DeleteFile(filepath);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
			file.Close();
			DeleteFile(filepath);

			// oh and randomize the dst's position

			int xs,ys,x,y;
			if (src >= MAX_BUSES)
			{
				xs = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
				ys = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
			}
			else 
			{
				xs = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
				ys = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
			}
			x=_pMachine[dst]->_x+32;
			y=_pMachine[dst]->_y+ys+8;

			bool bCovered = TRUE;
			while (bCovered)
			{
				bCovered = FALSE;
				for (int i=0; i < MAX_MACHINES; i++)
				{
					if (i != dst)
					{
						if (_pMachine[i])
						{
							if ((abs(_pMachine[i]->_x - x) < 32) &&
								(abs(_pMachine[i]->_y - y) < 32))
							{
								bCovered = TRUE;
								i = MAX_MACHINES;
								x = (rand())%(((CMainFrame *)theApp.m_pMainWnd)->m_wndView.CW-xs);
								y = (rand())%(((CMainFrame *)theApp.m_pMainWnd)->m_wndView.CH-ys);
							}
						}
					}
				}
			}
			_pMachine[dst]->_x = x;
			_pMachine[dst]->_y = y;

			// oh and delete all connections

			_pMachine[dst]->_numInputs = 0;
			_pMachine[dst]->_numOutputs = 0;

			for (int i = 0; i < MAX_CONNECTIONS; i++)
			{
				if (_pMachine[dst]->_connection[i])
				{
					_pMachine[dst]->_connection[i] = false;
					_pMachine[dst]->_outputMachines[i] = 255;
				}

				if (_pMachine[dst]->_inputCon[i])
				{
					_pMachine[dst]->_inputCon[i] = false;
					_pMachine[dst]->_inputMachines[i] = 255;
				}
			}

			int number = 1;
			char buf[sizeof(_pMachine[dst]->_editName)+4];
			strcpy (buf,_pMachine[dst]->_editName);
			char* ps = strrchr(buf,' ');
			if (ps)
			{
				number = atoi(ps);
				if (number < 1)
				{
					number =1;
				}
				else
				{
					ps[0] = 0;
					ps = strchr(_pMachine[dst]->_editName,' ');
					ps[0] = 0;
				}
			}

			for (int i = 0; i < MAX_MACHINES-1; i++)
			{
				if (i!=dst)
				{
					if (_pMachine[i])
					{
						if (strcmp(_pMachine[i]->_editName,buf)==0)
						{
							number++;
							sprintf(buf,"%s %d",_pMachine[dst]->_editName,number);
							i = -1;
						}
					}
				}
			}

			buf[sizeof(_pMachine[dst]->_editName)-1] = 0;
			strcpy(_pMachine[dst]->_editName,buf);

			return true;
		}


		bool Song::CloneIns(int src,int dst)
		{
			// src has to be occupied and dst must be empty
			if (!Global::_pSong->_pInstrument[src]->Empty() && !Global::_pSong->_pInstrument[dst]->Empty())
			{
				return false;
			}
			if (!Global::_pSong->_pInstrument[dst]->Empty())
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (Global::_pSong->_pInstrument[src]->Empty())
			{
				return false;
			}
			// ok now we get down to business

			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();

			// save our file

			CString filepath = Global::pConfig->GetInitialSongDir().c_str();
			filepath += "\\psycle.tmp";
			DeleteFile(filepath);
			OldPsyFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.Write("INSD",4);
			UINT version = CURRENT_FILE_VERSION_INSD;
			file.Write(&version,sizeof(version));
			long pos = file.GetPos();
			UINT size = 0;
			file.Write(&size,sizeof(size));

			int index = dst; // index
			file.Write(&index,sizeof(index));

			_pInstrument[src]->SaveFileChunk(&file);

			long pos2 = file.GetPos(); 
			size = pos2-pos-sizeof(size);
			file.Seek(pos);
			file.Write(&size,sizeof(size));

			file.Close();

			// now load it

			if (!file.Open(filepath.GetBuffer(1)))
			{
				DeleteFile(filepath);
				return false;
			}
			char Header[5];
			file.Read(&Header, 4);
			Header[4] = 0;

			if (strcmp(Header,"INSD")==0)
			{
				file.Read(&version,sizeof(version));
				file.Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(&index,sizeof(index));
					index = dst;
					if (index < MAX_INSTRUMENTS)
					{
						// we had better load it
						_pInstrument[index]->LoadFileChunk(&file,version);
					}
					else
					{
						file.Close();
						DeleteFile(filepath);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
			file.Close();
			DeleteFile(filepath);
			return true;
		}

		bool Song::IsPatternUsed(int i)
		{
			bool bUsed = FALSE;
			if (ppPatternData[i])
			{
				// we could also check to see if pattern is unused AND blank.
				for (int j = 0; j < playLength; j++)
				{
					if (playOrder[j] == i)
					{
						bUsed = TRUE;
						break;
					}
				}

				if (!bUsed)
				{
					// check to see if it is empty
					unsigned char blank[5]={255,255,255,0,0};
					unsigned char * pData = ppPatternData[i];
					for (int j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
					{
						for (int k = 0; k < 5; k++)
						{
							if (pData[j+k] != blank[k])
							{
								bUsed = TRUE;
								j = MULTIPLY2;
								break;
							}
						}
					}
				}
			}
			return bUsed;
		}
	#endif

	}
}
