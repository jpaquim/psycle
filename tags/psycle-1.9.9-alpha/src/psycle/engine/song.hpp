///\file
///\brief interface file for psycle::host::Song
#pragma once
#include "SongStructs.hpp"
#include "instrument.hpp"
#include "InstPreview.hpp"
#include "machine.hpp"
#include "constants.hpp" // for the bloat-sized arrays and many other stuffs that should actually be moved in this file
#include <psycle/engine/global.hpp>
#include "FileIO.hpp"
#include <cstdint>
#include <sigslot/sigslot.h>


#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
	#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
		#include <boost/thread/read_write_mutex.hpp>
	#else // original implementation
		class CCriticalSection;
	#endif
#endif



namespace psycle
{
	namespace host
	{
		/// Index of MasterMachine
		Machine::id_type const MASTER_INDEX(128);

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines and their initial parameters and coordinates, wavetables, ...
		class Song
		{
			public:
				/// constructor.
				Song();

				/// Initializes the song to an empty one.
				void New();

				/// destructor.
				virtual inline ~Song() throw();

				sigslot::signal2<const std::string &, const std::string &> report;
				sigslot::signal3<const std::uint32_t& , const std::uint32_t& , const std::string& > progress;
			///\name initial values for player-related stuff
			///\{
				public:
					typedef float BeatsPerMinType;
					const BeatsPerMinType BeatsPerMin(){return m_BeatsPerMin;};
					void BeatsPerMin(const BeatsPerMinType value)
					{ 
						if ( value < BeatsPerMinType(32) ) m_BeatsPerMin = BeatsPerMinType(32);
						else if ( value > BeatsPerMinType(999.99) ) m_BeatsPerMin = BeatsPerMinType(999.99);
						else m_BeatsPerMin = value;
					};

					const int LinesPerBeat(){return m_LinesPerBeat;};
					void LinesPerBeat(const int value)
					{
						if ( value < 1 )m_LinesPerBeat = 1;
						else if ( value > 31 ) m_LinesPerBeat = 31;
						else m_LinesPerBeat = value;
					};
			///\}

			///\name patterns
			///\{
				public:
					/// the number of pattern used in this song.
					int GetNumPatternsUsed();
					/// Returns the first unused pattern in the pPatternData[] Array.
					int GetBlankPatternUnused(int rval = 0);
					/// creates a new pattern.
					bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
					/// deletes all the patterns of this song.
					void DeleteAllPatterns();
					/// Used to detect if an especific pattern index contains any data.
					bool IsPatternUsed(int i) const;
					/// removes a pattern from this song.
					void RemovePattern(int ps);
			///\}

			///\name pattern tracks
			///\{
				public:
					/// The number of tracks in each pattern of this song.
					///\todo it should be unsigned but there's somewhere a piece of code that messes negative integers with this value
					/* unsigned */ int inline tracks() const throw() { return tracks_; }
					/// The number of tracks in each pattern of this song.
					///\todo it should be unsigned but there's somewhere a piece of code that messes negative integers with this value
					void inline tracks(/* unsigned */  int const tracks) throw() { assert(tracks >= 0); assert(tracks < MAX_TRACKS); this->tracks_ = tracks; }
				private:
					unsigned int tracks_;
			///\}

			///\name pattern low-level/memory access. \todo shouldn't be public considering it's way too low-level
			///\{
				public: // <-- argh!
					/// Returns the start offset of the requested pattern in memory, and creates one if none exists.
					/// This function now is the same as doing &pPatternData[ps]
					PSYCLE__DEPRECATED("This sux.")
					inline unsigned char * _ppattern(int ps)
					{
						if(!ppPatternData[ps]) return CreateNewPattern(ps);
						return ppPatternData[ps];
					};
					/// Returns the start offset of the requested track of pattern ps in the
					/// pPatternData Array and creates one if none exists.
					PSYCLE__DEPRECATED("This sux.")
					inline unsigned char * _ptrack(int ps, int track)
					{
						if(!ppPatternData[ps]) return CreateNewPattern(ps)+ (track*EVENT_SIZE);
						return ppPatternData[ps] + (track*EVENT_SIZE);
					};
					/// Returns the start offset of the requested line of the track of pattern ps in
					/// the pPatternData Array and creates one if none exists.
					PSYCLE__DEPRECATED("This sux.")
					inline unsigned char * _ptrackline(int ps, int track, int line)
					{
						if(!ppPatternData[ps]) return CreateNewPattern(ps)+ (track*EVENT_SIZE) + (line*MULTIPLY);
						return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
					};
					/// Allocates the memory fo a new pattern at position ps of the array pPatternData.
					///\todo doc ... how does this differs from bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
					PSYCLE__DEPRECATED("This sux.")
					unsigned char * CreateNewPattern(int ps);
			///\}

