// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_ARRAY_H)
#define PSY_AUDIO_ARRAY_H

// array wrapper (shared : float*,
//                notshared: universalis::os::aligned_memory_alloc(16,..)

typedef struct psy_audio_Array {
    float* ptr_, *base_;
    int cap_;
    int len_;
    int baselen_;
    int shared_;
    int can_aligned_;
} psy_audio_Array;

void psy_audio_array_init(psy_audio_Array*);
void psy_audio_array_init_shared(psy_audio_Array*, float* ptr, int len);
void psy_audio_array_dispose(psy_audio_Array*);
int psy_audio_array_len(psy_audio_Array*);
float* psy_audio_array_data(psy_audio_Array*);

#endif
