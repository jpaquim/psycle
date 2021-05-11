// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
// host
#include "cmdsgeneral.h"
#include "paramview.h"
#include "resources/resource.h"
#include "sequencecmds.h"
#include "styles.h"
// audio
#include <exclusivelock.h>
#include <songio.h>
// ui
#include <uiapp.h>
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

#ifdef PSYCLE_MAKE_DEFAULT_LANG
#include "defaultlang.h"
#endif

static void updateshowstate(psy_Property*, psy_ui_Component*);
// prototypes
// build
static void mainframe_initframe(MainFrame*);
static void mainframe_ondestroyed(MainFrame*);
static void mainframe_initworkspace(MainFrame*);
static void mainframe_initemptystatusbar(MainFrame*);
static void mainframe_initspacerleft(MainFrame*);
static void mainframe_initspacerright(MainFrame*);
static void mainframe_inittoparea(MainFrame*);
static void mainframe_initclientarea(MainFrame*);
static void mainframe_initmainviewarea(MainFrame*); 
static void mainframe_initleftarea(MainFrame*);
static void mainframe_initrightarea(MainFrame*);
static void mainframe_initterminal(MainFrame*);
static void mainframe_initkbdhelp(MainFrame*);
static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initviewstatusbars(MainFrame*);
static void mainframe_connectstatusbar(MainFrame*);
static void mainframe_inittabbars(MainFrame*);
static void mainframe_initnavigation(MainFrame*);
static void mainframe_initmaintabbar(MainFrame*);
static void mainframe_inithelpsettingstabbar(MainFrame*);
static void mainframe_initviewtabbars(MainFrame*);
static void mainframe_initmainviews(MainFrame*);
static void mainframe_initbars(MainFrame*);
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
static void mainframe_initminmaximize(MainFrame*);
static void mainframe_connectworkspace(MainFrame*);
static void mainframe_initinterpreter(MainFrame*);
// events
static void mainframe_onkeydown(MainFrame*, psy_ui_KeyboardEvent*);
static void mainframe_checkplaystartwithrctrl(MainFrame*, psy_ui_KeyboardEvent*);
static void mainframe_onkeyup(MainFrame*, psy_ui_KeyboardEvent*);
static void mainframe_delegatekeyboard(MainFrame*, intptr_t message,
	psy_ui_KeyboardEvent*);
static void mainframe_ontogglegearworkspace(MainFrame*, Workspace* sender);
static void mainframe_onmaxminimizeview(MainFrame*, psy_ui_Button* sender);
static void mainframe_onrecentsongs(MainFrame*, psy_ui_Component* sender);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileloadview(MainFrame*, psy_ui_Component* sender);
#endif
static void mainframe_onplugineditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, PropertiesView* sender,
	psy_Property*, uintptr_t* rebuild);
