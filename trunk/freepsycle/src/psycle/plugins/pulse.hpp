// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::pulse
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__PULSE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

class UNIVERSALIS__COMPILER__DYNAMIC_LINK pulse : public engine::node {
	protected: friend class virtual_factory_access;
		pulse(engine::plugin_library_reference &, engine::graph &, std::string const & name);
	protected:
		void do_process() throw(engine::exception) /*override*/;

	///\name beats per second
	///\{
		public:
			real beats_per_second() const { return beats_per_second_; }
			void beats_per_second(real beats_per_second) { this->beats_per_second_ = beats_per_second; }
		private:
			real beats_per_second_;
	///\}
	
	///\name beat
	///\{
		public:
			real const beat() { return beat_; }
			void beat(real beat) { this->beat_ = beat; }
		private:
			real beat_;
			std::size_t i;
	///\}
		
	///\name events
	///\{
		public:
			class event {
				public:
					event(real beat, real sample) : beat_(beat), sample_(sample) {}
					
				///\name beat
				///\{
					public:
						real beat() const { return beat_;}
					private:
						real beat_;
				///\}
					
				///\name sample
				///\{
					public:
						real sample() const { return sample_; }
						void sample(real sample) { this->sample_ = sample; }
					private:
						real sample_;
				///\}
			};

			void add_event(real beat, real sample) { events_.push_back(event(beat, sample)); }

			typedef std::vector<event> events_type;
			events_type       & events()       { return events_; }
			events_type const & events() const { return events_; }
		private:
			events_type         events_;
			/// index in events_ container corresponding to current beat_
			std::size_t index_;
	///\}
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

