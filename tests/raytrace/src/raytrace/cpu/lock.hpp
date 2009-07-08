// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__LOCK
#define RAYTRACE__LOCK
#pragma once

/*#include <gdk/gdk.h>*/ extern "C" { void gdk_threads_enter(); /*void gdk_flush();*/ void gdk_threads_leave(); }
#include <cassert>

namespace raytrace {

class lock {
	public:
		void static init();
		bool static initialized() { return initialized_; }
	private:
		bool static initialized_;

	public:
		lock() {
			class once { public: once() { init(); } };
			once static once;
			assert(initialized());
			::gdk_threads_enter();
		}
		
		~lock() throw() {
			//::gdk_flush();
			::gdk_threads_leave();
		}
};

}

#endif
