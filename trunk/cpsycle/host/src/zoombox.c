// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "zoombox.h"
// host
#include "styles.h"
// platform
#include "../../detail/portable.h"

#define sgn(x) ((x > 0) ? 1 : ((x < 0) ? -1 : 0))

// prototypes
static void zoombox_ondestroy(ZoomBox*);
static void zoombox_onzoomin(ZoomBox*, psy_ui_Component* sender);
static void zoombox_onzoomout(ZoomBox*, psy_ui_Component* sender);
static void zoombox_updatelabel(ZoomBox*);
static void zoombox_onmousewheel(ZoomBox*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(ZoomBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			zoombox_ondestroy;
		vtable.onmousewheel = (psy_ui_fp_component_onmouseevent)
			zoombox_onmousewheel;		
		vtable_initialized = TRUE;
	}
	return &vtable;
}

// implementation
void zoombox_init(ZoomBox* self, psy_ui_Component* parent)
{
	assert(self);

	// init base
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(zoombox_base(self), vtable_init(self));
	psy_ui_component_setstyletype(&self->component, STYLE_ZOOMBOX);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	// init ui elements
	psy_ui_button_init_connect(&self->zoomout, zoombox_base(self), NULL,
		self, zoombox_onzoomout);	
	psy_ui_button_preventtranslation(&self->zoomout);
	psy_ui_button_settext(&self->zoomout, "-");	
	psy_ui_button_setcharnumber(&self->zoomout, 2);
	psy_ui_label_init(&self->label, zoombox_base(self), NULL);
	psy_ui_label_preventtranslation(&self->label);
	psy_ui_label_settext(&self->label, "100%");
	psy_ui_label_setcharnumber(&self->label, 6);
	psy_ui_button_init_connect(&self->zoomin, zoombox_base(self), NULL,
		self, zoombox_onzoomin);
	psy_ui_button_preventtranslation(&self->zoomin);
	psy_ui_button_settext(&self->zoomin, "+");	
	psy_ui_button_setcharnumber(&self->zoomin, 2);		
	psy_ui_component_setalign_children(zoombox_base(self), psy_ui_ALIGN_LEFT);		
	// set defaults
	self->zoomrate = 1.0;
	self->zoomstep = 0.1;
	self->minrate = 0.10;
	self->maxrate = 10.0;
	// init signal	
	psy_signal_init(&self->signal_changed);
}

void zoombox_init_connect(ZoomBox* self, psy_ui_Component* parent,
	void* context, void* fp)
{
	assert(self);

	zoombox_init(self, parent);
	psy_signal_connect(&self->signal_changed, context, fp);
}

void zoombox_ondestroy(ZoomBox* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void zoombox_onzoomin(ZoomBox* self, psy_ui_Component* sender)
{
	assert(self);

	zoombox_setrate(self,  self->zoomrate + self->zoomstep);
}

void zoombox_onzoomout(ZoomBox* self, psy_ui_Component* sender)
{
	assert(self);

	zoombox_setrate(self, self->zoomrate - self->zoomstep);
}

void zoombox_setrate(ZoomBox* self, double rate)
{	
	assert(self);
	
	rate = psy_max(self->minrate, rate);
	rate = psy_min(self->maxrate, rate);
	if (rate != self->zoomrate) {
		self->zoomrate = rate;
		psy_signal_emit(&self->signal_changed, self, 0, self->zoomrate);
		zoombox_updatelabel(self);
	}	
}

void zoombox_setstep(ZoomBox* self, double step)
{
	assert(self);

	self->zoomstep = step;
}

void zoombox_updatelabel(ZoomBox* self)
{
	char text[40];

	assert(self);

	psy_snprintf(text, 40, "%d%%", (int)(self->zoomrate * 100 + 0.5));
	psy_ui_label_settext(&self->label, text);
}

void zoombox_onmousewheel(ZoomBox* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (ev->delta != 0) {
		zoombox_setrate(self, self->zoomrate + sgn(ev->delta) * self->zoomstep);
	}
	psy_ui_mouseevent_preventdefault(ev);
}