static void mainframe_ontabbarchanged(MainFrame*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void mainframe_onsettingshelptabbarchanged(MainFrame*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void mainframe_onsongchanged(MainFrame*, Workspace* sender,
	int flag, psy_audio_Song*);
static void mainframe_onsongloadprogress(MainFrame*, Workspace*, intptr_t progress);
static void mainframe_onpluginscanprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onviewselected(MainFrame*, Workspace*, uintptr_t view,
	uintptr_t section, int option);
static void mainframe_onfocusview(MainFrame*, Workspace*);
static void mainframe_onrender(MainFrame*, psy_ui_Component* sender);
static void mainframe_onexport(MainFrame*, psy_ui_Component* sender);
static void mainframe_updatesongtitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, uintptr_t timerid);
static void mainframe_maxminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, psy_EventDriver* sender);
static void mainframe_ontoggleseqeditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglestepsequencer(MainFrame*, psy_ui_Component* sender);
static void mainframe_updatestepsequencerbuttons(MainFrame*);
static void mainframe_connectstepsequencerbuttons(MainFrame*);
static void mainframe_updateseqeditorbuttons(MainFrame*);
static void mainframe_connectseqeditorbuttons(MainFrame*);
static void mainframe_ontoggleterminal(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglekbdhelp(MainFrame*, psy_ui_Component* sender);
static void mainframe_onexit(MainFrame*, psy_ui_Component* sender);
static void mainframe_onselectpatterndisplay(MainFrame*,
	psy_ui_Component* sender, PatternDisplayMode);
static void mainframe_onterminaloutput(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalwarning(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalerror(MainFrame*, Workspace* sender,
	const char* text);
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
static void mainframe_onstartup(MainFrame*);
// EventDriverCallback
static int mainframe_eventdrivercallback(MainFrame*, int msg, int param1,
	int param2);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileload(MainFrame*, FileView* sender);
#endif
static void mainframe_ongearselect(MainFrame*, Workspace* sender,
	psy_List* machinelist);
static void mainframe_resetfocus(MainFrame*);
static void mainframe_ondragover(MainFrame*, psy_ui_DragEvent*);
static void mainframe_ondrop(MainFrame*, psy_ui_DragEvent*);

// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainFrame* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onclose =
			(psy_ui_fp_component_onclose)
			mainframe_onclose;
		vtable.ondestroyed =
			(psy_ui_fp_component_ondestroyed)
			mainframe_ondestroyed;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			mainframe_onkeydown;
		vtable.onkeyup =
			(psy_ui_fp_component_onkeyevent)
			mainframe_onkeyup;
		vtable.ondragover =
			(psy_ui_fp_component_ondragover)
			mainframe_ondragover;
		vtable.ondrop =
			(psy_ui_fp_component_ondrop)
			mainframe_ondrop;
		vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			mainframe_ontimer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(mainframe_base(self), &vtable);
}
// implementation
void mainframe_init(MainFrame* self)
{			
	mainframe_initframe(self);		
	mainframe_initworkspace(self);	
	mainframe_initemptystatusbar(self);	
	mainframe_initterminal(self);
	mainframe_initkbdhelp(self);
	mainframe_inittoparea(self);	
	mainframe_initclientarea(self);
	mainframe_initspacerleft(self);
	mainframe_initspacerright(self);
	mainframe_initmainviewarea(self);	
	mainframe_inittabbars(self);	
	mainframe_initbars(self);	
	mainframe_initnavigation(self);	
	mainframe_initmaintabbar(self);
	mainframe_inithelpsettingstabbar(self);	
	mainframe_initviewtabbars(self);
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
#if PSYCLE_USE_TK == PSYCLE_TK_XT
	/* todo segfault X11 imp */
#else
	mainframe_initseqeditor(self);	
#endif
	mainframe_initplugineditor(self);
	mainframe_initstatusbar(self);
	mainframe_initminmaximize(self);
	mainframe_setstartpage(self);
	mainframe_updatesongtitle(self);
	mainframe_initinterpreter(self);
	mainstatusbar_updateterminalbutton(&self->statusbar);
	mainframe_connectworkspace(self);
	mainframe_updatestepsequencerbuttons(self);	
	mainframe_connectstepsequencerbuttons(self);
#if PSYCLE_USE_TK == PSYCLE_TK_XT
#else
	mainframe_updateseqeditorbuttons(self);
	mainframe_connectseqeditorbuttons(self);	
#endif	
#ifdef PSYCLE_MAKE_DEFAULT_LANG
	save_translator_default();
	save_translator_template();
#endif
	if (!workspace_hasplugincache(&self->workspace)) {
#if PSYCLE_USE_TK == PSYCLE_TK_XT
		/* todo dir enum linux */
#else
		workspace_scanplugins(&self->workspace);
#endif
	}
	self->machineview.wireview.centermaster = TRUE;
}

void mainframe_initframe(MainFrame* self)
{
	psy_ui_frame_init_main(mainframe_base(self));
	vtable_init(self);
	psy_ui_app()->main = mainframe_base(self);	
	psy_ui_component_seticonressource(mainframe_base(self), IDI_PSYCLEICON);
	inithoststyles(&psy_ui_appdefaults()->styles, psy_ui_defaults()->styles.theme);
	self->startup = TRUE;
	self->pluginscanprogress = -1;
}

void mainframe_ondestroyed(MainFrame* self)
{		
	psy_list_free(self->minmaximize);
	self->minmaximize = NULL;
	workspace_dispose(&self->workspace);
	interpreter_dispose(&self->interpreter);
	psy_ui_app_stop(psy_ui_app());
}

void mainframe_initworkspace(MainFrame* self)
{	
	self->startpage = FALSE;		
	workspace_init(&self->workspace, mainframe_base(self));
	workspace_load_configuration(&self->workspace);
	workspace_load_recentsongs(&self->workspace);	
}

void mainframe_initemptystatusbar(MainFrame* self)
{
	mainstatusbar_init(&self->statusbar, mainframe_base(self),
		&self->workspace);
	psy_ui_component_setalign(&self->statusbar.component, psy_ui_ALIGN_BOTTOM);
}

void mainframe_initspacerleft(MainFrame* self)
{
	psy_ui_component_init_align(&self->spacerleft, &self->component,
		NULL, psy_ui_ALIGN_LEFT);
	psy_ui_component_setpreferredsize(&self->spacerleft,
		psy_ui_size_make_em(0.5, 0.0));	
}

void mainframe_initspacerright(MainFrame* self)
{	
	psy_ui_component_init_align(&self->spacerright, &self->client,
		NULL, psy_ui_ALIGN_RIGHT);
	psy_ui_component_setpreferredsize(&self->spacerright,
	 	psy_ui_size_make_em(0.5, 0.0));	
}

void mainframe_inittoparea(MainFrame* self)
{	
	psy_ui_component_init_align(&self->top, mainframe_base(self),
		NULL, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());	
}

void mainframe_initclientarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->client, mainframe_base(self), NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->client, psy_ui_NOBACKGROUND);
}

