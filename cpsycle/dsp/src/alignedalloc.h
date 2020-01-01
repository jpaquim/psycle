// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_DSP_ALIGNEDALLOC_H)
#define PSY_DSP_ALIGNEDALLOC_H

#include <stddef.h>

void* psy_dsp_aligned_memory_alloc(size_t alignment, size_t count,
	size_t size);
void psy_dsp_aligned_memory_dealloc(void* address);


#endif
