/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VALUEUI_H)
#define VALUEUI_H

/* host */
#include "paramtweak.h"
/* ui */
#include <uilabel.h>

/*
** ValueUi
**
** Value ui to tweak a MachineParameter (MPF_STATE)
*/

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

typedef struct ValueUi {
	/* inherits */
	psy_ui_Label label;
	/* internal */	
	ParamTweak paramtweak;
	uintptr_t paramidx;
	/* references */	
	struct psy_audio_Machine* machine;
	struct psy_audio_MachineParam* param;
} ValueUi;

void valueui_init(ValueUi*, psy_ui_Component* parent,	
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

ValueUi* valueui_alloc(void);
ValueUi* valueui_allocinit(psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

INLINE psy_ui_Component* valueui_base(ValueUi* self)
{
	assert(self);

	return &self->label.component;
}

#ifdef __cplusplus
}
#endif

#endif /* VALUEUI_H */
