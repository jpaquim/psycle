// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright © 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief \implementation psycle::plugins::output
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "output.hpp"
namespace psycle
{
	namespace plugins
	{
		PSYCLE__PLUGINS__NODE_INSTANCIATOR(output)

		output::output(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(engine::exception)
		:
			base(plugin_library_reference, graph, name)
		{
		}
	}
}
