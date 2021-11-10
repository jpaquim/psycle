// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackbox.h"
// ui
#include <uiapp.h>
// platform
#include "../../detail/portable.h"

// TrackBox
static void trackbox_updatetrack(TrackBox*);
static void trackbox_onmute(TrackBox*, psy_ui_Button* sender);
static void trackbox_onsolo(TrackBox*, psy_ui_Button* sender);
static void trackbox_onclose(TrackBox*, psy_ui_Button* sender);
static void trackbox_onresizemousedown(TrackBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev);
static void trackbox_onresizemousemove(TrackBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev);
static void trackbox_onresizemouseup(TrackBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev);
// prototypes
static void trackbox_ondestroy(TrackBox*);
// vtable
static psy_ui_ComponentVtable trackbox_vtable;
static bool trackbox_vtable_initialized = FALSE;

static void trackbox_vtableinit_init(TrackBox* self)
{
	if (!trackbox_vtable_initialized) {
		trackbox_vtable = *(self->component.vtable);
		trackbox_vtable.ondestroy =
			(psy_ui_fp_component_event)
			trackbox_ondestroy;
		trackbox_vtable_initialized = TRUE;
	}
	self->component.vtable = &trackbox_vtable;
}

// implementation
void trackbox_init(TrackBox* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_Margin spacing;
	
	psy_ui_component_init(trackbox_base(self), parent, view);
	trackbox_vtableinit_init(self);
	psy_ui_margin_init_em(&spacing, 0.0, 0.0, 0.0, 1.0);
	psy_ui_component_setspacing(trackbox_base(self), spacing);
	psy_ui_component_init(&self->client, &self->component, view);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->client,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_setalignexpand(&self->client,
		psy_ui_HEXPAND);
	/* resize bar */
	psy_ui_component_init(&self->resize, &self->component, view);
	psy_ui_component_hide(&self->resize);
	psy_ui_component_setalign(&self->resize, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(&self->resize,
		psy_ui_size_make_px(0.0, 4));	
	psy_signal_connect(&self->resize.signal_mousedown, self,
		trackbox_onresizemousedown);
	psy_signal_connect(&self->resize.signal_mousemove, self,
		trackbox_onresizemousemove);
	psy_signal_connect(&self->resize.signal_mouseup, self,
		trackbox_onresizemouseup);
	self->doresize = FALSE;
	self->baseheight = 0.0;
	self->dragoffset = 0.0;
	/* track number */
	psy_ui_label_init(&self->track, &self->client, view);
	psy_ui_label_settextalignment(&self->track, psy_ui_ALIGNMENT_CENTER);
	psy_ui_label_preventtranslation(&self->track);		
	psy_ui_label_setcharnumber(&self->track, 5);
	psy_ui_button_init(&self->solo, &self->client, view);
	psy_ui_button_preventtranslation(&self->solo);
	psy_ui_button_settext(&self->solo, "S");	
	psy_ui_button_init(&self->mute, &self->client, view);
	psy_ui_button_preventtranslation(&self->mute);
	psy_ui_button_settext(&self->mute, "M");
	psy_ui_label_init(&self->desc, &self->client, view);
	psy_ui_label_preventtranslation(&self->desc);
	psy_ui_component_setalign(psy_ui_label_base(&self->desc),
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_init(&self->close, &self->client, view);
	psy_ui_button_preventtranslation(&self->close);
	psy_ui_button_settext(&self->close, "X");
	self->close.stoppropagation = FALSE;
	psy_ui_component_setalign(psy_ui_button_base(&self->close),
		psy_ui_ALIGN_RIGHT);
	psy_signal_init(&self->signal_mute);
	psy_signal_init(&self->signal_solo);
	psy_signal_init(&self->signal_close);
	psy_signal_init(&self->signal_resize);
	psy_signal_connect(&self->mute.signal_clicked, self, trackbox_onmute);
	psy_signal_connect(&self->solo.signal_clicked, self, trackbox_onsolo);
	psy_signal_connect(&self->close.signal_clicked, self, trackbox_onclose);	
	self->trackidx = psy_INDEX_INVALID;
	self->closeprevented = FALSE;
	trackbox_updatetrack(self);
}

void trackbox_ondestroy(TrackBox* self)
{
	psy_signal_dispose(&self->signal_mute);
	psy_signal_dispose(&self->signal_solo);
	psy_signal_dispose(&self->signal_close);
	psy_signal_dispose(&self->signal_resize);
}

TrackBox* trackbox_alloc(void)
{
	return (TrackBox*)malloc(sizeof(TrackBox));
}

TrackBox* trackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view)
{
	TrackBox* rv;

	rv = trackbox_alloc();
	if (rv) {
		trackbox_init(rv, parent, view);
		psy_ui_component_deallocateafterdestroyed(trackbox_base(rv));
	}
	return rv;
}

void trackbox_setdescription(TrackBox* self, const char* text)
{
	psy_ui_label_settext(&self->desc, text);
}

void trackbox_setindex(TrackBox* self, uintptr_t index)
{	
	self->trackidx = index;
	trackbox_updatetrack(self);
}

void trackbox_updatetrack(TrackBox* self)
{
	char text[64];

	if (self->trackidx == psy_INDEX_INVALID) {
		psy_snprintf(text, 64, "%s", "--");
	} else {
		psy_snprintf(text, 64, "%.2X", self->trackidx);
	}
	psy_ui_label_settext(&self->track, text);	
	if (self->closeprevented || self->trackidx == 0) {
		psy_ui_component_hide(psy_ui_button_base(&self->close));
	} else {
		psy_ui_component_show(psy_ui_button_base(&self->close));
	}
}

void trackbox_mute(TrackBox* self)
{
	psy_ui_button_highlight(&self->mute);
}

void trackbox_unmute(TrackBox* self)
{
	psy_ui_button_disablehighlight(&self->mute);
}

void trackbox_solo(TrackBox* self)
{
	psy_ui_button_highlight(&self->solo);
}

void trackbox_unsolo(TrackBox* self)
{
	psy_ui_button_disablehighlight(&self->solo);
}

void trackbox_preventclose(TrackBox* self)
{
	self->closeprevented = TRUE;
	if (self->trackidx == 0) {
		psy_ui_component_hide(psy_ui_button_base(&self->close));
	} else {
		psy_ui_component_show(psy_ui_button_base(&self->close));
	}
}

// delegate button clicked signals to trackbox signals
void trackbox_onmute(TrackBox* self, psy_ui_Button* sender)
{
	psy_signal_emit(&self->signal_mute, self, 0);
}

void trackbox_onsolo(TrackBox* self, psy_ui_Button* sender)
{
	psy_signal_emit(&self->signal_solo, self, 0);
}

void trackbox_onclose(TrackBox* self, psy_ui_Button* sender)
{
	psy_signal_emit(&self->signal_close, self, 0);
}

void trackbox_onresizemousedown(TrackBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;

	psy_ui_component_capture(&self->resize);
	self->doresize = TRUE;
	self->dragoffset = 0; // ev->pt.y;
	size = psy_ui_component_size_px(&self->component);
	self->baseheight = size.height;
	psy_ui_component_setcursor(&self->component,
		psy_ui_CURSORSTYLE_ROW_RESIZE);
}

void trackbox_onresizemousemove(TrackBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->doresize) {
		double offset;

		offset = ev->pt.y - self->dragoffset;
		psy_signal_emit(&self->signal_resize, self, 1, &offset);		
	}
	psy_ui_component_setcursor(&self->component,
		psy_ui_CURSORSTYLE_ROW_RESIZE);
}

void trackbox_onresizemouseup(TrackBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->resize);
	self->doresize = FALSE;
}
