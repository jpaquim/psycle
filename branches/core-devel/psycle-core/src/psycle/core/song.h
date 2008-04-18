// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psy::core::Song
/// based on psycle mfc revision 2730
#pragma once

#include "cstdint.h"
#include "patternsequence.h"
#include "songserializer.h"
#include "machine.h"
#include "instrument.h"
//#include "xminstrument.h"

namespace psy
{
	namespace core
	{
		/// forward declarations.
		class PluginFinderKey;
		class MachineFactory;

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines, wavetables
		/// and their initial parameters
		class CoreSong
		{
		public:
			CoreSong(); 
			virtual ~CoreSong();
			/// clears all song data
			virtual void clear();

		public:
			///\name serialization
			///\{
			bool load(std::string filename);
			bool save(std::string filename, int version=4);
		private:
			static SongSerializer serializer;
			///\}

			// signals
#if defined PSYCLE__CORE__SIGNALS
			boost::signal<void (const std::string &, const std::string &)> report;
			boost::signal<void (const std::uint32_t& , const std::uint32_t& , const std::string&)> progress;
#endif

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
			float bpm() const { return bpm_; }
			/// initial bpm for the song
			void setBpm(float bpm);
		private:
			float bpm_;
			///\}
			///\name the initial ticks per beat (TPB) when the song starts to play.
			/// With multisequence, ticksSpeed helps on syncronization and timing.
			/// Concretely, it helps to send the legacy "SequencerTick()" events to native plugins,
			/// helps in knowing for how much a command is going to be tick'ed ( tws, as well as 
			/// retrigger code need to know how much they last ) as well as helping Sampulse
			/// to get ticks according to legacy speed command of Modules.
			/// isTicks is used to identify if the value in ticksPerBeat_ means ticks, or speed.
			///\{
		public:
			unsigned int ticksSpeed() const { return ticks_; }
			bool isTicks() const { return isTicks_; }
			void setTicksSpeed(const unsigned int value, const bool isticks=true);
		private:
			unsigned int ticks_;
			bool isTicks_;
			///\}

			///\name track count
			/// legacy. maps to the value from pattternSequence.
			///\{
		public:
			unsigned int tracks() const { return patternSequence_.numTracks(); }
			void setTracks( unsigned int tracks) { patternSequence_.setNumTracks(tracks); }
			///\}


			///\name pattern sequence
			///\todo: Think about PatternPool. Should be owned by PatternSequence, or by CoreSong?
			///\{
		public:
			/// pattern sequence
			PatternSequence const & patternSequence() const { return patternSequence_; }
			/// pattern sequence
			PatternSequence * patternSequence() { return &patternSequence_; }
		private:
			PatternSequence patternSequence_;
			///\}

			///\name machines
			///\{
		public:
			/// access to the machines of the song
			Machine * machine(Machine::id_type id) { return machine_[id]; }
			/// access to the machines of the song
			Machine const * const machine(Machine::id_type id) const { return machine_[id]; }
		private:
			void machine(Machine::id_type id, Machine * machine)
			{
				assert(id >= 0 && id < MAX_MACHINES);
				machine_[id] = machine;
				machine->id(id);
			} 

			Machine * machine_[MAX_MACHINES];
			///\}

		public:

			///\name instruments
			///\{
			///\todo doc
			///\todo hardcoded limits and wastes
			///\todo XMInstrument
			Instrument * _pInstrument[MAX_INSTRUMENTS];
			///\}

			///\name cpu cost measurement
			///\{
#if 0 ///\todo Rethink about cpu measurements, and reenable/recode what's needed
		public:
			void             inline cpu_idle(cpu::cycles_type const & value)       throw() { cpu_idle_ = value; }
			cpu::cycles_type inline cpu_idle(                              ) const throw() { return cpu_idle_; }
		private:
			cpu::cycles_type        cpu_idle_;
#endif
			///\}

			///\name file-related stuff
			///\{
			/// The file name this song was loaded from.
			std::string fileName;
			/// Is this song saved to a file?
			bool _saved;
			///\}

			///\name actions with machines
			///\{
		public:
			/// add a new machine. The index comes from pmac.
			//  if machine id is -1, a free index is taken.
			virtual bool AddMachine(Machine* pmac);
			/// (add or) replace the machine in index idx.
			// idx cannot be -1.
			virtual bool ReplaceMachine(Machine* pmac, Machine::id_type idx);
			/// destroy a machine of this song.
			virtual void DeleteMachine(Machine* machine, bool write_locked = false);
			/// destroy a machine of this song.
			virtual void DeleteMachineDeprecated(Machine::id_type mac, bool write_locked = false)
			{
				if (machine(mac))
					DeleteMachine(machine(mac),write_locked);
			}
			/// destroys all the machines of this song.
			virtual void DeleteAllMachines(bool write_locked = false);

