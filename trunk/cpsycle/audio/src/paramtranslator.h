/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PARAMTRANSLATOR_H
#define psy_audio_PARAMTRANSLATOR_H

/* container */
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_ParamTranslator {
    psy_Table container;
} psy_audio_ParamTranslator;

void psy_audio_paramtranslator_init(psy_audio_ParamTranslator*);
void psy_audio_paramtranslator_dispose(psy_audio_ParamTranslator*);

psy_audio_ParamTranslator* psy_audio_paramtranslator_alloc(void);
psy_audio_ParamTranslator* psy_audio_paramtranslator_allocinit(void);

void psy_audio_paramtranslator_clear(psy_audio_ParamTranslator*);
void psy_audio_paramtranslator_set_virtual_index(
	psy_audio_ParamTranslator*, uintptr_t virtual_index,
	uintptr_t machine_index);
uintptr_t psy_audio_paramtranslator_translate(
	const psy_audio_ParamTranslator*, uintptr_t virtual_index);
uintptr_t psy_audio_paramtranslator_virtual_index(
	const psy_audio_ParamTranslator*, uintptr_t machine_index);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PARAMTRANSLATOR */