			///\name machines
			///\{
				public:
					/// creates a new machine in this song.
					Machine & CreateMachine(Machine::class_type subclass, int x, int y, std::string const & plugin_name = "dummy") throw(std::exception)
					{
						Machine::id_type const array_index(GetFreeMachine());
						if(array_index < 0) throw std::runtime_error("sorry, psycle doesn't dynamically allocate memory.");
						if(!CreateMachine(subclass, x, y, plugin_name, array_index))
							throw std::runtime_error("something bad happened while i was trying to create a machine, but i forgot what it was.");
						return *_pMachine[array_index];
					}

					/// creates a new machine in this song.
					bool CreateMachine(Machine::class_type, int x, int y, std::string const & plugin_name, Machine::id_type);

					/// Gets the first free slot in the pMachine[] Array
					///\todo it's low-level.. should be private.
					/// we have higer-level CreateMachine and CloneMachine functions already
					//PSYCLE__DEPRECATED("low-level")
					Machine::id_type GetFreeMachine();

					/// destroy a machine of this song.
					void DestroyMachine(Machine & machine, bool write_locked = false) { DestroyMachine(machine.id()); /* stupid circonvolution */ }

					/// destroy a machine of this song.
					void DestroyMachine(Machine::id_type mac, bool write_locked = false);

					/// destroys all the machines of this song.
					void DestroyAllMachines(bool write_locked = false);

					/// clones a machine.
					bool CloneMac(Machine & src, Machine & dst) { CloneMac(src.id(), dst.id()); /* stupid circonvolution */ }

					/// clones a machine.
					bool CloneMac(Machine::id_type src, Machine::id_type dst);
			///\}

			///\name machine connections
			///\{
				public:
					/// creates a new connection between two machines.
					///\todo kinda useless since machines can connect themselves with their ConnectTo function
					bool InsertConnection(Machine::id_type src, Machine::id_type dst, float volume = 1.0f);

					/// Changes the destination of a wire connection.
					///\param wiresource source mac index
					///\param wiredest new dest mac index
					///\param wireindex index of the wire in wiresource to change
					int ChangeWireDestMac(Machine::id_type wiresource, Machine::id_type wiredest, Wire::id_type wireindex);

					/// Changes the destination of a wire connection.
					///\param wiredest dest mac index
					///\param wiresource new source mac index
					///\param wireindex index of the wire in wiredest to change
					int ChangeWireSourceMac(Machine::id_type wiresource, Machine::id_type wiredest, Wire::id_type wireindex);

					/// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
					int GetFreeBus();
					/// Gets the first free slot in the Effects' bus (slots MAX_BUSES  to 2*MAX_BUSES-1)
					int GetFreeFxBus();
					/// Returns the Bus index out of a pMachine index.
					Machine::id_type FindBusFromIndex(Machine::id_type smac);
			///\}

