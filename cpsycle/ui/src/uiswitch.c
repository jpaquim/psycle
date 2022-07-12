/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiswitch.h"
/* std */
#include <stdlib.h>

/* prototypes */
static void psy_ui_switch_on_destroyed(psy_ui_Switch*);
static void psy_ui_switch_on_draw(psy_ui_Switch*, psy_ui_Graphics*);
static void psy_ui_switch_on_mouse_down(psy_ui_Switch*, psy_ui_MouseEvent*);
static void psy_ui_switch_on_property_changed(psy_ui_Switch*, psy_Property* sender);
static void psy_ui_switch_before_property_destroyed(psy_ui_Switch*, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static int vtable_initialized = FALSE;

static void vtable_init(psy_ui_Switch* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			psy_ui_switch_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_switch_on_draw;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_switch_on_mouse_down;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void psy_ui_switch_init(psy_ui_Switch* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_aligner(&self->component, NULL);
	psy_signal_init(&self->signal_clicked);
	self->state = FALSE;
	self->property = NULL;
	psy_ui_component_set_style_types(psy_ui_switch_base(self),
		psy_ui_STYLE_SWITCH, psy_ui_STYLE_SWITCH_HOVER,
		psy_ui_STYLE_SWITCH_SELECT, psy_INDEX_INVALID);
	psy_ui_component_set_preferred_size(psy_ui_switch_base(self),
		psy_ui_size_make_em(4.0, 1.5));
}

void psy_ui_switch_init_exchange(psy_ui_Switch* self, psy_ui_Component* parent,
	psy_Property* property)
{
	psy_ui_switch_init(self, parent);
	psy_ui_switch_data_exchange(self, property);
}

void psy_ui_switch_on_destroyed(psy_ui_Switch* self)
{	
	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
	psy_signal_dispose(&self->signal_clicked);	
}

psy_ui_Switch* psy_ui_switch_alloc(void)
{
	return (psy_ui_Switch*)malloc(sizeof(psy_ui_Switch));
}

psy_ui_Switch* psy_ui_switch_allocinit(psy_ui_Component* parent)
{
	psy_ui_Switch* rv;

	rv = psy_ui_switch_alloc();
	if (rv) {
		psy_ui_switch_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);		
	}
	return rv;
}

psy_ui_Switch* psy_ui_switch_allocinit_exchange(psy_ui_Component* parent,
	psy_Property* property)
{
	psy_ui_Switch* rv;

	rv = psy_ui_switch_alloc();
	if (rv) {
		psy_ui_switch_init_exchange(rv, parent, property);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_switch_on_property_changed(psy_ui_Switch* self, psy_Property* sender)
{
	bool checked;

	assert(self);

	checked = self->state;
	if (psy_property_is_choice_item(sender)) {
		checked = (psy_property_at_choice(psy_property_parent(sender))
			== sender);
	} else {
		checked = psy_property_item_bool(sender);
	}
	if (checked) {
		psy_ui_switch_check(self);
	} else {
		psy_ui_switch_uncheck(self);
	}
}

void psy_ui_switch_before_property_destroyed(psy_ui_Switch* self, psy_Property* sender)
{
	self->property = NULL;
}

void psy_ui_switch_on_draw(psy_ui_Switch* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	
	psy_ui_RealSize switchsize;
	psy_ui_RealSize knobsize;
	double centery;
	psy_ui_RealRectangle switchrect;
	psy_ui_RealRectangle knobrect;
	psy_ui_RealSize corner;	
		
	size = psy_ui_component_scroll_size_px(psy_ui_switch_base(self));
	tm = psy_ui_component_textmetric(psy_ui_switch_base(self));
	switchsize = psy_ui_realsize_make(
		floor(5.0 * tm->tmAveCharWidth), 1.0 * tm->tmHeight);
	centery = floor((size.height - switchsize.height) / 2.0);
	switchrect = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, centery),
		switchsize);
	knobsize = psy_ui_realsize_make(
		floor(3.0 * tm->tmAveCharWidth), 1.0 * tm->tmHeight);
	centery = floor((size.height - knobsize.height) / 2.0);
	knobrect = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, centery),
		knobsize);
	corner = psy_ui_realsize_make(6.0, 6.0);
	psy_ui_realrectangle_expand(&knobrect, -2.0, -1.0, -2.0, -1.0);	
	psy_ui_drawroundrectangle(g, switchrect, corner);
	if (self->state == FALSE) {		
		psy_ui_drawsolidroundrectangle(g, knobrect, corner,
			psy_ui_component_colour(psy_ui_switch_base(self)));
	} else {
		psy_ui_realrectangle_set_topleft(&knobrect,
			psy_ui_realpoint_make(switchsize.width - knobsize.width, knobrect.top));
		psy_ui_drawsolidroundrectangle(g, knobrect, corner,
			psy_ui_component_colour(psy_ui_switch_base(self)));
	}
}

void psy_ui_switch_on_mouse_down(psy_ui_Switch* self, psy_ui_MouseEvent* ev)
{
	self->state = !self->state;
	if (self->property) {
		if (psy_property_is_choice_item(self->property)) {
			intptr_t index;

			if (!psy_property_parent(self->property)) {
				return;
			}
			index = psy_property_index(self->property);
			psy_property_set_item_int(psy_property_parent(self->property), index);
		} else {
			psy_property_set_item_bool(self->property, self->state);
		}
	}
	psy_signal_emit(&self->signal_clicked, self, psy_SIGNAL_NOPARAMS);
}

void psy_ui_switch_data_exchange(psy_ui_Switch* self, psy_Property* property)
{
	self->property = property;
	if (self->property) {
		psy_ui_switch_on_property_changed(self, self->property);
		psy_property_connect(self->property, self, 
			psy_ui_switch_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			psy_ui_switch_before_property_destroyed);
	}
}

void psy_ui_switch_check(psy_ui_Switch* self)
{
	self->state = TRUE;
	psy_ui_component_add_style_state(psy_ui_switch_base(self),
		psy_ui_STYLESTATE_SELECT);	
}

void psy_ui_switch_uncheck(psy_ui_Switch* self)
{
	self->state = FALSE;
	psy_ui_component_remove_style_state(psy_ui_switch_base(self),
		psy_ui_STYLESTATE_SELECT);	
}

bool psy_ui_switch_checked(const psy_ui_Switch* self)
{
	return self->state != FALSE;
}
