/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(KNOBUI_H)
#define KNOBUI_H

/* host */
#include "paramtweak.h"
/* ui */
#include <uicomponent.h>

/*
** KnobUi
**
** Knob ui to tweak a MachineParameter (MPF_STATE)
*/

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

typedef struct KnobUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	ParamTweak paramtweak;
	uintptr_t paramidx;
	/* references */	
	struct psy_audio_Machine* machine;
	struct psy_audio_MachineParam* param;
} KnobUi;

void knobui_init(KnobUi*, psy_ui_Component* parent,	
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

KnobUi* knobui_alloc(void);
KnobUi* knobui_allocinit(psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

INLINE psy_ui_Component* knobui_base(KnobUi* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* KNOBUI_H */
