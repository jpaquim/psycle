/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LADSPAPARAM_H
#define psy_audio_LADSPAPARAM_H

/* local */
#include "custommachine.h"
#include "ladspa.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_LadspaParam {
	psy_audio_CustomMachineParam custommachineparam;
	LADSPA_PortDescriptor descriptor;
	LADSPA_PortRangeHint hint;
	const char *port_name;
	uintptr_t index;
	uintptr_t port_index;
	LADSPA_Data value_;
	LADSPA_Data minval_;
	LADSPA_Data maxval_;
} psy_audio_LadspaParam;

void psy_audio_ladspaparam_init(psy_audio_LadspaParam*,
	LADSPA_PortDescriptor descriptor,
	LADSPA_PortRangeHint hint,
	const char *newname,
	uintptr_t index,
	uintptr_t port_index);
void psy_audio_ladspaparam_dispose(psy_audio_LadspaParam*);

psy_audio_LadspaParam* psy_audio_ladspaparam_alloc(void);
psy_audio_LadspaParam* psy_audio_ladspaparam_allocinit(
	LADSPA_PortDescriptor descriptor,
	LADSPA_PortRangeHint hint,
	const char *newname,	
	uintptr_t index,
	uintptr_t port_index);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LADSPAPARAM_H */
