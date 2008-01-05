// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::pulse
#include <psycle/detail/project.private.hpp>
#include "pulse.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(pulse)

pulse::pulse(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
:
	node(plugin_library_reference, graph, name),
	sample_set_()
{
	engine::ports::output::create_on_heap(*this, "out");
}

void pulse::do_process() throw(engine::exception) {
	if(!*output_ports()[0]) return;
	engine::buffer & out(output_ports()[0]->buffer());
	if(!out.events()) return;
	if(!sample_set_) {
		for(std::size_t channel(0); channel < out.size(); ++channel)
			out[channel][0].index(out.events());
	} else {
		for(std::size_t channel(0); channel < out.size(); ++channel) {
			out[channel][0](index_, sample_);
			if(out.events() > 1) out[channel][1].index(out.events());
		}
		sample_set_ = false;
	}
}

}}

