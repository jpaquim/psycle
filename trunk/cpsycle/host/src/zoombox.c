// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "zoombox.h"
#include <portable.h>

static void zoombox_onzoomin(ZoomBox*, psy_ui_Component* sender);
static void zoombox_onzoomout(ZoomBox*, psy_ui_Component* sender);
static void zoombox_updatelabel(ZoomBox*);

void zoombox_init(ZoomBox* self, psy_ui_Component* parent)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_resize(&self->component, 0, 20);
	ui_button_init(&self->zoomout, &self->component);
	ui_button_settext(&self->zoomout, "-");
	ui_label_init(&self->label, &self->component);
	ui_label_settext(&self->label, "100");
	ui_button_init(&self->zoomin, &self->component);
	ui_button_settext(&self->zoomin, "+");	
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
	psy_signal_init(&self->signal_changed);
	self->zoomrate = 100;
	self->zoomstep = 25;
	psy_signal_connect(&self->zoomin.signal_clicked, self, zoombox_onzoomin);
	psy_signal_connect(&self->zoomout.signal_clicked, self, zoombox_onzoomout);
}

void zoombox_ondestroy(ZoomBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
}

void zoombox_onzoomin(ZoomBox* self, psy_ui_Component* sender)
{		
	self->zoomrate += self->zoomstep;	
	if (self->zoomrate < 0) {
		self->zoomrate = 0;
	}
	zoombox_updatelabel(self);
	psy_signal_emit(&self->signal_changed, self, 0, self->zoomrate);
}

void zoombox_onzoomout(ZoomBox* self, psy_ui_Component* sender)
{
	self->zoomrate -= self->zoomstep;	
	if (self->zoomrate > 1000) {
		self->zoomrate = 900;
	}
	zoombox_updatelabel(self);
	psy_signal_emit(&self->signal_changed, self, 0, self->zoomrate);
}

double zoombox_rate(ZoomBox* self)
{
	return self->zoomrate / (double) 100;
}

void zoombox_updatelabel(ZoomBox* self)
{
	char text[40];

	psy_snprintf(text, 40, "%d", self->zoomrate);
	ui_label_settext(&self->label, text);
}
