// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#include <psycle/detail/project.private.hpp>
#include "additioner.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(additioner)

void additioner::do_process() {
	if(!out_port()) return;
	if(!in_port()) return;
	bipolar_filter::do_process();
	buffer & in(in_port().buffer());
	buffer & out(out_port().buffer());
	assert(out.channels() == in.channels());
	for(std::size_t channel = 0; channel < in.channels() ; ++channel)
		for(std::size_t event = 0; event < in.events() && in[channel][event].index() < in.events() ; ++event)
			out[channel][event].sample() += in[channel][event].sample();
			// note that we do not need to set the index because in and out have the same buffer.
}

}}