			///\name instruments
			///\{
				public:
					/// clones an instrument.
					bool CloneIns(Instrument::id_type src, Instrument::id_type dst);
					/// resets the instrument and delete each sample/layer that it uses.
					void /*Reset*/DeleteInstrument(Instrument::id_type id) { Invalided=true; _pInstrument[id]->Delete(); Invalided=false; }
					/// resets the instrument and delete each sample/layer that it uses. (all instruments)
					void /*Reset*/DeleteInstruments() { Invalided=true; for(Instrument::id_type id(0) ; id < MAX_INSTRUMENTS ; ++id) _pInstrument[id]->Delete(); Invalided=false; }
					/// delete all instruments in this song.
					void /*Delete*/DestroyAllInstruments() { for(Instrument::id_type id(0) ; id < MAX_INSTRUMENTS ; ++id) { delete _pInstrument[id]; _pInstrument[id] = 0; } }
					// Removes the sample/layer of the instrument
					void DeleteLayer(Instrument::id_type id) { _pInstrument[id]->DeleteLayer(); }
			///\}

			///\name wavetable
			///\{
				public:
					/// ???
					bool WavAlloc(Instrument::id_type, const char * str);
					/// ???
					bool WavAlloc(Instrument::id_type, bool bStereo, long int iSamplesPerChan, const char * sName);
					/// ???
					bool IffAlloc(Instrument::id_type, const char * str);
			///\}

			///\name wave file previewing
			///\todo shouldn't belong to the song class.
			///\{
				public:
					//todo these ought to be dynamically allocated
					/// Wave preview.
					InstPreview wavprev;
					/// Wave editor playback.
					InstPreview waved;
					/// runs the wave previewing.
					void DoPreviews(int amount);
			///\}

			///\name (de)serialization
			///\{
				public:
					/// loads a file into this song object.
					///\param fullopen  used in context of the winamp/foobar player plugins (\todo doc not only!), where it allows to get the info of the file, without needing to open it completely.
					bool Load(RiffFile* pFile, bool fullopen=true);

					/// saves this song to a file.
					bool Save(RiffFile* pFile,bool autosave=false);

					#if !defined PSYCLE__CONFIGURATION__SERIALIZATION
						#error PSYCLE__CONFIGURATION__SERIALIZATION isn't defined! Check the code where this error is triggered.
					#elif PSYCLE__CONFIGURATION__SERIALIZATION
						/// saves this song to a file, as XML.
						void SaveXML(std::string const & file_name) throw(std::exception);
						friend class boost::serialization::access;
					#endif
			///\}

			///\name cpu cost measurement
			///\{
				public:
					void             inline cpu_idle(cpu::cycles_type const & value)       throw() { cpu_idle_ = value; }
					cpu::cycles_type inline cpu_idle(                              ) const throw() { return cpu_idle_; }
				private:
					cpu::cycles_type        cpu_idle_;

				public: ///\todo public->private
					/// Number of samples processed since all cpu cost counters were reset.
					/// We accumulate this sample count along with cpu costs until we compute the percentages, for example, every second.
					unsigned int _sampCount;
			///\}

			///\name IsInvalided
			///\todo doc ... what's that?
			///\{
				public:
					bool IsInvalided(){return Invalided;};
					void IsInvalided(const bool value){Invalided = value;};
				private:
					/// \todo doc
					bool Invalided;
			///\}

			///\name thread synchronization
			///\{
				#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
					#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
				#else
					#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
						public:
							boost::read_write_mutex inline & read_write_mutex() const { return this->read_write_mutex_; }
						private:
							boost::read_write_mutex mutable  read_write_mutex_;
					#else // original implementation
						public: // \todo public->private
							CCriticalSection mutable door;
					#endif
				#endif
			///\}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//\todo below are unencapsulated data members

			PSYCLE__PRIVATE:

				///\name authorship
				///\{
					/// the name of the song.
					///\todo hardcoded limits and wastes
					char Name[64];
					/// the author of the song.
					///\todo hardcoded limits and wastes
					char Author[64];
					/// the comments on the song
					///\todo hardcoded limits and wastes
					char Comment[256];
				///\}

				///\name patterns
				///\{
					/// Pattern name 
					///\todo hardcoded limits and wastes
					char patternName[MAX_PATTERNS][32];

					/// number of lines of each pattern
					///\todo hardcoded limits and wastes
					int patternLines[MAX_PATTERNS];

