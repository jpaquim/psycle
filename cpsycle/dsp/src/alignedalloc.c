// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#define __cplusplus
#include <diversalis/os.hpp>
#undef __cplusplus

#include "alignedalloc.h"
#include <malloc.h>
#include <stdio.h>
#include <assert.h>

void* aligned_memory_alloc(size_t alignment, size_t size) {		
#if defined DIVERSALIS__OS__POSIX
	void* address;
	int const err(posix_memalign(&address, alignment, size));
	if (err) throw exception(err, UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
	x = static_cast<X*>(address);
	// note: free with std::free
#elif 0///\todo defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__GNU
	x = static_cast<X*>(__mingw_aligned_malloc(size, alignment));
	// note: free with _mingw_aligned_free
#elif defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
	#if defined DIVERSALIS__CPU__X86__SSE
		return _aligned_malloc(size, alignment);
	#else
		assert(0);
		return 0;
	#endif
	// note: free with _aligned_free
#else
	// could also try _mm_malloc (#include <xmmintr.h> or <emmintr.h>?)
	// memalign on Solaris but not BSD (#include both <cstdlib> and <cmalloc>)
	// note that memalign is declared obsolete and does not specify how to free the allocated memory.

	size; // unused
	x = new X[count];
	// note: free with delete[]
#error "TODO alloc some extra mem and store orig pointer or alignment to the x[-1]"
#endif	
}

void aligned_memory_dealloc(void* address) {
#if defined DIVERSALIS__OS__POSIX
	std::free(address);
#elif 0///\todo: defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__GNU
	_aligned_free(address);
#elif defined DIVERSALIS__OS__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
	#if defined DIVERSALIS__CPU__X86__SSE
		_aligned_free(address);
	#else
		assert(0);
	#endif
#else
	delete[] address;
#error "TODO alloc some extra mem and store orig pointer or alignment to the x[-1]"
#endif
}