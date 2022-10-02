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
/* ui */
#include <uidefaults.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
/* build */
static void mainframe_on_destroyed(MainFrame*);
static void mainframe_init_frame(MainFrame*);
static void mainframe_init_frame_drag(MainFrame*);
static void mainframe_init_workspace(MainFrame*);
static void mainframe_init_layout(MainFrame*);
static void mainframe_init_terminal(MainFrame*);
static void mainframe_init_kbd_help(MainFrame*);
static void mainframe_connect_statusbar(MainFrame*);
static void mainframe_init_bars(MainFrame*);
static void mainframe_init_gear(MainFrame*);
static void mainframe_init_param_rack(MainFrame*);
static void mainframe_init_cpu_view(MainFrame*);
static void mainframe_init_midi_monitor(MainFrame*);
static void mainframe_init_step_sequencer_view(MainFrame*);
static void mainframe_init_keyboard_view(MainFrame*);
static void mainframe_init_seq_editor(MainFrame*);
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
static void mainframe_on_song_changed(MainFrame*, psy_audio_Player* sender);
static void mainframe_on_focus(MainFrame*);
static void mainframe_update_songtitle(MainFrame*);
static void mainframe_on_timer(MainFrame*, uintptr_t timerid);
static bool mainframe_on_input(MainFrame*, InputHandler* sender);
static bool mainframe_on_notes(MainFrame*, InputHandler* sender);
static void mainframe_on_piano_kbd(MainFrame*, psy_Property* sender);
static void mainframe_on_seq_editor(MainFrame*, psy_Property* sender);
static void mainframe_on_step_sequencer(MainFrame*, psy_Property* sender);
static void mainframe_connect_step_sequencer_buttons(MainFrame*);
static void mainframe_connect_piano_kbd_buttons(MainFrame*);
static void mainframe_connect_seq_editor_buttons(MainFrame*);
static void mainframe_on_toggle_terminal(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_toggle_kbd_help(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_exit(MainFrame*, psy_ui_Component* sender);
static void mainframe_on_songtracks_changed(MainFrame*,
	psy_audio_Patterns* sender);
static bool mainframe_on_close(MainFrame*);
static bool mainframe_on_input_handler_callback(MainFrame*, int message,
	void* param1);
static void mainframe_seqeditor_on_float(MainFrame*, psy_ui_Button* sender);
static void mainframe_on_metronome_bar(MainFrame*, psy_Property* sender);
static void mainframe_on_trackscope_view(MainFrame*, psy_Property* sender);
static void mainframe_on_plugin_editor(MainFrame*, psy_Property* sender);
static void mainframe_on_param_rack(MainFrame*, psy_Property* sender);
static void mainframe_on_gear(MainFrame*, psy_Property* sender);
static void mainframe_on_midi_monitor(MainFrame*, psy_Property* sender);
static void mainframe_on_cpu_view(MainFrame*, psy_Property* sender);
static void mainframe_on_help(MainFrame*, psy_ui_Button* sender);
static void mainframe_on_settings(MainFrame*, psy_ui_Button* sender);
static void mainframe_align(MainFrame*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainFrame* self)
{
	assert(self);
	
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onclose =
			(psy_ui_fp_component_onclose)
			mainframe_on_close;
		vtable.on_destroyed =
			(psy_ui_fp_component)
			mainframe_on_destroyed;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			mainframe_on_key_down;
		vtable.onkeyup =
			(psy_ui_fp_component_on_key_event)
			mainframe_on_key_up;		
		vtable.on_focus =
			(psy_ui_fp_component)
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
	assert(self);
		
	mainframe_init_frame(self);
	mainframe_init_workspace(self);		
	mainframe_init_layout(self);
	mainframe_init_terminal(self);
	mainframe_init_kbd_help(self);	
	mainframe_init_bars(self);
	mainframe_init_gear(self);
	mainframe_init_param_rack(self);
	mainframe_init_cpu_view(self);
	mainframe_init_midi_monitor(self);
	mainframe_init_sequence_view(self);
	mainframe_init_sequencer_bar(self);
	mainframe_init_keyboard_view(self);
	mainframe_init_step_sequencer_view(self);
	mainframe_init_seq_editor(self);
	mainframe_init_plugin_editor(self);
	mainframe_connect_statusbar(self);
	mainframe_init_minmaximize(self);
	mainframe_update_songtitle(self);
	mainframe_init_interpreter(self);
	mainstatusbar_update_terminal_button(&self->statusbar);
	mainframe_connect_song(self);
	mainframe_connect_workspace(self);	
	mainframe_connect_piano_kbd_buttons(self);	
	mainframe_connect_step_sequencer_buttons(self);	
	mainframe_connect_seq_editor_buttons(self);
	generalconfig_connect(&self->workspace.config.general,
		"bench.showplugineditor", self, mainframe_on_plugin_editor);
		generalconfig_connect(&self->workspace.config.general,
		"bench.showparamrack", self, mainframe_on_param_rack);
	generalconfig_connect(&self->workspace.config.general,
		"bench.showgear", self, mainframe_on_gear);
	generalconfig_connect(&self->workspace.config.general,
		"bench.showmidi", self, mainframe_on_midi_monitor);	
	generalconfig_connect(&self->workspace.config.general,
		"bench.showcpu", self, mainframe_on_cpu_view);
	if (!workspace_has_plugin_cache(&self->workspace)) {
		workspace_scan_plugins(&self->workspace);
	}
	startscript_init(&self->startscript, self);
	startscript_run(&self->startscript);	
	workspace_select_start_view(&self->workspace);
	self->mainviews.machineview.wireview.pane.centermaster = TRUE;
	mainframe_init_frame_drag(self);
	psy_ui_component_start_timer(mainframe_base(self), 0, 50);	
}

void mainframe_init_frame(MainFrame* self)
{
	assert(self);
	
	psy_ui_frame_init_main(mainframe_base(self));
	vtable_init(self);
	self->starting = TRUE;
	self->titlemodified = FALSE;
	self->allow_frame_move = FALSE;
	psy_ui_component_doublebuffer(mainframe_base(self));
	psy_ui_app_setmain(psy_ui_app(), mainframe_base(self));
	psy_ui_component_set_icon_ressource(mainframe_base(self), IDI_PSYCLEICON);
	init_host_styles(&psy_ui_appdefaults()->styles,
		psy_ui_defaults()->styles.theme_mode);	
	psy_ui_component_init_align(&self->pane, mainframe_base(self),
		mainframe_base(self), psy_ui_ALIGN_CLIENT);	
}

void mainframe_init_frame_drag(MainFrame* self)
{	
	assert(self);
	
	framedrag_init(&self->frame_drag, &self->component);	
	framedrag_add(&self->frame_drag, &self->top);
	framedrag_add(&self->frame_drag, &self->toprows);
	framedrag_add(&self->frame_drag, &self->toprow0_client);
	framedrag_add(&self->frame_drag, &self->filebar.component);
	framedrag_add(&self->frame_drag, &self->songbar.component);
	framedrag_add(&self->frame_drag, &self->playbar.component);
	framedrag_add(&self->frame_drag, &self->undoredobar.component);
	framedrag_add(&self->frame_drag, &self->machinebar.component);
	framedrag_add(&self->frame_drag, &self->vubar.component);
	framedrag_add(&self->frame_drag, &self->vubar.vumeter.component);
}

void mainframe_on_destroyed(MainFrame* self)
{	
	assert(self);
	
	paramviews_dispose(&self->paramviews);
	startscript_dispose(&self->startscript);	
	workspace_dispose(&self->workspace);
	interpreter_dispose(&self->interpreter);
	framedrag_dispose(&self->frame_drag);
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
	assert(self);
	
	workspace_init(&self->workspace, mainframe_base(self));
	paramviews_init(&self->paramviews, &self->component, &self->workspace);
	workspace_set_param_views(&self->workspace, &self->paramviews);	
	workspace_load_configuration(&self->workspace);
}

void mainframe_init_layout(MainFrame* self)
{	
	assert(self);
	
	mainstatusbar_init(&self->statusbar, &self->pane, &self->workspace);	
	psy_ui_component_set_align(mainstatusbar_base(&self->statusbar),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_init_align(&self->client, &self->pane, NULL,
		psy_ui_ALIGN_CLIENT);	
	psy_ui_component_init_align(&self->top, &self->pane, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->top, psy_ui_margin_make_em(
		0.0, 0.0, 0.5, 0.0));
	psy_ui_component_set_style_type(&self->top, STYLE_TOP);
	psy_ui_component_set_default_align(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	mainviews_init(&self->mainviews, &self->client, &self->pane,
		&self->workspace);
	psy_ui_component_set_align(&self->mainviews.component, psy_ui_ALIGN_CLIENT);	
}

void mainframe_init_terminal(MainFrame* self)
{
	assert(self);
	
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
	assert(self);
	
	kbdhelp_init(&self->kbdhelp, &self->pane, &self->workspace);
	psy_ui_component_set_align(kbdhelp_base(&self->kbdhelp),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
}

void mainframe_init_minmaximize(MainFrame* self)
{
	assert(self);
	
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar, &self->left);
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar, &self->toprow1);
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar,
		machinebar_base(&self->machinebar));
	mainviewbar_add_minmaximze(&self->mainviews.mainviewbar,
		trackscopeview_base(&self->trackscopeview));
}

void mainframe_connect_statusbar(MainFrame* self)
{
	assert(self);
	
	psy_signal_connect(&self->statusbar.toggleterminal.signal_clicked,
		self, mainframe_on_toggle_terminal);
	psy_signal_connect(&self->statusbar.turnoff.signal_clicked, self,
		mainframe_on_exit);
	psy_signal_connect(&self->statusbar.togglekbdhelp.signal_clicked, self,
		mainframe_on_toggle_kbd_help);
}

void mainframe_init_bars(MainFrame* self)
{
	assert(self);
	
	/* rows */
	psy_ui_component_init_align(&self->toprows, &self->top, NULL,
		psy_ui_ALIGN_TOP);	
	psy_ui_component_set_style_type(&self->toprows, STYLE_TOPROWS);
	psy_ui_component_set_default_align(&self->toprows, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	/* row0 */
	psy_ui_component_init(&self->toprow0, &self->toprows, NULL);		
	psy_ui_component_init(&self->toprow0_client, &self->toprow0, NULL);
	psy_ui_component_set_align(&self->toprow0_client, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_set_default_align(&self->toprow0_client, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
	/* filebar */
	filebar_init(&self->filebar, &self->toprow0_client, &self->workspace);
	/* undoredo */
	undoredobar_init(&self->undoredobar, &self->toprow0_client, &self->workspace);
	/* playbar */
	playbar_init(&self->playbar, &self->toprow0_client, &self->workspace);
	/* metronomebar */
	metronomebar_init(&self->metronomebar, playbar_base(&self->playbar),
		&self->workspace);
	if (!metronomeconfig_showmetronomebar(&self->workspace.config.metronome)) {
		psy_ui_component_hide(metronomebar_base(&self->metronomebar));
	}
	metronomeconfig_connect(&self->workspace.config.metronome,
		"showmetronome", self, mainframe_on_metronome_bar);	
	psy_ui_component_set_margin(metronomebar_base(&self->metronomebar),
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
	psy_ui_component_set_align(metronomebar_base(&self->metronomebar),
		psy_ui_ALIGN_TOP);		
	/* settings button */
	psy_ui_button_init_text_connect(&self->settings_btn, &self->toprow0,
		"main.settings", self, mainframe_on_settings);
	psy_ui_button_load_resource(&self->settings_btn,
		IDB_SETTINGS_LIGHT, IDB_SETTINGS_DARK,
		psy_ui_colour_white());
	psy_ui_component_set_id(psy_ui_button_base(&self->settings_btn),
		VIEW_ID_SETTINGSVIEW);
	psy_ui_component_set_align(psy_ui_button_base(&self->settings_btn),
		psy_ui_ALIGN_RIGHT);
	/* help button */
	psy_ui_button_init_text_connect(&self->help_btn, &self->toprow0,
		"main.help", self, mainframe_on_help);
	psy_ui_component_set_id(psy_ui_button_base(&self->help_btn),
		VIEW_ID_HELPVIEW);
	psy_ui_component_set_align(psy_ui_button_base(&self->help_btn),
		psy_ui_ALIGN_RIGHT);		
	/* playposition */	
	playposbar_init(&self->playposbar, &self->toprow0, workspace_player(
		&self->workspace));
	psy_ui_component_set_align(&self->playposbar.component, psy_ui_ALIGN_RIGHT);
	/* row1 */
	psy_ui_component_init(&self->toprow1, &self->toprows, NULL);
	/* songbar */	
	songbar_init(&self->songbar, &self->toprow1, &self->workspace);
	psy_ui_component_set_align(&self->songbar.component, psy_ui_ALIGN_CLIENT);	
	/* vubar */
	vubar_init(&self->vubar, &self->toprow1, &self->workspace);	
	psy_ui_component_set_align(&self->vubar.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_margin(&self->vubar.component,
		psy_ui_margin_make_em(0.0, 0.75, 0.0, 0.0));
	/* row2 */	
	machinebar_init(&self->machinebar, &self->toprows, &self->workspace);
	/* scopebar */
	trackscopeview_init(&self->trackscopeview, &self->top, &self->workspace);
	if (!patternviewconfig_show_trackscopes(psycleconfig_patview(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopes_stop(&self->trackscopeview.scopes);
	}
	patternviewconfig_connect(&self->workspace.config.visual.patview,
		"trackscopes", self, mainframe_on_trackscope_view);	
}

void mainframe_init_gear(MainFrame* self)
{
	assert(self);
	
	gear_init(&self->gear, &self->client, &self->paramviews, &self->workspace);
	psy_ui_component_hide(gear_base(&self->gear));
	psy_ui_component_set_align(gear_base(&self->gear), psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_init(&self->gearsplitter, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->gearsplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(psy_ui_splitter_base(&self->gearsplitter));
}

void mainframe_init_param_rack(MainFrame* self)
{
	assert(self);
	
	paramrack_init(&self->paramrack, &self->client, &self->workspace);
	psy_ui_component_hide(paramrack_base(&self->paramrack));
	psy_ui_component_set_align(&self->paramrack.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_splitter_init(&self->splitbarparamrack, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitbarparamrack),
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarparamrack));
}

void mainframe_init_cpu_view(MainFrame* self)
{
	assert(self);
	
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
	assert(self);
	
	midimonitor_init(&self->midimonitor, &self->client, &self->workspace);
	psy_ui_component_hide(midimonitor_base(&self->midimonitor));
	psy_ui_component_set_align(midimonitor_base(&self->midimonitor),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_init(&self->midisplitter, &self->client);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->midisplitter),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_component_hide(psy_ui_splitter_base(&self->midisplitter));	
}

void mainframe_on_plugin_editor(MainFrame* self, psy_Property* sender)
{	
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(&self->plugineditor.component);
		psy_ui_component_show(psy_ui_splitter_base(
			&self->splitbarplugineditor));
	} else {
		psy_ui_component_hide(&self->plugineditor.component);
		psy_ui_component_hide(psy_ui_splitter_base(
			&self->splitbarplugineditor));
	}
	mainframe_align(self);
}

void mainframe_on_param_rack(MainFrame* self, psy_Property* sender)
{	
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(&self->paramrack.component);
		psy_ui_component_show(psy_ui_splitter_base(&self->splitbarparamrack));
	} else {
		psy_ui_component_hide(&self->paramrack.component);
		psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarparamrack));
	}
	if (psy_ui_component_visible(&self->component)){
		psy_ui_component_align(&self->client);
		psy_ui_component_invalidate(&self->client);
	}
}

void mainframe_on_gear(MainFrame* self, psy_Property* sender)
{	
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(&self->gear.component);
		psy_ui_component_show(psy_ui_splitter_base(&self->gearsplitter));
	} else {
		psy_ui_component_hide(&self->gear.component);
		psy_ui_component_hide(psy_ui_splitter_base(&self->gearsplitter));
	}
	if (psy_ui_component_visible(&self->component)){
		psy_ui_component_align(&self->client);
		psy_ui_component_invalidate(&self->client);
	}
}

void mainframe_on_midi_monitor(MainFrame* self, psy_Property* sender)
{	
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(&self->midimonitor.component);
		psy_ui_component_show(psy_ui_splitter_base(&self->midisplitter));
	} else {
		psy_ui_component_hide(&self->midimonitor.component);
		psy_ui_component_hide(psy_ui_splitter_base(&self->midisplitter));
	}
	if (psy_ui_component_visible(&self->component)){
		psy_ui_component_align(&self->client);
		psy_ui_component_invalidate(&self->client);
	}
}

void mainframe_on_cpu_view(MainFrame* self, psy_Property* sender)
{	
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(&self->cpuview.component);
		psy_ui_component_show(psy_ui_splitter_base(&self->cpusplitter));
	} else {
		psy_ui_component_hide(&self->cpuview.component);
		psy_ui_component_hide(psy_ui_splitter_base(&self->cpusplitter));
	}
	if (psy_ui_component_visible(&self->component)){
		psy_ui_component_align(&self->client);
		psy_ui_component_invalidate(&self->client);
	}
}

void mainframe_init_step_sequencer_view(MainFrame* self)
{
	assert(self);
	
	stepsequencerview_init(&self->stepsequencerview, &self->pane,
		&self->workspace);
	psy_ui_component_set_align(stepsequencerview_base(&self->stepsequencerview),
		psy_ui_ALIGN_BOTTOM);
	if (!generalconfig_showstepsequencer(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(stepsequencerview_base(&self->stepsequencerview));
	}	
}

void mainframe_init_keyboard_view(MainFrame* self)
{
	assert(self);
	
	psy_ui_component_init(&self->keyboardview, &self->pane, NULL);
	psy_ui_component_set_align(&self->keyboardview, psy_ui_ALIGN_BOTTOM);
	keyboardstate_init(&self->keyboardstate, psy_ui_HORIZONTAL);	
	pianokeyboard_init(&self->keyboard, &self->keyboardview,
		&self->keyboardstate, &self->workspace.player, NULL);
	psy_ui_component_set_align(pianokeyboard_base(&self->keyboard),
		psy_ui_ALIGN_CENTER);
	psy_ui_component_set_preferred_width(&self->keyboard.component,
		psy_ui_value_make_ew((double)keyboardstate_num_keys(&self->keyboardstate)));		
	if (!generalconfig_showpianokbd(psycleconfig_general(
			workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(&self->keyboardview);
	}			
}

void mainframe_init_seq_editor(MainFrame* self)
{
	assert(self);
	
	seqeditor_init(&self->seqeditor, &self->pane, &self->workspace);
	psy_ui_component_set_align(seqeditor_base(&self->seqeditor),
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_component_setmaximumsize(seqeditor_base(&self->seqeditor),
		psy_ui_size_make(psy_ui_value_zero(), psy_ui_value_make_ph(0.7)));
	psy_ui_splitter_init(&self->splitseqeditor, &self->pane);
	// self->splitseqeditor.direct = TRUE;
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

void mainframe_init_sequence_view(MainFrame* self)
{
	psy_ui_Size size;
	
	assert(self);
	
	psy_ui_component_init_align(&self->left, &self->pane, NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_splitter_init(&self->splitbar, &self->pane);
	seqview_init(&self->sequenceview, &self->left, &self->workspace);
	psy_ui_component_set_align(seqview_base(&self->sequenceview),
		psy_ui_ALIGN_CLIENT);
	size = psy_ui_component_preferred_size(seqview_base(&self->sequenceview),
		NULL);
	psy_ui_component_set_preferred_width(&self->left, size.width);
}

void mainframe_init_sequencer_bar(MainFrame* self)
{
	assert(self);
	
	sequencerbar_init(&self->sequencerbar, &self->left,
		&self->workspace.config.misc, &self->workspace.config.general);
	psy_ui_component_set_align(sequencerbar_base(&self->sequencerbar),
		psy_ui_ALIGN_BOTTOM);
}

void mainframe_init_plugin_editor(MainFrame* self)
{
	assert(self);
	
	plugineditor_init(&self->plugineditor, &self->pane, &self->workspace);	
	psy_ui_component_set_align(plugineditor_base(&self->plugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(plugineditor_base(&self->plugineditor));	
	psy_ui_splitter_init(&self->splitbarplugineditor, &self->pane);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitbarplugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarplugineditor));	
}

void mainframe_connect_workspace(MainFrame* self)
{
	assert(self);
	
	workspace_configure_host(&self->workspace);
	inputhandler_connect(&self->workspace.inputhandler,
		INPUTHANDLER_IMM, psy_EVENTDRIVER_CMD, "general",
		psy_INDEX_INVALID, self, (fp_inputhandler_input)mainframe_on_input);
	inputhandler_connect(&self->workspace.inputhandler,
		INPUTHANDLER_IMM, psy_EVENTDRIVER_CMD, "notes",
		psy_INDEX_INVALID, self, (fp_inputhandler_input)mainframe_on_notes);
	inputhandler_connect_host(&self->workspace.inputhandler,
		self, (fp_inputhandler_hostcallback)mainframe_on_input_handler_callback);
	psy_signal_connect(&self->workspace.player.signal_song_changed, self,
		mainframe_on_song_changed);	
}

void mainframe_init_interpreter(MainFrame* self)
{
	assert(self);
	
	interpreter_init(&self->interpreter, &self->workspace);
	interpreter_start(&self->interpreter);
}

bool mainframe_on_input(MainFrame* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	assert(self);
	
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
				viewindex_make_all(VIEW_ID_PATTERNVIEW, 0, 0, psy_INDEX_INVALID));
		}
		if (!psy_ui_component_visible(&self->mainviews.patternview.properties.component)) {
			psy_ui_Tab* tab;
			psy_ui_component_toggle_visibility(&self->mainviews.patternview.properties.component);

			tab = psy_ui_tabbar_tab(&self->mainviews.patternview.tabbar.tabbar, 5);
			if (tab) {
				tab->checkstate = TABCHECKSTATE_ON;
				psy_ui_component_invalidate(psy_ui_tabbar_base(
					&self->mainviews.patternview.tabbar.tabbar));
			}
		}
		psy_ui_component_set_focus(&self->mainviews.patternview.properties.component);
		return 1;
	case CMD_IMM_MAXPATTERN:
		mainviewbar_toggle_minmaximze(&self->mainviews.mainviewbar);
		return 1;
	case CMD_IMM_TERMINAL:
		mainframe_on_toggle_terminal(self, mainframe_base(self));
		return 1;
	case CMD_IMM_STYLES:
		workspace_select_view(&self->workspace,	viewindex_make_all(
			VIEW_ID_STYLEVIEW, psy_INDEX_INVALID, psy_INDEX_INVALID,
			psy_INDEX_INVALID));
		return 1;
	case CMD_IMM_FULLSCREEN:
		psy_ui_component_togglefullscreen(mainframe_base(self));		
		return 1;
		break;
	case CMD_EDT_EDITQUANTIZEDEC:
		workspace_edit_quantize_change(&self->workspace, -1);
		// patterncursorstepbox_update(&self->patternviewbar.cursorstep);
		return 1;
	case CMD_EDT_EDITQUANTIZEINC:
		workspace_edit_quantize_change(&self->workspace, 1);
		// patterncursorstepbox_update(&self->patternviewbar.cursorstep);
		return 1;
	}
	workspace_on_input(&self->workspace, cmd.id);
	return 0;
}

bool mainframe_on_notes(MainFrame* self, InputHandler* sender)
{
	assert(self);
	
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

void mainframe_on_song_changed(MainFrame* self, psy_audio_Player* sender)
{
	assert(self);
	
	if (workspace_song_has_file(&self->workspace)) {
		if (generalconfig_showing_song_info_on_load(psycleconfig_general(
				workspace_conf(&self->workspace)))) {
			workspace_select_view(&self->workspace,
				viewindex_make(VIEW_ID_SONGPROPERTIES));
		}
	}
	mainframe_update_songtitle(self);
	mainframe_connect_song(self);
	vubar_reset(&self->vubar);
	clockbar_reset(&self->statusbar.clockbar);
	mainframe_align(self);
}

void mainframe_on_songtracks_changed(MainFrame* self,
	psy_audio_Patterns* sender)
{
	assert(self);

	mainframe_align(self);
}

void mainframe_connect_song(MainFrame* self)
{
	assert(self);
	
	if (workspace_song(&self->workspace)) {		
		psy_signal_connect(
			&workspace_song(&self->workspace)->patterns.signal_numsongtrackschanged,
			self, mainframe_on_songtracks_changed);
		vubar_set_machine(&self->vubar, psy_audio_machines_master(
			psy_audio_song_machines(workspace_song(&self->workspace))));
	} else {
		vubar_set_machine(&self->vubar, NULL);
	}
}

void mainframe_update_songtitle(MainFrame* self)
{
	char title[512];
	
	assert(self);

	workspace_app_title(&self->workspace, title, 512);
	psy_ui_component_set_title(mainframe_base(self), title);
	mainstatusbar_set_default_status_text(&self->statusbar,
		workspace_song_title(&self->workspace));
}

void mainframe_on_export(MainFrame* self, psy_ui_Component* sender)
{
	assert(self);
	
	workspace_select_view(&self->workspace, viewindex_make(VIEW_ID_EXPORTVIEW));
}

void mainframe_on_timer(MainFrame* self, uintptr_t timerid)
{	
	if (self->starting && !psy_ui_component_draw_visible(&self->component)) {
		return;
	}
	self->starting = FALSE;	
	vubar_idle(&self->vubar);
	workspace_idle(&self->workspace);
	mainstatusbar_idle(&self->statusbar);
	trackscopeview_idle(&self->trackscopeview);
	seqview_idle(&self->sequenceview);
	plugineditorbar_idle(&self->plugineditor.status);
 	playposbar_idle(&self->playposbar);	
	machineview_idle(&self->mainviews.machineview);	
	if (self->titlemodified != workspace_song_modified(&self->workspace)) {
		self->titlemodified = workspace_song_modified(&self->workspace);
		mainframe_update_songtitle(self);
	}
}

void mainframe_on_focus(MainFrame* self)
{
	psy_ui_Component* currview;
	
	assert(self);

	currview = psy_ui_notebook_active_page(&self->mainviews.notebook);
	if (currview) {
		psy_ui_component_set_focus(currview);
	}
}

void mainframe_on_exit(MainFrame* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_ui_app_close(psy_ui_app());
}

bool mainframe_on_close(MainFrame* self)
{	
	assert(self);
		
	if (keyboardmiscconfig_savereminder(&self->workspace.config.misc) &&
			workspace_song_modified(&self->workspace)) {
		workspace_confirm_close(&self->workspace);
		return FALSE;
	}
	psy_ui_component_stop_timer(mainframe_base(self), 0);
	return TRUE;
}

void mainframe_on_seq_editor(MainFrame* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(seqeditor_base(&self->seqeditor));
		psy_ui_component_show(&self->splitseqeditor.component);
	} else {
		psy_ui_component_hide(seqeditor_base(&self->seqeditor));
		psy_ui_component_hide(&self->splitseqeditor.component);
	}
	mainframe_align(self);
}

void mainframe_on_piano_kbd(MainFrame* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(&self->keyboardview);
	} else {
		psy_ui_component_hide(&self->keyboardview);
	}
	mainframe_align(self);
}

void mainframe_on_step_sequencer(MainFrame* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(stepsequencerview_base(
			&self->stepsequencerview));		
	} else {
		psy_ui_component_hide(stepsequencerview_base(
			&self->stepsequencerview));		
	}
	mainframe_align(self);
}

void mainframe_connect_seq_editor_buttons(MainFrame* self)
{	
	assert(self);
	
	generalconfig_connect(&self->workspace.config.general, 
		"bench.showsequenceedit", self, mainframe_on_seq_editor);	
	psy_ui_button_data_exchange(&self->sequencerbar.toggleseqedit,
		generalconfig_property(psycleconfig_general(workspace_conf(
			&self->workspace)), "bench.showsequenceedit"));
}

void mainframe_connect_step_sequencer_buttons(MainFrame* self)
{
	assert(self);
	
	generalconfig_connect(&self->workspace.config.general,
		"bench.showstepsequencer", self, mainframe_on_step_sequencer);
	psy_ui_button_data_exchange(&self->sequencerbar.togglestepseq,
		generalconfig_property(psycleconfig_general(workspace_conf(
			&self->workspace)), "bench.showstepsequencer"));	
}

void mainframe_connect_piano_kbd_buttons(MainFrame* self)
{
	assert(self);
	
	generalconfig_connect(&self->workspace.config.general,
		"bench.showpianokbd", self, mainframe_on_piano_kbd);
	psy_ui_button_data_exchange(&self->sequencerbar.togglekbd,
		generalconfig_property(psycleconfig_general(workspace_conf(
			&self->workspace)), "bench.showpianokbd"));	
}

void mainframe_on_toggle_terminal(MainFrame* self, psy_ui_Component* sender)
{
	assert(self);
	
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
	mainframe_align(self);
}

void mainframe_on_toggle_kbd_help(MainFrame* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_ui_component_toggle_visibility(kbdhelp_base(&self->kbdhelp));
}

void mainframe_on_key_down(MainFrame* self, psy_ui_KeyboardEvent* ev)
{	
	assert(self);
	
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
	assert(self);
	
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
	assert(self);
	
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		return;
	}	
	mainframe_delegate_keyboard(self, psy_EVENTDRIVER_RELEASE, ev);
}

/* delegate keyboard events to the keyboard driver */
void mainframe_delegate_keyboard(MainFrame* self, intptr_t message,
	psy_ui_KeyboardEvent* ev)
{
	assert(self);
	
	psy_eventdriver_write(workspace_kbd_driver(&self->workspace),
		psy_eventdriverinput_make(message, psy_ui_keyboardevent_encode(ev,
			message == psy_EVENTDRIVER_RELEASE),
			psy_ui_keyboardevent_repeat(ev)));
}

bool mainframe_on_input_handler_callback(MainFrame* self, int message, void* param1)
{
	assert(self);
	
	return ((message == INPUTHANDLER_HASFOCUS) && psy_ui_component_has_focus(
		(psy_ui_Component*)param1));		
}

void mainframe_seqeditor_on_float(MainFrame* self, psy_ui_Button* sender)
{
	assert(self);
	
	viewframe_allocinit(&self->component,
		&self->seqeditor.component,
		&self->splitseqeditor.component,
		&self->seqeditor.toolbar.alignbar,
		workspace_kbd_driver(&self->workspace));
}

void mainframe_on_metronome_bar(MainFrame* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(metronomebar_base(&self->metronomebar));
	} else {
		psy_ui_component_hide(metronomebar_base(&self->metronomebar));
	}	
	mainframe_align(self);	
}

void mainframe_on_trackscope_view(MainFrame* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show(trackscopeview_base(&self->trackscopeview));
		trackscopes_start(&self->trackscopeview.scopes);
	}
	else {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopes_stop(&self->trackscopeview.scopes);
	}
	mainframe_align(self);
}

void mainframe_on_help(MainFrame* self, psy_ui_Button* sender)
{
	assert(self);
	
	workspace_select_view(&self->workspace, viewindex_make(VIEW_ID_HELPVIEW));	
}

void mainframe_on_settings(MainFrame* self, psy_ui_Button* sender)
{
	assert(self);
	
	workspace_select_view(&self->workspace, viewindex_make(
		VIEW_ID_SETTINGSVIEW));
}

void mainframe_align(MainFrame* self)
{
	assert(self);

	if (psy_ui_component_draw_visible(&self->pane)) {
		psy_ui_component_align(&self->pane);
		psy_ui_component_invalidate(&self->pane);
	}
}
