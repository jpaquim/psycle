/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "mainframe.h"
/* host */
#include "cmdsgeneral.h"
#include "cmdsnotes.h"
#include "resources/resource.h"
#include "styles.h"
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

#ifdef PSYCLE_MAKE_DEFAULT_LANG
#include "defaultlang.h"
#endif


/* prototypes */
/* build */
static void mainframe_on_destroyed(MainFrame*);
static void mainframe_init_frame(MainFrame*);
static void mainframe_init_workspace(MainFrame*);
static void mainframe_init_layout(MainFrame*);
static void mainframe_init_terminal(MainFrame*);
static void mainframe_init_kbd_help(MainFrame*);
static void mainframe_init_status_bar(MainFrame*);
static void mainframe_init_view_statusbars(MainFrame*);
static void mainframe_connect_statusbar(MainFrame*);
static void mainframe_init_tabbars(MainFrame*);
static void mainframe_init_main_pane(MainFrame*);
static void mainframe_init_bars(MainFrame*);
static void mainframe_init_gear(MainFrame*);
static void mainframe_init_param_rack(MainFrame*);
static void mainframe_init_cpu_view(MainFrame*);
static void mainframe_init_midi_monitor(MainFrame*);
static void mainframe_init_step_sequencer_view(MainFrame*);
static void mainframe_init_seq_editor(MainFrame*);
static void mainframe_init_recent_view(MainFrame*);
static void mainframe_init_file_view(MainFrame*);
static void mainframe_init_sequence_view(MainFrame*);
static void mainframe_init_sequencer_bar(MainFrame*);
static void mainframe_init_plugin_editor(MainFrame*);
static void mainframe_connect_song(MainFrame*);
static void mainframe_init_minmaximize(MainFrame*);
static void mainframe_connect_workspace(MainFrame*);
static void mainframe_init_interpreter(MainFrame*);
/* events */
static void mainframe_on_key_down(MainFrame*, psy_ui_KeyboardEvent*);
static void mainframe_checkplaystartwithrctrl(MainFrame*, psy_ui_KeyboardEvent*);
static void mainframe_on_key_up(MainFrame*, psy_ui_KeyboardEvent*);
static void mainframe_delegate_keyboard(MainFrame*, intptr_t message,
	psy_ui_KeyboardEvent*);
