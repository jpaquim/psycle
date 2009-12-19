// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::front_ends::gui::lock
#include <psycle/detail/project.private.hpp>
#include "lock.hpp"
#include <glibmm/thread.h>

//#include <gdk/gdk.h>
extern "C" {
	void gdk_threads_init();
}

namespace psycle { namespace front_ends { namespace gui {

bool lock::initialized_(false);

void lock::init() {
	if(!initialized_) {
		if(!Glib::thread_supported()) Glib::thread_init();
		::gdk_threads_init();
		initialized_ = true;
	}
}

}}}
