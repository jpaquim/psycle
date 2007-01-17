///\file
///\brief interface file for psycle::host::Song based on Revision 2730
#pragma once
#include "songstructs.h"
#include "instrument.h"
#include "instpreview.h"
#include "machine.h"
#include "constants.h" 
#include "fileio.h"
#include "cstdint.h"
#include <ngrs/sigslot.h>
#include "patterndata.h"
#include "patternsequence.h"

namespace psycle
{
	namespace host
	{

		class PluginFinder;
		class PluginFinderKey;

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines 
		/// and their initial parameters and coordinates, wavetables

		class Song
		{
			public:
				Song();

				virtual ~Song();

				void clear(); // clears all song data

				PatternSequence* patternSequence();
				const PatternSequence& patternSequence() const;

				// loads a song
				bool load(const std::string& fileName);
				bool save(const std::string& fileName);

				//authorship
				void setName(const std::string & name);
				const std::string & name() const;
				void setAuthor(const std::string & author);
				const std::string & author() const;
				void setComment(const std::string & comment);
				const std::string & comment() const;

				// The number of tracks in each pattern of this song.
				unsigned int tracks() const;
				void setTracks( unsigned int trackCount) ;

				// start bpm for song, can be overwritten through global bpm event
				void setBpm(float bpm);
				float bpm() const;

				// signals
				sigslot::signal2<const std::string &, const std::string &> report;
				sigslot::signal3<const std::uint32_t& , const std::uint32_t& , const std::string& > progress;

			private:

				PatternSequence patternSequence_;

				unsigned int tracks_;
				float bpm_;

				//authorship
				std::string name_;
				std::string author_;
				std::string comment_;


			public:

			///\name machines
			///\{
				public:

					// creates a new machine in this song
					// requirements : PluginFinder 
					//				  PluginFinderKey
					// return values:
					//		succes  : machine ptr
					//		failure : 0
					// future  : iterator of machine (stl)container or at failure end()

					Machine* createMachine( const PluginFinder & finder, const PluginFinderKey & key, int x = 0, int y = 0 );

					/// creates a new machine in this song. .. deprecated
					Machine & CreateMachine(int type, int x, int y, std::string const & plugin_name = "dummy", int pluginIndex = 0) throw(std::exception)
					{
						int const array_index(GetFreeMachine());
						if(array_index < 0) throw std::runtime_error("sorry, psycle doesn't dynamically allocate memory.");
						if(!CreateMachine(type, x, y, plugin_name, array_index, pluginIndex))
							throw std::runtime_error("something bad happened while i was trying to create a machine, but i forgot what it was.");
						return *_pMachine[array_index];
					}

					/// creates a new machine in this song.
					bool CreateMachine(int, int x, int y, std::string const & plugin_name, int, int pluginIndex);

					/// Gets the first free slot in the pMachine[] Array
					///\todo it's low-level.. should be private.
					/// we have higer-level CreateMachine and CloneMachine functions already
					//PSYCLE__DEPRECATED("low-level")
					int GetFreeMachine();

					/// destroy a machine of this song.
					void DestroyMachine(Machine & machine, bool write_locked = false) { DestroyMachine(machine.id()); /* stupid circonvolution */ }

					/// destroy a machine of this song.
					void DestroyMachine(int mac, bool write_locked = false);

					/// destroys all the machines of this song.
					void DestroyAllMachines(bool write_locked = false);
					
			///\name machine connections
			///\{
				public:
					/// creates a new connection between two machines.
					///\todo kinda useless since machines can connect themselves with their ConnectTo function
					bool InsertConnection(int src, int dst, float volume = 1.0f);

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
				public:
					/// resets the instrument and delete each sample/layer that it uses.
					void /*Reset*/DeleteInstrument(int id) { Invalided=true; _pInstrument[id]->Delete(); Invalided=false; }
					/// resets the instrument and delete each sample/layer that it uses. (all instruments)
					void /*Reset*/DeleteInstruments() { Invalided=true; for(int id(0) ; id < MAX_INSTRUMENTS ; ++id) _pInstrument[id]->Delete(); Invalided=false; }
					/// delete all instruments in this song.
					void /*Delete*/DestroyAllInstruments() { for(int id(0) ; id < MAX_INSTRUMENTS ; ++id) { delete _pInstrument[id]; _pInstrument[id] = 0; } }
					// Removes the sample/layer of the instrument
					void DeleteLayer(int id) { _pInstrument[id]->DeleteLayer(); }
			///\}

			///\name wavetable
			///\{
				public:
					/// ???
					bool WavAlloc(int, const char * str);
					/// ???
					bool WavAlloc(int, bool bStereo, long int iSamplesPerChan, const char * sName);
					/// ???
					bool IffAlloc(int, const char * str);
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
					/// ngrs::Number of samples processed since all cpu cost counters were reset.
					/// We accumulate this sample count along with cpu costs until we compute the percentages, for example, every second.
					unsigned int _sampCount;
				public:

				///\name machines
				///\{
					/// Sort of semaphore to not allow doing something with machines when they are changing (deleting,creating, etc..)
					/// \todo change it by a real semaphore?
					bool _machineLock;
					/// the array of machines.
					
					///\todo more lightweight with a std::vector<Machine*>
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
					
					Instrument * _pInstrument[MAX_INSTRUMENTS];
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
					
					bool _trackMuted[MAX_TRACKS];
					/// The number of tracks Armed (enabled for record)
					/// \todo should this be here? (used exclusively in childview)
					int _trackArmedCount;
					/// Wether each of the tracks is armed (selected for recording data in)
					
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

					char currentOctave;

					// The volume of the preview wave in the wave load dialog->
					/// \todo This is a GUI thing... should not be here.
					float preview_vol; 
				///\}


			///\name deprecated by multiseq for appregio we need an workaround
			///\{
			public:
				/// the initial ticks per beat (TPB) when the song is started playing.
				/// This can be changed in patterns using a command, but this value will not be affected.
				int m_LinesPerBeat;
						const int LinesPerBeat(){return m_LinesPerBeat;};
						void LinesPerBeat(const int value)
						{
							if ( value < 1 )m_LinesPerBeat = 1;
							else if ( value > 31 ) m_LinesPerBeat = 31;
							else m_LinesPerBeat = value;
						};

		};
	}
}
