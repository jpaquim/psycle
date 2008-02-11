/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\implementation psycle::front_ends::gui::lock
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "lock.hpp"
#include <glibmm/thread.h>
/*#include <gdk/gdk.h>*/ extern "C" { void gdk_threads_init(); }
namespace psycle { namespace front_ends { namespace gui {

bool lock::initialized_(false);
void lock::init() {
	if(!initialized()) {
		if(!Glib::thread_supported()) Glib::thread_init();
		::gdk_threads_init();
		initialized_ = true;
	}
	assert(initialized());
}

}}}

