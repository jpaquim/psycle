// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::front_ends::gui::lock
#ifndef PSYCLE__FRONT_ENDS__GUI__LOCK__INCLUDED
#define PSYCLE__FRONT_ENDS__GUI__LOCK__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#include <cassert>

//#include <gdk/gdk.h>
extern "C" {
	void gdk_threads_enter();
	//void gdk_flush();
	void gdk_threads_leave();
}

namespace psycle { namespace front_ends { namespace gui {

class lock {
	public:
		void static init();
		bool static initialized() { return initialized_; }
	private:
		bool static initialized_;

	public:
		lock() {
			assert(initialized_);
			::gdk_threads_enter();
		}
		
		~lock() throw() {
			//::gdk_flush();
			::gdk_threads_leave();
		}
};

}}}
#endif