		protected:
			/// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
			Machine::id_type GetFreeBus();
			/// Gets the first free slot in the Effects' bus (slots MAX_BUSES  to 2*MAX_BUSES-1)
			Machine::id_type GetFreeFxBus();
			/// Returns the Bus index out of a machine id.
			Machine::id_type FindBusFromIndex(Machine::id_type);
			///\}

			///\name machine connections
			///\{
		public:
			/// creates a new connection between two machines.
			/// This funcion is to be used over the Machine's ConnectTo(). This one verifies the validity of the connections, and uses Machine's function
			Wire::id_type InsertConnection(Machine &srcMac, Machine &dstMac, InPort::id_type srctype=0, OutPort::id_type dsttype=0, float volume = 1.0f);

			/// Changes the destination of a wire connection.
			///\param wiresource source mac index
			///\param wiredest new dest mac index
			///\param wireindex index of the wire in wiresource to change
			bool ChangeWireDestMac(Machine& srcMac, Machine &newDstMac, OutPort::id_type srctype, Wire::id_type wiretochange, InPort::id_type dsttype);
			/// Changes the destination of a wire connection.
			///\param wiredest dest mac index
			///\param wiresource new source mac index
			///\param wireindex index of the wire in wiredest to change
			bool ChangeWireSourceMac(Machine& newSrcMac, Machine &dstMac, InPort::id_type dsttype, Wire::id_type wiretochange, OutPort::id_type srctype);
		protected:
			bool ValidateMixerSendCandidate(Machine& mac,bool rewiring=false);

			///\}

			///\name actions with instruments
			///\{
		public:
			///\todo: The loading code should not be inside the song class, only the assignation of the loaded one
			/// ???
			bool WavAlloc(Instrument::id_type, const char * str);
			/// ???
			bool WavAlloc(Instrument::id_type, bool bStereo, long int iSamplesPerChan, const char * sName);
			/// ???
			bool IffAlloc(Instrument::id_type, const char * str);
			///\}
			/// clones an instrument.
			bool CloneIns(Instrument::id_type src, Instrument::id_type dst);
			/// resets the instrument and delete each sample/layer that it uses.
			void /*Reset*/DeleteInstrument(Instrument::id_type id);
			/// resets the instrument and delete each sample/layer that it uses. (all instruments)
			void /*Reset*/DeleteInstruments();
		protected:
			/// deletes all instruments in this song.
			void /*Delete*/FreeInstrumentsMemory();
			/// removes the sample/layer of the instrument
			void DeleteLayer(Instrument::id_type id);
			///\}







			///\name IsInvalid
			///\todo This should be changed by a semaphore.
			///\{
		public:
			/// Sort of semaphore to not allow doing something with machines when they are changing (deleting,creating, etc..)
			/// \todo change it by a real semaphore?
			bool _machineLock;
			///\name IsInvalid
			///\todo doc ... what's that?
			bool IsInvalided(){return Invalided;};
			///\name IsInvalid
			///\todo doc ... what's that?
			void IsInvalided(const bool value) { Invalided = value; }
		private:
			bool Invalided;
			///\}

		public:
			void patternTweakSlide(int machine, int command, int value, int patternPosition, int track, int line);

		};

		/// UI stuff moved here
		class UISong : public CoreSong
		{
		protected:
			UISong();
		public:
			virtual ~UISong(){};
		};

		/// the actual song class used by qpsycle. it's simply the UISong class
		/// note that a simple typedef won't work due to the song class being forward-declared, as a class and not a typedef.
		//
		///\todo: For derived UI machines, the data is hold by Song in a class VisualMachine.
		// This means that a Song maintans a separate array of VisualMachines for the Non-visual counterparts and gives access
		// to these with ui-specific methods.
		// To help working with them, a VisualMachine can have a reference to the related non-visual machine, so that the UI parts
		// can access the real machine via the VisualMachine class.
		class Song : public UISong
		{
		protected:
			Song();
		public:
			virtual ~Song(){};
			virtual void clear();
			virtual void DeleteMachine(Machine* mac, bool write_locked = false);
		private:
			void clearMyData();

		public:
			///\name various ui-related stuff
			///\{
			/// Current selected machine number in the GUI
			Machine::id_type seqBus;
			
			/// Current selected instrument number in the GUI
		private:
			Instrument::id_type _instSelected;
		public:
			Instrument::id_type instSelected() const { return _instSelected; }
			void instSelected(Instrument::id_type id) {
				assert(id >= 0);
				assert(id < MAX_INSTRUMENTS);
				_instSelected = id;
			}
			/// The index of the selected MIDI program for note entering
			int midiSelected;
			/// The index for the auxcolumn selected (would be waveselected, midiselected, or an index to a machine parameter)
			int auxcolSelected;

			/// The index of the machine which plays in solo.
			///\todo ok it's saved in psycle "song" files, but that belongs to the player.
			Machine::id_type machineSoloed;
			/// The index of the track which plays in solo.
			///\todo ok it's saved in psycle "song" files, but that belongs to the player.
			int _trackSoloed;

			///\}

		};
	}
}
