// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief \implementation psycle::plugins::output
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "output.hpp"
namespace psycle
{
	namespace plugins
	{
		PSYCLE__PLUGINS__NODE_INSTANTIATOR(output)

		output::output(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(engine::exception)
		:
			base(plugin_library_reference, graph, name)
		{
		}
	}
}
