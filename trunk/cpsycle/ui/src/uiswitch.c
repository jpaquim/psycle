// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiswitch.h"

#include <stdlib.h>
#include <string.h>

static void ondestroy(psy_ui_Switch*);
static void ondraw(psy_ui_Switch*, psy_ui_Graphics*);
static void onmousedown(psy_ui_Switch*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Switch*);
static void onmouseleave(psy_ui_Switch*);
static void onpreferredsize(psy_ui_Switch*, psy_ui_Size* limit, psy_ui_Size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = FALSE;

static void vtable_init(psy_ui_Switch* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			ondestroy;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			onpreferredsize;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			ondraw;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			onmousedown;
		vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			onmouseenter;
		vtable.onmouseleave =
			(psy_ui_fp_component_onmouseleave)
			onmouseleave;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

void psy_ui_switch_init(psy_ui_Switch* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{		
	psy_ui_component_init(&self->component, parent, view);
	vtable_init(self);			
	psy_signal_init(&self->signal_clicked);	
	self->state = FALSE;	
}

void ondestroy(psy_ui_Switch* self)
{	
	psy_signal_dispose(&self->signal_clicked);	
}

psy_ui_Switch* psy_ui_switch_alloc(void)
{
	return (psy_ui_Switch*)malloc(sizeof(psy_ui_Switch));
}

psy_ui_Switch* psy_ui_switch_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_Switch* rv;

	rv = psy_ui_switch_alloc();
	if (rv) {
		psy_ui_switch_init(rv, parent, view);
		psy_ui_component_deallocateafterdestroyed(&rv->component);		
	}
	return rv;
}

void ondraw(psy_ui_Switch* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	
	psy_ui_RealSize switchsize;
	psy_ui_RealSize knobsize;
	double centery;
	psy_ui_RealRectangle switchrect;
	psy_ui_RealRectangle knobrect;
	psy_ui_RealSize corner;	
		
	size = psy_ui_component_offsetsize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
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
	psy_ui_setcolour(g, psy_ui_colour_make(0x00555555));
	psy_ui_drawroundrectangle(g, switchrect, corner);
	if (self->state == FALSE) {		
		psy_ui_drawsolidroundrectangle(g, knobrect, corner,
			psy_ui_colour_make(0x00555555));
	} else {
		psy_ui_realrectangle_settopleft(&knobrect,
			psy_ui_realpoint_make(switchsize.width - knobsize.width, knobrect.top));
		psy_ui_drawsolidroundrectangle(g, knobrect, corner,
			psy_ui_colour_make(0x00CACACA));
	}
}

void onpreferredsize(psy_ui_Switch* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	if (rv) {				
		rv->height = psy_ui_value_makeeh(1.5);
		rv->width = psy_ui_value_makeew(4);
	}
}

void onmousedown(psy_ui_Switch* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Switch* self)
{
	// psy_ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Switch* self)
{
	// psy_ui_component_invalidate(&self->component);
}

void psy_ui_switch_check(psy_ui_Switch* self)
{
	self->state = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_switch_uncheck(psy_ui_Switch* self)
{
	self->state = FALSE;
	psy_ui_component_invalidate(&self->component);
}

bool psy_ui_switch_checked(const psy_ui_Switch* self)
{
	return self->state != FALSE;
}

/*void propertiesrenderer_drawcheckbox(PropertiesRenderer* self, psy_Property* property,
	uintptr_t column)
{
	psy_ui_RealRectangle r;
	int checked = 0;
	const psy_ui_TextMetric* tm;
	psy_ui_Size size;
	psy_ui_Size cornersize;
	psy_ui_Size knobsize;

	tm = psy_ui_component_textmetric(&self->component);
	size.width = psy_ui_value_makeew(4);
	size.height = psy_ui_value_makeeh(1);
	knobsize.width = psy_ui_value_makeew(2);
	knobsize.height = psy_ui_value_makeeh(0.7);
	cornersize.width = psy_ui_value_makeew(0.6);
	cornersize.height = psy_ui_value_makeeh(0.6);
	r.left = propertiesrenderer_columnstart(self, column);
	r.top = self->cpy + (self->lineheight -
		psy_ui_value_px(&size.height, tm)) / 2;
	r.right = r.left + (int)(tm->tmAveCharWidth * 4.8);
	r.bottom = r.top + psy_ui_value_px(&size.height, tm);
	psy_ui_setcolour(self->g, psy_ui_colour_make(0x00555555));
	psy_ui_drawroundrectangle(self->g, r, cornersize);
	if (psy_property_ischoiceitem(property)) {
		if (psy_property_parent(property)) {
			checked = psy_property_at_choice(psy_property_parent(property)) ==
				property;
		} else {
			checked = FALSE;
		}
	} else {
		checked = psy_property_item_int(property) != 0;
	}
	if (!checked) {
		r.left = propertiesrenderer_columnstart(self, column) + (int)(tm->tmAveCharWidth * 0.4);
		r.top = self->cpy + (self->lineheight -
			psy_ui_value_px(&knobsize.height, tm)) / 2;
		r.right = r.left + (int)(tm->tmAveCharWidth * 2.5);
		r.bottom = r.top + psy_ui_value_px(&knobsize.height, tm);
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize,
			psy_ui_colour_make(0x00555555));
	} else {
		r.left = propertiesrenderer_columnstart(self, column) + tm->tmAveCharWidth * 2;
		r.top = self->cpy + (self->lineheight -
			psy_ui_value_px(&knobsize.height, tm)) / 2;
		r.right = r.left + (int)(tm->tmAveCharWidth * 2.5);
		r.bottom = r.top + psy_ui_value_px(&knobsize.height, tm);
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize,
			psy_ui_colour_make(0x00CACACA));
	}	
}*/
