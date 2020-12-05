// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
// host
#include "resources/resource.h"
// ui
#include <uiframe.h>
// std
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// ParameterBar
// implementation
void parameterbar_init(ParameterBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{			
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent);		
	// titlerow
	psy_ui_component_init(&self->titlerow, &self->component);
	psy_ui_label_init(&self->title, &self->titlerow);	
	psy_ui_component_setalign(&self->floatview.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->close, &self->titlerow);
	psy_ui_button_settext(&self->close, "X");
	psy_ui_button_init(&self->floatview, &self->titlerow);
	psy_ui_button_settext(&self->floatview, "Float");
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setalign(&self->close.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_setalign(&self->floatview.component, psy_ui_ALIGN_RIGHT);
	// row0
	psy_ui_component_init(&self->row0, &self->component);
	zoombox_init(&self->zoombox, &self->row0);
	self->zoombox.zoomstep = 0.10;
	psy_ui_button_init(&self->mute, &self->row0);	
	psy_ui_button_settext(&self->mute, "Mute");
	psy_ui_button_init(&self->parameters, &self->row0);
	psy_ui_button_settext(&self->parameters, "Parameters");	
	psy_ui_button_init(&self->command, &self->row0);
	psy_ui_button_settext(&self->command, "Command");
	psy_ui_button_init(&self->help, &self->row0);	
	psy_ui_button_settext(&self->help, "Help");
	psy_ui_button_init(&self->dock, &self->row0);
	psy_ui_button_settext(&self->dock, "Dock");	
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row0, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
	// row1	
	presetsbar_init(&self->presetsbar, &self->component, workspace);
	psy_ui_component_setalign(&self->presetsbar.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.25),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.25),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP, NULL));
	psy_ui_component_setmargin(&self->presetsbar.component, &margin);
}

// MachineFrame
// prototypes
static void machineframe_ondestroyed(MachineFrame* self,
	psy_ui_Component* frame);
