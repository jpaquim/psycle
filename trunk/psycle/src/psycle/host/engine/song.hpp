///\file
///\brief interface file for psycle::host::Song
#pragma once
#include <psycle/host/engine/constants.hpp>
#include <psycle/host/engine/FileIO.hpp>
#include <psycle/host/engine/SongStructs.hpp>
#include <psycle/host/engine/instrument.hpp>
#include <psycle/host/global.hpp>
#include <cstdint>



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
		class Machine; // forward declaration

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

			///\name initial values for player-related stuff
			///\{
				public:
					const int BeatsPerMin(){return m_BeatsPerMin;};
					void BeatsPerMin(const int value)
					{ 
						if ( value < 32 ) m_BeatsPerMin = 32;
						else if ( value > 999 ) m_BeatsPerMin = 999;
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
					bool IsPatternUsed(int i);
					/// removes a pattern from this song.
					void RemovePattern(int ps);
			///\}

			///\name pattern tracks
			///\{
				public:
					/// The number of tracks in each pattern of this song.
					unsigned int inline tracks() const throw() { return tracks_; }
					/// The number of tracks in each pattern of this song.
					void inline tracks(unsigned int const tracks) throw() { this->tracks_ = tracks_; }
				private:
					unsigned int tracks_;
			///\}

			///\name pattern low-level/memory access. \todo shouldn't be public considering it's way too low-level
			///\{
				public: // <-- argh!
					/// Returns the start offset of the requested pattern in memory, and creates one if none exists.
					/// This function now is the same as doing &pPatternData[ps]
					inline unsigned char * _ppattern(int ps){
						if(!ppPatternData[ps]) return CreateNewPattern(ps);
						return ppPatternData[ps];
					};
					/// Returns the start offset of the requested track of pattern ps in the
					/// pPatternData Array and creates one if none exists.
					inline unsigned char * _ptrack(int ps, int track){
						if(!ppPatternData[ps]) return CreateNewPattern(ps)+ (track*EVENT_SIZE);
						return ppPatternData[ps] + (track*EVENT_SIZE);
					};
					/// Returns the start offset of the requested line of the track of pattern ps in
					/// the pPatternData Array and creates one if none exists.
					inline unsigned char * _ptrackline(int ps, int track, int line){
						if(!ppPatternData[ps]) return CreateNewPattern(ps)+ (track*EVENT_SIZE) + (line*MULTIPLY);
						return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
					};
					/// Allocates the memory fo a new pattern at position ps of the array pPatternData.
					///\todo doc ... how does this differs from bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
					unsigned char * CreateNewPattern(int ps);
			///\}

			///\name machines
			///\{
				public:
					/// Gets the first free slot in the pMachine[] Array
					int GetFreeMachine();
					/// creates a new machine in this song.
					bool CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int index);
					/// destroy a machine of this song.
					void DestroyMachine(int mac, bool write_locked = false);
					/// destroys all the machines of this song.
					void DestroyAllMachines(bool write_locked = false);
					/// clones a machine.
					bool CloneMac(int src,int dst);
			///\}

			///\name machine connections
			///\{
				public:
					/// creates a new connection between two machines.
					bool InsertConnection(int src,int dst,float value = 1.0f);
					/// Changes the destination of a wire connection. wiresource= source mac index, wiredest= new dest mac index, wireindex= index of the wire in wiresource to change.
					int ChangeWireDestMac(int wiresource, int wiredest, int wireindex);
					/// Changes the destination of a wire connection. wiredest= dest mac index, wiresource= new source mac index, wireindex= index of the wire in wiredest to change.
					int ChangeWireSourceMac(int wiresource, int wiredest, int wireindex);
					/// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
					int GetFreeBus();
					/// Gets the first free slot in the Effects' bus (slots MAX_BUSES  to 2*MAX_BUSES-1)
					int GetFreeFxBus();
					/// Returns the Bus index out of a pMachine index.
					int FindBusFromIndex(int smac);
			///\}

			///\name instruments
			///\{
				public:
					/// clones an instrument.
					bool CloneIns(int src,int dst);
					/// deletes (resets) the instrument and deletes (and resets) each sample/layer that it uses.
					void DeleteInstrument(int i);
					/// deletes (resets) the instrument and deletes (and resets) each sample/layer that it uses. (all instruments)
					/// \todo doc ... What does this function really do?
					void DeleteInstruments();
					/// destroy all instruments in this song.
					/// \todo doc ... What does this function really do?
					void DestroyAllInstruments();
					// Removes the sample/layer of the instrument "instrument"
					void DeleteLayer(int instrument);
			///\}

			///\name wavetable
			///\{
				public:
					/// ???
					int WavAlloc(int iInstr,const char * str);
					/// ???
					int WavAlloc(int iInstr,bool bStereo,long iSamplesPerChan,const char * sName);
					/// ???
					int IffAlloc(int instrument,const char * str);
			///\}

			///\name wave file previewing
			///\todo shouldn't belong to the song class.
			///\{
				public:
					/// wave file previewing: work... \todo doc
					void PW_Work(float *psamplesL, float *pSamplesR, int numSamples);
					/// wave file previewing: starts the playback.
					void PW_Play();
					/// wave file previewing: current playback position, in samples.
					int PW_Phase;
					/// wave file previewing: Stage. 0 = Stopped. 1 = Playing. \todo doc
					int PW_Stage;
					/// wave file previewing: length of the wave (in samples?) \todo doc
					int PW_Length;
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

			/* private: */ public:

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
					unsigned char * ppPatternData[MAX_PATTERNS];
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
					Machine* _pMachine[MAX_MACHINES];
					/// Current selected machine number in the GUI
					/// \todo This is a gui thing... should not be here.
					int seqBus;
				///\}

				///\name instruments
				///\{
					///\todo doc
					int instSelected;
					///\todo doc
					///\todo hardcoded limits and wastes
					Instrument * _pInstrument[MAX_INSTRUMENTS];
				///\}

				///\name initial values for player-related stuff
				///\{
					/// the initial beats per minute (BPM) when the song is started playing.
					/// This can be changed in patterns using a command, but this value will not be affected.
					int m_BeatsPerMin;

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
					int machineSoloed;
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
