/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "alignedalloc.h"
/* std */
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
/* types */
#include "../../detail/psyconf.h"
#include "../../detail/os.h"
#include "../../detail/cpu.h"

/* implementation */
void* psy_dsp_aligned_memory_alloc(size_t alignment, size_t count, size_t size) {
#if defined(PSYCLE_USE_SSE) && defined(DIVERSALIS__CPU__X86__SSE)
#if defined(_WIN32)
	return _aligned_malloc(size * count, alignment);
#elif _POSIX_C_SOURCE >= 200112L
	void* address;
	int err;
	
	err = posix_memalign(&address, alignment, size * count);
	if (err) {
		free(address);
	    address = 0;
	}
	return address;		
#else
	return 0;
#endif
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
