// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "renderview.h"


void renderview_init(RenderView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_margin margin = { 0, 10, 0, 0 };


	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);

	// path group
	ui_component_init(&self->pathgroup, &self->component);
	ui_component_enablealign(&self->pathgroup);
	ui_component_setalign(&self->pathgroup, UI_ALIGN_TOP);
	ui_label_init(&self->pathdesc, &self->pathgroup);
	ui_label_settext(&self->pathdesc, "Output Path");
	ui_component_setalign(&self->pathdesc.component, UI_ALIGN_LEFT);
	ui_component_setmargin(&self->pathdesc.component, &margin);
	ui_edit_init(&self->pathedit, &self->pathgroup, 0);
	ui_component_setalign(&self->pathedit.component, UI_ALIGN_LEFT);
	ui_edit_setcharnumber(&self->pathedit, 20);
	ui_component_setmargin(&self->pathedit.component, &margin);
	ui_button_init(&self->browsebutton, &self->pathgroup);
	ui_button_settext(&self->browsebutton, "Browse");
	ui_component_setalign(&self->browsebutton.component, UI_ALIGN_LEFT);

	// options
	ui_component_init(&self->optiongroup, &self->component);
	ui_component_enablealign(&self->optiongroup);
	ui_component_setalign(&self->optiongroup, UI_ALIGN_TOP);
	ui_checkbox_init(&self->cbx0, &self->optiongroup);
	ui_component_setalign(&self->cbx0.component, UI_ALIGN_TOP);
	ui_checkbox_settext(&self->cbx0,
		"Save each unmuted input to master as a separated wav (wire number"
		" will be appended to filename)");
	ui_checkbox_init(&self->cbx1, &self->optiongroup);
	ui_component_setalign(&self->cbx1.component, UI_ALIGN_TOP);
	ui_checkbox_settext(&self->cbx1, "Save each unmuted track as a separated"
		"wav (track number will be appended to filename) ** may suffer from"
		"'delay bleed' - insert silence at the end of your file if this is a "
		"problem.");
	ui_checkbox_init(&self->cbx2, &self->optiongroup);
	ui_component_setalign(&self->cbx2.component, UI_ALIGN_TOP);
	ui_checkbox_settext(&self->cbx2,
		"Save each unmuted generator as a separated wav (generator number will"
		"be appended to filename) ** may suffer from 'delay bleed' - insert"
		" silence at the end of your file if this is a problem.");

	ui_button_init(&self->savebutton, &self->component);
	ui_button_settext(&self->savebutton, "Save wave");
	ui_component_setalign(&self->savebutton.component, UI_ALIGN_TOP);	
}
