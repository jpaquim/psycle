// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::multiplier
#include <psycle/detail/project.private.hpp>
#include "multiplier.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(multiplier)

void multiplier::do_process() throw(engine::exception) {
	if(!*output_ports()[0]) return;
	if(!*multiple_input_port()) return;
	bipolar_filter::do_process();
	engine::buffer & in(multiple_input_port()->buffer());
	engine::buffer & out(output_ports()[0]->buffer());
	assert(out.channels() == in.channels());
	for(std::size_t channel(0) ; channel < in.channels() ; ++channel)
		for(std::size_t event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event)
			out[channel][event].sample() *= in[channel][event].sample();
			// note that we do not need to set the index because in and out have the same buffer.
}

}}
