/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LADSPAINTERFACE_H
#define psy_audio_LADSPAINTERFACE_H

#include "../../detail/psydef.h"
/* local */
#include "machinedefs.h"
#include "ladspa.h"

/* dsp */
#include "../../dsp/src/dsptypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_LadspaInterface {
	const LADSPA_Descriptor* descriptor;
	LADSPA_Handle handle;	
} psy_audio_LadspaInterface;

void psy_audio_ladspainterface_init(psy_audio_LadspaInterface*,
	const LADSPA_Descriptor* descriptor,
	LADSPA_Handle handle);
	
void psy_audio_ladspainterface_activate(psy_audio_LadspaInterface*);
void psy_audio_ladspainterface_deactivate(psy_audio_LadspaInterface*);
void psy_audio_ladspainterface_cleanup(psy_audio_LadspaInterface*);
void psy_audio_ladspainterface_run(psy_audio_LadspaInterface*,
	uint32_t num_samples);
	
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LADSPAINTERFACE_H */
