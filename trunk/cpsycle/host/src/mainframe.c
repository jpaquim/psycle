// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
// host
#include "cmdsgeneral.h"
#include "paramview.h"
#include "resources/resource.h"
#include "styles.h"
#include "sequencecmds.h"
// audio
#include <exclusivelock.h>
#include <songio.h>
// ui
#include <uiapp.h>
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

// MainFrame
// prototypes
// build
static void mainframe_initframe(MainFrame*);
static void mainframe_ondestroyed(MainFrame*);
static void mainframe_initworkspace(MainFrame*);
static void mainframe_initemptystatusbar(MainFrame*);
static void mainframe_initspacerleft(MainFrame*);
static void mainframe_inittoparea(MainFrame*);
static void mainframe_initclientarea(MainFrame*);
static void mainframe_initleftarea(MainFrame*);
static void mainframe_initrightarea(MainFrame*);
static void mainframe_initterminal(MainFrame*);
static void mainframe_initkbdhelp(MainFrame*);
static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initviewstatusbars(MainFrame*);
static void mainframe_initstatusbarlabel(MainFrame*);
static void mainframe_initkbdhelpbutton(MainFrame*);
static void mainframe_initterminalbutton(MainFrame*);
static void mainframe_initterminalcolours(MainFrame*);
static void mainframe_initprogressbar(MainFrame*);
static void mainframe_inittabbars(MainFrame*);
static void mainframe_initnavigation(MainFrame*);
static void mainframe_initmaintabbar(MainFrame*);
static void mainframe_initmainviews(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_initgear(MainFrame*);
static void mainframe_initparamrack(MainFrame*);
static void mainframe_initcpuview(MainFrame*);
static void mainframe_initmidimonitor(MainFrame*);
static void mainframe_initstepsequencerview(MainFrame*);
static void mainframe_initseqeditor(MainFrame*);
static void mainframe_initrecentview(MainFrame*);
static void mainframe_initfileview(MainFrame*);
static void mainframe_initsequenceview(MainFrame*);
static void mainframe_initsequencerbar(MainFrame*);
static void mainframe_initplugineditor(MainFrame*);
static void mainframe_connectworkspace(MainFrame*);
static void mainframe_initinterpreter(MainFrame*);
// events
static void mainframe_onkeydown(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_checkplaystartwithrctrl(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_onkeyup(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_delegatekeyboard(MainFrame*, intptr_t message,
	psy_ui_KeyEvent*);
static void mainframe_onmousedown(MainFrame*, psy_ui_MouseEvent*);
static void mainframe_onsequenceselchange(MainFrame*,
	psy_audio_SequenceEntry*);
static void mainframe_ontogglegear(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglegearworkspace(MainFrame*, Workspace* sender);
static void mainframe_onhidegear(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontoggleparamrack(MainFrame*, psy_ui_Component* sender);
static void mainframe_onhideparamrack(MainFrame*, psy_ui_Component* sender);
static void mainframe_onhidemidimonitor(MainFrame*, psy_ui_Component* sender);
static void mainframe_oncpu(MainFrame*, psy_ui_Component* sender);
static void mainframe_onhidecpu(MainFrame*, psy_ui_Component* sender);
static void mainframe_onmidi(MainFrame*, psy_ui_Component* sender);
static void mainframe_onrecentsongs(MainFrame*, psy_ui_Component* sender);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileloadview(MainFrame*, psy_ui_Component* sender);
#endif
static void mainframe_onplugineditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_onaboutok(MainFrame*, psy_ui_Component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, PropertiesView* sender,
	psy_Property*);
static void mainframe_ontabbarchanged(MainFrame*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void mainframe_onsongchanged(MainFrame*, Workspace* sender,
	int flag, psy_audio_Song*);
static void mainframe_onsongloadprogress(MainFrame*, Workspace*, int progress);
static void mainframe_onpluginscanprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onviewselected(MainFrame*, Workspace*, uintptr_t view,
	uintptr_t section, int option);
static void mainframe_onfocusview(MainFrame*, Workspace*);
static void mainframe_onrender(MainFrame*, psy_ui_Component* sender);
static void mainframe_onexport(MainFrame*, psy_ui_Component* sender);
static void mainframe_updatesongtitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, uintptr_t timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, psy_EventDriver* sender);
static void mainframe_ontoggleseqeditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglestepsequencer(MainFrame*, psy_ui_Component* sender);
static void mainframe_updatestepsequencerbuttons(MainFrame*);
static void mainframe_connectstepsequencerbuttons(MainFrame*);
static void mainframe_updateseqeditorbuttons(MainFrame*);
static void mainframe_connectseqeditorbuttons(MainFrame*);
static void mainframe_ontoggleterminal(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglekbdhelp(MainFrame*, psy_ui_Component* sender);
static void mainframe_onselectpatterndisplay(MainFrame*,
	psy_ui_Component* sender, PatternDisplayMode);
static void mainframe_onterminaloutput(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalwarning(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalerror(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onstatus(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_updateterminalbutton(MainFrame*);
static void mainframe_onzoomboxchanged(MainFrame*, ZoomBox* sender);
static void mainframe_onsongtrackschanged(MainFrame*, psy_audio_Patterns* sender,
	uintptr_t numsongtracks);
static void mainframe_onchangecontrolskin(MainFrame*, Workspace* sender,
	const char* path);
static void mainframe_onfloatsection(MainFrame*, Workspace* sender,
	int view, uintptr_t section);
static void mainframe_ondocksection(MainFrame*, Workspace* sender,
	int view, uintptr_t section);
static bool mainframe_onclose(MainFrame*);
static void mainframe_oncheckunsaved(MainFrame*, ConfirmBox* sender,
	int option, int mode);
//static void mainframe_onconfirm(MainFrame*, Confirm* sender,
//	int option, int mode);
static void mainframe_onstartup(MainFrame*);
// EventDriverCallback
static int mainframe_eventdrivercallback(MainFrame*, int msg, int param1,
	int param2);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileload(MainFrame*, FileView* sender);
#endif
static void mainframe_ongearselect(MainFrame*, Workspace* sender,
	psy_List* machinelist);
static void mainframe_onthemechanged(MainFrame*, MachineViewConfig* sender,
	psy_Property*);
static void mainframe_updatethemes(MainFrame* self);
static void mainframe_onupdatestyles(MainFrame* self);

// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(MainFrame* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onclose = (psy_ui_fp_component_onclose)mainframe_onclose;
		vtable.ondestroyed = (psy_ui_fp_component_ondestroyed)
			mainframe_ondestroyed;
		vtable.onkeydown = (psy_ui_fp_component_onkeyevent)mainframe_onkeydown;
		vtable.onkeyup = (psy_ui_fp_component_onkeyevent)mainframe_onkeyup;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			mainframe_onmousedown;
		vtable.ontimer = (psy_ui_fp_component_ontimer)mainframe_ontimer;
		vtable.onupdatestyles = (psy_ui_fp_component_onupdatestyles)
			mainframe_onupdatestyles;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void mainframe_init(MainFrame* self)
{			
	mainframe_initframe(self);	
	mainframe_initworkspace(self);
	mainframe_initemptystatusbar(self);
	mainframe_initspacerleft(self);
	mainframe_initterminal(self);
	mainframe_initkbdhelp(self);
	mainframe_inittoparea(self);
	mainframe_initclientarea(self);	
	mainframe_inittabbars(self);	
	mainframe_initbars(self);	
	mainframe_initnavigation(self);
	mainframe_initmaintabbar(self);	
	mainframe_initmainviews(self);
	mainframe_initrightarea(self);
	mainframe_initgear(self);	
	mainframe_initparamrack(self);
	mainframe_initcpuview(self);
	mainframe_initmidimonitor(self);	
	mainframe_initrecentview(self);
	mainframe_initleftarea(self);
	mainframe_initfileview(self);
	mainframe_initsequenceview(self);
	mainframe_initsequencerbar(self);
	mainframe_initstepsequencerview(self);
	mainframe_initseqeditor(self);	
	mainframe_initplugineditor(self);
	mainframe_initstatusbar(self);
	mainframe_setstartpage(self);
	mainframe_updatesongtitle(self);
	mainframe_initinterpreter(self);
	mainframe_updateterminalbutton(self);
	mainframe_connectworkspace(self);
	mainframe_updatestepsequencerbuttons(self);	
	mainframe_connectstepsequencerbuttons(self);
	mainframe_updateseqeditorbuttons(self);
	mainframe_connectseqeditorbuttons(self);
	mainframe_updatethemes(self);	
}

void mainframe_initframe(MainFrame* self)
{
	psy_ui_frame_init_main(mainframe_base(self));
	psy_ui_component_setvtable(mainframe_base(self), vtable_init(self));
	psy_ui_component_seticonressource(mainframe_base(self), IDI_PSYCLEICON);	
	initdarkstyles(psy_ui_appdefaults());
	self->startup = TRUE;
	self->pluginscanprogress = -1;	
}

void mainframe_ondestroyed(MainFrame* self)
{
	workspace_save_configuration(&self->workspace);
	workspace_dispose(&self->workspace);
	interpreter_dispose(&self->interpreter);
	psy_ui_app_stop(psy_ui_app());
}

void mainframe_initworkspace(MainFrame* self)
{	
	self->startpage = FALSE;
	self->playrow = FALSE;
	self->restoreplaymode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->restorenumplaybeats = 4.0;
	self->restoreloop = TRUE;
	workspace_init(&self->workspace, mainframe_base(self));
	workspace_load_configuration(&self->workspace);
	workspace_load_recentsongs(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}	
}

void mainframe_initemptystatusbar(MainFrame* self)
{
	psy_ui_component_init_align(&self->statusbar, mainframe_base(self),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setstyletypes(&self->statusbar,
		STYLE_STATUSBAR, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_component_setdefaultalign(&self->statusbar, psy_ui_ALIGN_LEFT,
		psy_ui_margin_makeem(0.25, 1.0, 0.25, 0.0));
}

void mainframe_initspacerleft(MainFrame* self)
{	
	psy_ui_component_init(&self->spacerleft, &self->component, NULL);
	psy_ui_component_preventalign(&self->spacerleft);
	psy_ui_component_setpreferredsize(&self->spacerleft,
		psy_ui_size_make_em(2.0, 0.0));
	psy_ui_component_setalign(&self->spacerleft, psy_ui_ALIGN_LEFT);
}

void mainframe_inittoparea(MainFrame* self)
{	
	psy_ui_component_init_align(&self->top, mainframe_base(self),
		psy_ui_ALIGN_TOP);	
	psy_ui_component_setdefaultalign(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_margin_makeem(0.0, 0.0, 0.5, 0.0));
}

void mainframe_initclientarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->client, mainframe_base(self),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->client,
		psy_ui_NOBACKGROUND);	
}

void mainframe_initleftarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->left, mainframe_base(self),
		psy_ui_ALIGN_LEFT);
	psy_ui_splitbar_init(&self->splitbar, mainframe_base(self));
}

void mainframe_initrightarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->right, &self->client,
		psy_ui_ALIGN_RIGHT);
}

void mainframe_initterminal(MainFrame* self)
{
	psy_ui_terminal_init(&self->terminal, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_terminal_base(&self->terminal),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_resize(psy_ui_terminal_base(&self->terminal),
		psy_ui_size_zero());
	psy_ui_splitbar_init(&self->splitbarterminal, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitbarterminal),
		psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->workspace.signal_terminal_warning, self,
		mainframe_onterminalwarning);
	psy_signal_connect(&self->workspace.signal_terminal_out, self,
		mainframe_onterminaloutput);
	psy_signal_connect(&self->workspace.signal_terminal_error, self,
		mainframe_onterminalerror);
	psy_signal_connect(&self->workspace.signal_status_out, self,
		mainframe_onstatus);
}

void mainframe_initkbdhelp(MainFrame* self)
{
	kbdhelp_init(&self->kbdhelp, mainframe_base(self), &self->workspace);
	psy_ui_component_setalign(kbdhelp_base(&self->kbdhelp),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
}

void mainframe_initstatusbar(MainFrame* self)
{	
	zoombox_init_connect(&self->zoombox, &self->statusbar,
		self, mainframe_onzoomboxchanged);	
	mainframe_initstatusbarlabel(self);	
	mainframe_initviewstatusbars(self);
	mainframe_initkbdhelpbutton(self);
	mainframe_initterminalbutton(self);	
	mainframe_initprogressbar(self);
}

void mainframe_initviewstatusbars(MainFrame* self)
{
	psy_ui_notebook_init(&self->viewstatusbars, &self->statusbar);	
	psy_ui_component_setdefaultalign(
		psy_ui_notebook_base(&self->viewstatusbars),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	machineviewbar_init(&self->machineviewbar,
		psy_ui_notebook_base(&self->viewstatusbars),
		&self->workspace);
	self->machineview.wireview.statusbar = &self->machineviewbar;
	self->machineview.stackview.state.statusbar = &self->machineviewbar;
	patternviewbar_init(&self->patternbar,
		psy_ui_notebook_base(&self->viewstatusbars), &self->workspace);
	sampleeditorbar_init(&self->samplesview.sampleeditor.sampleeditortbar,
		psy_ui_notebook_base(&self->viewstatusbars),
		&self->samplesview.sampleeditor, &self->workspace);
	samplesview_connectstatusbar(&self->samplesview);
	instrumentsviewbar_init(&self->instrumentsviewbar,
		psy_ui_notebook_base(&self->viewstatusbars), &self->workspace);
	instrumentsview_setstatusbar(&self->instrumentsview,
		&self->instrumentsviewbar);
	psy_ui_notebook_select(&self->viewstatusbars, 0);
}

void mainframe_initstatusbarlabel(MainFrame* self)
{
	psy_ui_label_init(&self->statusbarlabel, &self->statusbar, NULL);	
	psy_ui_label_preventtranslation(&self->statusbarlabel);
	psy_ui_label_settext(&self->statusbarlabel, "Ready");
	psy_ui_label_setcharnumber(&self->statusbarlabel, 29);
}

void mainframe_initkbdhelpbutton(MainFrame* self)
{	
	psy_ui_button_init_text_connect(&self->togglekbdhelp, &self->statusbar, NULL,
		"main.kbd", self, mainframe_ontogglekbdhelp);
	psy_ui_component_setalign(psy_ui_button_base(&self->togglekbdhelp),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_bitmap_loadresource(&self->togglekbdhelp.bitmapicon, IDB_KBD);
	psy_ui_bitmap_settransparency(&self->togglekbdhelp.bitmapicon,
		psy_ui_colour_make(0x00FFFFFF));
}

void mainframe_initterminalbutton(MainFrame* self)
{	
	self->terminalmsgtype = TERMINALMSGTYPE_NONE;
	psy_ui_button_init_text_connect(&self->toggleterminal, &self->statusbar, NULL,
		"Terminal", self, mainframe_ontoggleterminal);
	psy_ui_component_setalign(psy_ui_button_base(&self->toggleterminal),
		psy_ui_ALIGN_RIGHT);
	mainframe_initterminalcolours(self);	
	psy_ui_bitmap_loadresource(&self->toggleterminal.bitmapicon, IDB_TERM);
	psy_ui_bitmap_settransparency(&self->toggleterminal.bitmapicon,
		psy_ui_colour_make(0x00FFFFFF));	
}

void mainframe_initterminalcolours(MainFrame* self)
{
	self->terminalbutton_colours[TERMINALMSGTYPE_NONE] =
		psy_ui_component_colour(psy_ui_button_base(&self->toggleterminal));
	self->terminalbutton_colours[TERMINALMSGTYPE_ERROR] =
		psy_ui_colour_make(psy_ui_app()->defaults.errorcolour);
	if (psy_ui_defaults()->hasdarktheme) {		
		self->terminalbutton_colours[TERMINALMSGTYPE_WARNING] =
			psy_ui_colour_make_argb(0x00f6b87f);
		self->terminalbutton_colours[TERMINALMSGTYPE_MESSAGE] =
			psy_ui_colour_make_argb(0x009ff6e2);
	} else {
		self->terminalbutton_colours[TERMINALMSGTYPE_NONE] =
			psy_ui_colour_make(0x00808080);
		self->terminalbutton_colours[TERMINALMSGTYPE_ERROR] =
			psy_ui_colour_make(0x00000080);
		self->terminalbutton_colours[TERMINALMSGTYPE_WARNING] =
			psy_ui_colour_make(0x0000A0A0);
		self->terminalbutton_colours[TERMINALMSGTYPE_MESSAGE] =
			psy_ui_colour_make(0x00008000);
	}
}

void mainframe_initprogressbar(MainFrame* self)
{
	psy_ui_progressbar_init(&self->progressbar, &self->statusbar, NULL);
	psy_ui_component_setalign(progressbar_base(&self->progressbar),
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->workspace.signal_loadprogress, self,
		mainframe_onsongloadprogress);
	psy_signal_connect(&self->workspace.signal_scanprogress, self,
		mainframe_onpluginscanprogress);
}

void mainframe_initbars(MainFrame* self)
{
	psy_ui_Margin margin;
	psy_ui_Margin toprowsmargin;
	psy_ui_Margin row0margin;	
	psy_ui_Margin scopemargin;

	psy_ui_margin_init_all_em(&row0margin, 0.5, 0.0, 0.5, 0.0);
	// Vugroup
	psy_ui_component_init(&self->topright, &self->top, NULL);
	psy_ui_component_setalign(&self->topright, psy_ui_ALIGN_RIGHT);
	vubar_init(&self->vubar, &self->topright, &self->workspace);	
	psy_ui_component_setalign(&self->vubar.component, psy_ui_ALIGN_TOP);
	// rows
	psy_ui_component_init_align(&self->toprows, &self->top,
		psy_ui_ALIGN_TOP);
	if (!patternviewconfig_showtrackscopes(psycleconfig_patview(
		workspace_conf(&self->workspace)))) {		
	}
	psy_ui_margin_init_all_em(&toprowsmargin, 0.0, 1.0, 0.0, 0.0);	
	psy_ui_component_setmargin(&self->toprows, toprowsmargin);
	psy_ui_component_setstyletypes(&self->toprows,
		STYLE_TOPROWS, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_component_setdefaultalign(&self->toprows, psy_ui_ALIGN_TOP,
		psy_ui_margin_make(
			psy_ui_value_makepx(0), psy_ui_value_makepx(0),
			psy_ui_value_makeeh(0.5), psy_ui_value_makeew(0.5)));
	// row0
	psy_ui_component_init(&self->toprow0, &self->toprows, NULL);
	psy_ui_component_setstyletypes(&self->toprow0, STYLE_TOPROW0,
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_component_setmargin(&self->toprow0, row0margin);
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setdefaultalign(&self->toprow0, psy_ui_ALIGN_LEFT,
		margin);
	filebar_init(&self->filebar, &self->toprow0, &self->workspace);
	undoredobar_init(&self->undoredobar, &self->toprow0, &self->workspace);
	playbar_init(&self->playbar, &self->toprow0, &self->workspace);
	playposbar_init(&self->playposbar, &self->toprow0, &self->workspace);			
	metronomebar_init(&self->metronomebar, &self->toprow0, &self->workspace);
	if (!metronomeconfig_showmetronomebar(&self->workspace.config.metronome)) {
		psy_ui_component_hide(&self->metronomebar.component);
	}
	margin.right = psy_ui_value_makepx(0);
	psy_ui_component_setmargin(metronomebar_base(&self->metronomebar), margin);
	// row1
	psy_ui_component_init(&self->toprow1, &self->toprows, NULL);
	psy_ui_component_setstyletypes(&self->toprow1, STYLE_TOPROW1,
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_component_setdefaultalign(&self->toprow1, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());
	songbar_init(&self->songbar, &self->toprow1, &self->workspace);
	// row2	
	psy_ui_component_init(&self->toprow2, &self->toprows, NULL);
	psy_ui_component_setdefaultalign(&self->toprow2, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());	
	psy_ui_component_setstyletypes(&self->toprow2, STYLE_TOPROW2,
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
	machinebar_init(&self->machinebar, &self->toprow2, &self->workspace);		
	// scopebar
	trackscopeview_init(&self->trackscopeview, &self->top, &self->workspace);	
	if (!patternviewconfig_showtrackscopes(psycleconfig_patview(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopes_stop(&self->trackscopeview.scopes);
	}
	psy_ui_margin_init_all_em(&scopemargin, 0.0, 1.0, 0.0, 0.0);	
	psy_ui_component_setmargin(&self->trackscopeview.component, scopemargin);
	psy_ui_component_init(&self->topspacer, &self->component, NULL);
	psy_ui_component_setalign(&self->topspacer, psy_ui_ALIGN_TOP);
	psy_ui_component_preventalign(&self->topspacer);
	psy_ui_component_setpreferredsize(&self->topspacer,
		psy_ui_size_make_em(0.0, 0.5));
	psy_ui_component_setstyletypes(&self->topspacer,
		STYLE_TOP, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
}

void mainframe_inittabbars(MainFrame* self)
{
	psy_ui_Margin spacing;

	psy_ui_margin_init_all_em(&spacing, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_init(&self->tabbars, &self->client, NULL);
	psy_ui_component_setalign(&self->tabbars, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->tabspacer, &self->client, NULL);
	psy_ui_component_setalign(&self->tabspacer, psy_ui_ALIGN_TOP);
	psy_ui_component_preventalign(&self->tabspacer);
	psy_ui_component_setpreferredsize(&self->tabspacer,
		psy_ui_size_make_em(0.0, 0.4));
}

void mainframe_initnavigation(MainFrame* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 2.0);		
	navigation_init(&self->navigation, &self->tabbars, &self->workspace);	
	psy_ui_component_setalign(navigation_base(&self->navigation),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(navigation_base(&self->navigation), margin);
}

void mainframe_initmaintabbar(MainFrame* self)
{	
	psy_ui_Tab* tab;

	psy_ui_tabbar_init(&self->tabbar, &self->tabbars);	

	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	psy_ui_component_setalignexpand(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_HORIZONTALEXPAND);	
	tab = psy_ui_tabbar_append(&self->tabbar, "main.machines");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_MACHINES_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_make(0x00FFFFFF));
	tab = psy_ui_tabbar_append(&self->tabbar, "main.patterns");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_NOTES_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_make(0x00FFFFFF));	
	psy_ui_tabbar_append(&self->tabbar, "main.samples");
	psy_ui_tabbar_append(&self->tabbar, "main.instruments");
	psy_ui_tabbar_append(&self->tabbar, "main.properties");		
	tab = psy_ui_tabbar_append(&self->tabbar, "main.settings");
	tab->component.margin.left = psy_ui_value_makeew(4.0);
	psy_ui_bitmap_loadresource(&tab->icon, IDB_SETTINGS_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_make(0x00FFFFFF));
	tab = psy_ui_tabbar_append(&self->tabbar, "main.help");	
	tab->component.margin.right = psy_ui_value_makeew(4.0);
	psy_ui_tabbar_tab(&self->tabbar, 0)->component.margin.left = psy_ui_value_makeew(1.0);	
	psy_ui_notebook_init(&self->viewtabbars, &self->tabbars);
	psy_ui_component_setalign(&self->viewtabbars.component, psy_ui_ALIGN_LEFT);	
}

void mainframe_initmainviews(MainFrame* self)
{
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	machineview_init(&self->machineview, psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component, &self->workspace);
	patternview_init(&self->patternview, psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars), &self->workspace);
	samplesview_init(&self->samplesview, psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars), &self->workspace);
	instrumentview_init(&self->instrumentsview,
		psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars),
		&self->workspace);
	songpropertiesview_init(&self->songpropertiesview,
		psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars),
		&self->workspace);
	propertiesview_init(&self->settingsview,
		psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars),
		&self->workspace.config.config,
		&self->workspace);
	psy_signal_connect(&self->settingsview.signal_changed, self,
		mainframe_onsettingsviewchanged);
	helpview_init(&self->helpview, psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars), &self->workspace);
	psy_signal_connect(&self->helpview.about.okbutton.signal_clicked, self,
		mainframe_onaboutok);
	renderview_init(&self->renderview, psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars), &self->workspace);
	psy_signal_connect(&self->filebar.renderbutton.signal_clicked, self,
		mainframe_onrender);
	exportview_init(&self->exportview, psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars), &self->workspace);
	psy_signal_connect(&self->filebar.exportbutton.signal_clicked, self,
		mainframe_onexport);
	psy_signal_connect(&self->workspace.signal_viewselected, self,
		mainframe_onviewselected);
	psy_signal_connect(&self->workspace.signal_focusview, self,
		mainframe_onfocusview);	
	confirmbox_init(&self->checkunsavedbox,
		psy_ui_notebook_base(&self->notebook),
		&self->workspace);
	psy_signal_connect(&self->tabbar.signal_change, self,
		mainframe_ontabbarchanged);
}

void mainframe_initgear(MainFrame* self)
{		
	gear_init(&self->gear, &self->client, &self->workspace);
	psy_ui_component_setalign(gear_base(&self->gear), psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(gear_base(&self->gear));
	psy_signal_connect(&self->machinebar.gear.signal_clicked, self,
		mainframe_ontogglegear);	
	psy_signal_connect(&gear_base(&self->gear)->signal_hide, self,
		mainframe_onhidegear);
	psy_ui_component_init(&self->gearspacer, &self->client, NULL);
	psy_ui_component_setalign(&self->gearspacer, psy_ui_ALIGN_RIGHT);
	psy_ui_component_preventalign(&self->gearspacer);
	psy_ui_component_setpreferredsize(&self->gearspacer,
		psy_ui_size_make_em(1.0, 0.0));
	psy_ui_component_hide(&self->gearspacer);
}

void mainframe_initparamrack(MainFrame* self)
{	
	paramrack_init(&self->paramrack, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->paramrack.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_splitbar_init(&self->splitbarparamrack, &self->client);
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitbarparamrack),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(paramrack_base(&self->paramrack));
	psy_ui_component_hide(psy_ui_splitbar_base(&self->splitbarparamrack));	
	psy_signal_connect(&self->machinebar.dock.signal_clicked, self,
		mainframe_ontoggleparamrack);
	psy_signal_connect(&paramrack_base(&self->paramrack)->signal_hide, self,
		mainframe_onhideparamrack);
}

void mainframe_initcpuview(MainFrame* self)
{
	cpuview_init(&self->cpuview, &self->client, &self->workspace);
	psy_ui_component_setalign(cpuview_base(&self->cpuview), psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(cpuview_base(&self->cpuview));
	psy_signal_connect(&self->machinebar.cpu.signal_clicked, self,
		mainframe_oncpu);
	psy_signal_connect(&cpuview_base(&self->cpuview)->signal_hide, self,
		mainframe_onhidecpu);
}

void mainframe_initmidimonitor(MainFrame* self)
{
	midimonitor_init(&self->midimonitor, &self->client, &self->workspace);
	psy_ui_component_setalign(midimonitor_base(&self->midimonitor), psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(midimonitor_base(&self->midimonitor));
	psy_signal_connect(&self->machinebar.midi.signal_clicked, self,
		mainframe_onmidi);
	psy_signal_connect(&midimonitor_base(&self->midimonitor)->signal_hide, self,
		mainframe_onhidemidimonitor);
}

void mainframe_initstepsequencerview(MainFrame* self)
{
	stepsequencerview_init(&self->stepsequencerview, &self->client,
		&self->workspace);
	psy_ui_component_setalign(stepsequencerview_base(&self->stepsequencerview),
		psy_ui_ALIGN_BOTTOM);	
	if (!generalconfig_showstepsequencer(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(stepsequencerview_base(&self->stepsequencerview));
	}
}

void mainframe_initseqeditor(MainFrame* self)
{
	seqeditor_init(&self->seqeditor, &self->client, &self->patternview.skin,
		&self->workspace);
	psy_ui_component_setalign(seqeditor_base(&self->seqeditor),
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_splitbar_init(&self->splitseqeditor, &self->client);
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitseqeditor),
		psy_ui_ALIGN_BOTTOM);
	if (!generalconfig_showsequenceedit(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(seqeditor_base(&self->seqeditor));
		psy_ui_component_hide(psy_ui_splitbar_base(&self->splitseqeditor));
	}	
}

void mainframe_initrecentview(MainFrame* self)
{
	// recent song view/playlist
	recentview_init(&self->recentview, mainframe_base(self),
		psy_ui_notebook_base(&self->viewtabbars),
		&self->workspace);
	psy_ui_component_setalign(recentview_base(&self->recentview),
		psy_ui_ALIGN_LEFT);
	if (!generalconfig_showplaylist(&self->workspace.config.general)) {
		psy_ui_component_hide(recentview_base(&self->recentview));
	}
	psy_signal_connect(&self->filebar.recentbutton.signal_clicked, self,
		mainframe_onrecentsongs);	
}

void mainframe_initfileview(MainFrame* self)
{
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
	// file load view
	fileview_init(&self->fileloadview, mainframe_base(self),
		&self->workspace);
	psy_ui_component_setalign(&self->fileloadview.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->fileloadview.component);
	psy_signal_connect(&self->fileloadview.signal_selected,
		self, mainframe_onfileload);
	psy_signal_connect(&self->filebar.loadbutton.signal_clicked, self,
		mainframe_onfileloadview);
#endif	
}

void mainframe_initsequenceview(MainFrame* self)
{
	sequenceview_init(&self->sequenceview, &self->left, &self->workspace);
	psy_ui_component_setalign(sequenceview_base(&self->sequenceview),
		psy_ui_ALIGN_CLIENT);	
}

void mainframe_initsequencerbar(MainFrame* self)
{
	sequencerbar_init(&self->sequencerbar, &self->left, &self->workspace);
	psy_ui_component_setalign(sequencerbar_base(&self->sequencerbar),
		psy_ui_ALIGN_BOTTOM);	
}

void mainframe_initplugineditor(MainFrame* self)
{
	plugineditor_init(&self->plugineditor, mainframe_base(self), &self->workspace);
	psy_ui_component_setalign(plugineditor_base(&self->plugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(plugineditor_base(&self->plugineditor));
	psy_signal_connect(&self->machinebar.editor.signal_clicked, self,
		mainframe_onplugineditor);
	psy_ui_splitbar_init(&self->splitbarplugineditor, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitbarplugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(psy_ui_splitbar_base(&self->splitbarplugineditor));
}

void mainframe_connectworkspace(MainFrame* self)
{
	workspace_configure_host(&self->workspace);
	psy_signal_connect(&self->workspace.player.eventdrivers.signal_input, self,
		mainframe_oneventdriverinput);	
	psy_signal_connect(&self->workspace.signal_changecontrolskin, self,
		mainframe_onchangecontrolskin);	
	psy_signal_connect(&self->workspace.signal_togglegear, self,
		mainframe_ontogglegearworkspace);	
	psy_signal_connect(&self->checkunsavedbox.signal_execute, self,
		mainframe_oncheckunsaved);	
//	psy_signal_connect(&self->confirm.signal_execute, self,
//		mainframe_onconfirm);
	psy_signal_connect(&self->workspace.signal_selectpatterndisplay, self,
		mainframe_onselectpatterndisplay);
	psy_signal_connect(&self->workspace.signal_floatsection, self,
		mainframe_onfloatsection);
	psy_signal_connect(&self->workspace.signal_docksection, self,
		mainframe_ondocksection);
	psy_audio_eventdrivers_setcallback(&self->workspace.player.eventdrivers,
		mainframe_eventdrivercallback, self);
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
	psy_signal_connect(&self->workspace.signal_gearselect, self,
		mainframe_ongearselect);
	psy_signal_connect(&workspace_song(
		&self->workspace)->patterns.signal_numsongtrackschanged,
		self, mainframe_onsongtrackschanged);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(&self->workspace))->signal_themechanged,
		self, mainframe_onthemechanged);
	psy_ui_component_starttimer(mainframe_base(self), 0, 50);
}

void mainframe_initinterpreter(MainFrame* self)
{
	interpreter_init(&self->interpreter, &self->workspace);
	interpreter_start(&self->interpreter);
}

void mainframe_setstartpage(MainFrame* self)
{		
	if (generalconfig_showaboutatstart(psycleconfig_general(
			workspace_conf(&self->workspace)))) {
		workspace_selectview(&self->workspace, VIEW_ID_HELPVIEW, 1, 0);
	} else {
		workspace_selectview(&self->workspace, VIEW_ID_MACHINEVIEW, 0, 0);
	}
	self->startpage = TRUE;
}

void mainframe_oneventdriverinput(MainFrame* self, psy_EventDriver* sender)
{
	psy_EventDriverCmd cmd;

	cmd = psy_eventdriver_getcmd(sender, "general");
	switch (cmd.id) {
	case CMD_IMM_HELP:
		psy_ui_tabbar_select(&self->helpview.tabbar, 0);
		psy_ui_tabbar_select(&self->tabbar, VIEW_ID_HELPVIEW);
		break;
	case CMD_IMM_HELPSHORTCUT:
		mainframe_ontogglekbdhelp(self, mainframe_base(self));
		break;
	case CMD_IMM_EDITMACHINE:
		if (workspace_currview(&self->workspace).id != VIEW_ID_MACHINEVIEW) {
			psy_ui_tabbar_select(&self->tabbar, VIEW_ID_MACHINEVIEW);
		} else {
			if (workspace_currview(&self->workspace).section == SECTION_ID_MACHINEVIEW_WIRES) {
				workspace_selectview(&self->workspace, VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_STACK, 0);
			} else {
				workspace_selectview(&self->workspace, VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_WIRES, 0);
			}
		}
		break;
	case CMD_IMM_EDITPATTERN:
		psy_ui_tabbar_select(&self->tabbar, VIEW_ID_PATTERNVIEW);
		break;
	case CMD_IMM_ADDMACHINE:						
		workspace_selectview(&self->workspace, VIEW_ID_MACHINEVIEW,
			SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPEND);
		break;
	case CMD_IMM_PLAYSONG:
		psy_audio_player_setposition(&self->workspace.player, 0);
		psy_audio_player_start(&self->workspace.player);
		break;
	case CMD_IMM_PLAYROWTRACK: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequenceEntry* entry;

		psy_audio_exclusivelock_enter();
		psy_audio_player_stop(&self->workspace.player);
		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		self->restoreplaymode = psy_audio_sequencer_playmode(&self->workspace.player.sequencer);
		self->restorenumplaybeats = self->workspace.player.sequencer.numplaybeats;
		self->restoreloop = psy_audio_sequencer_looping(&self->workspace.player.sequencer);
		psy_audio_sequencer_stoploop(&self->workspace.player.sequencer);
		psy_audio_sequencer_setplaymode(&self->workspace.player.sequencer,
			psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
		psy_audio_sequencer_setnumplaybeats(&self->workspace.player.sequencer,
			psy_audio_player_bpl(&self->workspace.player));
		self->workspace.player.sequencer.playtrack = self->workspace.patterneditposition.track;
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		self->playrow = TRUE;
		psy_audio_exclusivelock_leave();
		break; */ }
	case CMD_IMM_PLAYROWPATTERN: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequenceEntry* entry;

		psy_audio_exclusivelock_enter();
		psy_audio_player_stop(&self->workspace.player);			
		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		self->restoreplaymode = psy_audio_sequencer_playmode(&self->workspace.player.sequencer);
		self->restorenumplaybeats = self->workspace.player.sequencer.numplaybeats;
		self->restoreloop = psy_audio_sequencer_looping(&self->workspace.player.sequencer);
		psy_audio_sequencer_stoploop(&self->workspace.player.sequencer);
		psy_audio_sequencer_setplaymode(&self->workspace.player.sequencer,
			psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
		psy_audio_sequencer_setnumplaybeats(&self->workspace.player.sequencer,
			psy_audio_player_bpl(&self->workspace.player));
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		self->playrow = TRUE;
		psy_audio_exclusivelock_leave();
		break; */ }
	case CMD_IMM_PLAYSTART:
		workspace_playstart(&self->workspace);
		break;
	case CMD_IMM_PLAYFROMPOS: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequenceEntry* entry;

		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		break; */}
	case CMD_IMM_PLAYSTOP:
		psy_audio_player_stop(&self->workspace.player);
		break;
	case CMD_IMM_FOLLOWSONG:		
		if (workspace_followingsong(&self->workspace)) {
			workspace_stopfollowsong(&self->workspace);
		} else {
			workspace_followsong(&self->workspace);
		}
		break;
	case CMD_IMM_PATTERNINC: {
		SequenceCmds cmds;

		sequencecmds_init(&cmds, &self->workspace);
		sequencecmds_incpattern(&cmds);
		break; }
	case CMD_IMM_PATTERNDEC: {
		SequenceCmds cmds;

		sequencecmds_init(&cmds, &self->workspace);
		sequencecmds_decpattern(&cmds);
		break; }
	case CMD_IMM_SONGPOSDEC:
		workspace_songposdec(&self->workspace);			
		break;
	case CMD_IMM_SONGPOSINC:
		workspace_songposinc(&self->workspace);
		break;
	case CMD_IMM_INFOPATTERN:
		if (workspace_currview(&self->workspace).id != VIEW_ID_PATTERNVIEW) {
			workspace_selectview(&self->workspace, VIEW_ID_PATTERNVIEW, 0, 0);
		}
		if (!psy_ui_component_visible(&self->patternview.properties.component)) {
			psy_ui_Tab* tab;
			psy_ui_component_togglevisibility(&self->patternview.properties.component);

			tab = psy_ui_tabbar_tab(&self->patternview.tabbar, 5);
			if (tab) {
				tab->checkstate = TRUE;
				psy_ui_component_invalidate(psy_ui_tabbar_base(&self->patternview.tabbar));
			}				
		}
		psy_ui_component_setfocus(&self->patternview.properties.component);
		break;
	case CMD_IMM_MAXPATTERN:
		mainframe_maximizeorminimizeview(self);
		break;
	case CMD_IMM_INFOMACHINE:
		workspace_showparameters(&self->workspace,
			psy_audio_machines_selected(&self->workspace.song->machines));
		break;
	case CMD_IMM_EDITINSTR:
		workspace_selectview(&self->workspace, VIEW_ID_INSTRUMENTSVIEW, 0,
			0);
		break;
	case CMD_IMM_EDITSAMPLE:
		workspace_selectview(&self->workspace, VIEW_ID_SAMPLESVIEW, 0, 0);
		psy_ui_tabbar_select(&self->samplesview.clienttabbar, 0);
		break;
	case CMD_IMM_EDITWAVE:
		workspace_selectview(&self->workspace, VIEW_ID_SAMPLESVIEW, 0, 0);
		psy_ui_tabbar_select(&self->samplesview.clienttabbar, 2);
		break;
	case CMD_IMM_TERMINAL:
		mainframe_ontoggleterminal(self, mainframe_base(self));
		break;
	case CMD_IMM_INSTRDEC:
		if (self->workspace.song) {
			psy_audio_instruments_dec(&self->workspace.song->instruments);				
		}
		break;
	case CMD_IMM_INSTRINC:
		if (self->workspace.song) {
			psy_audio_instruments_inc(&self->workspace.song->instruments);
		}
		break;
	case CMD_IMM_SETTINGS:			
		workspace_selectview(&self->workspace, VIEW_ID_SETTINGSVIEW, 0, 0);
		break;
	case CMD_IMM_ENABLEAUDIO: {
		if (psycleconfig_audioenabled(workspace_conf(&self->workspace))) {
			psycleconfig_enableaudio(workspace_conf(&self->workspace),
				FALSE);
		} else {
			psycleconfig_enableaudio(workspace_conf(&self->workspace),
				TRUE);
		}
		break; }
	case CMD_IMM_LOADSONG:
		if (keyboardmiscconfig_savereminder(&self->workspace.config.misc) &&
				workspace_songmodified(&self->workspace)) {				
			workspace_selectview(&self->workspace, VIEW_ID_CHECKUNSAVED, 0, CONFIRM_LOAD);
		} else {
			workspace_loadsong_fileselect(&self->workspace);				
		}
		break;
	case CMD_IMM_SAVESONG:			
		workspace_savesong_fileselect(&self->workspace);			
		break;
	case CMD_EDT_EDITQUANTIZEDEC:
		workspace_editquantizechange(&self->workspace, -1);
		patterncursorstepbox_update(&self->patternbar.cursorstep);
		break;
	case CMD_EDT_EDITQUANTIZEINC:
		workspace_editquantizechange(&self->workspace, 1);
		patterncursorstepbox_update(&self->patternbar.cursorstep);
		break;
	case CMD_COLUMN_0:
	case CMD_COLUMN_1:
	case CMD_COLUMN_2:
	case CMD_COLUMN_3:
	case CMD_COLUMN_4:
	case CMD_COLUMN_5:
	case CMD_COLUMN_6:
	case CMD_COLUMN_7:
	case CMD_COLUMN_8:
	case CMD_COLUMN_9:
	case CMD_COLUMN_A:
	case CMD_COLUMN_B:
	case CMD_COLUMN_C:
	case CMD_COLUMN_D:
	case CMD_COLUMN_E:
	case CMD_COLUMN_F:
		if (workspace_song(&self->workspace) && psy_audio_song_numsongtracks(
				workspace_song(&self->workspace)) >= (cmd.id - CMD_COLUMN_0)) {
			psy_audio_PatternCursor cursor;

			cursor = workspace_patterncursor(&self->workspace);
			cursor.track = (cmd.id - CMD_COLUMN_0);
			workspace_setpatterncursor(&self->workspace, cursor);
		}
		break;
	default:
		break;
	}
}

void mainframe_maximizeorminimizeview(MainFrame* self)
{
	if (self->workspace.maximizeview.maximized) {
		self->workspace.maximizeview.maximized = 0;
		if (self->workspace.maximizeview.row0) {
			psy_ui_component_show(&self->toprow0);
		}
		if (self->workspace.maximizeview.row1) {
			psy_ui_component_show(&self->toprow1);
		}
		if (self->workspace.maximizeview.row2) {
			psy_ui_component_show(&self->toprow2);
		}
		if (self->workspace.maximizeview.trackscopes) {
			psy_ui_component_show(&self->trackscopeview.component);
		}		
		psy_ui_component_show(&self->left);		
	} else {
		self->workspace.maximizeview.maximized = TRUE;
		self->workspace.maximizeview.row0 = self->toprow0.visible;
		self->workspace.maximizeview.row1 = self->toprow1.visible;
		self->workspace.maximizeview.row2 = self->toprow2.visible;
		self->workspace.maximizeview.trackscopes =
			self->trackscopeview.component.visible;		
		psy_ui_component_hide(&self->toprow0);
		psy_ui_component_hide(&self->toprow1);
		psy_ui_component_hide(&self->trackscopeview.component);
		psy_ui_component_hide(&self->left);		
	}
	psy_ui_component_align(mainframe_base(self));
}

void mainframe_onsongloadprogress(MainFrame* self, Workspace* workspace,
	int progress)
{	
	if (progress == -1) {
		psy_ui_terminal_output(&self->terminal, "\n");
	}
	psy_ui_progressbar_setprogress(&self->progressbar, progress / 100.f);
}

void mainframe_onpluginscanprogress(MainFrame* self, Workspace* workspace,
	int progress)
{	
	self->pluginscanprogress = progress;	
}

void mainframe_onsongchanged(MainFrame* self, Workspace* sender, int flag,
	psy_audio_Song* song)
{		
	if (flag == WORKSPACE_LOADSONG) {		
		if (generalconfig_showsonginfoonload(psycleconfig_general(
				workspace_conf(sender)))) {
			psy_ui_tabbar_select(&self->tabbar, VIEW_ID_SONGPROPERTIES);
		}		
	}	
	mainframe_updatesongtitle(self);
	psy_ui_component_align(&self->client);
	if (flag == WORKSPACE_NEWSONG) {
		machinewireview_centermaster(&self->machineview.wireview);
	}
	if (workspace_song(&self->workspace)) {
		psy_signal_connect(
			&workspace_song(&self->workspace)->patterns.signal_numsongtrackschanged,
			self, mainframe_onsongtrackschanged);
	}	
	psy_ui_component_align(mainframe_base(self));
}

void mainframe_updatesongtitle(MainFrame* self)
{	
	psy_Path path;
	char title[512];

	psy_path_init(&path, self->workspace.filename);
	psy_snprintf(title, 512, "[%s.%s]  Psycle Modular Music Creation Studio ",
		psy_path_name(&path), psy_path_ext(&path));
	psy_ui_component_settitle(mainframe_base(self), title);
	if (workspace_song(&self->workspace)) {
		psy_ui_label_settext(&self->statusbarlabel,
			psy_audio_song_title(workspace_song(&self->workspace)));
		psy_ui_label_setdefaulttext(&self->statusbarlabel,
			psy_audio_song_title(workspace_song(&self->workspace)));
	}
	psy_path_dispose(&path);
}

void mainframe_ontogglegear(MainFrame* self, psy_ui_Component* sender)
{
	if (!psy_ui_component_visible(&self->gear.component)) {
		psy_ui_button_highlight(&self->machinebar.gear);		
	}
	psy_ui_component_togglevisibility(&self->gearspacer);
	psy_ui_component_togglevisibility(&self->gear.component);	
	if (!psy_ui_component_visible(&self->gear.component) &&
			psy_ui_notebook_activepage(&self->notebook)) {
		psy_ui_component_setfocus(psy_ui_notebook_activepage(
			&self->notebook));
	}
}

void mainframe_ontogglegearworkspace(MainFrame* self, Workspace* sender)
{
	mainframe_ontogglegear(self, NULL);
}

void mainframe_onhidegear(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.gear);
	psy_ui_component_hide_align(&self->gearspacer);
}

void mainframe_ontoggleparamrack(MainFrame* self, psy_ui_Component* sender)
{
	if (!psy_ui_component_visible(&self->paramrack.component)) {
		psy_ui_button_highlight(&self->machinebar.dock);
	}
	psy_ui_component_togglevisibility(&self->paramrack.component);
	psy_ui_component_togglevisibility(psy_ui_splitbar_base(&self->splitbarparamrack));
	if (!psy_ui_component_visible(&self->paramrack.component) &&
		psy_ui_notebook_activepage(&self->notebook)) {
		psy_ui_component_setfocus(psy_ui_notebook_activepage(
			&self->notebook));		
	}
}

void mainframe_onhideparamrack(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.dock);
}

void mainframe_onhidemidimonitor(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.midi);
}

void mainframe_oncpu(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->cpuview.component)) {		
		psy_ui_button_disablehighlight(&self->machinebar.cpu);
	} else {
		psy_ui_button_highlight(&self->machinebar.cpu);		
	}
	psy_ui_component_togglevisibility(&self->cpuview.component);
}

void mainframe_onhidecpu(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.cpu);
}

void mainframe_onmidi(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->midimonitor.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.midi);
	} else {
		psy_ui_button_highlight(&self->machinebar.midi);		
	}
	psy_ui_component_togglevisibility(&self->midimonitor.component);
}

void mainframe_onrecentsongs(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_seticon(&self->filebar.recentbutton,
		(psy_ui_component_visible(recentview_base(&self->recentview)))
		? psy_ui_ICON_MORE
		: psy_ui_ICON_LESS);
	psy_ui_component_togglevisibility(recentview_base(&self->recentview));
	generalconfig_setplaylistshowstate(&self->workspace.config.general,
		psy_ui_component_visible(recentview_base(&self->recentview)));	
}

#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
void mainframe_onfileloadview(MainFrame* self, psy_ui_Component* sender)
{	
	psy_ui_component_togglevisibility(&self->fileloadview.component);
}
#endif

void mainframe_onplugineditor(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->plugineditor.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.editor);
		psy_ui_component_hide(&self->splitbarplugineditor.component);
	} else {						
		psy_ui_button_highlight(&self->machinebar.editor);
		self->splitbarplugineditor.component.visible = 1;
		psy_ui_component_show(&self->splitbarplugineditor.component);
	}
	psy_ui_component_togglevisibility(&self->plugineditor.component);
}

void mainframe_onaboutok(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_tabbar_select(&self->tabbar, VIEW_ID_MACHINEVIEW);
}

void mainframe_onsettingsviewchanged(MainFrame* self, PropertiesView* sender,
	psy_Property* property)
{
	switch (psy_property_id(property)) {
	case PROPERTY_ID_SHOWSEQUENCEEDIT:
		if (psy_property_item_bool(property) != psy_ui_component_visible(
			seqeditor_base(&self->seqeditor))) {
			psy_ui_component_togglevisibility(
				seqeditor_base(&self->seqeditor));
		}
		break;
	case PROPERTY_ID_SHOWSTEPSEQUENCER:
		if (psy_property_item_bool(property) != psy_ui_component_visible(
				stepsequencerview_base(&self->stepsequencerview))) {
			psy_ui_component_togglevisibility(
				stepsequencerview_base(&self->stepsequencerview));
		}			
		break;
	case PROPERTY_ID_SHOWPLAYLIST:
		if (psy_property_item_bool(property) != psy_ui_component_visible(
			recentview_base(&self->recentview))) {
			psy_ui_component_togglevisibility(
				recentview_base(&self->recentview));
		}
		break;
	case PROPERTY_ID_TRACKSCOPES:
		if (psy_property_item_bool(property) != psy_ui_component_visible(
				trackscopeview_base(&self->trackscopeview))) {
			psy_ui_component_togglevisibility(
				trackscopeview_base(&self->trackscopeview));
			psy_ui_component_align(mainframe_base(self));
		}
		if (psy_property_item_bool(property)) {				
			trackscopes_start(&self->trackscopeview.scopes);
		} else {
			trackscopes_stop(&self->trackscopeview.scopes);
		}
		break;
	case PROPERTY_ID_SHOWMETRONOME:
		if (metronomeconfig_showmetronomebar(&self->workspace.config.metronome)) {
			psy_ui_component_show_align(&self->metronomebar.component);			
		} else {
			psy_ui_component_hide_align(&self->metronomebar.component);
		}
		break;	
	default:
		workspace_configurationchanged(&self->workspace, property);
		break;
	}
}

void mainframe_onrender(MainFrame* self, psy_ui_Component* sender)
{
	workspace_selectview(&self->workspace, VIEW_ID_RENDERVIEW, 0, 0);
}

void mainframe_onexport(MainFrame* self, psy_ui_Component* sender)
{
	workspace_selectview(&self->workspace, VIEW_ID_EXPORTVIEW, 0, 0);
}

void mainframe_ontimer(MainFrame* self, uintptr_t timerid)
{
	if (self->startup && psy_ui_component_visible(mainframe_base(self))) {		
		mainframe_onstartup(self);		
		machinewireview_centermaster(&self->machineview.wireview);
		self->startup = FALSE;
	}
	workspace_idle(&self->workspace);
	if (self->playrow && !psy_audio_player_playing(&self->workspace.player)) {
		self->playrow = FALSE;
		psy_audio_sequencer_setplaymode(&self->workspace.player.sequencer,
			self->restoreplaymode);
		psy_audio_sequencer_setnumplaybeats(&self->workspace.player.sequencer,
			self->restorenumplaybeats);
		if (self->restoreloop) {
			psy_audio_sequencer_loop(&self->workspace.player.sequencer);
		} else {
			psy_audio_sequencer_stoploop(&self->workspace.player.sequencer);
		}
		self->workspace.player.sequencer.playtrack = psy_INDEX_INVALID;
	}
	if (self->pluginscanprogress != -1) {
		if (self->pluginscanprogress == 0) {
			psy_ui_progressbar_setprogress(&self->progressbar, 0);
			self->pluginscanprogress = -1;
		} else {
			psy_ui_progressbar_tick(&self->progressbar);
		}
	}	
}

// event is called when mainframe has its final size and is visible on screen
void mainframe_onstartup(MainFrame* self)
{
	// The pattern display type is only now set, because the host needs to know
	// the view size to divide a splitted display correctly
	workspace_selectpatterndisplay(&self->workspace,
		workspace_patterndisplaytype(&self->workspace));
	// the preferredsize of the sequenceview was used to size it at start
	// prevent it from now on and let further set the size by the splitbar
	self->sequenceview.component.preventpreferredsize = TRUE;
}

void mainframe_onviewselected(MainFrame* self, Workspace* sender, uintptr_t index,
	uintptr_t section, int options)
{		
	psy_ui_Component* view;
	
	if (index == VIEW_ID_CHECKUNSAVED) {			
		if (options == CONFIRM_CLOSE) {
			self->checkunsavedbox.mode = options;
			confirmbox_setlabels(&self->checkunsavedbox,
				"msg.psyexit", "msg.saveexit", "msg.nosaveexit");
		} else if (options == CONFIRM_NEW) {				
			self->checkunsavedbox.mode = options;
			confirmbox_setlabels(&self->checkunsavedbox,
				"msg.newsong", "msg.savenew", "msg.nosavenew");
		} else if (options == CONFIRM_LOAD) {
			self->checkunsavedbox.mode = options;
			confirmbox_setlabels(&self->checkunsavedbox,
				"msg.loadsong", "msg.saveload", "msg.nosaveload");				
		} else if (options == CONFIRM_SEQUENCECLEAR) {
			self->checkunsavedbox.mode = options;
			confirmbox_setlabels(&self->checkunsavedbox,
				"msg.seqclear", "msg.yes", "msg.no");
		}
	}
	view = psy_ui_notebook_page(&self->notebook, index);		
	if (view) {		
		psy_ui_component_selectsection(view, section, options);
		psy_ui_tabbar_select(&self->tabbar, index);
		psy_ui_component_setfocus(view);
	}	
}

void mainframe_onfocusview(MainFrame* self, Workspace* sender)
{
	if (psy_ui_notebook_activepage(&self->notebook)) {
		psy_ui_component_setfocus(psy_ui_notebook_activepage(&self->notebook));
	}
}

void mainframe_ontabbarchanged(MainFrame* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	psy_ui_Component* component;

	if (self->startpage) {
		psy_ui_tabbar_select(&self->helpview.tabbar, 0);
		self->startpage = 0;
	}			
	psy_ui_notebook_select(&self->viewstatusbars, tabindex);	
	psy_ui_notebook_select(&self->viewtabbars, tabindex);
	component = psy_ui_notebook_activepage(&self->notebook);
	if (component) {
		ViewHistoryEntry viewentry;

		viewentry.id = tabindex;
		viewentry.section = psy_ui_component_section(component);
		viewentry.seqpos = psy_INDEX_INVALID;
		workspace_onviewchanged(&self->workspace, viewentry);
		psy_ui_component_setfocus(component);
	}
	psy_ui_component_align(&self->component);	
}

void mainframe_onterminaloutput(MainFrame* self, Workspace* sender,
	const char* text)
{
	if (self->terminalmsgtype == TERMINALMSGTYPE_NONE) {
		self->terminalmsgtype = TERMINALMSGTYPE_MESSAGE;
		mainframe_updateterminalbutton(self);
	}
	psy_ui_terminal_output(&self->terminal, text);	
}

void mainframe_onterminalwarning(MainFrame* self, Workspace* sender,
	const char* text)
{
	if (self->terminalmsgtype != TERMINALMSGTYPE_ERROR) {
		self->terminalmsgtype = TERMINALMSGTYPE_WARNING;
		mainframe_updateterminalbutton(self);
	}
	psy_ui_terminal_output(&self->terminal, (text)
		? text
		: "unknown warning\n");	
}

void mainframe_onterminalerror(MainFrame* self, Workspace* sender,
	const char* text)
{	 	
	psy_ui_terminal_output(&self->terminal, (text)
		? text
		: "unknown error\n");
	self->terminalmsgtype = TERMINALMSGTYPE_ERROR;
	mainframe_updateterminalbutton(self);
}

void mainframe_onstatus(MainFrame* self, Workspace* sender,
	const char* text)
{	
	psy_ui_label_settext(&self->statusbarlabel, text);	
	psy_ui_label_fadeout(&self->statusbarlabel);
}

void mainframe_updateterminalbutton(MainFrame* self)
{
	psy_ui_button_settextcolour(&self->toggleterminal,
		self->terminalbutton_colours[self->terminalmsgtype]);
}

void mainframe_onzoomboxchanged(MainFrame* self, ZoomBox* sender)
{	
	psy_ui_app_setzoomrate(psy_ui_app(), zoombox_rate(sender));	
}

void mainframe_onsongtrackschanged(MainFrame* self, psy_audio_Patterns* sender,
	uintptr_t numsongtracks)
{
	// TrackScopes can change its height, realign mainframe
	psy_ui_component_align(trackscopeview_base(&self->trackscopeview));
	psy_ui_component_align(mainframe_base(self));
}

void mainframe_onchangecontrolskin(MainFrame* self, Workspace* sender,
	const char* path)
{	
	machineparamconfig_releaseskin();
	machineparamconfig_skin(psycleconfig_macparam(workspace_conf(
		&self->workspace)));	
}

void mainframe_onfloatsection(MainFrame* self, Workspace* sender,
	int view, uintptr_t section)
{
	if (view == VIEW_ID_HELPVIEW) {
		if (section == HELPVIEWSECTION_HELP) {
			helpview_float(&self->helpview, section, &self->right);
			psy_ui_component_align(&self->client);
		}
	} else if (view == VIEW_ID_SETTINGSVIEW) {
		propertiesview_float(&self->settingsview, section, &self->right);
		psy_ui_component_align(&self->client);
	}
}

void mainframe_ondocksection(MainFrame* self, Workspace* sender,
	int view, uintptr_t section)
{
	if (view == VIEW_ID_HELPVIEW) {
		if (section == HELPVIEWSECTION_HELP) {
			helpview_dock(&self->helpview, section, &self->right);
			psy_ui_component_align(&self->client);
		}
	} else if (view == VIEW_ID_SETTINGSVIEW) {
		propertiesview_dock(&self->settingsview, section, &self->right);
		psy_ui_component_align(&self->client);
	}
}

void mainframe_onmousedown(MainFrame* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2 && psy_ui_mouseevent_target(ev) ==
			&self->recentview.component) {
		mainframe_onrecentsongs(self, mainframe_base(self));
	}
}

// called if a button is clicked in the checkunsavedbox
// option: which button pressed
// mode  : source of request(app close, song load, song new)
void mainframe_oncheckunsaved(MainFrame* self, ConfirmBox* sender,
	int option, int mode)
{	
	switch (option) {			
		case CONFIRM_YES:
			if (mode == CONFIRM_SEQUENCECLEAR) {
				workspace_restoreview(&self->workspace);
				sequenceview_clear(&self->sequenceview);				
			} else if (workspace_savesong_fileselect(&self->workspace)) {				
				if (mode == CONFIRM_CLOSE) {
					psy_ui_app_close(psy_ui_app());
				} else if (mode == CONFIRM_LOAD) {
					workspace_loadsong_fileselect(&self->workspace);
				} else if (mode == CONFIRM_NEW) {										
					workspace_newsong(&self->workspace);					
				}
			}			
			break;
		case CONFIRM_NO: {
			if (mode == CONFIRM_SEQUENCECLEAR) {
				workspace_restoreview(&self->workspace);
			} else {
				self->workspace.undosavepoint = psy_list_size(
					self->workspace.undoredo.undo);
				self->workspace.machines_undosavepoint = psy_list_size(
					self->workspace.song->machines.undoredo.undo);
				if (mode == CONFIRM_CLOSE) {
					psy_ui_app_close(psy_ui_app());
				} else if (mode == CONFIRM_LOAD) {
					workspace_loadsong_fileselect(&self->workspace);
				} else if (mode == CONFIRM_NEW) {
					workspace_newsong(&self->workspace);
				}
			}
			break; }
		case CONFIRM_CONTINUE:
			workspace_restoreview(&self->workspace);
			break;		
		default:		
			break;
	}	
}

bool mainframe_onclose(MainFrame* self)
{
	if (keyboardmiscconfig_savereminder(&self->workspace.config.misc) &&
			workspace_songmodified(&self->workspace)) {
		workspace_selectview(&self->workspace, VIEW_ID_CHECKUNSAVED, 0,
			CONFIRM_CLOSE);		
		return FALSE;
	}
	return TRUE;
}

void mainframe_ontoggleseqeditor(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_togglevisibility(seqeditor_base(
		&self->seqeditor));
	psy_ui_component_togglevisibility(psy_ui_splitbar_base(
		&self->splitseqeditor));
	generalconfig_setsequenceeditshowstate(psycleconfig_general(
		workspace_conf(&self->workspace)),
		psy_ui_component_visible(seqeditor_base(
			&self->seqeditor)));
	mainframe_updateseqeditorbuttons(self);
}

void mainframe_updateseqeditorbuttons(MainFrame* self)
{
	if (generalconfig_showsequenceedit(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_button_settext(&self->sequencerbar.toggleseqedit,
			"sequencerview.hideseqeditor");
		psy_ui_button_highlight(&self->sequencerbar.toggleseqedit);
		psy_ui_button_seticon(&self->sequencerbar.toggleseqedit,
			psy_ui_ICON_LESS);
	} else {
		psy_ui_button_settext(&self->sequencerbar.toggleseqedit,
			"sequencerview.showseqeditor");
		psy_ui_button_disablehighlight(
			&self->sequencerbar.toggleseqedit);
		psy_ui_button_seticon(&self->sequencerbar.toggleseqedit,
			psy_ui_ICON_MORE);
	}
}

void mainframe_connectseqeditorbuttons(MainFrame* self)
{	
	psy_signal_connect(
		&self->sequencerbar.toggleseqedit.signal_clicked, self,
		mainframe_ontoggleseqeditor);
	if (!generalconfig_showsequenceedit(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
			psy_ui_component_hide(seqeditor_base(&self->seqeditor));
			psy_ui_component_hide(psy_ui_splitbar_base(&self->splitseqeditor));
	}
}

void mainframe_ontogglestepsequencer(MainFrame* self, psy_ui_Component* sender)
{		
	psy_ui_component_togglevisibility(stepsequencerview_base(
		&self->stepsequencerview));		
	generalconfig_setstepsequencershowstate(psycleconfig_general(
		workspace_conf(&self->workspace)),
		psy_ui_component_visible(stepsequencerview_base(
			&self->stepsequencerview)));
	mainframe_updatestepsequencerbuttons(self);
}

void mainframe_updatestepsequencerbuttons(MainFrame* self)
{
	if (generalconfig_showstepsequencer(psycleconfig_general(
			workspace_conf(&self->workspace)))) {
		psy_ui_button_settext(&self->sequencerbar.togglestepseq,
			"sequencerview.hidestepsequencer");
		psy_ui_button_highlight(&self->sequencerbar.togglestepseq);
		psy_ui_button_seticon(&self->sequencerbar.togglestepseq,
			psy_ui_ICON_LESS);
	} else {
		psy_ui_button_settext(&self->sequencerbar.togglestepseq,
			"sequencerview.showstepsequencer");
		psy_ui_button_disablehighlight(
			&self->sequencerbar.togglestepseq);
		psy_ui_button_seticon(&self->sequencerbar.togglestepseq,
			psy_ui_ICON_MORE);		
	}
}

void mainframe_connectstepsequencerbuttons(MainFrame* self)
{	
	psy_signal_connect(
		&self->sequencerbar.togglestepseq.signal_clicked, self,
		mainframe_ontogglestepsequencer);
	if (!generalconfig_showstepsequencer(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(stepsequencerview_base(&self->stepsequencerview));
	}
}

void mainframe_ontoggleterminal(MainFrame* self, psy_ui_Component* sender)
{
	if (!psy_ui_isvaluezero(psy_ui_component_size(
		&self->terminal.component).height)) {
		psy_ui_component_resize(&self->terminal.component,
			psy_ui_size_zero());
		self->terminalmsgtype = TERMINALMSGTYPE_NONE;
		mainframe_updateterminalbutton(self);		
	} else {
		psy_ui_component_resize(&self->terminal.component,
			psy_ui_size_make(psy_ui_value_makepx(0),
				psy_ui_value_makeeh(10.0)));		
	}
	psy_ui_component_align(mainframe_base(self));
}

void mainframe_ontogglekbdhelp(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_togglevisibility(kbdhelp_base(&self->kbdhelp));	
}

void mainframe_onselectpatterndisplay(MainFrame* self, psy_ui_Component* sender,
	PatternDisplayMode display)
{	
	patternview_selectdisplay(&self->patternview,
		workspace_patterndisplaytype(&self->workspace));
}


#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
void mainframe_onfileload(MainFrame* self, FileView* sender)
{
	const char* path;

	path = fileview_path(sender);
	// workspace_loadsong(&self->workspace, path,
	//	workspace_playsongafterload(&self->workspace));
}
#endif

void mainframe_onkeydown(MainFrame* self, psy_ui_KeyEvent* ev)
{	
	mainframe_checkplaystartwithrctrl(self, ev);
	mainframe_delegatekeyboard(self, psy_EVENTDRIVER_KEYDOWN, ev);
}

void mainframe_checkplaystartwithrctrl(MainFrame* self, psy_ui_KeyEvent* ev)
{
	if (keyboardmiscconfig_playstartwithrctrl(
			&self->workspace.config.misc)) {
		if (ev->keycode == psy_ui_KEY_CONTROL) {
			// todo: this win32 detection obly
			int extended = (ev->keydata & 0x01000000) != 0;
			if (extended) {
				// right ctrl
				workspace_playstart(&self->workspace);
				return;
			}
		} else if (psy_audio_player_playing(&self->workspace.player) &&
			ev->keycode == psy_ui_KEY_SPACE) {			
			psy_audio_player_stop(&self->workspace.player);			
			return;
		}
	}
}

void mainframe_onkeyup(MainFrame* self, psy_ui_KeyEvent* ev)
{
	mainframe_delegatekeyboard(self, psy_EVENTDRIVER_KEYUP, ev);
}

// delegate keyboard events to the keyboard driver
void mainframe_delegatekeyboard(MainFrame* self, intptr_t message,
	psy_ui_KeyEvent* ev)
{
	psy_eventdriver_write(workspace_kbddriver(&self->workspace),
		psy_eventdriverinput_make(message,
			psy_audio_encodeinput(ev->keycode, ev->shift, ev->ctrl, ev->alt),
				0, workspace_currview(&self->workspace).id));
}

// eventdriver callback to handle chordmode, patternedit noterelease
int mainframe_eventdrivercallback(MainFrame* self, int msg, int param1,
	int param2)
{
	switch (msg) {
		case PSY_EVENTDRIVER_PATTERNEDIT:
			return psy_ui_component_hasfocus(
				&self->patternview.tracker.component) ||
				psy_ui_component_hasfocus(
					&self->patternview.pianoroll.grid.component) ||
				psy_ui_component_hasfocus(
					&self->patternview.griddefaults.component);
			break;
		case PSY_EVENTDRIVER_NOTECOLUMN:
			return self->patternview.gridstate.cursor.column == 0;
			break;
		case PSY_EVENTDRIVER_SETCHORDMODE:
			if (param1 == 1) {
				self->patternview.tracker.chordbegin =
					self->patternview.gridstate.cursor.track;
				self->patternview.tracker.chordmode = TRUE;
			}
			break;
		case PSY_EVENTDRIVER_INSERTNOTEOFF:
			trackergrid_inputnote(&self->patternview.tracker,
				psy_audio_NOTECOMMANDS_RELEASE,
				self->patternview.tracker.chordmode);
			break;
		case PSY_EVENTDRIVER_SECTION:
			trackergrid_inputnote(&self->patternview.tracker,
				psy_audio_NOTECOMMANDS_RELEASE,
				self->patternview.tracker.chordmode);
			break;
	}
	return 0;
}

void mainframe_ongearselect(MainFrame* self, Workspace* sender,
	psy_List* machinelist)
{
	gear_select(&self->gear, machinelist);
}

void mainframe_onthemechanged(MainFrame* self, MachineViewConfig* sender,
	psy_Property* theme)
{
}

void mainframe_updatethemes(MainFrame* self)
{	
}

void mainframe_onupdatestyles(MainFrame* self)
{
	self->terminalbutton_colours[TERMINALMSGTYPE_ERROR] =
		psy_ui_colour_make(psy_ui_app()->defaults.errorcolour);
	if (!self->startup) {
		mainframe_updateterminalbutton(self);		
	}	
}
