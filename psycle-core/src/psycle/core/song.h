// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psycle::core::Song

#ifndef PSYCLE__CORE__SONG__INCLUDED
#define PSYCLE__CORE__SONG__INCLUDED
#pragma once

#include "patternsequence.h"
#include "songserializer.h"
#include "machine.h"
#include "instrument.h"
#include "xminstrument.h"
#include <universalis/stdlib/cstdint.hpp>
#include <universalis/stdlib/mutex.hpp>

namespace psycle { namespace core {

using namespace universalis::stdlib;

// forward declarations.
class PluginFinderKey;
class MachineFactory;

/// songs hold everything comprising a "tracker module",
/// this include patterns, pattern sequence, machines, wavetables
/// and their initial parameters
class PSYCLE__CORE__DECL CoreSong {
	public:
		CoreSong();
		virtual ~CoreSong();

		/// clears all song data
		virtual void clear();

	///\name serialization
	///\{
		public:
			bool load(std::string const & filename);
			bool save(std::string const & filename, int version = 4);
		private:
			static SongSerializer serializer;
	///\}
	///\name file-related stuff
	///\{
		public:
			/// The file name this song was loaded from.
			std::string fileName;
	///\}

	///\name IsReady
	/// Identifies if the song is operational or in a loading/saving state. It serves as a non-locking synchronization
	///\{
		public:
			///\name IsReady
			/// Checks the ready state of song (if it can be played). Always call it after acquiring the lock.
			bool IsReady(){ return ready; };
			///\name SetReady
			/// Sets song to ready state (it can be played).
			void SetReady(const bool value) { scoped_lock lock(mutex_); ready = value; }
		private:
			bool ready;
	///\}


	#if defined PSYCLE__CORE__SIGNALS
		///\name signals
		///\{
			boost::signal<void (std::string const &, std::string const &)> report;
			boost::signal<void (std::uint32_t const &, std::uint32_t const &, std::string const &)> progress;
		///\}
	#endif

	///\name name of the song
	///\{
		public:
			/// the name of the song
			std::string const & name() const { return name_; }
			/// sets the name of the song
			void setName(std::string const & name) { name_ = name; }
		private:
			std::string name_;
	///\}

	///\name author of the song
	///\{
		public:
			/// the author of the song
			std::string const & author() const { return author_; }
			/// sets the author of the song
			void setAuthor(std::string const & author) { author_ = author; }
		private:
			std::string author_;
	///\}

	///\name comment on the song
	///\{
		public:
			/// the comment on the song
			std::string const & comment() const { return comment_; }
			/// sets the comment on the song
			void setComment(std::string const & comment) { comment_ = comment; }
		private:
			std::string comment_;
	///\}

	///\name bpm
	///\{
		public:
			/// initial bpm for the song
			float bpm() const { return bpm_; }
			/// initial bpm for the song
			void setBpm(float bpm) { if(bpm > 0 && bpm < 1000) bpm_ = bpm; }
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
			void setTicksSpeed(unsigned int const value, bool const isticks = true) {
				if(value < 1) ticks_ = 1;
				else if(value > 31) ticks_ = 31;
				else ticks_ = value;
				isTicks_ = isticks;
			}
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
	///\{
		public:
			/// pattern sequence
			Sequence const& patternSequence() const throw() { return patternSequence_; }
			/// pattern sequence
			Sequence& patternSequence() throw() { return patternSequence_; }
		private:
			Sequence patternSequence_;
	///\}

	///\name machines
	///\{
		public:
			/// access to the machines of the song
			Machine * machine(Machine::id_type id) { return machine_[id]; }
			/// access to the machines of the song
			Machine const * const machine(Machine::id_type id) const { return machine_[id]; }
		private:
			void machine(Machine::id_type id, Machine * machine) {
				assert(id >= 0 && id < MAX_MACHINES);
				machine_[id] = machine;
				machine->id(id);
			}
			Machine * machine_[MAX_MACHINES];
	///\}

	///\name instruments
	///\{
		public:
			XMInstrument & rInstrument(const int index){return m_Instruments[index];}
			XMInstrument::WaveData & SampleData(const int index){return m_rWaveLayer[index];}
			///\todo doc
			///\todo hardcoded limits and wastes
			Instrument * _pInstrument[MAX_INSTRUMENTS];
		private:
			XMInstrument m_Instruments[MAX_INSTRUMENTS];
			XMInstrument::WaveData m_rWaveLayer[MAX_INSTRUMENTS];

