// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::plugins::multiplier
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "multiplier.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(multiplier)

void multiplier::do_process() throw(engine::exception) {
	if(!multiple_input_port()->output_ports().size()) return;
	if(!output_ports()[0]->input_ports().size()) return;
	bipolar_filter::do_process();
	assert(&multiple_input_port()->buffer());
	assert(&output_ports()[0]->buffer());
	engine::buffer & in(multiple_input_port()->buffer());
	engine::buffer & out(output_ports()[0]->buffer());
	assert(out.channels() == in.channels());
	for(std::size_t channel(0) ; channel < in.channels() ; ++channel)
		for(std::size_t event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event) {
			// not needed because in and out have the same buffer: out[channel][event].index(event);
			out[channel][event].sample() *= in[channel][event].sample();
		}

}

}}

