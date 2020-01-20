// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENTS_H
#define psy_audio_INSTRUMENTS_H

#include "instrument.h"
#include <hashtbl.h>
#include <signal.h>
#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NOINSTRUMENT_INDEX UINTPTR_MAX

typedef struct psy_audio_Instruments {
	psy_Table container;
	uintptr_t slot;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_Signal signal_slotchange;
} psy_audio_Instruments;

void instruments_init(psy_audio_Instruments*);
void instruments_dispose(psy_audio_Instruments*);
void instruments_insert(psy_audio_Instruments*, psy_audio_Instrument*, uintptr_t slot);
void instruments_remove(psy_audio_Instruments*, uintptr_t slot);
void instruments_changeslot(psy_audio_Instruments*, uintptr_t slot);
uintptr_t instruments_slot(psy_audio_Instruments*);
psy_audio_Instrument* instruments_at(psy_audio_Instruments*, uintptr_t slot);
uintptr_t instruments_size(psy_audio_Instruments*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENTS_H */
