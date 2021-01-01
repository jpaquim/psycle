// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WAVEOSCTABLES_H
#define psy_audio_WAVEOSCTABLES_H

// local
#include "instrument.h"
#include "samples.h"
// container
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_audio_WAVESHAPE_SIN = 1,
	psy_audio_WAVESHAPE_SAW = 2,
	psy_audio_WAVESHAPE_SQR = 3,
	psy_audio_WAVESHAPE_TRI = 4,
	psy_audio_WAVESHAPE_PWM = 5,
	psy_audio_WAVESHAPE_RND = 6
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
psy_audio_Instrument* psy_audio_waveosctables_table(psy_audio_WaveOscTables*,
	psy_audio_WaveShape);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WAVEOSCTABLES_H */
