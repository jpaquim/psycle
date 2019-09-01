// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief \implementation psycle::plugins::multiplier
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "multiplier.hpp"
namespace psycle
{
	namespace plugins
	{
		PSYCLE__PLUGINS__NODE_INSTANCIATOR(multiplier)

		void multiplier::do_process() throw(engine::exception)
		{
			bipolar_filter::do_process();
			engine::buffer & in = multiple_input_port()->buffer();
			engine::buffer & out = output_ports()[0]->buffer();
			for(unsigned int channel(0) ; channel < in.size() ; ++channel)
				for(unsigned int event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event)
					out[channel][event].sample() *= in[channel][event].sample();
		}

	}
}
