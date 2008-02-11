/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::sequence
#pragma once
#include "plugin.hpp"
#include <map>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__SEQUENCE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// an event scheduler
class UNIVERSALIS__COMPILER__DYNAMIC_LINK sequence : public engine::node {
	protected: friend class virtual_factory_access;
		sequence(engine::plugin_library_reference &, engine::graph &, std::string const & name);
	protected:
		void do_process() throw(engine::exception) /*override*/;

	///\name events
	///\{
		public:
			/// inserts an event
			void insert_event(real beat, real sample);
			/// erases the range [begin_beat, end_beat[
			void erase_events(real begin_beat, real end_beat);
		private:
			typedef std::map<real, real> events_type;
			events_type events_;
			/// iterator in events_ container corresponding to current beat_
			events_type::const_iterator i_;
	///\}

	///\name beats per second
	///\{
		public:
			real beats_per_second() const { return beats_per_second_; }
			void beats_per_second(real beats_per_second) {
				this->beats_per_second_ =     beats_per_second;
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
			real const beat() { return beat_; }
			void beat(real beat);
		private:
			real beat_;
	///\}

	///\name position in second unit
	///\{
		public:
			real const seconds() { return beat_ * seconds_per_beat(); }
			void seconds(real seconds) { beat(seconds * beats_per_second()); }
	///\}
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

