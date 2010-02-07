// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

// interface psycle::core::Song

#ifndef PSYCLE__CORE__SONG__INCLUDED
#define PSYCLE__CORE__SONG__INCLUDED
#pragma once

#include "sequence.h"
#include "songserializer.h"
#include "machine.h"
#include "instrument.h"
#include "xminstrument.h"
#include <universalis/stdlib/cstdint.hpp>
#include <universalis/stdlib/mutex.hpp>

namespace psycle { namespace core {


class PluginFinderKey;
class MachineFactory;

// songs hold everything comprising a "tracker module",
// this include patterns, pattern sequence, machines, wavetables
// and their initial parameters
class PSYCLE__CORE__DECL CoreSong {
	public:
		CoreSong();
		virtual ~CoreSong();

		//todo private
		Instrument* _pInstrument[MAX_INSTRUMENTS];
		XMInstrument m_Instruments[MAX_INSTRUMENTS];
		XMInstrument::WaveData m_rWaveLayer[MAX_INSTRUMENTS];

		const Sequence& sequence() const throw() { return sequence_; }
		Sequence& sequence() throw() { return sequence_; }
		virtual void clear();
		// serialization
		bool load(const std::string& filename);
		bool save(const std::string& filename, int version = 4);
		boost::signal<void (const std::string&, const std::string&)> report;
		boost::signal<void (int, int, const std::string&)> progress;
		// The file name this song was loaded from.
		void set_filename(const std::string& filename) {
			filename_ = filename;
		}
		const std::string& filename() const { return filename_; }
		const std::string& name() const { return name_; } // song name
		void setName(const std::string& name) { name_ = name; }
		const std::string& comment() const { return comment_; }
		void setComment(const std::string& comment) { comment_ = comment; }
		// initial bpm for the song
		float bpm() const { return bpm_; }
		void setBpm(float bpm) { if(bpm > 0 && bpm < 1000) bpm_ = bpm; }
		// Identifies if the song is operational or in a loading/saving state.
		// It serves as a non-locking synchronization
		bool IsReady(){ return ready; }
		// Sets song to ready state (it can be played).
		void SetReady(const bool value) {
			scoped_lock lock(mutex_);
			ready = value;
		}
		const std::string& author() const { return author_; }
		void setAuthor(const std::string& author) { author_ = author; }
		// delegation from sequence
		unsigned int tracks() const { return sequence_.numTracks(); }
		void setTracks( unsigned int tracks) { 
			sequence_.setNumTracks(tracks);
		}
		// name the initial ticks per beat (TPB) when the song starts to play.
		// With multisequence, ticksSpeed helps on syncronization and timing.
		// Concretely, it helps to send the legacy "SequencerTick()" events to native plugins,
		// helps in knowing for how much a command is going to be tick'ed ( tws, as well as
		// retrigger code need to know how much they last ) as well as helping Sampulse
		// to get ticks according to legacy speed command of Modules.
		// isTicks is used to identify if the value in ticksPerBeat_ means ticks, or speed.
		unsigned int ticksSpeed() const { return ticks_; }
		bool isTicks() const { return isTicks_; }
		void setTicksSpeed(unsigned int const value, bool const isticks = true) {
			if(value < 1) ticks_ = 1;
			else if(value > 31) ticks_ = 31;
			else ticks_ = value;
			isTicks_ = isticks;
		}
		// access to the machines of the song
		Machine* machine(Machine::id_type id) { return machine_[id]; }
		// access to the machines of the song
		const Machine* const machine(Machine::id_type id) const {
			return machine_[id];
		}
		void machine(Machine::id_type id, Machine* machine) {
			assert(id >= 0 && id < MAX_MACHINES);
			machine_[id] = machine;
			machine->id(id);
		}
		
		XMInstrument& rInstrument(const int index) { return m_Instruments[index]; }
		XMInstrument::WaveData& SampleData(const int index) {
			return m_rWaveLayer[index];
		}

		// add a new machine. If newIdx is not -1 and a machine
		// does not exist in that place, it is taken as the new index
		// If a machine exists, or if newIdx is -1, the index is taken from pmac.
		// if pmac->id() is -1, then a free index is taken.
		//
		virtual void AddMachine(Machine * pmac, Machine::id_type newIdx = -1);
		// (add or) replace the machine in index idx.
		// idx cannot be -1.
		virtual void ReplaceMachine(Machine * pmac, Machine::id_type idx);
		// Exchange the position of two machines
		virtual void ExchangeMachines(Machine::id_type mac1, Machine::id_type mac2);
		// destroy a machine of this song.
		virtual void DeleteMachine(Machine * machine);
		// destroy a machine of this song.
		virtual void DeleteMachineDeprecated(Machine::id_type mac) {
			if (machine(mac)) DeleteMachine(machine(mac));
		}
		// destroys all the machines of this song.
		virtual void DeleteAllMachines();
		// Gets the first free slot in the Machines' bus (slots 0 to MAX_BUSES-1)
		Machine::id_type GetFreeBus();
		// Gets the first free slot in the Effects' bus (slots MAX_BUSES to 2*MAX_BUSES-1)
		Machine::id_type GetFreeFxBus();
		// Returns the Bus index out of a machine id.
		Machine::id_type FindBusFromIndex(Machine::id_type);