	///\}

	#if 0 ///\todo Rethink about cpu measurements, and reenable/recode what's needed
		///\name cpu cost measurement
		///\{
			public:
				void inline cpu_idle(cpu::cycles_type const & value) throw() { cpu_idle_ = value; }
				cpu::cycles_type inline cpu_idle() const throw() { return cpu_idle_; }
			private:
				cpu::cycles_type cpu_idle_;
		///\}
	#endif

	///\name actions with machines
	///\{
		public:
			/// add a new machine. If newIdx is not -1 and a machine
			/// does not exist in that place, it is taken as the new index
			/// If a machine exists, or if newIdx is -1, the index is taken from pmac.
			/// if pmac->id() is -1, then a free index is taken.
			///
			virtual void AddMachine(Machine * pmac, Machine::id_type newIdx = -1);
			/// (add or) replace the machine in index idx.
			// idx cannot be -1.
			virtual void ReplaceMachine(Machine * pmac, Machine::id_type idx);
			/// Exchange the position of two machines
			virtual void ExchangeMachines(Machine::id_type mac1, Machine::id_type mac2);
			/// destroy a machine of this song.
			virtual void DeleteMachine(Machine * machine);
			/// destroy a machine of this song.
			virtual void DeleteMachineDeprecated(Machine::id_type mac) {
				if (machine(mac)) DeleteMachine(machine(mac));
			}
			/// destroys all the machines of this song.
			virtual void DeleteAllMachines();
		
			/// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
			Machine::id_type GetFreeBus();
			/// Gets the first free slot in the Effects' bus (slots MAX_BUSES  to 2*MAX_BUSES-1)
			Machine::id_type GetFreeFxBus();
			/// Returns the Bus index out of a machine id.

		public:
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

			//\param wiredest new dest mac index
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
			/// Exchanges the positions of two instruments
			void ExchangeInstruments(Instrument::id_type src, Instrument::id_type dst);
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

	public:
		void patternTweakSlide(int machine, int command, int value, int patternPosition, int track, int line);

	///\name thread synchronisation
	///\{
		public:
			typedef class scoped_lock<mutex> scoped_lock;
			mutex & Mutex() const { return mutex_; }
		private:
			mutex mutable mutex_;
	///\}
};

/// Song extends CoreSong with UI-related stuff
class PSYCLE__CORE__DECL Song : public CoreSong {
	///\todo: For derived UI machines, the data is hold by Song in a class VisualMachine.
	// This means that a Song maintans a separate array of VisualMachines for the Non-visual counterparts and gives access
	// to these with ui-specific methods.
	// To help working with them, a VisualMachine can have a reference to the related non-visual machine, so that the UI parts
	// can access the real machine via the VisualMachine class.

	public:
		Song();
		virtual ~Song() {}

		virtual void clear();
		virtual void DeleteMachine(Machine* mac);
	private:
		void clearMyData();

	public:
		/// Is this song saved to a file?
		bool _saved;

	///\name various ui-related stuff
	///\{
		public:
			/// Current selected machine number in the GUI
			Machine::id_type seqBus;

		private:
			/// Current selected instrument number in the GUI
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

			// Compatibility with older psycle::host
			void SetDefaultPatternLines(int defaultPatLines) {}

			int GetHighestInstrumentIndex() {
				int i;
				for(i = MAX_INSTRUMENTS - 1; i >= 0; --i) if(! this->_pInstrument[i]->Empty()) break;
				return i;
			}

			const int BeatsPerMin() { return bpm(); }
			void BeatsPerMin(const int value) { setBpm(value); }

			const int LinesPerBeat(){return ticksSpeed();}
			void LinesPerBeat(const int value) { setTicksSpeed(value); }
			
			void New();
	///\}

	//Fake. Just to compile. They need to go out
	//{
		unsigned char asdf[5];
		unsigned char * _ppattern(int ps){ return asdf; }
		unsigned char * _ptrack(int ps, int track){ return asdf; }
		unsigned char * _ptrackline(int ps, int track, int line) { return asdf; }

		bool _trackArmed[MAX_TRACKS];
		bool _trackMuted[MAX_TRACKS];
		int _trackArmedCount;

		int playOrder[MAX_SONG_POSITIONS];
		int playOrderSel[MAX_SONG_POSITIONS];
		int playLength;

		int patternLines[MAX_PATTERNS];
		char patternName[MAX_PATTERNS][32];

		unsigned char currentOctave;
	//}
};

}}
#endif
