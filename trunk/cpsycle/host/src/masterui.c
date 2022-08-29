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
static void masterui_on_draw(MasterUi*, psy_ui_Graphics*);
static void masterui_on_mouse_double_click(MasterUi*, psy_ui_MouseEvent*);
static void masterui_move(MasterUi*, psy_ui_Point top_left);
static void masterui_draw_label(MasterUi*, psy_ui_Graphics*);

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
			masterui_on_draw;
		masterui_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			masterui_on_mouse_double_click;
		masterui_vtable.move =
			(psy_ui_fp_component_move)
			masterui_move;		
		masterui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &masterui_vtable);	
}

/* implementation */
void masterui_init(MasterUi* self, psy_ui_Component* parent,
	ParamViews* paramviews, psy_audio_Machines* machines)
{
	assert(self);
	assert(machines);	

	psy_ui_component_init(&self->component, parent, NULL);	
	masterui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_MASTER);
	self->paramviews = paramviews;
	self->machine = psy_audio_machines_master(machines);
	assert(self->machine);
	self->prevent_machine_pos = FALSE;	
}

MasterUi* masterui_alloc(void)
{
	return (MasterUi*)malloc(sizeof(MasterUi));
}

MasterUi* masterui_alloc_init(psy_ui_Component* parent, ParamViews* paramviews,
	psy_audio_Machines* machines)
{
	MasterUi* rv;
	
	rv = masterui_alloc();
	if (rv) {
		masterui_init(rv, parent, paramviews, machines);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void masterui_move(MasterUi* self, psy_ui_Point top_left)
{
	assert(self);

	masterui_super_vtable.move(&self->component, top_left);
	if (!self->prevent_machine_pos) {		
		psy_audio_machine_setposition(self->machine,
			psy_ui_value_px(&top_left.x, NULL, NULL),
			psy_ui_value_px(&top_left.y, NULL, NULL));
	}
}

void masterui_on_draw(MasterUi* self, psy_ui_Graphics* g)
{
	psy_ui_Style* style;

	assert(self);

	style = psy_ui_style(STYLE_MV_MASTER);
	if (psy_ui_bitmap_empty(&style->background.bitmap)) {
		masterui_draw_label(self, g);
	}	
}

void masterui_draw_label(MasterUi* self, psy_ui_Graphics* g)
{	
	psy_ui_RealPoint cp;	
	psy_ui_Style* style;

	assert(self);	

	cp = psy_ui_realpoint_make(5.0, 5.0);	
	style = psy_ui_style(STYLE_MV_MASTER);	
	psy_ui_textoutrectangle(g, cp, psy_ui_ETO_CLIPPED,
		psy_ui_realrectangle_make(cp, style->background.size),
		"Master", psy_strlen("Master"));
}

void masterui_on_mouse_double_click(MasterUi* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	if (psy_ui_mouseevent_button(ev) == 1 && self->paramviews) {
		paramviews_show(self->paramviews, psy_audio_machine_slot(
			self->machine));		
	}
	psy_ui_mouseevent_stop_propagation(ev);
}
