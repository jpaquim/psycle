// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::engine::sequence
#pragma once
#include "named.hpp"
#include "buffer.hpp"
#include "exception.hpp"
#include <map>
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

/// a sequence of events
class PSYCLE__DECL sequence : public named {
	public:
		/// constructs a sequence.
		sequence(std::string const & name) : named(name) {}

	///\name events
	///\{
		public:
			/// inserts an event
			void insert_event(real beat, real sample) { events_[beat] = sample; }
			/// erases an event
			void erase_event(real beat) { events_.erase(beat); }
			/// erases the range [begin_beat, end_beat[
			void erase_events(real begin_beat, real end_beat);
		private: friend class sequence_iterator;
			typedef std::map<real, real> events_type;
			events_type events_;
	///\}
};

/// an iterator over a sequence
class PSYCLE__DECL sequence_iterator {
	public:
		/// constructs a sequence iterator.
		sequence_iterator(sequence const &);

		/// outputs events to the given buffer and advances the beat position.
		void process(buffer & out, real events_per_second, std::size_t channels) throw(exception);

	private:
		/// the sequence on which the iteration is done
		sequence const & sequence_;
		/// iterator in the events_ container positionned at the current beat_
		sequence::events_type::const_iterator i_;

	///\name beats per second
	///\{
		public:
			real beats_per_second() const { return beats_per_second_; }
			void beats_per_second(real beats_per_second) {
				this->beats_per_second_ = beats_per_second;
				this->seconds_per_beat_ = beats_per_second ? 1 / beats_per_second : 0;
			}
		private:
			real beats_per_second_;
	///\}
	
	///\name seconds per beat
	///\{
		public:
			real seconds_per_beat() const { return seconds_per_beat_; }
			void seconds_per_beat(real seconds_per_beat) {
				this->seconds_per_beat_ =     seconds_per_beat;
				this->beats_per_second_ = 1 / seconds_per_beat;
			}
		private:
			real seconds_per_beat_;
	///\}

	///\name position in beat unit
	///\{
		public:
			real beat() const throw() { return beat_; }
			void beat(real beat);
		private:
			real beat_;
	///\}

	///\name position in second unit
	///\{
		public:
			real seconds() const throw() { return beat_ * seconds_per_beat(); }
			void seconds(real seconds) { beat(seconds * beats_per_second()); }
	///\}
};

}}
#include <psycle/detail/decl.hpp>
