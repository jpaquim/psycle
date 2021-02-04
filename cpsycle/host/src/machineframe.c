// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
// host
#include "resources/resource.h"
// ui
#include <uiframe.h>
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
	psy_ui_margin_init_all_em(&margin, 0.0, 1.0, 0.0, 0.0);
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
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row0, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
	// row1	
	presetsbar_init(&self->presetsbar, &self->component, workspace);
	psy_ui_component_setalign(&self->presetsbar.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all_em(&margin, 0.25, 0.0, 0.25, 0.0);		
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
static void machineframe_resize(MachineFrame*);
static void machineframe_onalign(MachineFrame*, psy_ui_Component* sender);
static void machineframe_preferredviewsizechanged(MachineFrame*,
	psy_ui_Component* sender);
static void ondefaultfontchanged(MachineFrame*, Workspace* sender);
static void machineframe_onzoomboxchanged(MachineFrame*, ZoomBox* sender);
static void machineframe_onmouseup(MachineFrame*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);

// implementation
void machineframe_init(MachineFrame* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->view = NULL;
	self->paramview = NULL;
	self->machine = NULL;
	self->machineview = parent;		
	self->workspace = workspace;		
	psy_ui_frame_init(&self->component, parent);
	psy_ui_component_move(&self->component,
		psy_ui_point_make(psy_ui_value_makepx(200),
			psy_ui_value_makepx(150)));	
	psy_ui_component_seticonressource(&self->component, IDI_MACPARAM);		
	parameterbar_init(&self->parameterbar, &self->component, workspace);
	psy_ui_component_setalign(&self->parameterbar.component, psy_ui_ALIGN_TOP);
	newvalview_init(&self->newval, &self->component, 0, 0, 0, 0, 0, "new val", workspace);
	psy_ui_component_setalign(&self->newval.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->newval.component);
	parameterlistbox_init(&self->parameterbox, &self->component, NULL, workspace);
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
	psy_signal_connect(&self->component.signal_mouseup, self,
		machineframe_onmouseup);
	psy_signal_connect(&self->component.signal_align, self,
		machineframe_onalign);	
	//psy_signal_connect(&self->workspace->signal_defaultfontchanged, self,
	//	ondefaultfontchanged);	
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
	//psy_signal_disconnect(&self->workspace->signal_defaultfontchanged, self,
	//	ondefaultfontchanged);
}

MachineFrame* machineframe_alloc(void)
{
	return (MachineFrame*) malloc(sizeof(MachineFrame));
}

MachineFrame* machineframe_allocinit(psy_ui_Component* parent,
	Workspace* workspace)
{
	MachineFrame* rv;

	rv = machineframe_alloc();
	if (rv) {
		machineframe_init(rv, parent, workspace);
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

void machineframe_resize(MachineFrame* self)
{
	psy_ui_Size viewsize;	
	psy_ui_Size bar;
	psy_ui_Size newval;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);	
	viewsize = psy_ui_component_preferredsize(self->view, 0);	
	if (psy_ui_component_visible(&self->parameterbox.component)) {				
		viewsize.width = psy_ui_value_makepx(
			psy_ui_value_px(&viewsize.width, tm) + 150);
	}	
	bar = psy_ui_component_preferredsize(&self->parameterbar.component,
		&viewsize);
	newval = psy_ui_component_preferredsize(&self->newval.component,
		&viewsize);
	viewsize.height = psy_ui_add_values(bar.height, viewsize.height, tm);
	if (self->newval.component.visible) {
		viewsize.height = psy_ui_add_values(newval.height, viewsize.height, tm);
	}
	psy_ui_component_clientresize(&self->component, viewsize);
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

void machineframe_onzoomboxchanged(MachineFrame* self, ZoomBox* sender)
{
	if (self->paramview) {		
		paramview_setzoom(self->paramview, sender->zoomrate);
		machineframe_resize(self);
	}
}

void machineframe_onmouseup(MachineFrame* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (ev->button == 2 && self->paramview && ev->target ==
			&self->paramview->component && self->machine &&
			self->paramview->lasttweak != psy_INDEX_INVALID) {
		psy_audio_MachineParam* tweakpar;
		intptr_t min_v = 1;
		intptr_t max_v = 1;
		char name[64], title[128];
		memset(name, 0, 64);

		tweakpar = psy_audio_machine_parameter(self->machine, self->paramview->lasttweak);
		if (tweakpar) {
			psy_audio_machine_parameter_range(self->machine, tweakpar, &min_v, &max_v);
			psy_audio_machine_parameter_name(self->machine, tweakpar, name);
			psy_snprintf
			(
				title, 128, "Param:'%.2x:%s' (Range from %d to %d)\0",
				(int)self->paramview->lasttweak,
				name,
				min_v,
				max_v
			);
			newvalview_reset(&self->newval, psy_audio_machine_slot(self->machine),
				self->paramview->lasttweak, psy_audio_machine_parameter_scaledvalue(
					self->machine, tweakpar), min_v, max_v, title);
			if (!psy_ui_component_visible(&self->newval.component)) {
				psy_ui_component_togglevisibility(&self->newval.component);
				machineframe_resize(self);
			}
			psy_ui_component_setfocus(psy_ui_edit_base(&self->newval.edit));
		}
	}
}

void machineframe_onalign(MachineFrame* self, psy_ui_Component* sender)
{
	if (self->paramview && self->newval.docancel) {
		self->newval.docancel = FALSE;
		self->newval.doapply = FALSE;
		machineframe_resize(self);
	} else if (self->paramview && self->newval.doapply) {
		psy_audio_MachineParam* tweakpar;

		self->newval.docancel = FALSE;
		self->newval.doapply = FALSE;
		if (self->machine && self->newval.paramindex != -1) {
			tweakpar = psy_audio_machine_parameter(self->machine, self->newval.paramindex);
			if (tweakpar) {
				psy_audio_machine_parameter_tweak_scaled(self->machine,
					tweakpar, self->newval.value);
			}
		}
		machineframe_resize(self);
	}
}
