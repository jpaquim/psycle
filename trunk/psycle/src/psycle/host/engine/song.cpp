///\file
///\brief implementation file for psycle::host::Song.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "song.hpp"
#include "machine.hpp"
#include "sampler.hpp"
#include "XMSampler.hpp"
#include "plugin.hpp"
#include "VSTHost.hpp"
#include "DataCompression.hpp"
#include "riff.hpp" // for Wave file loading.
#include <psycle/host/gui/psycle.hpp>
#include <psycle/host/gui/NewMachine.hpp> // Is this needed?
#include <psycle/host/gui/MainFrm.hpp> // Is this needed?
#include <psycle/host/gui/ChildView.hpp> // Is this needed?
#include <psycle/host/gui/ProgressDialog.hpp> // Is this needed?
#include <cstdint>
#include <cassert>
#include <sstream>

#if !defined PSYCLE__CONFIGURATION__SERIALIZATION
	#error PSYCLE__CONFIGURATION__SERIALIZATION isn't defined! Check the code where this error is triggered.
#elif PSYCLE__CONFIGURATION__SERIALIZATION
	#include "serialization.private.hpp"
#endif

#if !defined DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
	#error "sorry, only works on little endian machines"
#endif

namespace psycle
{
	namespace host
	{
		/// the riff WAVE/fmt chunk.
		///\todo this is already defined elsewhere
		///\todo is this used for file i/o ? if so we'd need byte-alignment
		//UNIVERSALIS__COMPILER__ALIGNED(1)
		class WavHeader
		{
		public:
			char chunkID[4];
			std::uint32_t chunkSize;
			std::uint16_t wFormatTag;
			std::uint16_t wChannels;
			std::uint32_t dwSamplesPerSec;
			std::uint32_t dwAvgBytesPerSec;
			std::uint16_t wBlockAlign;
			std::uint16_t wBitsPerSample;
		};
	}
}

namespace psycle
{
	namespace host
	{
		bool Song::CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int index)
		{
			Machine * machine(0);
			switch (type)
			{
				case MACH_MASTER:
					if(_pMachine[MASTER_INDEX]) return false;
					machine = new Master(index);
					index = MASTER_INDEX;
					break;
				case MACH_SAMPLER:
					machine = new Sampler(index);
					break;
				case MACH_XMSAMPLER:
					machine = new XMSampler(index);
					break;
				case MACH_DUPLICATOR:
					machine = new DuplicatorMac(index);
					break;
				case MACH_MIXER:
					machine = new Mixer(index);
					break;
				case MACH_LFO:
					machine = new LFO(index);
					break;
				case MACH_DUMMY:
					machine = new Dummy(index);
					break;
				case MACH_PLUGIN:
					{
						Plugin & plugin(*new Plugin(index));
						machine = &plugin;
						if(!CNewMachine::TestFilename(psPluginDll))
						{
							delete &plugin;
							return false;
						}
						try
						{
							plugin.Instance(psPluginDll);
						}
						catch(std::exception const & e)
						{
							loggers::exception(e.what());
							delete &plugin;
							return false;
						}
						catch(...)
						{
							delete &plugin;
							return false;
						}
					}
					break;
				{
					vst::plugin * plugin;
					case MACH_VST:
						machine = plugin = new vst::instrument(index);
					case MACH_VSTFX:
						machine = plugin = new vst::fx(index);
						if(!CNewMachine::TestFilename(psPluginDll))
						{
							delete plugin;
							return false;
						}
						try
						{
							plugin->Instance(psPluginDll);
						}
						catch(std::exception const & e)
						{
							loggers::exception(e.what());
							delete plugin;
							return false;
						}
						catch(...)
						{
							delete plugin;
							return false;
						}
						break;
				}
				default:
					loggers::warning("failed to create requested machine type");
					return false;
			}

			if(index < 0)
			{
				index =	GetFreeMachine();
				if(index < 0)
				{
					loggers::warning("no more machine slots");
					return false;
				}
			}
			if(_pMachine[index]) DestroyMachine(index);
			
			///\todo init problem
			{
				if(machine->_type == MACH_VSTFX || machine->_type == MACH_VST )
				{
					
					// Do not call VST Init() function after Instance.
					machine->Machine::Init();
				}
				else machine->Init();
			}

			machine->_x = x;
			machine->_y = y;

			// Finally, activate the machine
			_pMachine[index] = machine;
			return true;
		}

		int Song::FindBusFromIndex(int smac)
		{
			if(!_pMachine[smac])  return 255;
			return smac;
		}

