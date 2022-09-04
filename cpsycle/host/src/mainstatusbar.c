/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "mainstatusbar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void statuslabel_init_label(StatusLabel*);
static void statuslabel_init_progress_bar(StatusLabel*);
static void statuslabel_on_align(StatusLabel*);

/* vtable */
static psy_ui_ComponentVtable status_label_vtable;
static bool status_label_vtable_initialized = FALSE;

static void status_label_vtable_init(StatusLabel* self)
{
	if (!status_label_vtable_initialized) {
		status_label_vtable = *(self->component.vtable);
		status_label_vtable.onalign =
			(psy_ui_fp_component)
			statuslabel_on_align;		
		status_label_vtable_initialized = TRUE;
	}
	self->component.vtable = &status_label_vtable;
}

void statuslabel_init(StatusLabel* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	status_label_vtable_init(self);	
	statuslabel_init_progress_bar(self);
	statuslabel_init_label(self);
}

void statuslabel_init_label(StatusLabel* self)
{
	psy_ui_label_init(&self->label, &self->component);
	psy_ui_label_prevent_translation(&self->label);
	psy_ui_label_set_text(&self->label, "Ready");
	psy_ui_label_prevent_wrap(&self->label);
}

void statuslabel_init_progress_bar(StatusLabel* self)
{
	psy_ui_progressbar_init(&self->progressbar, &self->component);	
}

void statuslabel_on_align(StatusLabel* self)
{
	psy_ui_Size size;
	
	size = psy_ui_component_scroll_size(&self->component);
	psy_ui_component_resize(&self->progressbar.component, size);
	psy_ui_component_resize(psy_ui_label_base(&self->label), size);	
}


/* MainStatusBar */

/* prototypes */
static void mainstatusbar_on_destroyed(MainStatusBar*);
static void mainstatusbar_init_zoom_box(MainStatusBar*);
static void mainstatusbar_init_status_label(MainStatusBar*);
static void mainstatusbar_init_progress_bar(MainStatusBar*);
static void mainstatusbar_init_turnoff_button(MainStatusBar*);
static void mainstatusbar_init_clock_bar(MainStatusBar*);
static void mainstatusbar_init_kbd_help_button(MainStatusBar*);
static void mainstatusbar_init_terminal_button(MainStatusBar*);
static void mainstatusbar_on_song_load_progress(MainStatusBar*,
	Workspace* sender, intptr_t progress);
static void mainstatusbar_on_plugin_scan_progress(MainStatusBar*,
	PluginScanThread* sender, int progress);
static void mainstatusbar_on_status(MainStatusBar*, Workspace* sender,
	const char* text);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainStatusBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			mainstatusbar_on_destroyed;		
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void mainstatusbar_init(MainStatusBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_STATUSBAR);
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);	
	self->workspace = workspace;	
	psy_lock_init(&self->outputlock);
	self->strbuffer = NULL;	
	psy_ui_component_set_default_align(&self->pane, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	mainstatusbar_init_zoom_box(self);	
	mainstatusbar_init_status_label(self);	
	mainstatusbar_init_turnoff_button(self);
	mainstatusbar_init_clock_bar(self);
	mainstatusbar_init_kbd_help_button(self);
	mainstatusbar_init_terminal_button(self);
	mainstatusbar_init_progress_bar(self);
}

void mainstatusbar_on_destroyed(MainStatusBar* self)
{
	free(self->strbuffer);
	self->strbuffer = NULL;
	psy_lock_dispose(&self->outputlock);
}

void mainstatusbar_init_zoom_box(MainStatusBar* self)
{
	zoombox_init(&self->zoombox, &self->pane);
	zoombox_data_exchange(&self->zoombox, visualconfig_property(
		&self->workspace->config.visual, "zoom"));	
}

void mainstatusbar_init_status_label(MainStatusBar* self)
{
	statuslabel_init(&self->statusbarlabel, &self->pane);
	psy_ui_component_set_align(&self->statusbarlabel.component,
		psy_ui_ALIGN_CLIENT);
	workspace_connect_status(self->workspace, self,
		(fp_workspace_output)mainstatusbar_on_status);
}

void mainstatusbar_init_turnoff_button(MainStatusBar* self)
{
	psy_ui_button_init_text(&self->turnoff, &self->pane, "main.exit");
	psy_ui_button_load_resource(&self->turnoff, IDB_EXIT_LIGHT,
		IDB_EXIT_DARK, psy_ui_colour_white());
	psy_ui_component_set_align(psy_ui_button_base(&self->turnoff),
		psy_ui_ALIGN_RIGHT);		
}