void mainframe_initmainviewarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->mainviews, &self->client, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->mainviews, psy_ui_NOBACKGROUND);
}

void mainframe_initleftarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->left, mainframe_base(self), NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_setbackgroundmode(&self->left, psy_ui_NOBACKGROUND);
	psy_ui_splitter_init(&self->splitbar, mainframe_base(self));
}

void mainframe_initrightarea(MainFrame* self)
{
	psy_ui_component_init_align(&self->right, &self->client, NULL,
		psy_ui_ALIGN_RIGHT);
}

void mainframe_initterminal(MainFrame* self)
{
	psy_ui_terminal_init(&self->terminal, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_terminal_base(&self->terminal),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(psy_ui_terminal_base(&self->terminal),
		psy_ui_size_zero());
	psy_ui_splitter_init(&self->splitbarterminal, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_splitter_base(&self->splitbarterminal),
		psy_ui_ALIGN_BOTTOM);
	workspace_connectterminal(&self->workspace, self,
		(fp_workspace_output)mainframe_onterminaloutput,
		(fp_workspace_output)mainframe_onterminalwarning,
		(fp_workspace_output)mainframe_onterminalerror);
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
	mainframe_initviewstatusbars(self);	
	mainframe_connectstatusbar(self);	
}

void mainframe_initminmaximize(MainFrame* self)
{
	self->minmaximize = NULL;
	psy_list_append(&self->minmaximize, &self->left);
	psy_list_append(&self->minmaximize, &self->toprow1);
	psy_list_append(&self->minmaximize, &self->toprow2);
	psy_list_append(&self->minmaximize, &self->trackscopeview.component);
}

void mainframe_initviewstatusbars(MainFrame* self)
{		
	machineviewbar_init(&self->machineviewbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars),
		&self->workspace);	
	patternviewbar_init(&self->patternbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars), &self->workspace);
	sampleeditorbar_init(&self->samplesview.sampleeditor.sampleeditortbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars),
		&self->samplesview.sampleeditor, &self->workspace);
	samplesview_connectstatusbar(&self->samplesview);
	instrumentsviewbar_init(&self->instrumentsviewbar,
		psy_ui_notebook_base(&self->statusbar.viewstatusbars), &self->workspace);
	instrumentsview_setstatusbar(&self->instrumentsview,
		&self->instrumentsviewbar);
	psy_ui_notebook_select(&self->statusbar.viewstatusbars, 0);
}

void mainframe_connectstatusbar(MainFrame* self)
{	
	psy_signal_connect(&self->statusbar.zoombox.signal_changed,
		self, mainframe_onzoomboxchanged);
	psy_signal_connect(&self->statusbar.toggleterminal.signal_clicked,
		self, mainframe_ontoggleterminal);
	psy_signal_connect(&self->statusbar.turnoff.signal_clicked, self,
		mainframe_onexit);
	psy_signal_connect(&self->statusbar.togglekbdhelp.signal_clicked, self,
		mainframe_ontogglekbdhelp);
	workspace_connectloadprogress(&self->workspace, self,
		(fp_workspace_songloadprogress)mainframe_onsongloadprogress);
	psy_signal_connect(&self->workspace.signal_scanprogress, self,
		mainframe_onpluginscanprogress);
}

