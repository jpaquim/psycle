/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(EFFECTUI_H)
#define EFFECTUI_H

/* host */
#include "machineui.h"
#include "paramviews.h"

#ifdef __cplusplus
extern "C" {
#endif

/* EffectUi */
typedef struct EffectUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component mute;
	psy_ui_Component bypass;
	EditnameUi editname;
	PanUi pan;
	VuUi vu;
	bool prevent_machine_pos;
	uintptr_t counter;
	/* references */
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	ParamViews* paramviews;	
} EffectUi;

void effectui_init(EffectUi*, psy_ui_Component* parent, uintptr_t slot,
	ParamViews*, psy_audio_Machines*);
	
EffectUi* effectui_alloc(void);
EffectUi* effectui_alloc_init(psy_ui_Component* parent, uintptr_t mac_id,
	ParamViews*, psy_audio_Machines*);
	
INLINE void effectui_prevent_machine_pos(EffectUi* self)
{
	assert(self);
	
	self->prevent_machine_pos = TRUE;	
}

INLINE void effectui_enable_machine_pos(EffectUi* self)
{
	assert(self);
	
	self->prevent_machine_pos = FALSE;	
}	
	
	
INLINE psy_ui_Component* effectui_base(EffectUi* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* EFFECTUI_H */
