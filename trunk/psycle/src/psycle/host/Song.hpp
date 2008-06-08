// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::Song
#pragma once
#include "Constants.hpp"
#include "FileIO.hpp"
#include "SongStructs.hpp"
#include "Instrument.hpp"

#if !defined WINAMP_PLUGIN
	#include "InstPreview.hpp"
#endif // WINAMP_PLUGIN

class CCriticalSection;

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
			/// destructor.
			virtual ~Song() throw();
			/// the name of the song.
			std::string name;
			/// the author of the song.
			std::string author;
			/// the comments on the song
			std::string comments;
			///\todo: this variable was used in Player, but it didn't store a good value. Either try to fix it or remove.
			std::uint64_t cpuIdle;
			unsigned _sampCount;
			bool Invalided;
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
			///\}
			/// The index of the selected MIDI program for note entering
			/// \todo This is a gui thing... should not be here.
			int midiSelected;
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
			///\name machines
			///\{
			/// Sort of semaphore to not allow doing something with machines when they are changing (deleting,creating, etc..)
			/// \todo change it by a real semaphore?
			bool _machineLock;
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
			int WavAlloc(int iInstr,bool bStereo,long iSamplesPerChan,const char * sName);
			/// ???
			int IffAlloc(int instrument,const char * str);
			///\}
			/// Initializes the song to an empty one.
			void New();
			/// Resets some variables to their default values (used inside New(); )
			void Reset();
			/// Gets the first free slot in the pMachine[] Array
			int GetFreeMachine();
			/// creates a new machine in this song.
			bool CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int songIdx,int shellIdx=0);
			/// Creates a new machine, replacing an existing one.
			bool ReplaceMachine(Machine* origmac, MachineType type, int x, int y, char const* psPluginDll, int songIdx,int shellIdx=0);
			/// exchanges the position of two machines.
			bool ExchangeMachines(int one, int two);
			/// Creates a new machine cloned from an existing one.
			Machine* CloneMachine(Machine* origmac);
			/// destroy a machine of this song.
			void DestroyMachine(int mac, bool write_locked = false);
			/// destroys all the machines of this song.
			void DestroyAllMachines(bool write_locked = false);
			//the index off the last instrument containing data
			int GetLastInstrumentUsed();
			//the index of the last pattern containing data
			int GetLastPatternUsed();
			/// the number of pattern used in this song.
			int GetNumPatternsUsed();
			/// creates a new connection between two machines. returns index in the dest machine, or -1 if error.
			int InsertConnection(Machine* srcMac,Machine* dstMac,int srctype=0, int dsttype=0,float value = 1.0f);
			int InsertConnection(int srcMac,int dstMac,int srctype=0, int dsttype=0,float value = 1.0f)
			{
				if ( srcMac >= MAX_MACHINES || dstMac >= MAX_MACHINES) return -1;
				if ( !_pMachine[srcMac] || !_pMachine[dstMac] ) return -1;
				return InsertConnection(_pMachine[srcMac],_pMachine[dstMac],srctype,dsttype,value);
			}
			/// Changes the destination of a wire connection. wireindex= index of the wire in wiresource to change.
			/// returns index, or -1 if error. 
			bool ChangeWireDestMac(Machine* srcMac, Machine* dstMac, int wiresrc, int wiredest);
			/// Changes the destination of a wire connection. wireindex= index of the wire in wiredest to change.
			/// returns index, or -1 if error. 
			bool ChangeWireSourceMac(Machine* srcMac, Machine* dstMac, int wiresrc, int wiredest);
			/// Verifies that the new connection doesn't conflict with the mixer machine.
			bool Song::ValidateMixerSendCandidate(Machine* mac,bool rewiring=false);
			void RestoreMixerSendFlags();
			/// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
			int GetFreeBus();
			/// Gets the first free slot in the Effects' bus (slots MAX_BUSES  to 2*MAX_BUSES-1)
			int GetFreeFxBus();
			/// Returns the Bus index out of a pMachine index. (Legacy code. Used for validation purposes now)
			int FindBusFromIndex(int smac);
			/// Returns the first unused pattern in the pPatternData[] Array.
			int GetBlankPatternUnused(int rval = 0);
			/// creates a new pattern.
			bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
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
			// Removes the sample/layer of the instrument "instrument"
			void DeleteLayer(int instrument);
			/// destroy all instruments in this song.
			/// \todo ZapObject ??? What does this function really do?
			void DestroyAllInstruments();
			///  loads a file into this song object.
			///\param fullopen  used in context of the winamp/foobar player plugins, where it allows to get the info of the file, without needing to open it completely.
			bool Load(RiffFile* pFile, bool fullopen=true);
			/// saves this song to a file.
			bool Save(RiffFile* pFile,bool autosave=false);
			/// Used to detect if an especific pattern index contains any data.
			bool IsPatternUsed(int i);
			///\name wave file previewing
			///\todo shouldn't belong to the song class.
			///\{
		public:
			//todo these ought to be dynamically allocated
			/// Wave preview.
#if !defined WINAMP_PLUGIN
			InstPreview wavprev;
			/// Wave editor playback.
			InstPreview waved;
#else
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
			/// Allocates the memory fo a new pattern at position ps of the array pPatternData.
			unsigned char * CreateNewPattern(int ps);
			/// removes a pattern from this song.
			void RemovePattern(int ps);
			
			const int SongTracks(){return SONGTRACKS;}
			void SongTracks(const int value){ SONGTRACKS = value;}

			const int BeatsPerMin(){return m_BeatsPerMin;}
			void BeatsPerMin(const int value)
			{ 
				if ( value < 32 ) m_BeatsPerMin = 32;
				else if ( value > 999 ) m_BeatsPerMin = 999;
				else m_BeatsPerMin = value;
			}

			const int LinesPerBeat(){return m_LinesPerBeat;}
			void LinesPerBeat(const int value)
			{
				if ( value < 1 )m_LinesPerBeat = 1;
				else if ( value > 31 ) m_LinesPerBeat = 31;
				else m_LinesPerBeat = value;
			}

			const bool IsInvalided(){return Invalided;}
			void IsInvalided(const bool value){Invalided = value;}
			/// The file name this song was loaded from.
			std::string fileName;
			/// The index of the machine which plays in solo.
			int machineSoloed;
			/// ???
			CPoint viewSize;
			/// Is this song saved to a file?
			bool _saved;
			/// The index of the track which plays in solo.
			int _trackSoloed;

			CCriticalSection mutable door;
		};
	}
}