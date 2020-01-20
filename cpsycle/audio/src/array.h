// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_ARRAY_H)
#define PSY_AUDIO_ARRAY_H

#include "../../detail/stdint.h"

// array wrapper (shared : float*,
//                notshared: universalis::os::aligned_memory_alloc(16,..)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_Array {
    float* ptr_, *base_;
    uintptr_t cap_;
    uintptr_t len_;
    uintptr_t baselen_;
    int shared_;    
} psy_audio_Array;

void psy_audio_array_init(psy_audio_Array*);
void psy_audio_array_init_shared(psy_audio_Array*, float* ptr, int len);
void psy_audio_array_dispose(psy_audio_Array*);
int psy_audio_array_len(psy_audio_Array*);
void psy_audio_array_set(psy_audio_Array*, uintptr_t index, float value);
float psy_audio_array_at(psy_audio_Array*, uintptr_t index);
float* psy_audio_array_data(psy_audio_Array*);
void psy_audio_array_clear(psy_audio_Array*);
void psy_audio_array_fill(psy_audio_Array*, float val);
void psy_audio_array_fillfrom(psy_audio_Array*, float val, uintptr_t pos);
void psy_audio_array_mul_constant(psy_audio_Array*, float factor);
void psy_audio_array_mul_array(psy_audio_Array*, psy_audio_Array* other);
void psy_audio_array_mix(psy_audio_Array*, psy_audio_Array* other,
	float factor);
void psy_audio_array_add(psy_audio_Array*, psy_audio_Array* other);
void psy_audio_array_addconstant(psy_audio_Array*, float value);
void psy_audio_array_sub(psy_audio_Array*, psy_audio_Array* other);
void psy_audio_array_rsum(psy_audio_Array*, double lv);
int psy_audio_array_copyfrom(psy_audio_Array*, psy_audio_Array* other,
	uintptr_t pos);
void psy_audio_array_sqrt(psy_audio_Array*);
void psy_audio_array_sin(psy_audio_Array*);
void psy_audio_array_cos(psy_audio_Array*);
void psy_audio_array_tan(psy_audio_Array*);
void psy_audio_array_ceil(psy_audio_Array*);
void psy_audio_array_floor(psy_audio_Array*);
void psy_audio_array_fabs(psy_audio_Array*);

#ifdef __cplusplus
}
#endif

#endif
