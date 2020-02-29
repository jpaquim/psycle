// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WAVEOSCTABLES_H
#define psy_audio_WAVEOSCTABLES_H

#include <hashtbl.h>
#include "samples.h"
#include "instrument.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SIN = 1,
	SAW = 2,
	SQR = 3,
	TRI = 4,
	PWM = 5,
	RND = 6
} psy_audio_WaveShape;

typedef struct psy_audio_WaveOscTables {
	psy_audio_Samples container;
	psy_audio_Instrument sin_tbl;
	psy_audio_Instrument saw_tbl;
	psy_audio_Instrument sqr_tbl;
	psy_audio_Instrument tri_tbl;
	psy_audio_Instrument rnd_tbl;
} psy_audio_WaveOscTables;

void psy_audio_waveosctables_init(psy_audio_WaveOscTables*);
void psy_audio_waveosctables_dispose(psy_audio_WaveOscTables*);
void psy_audio_waveosctables_clear(psy_audio_WaveOscTables*);
void psy_audio_waveosctables_makeall(psy_audio_WaveOscTables*, uintptr_t sr);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WAVEOSCTABLES_H */
