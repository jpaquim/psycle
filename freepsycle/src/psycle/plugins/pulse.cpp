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
	beats_per_second_(),
	beat_(),
	i()
{
	engine::ports::output::create_on_heap(*this, "out");
}

void pulse::do_process() throw(engine::exception) {
//std::clog << "pulse " << name() << '\n';
	if(!*output_ports()[0]) return;
	engine::buffer & out(output_ports()[0]->buffer());
	if(!out.events()) return; // ?
	real const samples_per_beat = output_ports()[0]->events_per_second() / beats_per_second_;
	std::uint64_t const initial_sample(static_cast<std::size_t>(beat_ * samples_per_beat));
	real const last_beat(beat_ + out.events() / samples_per_beat);
//std::clog << samples_per_beat << ' ' << out.events() << ' ' << initial_sample << ' ' << i << ' ' << last_beat << '\n';
	std::size_t const channels(output_ports()[0]->channels());
	std::size_t last_index(0);
	for(; i < events_.size() && events_[i].beat() < last_beat; ++i) {
		event & e(events_[i]);
		std::size_t const i(static_cast<std::size_t>(e.beat() * samples_per_beat - initial_sample));
		if(i < out.events()) {
//std::clog << "event " << e.beat() << ' ' << e.sample() << ' ' << i << '\n';
			for(std::size_t c(0); c < channels; ++c) out[c][last_index](i, e.sample());
			++last_index;
		}
		else {
			// event lost!
std::clog << "event lost " << e.beat() << ' ' << e.sample() << ' ' << i << '\n';
		}
	}
	if(last_index < out.events()) for(std::size_t c(0); c < channels; ++c) out[c][last_index].index(out.events());
	beat_ = last_beat;
//std::clog << last_index << '\n';
}

}}

