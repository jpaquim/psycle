///\file
///\brief interface file for psycle::host::Song
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "FileIO.hpp"
#include "Instrument.hpp"
#include "XMInstrument.hpp"
#include "ExclusiveLock.hpp"
#include "cpu_time_clock.hpp"
#include "InstPreview.hpp"
#include <universalis/stdlib/chrono.hpp>
#include <universalis/os/loggers.hpp>

class CCriticalSection;

namespace psycle
{
	namespace host
	{
		class Machine; // forward declaration
		class CProgressDialog;

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines and their initial parameters and coordinates, wavetables, ...
		class Song
		{
			static int defaultPatLines;
		public:
			/// constructor.
			Song();
			/// destructor.
			virtual ~Song() throw();

			static void SetDefaultPatLines(int lines);
			static int GetDefaultPatLines() { return defaultPatLines; };

			/// the name of the song.
			std::string name;
			/// the author of the song.
			std::string author;
			/// the comments on the song
			std::string comments;
			/// the initial beats per minute (BPM) when the song is started playing.
			/// This can be changed in patterns using a command, but this value will not be affected.
			int m_BeatsPerMin;
			/// the initial ticks per beat (TPB) when the song is started playing.
			/// This can be changed in patterns using a command, but this value will not be affected.
			int m_LinesPerBeat;
			/// \todo This is a GUI thing... should not be here.
			char currentOctave;
			/// Array of Pattern data.
			unsigned char * ppPatternData[MAX_PATTERNS];
			/// Length, in patterns, of the sequence.
			int playLength;
			/// Sequence of patterns.
			unsigned char playOrder[MAX_SONG_POSITIONS];
			/// Selection of patterns (for the "playBlock()" play mode)
			bool playOrderSel[MAX_SONG_POSITIONS];
			/// number of lines of each pattern
			int patternLines[MAX_PATTERNS];
			/// Pattern name 
			char patternName[MAX_PATTERNS][32];
			/// The number of tracks in each pattern of this song.
			int SONGTRACKS;
			/// ???
			///\name instrument
			///\{
			///
			int instSelected;
			///
			Instrument * _pInstrument[MAX_INSTRUMENTS];
			SampleList samples;
			InstrumentList xminstruments;
			///\}
			/// The index of the selected MIDI program for note entering
			/// \todo This is a gui thing... should not be here.
			int paramSelected;
			/// The index for the auxcolumn selected (would be waveselected, midiselected, or an index to a machine parameter)
			/// \todo This is a gui thing... should not be here.
			int auxcolSelected;
			/// Wether each of the tracks is muted.
			bool _trackMuted[MAX_TRACKS];
			/// The number of tracks Armed (enabled for record)
			/// \todo should this be here? (used exclusively in childview)
			int _trackArmedCount;
			/// Wether each of the tracks is armed (selected for recording data in)
			bool _trackArmed[MAX_TRACKS];
			/// The names of the trakcs
			std::string _trackNames[MAX_PATTERNS][MAX_TRACKS];
			void ChangeTrackName(int patIdx, int trackidx, std::string name);
			void SetTrackNameShareMode(bool shared);
			void CopyNamesFrom(int patorigin, int patdest);
			bool shareTrackNames;
			///\name machines
			///\{
			/// the array of machines.
			Machine* _pMachine[MAX_MACHINES];
			/// Current selected machine number in the GUI
			/// \todo This is a gui thing... should not be here.
			int seqBus;
			///\name wavetable
			///\{
			/// ???
			int WavAlloc(int iInstr,const char * str);
			/// ???
			int WavAlloc(int iInstr,bool bStereo,std::uint32_t iSamplesPerChan,const char * sName);
			/// ???
			int IffAlloc(int instrument,const char * str);
			///\}
			/// Initializes the song to an empty one. (thread safe)
			void New();
			/// Initializes the song to an empty one. (non thread safe)
			void DoNew();
			/// Resets some variables to their default values (used inside New(); )
			void Reset();
			/// Gets the first free slot in the pMachine[] Array
			int GetFreeMachine() const ;
			/// creates a new machine in this song.
		protected:
			Machine* Song::CreateMachine(MachineType type, char const* psPluginDll,int songIdx,std::int32_t shellIdx);
		public:
			/// creates a new machine in this song.
			bool CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int songIdx,std::int32_t shellIdx=0);
			/// Creates a new machine, replacing an existing one.
			bool ReplaceMachine(Machine* origmac, MachineType type, int x, int y, char const* psPluginDll, int songIdx,std::int32_t shellIdx=0);
			/// exchanges the position of two machines.
			bool ExchangeMachines(int one, int two);
			/// destroy a machine of this song.
			void DestroyMachine(int mac);
			/// destroys a machine, but rewires its neighbours
			void DeleteMachineRewiring(int macIdx);
			/// destroys all the machines of this song.
			void DestroyAllMachines();
			/// Tells all the samplers of this song, that if they play this sample, stop playing it. (I know this isn't exactly a thing to do for a Song Class)
			void StopInstrument(int instrumentIdx);
			// the highest index of the instruments used
			int GetHighestInstrumentIndex() const;
			// the highest index of the patterns used
			int GetHighestPatternIndexInSequence() const;
			// the number of instruments used.
			int GetNumInstruments() const;
			/// the number of pattern used in this song.
			int GetNumPatterns() const;