					/// Array of Pattern data.
					///\todo hardcoded limits and wastes
					PSYCLE__DEPRECATED("This sux.")
					unsigned char * ppPatternData[MAX_PATTERNS];

					#if 0 // more lightweight
						class pattern
						{
							private:
								std::string name;
								unsigned int lines, tracks;
								std::vector<PatternEntries> entries;
						};
						std::vector<pattern> patterns;
					#endif
				///\}

				///\name pattern sequence
				///\{
					/// Length, in patterns, of the sequence.
					int playLength;

					/// Sequence of patterns.
					///\todo hardcoded limits and wastes
					unsigned char playOrder[MAX_SONG_POSITIONS];

					/// Selection of patterns (for the "playBlock()" play mode)
					///\todo hardcoded limits and wastes
					bool playOrderSel[MAX_SONG_POSITIONS];
				///\}

				///\name machines
				///\{
					/// Sort of semaphore to not allow doing something with machines when they are changing (deleting,creating, etc..)
					/// \todo change it by a real semaphore?
					bool _machineLock;
					/// the array of machines.
					///\todo hardcoded limits and wastes
					///\todo more lightweight with a std::vector<Machine*>
					Machine* _pMachine[MAX_MACHINES];
					/// Current selected machine number in the GUI
					/// \todo This is a gui thing... should not be here.
					Machine::id_type seqBus;
				///\}

				///\name instruments
				///\{
					///\todo doc
					Instrument::id_type instSelected;
					///\todo doc
					///\todo hardcoded limits and wastes
					Instrument * _pInstrument[MAX_INSTRUMENTS];
				///\}

				///\name initial values for player-related stuff
				///\{
					/// the initial beats per minute (BPM) when the song is started playing.
					/// This can be changed in patterns using a command, but this value will not be affected.
					float m_BeatsPerMin;

					/// the initial ticks per beat (TPB) when the song is started playing.
					/// This can be changed in patterns using a command, but this value will not be affected.
					int m_LinesPerBeat;
				///\}

				///\name various player-related stuff
				///\{
					/// The index of the selected MIDI program for note entering
					/// \todo This is a gui thing... should not be here.
					int midiSelected;
					/// The index for the auxcolumn selected (would be waveselected, midiselected, or an index to a machine parameter)
					/// \todo This is a gui thing... should not be here.
					int auxcolSelected;
					/// Wether each of the tracks is muted.
					///\todo hardcoded limits and wastes
					bool _trackMuted[MAX_TRACKS];
					/// The number of tracks Armed (enabled for record)
					/// \todo should this be here? (used exclusively in childview)
					int _trackArmedCount;
					/// Wether each of the tracks is armed (selected for recording data in)
					///\todo hardcoded limits and wastes
					bool _trackArmed[MAX_TRACKS];
					/// The index of the machine which plays in solo.
					///\todo ok it's saved in psycle "song" files, but that belongs to the player.
					Machine::id_type machineSoloed;
					/// The index of the track which plays in solo.
					///\todo ok it's saved in psycle "song" files, but that belongs to the player.
					int _trackSoloed;
				///\}

				///\name file-related stuff
				///\{
					/// The file name this song was loaded from.
					std::string fileName;

					/// Is this song saved to a file?
					bool _saved;
				///\}

				///\name undocumented/missplaced stuff
				///\{
					/// \todo doc
					/// \todo If, whatever it is, is a GUI thing, it should not be here.
					CPoint viewSize;

					/// \todo This is a GUI thing... should not be here.
					char currentOctave;

					// The volume of the preview wave in the wave load dialog.
					/// \todo This is a GUI thing... should not be here.
					float preview_vol; 
				///\}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// private implementation details

			private:

				/// Resets some variables to their default values (used inside New(); )
				void Reset();

				/// used only during loading?
				/// \todo why is this here?
				class VSTLoader
				{
					public:
						bool valid;
						char dllName[128];
						int numpars;
						float * pars;
				};

				/// Loader for psycle fileformat version 2.
				bool LoadOldFileFormat(RiffFile* pFile, bool fullopen);
		};
	}
}