void mainstatusbar_init_clock_bar(MainStatusBar* self)
{
	clockbar_init(&self->clockbar, &self->pane);
	psy_ui_component_set_align(clockbar_base(&self->clockbar),
		psy_ui_ALIGN_RIGHT);
	clockbar_start(&self->clockbar);
}

void mainstatusbar_init_kbd_help_button(MainStatusBar* self)
{	
	psy_ui_Margin margin;

	psy_ui_button_init_text(&self->togglekbdhelp, &self->pane, "main.kbd");
	margin = psy_ui_component_margin(psy_ui_button_base(&self->togglekbdhelp));
	psy_ui_margin_setright(&margin, psy_ui_value_make_ew(4.0));
	psy_ui_component_set_margin(psy_ui_button_base(&self->togglekbdhelp),
		 margin);
	psy_ui_component_set_align(psy_ui_button_base(&self->togglekbdhelp),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_button_load_resource(&self->togglekbdhelp, IDB_KBD, IDB_KBD,
		psy_ui_colour_white());
}

void mainstatusbar_init_terminal_button(MainStatusBar* self)
{		
	psy_ui_button_init_text(&self->toggleterminal, &self->pane, "Terminal");
	psy_ui_component_set_style_type(&self->toggleterminal.component,
		self->workspace->terminalstyleid);
	psy_ui_component_set_align(psy_ui_button_base(&self->toggleterminal),
		psy_ui_ALIGN_RIGHT);
	psy_ui_button_load_resource(&self->toggleterminal, IDB_TERM, IDB_TERM,
		psy_ui_colour_white());
}

void mainstatusbar_init_progress_bar(MainStatusBar* self)
{
	self->pluginscanprogress = -1;	
	workspace_connect_load_progress(self->workspace, self,
		(fp_workspace_songloadprogress)mainstatusbar_on_song_load_progress);
	psy_signal_connect(&self->workspace->pluginscanthread.signal_scanprogress, self,
		mainstatusbar_on_plugin_scan_progress);
}

void mainstatusbar_on_status(MainStatusBar* self, Workspace* sender,
	const char* text)
{
	assert(self);

	if (text) {
		psy_lock_enter(&self->outputlock);
		psy_strreset(&self->strbuffer, text);
		psy_lock_leave(&self->outputlock);
	}	
}

void mainstatusbar_update_terminal_button(MainStatusBar* self)
{
	if (psy_ui_componentstyle_style_id(&self->component.style,
			psy_ui_STYLESTATE_NONE) != self->workspace->terminalstyleid) {
		psy_ui_component_set_style_type(
			psy_ui_button_base(&self->toggleterminal),
			self->workspace->terminalstyleid);
		psy_ui_component_invalidate(psy_ui_button_base(&self->toggleterminal));
	}
}

void mainstatusbar_set_default_status_text(MainStatusBar* self,
	const char* text)
{
	psy_ui_label_set_text(&self->statusbarlabel.label, text);
	psy_ui_label_set_default_text(&self->statusbarlabel.label, text);
}

void mainstatusbar_on_song_load_progress(MainStatusBar* self,
	Workspace* sender, intptr_t progress)
{
	if (progress == -1) {
		workspace_output(self->workspace, "\n");
	}
	psy_ui_progressbar_set_progress(&self->statusbarlabel.progressbar,
		progress / 100.f);
}

void mainstatusbar_on_plugin_scan_progress(MainStatusBar* self,
	PluginScanThread* sender, int progress)
{
	self->pluginscanprogress = progress;
}

void mainstatusbar_idle(MainStatusBar* self)
{
	if (self->pluginscanprogress != -1) {
		if (self->pluginscanprogress == 0) {
			psy_ui_progressbar_set_progress(&self->statusbarlabel.progressbar,
				0);
			self->pluginscanprogress = -1;
		} else {
			psy_ui_progressbar_tick(&self->statusbarlabel.progressbar);
		}
	}		
	psy_lock_enter(&self->outputlock);		
	if (self->strbuffer) {
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
		psy_ui_label_set_text(&self->statusbarlabel.label,
			(const char*)self->strbuffer);
		psy_ui_label_fadeout(&self->statusbarlabel.label);
#endif		
		free(self->strbuffer);
		self->strbuffer = NULL;
	}
	psy_lock_leave(&self->outputlock);	
	mainstatusbar_update_terminal_button(self);
}