			/// creates a new connection between two machines. returns index in the dest machine, or -1 if error.
			int InsertConnectionBlocking(Machine* srcMac,Machine* dstMac,int srctype=0, int dsttype=0,float value = 1.0f)
			{
				CExclusiveLock lock(&semaphore, 2, true);
				return InsertConnectionNonBlocking(srcMac, dstMac, srctype, dsttype, value);
			}
			int InsertConnectionNonBlocking(Machine* srcMac,Machine* dstMac,int srctype=0, int dsttype=0,float value = 1.0f);
			int InsertConnectionNonBlocking(int srcMac,int dstMac,int srctype=0, int dsttype=0,float value = 1.0f)
			{
				if ( srcMac >= MAX_MACHINES || dstMac >= MAX_MACHINES) return -1;
				if ( !_pMachine[srcMac] || !_pMachine[dstMac] ) return -1;
				return InsertConnectionNonBlocking(_pMachine[srcMac],_pMachine[dstMac],srctype,dsttype,value);
			}
			/// Changes the destination of a wire. srcMac is its origin, newdstMac is its new destionation. 
			/// outputwiresrc is the output wire index in srcMac. newinputwiredest is the input wire index in newdstMac
			/// returns true if succeeded. 
			bool ChangeWireDestMacBlocking(Machine* srcMac, Machine* newdstMac, int outputwiresrc, int newinputwiredest) {
				CExclusiveLock lock(&semaphore, 2, true);
				return ChangeWireDestMacNonBlocking(srcMac, newdstMac, outputwiresrc, newinputwiredest);
			}
			bool ChangeWireDestMacNonBlocking(Machine* srcMac, Machine* newdstMac, int outputwiresrc, int newinputwiredest);
			/// Changes the source of a wire. newsrcMac is the new source. dstMac is its destination. 
			/// newoutputwiresrc is the output wire index in newsrcMac. inputwiredest is the input wire index in dstMac
			/// returns true if succeeded. 
			bool ChangeWireSourceMacBlocking(Machine* newsrcMac, Machine* dstMac, int newoutputwiresrc, int inputwiredest) {
				CExclusiveLock lock(&semaphore, 2, true);
				return ChangeWireSourceMacNonBlocking(newsrcMac, dstMac, newoutputwiresrc, inputwiredest);
			}
			bool ChangeWireSourceMacNonBlocking(Machine* newsrcMac, Machine* dstMac, int newoutputwiresrc, int inputwiredest);
			/// Verifies that the new connection doesn't conflict with the mixer machine.
			bool ValidateMixerSendCandidate(Machine& mac,bool rewiring=false);
			void RestoreMixerSendsReturns();
			/// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
			int GetFreeBus() const;
			/// Gets the first free slot in the Effects' bus (slots MAX_BUSES  to 2*MAX_BUSES-1)
			int GetFreeFxBus() const;
			/// Returns the Bus index out of a pMachine index. (Legacy code. Used for validation purposes now)
			int FindBusFromIndex(int smac) const;
			/// Returns the first unused pattern in the pPatternData[] Array.
			int GetBlankPatternUnused(int rval = 0) const;
			/// creates a new pattern.
			bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
			/// Adds an empty track at the index indicated. If pattern is -1, it does it for all patterns.
			void AddNewTrack(int pattern, int trackIdx);
			/// clones a machine.
			bool CloneMac(int src,int dst);
			/// clones an instrument.
			bool CloneIns(int src,int dst);
			/// Exchanges the index of two instruments
			void ExchangeInstruments(int one, int two);
			/// deletes all the patterns of this song.
			void DeleteAllPatterns();
			/// deletes (resets) the instrument and deletes (and resets) each sample/layer that it uses.
			void DeleteInstrument(int i);
			/// deletes (resets) the instrument and deletes (and resets) each sample/layer that it uses. (all instruments)
			void DeleteInstruments();
			/// destroy all instruments in this song.
			void DestroyAllInstruments();
			///  loads a file into this song object.
			///\param fullopen  used in context of the winamp/foobar player plugins, where it allows to get the info of the file, without needing to open it completely.
			bool Load(RiffFile* pFile,CProgressDialog& progress,bool fullopen=true);
			/// saves this song to a file.
			bool Save(RiffFile* pFile,CProgressDialog& progress,bool autosave=false);
			/// Used to detect if an especific pattern index is used in the sequence.
			bool IsPatternUsed(int i) const;
			//Used to check the contents of the pattern.
			bool IsPatternEmpty(int i) const;
			///\name wave file previewing
			///\todo shouldn't belong to the song class.
			///\{
		public:
			//todo these ought to be dynamically allocated
			/// Wave preview/wave editor playback.
			InstPreview wavprev;
#if defined WINAMP_PLUGIN
			int filesize;
#endif // WINAMP_PLUGIN
			/// runs the wave previewing.
			void DoPreviews(int amount);
			///\}

