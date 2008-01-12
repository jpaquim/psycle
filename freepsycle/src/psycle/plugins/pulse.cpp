// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::pulse
#include <psycle/detail/project.private.hpp>
#include "pulse.hpp"
#include <cstdint>
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(pulse)

pulse::pulse(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
:
	node(plugin_library_reference, graph, name),
	i_(events_.end()),
	beats_per_second_(),
	seconds_per_beat_(),
	beat_()
{
	engine::ports::output::create_on_heap(*this, "out");
}

void pulse::beat(real beat) {
	this->beat_ = beat;
	i_ = events_.begin();
	while(i_ != events_.end() && i_->beat() < beat) ++i_;
}

void pulse::insert_event(real beat, real sample) {
	if(events_.size() && events_.back().beat() < beat) events_.push_back(event(beat, sample));
	else {
		events_type::iterator i(events_.begin());
		while(i != events_.end() && i->beat() < beat) ++i;
		events_.insert(i, event(beat, sample));
	}
	this->beat(this->beat()); // recompute the iterator
}

void pulse::erase_events(real begin_beat, real end_beat) {
	events_type::iterator first(events_.begin());
	while(first != events_.end() && first->beat() < begin_beat) ++first;
	if(first == events_.end()) return;
	events_type::iterator last(first);
	while(last != events_.end() && last->beat() > end_beat) ++last;
	if(last != events_.end() && first != last && last->beat() == end_beat) --last; // exclude end_beat from range
	events_.erase(first, last);
	beat(beat()); // recompute the iterator
}

void pulse::do_process() throw(engine::exception) {
	if(!*output_ports()[0]) return;
	engine::buffer & out(output_ports()[0]->buffer());
	if(!out.events()) return; // ?
	real const samples_per_beat = output_ports()[0]->events_per_second() / beats_per_second_;
	std::uint64_t const initial_sample(static_cast<std::size_t>(beat_ * samples_per_beat));
	real last_beat(beat_ + (out.events() - 1) / samples_per_beat);
	std::size_t const channels(output_ports()[0]->channels());
	std::size_t last_index(0);
	for(; i_ != events_.end() && i_->beat() < last_beat; ++i_) {
		event const & e(*i_);
		std::size_t const i(static_cast<std::size_t>(e.beat() * samples_per_beat - initial_sample));
		if(i < out.events()) {
			for(std::size_t c(0); c < channels; ++c) out[c][last_index](i, e.sample());
			++last_index;
		}
		else {
			// event lost! should never happen.
			if(loggers::warning()) {
				std::ostringstream s;
				s << "event lost: " << e.beat() << ' ' << e.sample();
				loggers::warning()(s.str());
			}
		}
	}
	if(last_index < out.events()) for(std::size_t c(0); c < channels; ++c) out[c][last_index].index(out.events());
	beat_ = last_beat;
}

}}