		//name machine connections
		// creates a new connection between two machines.
		// This funcion is to be used over the Machine's ConnectTo(). This one verifies the validity of the connections, and uses Machine's function
		Wire::id_type InsertConnection(
			Machine& srcMac,
			Machine& dstMac,
			InPort::id_type srctype=0,
			OutPort::id_type dsttype=0,
			float volume = 1.0f);
		// Changes the destination of a wire connection.
		// param wiresource source mac index
		// param wiredest new dest mac index
		// param wireindex index of the wire in wiresource to change
		bool ChangeWireDestMac(
			Machine& srcMac,
			Machine& newDstMac,
			OutPort::id_type srctype,
			Wire::id_type wiretochange,
			InPort::id_type dsttype);
		// Changes the destination of a wire connection.
		// param wiredest dest mac index
		//param wiresource new source mac index
		//param wireindex index of the wire in wiredest to change
		bool ChangeWireSourceMac(
			Machine& newSrcMac,
			Machine &dstMac,
			InPort::id_type dsttype,
			Wire::id_type wiretochange,
			OutPort::id_type srctype);
	
		//thread synchronisation
		typedef class scoped_lock<mutex> scoped_lock;
		operator mutex& () const { return mutex_; }

		// instrument actions
		// todo: The loading code should not be inside the song class, only the assignation of the loaded one
		bool WavAlloc(Instrument::id_type, const char* str);
		bool WavAlloc(Instrument::id_type, bool bStereo, long int iSamplesPerChan, const char* sName);
		bool IffAlloc(Instrument::id_type, const char* str);
		/// clones an instrument.
		bool CloneIns(Instrument::id_type src, Instrument::id_type dst);
		// Exchanges the positions of two instruments
		void ExchangeInstruments(Instrument::id_type src, Instrument::id_type dst);
		// resets the instrument and delete each sample/layer that it uses.
		void /*Reset*/DeleteInstrument(Instrument::id_type id);
		// resets the instrument and delete each sample/layer that it uses. (all instruments)
		void /*Reset*/DeleteInstruments();
		void patternTweakSlide(
			int machine,
			int command,
			int value, 
			int patternPosition,
			int track,
			int line);

		// just here till i find in the host another solution
		// Current selected machine number in the GUI
		Machine::id_type seqBus;

		/// cpu time usage measurement
		void reset_time_measurement() { accumulated_processing_time_ = accumulated_routing_time_ = 0; }

		/// total processing cpu time usage measurement
		nanoseconds accumulated_processing_time() const throw() { return accumulated_processing_time_; }

		/// total processing cpu time usage measurement
		void accumulate_processing_time(nanoseconds ns) throw() {
			if(loggers::warning() && ns.get_count() < 0) {
				std::ostringstream s;
				s << "time went backward by: " << ns.get_count() * 1e-9 << 's';
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			} else accumulated_processing_time_ += ns;
		}

		/// routing cpu time usage measurement
		nanoseconds accumulated_routing_time() const throw() { return accumulated_routing_time_; }
		/// routing cpu time usage measurement
		void accumulate_routing_time(nanoseconds ns) throw() {
			if(loggers::warning() && ns.get_count() < 0) {
				std::ostringstream s;
				s << "time went backward by: " << ns.get_count() * 1e-9 << 's';
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			} else accumulated_routing_time_ += ns;
		}

	protected:
		bool ValidateMixerSendCandidate(Machine& mac,bool rewiring=false);
		protected:
		// deletes all instruments in this song.
		void /*Delete*/FreeInstrumentsMemory();
		// removes the sample/layer of the instrument
		void DeleteLayer(Instrument::id_type id);

	private:
		static SongSerializer serializer;
		std::string filename_;
		bool ready;
		std::string name_;
		std::string author_;
		std::string comment_;
		float bpm_;
		unsigned int ticks_;
		bool isTicks_;
		Sequence sequence_;
		Machine* machine_[MAX_MACHINES];
		mutable mutex mutex_;
		nanoseconds accumulated_processing_time_, accumulated_routing_time_;
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
			
	///\}

	///\todo Fake. Just to compile. They need to go out
	//{
		int _trackArmedCount;

		unsigned char currentOctave;
	//}
};

}}
#endif
