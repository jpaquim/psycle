/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "masterui.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void masterui_ondraw(MasterUi*, psy_ui_Graphics*);
static void masterui_onmousedoubleclick(MasterUi*, psy_ui_MouseEvent*);
static void masterui_drawhighlight(MasterUi*, psy_ui_Graphics*);
static void masterui_move(MasterUi*, psy_ui_Point topleft);

/* vtable */
static psy_ui_ComponentVtable masterui_vtable;
static psy_ui_ComponentVtable masterui_super_vtable;
static bool masterui_vtable_initialized = FALSE;

static void masterui_vtable_init(MasterUi* self)
{
	assert(self);

	if (!masterui_vtable_initialized) {
		masterui_vtable = *(self->component.vtable);
		masterui_super_vtable = masterui_vtable;		
		masterui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			masterui_ondraw;
		masterui_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			masterui_onmousedoubleclick;
		masterui_vtable.move =
			(psy_ui_fp_component_move)
			masterui_move;		
		masterui_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &masterui_vtable);	
}

/* implementation */
void masterui_init(MasterUi* self, psy_ui_Component* parent,
	ParamViews* paramviews, Workspace* workspace)
{
	assert(self);
	assert(parent);
	assert(workspace);
	assert(workspace->song);	

	psy_ui_component_init(&self->component, parent, NULL);	
	masterui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_MASTER);
	self->paramviews = paramviews;
	self->machine = psy_audio_machines_master(&workspace->song->machines);
	assert(self->machine);
	self->preventmachinepos = FALSE;
}

void masterui_move(MasterUi* self, psy_ui_Point topleft)
{
	assert(self);

	masterui_super_vtable.move(&self->component, topleft);
	if (!self->preventmachinepos) {		
		psy_audio_machine_setposition(self->machine,
			psy_ui_value_px(&topleft.x, NULL, NULL),
			psy_ui_value_px(&topleft.y, NULL, NULL));
	}
}

void masterui_ondraw(MasterUi* self, psy_ui_Graphics* g)
{
	psy_ui_Style* style;

	assert(self);

	style = psy_ui_style(STYLE_MV_MASTER);
	if (style && psy_ui_bitmap_empty(&style->background.bitmap)) {
		psy_ui_RealRectangle r;

		r = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(5.0, 5.0), style->background.size);
		psy_ui_textoutrectangle(g,
			psy_ui_realpoint_make(5.0, 5.0), psy_ui_ETO_CLIPPED, r, "Master",
			psy_strlen("Master"));
	}	
}

void masterui_onmousedoubleclick(MasterUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1 && self->paramviews) {
		paramviews_show(self->paramviews, psy_audio_machine_slot(self->machine));		
	}
	psy_ui_mouseevent_stop_propagation(ev);
}
