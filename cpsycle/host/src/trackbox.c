// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackbox.h"
// host
#include "styles.h"
// ui
#include <uiapp.h>
// platform
#include "../../detail/portable.h"

// TrackBox
// implementation
void trackbox_init(TrackBox* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_Margin spacing;
	
	psy_ui_component_init(trackbox_base(self), parent, view);	
	psy_ui_component_setdefaultalign(trackbox_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_margin_init_all_em(&spacing, 0.0, 0.0, 0.0, 1.0);
	psy_ui_component_setspacing(trackbox_base(self), &spacing);
	psy_ui_component_setalignexpand(trackbox_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->trackidx, trackbox_base(self), view);
	psy_ui_label_preventtranslation(&self->trackidx);
	psy_ui_label_settext(&self->trackidx, "00");
	psy_ui_label_setcharnumber(&self->trackidx, 3);
	psy_ui_button_init(&self->solo, trackbox_base(self), view);
	psy_ui_button_preventtranslation(&self->solo);
	psy_ui_button_settext(&self->solo, "S");	
	psy_ui_button_init(&self->mute, trackbox_base(self), view);
	psy_ui_button_preventtranslation(&self->mute);
	psy_ui_button_settext(&self->mute, "M");
	psy_ui_label_init(&self->desc, trackbox_base(self), view);
	psy_ui_label_preventtranslation(&self->desc);
	psy_ui_component_setalign(psy_ui_label_base(&self->desc),
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_init(&self->close, trackbox_base(self), view);
	psy_ui_button_preventtranslation(&self->close);
	psy_ui_button_settext(&self->close, "X");
	psy_ui_component_setalign(psy_ui_button_base(&self->close),
		psy_ui_ALIGN_RIGHT);
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
	char text[128];

	psy_snprintf(text, 128, "%.2X", index);
	psy_ui_label_settext(&self->trackidx, text);
	self->solo.data = index;
	self->mute.data = index;
	self->close.data = index;
	if (index == 0) {
		psy_ui_component_hide(psy_ui_button_base(&self->close));
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
