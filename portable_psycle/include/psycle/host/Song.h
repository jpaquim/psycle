#pragma once
#include "Constants.h"
#include "FileIO.h"
#include "SongStructs.h"
#include "Instrument.h"
class CCriticalSection;
///\file
///\brief interface file for psycle::host::Song
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
			#if defined _WINAMP_PLUGIN_
				/// The file name this song was loaded from.
				char fileName[_MAX_PATH];
				/// The size of the file this song was loaded from.
				/// Why is it stored?
				long filesize;
			#else
				/// The index of the machine which plays in solo.
				int machineSoloed;
				/// The file name this song was loaded from.
				CString fileName;
				/// ???
				CPoint viewSize;
			#endif
			/// wether this song has been saved to a file
			bool _saved;
			/// The index of the track which plays in solo.
			int _trackSoloed;
			#if !defined _WINAMP_PLUGIN_
				/// ...
				CCriticalSection door;
			#endif
			/// constructor.
			Song();
			/// destructor.
			virtual ~Song() throw();
			/// the name of the song.
			char Name[64];
			/// the author of the song.
			char Author[64];
			/// the comments on the song
			char Comment[256];
			#if !defined _WINAMP_PLUGIN_
				bool Tweaker;
				unsigned cpuIdle;
				unsigned _sampCount;
				bool Invalided;
			#endif
			/// the initial beats per minute (BPM) when the song is started playing.
			/// This can be changed in patterns using a command, but this value will not be affected.
			int BeatsPerMin;
			/// the initial ticks per beat (TPB) when the song is started playing.
			/// This can be changed in patterns using a command, but this value will not be affected.
			int _ticksPerBeat;
			/// samples per tick.
			/// This is computed from the BeatsPerMin, _ticksPerBeat, and SamplesPerSeconds()
			int SamplesPerTick;
			/// ???
			int LineCounter;
			/// ???
			bool LineChanged;
			/// This is a GUI thing... should not be here.
			char currentOctave;
			// The volume of the preview wave in the wave load dialog.
			/// This is a GUI thing... should not be here.
			float preview_vol; 
			/// Pattern data
			unsigned char * ppPatternData[MAX_PATTERNS];
			/// ???
			int playLength;
			/// ???
			unsigned char playOrder[MAX_SONG_POSITIONS];
			#if !defined _WINAMP_PLUGIN_
				/// ???
				bool playOrderSel[MAX_SONG_POSITIONS];
			#endif
			/// ??? number of lines in each pattern?
			int patternLines[MAX_PATTERNS];
			/// ???
			char patternName[MAX_PATTERNS][32];
			/// The number of tracks inr each pattern of this song.
			int SONGTRACKS;
			/// ???
			int midiSelected;
			/// ???
			int auxcolSelected;
			/// ???
			int _trackArmedCount;
			///\name instrument
			///\{
			///
			int instSelected;
			///
			Instrument * _pInstrument[MAX_INSTRUMENTS];
			///\}
			/// Wether each of the tracks is muted.
			bool _trackMuted[MAX_TRACKS];
			/// ???
			bool _trackArmed[MAX_TRACKS];
			/// The index of the selected waveform in the wavetable.
			/// This is a gui thing... should not be here.
			int waveSelected;
			///\name machines
			///\{
			/// ???
			bool _machineLock;
			/// the array of machines.
			Machine* _pMachine[MAX_MACHINES];
			/// ???
			int seqBus;
			#if !defined _WINAMP_PLUGIN_
				///\name wavetable
				///\{
				/// ???
				int WavAlloc(int iInstr,int iLayer,const char * str);
				/// ???
				int WavAlloc(int iInstr,int iLayer,bool bStereo,long iSamplesPerChan,const char * sName);
				/// ???
				int IffAlloc(int instrument,int layer,const char * str);
				///\}
			#endif
			/// ???
			void New();
			/// resets this song to an empty one.
			void Reset();
			/// the first free slot for a new machine.
			int GetFreeMachine();
			/// creates a new machine in this song.
			bool CreateMachine(MachineType type, int x, int y, char* psPluginDll, int index);
			/// destroy a machine of this song.
			void DestroyMachine(int mac);
			/// destroys all the machines of this song.
			void DestroyAllMachines();
			/// the number of pattern used in this song.
			int GetNumPatternsUsed();
			#if !defined _WINAMP_PLUGIN_
				/// creates a new connection between two machines.
				bool InsertConnection(int src,int dst,float value = 1.0f);
				/// ???
				int GetFreeBus();
				/// ???
				int GetFreeFxBus();
				/// ???
				int FindBusFromIndex(int smac);
				/// ???
				int GetBlankPatternUnused(int rval = 0);
				/// creates a new pattern.
				bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
				/// clones a machine.
				bool CloneMac(int src,int dst);
				/// clones an instrument.
				bool CloneIns(int src,int dst);
			#endif
			/// deletes all the patterns of this song.
			void DeleteAllPatterns();
			/// this actually just resets to a blank instrument.
			void DeleteInstrument(int i);
			/// this actually just resets to a blank instrument.
			void DeleteInstruments();
			// seeks and destroys allocated instruments.
			void DeleteLayer(int instrument, int layer);
			/// destroy all instruments in this song.
			void DestroyAllInstruments();
			/// sets a new BPM, TPB, and sample per seconds.
			void SetBPM(int bpm, int tpb, int srate);
			///  loads a file into this song object.
			///\param fullopen  used in context of the winamp/foobar player plugins, where it allows to get the info of the file, without needing to open it completely.
			bool Load(RiffFile* pFile, bool fullopen=true);
			#if !defined _WINAMP_PLUGIN_
				/// saves this song to a file.
				bool Save(RiffFile* pFile,bool autosave=false);
				/// ???
				bool IsPatternUsed(int i);
				///\name previews waving
				///\{
				/// ???
				void PW_Work(float *psamplesL, float *pSamplesR, int numSamples);
				/// ???
				void PW_Play();
				/// ???
				int PW_Phase;
				/// ???
				int PW_Stage;
				/// ???
				int PW_Length;
				///\}
			#endif
			/// either returns a requested pattern or creates one
			/// if none exists and returns that (as an unsigned char pointer?)
			inline unsigned char * _ppattern(int ps);
			/// ???
			inline unsigned char * _ptrack(int ps, int track);
			/// ???
			inline unsigned char * _ptrackline(int ps, int track, int line);
			/// ???
			unsigned char * CreateNewPattern(int ps);
			/// removes a pattern from this song.
			void RemovePattern(int ps);
		};

		inline unsigned char * Song::_ppattern(int ps)
		{
			if(!ppPatternData[ps]) return CreateNewPattern(ps);
			return ppPatternData[ps];
		}

		inline unsigned char * Song::_ptrack(int ps, int track)
		{
			if(!ppPatternData[ps]) return CreateNewPattern(ps);
			return ppPatternData[ps] + (track*EVENT_SIZE);
		}	

		inline unsigned char * Song::_ptrackline(int ps, int track, int line)
		{
			if(!ppPatternData[ps]) return CreateNewPattern(ps);
			return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
		}
	}
}
