// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::os::aligned_memory_alloc

#ifndef UNIVERSALIS__OS__ALIGNED_MEMORY_ALLOC__INCLUDED
#define UNIVERSALIS__OS__ALIGNED_MEMORY_ALLOC__INCLUDED
#pragma once

#include "exception.hpp"
#include <diversalis/os.hpp>
#include <diversalis/compiler.hpp>
#include <universalis/compiler/location.hpp>

#if defined DIVERSALIS__OS__POSIX
	#include <cstdlib> // for posix_memalign
#endif

namespace universalis { namespace os {

template<typename X>
void aligned_memory_alloc(std::size_t alignment, X *& x, std::size_t count) {
	std::size_t const size(count * sizeof(X));
	#if defined DIVERSALIS__OS__POSIX
			void * address;
			int const err(posix_memalign(&address, alignment, size));
			if(err) throw exception(err, UNIVERSALIS__COMPILER__LOCATION);
			x = static_cast<X*>(address);
			// note: free with std::free
	#elif 0///\todo defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__GNU
			x = static_cast<X*>(__mingw_aligned_malloc(size, alignment));
			// note: free with _mingw_aligned_free
	#elif defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
			x = static_cast<X*>(_aligned_malloc(size, alignment));
			// note: free with _aligned_free
	#else
		// could also try _mm_malloc (#include <xmmintr.h> or <emmintr.h>?)
		// memalign on SunOS but not BSD (#include both <cstdlib> and <cmalloc>)
		// note that memalign is declared obsolete and does not specify how to free the allocated memory.
		
		size; // unused
		x = new X[count];
		// note: free with delete[]
	#endif
}

template<typename X>
void aligned_memory_dealloc(X *& address) {
	#if defined DIVERSALIS__OS__POSIX
		std::free(address);
	#elif 0///\todo: defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__GNU
		_aligned_free(address);
	#elif defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
		_aligned_free(address);
	#else
		delete[] address;
	#endif
}

}}

#endif