static void machineframe_toggleparameterbox(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_oncommand(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_togglehelp(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_toggledock(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_onfloatview(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_onclose(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_resize(MachineFrame*);
static void machineframe_preferredviewsizechanged(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_setfloatbar(MachineFrame*);
static void machineframe_setdockbar(MachineFrame*);
static void ondefaultfontchanged(MachineFrame*, Workspace* sender);
static void machineframe_onzoomboxchanged(MachineFrame*, ZoomBox* sender);
// implementation
void machineframe_init(MachineFrame* self, psy_ui_Component* parent,
	bool floated, Workspace* workspace)
{
	self->view = NULL;
	self->paramview = NULL;
	self->machine = NULL;
	self->machineview = parent;
	self->dofloat = 0;
	self->dodock = 0;
	self->doclose = 0;
	self->floated = floated;
	self->workspace = workspace;	
	if (floated) {
		psy_ui_frame_init(&self->component, parent);
		psy_ui_component_move(&self->component,
			psy_ui_point_make(
				psy_ui_value_makepx(200),
				psy_ui_value_makepx(150)));
	} else {
		psy_ui_component_init(&self->component, parent);
	}
	psy_ui_component_seticonressource(&self->component, IDI_MACPARAM);	
	parameterbar_init(&self->parameterbar, &self->component, workspace);
	psy_ui_component_setalign(&self->parameterbar.component, psy_ui_ALIGN_TOP);
	parameterlistbox_init(&self->parameterbox, &self->component, NULL);
	psy_ui_component_setalign(&self->parameterbox.component,
		psy_ui_ALIGN_RIGHT);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_editor_init(&self->help, psy_ui_notebook_base(&self->notebook));
	psy_ui_editor_addtext(&self->help, "About");
	psy_signal_connect(&self->component.signal_destroyed, self,
		machineframe_ondestroyed);
	psy_signal_connect(&self->parameterbar.parameters.signal_clicked, self,
		machineframe_toggleparameterbox);
	psy_signal_connect(&self->parameterbar.command.signal_clicked, self,
		machineframe_oncommand);
	psy_signal_connect(&self->parameterbar.help.signal_clicked, self,
		machineframe_togglehelp);
	psy_signal_connect(&self->parameterbar.dock.signal_clicked, self,
		machineframe_toggledock);
	psy_signal_connect(&self->parameterbar.dock.signal_clicked, self,
		machineframe_toggledock);
	psy_signal_connect(&self->parameterbar.floatview.signal_clicked, self,
		machineframe_onfloatview);
	psy_signal_connect(&self->parameterbar.close.signal_clicked, self,
		machineframe_onclose);
	if (floated) {
		machineframe_setfloatbar(self);
	} else {
		machineframe_setdockbar(self);
	}	
	psy_signal_connect(&self->workspace->signal_defaultfontchanged, self,
		ondefaultfontchanged);	
	psy_signal_connect(&self->parameterbar.zoombox.signal_changed, self,
		machineframe_onzoomboxchanged);
}

void machineframe_setparamview(MachineFrame* self, ParamView* view,
	psy_audio_Machine* machine)
{
	self->paramview = view;
	machineframe_setview(self, &view->component, machine);
}

void machineframe_setview(MachineFrame* self, psy_ui_Component* view,
	psy_audio_Machine* machine)
{	
	char text[128];
	
	self->view = view;
	self->machine = machine;
	psy_ui_component_setalign(self->view, psy_ui_ALIGN_CLIENT);	
	parameterlistbox_setmachine(&self->parameterbox, machine);
	psy_ui_component_hide(&self->parameterbox.component);
	presetsbar_setmachine(&self->parameterbar.presetsbar, machine);			
	machineframe_resize(self);
	if (self->machine && psy_audio_machine_editname(self->machine)) {
		psy_snprintf(text, 128, "%.2X : %s",
			psy_audio_machine_slot(self->machine),
			psy_audio_machine_editname(self->machine));
	} else {
		psy_ui_component_settitle(&self->component, text);
			psy_snprintf(text, 128, "%.2X :",
				psy_audio_machine_slot(self->machine));
	}
	psy_ui_notebook_select(&self->notebook, 1);
	psy_ui_component_settitle(&self->component, text);
	psy_ui_component_align(&self->component);
	psy_signal_connect(&view->signal_preferredsizechanged, self,
		machineframe_preferredviewsizechanged);	
	psy_ui_label_settext(&self->parameterbar.title,
		(self->machine)
		? psy_audio_machine_editname(self->machine)
		: "");
	if (self->machine && psy_audio_machine_info(machine)) {
		psy_ui_editor_settext(&self->help,
			(psy_audio_machine_info(machine)->helptext)
			? psy_audio_machine_info(machine)->helptext
			: "");
	} else {
		psy_ui_editor_settext(&self->help, "");
	}	
}

void machineframe_ondestroyed(MachineFrame* self, psy_ui_Component* frame)
{	
	if (self->view) {
		free(self->view);
	}
	psy_signal_disconnect(&self->workspace->signal_defaultfontchanged, self,
		ondefaultfontchanged);
}

MachineFrame* machineframe_alloc(void)
{
	return (MachineFrame*) malloc(sizeof(MachineFrame));
}

MachineFrame* machineframe_allocinit(psy_ui_Component* parent, bool floated,
	Workspace* workspace)
{
	MachineFrame* rv;

	rv = machineframe_alloc();
	if (rv) {
		machineframe_init(rv, parent, floated, workspace);
	}
	return rv;	
}

void machineframe_toggleparameterbox(MachineFrame* self,
	psy_ui_Component* sender)
{
	psy_ui_Size viewsize;

	viewsize = psy_ui_component_preferredsize(self->view, 0);
	if (psy_ui_component_visible(&self->parameterbox.component)) {
		psy_ui_component_hide(&self->parameterbox.component);		
		psy_ui_button_disablehighlight(&self->parameterbar.parameters);
	} else {
		psy_ui_component_show(&self->parameterbox.component);		
		psy_ui_button_highlight(&self->parameterbar.parameters);
	}
	machineframe_resize(self);
}

void machineframe_oncommand(MachineFrame* self, psy_ui_Component* sender)
{
	if (self->machine) {
		psy_audio_machine_command(self->machine);
	}
}

void machineframe_togglehelp(MachineFrame* self,
	psy_ui_Component* sender)
{	
	if (psy_ui_notebook_pageindex(&self->notebook) == 0) {			
		psy_ui_notebook_select(&self->notebook, 1);
		psy_ui_button_disablehighlight(&self->parameterbar.help);
	} else {		
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_button_highlight(&self->parameterbar.help);
	}	
}

void machineframe_toggledock(MachineFrame* self, psy_ui_Component* sender)
{
	if (!self->floated) {
		self->dofloat = 1;
	} else {
		self->dodock = 1;
	}
}

void machineframe_onfloatview(MachineFrame* self, psy_ui_Component* sender)
{
	if (!self->floated) {
		self->dofloat = 1;
	}	
}

void machineframe_onclose(MachineFrame* self, psy_ui_Component* sender)
{
	self->doclose = 1;
}

void machineframe_resize(MachineFrame* self)
{
	psy_ui_Size viewsize;	
	psy_ui_Size bar;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);	
	viewsize = psy_ui_component_preferredsize(self->view, 0);	
	if (psy_ui_component_visible(&self->parameterbox.component)) {				
		viewsize.width = psy_ui_value_makepx(
			psy_ui_value_px(&viewsize.width, &tm) + 150);
	}	
	bar = psy_ui_component_preferredsize(&self->parameterbar.component,
		&viewsize);	
	psy_ui_component_clientresize(&self->component,
		psy_ui_size_make(
			viewsize.width,
			psy_ui_add_values(bar.height, viewsize.height, &tm)));
}

void machineframe_preferredviewsizechanged(MachineFrame* self,
	psy_ui_Component* sender)
{	
	machineframe_resize(self);	
}

void ondefaultfontchanged(MachineFrame* self, Workspace* sender)
{
	machineframe_resize(self);
}

void machineframe_setfloatbar(MachineFrame* self)
{	
	psy_ui_component_hide(&self->parameterbar.titlerow);
	psy_ui_component_show(&self->parameterbar.dock.component);
}

void machineframe_setdockbar(MachineFrame* self)
{
	if (self->machine) {
		psy_ui_label_settext(&self->parameterbar.title,
			psy_audio_machine_editname(self->machine));
	} else {
		psy_ui_label_settext(&self->parameterbar.title, "");
	}
	psy_ui_component_hide(&self->parameterbar.dock.component);
	psy_ui_component_show(&self->parameterbar.titlerow);
}

void machineframe_onzoomboxchanged(MachineFrame* self, ZoomBox* sender)
{
	if (self->paramview) {		
		paramview_setzoom(self->paramview, sender->zoomrate);
		machineframe_resize(self);
	}
}
