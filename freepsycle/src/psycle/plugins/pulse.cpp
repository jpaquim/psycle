// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

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
	beats_per_second_(1),
	seconds_per_beat_(1),
	beat_()
{
	engine::ports::output::create_on_heap(*this, "out");
}

void pulse::beat(real beat) {
	this->beat_ = beat;
	i_ = events_.lower_bound(beat);
}

void pulse::insert_event(real beat, real sample) {
	events_[beat] = sample;
	this->beat(this->beat()); // recompute the iterator
}

void pulse::erase_events(real begin_beat, real end_beat) {
	events_type::iterator last(events_.lower_bound(end_beat));
	if(last != events_.end()) --last; // exclude end_beat from range
	events_.erase(events_.lower_bound(begin_beat), last);
	beat(beat()); // recompute the iterator
}

void pulse::do_process() throw(engine::exception) {
	if(!*output_ports()[0]) return;
	engine::buffer & out(output_ports()[0]->buffer());
	real const samples_per_beat = output_ports()[0]->events_per_second() / beats_per_second_;
	std::uint64_t const initial_sample(static_cast<std::size_t>(beat_ * samples_per_beat));
	real last_beat(beat_ + (out.events() - 1) / samples_per_beat);
	std::size_t const channels(output_ports()[0]->channels());
	std::size_t last_index(0);
	for(; i_ != events_.end() && i_->first < last_beat; ++i_) {
		real const b(i_->first), s(i_->second);
		std::size_t const i(static_cast<std::size_t>(b * samples_per_beat - initial_sample));
		if(i < out.events()) {
			for(std::size_t c(0); c < channels; ++c) out[c][last_index](i, s);
			++last_index;
		} else { // event lost! should never happen.
			if(loggers::warning()) {
				std::ostringstream s;
				s << "event lost: " << b << ' ' << s;
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}
	if(last_index) for(std::size_t c(0); c < channels; ++c) out[c].flag(channel::flags::discrete);
	else for(std::size_t c(0); c < channels; ++c) out[c].flag(channel::flags::empty);
	if(last_index < out.events()) for(std::size_t c(0); c < channels; ++c) out[c][last_index].index(out.events());
	beat_ = last_beat;
}

}}

