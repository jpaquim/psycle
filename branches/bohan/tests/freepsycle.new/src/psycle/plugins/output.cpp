// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::output
#include <psycle/detail/project.private.hpp>
#include "output.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(output)

output::output(class plugin_library_reference & plugin_library_reference, name_type const & name) throw(exception)
:
	base(plugin_library_reference, name)
{}

}}