static void mainframe_on_toggle_gear(MainFrame*, Workspace* sender);
static void mainframe_on_recent_songs(MainFrame*, psy_Property* sender);
static void mainframe_on_file_save_view(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_disk_op(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_plugin_editor(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_tabbar_changed(MainFrame*, psy_ui_TabBar* sender,
	uintptr_t tabindex);
static void mainframe_on_script_tabbar_changed(MainFrame*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void mainframe_on_song_changed(MainFrame*, Workspace* sender);
static void mainframe_on_view_selected(MainFrame*, Workspace*, uintptr_t view,
	uintptr_t section, int option);
static void mainframe_on_focus(MainFrame*);
static void mainframe_on_render(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_export(MainFrame*, psy_ui_Component* sender);
static void mainframe_update_songtitle(MainFrame*);
static void mainframe_on_timer(MainFrame*, uintptr_t timerid);
static bool mainframe_on_input(MainFrame*, InputHandler* sender);
static bool mainframe_on_notes(MainFrame*, InputHandler* sender);
static void mainframe_on_seq_editor(MainFrame*, psy_Property* sender);
static void mainframe_on_step_sequencer(MainFrame*, psy_Property* sender);
static void mainframe_on_toggle_scripts(MainFrame*, psy_ui_Component* sender);
static void mainframe_update_step_sequencer_buttons(MainFrame*);
static void mainframe_connect_step_sequencer_buttons(MainFrame*);
static void mainframe_update_seq_editor_buttons(MainFrame*);
static void mainframe_connect_seq_editor_buttons(MainFrame*);
static void mainframe_on_toggle_terminal(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_toggle_kbd_help(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_exit(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_songtracks_changed(MainFrame*, psy_audio_Patterns* sender,
	uintptr_t numsongtracks);
static bool mainframe_on_close(MainFrame*);
static void mainframe_on_check_unsaved(MainFrame*, ConfirmBox* sender,
	int option, int mode);
static void mainframe_on_file_load(MainFrame*, FileView* sender);
static void mainframe_on_gear_select(MainFrame*, Workspace* sender,
	psy_List* machinelist);
static void mainframe_on_drag_over(MainFrame*, psy_ui_DragEvent*);
static void mainframe_on_drop(MainFrame*, psy_ui_DragEvent*);
static void mainframe_on_mouse_down(MainFrame*, psy_ui_MouseEvent*);
static void mainframe_on_mouse_double_click(MainFrame*, psy_ui_MouseEvent* ev);
static void mainframe_on_mouse_move(MainFrame*, psy_ui_MouseEvent*);
static void mainframe_on_mouse_up(MainFrame*, psy_ui_MouseEvent* ev);
static bool mainframe_accept_frame_move(const MainFrame*, psy_ui_Component*
	target);
static bool mainframe_on_input_handler_callback(MainFrame*, int message,
	void* param1);
static void mainframe_plugineditor_on_focus(MainFrame*, psy_ui_Component* sender);
static void mainframe_seqeditor_on_float(MainFrame*, psy_ui_Button* sender);
static void mainframe_on_metronome_bar(MainFrame*, psy_Property* sender);
static void mainframe_on_trackscope_view(MainFrame*, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainFrame* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onclose =
			(psy_ui_fp_component_onclose)
			mainframe_on_close;
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			mainframe_on_destroyed;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			mainframe_on_key_down;
		vtable.onkeyup =
			(psy_ui_fp_component_on_key_event)
			mainframe_on_key_up;
		vtable.ondragover =
			(psy_ui_fp_component_ondragover)
			mainframe_on_drag_over;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			mainframe_on_mouse_down;
		vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			mainframe_on_mouse_double_click;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			mainframe_on_mouse_up;
		vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			mainframe_on_mouse_move;
		vtable.ondrop =
			(psy_ui_fp_component_ondrop)
			mainframe_on_drop;
		vtable.on_focus =
			(psy_ui_fp_component_event)
			mainframe_on_focus;
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			mainframe_on_timer;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(mainframe_base(self), &vtable);
}

/* implementation */
void mainframe_init(MainFrame* self)
{
	mainframe_init_frame(self);
	mainframe_init_workspace(self);
	mainframe_init_layout(self);
	mainframe_init_terminal(self);
	mainframe_init_kbd_help(self);
	mainframe_init_tabbars(self);
	mainframe_init_bars(self);	
	mainframe_init_main_pane(self);
	mainframe_init_gear(self);
	mainframe_init_param_rack(self);
	mainframe_init_cpu_view(self);
	mainframe_init_midi_monitor(self);
	mainframe_init_recent_view(self);
	mainframe_init_file_view(self);
	mainframe_init_sequence_view(self);
	mainframe_init_sequencer_bar(self);
	mainframe_init_step_sequencer_view(self);
	mainframe_init_seq_editor(self);
	mainframe_init_plugin_editor(self);
	mainframe_init_status_bar(self);
	mainframe_init_minmaximize(self);
	mainframe_update_songtitle(self);
	mainframe_init_interpreter(self);
	mainstatusbar_update_terminal_button(&self->statusbar);
	mainframe_connect_song(self);
	mainframe_connect_workspace(self);
	mainframe_update_step_sequencer_buttons(self);
	mainframe_connect_step_sequencer_buttons(self);
	mainframe_update_seq_editor_buttons(self);
	mainframe_connect_seq_editor_buttons(self);	
#ifdef PSYCLE_MAKE_DEFAULT_LANG
	save_translator_default();
	save_translator_template();
#endif
	if (!workspace_has_plugin_cache(&self->workspace)) {
		workspace_scan_plugins(&self->workspace);
	}
	startscript_init(&self->startscript, self);
	startscript_run(&self->startscript);
	psy_signal_connect(&self->scripttabbar.signal_change, self,
		mainframe_on_script_tabbar_changed);
	workspace_select_start_view(&self->workspace);
	self->machineview.wireview.centermaster = TRUE;	
}

void mainframe_init_frame(MainFrame* self)
{
	psy_ui_frame_init_main(mainframe_base(self));
	vtable_init(self);			
	psy_ui_component_doublebuffer(mainframe_base(self));
	psy_ui_app_setmain(psy_ui_app(), mainframe_base(self));
	psy_ui_component_seticonressource(mainframe_base(self), IDI_PSYCLEICON);
	init_host_styles(&psy_ui_appdefaults()->styles,
		psy_ui_defaults()->styles.theme);
	self->titlemodified = FALSE;
	self->allow_frame_move = FALSE;
	psy_ui_component_init(&self->pane, mainframe_base(self),
		mainframe_base(self));	
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	links_init(&self->links);	
}

void mainframe_on_destroyed(MainFrame* self)
{
	paramviews_dispose(&self->paramviews);
	startscript_dispose(&self->startscript);
	links_dispose(&self->links);
	workspace_dispose(&self->workspace);
	interpreter_dispose(&self->interpreter);
	psy_ui_app_stop(psy_ui_app());
}

MainFrame* mainframe_alloc(void)
{
	return (MainFrame*)malloc(sizeof(MainFrame));
}

MainFrame* mainframe_allocinit(void)
{
	MainFrame* rv;

	rv = mainframe_alloc();
	if (rv) {
		mainframe_init(rv);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void mainframe_init_workspace(MainFrame* self)
{
	workspace_init(&self->workspace, mainframe_base(self));
	paramviews_init(&self->paramviews, &self->component, &self->workspace);
	workspace_set_param_views(&self->workspace, &self->paramviews);
	workspace_load_recent_songs(&self->workspace);
	workspace_load_configuration(&self->workspace);
}

void mainframe_init_layout(MainFrame* self)
{	
	mainstatusbar_init(&self->statusbar, &self->pane, &self->workspace);
	psy_ui_component_set_align(mainstatusbar_base(&self->statusbar),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_init_align(&self->client, &self->pane, NULL,
		psy_ui_ALIGN_CLIENT);	
	psy_ui_component_init_align(&self->top, &self->pane,
		NULL, psy_ui_ALIGN_TOP);	
	psy_ui_component_set_default_align(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	mainviews_init(&self->mainviews, &self->client, &self->pane, &self->workspace);
	psy_ui_component_set_align(&self->mainviews.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_init_align(&self->right, &self->client, NULL,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_show(&self->statusbar.component);
}

void mainframe_init_terminal(MainFrame* self)
{
	psy_ui_terminal_init(&self->terminal, &self->pane);
	psy_ui_component_set_align(psy_ui_terminal_base(&self->terminal),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_preferred_size(psy_ui_terminal_base(&self->terminal),
		psy_ui_size_zero());
	psy_ui_splitter_init(&self->splitbarterminal, &self->pane);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitbarterminal),
		psy_ui_ALIGN_BOTTOM);
	workspace_set_terminal_output(&self->workspace,
		psy_ui_terminal_logger_base(&self->terminal));	
}

void mainframe_init_kbd_help(MainFrame* self)
{
	kbdhelp_init(&self->kbdhelp, &self->pane, &self->workspace);
	psy_ui_component_set_align(kbdhelp_base(&self->kbdhelp),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
}

void mainframe_init_status_bar(MainFrame* self)
{
	mainframe_init_view_statusbars(self);
	mainframe_connect_statusbar(self);
}

void mainframe_init_minmaximize(MainFrame* self)
{
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar, &self->left);
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar,
		songbar_base(&self->songbar));
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar,
		machinebar_base(&self->machinebar));
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar,
		trackscopeview_base(&self->trackscopeview));
}

void mainframe_init_view_statusbars(MainFrame* self)
{
	machineviewbar_init(&self->machineviewbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars),
		&self->workspace);
	patternviewbar_init(&self->patternviewbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars),
		&self->workspace.config.visual.patview,
		&self->workspace);	
	sampleeditorbar_init(&self->samplesview.sampleeditor.sampleeditortbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars),
		&self->samplesview.sampleeditor, &self->workspace);
	samplesview_connectstatusbar(&self->samplesview);
	instrumentsviewbar_init(&self->instrumentsviewbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars), &self->workspace);
	instrumentsview_setstatusbar(&self->instrumentsview,
		&self->instrumentsviewbar);
	plugineditorbar_init(&self->plugineditorbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars));
	plugineditorbar_set_editor(&self->plugineditorbar, &self->plugineditor);
	psy_ui_notebook_select(&self->statusbar.viewstatusbars, 0);
}

void mainframe_connect_statusbar(MainFrame* self)
{
	psy_signal_connect(&self->statusbar.toggleterminal.signal_clicked,
		self, mainframe_on_toggle_terminal);
	psy_signal_connect(&self->statusbar.turnoff.signal_clicked, self,
		mainframe_on_exit);
	psy_signal_connect(&self->statusbar.togglekbdhelp.signal_clicked, self,
		mainframe_on_toggle_kbd_help);
}

void mainframe_init_bars(MainFrame* self)
{
	psy_ui_Margin margin;

	/* rows */
	psy_ui_component_init_align(&self->toprows, &self->top, NULL,
		psy_ui_ALIGN_TOP);	
	psy_ui_component_set_style_type(&self->toprows, STYLE_TOPROWS);
	psy_ui_component_set_default_align(&self->toprows, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	/* row0 */
	psy_ui_component_init(&self->toprow0, &self->toprows, NULL);
	/* vugroup */
	vubar_init(&self->vubar, &self->toprow0, &self->workspace);	
	psy_ui_component_set_align(&self->vubar.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_style_type(&self->toprow0, STYLE_TOPROW0);
	psy_ui_component_init(&self->toprow0_client, &self->toprow0, NULL);
	psy_ui_component_set_align(&self->toprow0_client, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->toprow0_bars, &self->toprow0_client, NULL);
	psy_ui_component_set_align(&self->toprow0_bars, psy_ui_ALIGN_TOP);
	psy_ui_component_set_style_type(&self->toprow0_bars, STYLE_TOPROW0_BARS);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_set_default_align(&self->toprow0_bars, psy_ui_ALIGN_LEFT,
		margin);
	filebar_init(&self->filebar, &self->toprow0_bars, &self->workspace);
	undoredobar_init(&self->undoredobar, &self->toprow0_bars, &self->workspace);
	playbar_init(&self->playbar, &self->toprow0_bars, &self->workspace);	
	playposbar_init(&self->playposbar, &self->toprow0_bars,
		workspace_player(&self->workspace));
	metronomebar_init(&self->metronomebar, &self->toprow0_bars, &self->workspace);
	if (!metronomeconfig_showmetronomebar(&self->workspace.config.metronome)) {
		psy_ui_component_hide(metronomebar_base(&self->metronomebar));
	}
	metronomeconfig_connect(&self->workspace.config.metronome,
		"showmetronome", self, mainframe_on_metronome_bar);
	margin.right = psy_ui_value_make_px(0);
	psy_ui_component_set_margin(metronomebar_base(&self->metronomebar), margin);
	/* row1 */	
	songbar_init(&self->songbar, &self->toprows, &self->workspace);
	psy_ui_component_set_style_type(&self->songbar.component, STYLE_TOPROW1);
	/* row2 */	
	machinebar_init(&self->machinebar, &self->toprows, &self->workspace);
	psy_ui_component_set_style_type(&self->machinebar.component, STYLE_TOPROW2);
	/* scopebar */
	trackscopeview_init(&self->trackscopeview, &self->top, &self->workspace);
	if (!patternviewconfig_show_trackscopes(psycleconfig_patview(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopes_stop(&self->trackscopeview.scopes);
	}
	patternviewconfig_connect(&self->workspace.config.visual.patview,
		"trackscopes", self, mainframe_on_trackscope_view);
	psy_ui_component_init(&self->topspacer, &self->pane, NULL);
	psy_ui_component_set_align(&self->topspacer, psy_ui_ALIGN_TOP);	
	psy_ui_component_set_preferred_size(&self->topspacer,
		psy_ui_size_make_em(0.0, 0.5));
	psy_ui_component_set_style_type(&self->topspacer, STYLE_TOP);
}

void mainframe_init_tabbars(MainFrame* self)
{	
	psy_ui_tabbar_init(&self->scripttabbar, &self->mainviews.component);
	psy_ui_component_set_align(&self->scripttabbar.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->scripttabbar.component);	
	psy_ui_button_init_text_connect(&self->togglescripts, &self->mainviews.mainviewbar.tabbars,
		"main.scripts", self, mainframe_on_toggle_scripts);
	psy_ui_component_set_align(psy_ui_button_base(&self->togglescripts),
		psy_ui_ALIGN_LEFT);
}

void mainframe_init_main_pane(MainFrame* self)
{	
	machineview_init(&self->machineview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	patternview_init(&self->patternview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	samplesview_init(&self->samplesview, psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	instrumentview_init(&self->instrumentsview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	songpropertiesview_init(&self->songpropertiesview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	propertiesview_init(&self->settingsview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace.config.config, 3, TRUE,
		&self->workspace.inputhandler);
	psy_ui_component_set_id(&self->settingsview.component,
		VIEW_ID_SETTINGSVIEW);
	psy_ui_component_set_title(&self->settingsview.component, "main.settings");	
	helpview_init(&self->helpview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	renderview_init(&self->renderview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	psy_signal_connect(&self->filebar.renderbutton.signal_clicked, self,
		mainframe_on_render);
	exportview_init(&self->exportview,
		psy_ui_notebook_base(&self->mainviews.notebook),
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),
		&self->workspace);
	psy_signal_connect(&self->filebar.exportbutton.signal_clicked, self,
		mainframe_on_export);
	psy_signal_connect(&self->workspace.signal_viewselected, self,
		mainframe_on_view_selected);
	confirmbox_init(&self->checkunsavedbox,
		psy_ui_notebook_base(&self->mainviews.notebook));
	psy_ui_component_set_id(confirmbox_base(&self->checkunsavedbox),
		VIEW_ID_CHECKUNSAVED);
	psy_signal_connect(&self->mainviews.mainviewbar.tabbar.signal_change, self,
		mainframe_on_tabbar_changed);	
}

void mainframe_init_gear(MainFrame* self)
{
	gear_init(&self->gear, &self->client, &self->paramviews, &self->workspace);
	psy_ui_component_hide(gear_base(&self->gear));
	psy_ui_component_set_align(gear_base(&self->gear), psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_init(&self->gearsplitter, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->gearsplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_settoggle(&self->gearsplitter, gear_base(&self->gear));
	psy_ui_splitter_setbutton(&self->gearsplitter,
		&self->machinebar.gear);
	psy_ui_component_hide(psy_ui_splitter_base(&self->gearsplitter));
}

void mainframe_init_param_rack(MainFrame* self)
{
	paramrack_init(&self->paramrack, &self->client, &self->workspace);
	psy_ui_component_hide(paramrack_base(&self->paramrack));
	psy_ui_component_set_align(&self->paramrack.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_splitter_init(&self->splitbarparamrack, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitbarparamrack),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_splitter_settoggle(&self->splitbarparamrack,
		&self->paramrack.component);
	psy_ui_splitter_setbutton(&self->splitbarparamrack,
		&self->machinebar.dock);
	psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarparamrack));
}

void mainframe_init_cpu_view(MainFrame* self)
{
	cpuview_init(&self->cpuview, &self->client, &self->workspace);
	psy_ui_component_hide(cpuview_base(&self->cpuview));
	psy_ui_component_set_align(cpuview_base(&self->cpuview), psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_init(&self->cpusplitter, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->cpusplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_settoggle(&self->cpusplitter,
		cpuview_base(&self->cpuview));
	psy_ui_splitter_setbutton(&self->cpusplitter,
		&self->machinebar.cpu);
	psy_ui_component_hide(psy_ui_splitter_base(&self->cpusplitter));
}

void mainframe_init_midi_monitor(MainFrame* self)
{
	midimonitor_init(&self->midimonitor, &self->client, &self->workspace);
	psy_ui_component_hide(midimonitor_base(&self->midimonitor));
	psy_ui_component_set_align(midimonitor_base(&self->midimonitor), psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_init(&self->midisplitter, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->midisplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_settoggle(&self->midisplitter,
		midimonitor_base(&self->midimonitor));
	psy_ui_splitter_setbutton(&self->midisplitter,
		&self->machinebar.midi);
	psy_ui_component_hide(psy_ui_splitter_base(&self->midisplitter));
}

void mainframe_init_step_sequencer_view(MainFrame* self)
{
	stepsequencerview_init(&self->stepsequencerview, &self->pane,
		&self->workspace);
	psy_ui_component_set_align(stepsequencerview_base(&self->stepsequencerview),
		psy_ui_ALIGN_BOTTOM);
	if (!generalconfig_showstepsequencer(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(stepsequencerview_base(&self->stepsequencerview));
	}	
}

void mainframe_init_seq_editor(MainFrame* self)
{
	seqeditor_init(&self->seqeditor, &self->pane, &self->workspace);
	psy_ui_component_set_align(seqeditor_base(&self->seqeditor),
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_component_setmaximumsize(seqeditor_base(&self->seqeditor),
		psy_ui_size_make(psy_ui_value_zero(), psy_ui_value_make_ph(0.7)));
	psy_ui_splitter_init(&self->splitseqeditor, &self->pane);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitseqeditor),
		psy_ui_ALIGN_BOTTOM);
	if (!generalconfig_showsequenceedit(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(seqeditor_base(&self->seqeditor));
		psy_ui_component_hide(psy_ui_splitter_base(&self->splitseqeditor));
	}
	/* connect float */
	psy_signal_connect(&self->seqeditor.toolbar.view_float.signal_clicked,
		self, mainframe_seqeditor_on_float);
}

void mainframe_init_recent_view(MainFrame* self)
{
	playlistview_init(&self->playlist, &self->pane,
		psy_ui_notebook_base(&self->mainviews.mainviewbar.viewtabbars),		
		&self->workspace);
	psy_ui_component_set_align(playlistview_base(&self->playlist),
		psy_ui_ALIGN_LEFT);
	 psy_ui_splitter_init(&self->playlistsplitter, &self->pane);
	 psy_ui_component_set_align(psy_ui_splitter_base(&self->playlistsplitter),
		psy_ui_ALIGN_LEFT);
	if (!generalconfig_showplaylist(&self->workspace.config.general)) {
		psy_ui_component_hide(playlistview_base(&self->playlist));
		psy_ui_component_hide(psy_ui_splitter_base(&self->playlistsplitter));
	}
	generalconfig_connect(&self->workspace.config.general,
		"showplaylist", self, mainframe_on_recent_songs);	
}

void mainframe_init_file_view(MainFrame* self)
{
	/* ft2 style file load view */
	fileview_init(&self->fileview, &self->pane);
#if defined(DIVERSALIS__OS__MICROSOFT)	
	fileview_set_directory(&self->fileview,
		dirconfig_songs(&self->workspace.config.directories));
#endif		
	psy_ui_component_set_align(fileview_base(&self->fileview),
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->fileview.save.signal_clicked, self,
		mainframe_on_file_save_view);
	psy_ui_component_hide(&self->fileview.component);
	psy_signal_connect(&self->fileview.signal_selected,
		self, mainframe_on_file_load);
	if (keyboardmiscconfig_ft2fileexplorer(psycleconfig_misc(
		workspace_conf(&self->workspace)))) {
		filebar_useft2fileexplorer(&self->filebar);
	}
	psy_signal_connect(&self->filebar.diskop.signal_clicked, self,
		mainframe_on_disk_op);
}

void mainframe_init_sequence_view(MainFrame* self)
{
	psy_ui_Size size;
	psy_ui_component_init_align(&self->left, &self->pane, NULL,
		psy_ui_ALIGN_LEFT);	
	psy_ui_splitter_init(&self->splitbar, &self->pane);
	seqview_init(&self->sequenceview, &self->left, &self->workspace);
	psy_ui_component_set_align(seqview_base(&self->sequenceview),
		psy_ui_ALIGN_CLIENT);
	size = psy_ui_component_preferred_size(seqview_base(&self->sequenceview),
		NULL);
	psy_ui_component_set_preferred_size(&self->left,
		psy_ui_size_make(size.width, psy_ui_value_zero()));
}

void mainframe_init_sequencer_bar(MainFrame* self)
{
	sequencerbar_init(&self->sequencerbar, &self->left,
		&self->workspace.config.misc, &self->workspace.config.general);
	psy_ui_component_set_align(sequencerbar_base(&self->sequencerbar),
		psy_ui_ALIGN_BOTTOM);	
}

void mainframe_init_plugin_editor(MainFrame* self)
{
	plugineditor_init(&self->plugineditor, &self->pane, &self->workspace);	
	psy_ui_component_set_align(plugineditor_base(&self->plugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(plugineditor_base(&self->plugineditor));
	psy_signal_connect(&self->machinebar.editor.signal_clicked, self,
		mainframe_on_plugin_editor);                   
	psy_ui_splitter_init(&self->splitbarplugineditor, &self->pane);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitbarplugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarplugineditor));
	psy_signal_connect(&self->plugineditor.editor.textarea.pane.component.signal_focus,
		self, mainframe_plugineditor_on_focus);
}

void mainframe_connect_workspace(MainFrame* self)
{
	workspace_configure_host(&self->workspace);
	inputhandler_connect(&self->workspace.inputhandler,
		INPUTHANDLER_IMM, psy_EVENTDRIVER_CMD, "general",
		psy_INDEX_INVALID, self, (fp_inputhandler_input)mainframe_on_input);
	inputhandler_connect(&self->workspace.inputhandler,
		INPUTHANDLER_IMM, psy_EVENTDRIVER_CMD, "notes",
		psy_INDEX_INVALID, self, (fp_inputhandler_input)mainframe_on_notes);
	inputhandler_connect_host(&self->workspace.inputhandler,
		self, (fp_inputhandler_hostcallback)mainframe_on_input_handler_callback);	
	psy_signal_connect(&self->workspace.signal_togglegear, self,
		mainframe_on_toggle_gear);
	psy_signal_connect(&self->checkunsavedbox.signal_execute, self,
		mainframe_on_check_unsaved);	
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_on_song_changed);
	psy_signal_connect(&self->workspace.signal_gearselect, self,
		mainframe_on_gear_select);	
	psy_ui_component_start_timer(mainframe_base(self), 0, 50);
}

void mainframe_init_interpreter(MainFrame* self)
{
	interpreter_init(&self->interpreter, &self->workspace);
	interpreter_start(&self->interpreter);
}

bool mainframe_on_input(MainFrame* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);
	switch (cmd.id) {
	case CMD_IMM_HELPSHORTCUT:
		mainframe_on_toggle_kbd_help(self, mainframe_base(self));
		return 1;
	case CMD_IMM_INFOMACHINE:
		if (self->workspace.song) {
			paramviews_show(&self->paramviews,
				psy_audio_machines_selected(&self->workspace.song->machines));
		}
		break;
	case CMD_IMM_INFOPATTERN:
		if (workspace_current_view(&self->workspace).id != VIEW_ID_PATTERNVIEW) {
			workspace_select_view(&self->workspace,
				viewindex_make(VIEW_ID_PATTERNVIEW, 0, 0, psy_INDEX_INVALID));
		}
		if (!psy_ui_component_visible(&self->patternview.properties.component)) {
			psy_ui_Tab* tab;
			psy_ui_component_toggle_visibility(&self->patternview.properties.component);

			tab = psy_ui_tabbar_tab(&self->patternview.tabbar.tabbar, 5);
			if (tab) {
				tab->checkstate = TABCHECKSTATE_ON;
				psy_ui_component_invalidate(psy_ui_tabbar_base(
					&self->patternview.tabbar.tabbar));
			}
		}
		psy_ui_component_set_focus(&self->patternview.properties.component);
		return 1;
	case CMD_IMM_MAXPATTERN:
		mainviewbar_toggle_minmaximze(&self->mainviews.mainviewbar);
		return 1;
	case CMD_IMM_TERMINAL:
		mainframe_on_toggle_terminal(self, mainframe_base(self));
		return 1;
	case CMD_IMM_FULLSCREEN:
		psy_ui_component_togglefullscreen(mainframe_base(self));		
		return 1;
		break;
	case CMD_EDT_EDITQUANTIZEDEC:
		workspace_edit_quantize_change(&self->workspace, -1);
		patterncursorstepbox_update(&self->patternviewbar.cursorstep);
		return 1;
	case CMD_EDT_EDITQUANTIZEINC:
		workspace_edit_quantize_change(&self->workspace, 1);
		patterncursorstepbox_update(&self->patternviewbar.cursorstep);
		return 1;
	}
	workspace_on_input(&self->workspace, cmd.id);
	return 0;
}

bool mainframe_on_notes(MainFrame* self, InputHandler* sender)
{
	psy_audio_PatternEvent ev;
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);
	if (cmd.id >= 0 && cmd.id < 256) {
		ev = psy_audio_player_pattern_event(workspace_player(&self->workspace),
			(uint8_t)cmd.id);
		if (cmd.id >= CMD_NOTE_OFF_C_0) {
			ev.note = CMD_NOTE_STOP;			
		}
		psy_audio_player_playevent(workspace_player(&self->workspace), &ev);
		return 1;
	}
	return 0;
}

void mainframe_on_song_changed(MainFrame* self, Workspace* sender)
{
	if (workspace_song_has_file(sender)) {
		if (generalconfig_showing_song_info_on_load(psycleconfig_general(
				workspace_conf(sender)))) {
			workspace_select_view(&self->workspace,
				viewindex_make(VIEW_ID_SONGPROPERTIES, psy_INDEX_INVALID,
					psy_INDEX_INVALID, psy_INDEX_INVALID));			
		}
	}
	mainframe_update_songtitle(self);
	mainframe_connect_song(self);
	vubar_reset(&self->vubar);
	clockbar_reset(&self->statusbar.clockbar);
	psy_ui_component_align(&self->client);
	psy_ui_component_align(mainframe_base(self));
	if (!workspace_song_has_file(sender)) {
		machinewireview_centermaster(&self->machineview.wireview);
	}
}

void mainframe_connect_song(MainFrame* self)
{
	if (workspace_song(&self->workspace)) {		
		psy_signal_connect(
			&workspace_song(&self->workspace)->patterns.signal_numsongtrackschanged,
			self, mainframe_on_songtracks_changed);
	}
}

void mainframe_update_songtitle(MainFrame* self)
{
	char title[512];

	workspace_app_title(&self->workspace, title, 512);
	psy_ui_component_set_title(mainframe_base(self), title);
	mainstatusbar_set_default_status_text(&self->statusbar,
		workspace_song_title(&self->workspace));
}

void mainframe_on_toggle_gear(MainFrame* self, Workspace* sender)
{
	if (psy_ui_component_visible(&self->gearsplitter.component)) {
		psy_ui_component_hide(&self->gearsplitter.component);
	} else {
		psy_ui_component_show(&self->gearsplitter.component);
	}
}

void mainframe_on_recent_songs(MainFrame* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show_align(playlistview_base(&self->playlist));
		psy_ui_component_show_align(&self->playlistsplitter.component);
	} else {
		psy_ui_component_hide_align(playlistview_base(&self->playlist));
		psy_ui_component_hide_align(&self->playlistsplitter.component);
	}	
}

void mainframe_on_file_save_view(MainFrame* self, psy_ui_Component* sender)
{
	char path[4096];

	psy_ui_component_show_align(fileview_base(&self->fileview));
	fileview_filename(&self->fileview, path, 4096);
	workspace_save_song(&self->workspace, path);	
}

void mainframe_on_disk_op(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_toggle_visibility(&self->fileview.component);
}

void mainframe_on_plugin_editor(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->plugineditor.component)) {
		psy_ui_button_disable_highlight(&self->machinebar.editor);
		psy_ui_component_hide(&self->splitbarplugineditor.component);
	} else {
		psy_ui_button_highlight(&self->machinebar.editor);
		psy_ui_component_show(&self->splitbarplugineditor.component);
	}
	psy_ui_component_toggle_visibility(&self->plugineditor.component);
}

void mainframe_on_render(MainFrame* self, psy_ui_Component* sender)
{
	workspace_select_view(&self->workspace, viewindex_make(
		VIEW_ID_RENDERVIEW, 0, psy_INDEX_INVALID, psy_INDEX_INVALID));
}

void mainframe_on_export(MainFrame* self, psy_ui_Component* sender)
{
	workspace_select_view(&self->workspace, viewindex_make(
		VIEW_ID_EXPORTVIEW, 0, psy_INDEX_INVALID, psy_INDEX_INVALID));
}

void mainframe_on_timer(MainFrame* self, uintptr_t timerid)
{	
	vubar_idle(&self->vubar);
	workspace_idle(&self->workspace);
	mainstatusbar_idle(&self->statusbar);
	trackscopeview_idle(&self->trackscopeview);
	seqview_idle(&self->sequenceview);
	plugineditorbar_idle(&self->plugineditorbar);
 	playposbar_idle(&self->playposbar);	
	machineview_idle(&self->machineview);	
	if (self->titlemodified != workspace_song_modified(&self->workspace)) {
		self->titlemodified = workspace_song_modified(&self->workspace);
		mainframe_update_songtitle(self);
	}
}

void mainframe_on_view_selected(MainFrame* self, Workspace* sender, uintptr_t index,
	uintptr_t section, int options)
{
	psy_ui_Component* view;

	if (index == VIEW_ID_CHECKUNSAVED) {
		if (options == CONFIRM_CLOSE) {
			self->checkunsavedbox.mode = (ConfirmBoxAction)options;
			confirmbox_set_labels(&self->checkunsavedbox,
				"msg.psyexit", "msg.saveexit", "msg.nosaveexit");
		} else if (options == CONFIRM_NEW) {
			self->checkunsavedbox.mode = (ConfirmBoxAction)options;
			confirmbox_set_labels(&self->checkunsavedbox,
				"msg.newsong", "msg.savenew", "msg.nosavenew");
		} else if (options == CONFIRM_LOAD) {
			self->checkunsavedbox.mode = (ConfirmBoxAction)options;
			confirmbox_set_labels(&self->checkunsavedbox,
				"msg.loadsong", "msg.saveload", "msg.nosaveload");
		} else if (options == CONFIRM_SEQUENCECLEAR) {
			self->checkunsavedbox.mode = (ConfirmBoxAction)options;
			confirmbox_set_labels(&self->checkunsavedbox,
				"msg.seqclear", "msg.yes", "msg.no");
		}
	}
	if (index == psy_INDEX_INVALID) {
		view = psy_ui_notebook_active_page(&self->mainviews.notebook);
	} else {
		psy_ui_notebook_select_by_component_id(&self->mainviews.notebook, index);
		view = psy_ui_notebook_active_page(&self->mainviews.notebook);
	}
	if (view) {
		if (index != psy_INDEX_INVALID) {
			psy_ui_tabbar_select(&self->mainviews.mainviewbar.tabbar, index);
		}
		if (section != psy_INDEX_INVALID) {
			psy_ui_component_select_section(view, section, options);
		}
		psy_ui_component_set_focus(view);
	}	
}

void mainframe_on_focus(MainFrame* self)
{
	psy_ui_Component* currview;

	currview = psy_ui_notebook_active_page(&self->mainviews.notebook);
	if (currview) {
		psy_ui_component_set_focus(currview);
	}
}

void mainframe_on_tabbar_changed(MainFrame* self, psy_ui_TabBar* sender,
	uintptr_t tabindex)
{
	psy_ui_Component* component;
	psy_ui_Tab* tab;
	
	tab = psy_ui_tabbar_tab(sender, tabindex);
	if (!tab) {
		return;
	}	
	psy_ui_component_select_section(&self->mainviews.notebook.component,
		psy_ui_tab_target_id(tab), psy_INDEX_INVALID);	
	psy_ui_notebook_select(&self->statusbar.viewstatusbars, tabindex);
	psy_ui_notebook_select(&self->mainviews.mainviewbar.viewtabbars, tabindex);
	component = psy_ui_notebook_active_page(&self->mainviews.notebook);
	if (component) {
		workspace_on_view_changed(&self->workspace, viewindex_make(
			tabindex, psy_ui_component_section(component), psy_INDEX_INVALID,
			psy_INDEX_INVALID));
		psy_ui_component_set_focus(component);
	}
	psy_ui_component_align(&self->mainviews.component);
	psy_ui_component_invalidate(&self->mainviews.component);	
}

void mainframe_on_script_tabbar_changed(MainFrame* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	const Link* link;

	link = links_at(&self->links, tabindex);
	if (link) {
		psy_audio_Machine* machine;
		
		machine = psy_audio_machinefactory_makemachinefrompath(
			&self->workspace.machinefactory, psy_audio_LUA, link->dllname_,
			0, 0);
		if (machine) {
			psy_audio_machine_sethostview(machine, &self->mainviews.notebook.component);
		}
	}
}

void mainframe_on_songtracks_changed(MainFrame* self, psy_audio_Patterns* sender,
	uintptr_t numsongtracks)
{
	/* TrackScopes can change its height, realign mainframe */
	psy_ui_component_align(trackscopeview_base(&self->trackscopeview));
	psy_ui_component_align(&self->pane);
	psy_ui_component_invalidate(&self->pane);
}

/*
** called if a button is clicked in the checkunsavedbox
** option: which button pressed
** mode  : source of request(app close, song load, song new)
*/
void mainframe_on_check_unsaved(MainFrame* self, ConfirmBox* sender,
	int option, int mode)
{
	switch (option) {
	case CONFIRM_YES:
		if (mode == CONFIRM_SEQUENCECLEAR) {
			workspace_restore_view(&self->workspace);
			seqview_clear(&self->sequenceview);
		} else if (workspace_save_song_fileselect(&self->workspace)) {
			if (mode == CONFIRM_CLOSE) {
				psy_ui_app_close(psy_ui_app());
			} else if (mode == CONFIRM_LOAD) {
				if (keyboardmiscconfig_ft2fileexplorer(psycleconfig_misc(
					workspace_conf(&self->workspace)))) {
					char path[4096];
					
					fileview_filename(&self->fileview, path, 4096);
					workspace_load_song(&self->workspace, path,
						generalconfig_playsongafterload(psycleconfig_general(
							workspace_conf(&self->workspace))));
				} else {
					workspace_load_song_fileselect(&self->workspace);
				}
			} else if (mode == CONFIRM_NEW) {
				workspace_newsong(&self->workspace);
			}
		}
		break;
	case CONFIRM_NO: {
		if (mode == CONFIRM_SEQUENCECLEAR) {
			workspace_restore_view(&self->workspace);
		} else {
			self->workspace.modified_without_undo = FALSE;
			self->workspace.undo_save_point = psy_list_size(
				self->workspace.undoredo.undo);
			self->workspace.machines_undo_save_point = psy_list_size(
				self->workspace.song->machines.undoredo.undo);
			if (mode == CONFIRM_CLOSE) {				
				psy_ui_app_close(psy_ui_app());
			} else if (mode == CONFIRM_LOAD) {
				if (keyboardmiscconfig_ft2fileexplorer(psycleconfig_misc(
					workspace_conf(&self->workspace)))) {
					char path[4096];

					fileview_filename(&self->fileview, path, 4096);
					workspace_load_song(&self->workspace,
						path,
						generalconfig_playsongafterload(psycleconfig_general(
							workspace_conf(&self->workspace))));
				} else {
					workspace_load_song_fileselect(&self->workspace);
				}
			} else if (mode == CONFIRM_NEW) {
				workspace_newsong(&self->workspace);
			}
		}
		break; }
	case CONFIRM_CONTINUE:
		workspace_restore_view(&self->workspace);
		break;
	default:
		break;
	}
}

void mainframe_on_exit(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_app_close(psy_ui_app());
}

bool mainframe_on_close(MainFrame* self)
{
	workspace_save_configuration(&self->workspace);
	if (keyboardmiscconfig_savereminder(&self->workspace.config.misc) &&
			workspace_song_modified(&self->workspace)) {
		workspace_select_view(&self->workspace,
			viewindex_make(VIEW_ID_CHECKUNSAVED, 0,
			CONFIRM_CLOSE, psy_INDEX_INVALID));
		return FALSE;
	}
	psy_ui_component_stop_timer(mainframe_base(self), 0);
	return TRUE;
}

void mainframe_on_seq_editor(MainFrame* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show_align(seqeditor_base(&self->seqeditor));
		psy_ui_component_show_align(&self->splitseqeditor.component);
	} else {
		psy_ui_component_hide_align(seqeditor_base(&self->seqeditor));
		psy_ui_component_hide_align(&self->splitseqeditor.component);
	}
	mainframe_update_seq_editor_buttons(self);
}

void mainframe_update_seq_editor_buttons(MainFrame* self)
{
	if (generalconfig_showsequenceedit(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_button_set_text(&self->sequencerbar.toggleseqedit,
			"seqview.hideseqeditor");
		psy_ui_button_highlight(&self->sequencerbar.toggleseqedit);
		psy_ui_button_set_icon(&self->sequencerbar.toggleseqedit,
			psy_ui_ICON_LESS);		
	} else {
		psy_ui_button_set_text(&self->sequencerbar.toggleseqedit,
			"seqview.showseqeditor");
		psy_ui_button_disable_highlight(
			&self->sequencerbar.toggleseqedit);
		psy_ui_button_set_icon(&self->sequencerbar.toggleseqedit,
			psy_ui_ICON_MORE);
	}
}

void mainframe_connect_seq_editor_buttons(MainFrame* self)
{	
	generalconfig_connect(&self->workspace.config.general, 
		"showsequenceedit", self, mainframe_on_seq_editor);	
	psy_ui_button_data_exchange(&self->sequencerbar.toggleseqedit,
		generalconfig_property(psycleconfig_general(workspace_conf(
			&self->workspace)), "showsequenceedit"));
}

void mainframe_on_toggle_scripts(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_toggle_visibility(psy_ui_tabbar_base(&self->scripttabbar));
}

void mainframe_on_step_sequencer(MainFrame* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show_align(stepsequencerview_base(
			&self->stepsequencerview));		
	} else {
		psy_ui_component_hide_align(stepsequencerview_base(
			&self->stepsequencerview));		
	}
	mainframe_update_step_sequencer_buttons(self);
}

void mainframe_update_step_sequencer_buttons(MainFrame* self)
{
	if (generalconfig_showstepsequencer(psycleconfig_general(
			workspace_conf(&self->workspace)))) {
		psy_ui_button_set_text(&self->sequencerbar.togglestepseq,
			"seqview.hidestepsequencer");
		psy_ui_button_highlight(&self->sequencerbar.togglestepseq);
		psy_ui_button_set_icon(&self->sequencerbar.togglestepseq,
			psy_ui_ICON_LESS);
	} else {
		psy_ui_button_set_text(&self->sequencerbar.togglestepseq,
			"seqview.showstepsequencer");
		psy_ui_button_disable_highlight(
			&self->sequencerbar.togglestepseq);
		psy_ui_button_set_icon(&self->sequencerbar.togglestepseq,
			psy_ui_ICON_MORE);
	}
}

void mainframe_connect_step_sequencer_buttons(MainFrame* self)
{
	generalconfig_connect(&self->workspace.config.general,
		"showstepsequencer", self, mainframe_on_step_sequencer);
	psy_ui_button_data_exchange(&self->sequencerbar.togglestepseq,
		generalconfig_property(psycleconfig_general(workspace_conf(
			&self->workspace)), "showstepsequencer"));	
}

void mainframe_on_toggle_terminal(MainFrame* self, psy_ui_Component* sender)
{
	if (!psy_ui_isvaluezero(psy_ui_component_scroll_size(
			&self->terminal.component).height)) {
		psy_ui_component_set_preferred_size(&self->terminal.component,
			psy_ui_size_zero());
		self->workspace.terminalstyleid = STYLE_TERM_BUTTON;
	} else {
		psy_ui_component_set_preferred_size(&self->terminal.component,
			psy_ui_size_make(psy_ui_value_make_px(0),
				psy_ui_value_make_eh(10.0)));
	}
	psy_ui_component_align(mainframe_base(self));
	psy_ui_component_invalidate(mainframe_base(self));
}

void mainframe_on_toggle_kbd_help(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_toggle_visibility(kbdhelp_base(&self->kbdhelp));
}

void mainframe_on_file_load(MainFrame* self, FileView* sender)
{
	char path[4096];	

	fileview_filename(sender, path, 4096);
	if (psy_strlen(path) == 0) {
		return;
	}
	if (keyboardmiscconfig_savereminder(&self->workspace.config.misc) &&
			workspace_song_modified(&self->workspace)) {
		workspace_select_view(&self->workspace,
			viewindex_make(VIEW_ID_CHECKUNSAVED, SECTION_ID_FILEVIEW,
			CONFIRM_LOAD, psy_INDEX_INVALID));
	} else {
		workspace_load_song(&self->workspace,
			path,
			generalconfig_playsongafterload(psycleconfig_general(
				workspace_conf(&self->workspace))));
	}
}

void mainframe_on_key_down(MainFrame* self, psy_ui_KeyboardEvent* ev)
{	
	/* TODO add immediate mode */
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		if (psy_ui_component_has_focus(&self->sequenceview.listview.component)) {
			psy_ui_Component* currview;

			currview = psy_ui_notebook_active_page(&self->mainviews.notebook);
			if (currview) {
				psy_ui_component_set_focus(currview);
			}
		} else {
			psy_ui_component_set_focus(&self->sequenceview.listview.component);
		}
	} else {
		mainframe_checkplaystartwithrctrl(self, ev);
		mainframe_delegate_keyboard(self, psy_EVENTDRIVER_PRESS, ev);
	}
}

void mainframe_checkplaystartwithrctrl(MainFrame* self, psy_ui_KeyboardEvent* ev)
{
	if (keyboardmiscconfig_playstartwithrctrl(
			&self->workspace.config.misc)) {
		if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_CONTROL) {
			/* todo: this win32 detection only */
			int extended = (psy_ui_keyboardevent_keydata(ev) & 0x01000000) != 0;
			if (extended) {
				/* right ctrl */
				psy_audio_player_start_currseqpos(workspace_player(
					&self->workspace));				
				return;
			}
		} else if (psy_audio_player_playing(&self->workspace.player) &&
				psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_SPACE) {
			psy_audio_player_stop(&self->workspace.player);
			return;
		}
	}
}

void mainframe_on_key_up(MainFrame* self, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		return;
	}	
	mainframe_delegate_keyboard(self, psy_EVENTDRIVER_RELEASE, ev);
}

/* delegate keyboard events to the keyboard driver */
void mainframe_delegate_keyboard(MainFrame* self, intptr_t message,
	psy_ui_KeyboardEvent* ev)
{
	psy_eventdriver_write(workspace_kbd_driver(&self->workspace),
		psy_eventdriverinput_make(message, psy_ui_keyboardevent_encode(ev,
			message == psy_EVENTDRIVER_RELEASE),
			psy_ui_keyboardevent_repeat(ev)));
}

void mainframe_on_gear_select(MainFrame* self, Workspace* sender,
	psy_List* machinelist)
{
	gear_select(&self->gear, machinelist);
}

void mainframe_on_drag_over(MainFrame* self, psy_ui_DragEvent* ev)
{
	if (ev->dataTransfer) {
		psy_Property* p;

		p = psy_property_at_section(ev->dataTransfer, "data");
		if (p) {
			p = psy_property_at(p, "dragview", PSY_PROPERTY_TYPE_STRING);
			if (p) {
				psy_ui_dragevent_prevent_default(ev);
			}
		}
	}
}

void mainframe_on_drop(MainFrame* self, psy_ui_DragEvent* ev)
{
	if (ev->dataTransfer) {
		psy_Property* p;

		p = psy_property_at_section(ev->dataTransfer, "data");
		if (p) {
			p = psy_property_at(p, "dragview", PSY_PROPERTY_TYPE_STRING);
			if (p) {
				const char* id;
				psy_ui_Component* view;
				psy_ui_Component* splitter;

				id = psy_property_item_str(p);
				view = NULL;
				splitter = NULL;
				if (strcmp(id, "gear") == 0) {
					view = &self->gear.component;
					splitter = &self->gearsplitter.component;
				} else if (strcmp(id, "cpu") == 0) {
					view = &self->cpuview.component;
					splitter = &self->cpusplitter.component;
				}
				if (view) {
					if (view->align == psy_ui_ALIGN_RIGHT) {
						psy_ui_component_set_align(view, psy_ui_ALIGN_LEFT);
					} else {
						psy_ui_component_set_align(view, psy_ui_ALIGN_RIGHT);
					}
					if (splitter) {
						splitter->align = view->align;
					}
					psy_ui_component_align(psy_ui_component_parent(view));
					psy_ui_component_invalidate(psy_ui_component_parent(view));
				}
				psy_ui_dragevent_prevent_default(ev);
			}
		}
	}
}

void mainframe_on_mouse_down(MainFrame* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealRectangle position;	

	position = psy_ui_component_position(&self->component);	
	self->frame_drag_offset = psy_ui_mouseevent_offset(ev);	
	self->allow_frame_move = mainframe_accept_frame_move(self,
		psy_ui_mouseevent_target(ev));
}

void mainframe_on_mouse_double_click(MainFrame* self, psy_ui_MouseEvent* ev)
{	
	if (mainframe_accept_frame_move(self, psy_ui_mouseevent_target(ev))) {		
		switch (psy_ui_component_state(&self->component)) {
		case psy_ui_COMPONENTSTATE_NORMAL:
			psy_ui_component_set_state(&self->component,
				psy_ui_COMPONENTSTATE_MAXIMIZED);
			break;
		case psy_ui_COMPONENTSTATE_MAXIMIZED:
			psy_ui_component_set_state(&self->component,
				psy_ui_COMPONENTSTATE_NORMAL);
			break;
		case psy_ui_COMPONENTSTATE_FULLSCREEN:
			psy_ui_component_set_state(&self->component,
				psy_ui_COMPONENTSTATE_NORMAL);
			break;
		default:
			break;
		}
	}
}

bool mainframe_accept_frame_move(const MainFrame* self, psy_ui_Component*
	target)
{
	const psy_ui_Component* curr;
	const psy_ui_Component* accept[] = {
		&self->top,
		&self->toprows,
		&self->toprow0_bars,
		&self->toprow0_client,
		&self->filebar.component,
		&self->songbar.component,
		&self->playbar.component,
		&self->undoredobar.component,
		&self->machineviewbar.component,
		&self->vubar.component,
		&self->vubar.vumeter.component,
		NULL
	};
	bool rv;
	uintptr_t i;

	i = 0;
	curr = accept[i];
	rv = FALSE;
	while (curr) {
		if (curr == target) {
			rv = TRUE;
			break;
		}
		++i;
		curr = accept[i];
	}
	return rv;
}

void mainframe_on_mouse_move(MainFrame* self, psy_ui_MouseEvent* ev)
{	
	if (psy_ui_mouseevent_button(ev) == 1 && self->allow_frame_move) {
		psy_ui_RealRectangle position;
		psy_ui_ComponentState state;
			
		position = psy_ui_component_position(&self->component);
		state = psy_ui_component_state(&self->component);
		if (state == psy_ui_COMPONENTSTATE_FULLSCREEN ||
				state == psy_ui_COMPONENTSTATE_MAXIMIZED) {
			psy_ui_RealRectangle normal;
			double left;
			double width;

			normal = psy_ui_component_restore_position(&self->component);
			psy_ui_component_set_state(&self->component,
				psy_ui_COMPONENTSTATE_NORMAL);			
			width = position.right - position.left;
			if (self->frame_drag_offset.x > width / 2.0) {
				double margin_right;

				margin_right = width - psy_ui_mouseevent_offset(ev).x;
				width = normal.right - normal.left;
				left = psy_ui_mouseevent_offset(ev).x - (width - margin_right);				
				self->frame_drag_offset.x = (width - margin_right);				
			} else {
				left = 0.0;
			}
			psy_ui_component_move(&self->component, psy_ui_point_make_px(left,
				0.0));
		} else {
			psy_ui_component_move(&self->component, psy_ui_point_make_px(
				position.left + (psy_ui_mouseevent_offset(ev).x -
					self->frame_drag_offset.x),
				position.top + (psy_ui_mouseevent_offset(ev).y -
					self->frame_drag_offset.y)));
		}
	}
}

void mainframe_on_mouse_up(MainFrame* self, psy_ui_MouseEvent* ev)
{
	self->allow_frame_move = FALSE;
}

bool mainframe_on_input_handler_callback(MainFrame* self, int message, void* param1)
{
	return ((message == INPUTHANDLER_HASFOCUS) && psy_ui_component_has_focus(
		(psy_ui_Component*)param1));		
}

void mainframe_plugineditor_on_focus(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_notebook_select(&self->statusbar.viewstatusbars, 4);
}

void mainframe_add_link(MainFrame* self, Link* link)
{
	links_add(&self->links, link);
	psy_ui_tabbar_append(&self->scripttabbar, link->label_,
		psy_INDEX_INVALID,
		psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_ui_colour_white());
}

void mainframe_seqeditor_on_float(MainFrame* self, psy_ui_Button* sender)
{
	viewframe_allocinit(&self->component,
		&self->seqeditor.component,
		&self->splitseqeditor.component,
		&self->seqeditor.toolbar.alignbar,
		workspace_kbd_driver(&self->workspace));
}

void mainframe_on_metronome_bar(MainFrame* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show_align(metronomebar_base(
			&self->metronomebar));
	} else {
		psy_ui_component_hide_align(metronomebar_base(
			&self->metronomebar));
	}
}

void mainframe_on_trackscope_view(MainFrame* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(trackscopeview_base(&self->trackscopeview));
		trackscopes_start(&self->trackscopeview.scopes);
	}
	else {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopes_stop(&self->trackscopeview.scopes);
	}
	psy_ui_component_align(&self->pane);
	psy_ui_component_invalidate(&self->pane);
}