void mainframe_initbars(MainFrame* self)
{
	psy_ui_Margin margin;	
	
	// rows
	psy_ui_component_init_align(&self->toprows, &self->top, NULL,
		psy_ui_ALIGN_TOP);
	if (!patternviewconfig_showtrackscopes(psycleconfig_patview(
		workspace_conf(&self->workspace)))) {		
	}	
	psy_ui_component_setstyletypes(&self->toprows,
		STYLE_TOPROWS, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_component_setdefaultalign(&self->toprows, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());			
	// row0
	psy_ui_component_init(&self->toprow0, &self->toprows, NULL);	
	// Vugroup		
	vubar_init(&self->vubar, &self->toprow0, &self->workspace);
	psy_ui_component_setalign(&self->vubar.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_setstyletype(&self->toprow0, STYLE_TOPROW0);
	psy_ui_component_init(&self->toprow0_client, &self->toprow0, NULL);
	psy_ui_component_setalign(&self->toprow0_client, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->toprow0_bars, &self->toprow0_client, NULL);
	psy_ui_component_setalign(&self->toprow0_bars, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setdefaultalign(&self->toprow0_bars, psy_ui_ALIGN_LEFT,
		margin);		
	filebar_init(&self->filebar, &self->toprow0_bars, &self->workspace);
	undoredobar_init(&self->undoredobar, &self->toprow0_bars, &self->workspace);	
	playbar_init(&self->playbar, &self->toprow0_bars, &self->workspace);
	playposbar_init(&self->playposbar, &self->toprow0_bars, &self->workspace);	
	metronomebar_init(&self->metronomebar, &self->toprow0_bars, &self->workspace);
	if (!metronomeconfig_showmetronomebar(&self->workspace.config.metronome)) {
		psy_ui_component_hide(&self->metronomebar.component);
	}
	margin.right = psy_ui_value_make_px(0);
	psy_ui_component_setmargin(metronomebar_base(&self->metronomebar), margin);
	// row1
	psy_ui_component_init(&self->toprow1, &self->toprows, NULL);
	psy_ui_component_setstyletype(&self->toprow1, STYLE_TOPROW1);		
	psy_ui_component_setdefaultalign(&self->toprow1, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());
	songbar_init(&self->songbar, &self->toprow1, &self->workspace);
	// row2	
	psy_ui_component_init(&self->toprow2, &self->toprows, NULL);
	psy_ui_component_setdefaultalign(&self->toprow2, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());	
	psy_ui_component_setstyletype(&self->toprow2, STYLE_TOPROW2);
	machinebar_init(&self->machinebar, &self->toprow2, &self->workspace);		
	// scopebar
	trackscopeview_init(&self->trackscopeview, &self->top, &self->workspace);
	if (!patternviewconfig_showtrackscopes(psycleconfig_patview(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopes_stop(&self->trackscopeview.scopes);
	}	
	psy_ui_component_init(&self->topspacer, &self->component, NULL);
	psy_ui_component_setalign(&self->topspacer, psy_ui_ALIGN_TOP);
	psy_ui_component_preventalign(&self->topspacer);
	psy_ui_component_setpreferredsize(&self->topspacer,
		psy_ui_size_make_em(0.0, 0.5));
	psy_ui_component_setstyletype(&self->topspacer, STYLE_TOP);
}

void mainframe_inittabbars(MainFrame* self)
{	
	psy_ui_component_init(&self->maximize, &self->mainviews, NULL);
	psy_ui_component_setstyletype(&self->maximize, STYLE_MAINVIEWTOPBAR);
	psy_ui_component_setalign(&self->maximize, psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->tabbars, &self->maximize, NULL,
		psy_ui_ALIGN_CLIENT);	
	psy_ui_button_init_connect(&self->maximizebtn, &self->maximize, NULL,
		self, mainframe_onmaxminimizeview);
	psy_ui_component_setalign(psy_ui_button_base(&self->maximizebtn),
		psy_ui_ALIGN_RIGHT);
	psy_ui_button_setbitmapresource(&self->maximizebtn, IDB_EXPAND_DARK);
	psy_ui_button_setbitmaptransparency(&self->maximizebtn,
		psy_ui_colour_white());
	psy_ui_component_setspacing(psy_ui_button_base(&self->maximizebtn),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_component_init(&self->tabspacer, &self->mainviews, NULL);
	psy_ui_component_setalign(&self->tabspacer, psy_ui_ALIGN_TOP);
	psy_ui_component_preventalign(&self->tabspacer);
	psy_ui_component_setpreferredsize(&self->tabspacer,
		psy_ui_size_make_em(0.0, 0.4));
}

void mainframe_initnavigation(MainFrame* self)
{
	navigation_init(&self->navigation, &self->tabbars, &self->workspace);	
	psy_ui_component_setalign(navigation_base(&self->navigation),
		psy_ui_ALIGN_LEFT);
}

void mainframe_initmaintabbar(MainFrame* self)
{	
	psy_ui_Tab* tab;	

	psy_ui_tabbar_init(&self->tabbar, &self->tabbars);		
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);	
	tab = psy_ui_tabbar_append(&self->tabbar, "main.machines");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_MACHINES_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_white());
	tab = psy_ui_tabbar_append(&self->tabbar, "main.patterns");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_NOTES_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_white());
	psy_ui_tabbar_append_tabs(&self->tabbar, "main.samples",
		"main.instruments", "main.properties", NULL);
}

void mainframe_inithelpsettingstabbar(MainFrame* self)
{
	psy_ui_Tab* tab;	
	
	psy_ui_tabbar_init(&self->helpsettingstabbar, &self->tabbars);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->helpsettingstabbar),
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_setmargin(psy_ui_tabbar_base(&self->helpsettingstabbar),
		psy_ui_margin_make_em(0.0, 4.0, 0.0, 4.0));
	tab = psy_ui_tabbar_append(&self->helpsettingstabbar, "main.settings");	
	psy_ui_bitmap_loadresource(&tab->icon, IDB_SETTINGS_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_white());
	tab = psy_ui_tabbar_append(&self->helpsettingstabbar, "main.help");	
}

void mainframe_initviewtabbars(MainFrame* self)
{
	psy_ui_notebook_init(&self->viewtabbars, &self->tabbars);
	psy_ui_component_setalign(&self->viewtabbars.component, psy_ui_ALIGN_LEFT);
}

void mainframe_initmainviews(MainFrame* self)
{
	psy_ui_notebook_init(&self->notebook, &self->mainviews);
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
		&self->workspace.config.config, 3,
		&self->workspace);		
	psy_signal_connect(&self->settingsview.signal_changed, self,
		mainframe_onsettingsviewchanged);
	helpview_init(&self->helpview, psy_ui_notebook_base(&self->notebook),
		psy_ui_notebook_base(&self->viewtabbars), &self->workspace);	
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
	psy_signal_connect(&self->helpsettingstabbar.signal_change, self,
		mainframe_onsettingshelptabbarchanged);
}

void mainframe_initgear(MainFrame* self)
{		
	gear_init(&self->gear, &self->client, &self->workspace);
	psy_ui_component_hide(gear_base(&self->gear));
	psy_ui_component_setalign(gear_base(&self->gear), psy_ui_ALIGN_RIGHT);	
	psy_ui_splitter_init(&self->gearsplitter, &self->client);
	psy_ui_component_setalign(psy_ui_splitter_base(&self->gearsplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_settoggle(&self->gearsplitter,
		gear_base(&self->gear));
	psy_ui_splitter_setbutton(&self->gearsplitter,
		&self->machinebar.gear);
	psy_ui_component_hide(psy_ui_splitter_base(&self->gearsplitter));
}

void mainframe_initparamrack(MainFrame* self)
{	
	paramrack_init(&self->paramrack, &self->client, &self->workspace);
	psy_ui_component_hide(paramrack_base(&self->paramrack));
	psy_ui_component_setalign(&self->paramrack.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_splitter_init(&self->splitbarparamrack, &self->client);
	psy_ui_component_setalign(psy_ui_splitter_base(&self->splitbarparamrack),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_splitter_settoggle(&self->splitbarparamrack,
		&self->paramrack.component);
	psy_ui_splitter_setbutton(&self->splitbarparamrack,
		&self->machinebar.dock);
	psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarparamrack));	
}

void mainframe_initcpuview(MainFrame* self)
{
	cpuview_init(&self->cpuview, &self->client, &self->workspace);
	psy_ui_component_hide(cpuview_base(&self->cpuview));
	psy_ui_component_setalign(cpuview_base(&self->cpuview), psy_ui_ALIGN_RIGHT);	
	psy_ui_splitter_init(&self->cpusplitter, &self->client);
	psy_ui_component_setalign(psy_ui_splitter_base(&self->cpusplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_settoggle(&self->cpusplitter,
		cpuview_base(&self->cpuview));
	psy_ui_splitter_setbutton(&self->cpusplitter,
		&self->machinebar.cpu);
	psy_ui_component_hide(psy_ui_splitter_base(&self->cpusplitter));
}

void mainframe_initmidimonitor(MainFrame* self)
{
	midimonitor_init(&self->midimonitor, &self->client, &self->workspace);
	psy_ui_component_hide(midimonitor_base(&self->midimonitor));
	psy_ui_component_setalign(midimonitor_base(&self->midimonitor), psy_ui_ALIGN_RIGHT);	
	psy_ui_splitter_init(&self->midisplitter, &self->client);
	psy_ui_component_setalign(psy_ui_splitter_base(&self->midisplitter),
		psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_settoggle(&self->midisplitter,
		midimonitor_base(&self->midimonitor));
	psy_ui_splitter_setbutton(&self->midisplitter,
		&self->machinebar.midi);
	psy_ui_component_hide(psy_ui_splitter_base(&self->midisplitter));	
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
	psy_ui_splitter_init(&self->splitseqeditor, &self->client);
	psy_ui_component_setalign(psy_ui_splitter_base(&self->splitseqeditor),
		psy_ui_ALIGN_BOTTOM);
	if (!generalconfig_showsequenceedit(psycleconfig_general(
		workspace_conf(&self->workspace)))) {
		psy_ui_component_hide(seqeditor_base(&self->seqeditor));
		psy_ui_component_hide(psy_ui_splitter_base(&self->splitseqeditor));
	}	
}

void mainframe_initrecentview(MainFrame* self)
{
	playlistview_init(&self->playlist, mainframe_base(self),
		psy_ui_notebook_base(&self->viewtabbars),
		&self->workspace);
	psy_ui_component_setalign(playlistview_base(&self->playlist),
		psy_ui_ALIGN_LEFT);
	psy_ui_splitter_init(&self->playlistsplitter, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_splitter_base(&self->playlistsplitter),
		psy_ui_ALIGN_LEFT);
	if (!generalconfig_showplaylist(&self->workspace.config.general)) {
		psy_ui_component_hide(playlistview_base(&self->playlist));
		psy_ui_component_hide(psy_ui_splitter_base(&self->playlistsplitter));
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
	psy_ui_splitter_init(&self->splitbarplugineditor, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_splitter_base(&self->splitbarplugineditor),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(psy_ui_splitter_base(&self->splitbarplugineditor));
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
	psy_signal_connect(&self->workspace.signal_selectpatterndisplay, self,
		mainframe_onselectpatterndisplay);
	psy_signal_connect(&self->workspace.signal_floatsection, self,
		mainframe_onfloatsection);
	psy_signal_connect(&self->workspace.signal_docksection, self,
		mainframe_ondocksection);
	psy_audio_eventdrivers_setcallback(&self->workspace.player.eventdrivers,
		(EVENTDRIVERWORKFN)mainframe_eventdrivercallback, self);
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
	psy_signal_connect(&self->workspace.signal_gearselect, self,
		mainframe_ongearselect);
	psy_signal_connect(&workspace_song(
		&self->workspace)->patterns.signal_numsongtrackschanged,
		self, mainframe_onsongtrackschanged);	
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
	case CMD_IMM_HELPSHORTCUT:
		mainframe_ontogglekbdhelp(self, mainframe_base(self));
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
		mainframe_maxminimizeview(self);
		break;		
	case CMD_IMM_TERMINAL:
		mainframe_ontoggleterminal(self, mainframe_base(self));
		break;	
	case CMD_EDT_EDITQUANTIZEDEC:
		workspace_editquantizechange(&self->workspace, -1);
		patterncursorstepbox_update(&self->patternbar.cursorstep);
		break;
	case CMD_EDT_EDITQUANTIZEINC:
		workspace_editquantizechange(&self->workspace, 1);
		patterncursorstepbox_update(&self->patternbar.cursorstep);
		break;		
	}
}

void mainframe_maxminimizeview(MainFrame* self)
{
	if (self->minmaximize) {
		psy_List* p;
		bool show;

		show = TRUE;
		for (p = self->minmaximize; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (p == self->minmaximize) {
				show = !psy_ui_component_visible(component);
			}
			if (show) {
				psy_ui_component_show(component);
			} else {
				psy_ui_component_hide(component);
			}
		}
		psy_ui_component_align(&self->component);
	}	
}

void mainframe_onsongloadprogress(MainFrame* self, Workspace* workspace,
	intptr_t progress)
{	
	if (progress == -1) {
		psy_ui_terminal_output(&self->terminal, "\n");
	}
	psy_ui_progressbar_setprogress(&self->statusbar.progressbar, progress / 100.f);
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
	if (workspace_song(&self->workspace)) {
		psy_signal_connect(
			&workspace_song(&self->workspace)->patterns.signal_numsongtrackschanged,
			self, mainframe_onsongtrackschanged);
	}
	vubar_reset(&self->vubar);
	psy_ui_component_align(&self->client);	
	psy_ui_component_align(mainframe_base(self));
	if (flag == WORKSPACE_NEWSONG) {
		machinewireview_centermaster(&self->machineview.wireview);
	}
}

void mainframe_updatesongtitle(MainFrame* self)
{		
	char title[512];

	workspace_apptitle(&self->workspace, title);	
	psy_ui_component_settitle(mainframe_base(self), title);	
	mainstatusbar_setdefaultstatustext(&self->statusbar,
		workspace_songtitle(&self->workspace));
}

void mainframe_ontogglegearworkspace(MainFrame* self, Workspace* sender)
{
	if (psy_ui_component_visible(&self->gearsplitter.component)) {
		psy_ui_component_hide(&self->gearsplitter.component);
	} else {
		psy_ui_component_show(&self->gearsplitter.component);
	}
}

void mainframe_resetfocus(MainFrame* self)
{
	if (!psy_ui_component_visible(&self->paramrack.component) &&
		psy_ui_notebook_activepage(&self->notebook)) {
		psy_ui_component_setfocus(psy_ui_notebook_activepage(
			&self->notebook));
	}
}

void mainframe_onmaxminimizeview(MainFrame* self, psy_ui_Button* sender)
{
	mainframe_maxminimizeview(self);
}

void mainframe_onrecentsongs(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(playlistview_base(&self->playlist))) {
		psy_ui_button_seticon(&self->filebar.recentbutton, psy_ui_ICON_MORE);
		psy_ui_component_hide(playlistview_base(&self->playlist));
		psy_ui_component_hide_align(psy_ui_splitter_base(&self->playlistsplitter));
	} else {
		psy_ui_button_seticon(&self->filebar.recentbutton, psy_ui_ICON_LESS);
		playlistview_base(&self->playlist)->visible = 1;		
		psy_ui_splitter_base(&self->playlistsplitter)->visible = 1;
		psy_ui_component_align(&self->component);
		psy_ui_component_show(playlistview_base(&self->playlist));
		psy_ui_component_show(psy_ui_splitter_base(&self->playlistsplitter));		
	}
	psy_ui_component_invalidate(mainframe_base(self));
	generalconfig_setplaylistshowstate(&self->workspace.config.general,
		psy_ui_component_visible(playlistview_base(&self->playlist)));	
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
		psy_ui_component_show(&self->splitbarplugineditor.component);
	}
	psy_ui_component_togglevisibility(&self->plugineditor.component);
}

void mainframe_onsettingsviewchanged(MainFrame* self, PropertiesView* sender,
	psy_Property* property, uintptr_t* rebuild_level)
{
	switch (psy_property_id(property)) {
	case PROPERTY_ID_SHOWSEQUENCEEDIT:
		updateshowstate(property, seqeditor_base(&self->seqeditor));		
		break;
	case PROPERTY_ID_SHOWSTEPSEQUENCER:
		updateshowstate(property,
			stepsequencerview_base(&self->stepsequencerview));				
		break;
	case PROPERTY_ID_SHOWPLAYLIST:
		updateshowstate(property, playlistview_base(&self->playlist));		
		break;
	case PROPERTY_ID_TRACKSCOPES:
		updateshowstate(property, trackscopeview_base(&self->trackscopeview));
		if (psy_property_item_bool(property)) {				
			trackscopes_start(&self->trackscopeview.scopes);
		} else {
			trackscopes_stop(&self->trackscopeview.scopes);
		}
		break;
	case PROPERTY_ID_SHOWMETRONOME:
		updateshowstate(property, metronomebar_base(&self->metronomebar));		
		break;	
	default:
		workspace_configurationchanged(&self->workspace, property, rebuild_level);
		break;
	}
}

void updateshowstate(psy_Property* property, psy_ui_Component* component)
{	
	if (psy_property_item_bool(property)) {
		psy_ui_component_show_align(component);			
	} else {
		psy_ui_component_hide_align(component);
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
		self->startup = FALSE;
	}
	workspace_idle(&self->workspace);	
	if (self->pluginscanprogress != -1) {
		if (self->pluginscanprogress == 0) {
			psy_ui_progressbar_setprogress(&self->statusbar.progressbar, 0);
			self->pluginscanprogress = -1;
		} else {
			psy_ui_progressbar_tick(&self->statusbar.progressbar);
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
	if (psy_ui_component_visible(machineview_base(&self->machineview))) {
		machineview_centermaster(&self->machineview);
	}
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
		psy_ui_tabbar_select(&self->tabbar, index);
		if (section != psy_INDEX_INVALID) {
			psy_ui_component_selectsection(view, section, options);
		}
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
	psy_ui_tabbar_unmark(&self->helpsettingstabbar);
	psy_ui_notebook_select(&self->statusbar.viewstatusbars, tabindex);
	psy_ui_notebook_select(&self->viewtabbars, tabindex);
	component = psy_ui_notebook_activepage(&self->notebook);
	if (component) {				
		workspace_onviewchanged(&self->workspace, viewhistoryentry_make(
			tabindex, psy_ui_component_section(component), psy_INDEX_INVALID));
		psy_ui_component_setfocus(component);
	}
	psy_ui_component_align(&self->component);	
}

void mainframe_onsettingshelptabbarchanged(MainFrame* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	uintptr_t viewid;
	psy_ui_Component* component;

	psy_ui_tabbar_unmark(&self->tabbar);
	switch (tabindex) {
		case 0:
			viewid = VIEW_ID_SETTINGSVIEW;
			break;
		case 1:
			viewid = VIEW_ID_HELPVIEW;
			break;		
		default:
			viewid = VIEW_ID_SETTINGSVIEW;
			break;
	}
	psy_ui_notebook_select(&self->notebook, viewid);
	psy_ui_notebook_select(&self->statusbar.viewstatusbars, viewid);
	psy_ui_notebook_select(&self->viewtabbars, viewid);
	component = psy_ui_notebook_activepage(&self->notebook);
	if (component) {		
		workspace_onviewchanged(&self->workspace, viewhistoryentry_make(viewid,
			psy_ui_component_section(component), psy_INDEX_INVALID));
		psy_ui_component_setfocus(component);
	}
	psy_ui_component_align(&self->component);
}

void mainframe_onterminaloutput(MainFrame* self, Workspace* sender,
	const char* text)
{
	if (self->statusbar.terminalstyleid == STYLE_TERM_BUTTON) {
		mainstatusbar_updateterminalbutton(&self->statusbar);
	}
	psy_ui_terminal_output(&self->terminal, text);	
}

void mainframe_onterminalwarning(MainFrame* self, Workspace* sender,
	const char* text)
{
	if (self->statusbar.terminalstyleid != STYLE_TERM_BUTTON_ERROR) {
		self->statusbar.terminalstyleid = STYLE_TERM_BUTTON_WARNING;
		mainstatusbar_updateterminalbutton(&self->statusbar);
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
	self->statusbar.terminalstyleid = STYLE_TERM_BUTTON_ERROR;	
	mainstatusbar_updateterminalbutton(&self->statusbar);
}

void mainframe_onzoomboxchanged(MainFrame* self, ZoomBox* sender)
{	
	// psy_ui_app()->setpositioncacheonly = TRUE;
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
	if (view == VIEW_ID_HELPVIEW && section == HELPVIEWSECTION_HELP) {
		helpview_float(&self->helpview, section, &self->right);
		psy_ui_component_align(&self->client);		
	}
}

void mainframe_ondocksection(MainFrame* self, Workspace* sender,
	int view, uintptr_t section)
{
	if (view == VIEW_ID_HELPVIEW && section == HELPVIEWSECTION_HELP) {
		helpview_dock(&self->helpview, section, &self->right);
		psy_ui_component_align(&self->client);		
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
	workspace_save_configuration(&self->workspace);
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
	psy_ui_component_togglevisibility(psy_ui_splitter_base(
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
			psy_ui_component_hide(psy_ui_splitter_base(&self->splitseqeditor));
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
	if (!psy_ui_isvaluezero(psy_ui_component_scrollsize(
			&self->terminal.component).height)) {
		psy_ui_component_setpreferredsize(&self->terminal.component,
			psy_ui_size_zero());
		self->statusbar.terminalstyleid = STYLE_TERM_BUTTON;
		mainstatusbar_updateterminalbutton(&self->statusbar);
	} else {
		psy_ui_component_setpreferredsize(&self->terminal.component,
			psy_ui_size_make(psy_ui_value_make_px(0),
				psy_ui_value_make_eh(10.0)));		
	}
	psy_ui_component_align(mainframe_base(self));
	psy_ui_component_invalidate(mainframe_base(self));	
}

void mainframe_ontogglekbdhelp(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_togglevisibility(kbdhelp_base(&self->kbdhelp));	
}

void mainframe_onexit(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_app_close(psy_ui_app());
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

void mainframe_onkeydown(MainFrame* self, psy_ui_KeyboardEvent* ev)
{	
	mainframe_checkplaystartwithrctrl(self, ev);
	mainframe_delegatekeyboard(self, psy_EVENTDRIVER_KEYDOWN, ev);
}

void mainframe_checkplaystartwithrctrl(MainFrame* self, psy_ui_KeyboardEvent* ev)
{
	if (keyboardmiscconfig_playstartwithrctrl(
			&self->workspace.config.misc)) {
		if (ev->keycode == psy_ui_KEY_CONTROL) {
			// todo: this win32 detection only
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

void mainframe_onkeyup(MainFrame* self, psy_ui_KeyboardEvent* ev)
{
	mainframe_delegatekeyboard(self, psy_EVENTDRIVER_KEYUP, ev);
}

// delegate keyboard events to the keyboard driver
void mainframe_delegatekeyboard(MainFrame* self, intptr_t message,
	psy_ui_KeyboardEvent* ev)
{
	psy_eventdriver_write(workspace_kbddriver(&self->workspace),
		psy_eventdriverinput_make(message,
			psy_audio_encodeinput(ev->keycode, ev->shift_key, ev->ctrl_key, ev->alt_key),
				ev->repeat, workspace_currview(&self->workspace).id));
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

void mainframe_ondragover(MainFrame* self, psy_ui_DragEvent* ev)
{
	if (ev->dataTransfer) {
		psy_Property* p;

		p = psy_property_at_section(ev->dataTransfer, "data");
		if (p) {
			p = psy_property_at(p, "dragview", PSY_PROPERTY_TYPE_STRING);
			if (p) {
				ev->mouse.event.default_prevented = TRUE;
			}
		}
	}
}

void mainframe_ondrop(MainFrame* self, psy_ui_DragEvent* ev)
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
						psy_ui_component_setalign(view, psy_ui_ALIGN_LEFT);
					} else {
						psy_ui_component_setalign(view, psy_ui_ALIGN_RIGHT);
					}
					if (splitter) {
						splitter->align = view->align;
					}
					psy_ui_component_align(psy_ui_component_parent(view));
					psy_ui_component_invalidate(psy_ui_component_parent(view));
				}				
				ev->mouse.event.default_prevented = TRUE;
			}
		}
	}
}
