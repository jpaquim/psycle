/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MASTERUI_H)
#define MASTERUI_H

/* host */
#include "machineui.h" /* vst view */
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MasterUi */
typedef struct MasterUi {
	/* inherits */
	psy_ui_Component component;
	bool prevent_machine_pos;
	/* internal */	
	psy_audio_Machine* machine;
	ParamViews* paramviews;
} MasterUi;

void masterui_init(MasterUi*, psy_ui_Component* parent, ParamViews*,
	psy_audio_Machines*);

MasterUi* masterui_alloc(void);
MasterUi* masterui_alloc_init(psy_ui_Component* parent, ParamViews*,
	psy_audio_Machines*);

INLINE void masterui_prevent_machine_pos(MasterUi* self)
{
	assert(self);
	
	self->prevent_machine_pos = TRUE;	
}

INLINE void masterui_enable_machine_pos(MasterUi* self)
{
	assert(self);
	
	self->prevent_machine_pos = FALSE;	
}

INLINE psy_ui_Component* masterui_base(MasterUi* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MASTERUI_H */
