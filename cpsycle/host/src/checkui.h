/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CHECKUI_H)
#define CHECKUI_H

/* host */
#include "paramtweak.h"
/* ui */
#include <uicomponent.h>

/*
** CheckUi
**
** Check Button to display a MachineParameter (MPF_CHECK)
*/

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

/* CheckUi */
typedef struct CheckUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	ParamTweak paramtweak;
	uintptr_t paramidx;
	/* references */
	struct ParamSkin* skin;		
	struct psy_audio_Machine* machine;	
	struct psy_audio_MachineParam* param;
} CheckUi;

void checkui_init(CheckUi*, psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*, struct ParamSkin*);

CheckUi* checkui_alloc(void);
CheckUi* checkui_allocinit(psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*, struct ParamSkin*);

INLINE psy_ui_Component* checkui_base(CheckUi* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CHECKUI_H */