			/// Returns the start offset of the requested pattern in memory, and creates one if none exists.
			/// This function now is the same as doing &pPatternData[ps]
			inline unsigned char * _ppattern(int ps){
				if(!ppPatternData[ps]) return CreateNewPattern(ps);
				return ppPatternData[ps];
			}
			/// Returns the start offset of the requested track of pattern ps in the
			/// pPatternData Array and creates one if none exists.
			inline unsigned char * _ptrack(int ps, int track){
				if(!ppPatternData[ps]) return CreateNewPattern(ps)+ (track*EVENT_SIZE);
				return ppPatternData[ps] + (track*EVENT_SIZE);
			}
			/// Returns the start offset of the requested line of the track of pattern ps in
			/// the pPatternData Array and creates one if none exists.
			inline unsigned char * _ptrackline(int ps, int track, int line){
				if(!ppPatternData[ps]) return CreateNewPattern(ps)+ (track*EVENT_SIZE) + (line*MULTIPLY);
				return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
			}
			inline const unsigned char * _ptrackline(int ps, int track, int line) const {
				return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
			}
			/// Allocates the memory fo a new pattern at position ps of the array pPatternData.
			unsigned char * CreateNewPattern(int ps);
			/// removes a pattern from this song.
			void RemovePattern(int ps);

			
			const int SongTracks() const {return SONGTRACKS;}
			void SongTracks(const int value){ SONGTRACKS = value;}

			const int BeatsPerMin() const {return m_BeatsPerMin;}
			void BeatsPerMin(const int value)
			{ 
				if ( value < 32 ) m_BeatsPerMin = 32;
				else if ( value > 999 ) m_BeatsPerMin = 999;
				else m_BeatsPerMin = value;
			}

			const int LinesPerBeat() const {return m_LinesPerBeat;}
			void LinesPerBeat(const int value)
			{
				if ( value < 1 )m_LinesPerBeat = 1;
				else if ( value > 31 ) m_LinesPerBeat = 31;
				else m_LinesPerBeat = value;
			}

			/// The file name this song was loaded from.
			std::string fileName;
			/// The index of the machine which plays in solo.
			int machineSoloed;
			/// Is this song saved to a file?
			bool _saved;
			/// The index of the track which plays in solo.
			int _trackSoloed;
			
			//Semaphore used for song (and machine) manipulation. The semaphore accepts at much two threads to run:
			//Player::Work and ChildView::OnTimer (GUI update).
			//There is a third CSingleLock in InfoDlg (which is not a perfect situation, but it's fast enough so it shouldn't disturb Player::Work).
			//For an exclusive lock (i.e. the rest of the cases) use the CExclusiveLock.
			CSemaphore mutable semaphore;

			cpu_time_clock::duration accumulated_processing_time_, accumulated_routing_time_;

		/// cpu time usage measurement
		void reset_time_measurement() { accumulated_processing_time_ = accumulated_routing_time_ = 0; }

		/// total processing cpu time usage measurement
		cpu_time_clock::duration accumulated_processing_time() const throw() { return accumulated_processing_time_; }
		/// total processing cpu time usage measurement
		void accumulate_processing_time(cpu_time_clock::duration d) throw() {
			if(loggers::warning() && d.count() < 0) {
				std::ostringstream s;
				s << "time went backward by: " << std::chrono::nanoseconds(d).count() * 1e-9 << 's';
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			} else accumulated_processing_time_ += d;
		}

		/// routing cpu time usage measurement
		cpu_time_clock::duration accumulated_routing_time() const throw() { return accumulated_routing_time_; }
		/// routing cpu time usage measurement
		void accumulate_routing_time(cpu_time_clock::duration d) throw() {
			if(loggers::warning() && d.count() < 0) {
				std::ostringstream s;
				s << "time went backward by: " << std::chrono::nanoseconds(d).count() * 1e-9 << 's';
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			} else accumulated_routing_time_ += d;
		}
		};
	}
}