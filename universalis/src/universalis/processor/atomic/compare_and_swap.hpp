// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::processor::atomic::compare_and_swap
#pragma once
#include <universalis/operating_system/exception.hpp>
#include <diversalis/compiler.hpp>
#if defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION >= 40100 // 4.1.0
	// gcc built-in functions documented at http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html
#else
	#include <glib/gatomic.h>
#endif
namespace universalis { namespace processor { namespace atomic {

// see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2047.html
// see http://tech.groups.yahoo.com/group/liblf-dev
// see http://www.audiomulch.com/~rossb/code/lockfree/liblfds
// see http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_memorybarrier.h
// see http://webcvs.freedesktop.org/gstreamer/gst-plugins-base/gst-libs/gst/audio/gstringbuffer.c?view=markup
// see http://svn.gnome.org/viewvc/glib/trunk/glib/gatomic.c?view=markup

/// atomic compare and swap.
/// with full memory barrier.
/// does not fail spuriously.
template<typename Value>
bool inline compare_and_swap(Value * const address, Value const old_value, Value const new_value) {
	#if defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION >= 40100 // 4.1.0
		return __sync_bool_compare_and_swap(address, old_value, new_value);
	#else
		// no return statement, on purpose, to use the sfinae rule
	#endif
}

#if !defined DIVERSALIS__COMPILER__GNU || DIVERSALIS__COMPILER__VERSION < 40100 // 4.1.0
	template<>
	bool inline compare_and_swap< ::gpointer>(::gpointer * const address, ::gpointer const old_value, ::gpointer const new_value) {
		return ::g_atomic_pointer_compare_and_exchange(address, old_value, new_value);
	}

	template<>
	bool inline compare_and_swap< ::gint>(::gint * const address, ::gint const old_value, ::gint const new_value) {
		return ::g_atomic_int_compare_and_exchange(address, old_value, new_value);
	}
#endif

#if 0
	// other operations
	
	// gcc
	// "..." optional list of values to protect (currently unused)
	
	type __sync_val_compare_and_swap (type * p, type oldval, type newval, ...);
	// returns the content of *ptr before the operation
	
	void __sync_synchronize();
	// full memory barrier
	
	with op in (add, sub, or, and, xor, nand):
		type __sync_fetch_and_<op>(type * p, type value, ...);
		type __sync_<op>_and_fetch(type * p, type value, ...);
	
	type __sync_lock_test_and_set (type * p, type value, ...);
	// not a full memory barrier
	
	void __sync_lock_release(type * p, ...);
	// not a full memory barrier
		
#endif

}}}
