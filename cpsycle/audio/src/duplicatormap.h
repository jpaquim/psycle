// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_DUPLICATORMAP_H
#define psy_audio_DUPLICATORMAP_H

#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int machine;
	int offset;
	int lowkey;
	int highkey;
	psy_Table allochans;
} psy_audio_DuplicatorOutput;

void psy_audio_duplicatoroutput_setall(psy_audio_DuplicatorOutput*,
	int machine, int offset, int lowkey, int highkey);

typedef struct {
	psy_Table outputs;
	psy_Table unavail;
	uintptr_t maxtracks;
} psy_audio_DuplicatorMap;

void psy_audio_duplicatormap_init(psy_audio_DuplicatorMap*,
	uintptr_t numoutputs, uintptr_t maxtracks);
void psy_audio_duplicatormap_dispose(psy_audio_DuplicatorMap*);
void psy_audio_duplicatormap_clear(psy_audio_DuplicatorMap*);
int psy_audio_duplicatormap_channel(psy_audio_DuplicatorMap*,
	uintptr_t patternchannel, psy_audio_DuplicatorOutput*);
void psy_audio_duplicatormap_release(psy_audio_DuplicatorMap*,
	uintptr_t patternchannel, uintptr_t duplicatorchannel,
	psy_audio_DuplicatorOutput*);
psy_TableIterator psy_audio_duplicatormap_begin(psy_audio_DuplicatorMap*);
psy_audio_DuplicatorOutput* psy_audio_duplicatormap_output(
	psy_audio_DuplicatorMap*, int output);
uintptr_t psy_audio_duplicatormap_numoutputs(psy_audio_DuplicatorMap*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_DUPLICATORMAP_H */
