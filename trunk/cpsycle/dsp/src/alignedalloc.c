// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "alignedalloc.h"
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include "../../detail/psyconf.h"

void* psy_dsp_aligned_memory_alloc(size_t alignment, size_t count, size_t size) {
#if POSIX
	void* address;
	int err;
	
	err = posix_memalign(&address, alignment, size * count);
	if (err) {
	    address = 0;
	}
	return address;
#elif defined(_WIN32) && defined(PSYCLE_USE_SSE) && defined(DIVERSALIS__CPU__X86__SSE)
		return _aligned_malloc(size * count, alignment);
#else
	return 0;
#endif	
}

void psy_dsp_aligned_memory_dealloc(void* address) {
#if defined _WIN32 && defined PSYCLE_USE_SSE && defined DIVERSALIS__CPU__X86__SSE
	_aligned_free(address);	
#else
	free(address);
#endif
}
