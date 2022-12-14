/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineframe.h"
/* host */
#include "paramviews.h"
#include "resources/resource.h"
#include "machineeditorview.h"
#include "paramview.h"
#include "styles.h"
/* ui */
#include <uiframe.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* ParameterBar */
static void machineframe_init_help(MachineFrame*, psy_ui_Component* parent);
static void parameterbar_onalign(ParameterBar*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(ParameterBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onalign =
			(psy_ui_fp_component)
			parameterbar_onalign;
		vtable_initialized = TRUE;
	}
	return &vtable;
}

/* implementation */
void parameterbar_init(ParameterBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{			
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent, parent);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_set_vtable(&self->component, vtable_init(self));
	psy_ui_margin_init_em(&margin, 0.0, 1.0, 0.0, 0.0);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	/* row0 */
	psy_ui_component_init(&self->row0, &self->component, NULL);
	psy_ui_component_set_align_expand(&self->row0,
		psy_ui_HEXPAND);	
	zoombox_init(&self->zoombox, &self->row0);
	psy_ui_component_set_align(zoombox_base(&self->zoombox), psy_ui_ALIGN_LEFT);	
	psy_ui_component_init(&self->buttons, &self->row0, NULL);
	psy_ui_component_set_align(&self->buttons, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align_expand(&self->buttons,
		psy_ui_HEXPAND);
	psy_ui_component_set_default_align(&self->buttons, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->power, &self->buttons);
	psy_ui_button_set_text(&self->power, "machineframe.pwr");
	margin = psy_ui_component_margin(&self->power.component);
	margin.left = psy_ui_value_make_ew(1.0);
	psy_ui_component_set_margin(&self->power.component, margin);
	psy_ui_bitmap_load_resource(&self->power.bitmapicon, IDB_POWER_DARK);
	psy_ui_bitmap_settransparency(&self->power.bitmapicon,
		psy_ui_colour_make(0x00FFFFFF));
	psy_ui_button_init(&self->parameters, &self->buttons);
	psy_ui_button_set_text(&self->parameters, "machineframe.parameters");
	psy_ui_button_init(&self->parammap, &self->buttons);
	psy_ui_button_set_text(&self->parammap, "machineframe.parammap");
	psy_ui_button_init(&self->command, &self->buttons);
	psy_ui_button_set_text(&self->command, "machineframe.command");
	psy_ui_button_init(&self->help, &self->buttons);
	psy_ui_button_set_text(&self->help, "machineframe.help");	
	psy_ui_button_init(&self->isbus, &self->buttons);
	psy_ui_button_set_text(&self->isbus, "machineframe.bus");	
	psy_ui_button_init(&self->bank, &self->buttons);
	psy_ui_button_set_text(&self->bank, "machineframe.bank");	
	psy_ui_button_init(&self->more, &self->row0);
	psy_ui_button_prevent_translation(&self->more);
	psy_ui_button_set_text(&self->more, ". . .");
	psy_ui_component_set_align(psy_ui_button_base(&self->more),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_margin(psy_ui_button_base(&self->more),
		psy_ui_margin_zero());	
	/* row1 */
	presetsbar_init(&self->presetsbar, &self->component, workspace);
	psy_ui_component_set_align(&self->presetsbar.component, psy_ui_ALIGN_TOP);	
}

void parameterbar_onalign(ParameterBar* self)
{
	psy_ui_Size preferredsize;
	psy_ui_Size preferredsize_more;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;

	preferredsize = psy_ui_component_preferred_size(&self->component, NULL);
	preferredsize_more = psy_ui_component_preferred_size(
		psy_ui_button_base(&self->more), NULL);
	size = psy_ui_component_scroll_size_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	if (size.width < psy_ui_value_px(&preferredsize.width, tm, NULL)) {
		if (!psy_ui_component_visible(psy_ui_button_base(&self->more))) {
			/* psy_ui_component_show(psy_ui_button_base(&self->more)); */
		}
	} else {
		if (psy_ui_component_visible(psy_ui_button_base(&self->more))) {
			/* psy_ui_component_hide(psy_ui_button_base(&self->more)); */
		}
	}
}

/* MachineFrame */
/* prototypes */

static void machineframe_on_destroyed(MachineFrame*);
static void machineframe_initparamview(MachineFrame*, Workspace*);
static void machineframe_toggleparameterbox(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_toggleparammap(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_oncommand(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_togglepwr(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_updatepwr(MachineFrame*);
static void machineframe_togglehelp(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_togglebus(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_on_bank_manager(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_toggleshowfullmenu(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_resize(MachineFrame*);
static void machineframe_onalign(MachineFrame*, psy_ui_Component* sender);
static void machineframe_preferredviewsizechanged(MachineFrame*,
	psy_ui_Component* sender);
static void ondefaultfontchanged(MachineFrame*, Workspace* sender);
static void machineframe_onzoomboxchanged(MachineFrame*, ZoomBox* sender);
static void machineframe_on_mouse_up(MachineFrame*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void machineframe_on_timer(MachineFrame* self, uintptr_t timerid);
static bool machineframe_on_close(MachineFrame*);

/* vtable */
static psy_ui_ComponentVtable machineframe_vtable;
static bool machineframe_vtable_initialized = FALSE;

static void machineframe_vtable_init(MachineFrame* self)
{
	if (!machineframe_vtable_initialized) {
		machineframe_vtable = *(self->component.vtable);
		machineframe_vtable.onclose =
			(psy_ui_fp_component_onclose)
			machineframe_on_close;
		machineframe_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			machineframe_on_timer;
		machineframe_vtable_initialized = TRUE;
	}
	self->component.vtable = &machineframe_vtable;
}

/* implementation */
void machineframe_init(MachineFrame* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, ParamViews* paramviews, Workspace* workspace)
{	
	psy_ui_toolframe_init(&self->component, parent);
	machineframe_vtable_init(self);
	psy_ui_component_move(&self->component,
		psy_ui_point_make_em(20.0, 15.0));
	psy_ui_component_set_icon_ressource(&self->component, IDI_MACPARAM);	
	self->view = NULL;
	self->paramview = NULL;
	self->editorview = NULL;
	self->paramviews = paramviews;
	self->machine = NULL;
	self->machineview = parent;
	self->showfullmenu = FALSE;
	self->macid = psy_INDEX_INVALID;
	self->machine = machine;
	self->workspace = workspace;
	parameterbar_init(&self->parameterbar, &self->component, workspace);
	psy_ui_component_set_align(&self->parameterbar.component, psy_ui_ALIGN_TOP);
	newvalview_init(&self->newval, &self->component, 0, 0, 0, 0, 0,
		"new val", workspace);
	psy_ui_component_set_align(&self->newval.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->newval.component);	
	psy_ui_component_init(&self->client, &self->component, NULL);	
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	parammap_init(&self->parammap, &self->client, NULL,
		psycleconfig_macparam(workspace_conf(workspace)));
	psy_ui_component_set_align(&self->parammap.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_init(&self->parameters, &self->client, &self->client);
	psy_ui_component_set_align(&self->parameters, psy_ui_ALIGN_RIGHT);
	parameterlistbox_init(&self->parameterbox, &self->parameters, NULL,
		psycleconfig_macparam(workspace_conf(workspace)));	
	psy_ui_component_set_align(&self->parameterbox.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	machineframe_init_help(self, psy_ui_notebook_base(&self->notebook));	
	psy_signal_connect(&self->parameterbar.parameters.signal_clicked, self,
		machineframe_toggleparameterbox);
	psy_signal_connect(&self->parameterbar.parammap.signal_clicked, self,
		machineframe_toggleparammap);
	psy_signal_connect(&self->parameterbar.command.signal_clicked, self,
		machineframe_oncommand);
	psy_signal_connect(&self->parameterbar.help.signal_clicked, self,
		machineframe_togglehelp);
	psy_signal_connect(&self->parameterbar.power.signal_clicked, self,
		machineframe_togglepwr);
	psy_signal_connect(&self->parameterbar.isbus.signal_clicked, self,
		machineframe_togglebus);
	psy_signal_connect(&self->parameterbar.bank.signal_clicked, self,
		machineframe_on_bank_manager);
	psy_signal_connect(&self->parameterbar.more.signal_clicked, self,
		machineframe_toggleshowfullmenu);
	psy_signal_connect(&self->component.signal_mouseup, self,
		machineframe_on_mouse_up);
	psy_signal_connect(&self->component.signal_align, self,
		machineframe_onalign);	
	psy_signal_connect(&self->parameterbar.zoombox.signal_changed, self,
		machineframe_onzoomboxchanged);	
	machineframe_initparamview(self, workspace);
	machineframe_updatepwr(self);	
}

void machineframe_init_help(MachineFrame* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->helpview, parent, NULL);
	psy_ui_component_set_style_type(&self->helpview, STYLE_MACPARAM_TITLE);
	//psy_ui_component_doublebuffer(&self->helpview);
	psy_ui_label_init_text(&self->help, &self->helpview, "machineframe.about");	
	psy_ui_component_set_wheel_scroll(&self->help.component, 4);	
	psy_ui_component_set_overflow(&self->help.component,
		psy_ui_OVERFLOW_SCROLL);
	psy_ui_component_set_align(&self->help.component, psy_ui_ALIGN_HCLIENT);
	psy_ui_label_enable_wrap(&self->help);
	psy_ui_component_set_scroll_step_height(psy_ui_label_base(&self->help),
		psy_ui_value_make_eh(1.0));	
	psy_ui_scroller_init(&self->scroller, &self->helpview, NULL, NULL);
	psy_ui_component_set_style_type(&self->scroller.pane, STYLE_MACPARAM_TITLE);
	psy_ui_scroller_set_client(&self->scroller, &self->help.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_init_align(&self->right, &self->helpview, NULL,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_padding(&self->right, psy_ui_margin_make_em(
		1.0, 1.0, 1.0, 5.0));
	psy_ui_component_setmaximumsize(&self->right, 
			psy_ui_size_make_em(40.0, 0.0));
	if (self->machine) {			
		const psy_audio_MachineInfo* machine_info;
		
		machine_info = psy_audio_machine_info(self->machine);
		if (machine_info) {			
			psy_ui_label_init_text(&self->image_desc, &self->right,
				machine_info->image_desc);
			psy_ui_label_enable_wrap(&self->image_desc);
			psy_ui_component_set_align(&self->image_desc.component,
				psy_ui_ALIGN_BOTTOM);			
			psy_ui_image_init_resource(&self->image, &self->right,
				machine_info->image_id);
			psy_ui_image_setbitmapalignment(&self->image,
				psy_ui_ALIGNMENT_RIGHT);
			psy_ui_component_set_align(&self->image.component,
				psy_ui_ALIGN_BOTTOM);
			psy_ui_component_set_margin(&self->image.component,
				psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));			
		}
	}	
}

bool machineframe_on_close(MachineFrame* self)
{
	if (self->editorview && self->machine) {
		psy_audio_machine_seteditorhandle(self->machine, NULL);
		psy_signal_disconnect_context(
			&self->workspace->signal_machineeditresize, self->editorview);
		psy_ui_component_stop_timer(&self->editorview->component, 0);
	}
	/*
	** Paramview stores pointers of all machineframes erase the frame from
	** paramviews
	*/
	if (self->machine) {
		paramviews_erase(self->paramviews, self->macid);
	}
	self->machine = NULL;
	self->view = NULL;	
	return TRUE;
}

MachineFrame* machineframe_alloc(void)
{
	return (MachineFrame*)malloc(sizeof(MachineFrame));
}

MachineFrame* machineframe_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, struct ParamViews* paramviews,
	Workspace* workspace)
{
	MachineFrame* rv;

	rv = machineframe_alloc();
	if (rv) {
		machineframe_init(rv, parent, machine, paramviews, workspace);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void machineframe_initparamview(MachineFrame* self, Workspace* workspace)
{
	if (!self->machine) {
		return;
	}
	if (psy_audio_machine_haseditor(self->machine)) {
		self->editorview = machineeditorview_allocinit(
			psy_ui_notebook_base(&self->notebook), self->machine, workspace);
		if (self->editorview) {
			machineframe_setview(self, &self->editorview->component,
				self->machine);
		}
	} else {
		ParamView* paramview;

		paramview = paramview_allocinit(
			psy_ui_notebook_base(&self->notebook),
			self->machine,
			psycleconfig_macparam(workspace_conf(workspace)),
			&self->view);
		if (paramview) {
			machineframe_setparamview(self, paramview, self->machine);
		}
	}
}

void machineframe_setparamview(MachineFrame* self, ParamView* view,
	psy_audio_Machine* machine)
{
	self->paramview = view;
	machineframe_setview(self, &view->component, machine);
	machineframe_updatepwr(self);
}

void machineframe_setview(MachineFrame* self, psy_ui_Component* view,
	psy_audio_Machine* machine)
{	
	char text[128];
	
	self->view = view;
	self->machine = machine;
	psy_ui_component_set_align(self->view, psy_ui_ALIGN_CLIENT);		
	parameterlistbox_setmachine(&self->parameterbox, machine);
	psy_ui_component_hide(&self->parameterbox.component);
	parammap_setmachine(&self->parammap, machine);
	psy_ui_component_hide(&self->parammap.component);
	presetsbar_setmachine(&self->parameterbar.presetsbar, machine);	
	if (self->machine) {
		self->macid = psy_audio_machine_slot(self->machine);
	} else {
		self->macid = psy_INDEX_INVALID;
	}
	if (self->machine &&
			psy_audio_machine_mode(self->machine) == psy_audio_MACHMODE_GENERATOR) {
		psy_ui_component_hide(&self->parameterbar.isbus.component);
	}
	if (self->machine && psy_audio_machine_haseditor(self->machine)) {
		psy_ui_component_hide(&self->parameterbar.zoombox.component);
	}
	if (self->machine && psy_audio_machine_editname(self->machine)) {
		psy_snprintf(text, 128, "%.2X : %s",
			psy_audio_machine_slot(self->machine),
			psy_audio_machine_editname(self->machine));
	} else {
		psy_ui_component_set_title(&self->component, text);
			psy_snprintf(text, 128, "%.2X :",
				psy_audio_machine_slot(self->machine));
	}
	psy_ui_notebook_select(&self->notebook, 1);
	psy_ui_component_set_title(&self->component, text);
	psy_ui_component_align(&self->component);
	psy_signal_connect(&view->signal_preferredsizechanged, self,
		machineframe_preferredviewsizechanged);	
	if (self->machine && psy_audio_machine_info(machine)) {
		psy_ui_label_set_text(&self->help,
			(psy_audio_machine_info(machine)->helptext)
			? psy_audio_machine_info(machine)->helptext
			: "");
	} else {
		psy_ui_label_set_text(&self->help, "");
	}
	if (self->machine) {
		if (psy_audio_machine_isbus(self->machine)) {	
			psy_ui_button_highlight(&self->parameterbar.isbus);
		} else {			
			psy_ui_button_disable_highlight(&self->parameterbar.isbus);			
		}
	}	
	machineframe_resize(self);		
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void machineframe_toggleparameterbox(MachineFrame* self,
	psy_ui_Component* sender)
{
	psy_ui_Size viewsize;

	viewsize = psy_ui_component_preferred_size(self->view, 0);
	if (psy_ui_component_visible(&self->parameterbox.component)) {
		psy_ui_component_hide(&self->parameterbox.component);		
		psy_ui_button_disable_highlight(&self->parameterbar.parameters);
	} else {
		psy_ui_component_show(&self->parameterbox.component);		
		psy_ui_button_highlight(&self->parameterbar.parameters);
	}
	machineframe_resize(self);
	psy_ui_component_align(&self->client);
}

void machineframe_toggleparammap(MachineFrame* self,
	psy_ui_Component* sender)
{
	psy_ui_Size viewsize;

	viewsize = psy_ui_component_preferred_size(self->view, 0);
	if (psy_ui_component_visible(&self->parammap.component)) {
		psy_ui_component_hide(&self->parammap.component);
		psy_ui_button_disable_highlight(&self->parameterbar.parammap);
	} else {
		psy_ui_component_show(&self->parammap.component);
		psy_ui_button_highlight(&self->parameterbar.parammap);
	}
	machineframe_resize(self);
	psy_ui_component_align(&self->client);
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
		psy_ui_button_disable_highlight(&self->parameterbar.help);
	} else {		
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_button_highlight(&self->parameterbar.help);
	}	
}

void machineframe_togglepwr(MachineFrame* self,
	psy_ui_Component* sender)
{
	if (self->machine) {
		if (psy_audio_machine_hasstandby(self->machine)) {
			psy_audio_machine_deactivatestandby(self->machine);			
		} else {
			psy_audio_machine_standby(self->machine);						
		}
		machineframe_updatepwr(self);
	}	
}

void machineframe_updatepwr(MachineFrame* self) {
	if (self->machine) {
		if (psy_audio_machine_hasstandby(self->machine)) {
			psy_ui_button_disable_highlight(&self->parameterbar.power);
		} else {
			psy_ui_button_highlight(&self->parameterbar.power);			
		}
	}
}

void machineframe_togglebus(MachineFrame* self,
	psy_ui_Component* sender)
{
	if (self->machine) {
		if (psy_audio_machine_isbus(self->machine)) {
			psy_audio_machine_unsetbus(self->machine);
			psy_ui_button_disable_highlight(&self->parameterbar.isbus);
		} else {
			psy_audio_machine_setbus(self->machine);
			psy_ui_button_highlight(&self->parameterbar.isbus);
		}
	}
}

void machineframe_toggleshowfullmenu(MachineFrame* self,
	psy_ui_Component* sender)
{
	self->showfullmenu = !self->showfullmenu;
	machineframe_resize(self);
}

void machineframe_on_bank_manager(MachineFrame* self,
	psy_ui_Component* sender)
{	
	workspace_select_view(self->workspace, viewindex_make_all(
		VIEW_ID_MACHINEVIEW, SECTION_ID_MACHINEVIEW_BANK_MANGER, self->macid,
		psy_INDEX_INVALID));
}

void machineframe_resize(MachineFrame* self)
{
	psy_ui_Size viewsize;
	psy_ui_Size bar;
	psy_ui_Size newval;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);	
	viewsize = psy_ui_component_preferred_size(self->view, NULL);	
	if (psy_ui_component_visible(&self->parameterbox.component)) {
		psy_ui_Size paramsize;		
		uintptr_t numparams;

		paramsize = psy_ui_component_preferred_size(
			&self->parameterbox.component, NULL);
		viewsize.width = psy_ui_add_values(viewsize.width, 
			paramsize.width, tm, NULL);
		numparams = psy_audio_machine_numtweakparameters(self->machine);
		viewsize.height = psy_ui_max_values(viewsize.height,
			psy_ui_value_make_eh(psy_min(numparams + 4.0, 8.0)), tm, NULL);
	}
	if (psy_ui_component_visible(&self->parammap.component)) {
		psy_ui_Size paramsize;
		uintptr_t numparams;

		paramsize = psy_ui_component_preferred_size(
			&self->parammap.component, NULL);
		viewsize.width = psy_ui_add_values(viewsize.width,
			paramsize.width, tm, NULL);
		numparams = psy_audio_machine_numtweakparameters(self->machine);
		viewsize.height = psy_ui_max_values(viewsize.height,
			psy_ui_value_make_eh(psy_min(numparams + 4.0, 8.0)), tm, NULL);
	}
	if (self->showfullmenu) {
		bar = psy_ui_component_preferred_size(&self->parameterbar.component,
			NULL);
		viewsize.width = psy_ui_max_values(viewsize.width, bar.width, tm, NULL);
	}
	newval = psy_ui_component_preferred_size(&self->newval.component,
		&viewsize);
	bar = psy_ui_component_preferred_size(&self->parameterbar.component,
		&viewsize);
	viewsize.height = psy_ui_add_values(bar.height, viewsize.height, tm, NULL);
	if (psy_ui_component_visible(&self->newval.component)) {
		viewsize.height = psy_ui_add_values(newval.height, viewsize.height, tm, NULL);
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
		paramview_set_zoom(self->paramview, sender->zoomrate);
		machineframe_resize(self);
	}
}

void machineframe_on_mouse_up(MachineFrame* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	/*if (ev->button == 2 && self->paramview && ev->target ==
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
				psy_ui_component_toggle_visibility(&self->newval.component);
				machineframe_resize(self);
			}
			psy_ui_component_set_focus(psy_ui_textarea_base(&self->newval.edit));
		}
	}*/
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

void machineframe_on_timer(MachineFrame* self, uintptr_t timerid)
{
	if (self->view) {
		self->view->vtable->on_timer(self->view, 0);
	}	
	machineframe_updatepwr(self);
	if (psy_ui_component_visible(&self->parameterbox.component)) {
		psy_ui_component_invalidate(&self->parameterbox.knob.component);
	}	
}