		Song::Song()
		#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
			#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
		#else
			#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
				: read_write_mutex_(boost::read_write_scheduling_policy::alternating_single_read) // see: http://boost.org/doc/html/threads/concepts.html#threads.concepts.read-write-scheduling-policies.inter-class
			#else // original implementation
				// nothing
			#endif
		#endif
		{
			_machineLock = false;
			Invalided = false;
			PW_Phase = 0;
			PW_Stage = 0;
			PW_Length = 0;
			// setting the preview wave volume to 25%
			preview_vol = 0.25f;
			
			for(int i(0) ; i < MAX_PATTERNS; ++i) ppPatternData[i] = NULL;
			for(int i(0) ; i < MAX_MACHINES; ++i) _pMachine[i] = NULL;
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument;
			Reset();
		}

		Song::~Song()
		{
			DestroyAllMachines();
			DestroyAllInstruments();
			DeleteAllPatterns();
		}

		void Song::DestroyAllMachines(bool write_locked)
		{
			_machineLock = true;
			for(int c(0) ;  c < MAX_MACHINES; ++c)
			{
				if(_pMachine[c])
				{
					for(int j(c + 1) ; j < MAX_MACHINES; ++j)
					{
						if(_pMachine[c] == _pMachine[j])
						{
							///\todo wtf? duplicate machine? could happen if loader messes up?
							{
								std::ostringstream s;
								s << c << " and " << j << " have duplicate pointers";
								MessageBox(0, s.str().c_str(), "Duplicate Machine", 0);
							}
							_pMachine[j] = 0;
						}
					}
					DestroyMachine(c, write_locked);
				}
				_pMachine[c] = 0;
			}
			_machineLock = false;
		}

		void Song::DeleteLayer(int i)
		{
			_pInstrument[i]->DeleteLayer();
		}

