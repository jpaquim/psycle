// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\implementation psycle::plugins::additioner
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "additioner.hpp"
namespace psycle { namespace plugins {

	PSYCLE__PLUGINS__NODE_INSTANTIATOR(additioner)

	void additioner::do_process() throw(engine::exception) {
		bipolar_filter::do_process();
		engine::buffer & in = multiple_input_port()->buffer();
		engine::buffer & out = output_ports()[0]->buffer();
		for(std::size_t channel(0) ; channel < in.size() ; ++channel)
			for(std::size_t event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event)
				out[channel][event].sample() += in[channel][event].sample();
	}
}}

