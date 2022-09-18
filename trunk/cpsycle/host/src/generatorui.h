/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(GENERATORUI_H)
#define GENERATORUI_H

/* host */
#include "machineui.h"
#include "paramviews.h"
/* ui */
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GeneratorUi */
typedef struct GeneratorUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component mute;
	psy_ui_Component solo;
	EditnameUi editname;
	PanUi pan;
	VuUi vu;
	bool prevent_machine_pos;
	uintptr_t counter;
	/* references */
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	ParamViews* paramviews;
} GeneratorUi;

void generatorui_init(GeneratorUi*, psy_ui_Component* parent, uintptr_t slot,
	ParamViews* paramviews, psy_audio_Machines*);
	
GeneratorUi* generatorui_alloc(void);
GeneratorUi* generatorui_alloc_init(psy_ui_Component* parent, uintptr_t mac_id,
	ParamViews*, psy_audio_Machines*);	
	
INLINE void generatorui_prevent_machine_pos(GeneratorUi* self)
{
	assert(self);
	
	self->prevent_machine_pos = TRUE;	
}

INLINE void generator_enable_machine_pos(GeneratorUi* self)
{
	assert(self);
	
	self->prevent_machine_pos = FALSE;	
}	
	
INLINE psy_ui_Component* generatorui_base(GeneratorUi* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* GENERATORUI_H */