		void Song::DeleteInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) DeleteInstrument(i);
		}

		void Song::DestroyAllInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i)
			{
				delete _pInstrument[i];
				_pInstrument[i] = 0;
			}
		}

		void Song::DeleteInstrument(int i)
		{
			Invalided=true;
			_pInstrument[i]->Delete();
			Invalided=false;
		}

		void Song::Reset()
		{
			cpu_idle(0);
			_sampCount=0;
			// Cleaning pattern allocation info
			for(int i(0) ; i < MAX_INSTRUMENTS; ++i) _pInstrument[i]->waveLength=0;
			for(int i(0) ; i < MAX_MACHINES ; ++i)
			{
					delete _pMachine[i];
					_pMachine[i] = 0;
			}
			for(int i(0) ; i < MAX_PATTERNS; ++i)
			{
				// All pattern reset
				if(Global::pConfig) patternLines[i]=Global::pConfig->defaultPatLines;
				else patternLines[i]=64;
				std::sprintf(patternName[i], "Untitled"); 
			}
			CreateNewPattern(0);
			_trackArmedCount = 0;
			for(int i(0) ; i < MAX_TRACKS; ++i)
			{
				_trackMuted[i] = false;
				_trackArmed[i] = false;
			}
			machineSoloed = -1;
			_trackSoloed = -1;
			playLength=1;
			for(int i(0) ; i < MAX_SONG_POSITIONS; ++i)
			{
				playOrder[i]=0; // All pattern reset
				playOrderSel[i]=false;
			}
			playOrderSel[0]=true;
		}

		void Song::New()
		{
			#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
				#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
			#else
				#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
					boost::read_write_mutex::scoped_write_lock lock(read_write_mutex());
				#else // original implementation
					CSingleLock lock(&door,TRUE);
				#endif
			#endif
			seqBus=0;
			// Song reset
			std::memset(Name, 0, sizeof Name);
			std::memset(Author, 0, sizeof Author);
			std::memset(Comment, 0, sizeof Comment);
			std::sprintf(Name, "Untitled");
			std::sprintf(Author, "Unnamed");
			std::sprintf(Comment, "No Comments");
			currentOctave=4;
			// General properties
			m_BeatsPerMin=125;
			m_LinesPerBeat=4;
//			LineCounter=0;
//			LineChanged=false;
			//MessageBox(0, "Machines", 0, 0);
			// Clean up allocated machines.
			DestroyAllMachines(true);
			//MessageBox(0, "Insts", 0, 0);
			// Cleaning instruments
			DeleteInstruments();
			//MessageBox(0, "Pats", 0, 0);
			// Clear patterns
			DeleteAllPatterns();
			// Clear sequence
			Reset();
			instSelected = 0;
			midiSelected = 0;
			auxcolSelected = 0;
			_saved=false;
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

		bool Song::InsertConnection(int src, int dst, float value)
		{
			Machine *srcMac = _pMachine[src];
			Machine *dstMac = _pMachine[dst];
			if(!srcMac || !dstMac) return false;
			return srcMac->ConnectTo(dstMac,0,0,value);
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
						_pMachine[wiresource]->_connection[wireindex] = false;
						_pMachine[wiresource]->_outputMachines[wireindex] = -1;
						_pMachine[wiresource]->_connectedOutputs--;

						dmac->_inputCon[w] = false;
						dmac->_inputMachines[w] = -1;
						dmac->_connectedInputs--;
					}
					/*
					else
					{
						MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
					}
					*/
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
						int wire = smac->FindOutputWire(wiredest);
						smac->_connection[wire] = FALSE;
						smac->_outputMachines[wire] = 255;
						smac->_connectedOutputs--;

						_pMachine[wiredest]->_inputCon[wireindex] = FALSE;
						_pMachine[wiredest]->_inputMachines[wireindex] = 255;
						_pMachine[wiredest]->_connectedInputs--;
					}
					/*
					else
					{
						MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
					}
					*/
				}
			}
			return 0;
		}

		void Song::DestroyMachine(int mac, bool write_locked)
		{
			#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
				#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
			#else
				#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
					boost::read_write_mutex::scoped_write_lock lock(read_write_mutex(), !write_locked); // only lock if not already locked
				#else // original implementation
					CSingleLock lock(&door, TRUE);
				#endif
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
										iMac2->_outputMachines[x]=-1;
										iMac2->_connectedOutputs--;
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
										iMac2->_inputMachines[x]=-1;
										iMac2->_connectedInputs--;
										break;
									}
								}
							}
						}
					}
				}
			}
			if(mac == machineSoloed) machineSoloed = -1;
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
			zapArray(ppPatternData[ps]);
		}

		unsigned char * Song::CreateNewPattern(int ps)
		{
			RemovePattern(ps);
			ppPatternData[ps] = new unsigned char[MULTIPLY2];
			PatternEntry blank;
			unsigned char * pData = ppPatternData[ps];
			for(int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
			{
				memcpy(pData,&blank,EVENT_SIZE);
				pData+= EVENT_SIZE;
			}
			return ppPatternData[ps];
		}

		bool Song::AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
		{
			PatternEntry blank;
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
							std::memcpy(toffset + (l * MULTIPLY), &blank, EVENT_SIZE);
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
								std::memcpy(toffset + tz * MULTIPLY, &blank, EVENT_SIZE);
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
						memcpy(toffset,&blank,EVENT_SIZE);
					}
					++l;
				}
				patternLines[pattern] = lines;
			}
			std::sprintf(patternName[pattern], name);
			return true;
		}

		int Song::GetNumPatternsUsed()
		{
			int rval(0);
			for(int c(0) ; c < playLength ; ++c) if(rval < playOrder[c]) rval = playOrder[c];
			++rval;
			if(rval > MAX_PATTERNS - 1) rval = MAX_PATTERNS - 1;
			return rval;
		}

		int Song::GetBlankPatternUnused(int rval)
		{
			for(int i(0) ; i < MAX_PATTERNS; ++i) if(!IsPatternUsed(i)) return i;
			PatternEntry blank;
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
						if(memcmp(offset_source+t,&blank,EVENT_SIZE) != 0 )
						{
							++rval;
							bTryAgain = true;
							t = MULTIPLY2;
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

		int Song::IffAlloc(int instrument,const char * str)
		{
			if(instrument != PREV_WAV_INS)
			{
				Invalided = true;
				::Sleep(LOCK_LATENCY); ///< ???
			}
			RiffFile file;
			RiffChunkHeader hd;
			std::uint32_t data;
			endian::big::uint32_t tmp;
			int bits = 0;
			// opens the file and reads the "FORM" header.
			if(!file.Open(const_cast<char*>(str)))
			{
				Invalided = false;
				return 0;
			}
			DeleteLayer(instrument);
			file.Read(data);
			if( data == file.FourCC("16SV")) bits = 16;
			else if(data == file.FourCC("8SVX")) bits = 8;
			file.Read(hd);
			if(hd._id == file.FourCC("NAME"))
			{
				file.Read(_pInstrument[instrument]->waveName, 22); ///\todo should be hd._size instead of "22", but it is incorrectly read.
				strncpy(_pInstrument[instrument]->_sName,str, 31);
				_pInstrument[instrument]->_sName[31]='\0';
				file.Read(hd);
			}
			if ( hd._id == file.FourCC("VHDR"))
			{
				std::uint32_t Datalen, ls, le;
				file.Read(tmp); Datalen = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				file.Read(tmp); ls = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				file.Read(tmp); le = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				if(bits == 16)
				{
					Datalen >>= 1;
					ls >>= 1;
					le >>= 1;
				}
				_pInstrument[instrument]->waveLength=Datalen;
				if(ls != le)
				{
					_pInstrument[instrument]->waveLoopStart = ls;
					_pInstrument[instrument]->waveLoopEnd = ls + le;
					_pInstrument[instrument]->waveLoopType = true;
				}
				file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
				file.Read(hd);
			}
			if(hd._id == file.FourCC("BODY"))
			{
				std::int16_t * csamples;
				std::uint32_t const Datalen(_pInstrument[instrument]->waveLength);
				_pInstrument[instrument]->waveStereo = false;
				_pInstrument[instrument]->waveDataL = new std::int16_t[Datalen];
				csamples = _pInstrument[instrument]->waveDataL;
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

		int Song::WavAlloc(int iInstr, bool bStereo, long iSamplesPerChan, const char * sName)
		{
			assert(iSamplesPerChan<(1<<30)); ///< Since in some places, signed values are used, we cannot use the whole range.
			DeleteLayer(iInstr);
			_pInstrument[iInstr]->waveDataL = new std::int16_t[iSamplesPerChan];
			if(bStereo)
			{	_pInstrument[iInstr]->waveDataR = new std::int16_t[iSamplesPerChan];
				_pInstrument[iInstr]->waveStereo = true;
			} else {
				_pInstrument[iInstr]->waveStereo = false;
			}
			_pInstrument[iInstr]->waveLength = iSamplesPerChan;
			std::strncpy(_pInstrument[iInstr]->waveName, sName, 31);
			_pInstrument[iInstr]->waveName[31] = '\0';
			std::strncpy(_pInstrument[iInstr]->_sName,sName,31);
			_pInstrument[iInstr]->_sName[31]='\0';
			return true;
		}

		int Song::WavAlloc(int instrument,const char * Wavfile)
		{ 
			assert(Wavfile != 0);
			WaveFile file;
			ExtRiffChunkHeader hd;
			// opens the file and read the format Header.
			DDCRET retcode(file.OpenForRead(const_cast<char*>(Wavfile)));
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
			long int Datalen(file.NumSamples());
			// Initializes the layer.
			WavAlloc(instrument, st_type == 2, Datalen, Wavfile);
			// Reading of Wave data.
			// We don't use the WaveFile "ReadSamples" functions, because there are two main differences:
			// We need to convert 8bits to 16bits, and stereo channels are in different arrays.
			std::int16_t * sampL(_pInstrument[instrument]->waveDataL);

			///\todo use template code for all this semi-repetitive code.

			std::int32_t io; /// \todo why is this declared here?
			// mono
			if(st_type == 1)
			{
				std::uint8_t smp8;
				switch(bits)
				{
					case 8:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.Read(smp8);
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
							file.Read(smp8); ///\todo [bohan] is the lsb just discarded?
							file.ReadData(sampL, 1);
							++sampL;
						}
						break;
					default:
						break; ///\todo should throw an exception
				}
			}
			// stereo
			else
			{
				std::int16_t *sampR(_pInstrument[instrument]->waveDataR);
				std::uint8_t smp8;
				switch(bits)
				{
					case 8:
						for(io = 0 ; io < Datalen ; ++io)
						{
							file.Read(smp8);
							*sampL = (smp8 << 8) - 32768;
							++sampL;
							file.Read(smp8);
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
							file.Read(smp8); ///\todo [bohan] is the lsb just discarded?
							file.ReadData(sampL, 1);
							++sampL;
							file.Read(smp8); ///\todo [bohan] is the lsb just discarded?
							file.ReadData(sampR, 1);
							++sampR;
						}
						break;
					default:
						break; ///\todo should throw an exception
				}
			}
			retcode = file.Read(static_cast<void*>(&hd), 8); ///\todo bloergh!
			while(retcode == DDC_SUCCESS)
			{
				if(hd.ckID == FourCC("smpl"))
				{
					file.Skip(28);
					char pl;
					file.Read(pl);
					if(pl == 1)
					{
						file.Skip(15);
						std::uint32_t ls; file.Read(ls);
						std::uint32_t le; file.Read(le);
						_pInstrument[instrument]->waveLoopStart = ls;
						_pInstrument[instrument]->waveLoopEnd = le;
						// only for my bad sample collection
						//if(!((ls <= 0) && (le >= Datalen - 1)))
						{
							_pInstrument[instrument]->waveLoopType = true;
						}
					}
					file.Skip(9);
				}
				else if(hd.ckSize > 0)
					file.Skip(hd.ckSize);
				else
					file.Skip(1);
				retcode = file.Read(static_cast<void*>(&hd), 8); ///\todo bloergh!
			}
			file.Close();
			Invalided = false;
			return 1;
		}

		bool Song::Load(RiffFile* pFile, bool fullopen)
		{
			char Header[9];
			pFile->Read(&Header, 8);
			Header[8]=0;

			if (strcmp(Header,"PSY3SONG")==0)
			{
				CProgressDialog Progress;
				Progress.Create();
				Progress.SetWindowText("Loading...");
				Progress.ShowWindow(SW_SHOW);
				std::uint32_t version = 0;
				std::uint32_t size = 0;
				std::uint32_t index = 0;
				std::uint32_t temp;
				std::uint32_t solo(0);
				std::uint32_t chunkcount=0;
				Header[4]=0;
				std::fpos_t filesize = pFile->FileSize();
				pFile->Read(version);
				pFile->Read(size);
				if(version > CURRENT_FILE_VERSION)
				{
					MessageBox(0,"This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning", MB_OK | MB_ICONERROR);
				}
				if (size == 4) // Since "version" is used for File version, we use size as version identifier
				{
					pFile->Read(chunkcount);
				}
				/*
				if (size == )
				{
				// This is left here if someday, extra data is added to the file version chunk.
				// Modify "pFile->Skip(size - 4);" as necessary. Ex:  pFile->Skip(size - bytesread);
				}
				*/
				if ( size-4 > 0) pFile->Skip(size - 4);// Size of the current Header DATA // This ensures that any extra data is skipped.

				DestroyAllMachines();
				_machineLock = true;
				DeleteInstruments();
				DeleteAllPatterns();
				Reset(); //added by sampler mainly to reset current pattern showed.
				while(pFile->Read(&Header, 4))
				{
					Progress.m_Progress.SetPos(f2i((pFile->GetPos()*16384.0f)/filesize));
					::Sleep(1); ///< Allow screen refresh.
					// we should use the size to update the index, but for now we will skip it
					if(std::strcmp(Header,"INFO") == 0)
					{
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_INFO)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Info Seqment of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->ReadString(Name, sizeof Name);
							pFile->ReadString(Author, sizeof Author);
							pFile->ReadString(Comment, sizeof Comment);
						}
					}
					else if(std::strcmp(Header,"SNGI")==0)
					{
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_SNGI)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Song Segment of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// why all these temps?  to make sure if someone changes the defs of
							// any of these members, the rest of the file reads ok.  assume 
							// everything is 32-bit, when we write we do the same thing.

							// # of tracks for whole song
							pFile->Read(temp);
							SONGTRACKS = temp;
							// bpm
							pFile->Read(temp);
							m_BeatsPerMin = temp;
							// tpb
							pFile->Read(temp);
							m_LinesPerBeat = temp;
							// current octave
							pFile->Read(temp);
							currentOctave = temp;
							// machineSoloed
							// we need to buffer this because destroy machine will clear it
							pFile->Read(temp);
							solo = temp;
							// trackSoloed
							pFile->Read(temp);
							_trackSoloed = temp;
							pFile->Read(temp);
							seqBus = temp;
							pFile->Read(temp);
							midiSelected = temp;
							pFile->Read(temp);
							auxcolSelected = temp;
							pFile->Read(temp);
							instSelected = temp;
							// sequence width, for multipattern
							pFile->Read(temp);
							_trackArmedCount = 0;
							for(int i(0) ; i < SONGTRACKS; ++i)
							{
								pFile->Read(_trackMuted[i]);
								// remember to count them
								pFile->Read(_trackArmed[i]);
								if(_trackArmed[i]) ++_trackArmedCount;
							}
							Global::pPlayer->SetBPM(m_BeatsPerMin,m_LinesPerBeat);
						}
					}
					else if(std::strcmp(Header,"SEQD")==0)
					{
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_SEQD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Sequence section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// index, for multipattern - for now always 0
							pFile->Read(index);
							if (index < MAX_SEQUENCES)
							{
								char pTemp[256];
								// play length for this sequence
								pFile->Read(temp);
								playLength = temp;
								// name, for multipattern, for now unused
								pFile->ReadString(pTemp, sizeof pTemp);
								for (int i(0) ; i < playLength; ++i)
								{
									pFile->Read(temp);
									playOrder[i] = temp;
								}
							}
							else
							{
								//MessageBox(0, "Sequence section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"PATD") == 0)
					{
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_PATD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Pattern section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							// index
							pFile->Read(index);
							if(index < MAX_PATTERNS)
							{
								// num lines
								pFile->Read(temp);
								// clear it out if it already exists
								RemovePattern(index);
								patternLines[index] = temp;
								// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
								pFile->Read(temp);
								pFile->ReadString(patternName[index], sizeof *patternName);
								pFile->Read(size);
								unsigned char * pSource = new unsigned char[size];
								pFile->Read(pSource, size);
								unsigned char * pDest;
								BEERZ77Decomp2(pSource, &pDest);
								zapArray(pSource,pDest);
								for(int y(0) ; y < patternLines[index] ; ++y)
								{
									unsigned char* pData(_ppattern(index) + (y * MULTIPLY));
									std::memcpy(pData, pSource, SONGTRACKS * EVENT_SIZE);
									pSource += SONGTRACKS * EVENT_SIZE;
								}
								zapArray(pDest);
							}
							else
							{
								//MessageBox(0, "Pattern section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"MACD") == 0)
					{
						int curpos(0);
						pFile->Read(version);
						pFile->Read(size);
						--chunkcount;
						if(!fullopen)
						{
							curpos = pFile->GetPos();
						}
						if(version > CURRENT_FILE_VERSION_MACD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Machine section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(index);
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
								//MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"INSD") == 0)
					{
						pFile->Read(version);
						pFile->Read(size);
						--chunkcount;
						if(version > CURRENT_FILE_VERSION_INSD)
						{
							// there is an error, this file is newer than this build of psycle
							//MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(index);
							if(index < MAX_INSTRUMENTS)
							{
								_pInstrument[index]->LoadFileChunk(pFile, version, fullopen);
							}
							else
							{
								//MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else 
					{
						loggers::warning("foreign chunk found. skipping it.");
						pFile->Read(version);
						pFile->Read(size);
						pFile->Skip(size);
					}
				}
				// now that we have loaded all the modules, time to prepare them.
				Progress.m_Progress.SetPos(16384);
				::Sleep(1); ///< ???
				// test all connections for invalid machines. disconnect invalid machines.
				for(int i(0) ; i < MAX_MACHINES ; ++i)
				{
					if(_pMachine[i])
					{
						_pMachine[i]->_connectedInputs = 0;
						_pMachine[i]->_connectedOutputs = 0;
						for (int c(0) ; c < MAX_CONNECTIONS ; ++c)
						{
							if(_pMachine[i]->_connection[c])
							{
								if(_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_connection[c] = false;
									_pMachine[i]->_outputMachines[c] = -1;
								}
								else if(!_pMachine[_pMachine[i]->_outputMachines[c]])
								{
									_pMachine[i]->_connection[c] = false;
									_pMachine[i]->_outputMachines[c] = -1;
								}
								else 
								{
									_pMachine[i]->_connectedOutputs++;
								}
							}
							else
							{
								_pMachine[i]->_outputMachines[c] = -1;
							}

							if (_pMachine[i]->_inputCon[c])
							{
								if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
								{
									_pMachine[i]->_inputCon[c] = false;
									_pMachine[i]->_inputMachines[c] = -1;
								}
								else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
								{
									_pMachine[i]->_inputCon[c] = false;
									_pMachine[i]->_inputMachines[c] = -1;
								}
								else
								{
									_pMachine[i]->_connectedInputs++;
								}
							}
							else
							{
								_pMachine[i]->_inputMachines[c] = -1;
							}
						}
					}
				}

				// translate any data that is required
				static_cast<CMainFrame*>(theApp.m_pMainWnd)->UpdateComboGen();
				machineSoloed = solo;
				// allow stuff to work again
				_machineLock = false;
				Progress.OnCancel();
				if((!pFile->Close()) || (chunkcount))
				{
					std::ostringstream s;
					s << "Error reading from file '" << pFile->file_name() << "'" << std::endl;
					if(chunkcount) s << "some chunks were missing in the file";
					else s << "could not close the file";
					MessageBox(0, s.str().c_str(), "File Error!!!", 0);
					return false;
				}
				return true;
			}
			else if(std::strcmp(Header, "PSY2SONG") == 0)
			{
				return LoadOldFileFormat(pFile, fullopen);
			}

			// load did not work
			MessageBox(NULL,"Incorrect file format","Error",MB_OK);
			return false;
		}

		bool Song::Save(RiffFile* pFile,bool autosave)
		{
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			try
			{
				CProgressDialog Progress;
				if ( !autosave ) 
				{
					Progress.Create();
					Progress.SetWindowText("Saving...");
					Progress.ShowWindow(SW_SHOW);
				}

				std::uint32_t version, size, temp, chunkcount;

				/*
				===================
				FILE HEADER
				===================
				id = "PSY3SONG"; // PSY2 was 1.66
				*/
				// header, this has to be at the top of the file
				{
					chunkcount = 3; // 3 chunks plus:
					for(unsigned int i(0) ; i < MAX_PATTERNS    ; ++i) if(IsPatternUsed(i))          ++chunkcount; // check every pattern for validity
					for(unsigned int i(0) ; i < MAX_MACHINES    ; ++i) if(_pMachine[i])              ++chunkcount;
					for(unsigned int i(0) ; i < MAX_INSTRUMENTS ; ++i) if(!_pInstrument[i]->Empty()) ++chunkcount;

					if ( !autosave ) 
					{
						Progress.m_Progress.SetRange(0,chunkcount);
						Progress.m_Progress.SetStep(1);
					}

					// chunk header
					{
						pFile->Write("PSY3SONG", 8);

						version = CURRENT_FILE_VERSION;
						pFile->Write(version);

						size = sizeof chunkcount;
						pFile->Write(size);
					}
					// chunk data
					{
						pFile->Write(chunkcount);
					}
				}

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
				{
					// chunk header
					{
						pFile->Write("INFO",4);

						version = CURRENT_FILE_VERSION_INFO;
						pFile->Write(version);

						size = strlen(Name)+strlen(Author)+strlen(Comment)+3; // [bohan] since those are variable length, we could change from fixed size arrays to std::string
						pFile->Write(size);
					}
					// chunk data
					{
						pFile->Write(&Name,strlen(Name)+1);
						pFile->Write(&Author,strlen(Author)+1);
						pFile->Write(&Comment,strlen(Comment)+1);
					}
				}

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
				{
					// chunk header
					{
						pFile->Write("SNGI",4);

						version = CURRENT_FILE_VERSION_SNGI;
						pFile->Write(version);

						size = (11*sizeof(temp))+(SONGTRACKS*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])));
						pFile->Write(size);
					}
					// chunk data
					{
						temp = SONGTRACKS;     pFile->Write(temp);
						temp = m_BeatsPerMin;  pFile->Write(temp);
						temp = m_LinesPerBeat; pFile->Write(temp);
						temp = currentOctave;  pFile->Write(temp);
						temp = machineSoloed;  pFile->Write(temp);
						temp = _trackSoloed;   pFile->Write(temp);

						temp = seqBus; pFile->Write(temp);

						temp = midiSelected; pFile->Write(temp);
						temp = auxcolSelected; pFile->Write(temp);
						temp = instSelected; pFile->Write(temp);

						temp = 1;  pFile->Write(temp); // sequence width

						for(unsigned int i = 0; i < SONGTRACKS; i++)
						{
							pFile->Write(_trackMuted[i]);
							pFile->Write(_trackArmed[i]); // remember to count them
						}
					}
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
				for(std::uint32_t index(0) ; index < MAX_SEQUENCES ; ++index)
				{
					char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequence.

					// chunk header
					{
						pFile->Write("SEQD",4);

						version = CURRENT_FILE_VERSION_SEQD;
						pFile->Write(version);

						size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
						pFile->Write(size);
					}
					// chunk data
					{
						pFile->Write(index); // Sequence Track number
						temp = playLength; pFile->Write(temp); // Sequence length
						
						pFile->Write(pSequenceName,strlen(pSequenceName)+1); // Sequence Name

						for (unsigned int i = 0; i < playLength; i++)
						{
							temp = playOrder[i]; pFile->Write(temp); // Sequence data.
						}
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
				for(std::uint32_t index(0) ; index < MAX_PATTERNS; ++index)
				{
					// check every pattern for validity
					if (IsPatternUsed(index))
					{
						// ok save it

						unsigned char * pSource = new unsigned char[SONGTRACKS*patternLines[index]*EVENT_SIZE];
						unsigned char * pCopy = pSource;

						for (int y = 0; y < patternLines[index]; y++)
						{
							unsigned char * pData = ppPatternData[index]+(y*MULTIPLY);
							std::memcpy(pCopy,pData,EVENT_SIZE*SONGTRACKS);
							pCopy+=EVENT_SIZE*SONGTRACKS;
						}
						
						std::uint32_t sizez77 = BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[index]*EVENT_SIZE);
						delete[] pSource;

						// chunk header
						{
							pFile->Write("PATD",4);

							version = CURRENT_FILE_VERSION_PATD;
							pFile->Write(version);

							size = sizez77 + 4 * sizeof temp + strlen(patternName[index]) + 1;
							pFile->Write(size);
						}
						// chunk data
						{
							pFile->Write(index);
							temp = patternLines[index]; pFile->Write(temp);
							temp = SONGTRACKS; pFile->Write(temp); // eventually this may be variable per pattern

							pFile->Write(&patternName[index],strlen(patternName[index])+1);

							pFile->Write(sizez77);
							pFile->Write(pCopy,sizez77);
						}

						delete[] pCopy;
					}
				}

				if ( !autosave ) 
				{
					Progress.m_Progress.StepIt();
					::Sleep(1);
				}

				/*
				===================
				MACHINE DATA
				===================
				id = "MACD"; 
				*/
				// machine and instruments handle their save and load in their respective classes
				for(std::uint32_t index(0) ; index < MAX_MACHINES; ++index)
				{
					if (_pMachine[index])
					{
						std::fpos_t pos;

						// chunk header
						{
							pFile->Write("MACD",4);

							version = CURRENT_FILE_VERSION_MACD;
							pFile->Write(version);

							pos = pFile->GetPos();

							size = 0;
							pFile->Write(size);
						}
						// chunk data
						{
							pFile->Write(index);
							_pMachine[index]->SaveFileChunk(pFile);
						}
						// chunk size in header
						{
							std::fpos_t const pos2(pFile->GetPos());
							size = pos2 - pos - sizeof size;
							pFile->Seek(pos);
							pFile->Write(size);
							pFile->Seek(pos2);
						}

						if ( !autosave ) 
						{
							Progress.m_Progress.StepIt();
							::Sleep(1);
						}
					}
				}

				/*
				===================
				Instrument DATA
				===================
				id = "INSD"; 
				*/
				for(std::uint32_t index(0) ; index < MAX_INSTRUMENTS; ++index)
				{
					if (!_pInstrument[index]->Empty())
					{
						std::fpos_t pos;

						// chunk header
						{
							pFile->Write("INSD",4);

							version = CURRENT_FILE_VERSION_INSD;
							pFile->Write(version);

							pos = pFile->GetPos();

							size = 0;
							pFile->Write(size);
						}
						// chunk data
						{
							pFile->Write(index);
							_pInstrument[index]->SaveFileChunk(pFile);
						}
						// chunk size in header
						{
							std::fpos_t const pos2(pFile->GetPos());
							size = pos2 - pos - sizeof size;
							pFile->Seek(pos);
							pFile->Write(size);
							pFile->Seek(pos2);
						}

						if ( !autosave ) 
						{
							Progress.m_Progress.StepIt();
							::Sleep(1);
						}
					}
				}

				/*
				===
				end
				===
				*/

				if ( !autosave ) 
				{
					Progress.m_Progress.SetPos(chunkcount);
					::Sleep(1);
					Progress.OnCancel();
				}

				if (!pFile->Close()) throw std::runtime_error("couldn't close file");
			}
			catch(...)
			{
				std::ostringstream s;
				s << "Error writing to " << pFile->file_name() << " !!!";
				MessageBox(NULL,s.str().c_str(),"File Error!!!",0);
				return false;
			}
			return true;
		}

		void Song::PW_Play()
		{
			if (PW_Stage==0)
			{
				PW_Length=_pInstrument[PREV_WAV_INS]->waveLength;
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
			
			std::int16_t *wl=_pInstrument[PREV_WAV_INS]->waveDataL;
			std::int16_t *wr=_pInstrument[PREV_WAV_INS]->waveDataR;
			bool const stereo=_pInstrument[PREV_WAV_INS]->waveStereo;
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
				
			} while(--numSamples);
		}

		///\todo mfc+winapi->std
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

			CString filepath = Global::pConfig->GetSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			RiffFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.Write("MACD",4);
			std::uint32_t version = CURRENT_FILE_VERSION_MACD;
			file.Write(version);
			std::fpos_t pos = file.GetPos();
			std::uint32_t size = 0;
			file.Write(size);

			std::uint32_t index = dst; // index
			file.Write(index);

			_pMachine[src]->SaveFileChunk(&file);

			std::fpos_t pos2 = file.GetPos(); 
			size = pos2 - pos - sizeof size;
			file.Seek(pos);
			file.Write(size);
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
				file.Read(version);
				file.Read(size);
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(index);
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

			_pMachine[dst]->_connectedInputs = 0;
			_pMachine[dst]->_connectedOutputs = 0;

			for (int i = 0; i < MAX_CONNECTIONS; i++)
			{
				if (_pMachine[dst]->_connection[i])
				{
					_pMachine[dst]->_connection[i] = false;
					_pMachine[dst]->_outputMachines[i] = -1;
				}

				if (_pMachine[dst]->_inputCon[i])
				{
					_pMachine[dst]->_inputCon[i] = false;
					_pMachine[dst]->_inputMachines[i] = -1;
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

		///\todo mfc+winapi->std
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

			CString filepath = Global::pConfig->GetSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			RiffFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.Write("INSD",4);
			std::uint32_t version = CURRENT_FILE_VERSION_INSD;
			file.Write(version);
			std::fpos_t pos = file.GetPos();
			std::uint32_t size = 0;
			file.Write(size);

			std::uint32_t index = dst; // index
			file.Write(index);

			_pInstrument[src]->SaveFileChunk(&file);

			std::fpos_t pos2 = file.GetPos(); 
			size = pos2 - pos - sizeof size;
			file.Seek(pos);
			file.Write(size);

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
				file.Read(version);
				file.Read(size);
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(index);
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
			if(!ppPatternData[i]) return false;

			for (int j = 0; j < playLength; j++) if (playOrder[j] == i) return true;

			// check to see if it is empty
			{
				PatternEntry blank;
				unsigned char * pData = ppPatternData[i];
				for (int j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
					if (memcmp(pData+j,&blank,EVENT_SIZE) != 0 )
						return true;
			}

			return false;
		}
	}
}
