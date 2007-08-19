// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::clocks
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "clocks.hpp"
namespace universalis { namespace operating_system { namespace clocks {
	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		::clockid_t static wall_clock_id, process_clock_id, thread_clock_id;
	#endif
}}}
