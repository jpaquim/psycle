///\file
///\brief implementation file for psycle::host::Song.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "song.hpp"
#include "machine.hpp"
#include "internal_machines.hpp"
#include "sampler.hpp"
#include "XMSampler.hpp"
#include "plugin.hpp"
#include "VSTHost.hpp"
#include <psycle/helpers/DataCompression.hpp>
#include <psycle/helpers/riff.hpp> // for Wave file loading.
//\todo:
#include <psycle/host/cacheddllfinder.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/Psy3filter.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
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
		bool Song::CreateMachine(Machine::class_type subclass, int x, int y, std::string const & plugin_name, Machine::id_type index)
		{
			Machine * machine(0);
			switch (subclass)
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
				case MACH_AUTOMATOR:
					machine = new Automator(index);
					break;
				case MACH_DUMMY:
					machine = new Dummy(index);
					break;
				case MACH_PLUGIN:
					{
						Plugin & plugin(*new Plugin(index));
						machine = &plugin;
						std::string path = plugin_name;
						/*
						if(!Global::dllfinder().LookupDllPath(path,MACH_PLUGIN)) 
						{
							delete &plugin;
							return false;
						}
						*/
						try
						{
							// Functions "LoadDll()" from both, Plugin and vst::plugin should be modified
							// in order to allow output to the logger. Else, it always shows the message to the user.
							plugin.LoadDll(path);
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
				case MACH_VST:
				case MACH_VSTFX:
					{
						vst::plugin * plugin(0);
						if (subclass == MACH_VST) machine = plugin = new vst::instrument(index);
						else if (subclass == MACH_VSTFX)	machine = plugin = new vst::fx(index);
						std::string path = plugin_name;
						/*
						if(!Global::dllfinder().LookupDllPath(path,MACH_VST)) 
						{
							delete plugin;
							return false;
						}
						*/
						try
						{
							plugin->Instance(path);
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
				if(machine->subclass() == MACH_VSTFX || machine->subclass() == MACH_VST )
				{
					
					// Do not call VST Init() function after Instance.
					machine->Machine::Init();
				}
				else machine->Init();
			}

			machine->SetPosX(x);
			machine->SetPosY(y);

			// Finally, activate the machine
			_pMachine[index] = machine;
			return true;
		}

		Machine::id_type Song::FindBusFromIndex(Machine::id_type smac)
		{
			if(!_pMachine[smac]) return Machine::id_type(255);
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
			tracks_= MAX_TRACKS;
			_machineLock = false;
			Invalided = false;
//			PW_Phase = 0;
//			PW_Stage = 0;
//			PW_Length = 0;
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
			for(Machine::id_type c(0); c < MAX_MACHINES; ++c)
			{
				if(_pMachine[c])
				{
					for(Machine::id_type j(c + 1); j < MAX_MACHINES; ++j)
					{
						if(_pMachine[c] == _pMachine[j])
						{
							///\todo wtf? duplicate machine? could happen if loader messes up?
							{
								std::ostringstream s;
								s << c << " and " << j << " have duplicate pointers";
								report.emit(s.str(), "Song Delete Error");
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
				if(Global::pConfig) patternLines[i]=Global::configuration().defaultPatLines;
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
					CSingleLock lock(&door,true);
				#endif
			#endif

			seqBus=0;

			///\todo std::string
			{
				std::memset(Name, 0, sizeof Name);
				std::memset(Author, 0, sizeof Author);
				std::memset(Comment, 0, sizeof Comment);
				std::sprintf(Name, "Untitled");
				std::sprintf(Author, "Unnamed");
				std::sprintf(Comment, "No Comments");
			}

			/// gui stuff
			currentOctave=4;

			// General properties
			{
				m_BeatsPerMin=125.0;
				m_LinesPerBeat=4;
			}
			// Clean up allocated machines.
			DestroyAllMachines(true);
			// Cleaning instruments
			DeleteInstruments();
			// Clear patterns
			DeleteAllPatterns();
			
			// Clear sequence
			Reset();

			instSelected = 0;
			midiSelected = 0;
			auxcolSelected = 0;

			_saved=false;
			fileName = "Untitled.psy";
			
			CreateMachine(MACH_MASTER, 320, 200, "master", MASTER_INDEX);
		}

		Machine::id_type Song::GetFreeMachine()
		{
			Machine::id_type tmac(0);
			for(;;)
			{
				if(!_pMachine[tmac]) return tmac;
				if(tmac++ >= MAX_MACHINES) return Machine::id_type(-1); // that's why ids can't be unsigned :-(
			}
		}

		bool Song::InsertConnection(Machine::id_type src, Machine::id_type dst, float volume)
		{
			Machine *srcMac = _pMachine[src];
			Machine *dstMac = _pMachine[dst];
			if(!srcMac || !dstMac)
			{
				std::ostringstream s;
				s << "attempted to use a null machine pointer when connecting machines ids: src: " << src << ", dst: " << dst << std::endl;
				s << "src pointer is " << srcMac << ", dst pointer is: " << dstMac;
				loggers::warning(s.str());
				return false;
			}
			return srcMac->ConnectTo(*dstMac, InPort::id_type(0), OutPort::id_type(0), volume);
		}

		int Song::ChangeWireDestMac(Machine::id_type wiresource, Machine::id_type wiredest, Wire::id_type wireindex)
		{
			Wire::id_type w;
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

		int Song::ChangeWireSourceMac(Machine::id_type wiresource, Machine::id_type wiredest, Wire::id_type wireindex)
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
						smac->_connection[wire] = false;
						smac->_outputMachines[wire] = 255;
						smac->_connectedOutputs--;

						_pMachine[wiredest]->_inputCon[wireindex] = false;
						_pMachine[wiredest]->_inputMachines[wireindex] = -1;
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

		void Song::DestroyMachine(Machine::id_type mac, bool write_locked)
		{
			#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
				#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
			#else
				#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
					boost::read_write_mutex::scoped_write_lock lock(read_write_mutex(), !write_locked); // only lock if not already locked
				#else // original implementation
					CSingleLock lock(&door, true);
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
			tracks(16);
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
					for(int t=0;t<tracks();t++)
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
					for(int t=0;t<tracks();t++)
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
					for(int t(0) ; t < tracks() ; ++t)
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

		bool Song::IffAlloc(Instrument::id_type instrument,const char * str)
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
				return false;
			}
			DeleteLayer(instrument);
			file.Read(data);
			if( data == file.FourCC("16SV")) bits = 16;
			else if(data == file.FourCC("8SVX")) bits = 8;
			file.Read(hd);
			if(hd._id == file.FourCC("NAME"))
			{
				file.ReadChunk(_pInstrument[instrument]->waveName, 22); _pInstrument[instrument]->waveName[21]=0;///\todo should be hd._size instead of "22", but it is incorrectly read.
				std::strncpy(_pInstrument[instrument]->_sName,str, 31);
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
						file.ReadChunk(&tmp, 2);
						*csamples = tmp.hilo * 256 + tmp.hihi;
						++csamples;
					}
				}
				else
				{
					for(unsigned int smp(0) ; smp < Datalen; ++smp)
					{
						file.ReadChunk(&tmp, 1);
						*csamples = tmp.hihi * 256 + tmp.hihi;
						++csamples;
					}
				}
			}
			file.Close();
			Invalided = false;
			return true;
		}

		bool Song::WavAlloc(Instrument::id_type iInstr, bool bStereo, long iSamplesPerChan, const char * sName)
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

		bool Song::WavAlloc(Instrument::id_type instrument,const char * Wavfile)
		{ 
			assert(Wavfile != 0);
			WaveFile file;
			ExtRiffChunkHeader hd;
			// opens the file and read the format Header.
			DDCRET retcode(file.OpenForRead(Wavfile));
			if(retcode != DDC_SUCCESS) 
			{
				Invalided = false;
				return false; 
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
							file.Read(smp8); ///\todo [bohan] is the lsb just discarded? [JosepMa]: yes. sampler only knows about 16bit samples
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
							file.Read(smp8); ///\todo [bohan] is the lsb just discarded? [JosepMa]: yes. sampler only knows about 16bit samples
							file.ReadData(sampL, 1);
							++sampL;
							file.Read(smp8); ///\todo [bohan] is the lsb just discarded? [JosepMa]: yes. sampler only knows about 16bit samples
							file.ReadData(sampR, 1);
							++sampR;
						}
						break;
					default:
						break; ///\todo should throw an exception
				}
			}
			retcode = file.Read(hd);
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
						if(!((ls <= 0) && (le >= Datalen - 1)))
						{
							_pInstrument[instrument]->waveLoopType = true;
						}
						else { ls = 0; le = 0;	}
					}
					file.Skip(9);
				}
				else if(hd.ckSize > 0)
					file.Skip(hd.ckSize);
				else
					file.Skip(1);
				retcode = file.Read(hd); ///\todo bloergh!
			}
			file.Close();
			Invalided = false;
			return true;
		}

		bool Song::Load(RiffFile* pFile, bool fullopen)
		{
			char Header[9];
			pFile->ReadChunk(&Header, 8);
			Header[8]=0;

			if (strcmp(Header,"PSY3SONG")==0)
			{
				loggers::trace("file header: PSY3SONG");
				//\todo: This is just an implementation in order to test and validate the new loader.
				Psy3Filter songloader;
				if (!songloader.Test(Header)) report.emit("songloader.test() hasn't worked!","bla");
				songloader.Load(pFile,*this);
				return true;

#if 0
				progress.emit(1,0,"");
				progress.emit(2,0,"Loading... psycle song fileformat version 3...");
				std::uint32_t version = 0;
				std::uint32_t size = 0;
				std::uint32_t index = 0;
				std::uint32_t temp;
				std::uint16_t temp16;
				std::uint32_t solo(0);
				std::uint32_t chunkcount=0;
				Header[4]=0;
				std::fpos_t filesize = pFile->FileSize();
				pFile->Read(version);
				pFile->Read(size);
				if(version > CURRENT_FILE_VERSION)
				{
					report.emit("This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning");
				}

				pFile->Read(chunkcount);
				if ( size > 4)
				{
					/*
					pFile->Read(fileversion);
					if (version == x)
					{}
					else if (...)
					{}
					// This is left here if someday, extra data is added to the file version chunk.
					// Modify "pFile->Skip(size - 4);" as necessary. Ex:  pFile->Skip(size - bytesread);
					}
					*/
					pFile->Skip(size - 4);// Size of the current Header DATA // This ensures that any extra data is skipped.
				}

				DestroyAllMachines();
				_machineLock = true;
				DeleteInstruments();
				DeleteAllPatterns();
				Reset(); //added by sampler mainly to reset current pattern showed.
				bool zero_size_foreign_chunk(false);
				/* chunk_loop: */
				while(pFile->ReadChunk(&Header, 4))
				{
					progress.emit(4,f2i((pFile->GetPos()*16384.0f)/filesize),"");
					::Sleep(1); ///< Allow screen refresh.
					// we should use the size to update the index, but for now we will skip it
					if(std::strcmp(Header,"INFO") == 0)
					{
						loggers::trace("chunk: INFO");
						progress.emit(2,0,"Loading... fileformat version information...");
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
						loggers::trace("chunk: SNGI");
						progress.emit(2,0,"Loading... authorship information...");
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_SNGI)
						{
							// there is an error, this file is newer than this build of psycle
							report.emit("Song Segment of File is from a newer version of psycle!","Song Load Error.");
							pFile->Skip(size);
						}
						else
						{
							// why all these temps?  to make sure if someone changes the defs of
							// any of these members, the rest of the file reads ok.  assume 
							// everything is 32-bit, when we write we do the same thing.

							// # of tracks for whole song
							pFile->Read(temp);
							tracks(temp);
							// bpm
							pFile->Read(temp16);
							int BPMCoarse = temp16;
							pFile->Read(temp16);
							m_BeatsPerMin = BPMCoarse + temp16/100.0f;
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
							for(int i(0) ; i < tracks(); ++i)
							{
								pFile->Read(_trackMuted[i]);
								// remember to count them
								pFile->Read(_trackArmed[i]);
								if(_trackArmed[i]) ++_trackArmedCount;
							}
							Global::player().SetBPM(m_BeatsPerMin,m_LinesPerBeat);
						}
					}
					else if(std::strcmp(Header,"SEQD")==0)
					{
						loggers::trace("chunk: SEQD");
						progress.emit(2,0,"Loading... sequence...");
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_SEQD)
						{
							// there is an error, this file is newer than this build of psycle
							report.emit("Sequence section of File is from a newer version of psycle!", "Song Load Error.");
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
								loggers::warning("Sequence section of File is from a newer version of psycle!");
								pFile->Skip(size - sizeof index);
							}
						}
					}
					else if(std::strcmp(Header,"PATD") == 0)
					{
						loggers::trace("chunk: PATD");
						progress.emit(2,0,"Loading... patterns...");
						--chunkcount;
						pFile->Read(version);
						pFile->Read(size);
						if(version > CURRENT_FILE_VERSION_PATD)
						{
							// there is an error, this file is newer than this build of psycle
							loggers::warning("Pattern section of File is from a newer version of psycle!");
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
								pFile->ReadChunk(pSource, size);
								unsigned char * pDest;
								BEERZ77Decomp2(pSource, &pDest);
								zapArray(pSource,pDest);
								for(int y(0) ; y < patternLines[index] ; ++y)
								{
									unsigned char* pData(_ppattern(index) + (y * MULTIPLY));
									std::memcpy(pData, pSource, tracks() * EVENT_SIZE);
									pSource += tracks() * EVENT_SIZE;
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
						loggers::trace("chunk: MACD");
						progress.emit(2,0,"Loading... machines...");
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
							loggers::warning("Machine section of File is from a newer version of psycle!");
							pFile->Skip(size);
						}
						else
						{
							pFile->Read(index);
							if(index < MAX_MACHINES)
							{
								Machine::id_type const id(index);
								// we had better load it
								DestroyMachine(id);
								_pMachine[index] = Machine::LoadFileChunk(pFile, id, version, fullopen);
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
						loggers::trace("chunk: INSD");
						progress.emit(2,0,"Loading... instruments...");
						pFile->Read(version);
						pFile->Read(size);
						--chunkcount;
						if(version > CURRENT_FILE_VERSION_INSD)
						{
							// there is an error, this file is newer than this build of psycle
							loggers::warning("Instrument section of File is from a newer version of psycle!");
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
						if(!zero_size_foreign_chunk)
						{
							loggers::warning("foreign chunk found. skipping it.");
							progress.emit(2,0,"Loading... foreign chunk found. skipping it...");
						}
						pFile->Read(version);
						pFile->Read(size);
						if(size)
						{
							std::ostringstream s;
							s << "foreign chunk: version: " << version << ", size: " << size;
							loggers::trace(s.str());
						}
						else if(!zero_size_foreign_chunk)
						{
							loggers::warning("foreign chunk: size is zero. supressing messages until non zero-sized chunk is found.");
						}
						zero_size_foreign_chunk = !size;
						bool skip_failed(false);
						try
						{
							pFile->Skip(size);
						}
						catch(...)
						{
							skip_failed = true;
						}
						if(skip_failed)
						{
							loggers::exception("foreign chunk is actually random/corrupted data. not reading further data.");
							//break chunk_loop;
							goto quit_chunk_loop;
						}
					}
				}
				quit_chunk_loop:
				// now that we have loaded all the modules, time to prepare them.
				progress.emit(4,16384,"");
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

				// allow stuff to work again
				machineSoloed = solo;
				_machineLock = false;
				progress.emit(5,0,"");
				if(chunkcount)
				{
					if (!_pMachine[MASTER_INDEX] )
					{
						_pMachine[MASTER_INDEX] = new Master(MASTER_INDEX);
						_pMachine[MASTER_INDEX]->Init();
					}
					std::ostringstream s;
					s << "Error reading from file '" << pFile->file_name() << "'" << std::endl;
					s << "some chunks were missing in the file";
					report.emit(s.str(), "Song Load Error.");
					return false;
				}
				return true;
#endif
			}
			else if(std::strcmp(Header, "PSY2SONG") == 0)
			{
				loggers::trace("chunk: PSY2SONG");
				return LoadOldFileFormat(pFile, fullopen);
			}

			// load did not work
			report.emit("Incorrect file format", "Song Load Error.");
			return false;
		}

		bool Song::Save(RiffFile* pFile,bool autosave)
		{
			// NEW FILE FORMAT!!!
			// this is much more flexible, making maintenance a breeze compared to that old hell.
			// now you can just update one module without breaking the whole thing.

			try
			{
				if ( !autosave ) 
				{
					progress.emit(1,0,"");
					progress.emit(2,0,"Saving...");
				}

				std::uint32_t version, size, temp, chunkcount;
				std::uint16_t temp16;

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
						progress.emit(3,chunkcount,"");
					}

					// chunk header
					{
						pFile->WriteChunk("PSY3SONG",8);

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
					progress.emit(4,-1,"");
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
						pFile->WriteChunk("INFO",4);

						version = CURRENT_FILE_VERSION_INFO;
						pFile->Write(version);

						size = strlen(Name)+strlen(Author)+strlen(Comment)+3; // [bohan] since those are variable length, we could change from fixed size arrays to std::string
						pFile->Write(size);
					}
					// chunk data
					{
						pFile->WriteChunk(Name,strlen(Name)+1);
						pFile->WriteChunk(Author,strlen(Author)+1);
						pFile->WriteChunk(Comment,strlen(Comment)+1);
					}
				}

				if ( !autosave ) 
				{
					progress.emit(4,-1,"");
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
						pFile->WriteChunk("SNGI",4);

						version = CURRENT_FILE_VERSION_SNGI;
						pFile->Write(version);

						size = (11*sizeof(temp))+(tracks()*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])));
						pFile->Write(size);
					}
					// chunk data
					{
						temp = tracks();     pFile->Write(temp);
						temp16 = int(floor(m_BeatsPerMin));							pFile->Write(temp16);
						temp16 = int((m_BeatsPerMin-floor(m_BeatsPerMin))*100);		pFile->Write(temp16);
						temp = m_LinesPerBeat; pFile->Write(temp);
						temp = currentOctave;  pFile->Write(temp);
						temp = machineSoloed;  pFile->Write(temp);
						temp = _trackSoloed;   pFile->Write(temp);

						temp = seqBus; pFile->Write(temp);

						temp = midiSelected; pFile->Write(temp);
						temp = auxcolSelected; pFile->Write(temp);
						temp = instSelected; pFile->Write(temp);

						temp = 1;  pFile->Write(temp); // sequence width

						for(int i = 0; i < tracks(); i++)
						{
							pFile->Write(_trackMuted[i]);
							pFile->Write(_trackArmed[i]); // remember to count them
						}
					}
				}

				if ( !autosave ) 
				{
					progress.emit(4,-1,"");
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
						pFile->WriteChunk("SEQD",4);

						version = CURRENT_FILE_VERSION_SEQD;
						pFile->Write(version);

						size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
						pFile->Write(size);
					}
					// chunk data
					{
						pFile->Write(index); // Sequence Track number
						temp = playLength; pFile->Write(temp); // Sequence length
						
						pFile->WriteChunk(pSequenceName,strlen(pSequenceName)+1); // Sequence Name

						for (int i = 0; i < playLength; i++)
						{
							temp = playOrder[i]; pFile->Write(temp); // Sequence data.
						}
					}
				}

				if ( !autosave ) 
				{
					progress.emit(4,-1,"");
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

						unsigned char * pSource = new unsigned char[tracks()*patternLines[index]*EVENT_SIZE];
						unsigned char * pCopy = pSource;

						for (int y = 0; y < patternLines[index]; y++)
						{
							unsigned char * pData = ppPatternData[index]+(y*MULTIPLY);
							std::memcpy(pCopy,pData,EVENT_SIZE*tracks());
							pCopy+=EVENT_SIZE*tracks();
						}
						
						std::uint32_t sizez77 = BEERZ77Comp2(pSource, &pCopy, tracks()*patternLines[index]*EVENT_SIZE);
						delete[] pSource;

						// chunk header
						{
							pFile->WriteChunk("PATD",4);

							version = CURRENT_FILE_VERSION_PATD;
							pFile->Write(version);

							size = sizez77 + 4 * sizeof temp + strlen(patternName[index]) + 1;
							pFile->Write(size);
						}
						// chunk data
						{
							pFile->Write(index);
							temp = patternLines[index]; pFile->Write(temp);
							temp = tracks(); pFile->Write(temp); // eventually this may be variable per pattern

							pFile->WriteChunk(&patternName[index],strlen(patternName[index])+1);

							pFile->Write(sizez77);
							pFile->WriteChunk(pCopy,sizez77);
						}

						delete[] pCopy;
					}
				}

				if ( !autosave ) 
				{
					progress.emit(4,-1,"");
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
							pFile->WriteChunk("MACD",4);

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
							progress.emit(4,-1,"");
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
							pFile->WriteChunk("INSD",4);

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
							progress.emit(4,-1,"");
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
					progress.emit(5,0,"");
				}
			}
			catch(...)
			{
				std::ostringstream s;
				s << "Error writing to " << pFile->file_name() << " !!!";
				report.emit(s.str(),"File Save Error.");
				progress.emit(5,0,"");
				return false;
			}
			return true;
		}

		void Song::DoPreviews(int amount)
		{
			//todo do better.. use a vector<InstPreview*> or something instead
			if(wavprev.IsEnabled())
			{
				wavprev.Work(_pMachine[MASTER_INDEX]->_pSamplesL, _pMachine[MASTER_INDEX]->_pSamplesR, amount);
			}
			if(waved.IsEnabled())
			{
				waved.Work(_pMachine[MASTER_INDEX]->_pSamplesL, _pMachine[MASTER_INDEX]->_pSamplesR, amount);
			}
		}

		///\todo mfc+winapi->std
		bool Song::CloneMac(Machine::id_type src, Machine::id_type dst)
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

			boost::filesystem::path path(Global::configuration().GetSongDir(), boost::filesystem::native);
			path /= "psycle.tmp";

			boost::filesystem::remove(path);

			RiffFile file;
			if(!file.Create(path.string(), true)) return false;

			file.WriteChunk("MACD",4);
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

			if(!file.Open(path.string()))
			{
				boost::filesystem::remove(path);
				return false;
			}

			char Header[5];
			file.ReadChunk(&Header, 4);
			Header[4] = 0;
			if (strcmp(Header,"MACD")==0)
			{
				file.Read(version);
				file.Read(size);
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					boost::filesystem::remove(path);
					return false;
				}
				else
				{
					file.Read(index);
					index = dst;
					if (index < MAX_MACHINES)
					{
						Machine::id_type id(index);
						// we had better load it
						DestroyMachine(id);
						_pMachine[index] = Machine::LoadFileChunk(&file,id,version);
					}
					else
					{
						file.Close();
						boost::filesystem::remove(path);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				boost::filesystem::remove(path);
				return false;
			}
			file.Close();
			boost::filesystem::remove(path);

			_pMachine[dst]->SetPosX(_pMachine[dst]->GetPosX()+32);
			_pMachine[dst]->SetPosY(_pMachine[dst]->GetPosY()+8);

			// delete all connections

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

			///\todo rewrite this so that it auto-numerates/auto-increases number.
			std::stringstream s;
			s << _pMachine[dst]->GetEditName() << " " << std::hex << dst << " (cloned from " << std::hex << src << ")";
			_pMachine[dst]->SetEditName(s.str());
			return true;
		}

		///\todo mfc+winapi->std
		bool Song::CloneIns(Instrument::id_type src, Instrument::id_type dst)
		{
			// src has to be occupied and dst must be empty
			if (!Global::song()._pInstrument[src]->Empty() && !Global::song()._pInstrument[dst]->Empty())
			{
				return false;
			}
			if (!Global::song()._pInstrument[dst]->Empty())
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (Global::song()._pInstrument[src]->Empty())
			{
				return false;
			}
			// ok now we get down to business
			// save our file

			CString filepath = Global::configuration().GetSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			RiffFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.WriteChunk("INSD",4);
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
			file.ReadChunk(&Header, 4);
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

		bool Song::IsPatternUsed(int i) const
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
