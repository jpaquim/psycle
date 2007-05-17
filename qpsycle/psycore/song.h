///\file
///\brief interface file for psy::core::Song
/// based on xpsycle revision 2730
#pragma once
#include "patternsequence.h"
#include "songstructs.h"
#include "instrument.h"
#include "instpreview.h"
#include "machine.h"
#include "constants.h" 
#include "fileio.h"
#include "cstdint.h"
#include "patterndata.h"
#if defined PSYCLE__CORE__SIGNALS
	#include <boost/signals.hpp>
#endif
namespace psy
{
	namespace core
	{
		class PluginFinder;
		class PluginFinderKey;

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines 
		///and their initial parameters and coordinates, wavetables
		class Song 
		{
			public:
				Song(MachineCallbacks*);
				virtual ~Song();

				/// clears all song data
				void clear();

			///\name pattern sequence
			///\{
				public:
					/// pattern sequence
					PatternSequence const & patternSequence() const { return patternSequence_; }
					/// pattern sequence
					PatternSequence * patternSequence() { return &patternSequence_; }
				private:
					PatternSequence patternSequence_;
			///\}

			///\name (de)serialisation
			///\{
				public:
					/// loads the song
					bool load(const std::string & fileName);
					/// saves the song
					bool save(const std::string & fileName);
			///\}

			///\name name of the song
			///\{
				public:
					/// name of the song
					void setName(const std::string & name);
					/// name of the song
					std::string const & name() const { return name_; }
				private:
					std::string name_;
			///\}
				
			///\name author of the song
				public:
					/// author of the song
					void setAuthor(const std::string & author);
					/// author of the song
					std::string const & author() const { return author_; }
				private:
					std::string author_;
			///\}
							
			///\name comment on the song
			///\{
				public:
					/// comment on the song
					std::string const & comment() const { return comment_; }
					/// comment on the song
					void setComment(const std::string & comment);
				private:
					std::string comment_;
			///\}
			
			///\name bpm
			///\{
				public:
					/// initial bpm for the song
					/// can be overwritten through global bpm event
					float bpm() const { return bpm_; }
					/// initial bpm for the song
					/// can be overwritten through global bpm event
					void setBpm(float bpm);
				private:
					float bpm_;
			///\}

			///\name track count
			///\{
				public:
					/// number of tracks in each pattern of this song.
					unsigned int tracks() const { return tracks_; }
					/// number of tracks in each pattern of this song.
					void setTracks( unsigned int tracks);
				private:
					unsigned int tracks_;
			///\}
			
			public:
				// signals
				#if defined PSYCLE__CORE__SIGNALS
					boost::signal<void (const std::string &, const std::string &)> report;
					boost::signal<void (const std::uint32_t& , const std::uint32_t& , const std::string&)> progress;
				#endif

			///\name machines
			///\{
				public:

					/// creates a new machine in this song
					/// requirements : PluginFinder, PluginFinderKey
					///\return a pointer to a new machine upon success, 0 otherwise
					// future : iterator of machine (stl)container or at failure end()
					Machine* createMachine(const PluginFinder & finder, const PluginFinderKey & key, int x = 0, int y = 0 );

					/// creates a new machine in this song. .. deprecated
					Machine & CreateMachine(Machine::type_type type, int x, int y, std::string const & plugin_name = "dummy" ) throw(std::exception);

					/// creates a new machine in this song.
					bool CreateMachine(Machine::type_type, int x, int y, std::string const & plugin_name, Machine::id_type);

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

			///\name IsInvalid
			///\todo doc ... what's that?
			///\{
				public:
					///\name IsInvalid
					///\todo doc ... what's that?
					bool IsInvalided(){return Invalided;};
					///\name IsInvalid
					///\todo doc ... what's that?
					void IsInvalided(const bool value) { Invalided = value; }
				private:
					bool Invalided;
			///\}

			///\name instruments
			///\{
				public:
					/// clones an instrument.
					bool CloneIns(Instrument::id_type src, Instrument::id_type dst);
					/// resets the instrument and delete each sample/layer that it uses.
					void /*Reset*/DeleteInstrument(Instrument::id_type id);
					/// resets the instrument and delete each sample/layer that it uses. (all instruments)
					void /*Reset*/DeleteInstruments();
					/// deletes all instruments in this song.
					void /*Delete*/DestroyAllInstruments();
					/// removes the sample/layer of the instrument
					void DeleteLayer(Instrument::id_type id);
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

			///\name cpu cost measurement
			///\{
				public:
/*					void             inline cpu_idle(cpu::cycles_type const & value)       throw() { cpu_idle_ = value; }
					cpu::cycles_type inline cpu_idle(                              ) const throw() { return cpu_idle_; }
				private:
					cpu::cycles_type        cpu_idle_;*/ // How do make cpu on linux ?

				public: ///\todo public->private
					/// ngrs::Number of samples processed since all cpu cost counters were reset.
					/// We accumulate this sample count along with cpu costs until we compute the percentages, for example, every second.
					unsigned int _sampCount;
			///\}

			///\todo below are unencapsulated data members
			//PSYCLE__PRIVATE: preprocessor macro stuff sux more -- stefan
			public:

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

					// The volume of the preview wave in the wave load dialog->
					/// \todo This is a GUI thing... should not be here.
					float preview_vol; 
				///\}


			///\name deprecated by multiseq for appregio we need an workaround
			///\{
				public:
					/// the initial ticks per beat (TPB) when the song is started playing.
					/// This can be changed in patterns using a command, but this value will not be affected.
					unsigned int linesPerBeat() const { return linesPerBeat_; }
					void setLinesPerBeat(const unsigned int value);
				private:
					unsigned int linesPerBeat_;

				public:
					void patternTweakSlide(int machine, int command, int value, int patternPosition, int track, int line);

				private:
					MachineCallbacks *machinecallbacks;
			///\}
		};
	}
}
