// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
// local
#include "cmdsgeneral.h"
#include "paramview.h"
#include "resources/resource.h"
#include "settingsview.h"
// audio
#include <songio.h>
// ui
#include <uiapp.h>
#include <uiopendialog.h>
#include <uisavedialog.h>
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

#define GEARVIEW 10

// MainFrame
// prototypes
static void mainframe_initframe(MainFrame*);
static void mainframe_ondestroyed(MainFrame*);
static void mainframe_initworkspace(MainFrame*);
static void mainframe_initemptystatusbar(MainFrame*);
static void mainframe_inittoparea(MainFrame*);
static void mainframe_initclientarea(MainFrame*);
static void mainframe_initterminal(MainFrame*);
static void mainframe_initkbdhelp(MainFrame*);
static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initstatusbarlabel(MainFrame*, psy_ui_Margin*);
static void mainframe_initkbdhelpbutton(MainFrame*);
static void mainframe_initterminalbutton(MainFrame*);
static void mainframe_initprogressbar(MainFrame*);
static void mainframe_inittabbars(MainFrame*);
static void mainframe_initnavigation(MainFrame*);
static void mainframe_initmaintabbar(MainFrame*);
static void mainframe_initmainviews(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_initgear(MainFrame*);
static void mainframe_initcpuview(MainFrame*);
static void mainframe_initmidimonitor(MainFrame*);
static void mainframe_initstepsequencerview(MainFrame*);
static void mainframe_initseqeditor(MainFrame*);
static void mainframe_initrecentview(MainFrame*);
static void mainframe_initfileview(MainFrame*);
static void mainframe_initsequenceview(MainFrame*);
static void mainframe_initplugineditor(MainFrame*);
static void mainframe_connectworkspace(MainFrame*);
static void mainframe_initinterpreter(MainFrame*);
static void mainframe_updatetext(MainFrame*, Translator*);
static void mainframe_onskinchanged(MainFrame*, Workspace*);
static void mainframe_onkeydown(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_onkeyup(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_delegatekeyboard(MainFrame*, intptr_t message,
	psy_ui_KeyEvent*);
static void mainframe_onmousedown(MainFrame*, psy_ui_MouseEvent*);
static void mainframe_onsequenceselchange(MainFrame*,
	psy_audio_SequenceEntry*);
static void mainframe_ontogglegear(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglegearworkspace(MainFrame*, Workspace* sender);
static void mainframe_onhidegear(MainFrame*, psy_ui_Component* sender);
static void mainframe_onhidemidimonitor(MainFrame*, psy_ui_Component* sender);
static void mainframe_oncpu(MainFrame*, psy_ui_Component* sender);
static void mainframe_onhidecpu(MainFrame*, psy_ui_Component* sender);
static void mainframe_onmidi(MainFrame*, psy_ui_Component* sender);
static void mainframe_onrecentsongs(MainFrame*, psy_ui_Component* sender);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileloadview(MainFrame*, psy_ui_Component* sender);
#endif
static void mainframe_onplugineditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ongearcreate(MainFrame*, psy_ui_Component* sender);
static void mainframe_onaboutok(MainFrame*, psy_ui_Component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, PropertiesView* sender,
	psy_Property*);
static void mainframe_ontabbarchanged(MainFrame*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void mainframe_onsongchanged(MainFrame*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void mainframe_onsongloadprogress(MainFrame*, Workspace*, int progress);
static void mainframe_onpluginscanprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onviewselected(MainFrame*, Workspace*, int view,
	uintptr_t section, int option);
static void mainframe_onrender(MainFrame*, psy_ui_Component* sender);
static void mainframe_updatesongtitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, uintptr_t timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, psy_EventDriver* sender);
static void mainframe_ontoggleseqeditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontoggleterminal(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglekbdhelp(MainFrame*, psy_ui_Component* sender);
static void mainframe_onselectpatterndisplay(MainFrame*, psy_ui_Component* sender, PatternDisplayType);
static void mainframe_onterminaloutput(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalwarning(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalerror(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_updateterminalbutton(MainFrame* self);
static void mainframe_onzoomboxchanged(MainFrame*, ZoomBox* sender);
static void mainframe_onsongtrackschanged(MainFrame*, psy_audio_Player* sender,
	uintptr_t numsongtracks);
static void mainframe_onchangecontrolskin(MainFrame*, Workspace* sender,
	const char* path);
static void mainframe_ondockview(MainFrame*, Workspace* sender,
	psy_ui_Component* view);
static void mainframe_onlanguagechanged(MainFrame*, Translator* sender);
static bool mainframe_onclose(MainFrame*);
static void mainframe_oncheckunsaved(MainFrame*, CheckUnsavedBox* sender,
	int option, int mode);
static void mainframe_onstartup(MainFrame*);
// EventDriverCallback
static int mainframe_eventdrivercallback(MainFrame*, int msg, int param1,
	int param2);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileload(MainFrame*, FileView* sender);
#endif
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
		vtable.onkeydown = (psy_ui_fp_component_onkeydown)mainframe_onkeydown;
		vtable.onkeyup = (psy_ui_fp_component_onkeyup)mainframe_onkeyup;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			mainframe_onmousedown;
		vtable.ontimer = (psy_ui_fp_component_ontimer)mainframe_ontimer;
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
	mainframe_initterminal(self);
	mainframe_initkbdhelp(self);
	mainframe_inittoparea(self);	
	mainframe_initclientarea(self);
	mainframe_inittabbars(self);
	mainframe_initbars(self);	
	mainframe_initnavigation(self);
	mainframe_initmaintabbar(self);	
	mainframe_initmainviews(self);
	mainframe_initgear(self);
	mainframe_initcpuview(self);
	mainframe_initmidimonitor(self);		
	mainframe_initstepsequencerview(self);	
	mainframe_initseqeditor(self);
	mainframe_initrecentview(self);	
	mainframe_initfileview(self);
	mainframe_initsequenceview(self);			
	mainframe_initplugineditor(self);
	mainframe_initstatusbar(self);	
	mainframe_setstartpage(self);
	mainframe_updatesongtitle(self);
	mainframe_updatetext(self, workspace_translator(&self->workspace));
	mainframe_initinterpreter(self);
	mainframe_updateterminalbutton(self);
	mainframe_connectworkspace(self);	
}

void mainframe_initframe(MainFrame* self)
{
	self->startup = TRUE;
	psy_ui_frame_init_main(mainframe_base(self));
	psy_ui_component_setvtable(mainframe_base(self), vtable_init(self));
	psy_ui_component_seticonressource(mainframe_base(self), IDI_PSYCLEICON);
}

void mainframe_ondestroyed(MainFrame* self)
{
	extern psy_ui_App app;

	workspace_save_configuration(&self->workspace);
	workspace_dispose(&self->workspace);
	interpreter_dispose(&self->interpreter);
	psy_ui_app_stop(&app);
}

void mainframe_updatetext(MainFrame* self, Translator* translator)
{
	assert(self);
	
	if (translator) {
		tabbar_rename_tabs(&self->tabbar,
			translator_translate(translator, "main.machines"),
			translator_translate(translator, "main.patterns"),
			translator_translate(translator, "main.samples"),
			translator_translate(translator, "main.instruments"),
			translator_translate(translator, "main.properties"),
			translator_translate(translator, "main.settings"),
			translator_translate(translator, "help.help"),
			NULL);
		psy_ui_button_settext(&self->toggleterminal,
			translator_translate(translator, "main.terminal"));
		psy_ui_button_settext(&self->togglekbdhelp,
			translator_translate(translator, "main.kbd"));
	} else {
		tabbar_rename_tabs(&self->tabbar,
			"Machines", "Patterns", "Samples", "Instruments", "Properties",
			"Settings", "Help",
			NULL);
		psy_ui_button_settext(&self->toggleterminal, "Terminal");
		psy_ui_button_settext(&self->togglekbdhelp, "Kbd");
	}
}

void mainframe_onlanguagechanged(MainFrame* self, Translator* sender)
{
	mainframe_updatetext(self, sender);
	psy_ui_component_alignall(mainframe_base(self));
}

void mainframe_initworkspace(MainFrame* self)
{
	self->startpage = FALSE;
	workspace_init(&self->workspace, mainframe_base(self));
	workspace_load_configuration(&self->workspace);
	workspace_load_recentsongs(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
}

void mainframe_initemptystatusbar(MainFrame* self)
{
	psy_ui_component_init(&self->statusbar, mainframe_base(self));
	psy_ui_component_setalign(&self->statusbar, psy_ui_ALIGN_BOTTOM);
}

void mainframe_inittoparea(MainFrame* self)
{
	psy_ui_component_init(&self->top, mainframe_base(self));
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
}

void mainframe_initclientarea(MainFrame* self)
{
	psy_ui_component_init(&self->client, mainframe_base(self));
	psy_ui_component_setbackgroundmode(&self->client,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_init(&self->paramviews, &self->client);
	psy_ui_component_setalign(&self->paramviews, psy_ui_ALIGN_BOTTOM);
}

void mainframe_initterminal(MainFrame* self)
{
	psy_ui_terminal_init(&self->terminal, mainframe_base(self));
	psy_signal_connect(&self->workspace.signal_terminal_warning, self,
		mainframe_onterminalwarning);
	psy_signal_connect(&self->workspace.signal_terminal_out, self,
		mainframe_onterminaloutput);
	psy_signal_connect(&self->workspace.signal_terminal_error, self,
		mainframe_onterminalerror);
	psy_ui_component_setalign(psy_ui_terminal_base(&self->terminal),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_resize(psy_ui_terminal_base(&self->terminal),
		psy_ui_size_zero());
	psy_ui_splitbar_init(&self->splitbarterminal, mainframe_base(self));
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitbarterminal),
		psy_ui_ALIGN_BOTTOM);		
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
	psy_ui_Margin margin;

	zoombox_init(&self->zoombox, &self->statusbar);	
	psy_signal_connect(&self->zoombox.signal_changed, self,
		mainframe_onzoomboxchanged);
	mainframe_initstatusbarlabel(self, &margin);	
	psy_ui_notebook_init(&self->viewstatusbars, &self->statusbar);	
	machineviewbar_init(&self->machineviewbar, &self->viewstatusbars.component,
		&self->workspace);	
	self->machineview.wireview.statusbar = &self->machineviewbar;
	patternviewbar_init(&self->patternbar,
		psy_ui_notebook_base(&self->viewstatusbars), &self->workspace);
	sampleeditorbar_init(&self->samplesview.sampleeditor.sampleeditortbar,
		psy_ui_notebook_base(&self->viewstatusbars),
		&self->samplesview.sampleeditor,
		&self->workspace);
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.25),
		psy_ui_value_makeew(1.0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(1.0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(psy_ui_notebook_base(&self->viewstatusbars),
		psy_ui_NONRECURSIVE), psy_ui_ALIGN_LEFT, &margin));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->statusbar, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
	psy_ui_notebook_setpageindex(&self->viewstatusbars, 0);
	mainframe_initkbdhelpbutton(self);
	mainframe_initterminalbutton(self);		
	mainframe_initprogressbar(self);
}

void mainframe_initstatusbarlabel(MainFrame* self, psy_ui_Margin* margin)
{
	psy_ui_label_init(&self->statusbarlabel, &self->statusbar);
	psy_ui_label_settext(&self->statusbarlabel, "Ready");
	psy_ui_label_setcharnumber(&self->statusbarlabel, 29);
	psy_ui_component_setmargin(psy_ui_label_base(&self->statusbarlabel),
		margin);
	psy_ui_component_setalign(psy_ui_label_base(&self->statusbarlabel),
		psy_ui_ALIGN_LEFT);
}

void mainframe_initkbdhelpbutton(MainFrame* self)
{
	psy_ui_button_init(&self->togglekbdhelp, &self->statusbar);
	psy_ui_button_settext(&self->togglekbdhelp, "Kbd");
	psy_ui_component_setalign(psy_ui_button_base(&self->togglekbdhelp),
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->togglekbdhelp.signal_clicked, self,
		mainframe_ontogglekbdhelp);
}

void mainframe_initterminalbutton(MainFrame* self)
{
	self->terminalbutton_colours[TERMINALMSGTYPE_NONE] =
		psy_ui_component_colour(psy_ui_button_base(&self->toggleterminal));
	self->terminalbutton_colours[TERMINALMSGTYPE_ERROR] =
		psy_ui_colour_make(0x000000FF);
	self->terminalbutton_colours[TERMINALMSGTYPE_WARNING] =
		psy_ui_colour_make(0x0000FFFF);
	self->terminalbutton_colours[TERMINALMSGTYPE_MESSAGE] =
		psy_ui_colour_make(0x0000FF00);
	self->terminalmsgtype = TERMINALMSGTYPE_NONE;
	psy_ui_button_init(&self->toggleterminal, &self->statusbar);
	psy_ui_button_settext(&self->toggleterminal, "  Terminal");
	psy_ui_component_setalign(psy_ui_button_base(&self->toggleterminal),
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->toggleterminal.signal_clicked, self,
		mainframe_ontoggleterminal);
}

void mainframe_initprogressbar(MainFrame* self)
{
	psy_ui_progressbar_init(&self->progressbar, &self->statusbar);
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
	psy_ui_Margin row0margin;
	psy_ui_Margin rowmargin;	

	psy_ui_margin_init_all(&row0margin, psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeeh(0.5));
	psy_ui_margin_init_all(&rowmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeeh(0.5));	
	psy_ui_component_setalign(&self->toprow0, psy_ui_ALIGN_TOP);
	// Vugroup
	psy_ui_component_init(&self->topright, &self->top);
	psy_ui_component_setalign(&self->topright, psy_ui_ALIGN_RIGHT);
	vubar_init(&self->vubar, &self->topright, &self->workspace);	
	psy_ui_component_setalign(&self->vubar.component, psy_ui_ALIGN_TOP);
	// row0
	psy_ui_component_init(&self->toprow0, &self->top);
	psy_ui_component_setalign(&self->toprow0, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->toprow0, &row0margin);
	// row1
	psy_ui_component_init(&self->toprow1, &self->top);
	psy_ui_component_setalign(&self->toprow1, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->toprow1, &rowmargin);
	// row2
	psy_ui_component_init(&self->toprow2, &self->top);
	psy_ui_component_setalign(&self->toprow2, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->toprow2, &rowmargin);
	// scopebar
	trackscopeview_init(&self->trackscopeview, &self->top, &self->workspace);
	psy_ui_component_setalign(trackscopeview_base(&self->trackscopeview),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(trackscopeview_base(&self->trackscopeview),
		&rowmargin);
	if (!workspace_showtrackscopes(&self->workspace)) {
		psy_ui_component_hide(trackscopeview_base(&self->trackscopeview));
		trackscopeview_stop(&self->trackscopeview);
	}
	// add bars to rows
	// row0
	filebar_init(&self->filebar, &self->toprow0, &self->workspace);	
	undoredobar_init(&self->undoredobar, &self->toprow0, &self->workspace);	
	playbar_init(&self->playbar, &self->toprow0, &self->workspace);	
	playposbar_init(&self->playposbar, &self->toprow0, &self->workspace);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->toprow0, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	margin.right = psy_ui_value_makepx(0);
	psy_ui_component_setmargin(playposbar_base(&self->playposbar), &margin);
	// row1
	// Songbar	
	songbar_init(&self->songbar, &self->toprow1, &self->workspace);
	psy_ui_component_setalign(songbar_base(&self->songbar), psy_ui_ALIGN_LEFT);
	// row2
	// Machinebar
	machinebar_init(&self->machinebar, &self->toprow2, &self->workspace);	
	psy_ui_component_setalign(machinebar_base(&self->machinebar),
		psy_ui_ALIGN_LEFT);
}

void mainframe_inittabbars(MainFrame* self)
{
	psy_ui_Margin spacing;

	psy_ui_margin_init_all(&spacing, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->tabbars, &self->client);
	psy_ui_component_setspacing(&self->tabbars, &spacing);
	psy_ui_component_setalign(&self->tabbars, psy_ui_ALIGN_TOP);
}

void mainframe_initnavigation(MainFrame* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0));
	navigation_init(&self->navigation, &self->tabbars, &self->workspace);	
	psy_ui_component_setalign(navigation_base(&self->navigation),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(navigation_base(&self->navigation), &margin);
}

void mainframe_initmaintabbar(MainFrame* self)
{
	tabbar_init(&self->tabbar, &self->tabbars);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	psy_ui_component_setalignexpand(tabbar_base(&self->tabbar),
		psy_ui_HORIZONTALEXPAND);
	// Machines, Patterns, Samples, Instruments, Properties
	// text set in updatetext with translation
	tabbar_append_tabs(&self->tabbar, "", "", "", "", "", NULL);
	// ident setting help tabs
	tabbar_append(&self->tabbar, "" /* Settings */)->margin.left =
		psy_ui_value_makeew(4.0);
	tabbar_append(&self->tabbar, "" /* Help */)->margin.right =
		psy_ui_value_makeew(4.0);
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
		&self->viewtabbars.component, &self->workspace);
	samplesview_init(&self->samplesview, psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component, &self->workspace);
	instrumentview_init(&self->instrumentsview,
		psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component,
		&self->workspace);
	songpropertiesview_init(&self->songpropertiesview,
		psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component,
		&self->workspace);
	propertiesview_init(&self->settingsview,
		psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component,
		&self->workspace.config,
		&self->workspace);
	psy_signal_connect(&self->settingsview.signal_changed, self,
		mainframe_onsettingsviewchanged);
	helpview_init(&self->helpview, psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component, &self->workspace);
	psy_signal_connect(&self->helpview.about.okbutton.signal_clicked, self,
		mainframe_onaboutok);
	renderview_init(&self->renderview, psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component, &self->workspace);
	psy_signal_connect(&self->filebar.renderbutton.signal_clicked, self,
		mainframe_onrender);
	psy_signal_connect(&self->workspace.signal_viewselected, self,
		mainframe_onviewselected);
	checkunsavedbox_init(&self->checkunsavedbox,
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
	psy_signal_connect(&self->gear.buttons.createreplace.signal_clicked, self,
		mainframe_ongearcreate);
	psy_signal_connect(&gear_base(&self->gear)->signal_hide, self,
		mainframe_onhidegear);
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
	if (!workspace_showstepsequencer(&self->workspace)) {
		psy_ui_component_hide(stepsequencerview_base(&self->stepsequencerview));
	}
}

void mainframe_initseqeditor(MainFrame* self)
{
	seqeditor_init(&self->seqeditor, &self->client, &self->workspace);
	psy_ui_component_setalign(seqeditor_base(&self->seqeditor),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(seqeditor_base(&self->seqeditor));
	psy_ui_splitbar_init(&self->splitseqeditor, &self->client);
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitseqeditor),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(psy_ui_splitbar_base(&self->splitseqeditor));
}

void mainframe_initrecentview(MainFrame* self)
{
	// recent song view
	recentview_init(&self->recentview, mainframe_base(self),
		psy_ui_notebook_base(&self->viewtabbars),
		&self->workspace);
	psy_ui_component_setalign(recentview_base(&self->recentview),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(recentview_base(&self->recentview));
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
	sequenceview_init(&self->sequenceview, mainframe_base(self), &self->workspace);
	psy_ui_component_setalign(sequenceview_base(&self->sequenceview),
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(
		&self->sequenceview.options.toggleseqediticon.signal_clicked, self,
		mainframe_ontoggleseqeditor);
	psy_signal_connect(
		&self->sequenceview.options.toggleseqedit.signal_clicked, self,
		mainframe_ontoggleseqeditor);
	psy_ui_splitbar_init(&self->splitbar, mainframe_base(self));
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
	psy_signal_emit(&self->workspace.signal_configchanged,
		&self->workspace, 1, &self->workspace.config);
	workspace_addhistory(&self->workspace);
	psy_signal_connect(&self->workspace.player.eventdrivers.signal_input, self,
		mainframe_oneventdriverinput);
	psy_signal_connect(&self->workspace.player.signal_numsongtrackschanged,
		self, mainframe_onsongtrackschanged);
	psy_signal_connect(&self->workspace.signal_changecontrolskin, self,
		mainframe_onchangecontrolskin);
	psy_signal_connect(&self->workspace.signal_dockview, self,
		mainframe_ondockview);
	psy_signal_connect(&self->workspace.signal_togglegear, self,
		mainframe_ontogglegearworkspace);
	psy_signal_connect(&self->workspace.signal_languagechanged, self,
		mainframe_onlanguagechanged);
	psy_signal_connect(&self->checkunsavedbox.signal_execute, self,
		mainframe_oncheckunsaved);
	psy_signal_connect(&self->workspace.signal_skinchanged, self,
		mainframe_onskinchanged);
	psy_signal_connect(&self->workspace.signal_selectpatterndisplay, self,
		mainframe_onselectpatterndisplay);
	psy_audio_eventdrivers_setcallback(&self->workspace.player.eventdrivers,
		mainframe_eventdrivercallback, self);
	psy_ui_component_starttimer(mainframe_base(self), 0, 50);
}

void mainframe_initinterpreter(MainFrame* self)
{
	interpreter_init(&self->interpreter, &self->workspace);
	interpreter_start(&self->interpreter);
}

void mainframe_setstartpage(MainFrame* self)
{		
	if (workspace_showaboutatstart(&self->workspace)) {
		workspace_selectview(&self->workspace, TABPAGE_HELPVIEW, 1, 0);
	} else {
		workspace_selectview(&self->workspace, TABPAGE_MACHINEVIEW, 0, 0);
	}
	self->startpage = TRUE;
}

void mainframe_oneventdriverinput(MainFrame* self, psy_EventDriver* sender)
{
	psy_EventDriverCmd cmd;

	cmd = psy_eventdriver_getcmd(sender, "general");
	switch (cmd.id) {
		case CMD_IMM_HELP:
			tabbar_select(&self->helpview.tabbar, 0);
			tabbar_select(&self->tabbar, TABPAGE_HELPVIEW);
			break;
		case CMD_IMM_HELPSHORTCUT:
			mainframe_ontogglekbdhelp(self, mainframe_base(self));
			break;
		case CMD_IMM_EDITMACHINE:
			tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
			break;
		case CMD_IMM_EDITPATTERN:
			tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
			break;
		case CMD_IMM_ADDMACHINE:
			self->machineview.newmachine.pluginsview.calledby =
				self->tabbar.selected;
			workspace_selectview(&self->workspace, TABPAGE_MACHINEVIEW,
				1, 0);
			break;
		case CMD_IMM_PLAYSONG:
			psy_audio_player_setposition(&self->workspace.player, 0);
			psy_audio_player_start(&self->workspace.player);
			break;
		case CMD_IMM_PLAYSTART: {
			psy_audio_SequenceEntry* entry;

			entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
			psy_audio_player_setposition(&self->workspace.player,
				(entry) ? entry->offset : 0);
			psy_audio_player_start(&self->workspace.player);
			break; }
		case CMD_IMM_PLAYFROMPOS: {
			psy_dsp_big_beat_t playposition = 0;
			psy_audio_SequenceEntry* entry;

			entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
			playposition = (entry ? entry->offset : 0) +
				(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
			psy_audio_player_setposition(&self->workspace.player, playposition);
			psy_audio_player_start(&self->workspace.player);
			break; }
		case CMD_IMM_PLAYSTOP:
			psy_audio_player_stop(&self->workspace.player);
			break;
		case CMD_IMM_SONGPOSDEC:
			workspace_songposdec(&self->workspace);			
			break;
		case CMD_IMM_SONGPOSINC:
			workspace_songposinc(&self->workspace);
			break;
		case CMD_IMM_MAXPATTERN:
			mainframe_maximizeorminimizeview(self);
			break;
		case CMD_IMM_INFOMACHINE:
			workspace_showparameters(&self->workspace,
				psy_audio_machines_slot(&self->workspace.song->machines));
			break;
		case CMD_IMM_EDITINSTR:
			workspace_selectview(&self->workspace, TABPAGE_INSTRUMENTSVIEW, 0,
				0);
			break;
		case CMD_IMM_EDITSAMPLE:
			workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW, 0, 0);
			tabbar_select(&self->samplesview.clienttabbar, 0);
			break;
		case CMD_IMM_EDITWAVE:
			workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW, 0, 0);
			tabbar_select(&self->samplesview.clienttabbar, 2);
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
		psy_ui_component_resize(&self->sequenceview.component,
			psy_ui_size_make(
			self->workspace.maximizeview.sequenceviewrestorewidth,
			psy_ui_value_makepx(0)));
	} else {
		psy_ui_Size sequenceviewsize;		

		self->workspace.maximizeview.maximized = TRUE;
		self->workspace.maximizeview.row0 = self->toprow0.visible;
		self->workspace.maximizeview.row1 = self->toprow1.visible;
		self->workspace.maximizeview.row2 = self->toprow2.visible;
		self->workspace.maximizeview.trackscopes =
			self->trackscopeview.component.visible;
		sequenceviewsize = psy_ui_component_size(
			&self->sequenceview.component);
		self->workspace.maximizeview.sequenceviewrestorewidth =
			sequenceviewsize.width;
		psy_ui_component_hide(&self->toprow0);
		psy_ui_component_hide(&self->toprow1);
		psy_ui_component_hide(&self->trackscopeview.component);
		self->sequenceview.component.preventpreferredsizeatalign = TRUE;
		psy_ui_component_resize(&self->sequenceview.component,
			psy_ui_size_zero());
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
	if (progress == 0) {
		psy_ui_progressbar_setprogress(&self->progressbar, 0);
	} else {
		psy_ui_progressbar_tick(&self->progressbar);
	}
}

void mainframe_onsongchanged(MainFrame* self, Workspace* sender, int flag,
	psy_audio_SongFile* songfile)
{		
	if (flag == WORKSPACE_LOADSONG) {
		if (workspace_showsonginfoonload(sender)) {
			tabbar_select(&self->tabbar, TABPAGE_PROPERTIESVIEW);
		}		
	}	
	mainframe_updatesongtitle(self);	
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
	}
	psy_path_dispose(&path);
}

void mainframe_ontogglegear(MainFrame* self, psy_ui_Component* sender)
{
	if (!psy_ui_component_visible(&self->gear.component)) {
		psy_ui_button_highlight(&self->machinebar.gear);
	}
	psy_ui_component_togglevisibility(&self->gear.component);
}

void mainframe_ontogglegearworkspace(MainFrame* self, Workspace* sender)
{
	mainframe_ontogglegear(self, NULL);
}

void mainframe_onhidegear(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.gear);
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
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
}

void mainframe_ongearcreate(MainFrame* self, psy_ui_Component* sender)
{
	self->machineview.newmachine.pluginsview.calledby = GEARVIEW;
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	tabbar_select(&self->machineview.tabbar, 1);
}

void mainframe_onsettingsviewchanged(MainFrame* self, PropertiesView* sender,
	psy_Property* property)
{
	switch (psy_property_id(property)) {
		case PROPERTY_ID_SHOWSTEPSEQUENCER:
			if (psy_property_item_bool(property) != psy_ui_component_visible(
					stepsequencerview_base(&self->stepsequencerview))) {
				psy_ui_component_togglevisibility(
					stepsequencerview_base(&self->stepsequencerview));
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
				trackscopeview_start(&self->trackscopeview);
			} else {
				trackscopeview_stop(&self->trackscopeview);
			}
			break;
		default:
			workspace_configurationchanged(&self->workspace, property);
			break;
	}
}

void mainframe_onrender(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_notebook_setpageindex(&self->notebook, TABPAGE_RENDERVIEW);
}

void mainframe_ontimer(MainFrame* self, uintptr_t timerid)
{
	if (self->startup && psy_ui_component_visible(mainframe_base(self))) {		
		mainframe_onstartup(self);
		self->startup = FALSE;
	}
	workspace_idle(&self->workspace);
}

// event is called when mainframe has its final size and is visible on screen
void mainframe_onstartup(MainFrame* self)
{
	// The pattern display type is only now set, because the host needs to know
	// the view size to divide a splitted display correctly
	workspace_selectpatterndisplay(&self->workspace,
		workspace_patterndisplaytype(&self->workspace));
}

void mainframe_onviewselected(MainFrame* self, Workspace* sender, int index,
	uintptr_t section, int option)
{	
	if (index != GEARVIEW) {
		psy_ui_Component* view;

		if (index == TABPAGE_CHECKUNSAVED) {
			if (option == CHECKUNSAVE_CLOSE) {
				self->checkunsavedbox.mode = option;
				checkunsavedbox_setlabels(&self->checkunsavedbox,
					"Exit Psycle Request, but your Song is not saved!",
					"Save and Exit",
					"Exit (no save)");
			} else if (option == CHECKUNSAVE_NEW) {				
				self->checkunsavedbox.mode = option;
				checkunsavedbox_setlabels(&self->checkunsavedbox,
					"New Song Request, but your Song is not saved!",
					"Save and Create New Song",
					"Create New Song (no save)");
			} else if (option == CHECKUNSAVE_LOAD) {
				self->checkunsavedbox.mode = option;
				checkunsavedbox_setlabels(&self->checkunsavedbox,
					"Song Load Request, but your Song is not saved!",
					"Save and Load Song",
					"Load Song (no save)");
			}
		}
		tabbar_select(&self->tabbar, index);
		view = psy_ui_notebook_activepage(&self->notebook);
		if (view) {
			psy_ui_component_setfocus(view);
			psy_ui_component_selectsection(view, section);
		}
		if (index == TABPAGE_MACHINEVIEW && section == 1) {
			if (option == 20) {
				self->machineview.wireview.randominsert = 0;
				self->machineview.wireview.addeffect = 1;
			}
		}
	}
}

void mainframe_ontabbarchanged(MainFrame* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	psy_ui_Component* component;

	if (self->startpage) {
		tabbar_select(&self->helpview.tabbar, 0);
		self->startpage = 0;
	}
	workspace_onviewchanged(&self->workspace, tabindex);				
	psy_ui_notebook_setpageindex(&self->viewstatusbars, tabindex);	
	psy_ui_notebook_setpageindex(&self->viewtabbars, tabindex);
	component = psy_ui_notebook_activepage(&self->notebook);
	if (component) {
		psy_ui_component_setfocus(component);
	}
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

void mainframe_updateterminalbutton(MainFrame* self)
{
	psy_ui_button_settextcolour(&self->toggleterminal,
		self->terminalbutton_colours[self->terminalmsgtype]);
}

void mainframe_onzoomboxchanged(MainFrame* self, ZoomBox* sender)
{	
	workspace_zoom(&self->workspace, zoombox_rate(sender));
}

void mainframe_onsongtrackschanged(MainFrame* self, psy_audio_Player* sender,
	uintptr_t numsongtracks)
{
	// TrackScopeView can change its height, realign mainframe
	psy_ui_component_align(mainframe_base(self));
}

void mainframe_onchangecontrolskin(MainFrame* self, Workspace* sender,
	const char* path)
{
	paramview_changecontrolskin(path);
}

void mainframe_ondockview(MainFrame* self, Workspace* sender,
	psy_ui_Component* view)
{	
	psy_ui_component_resize(view, psy_ui_size_zero());
	psy_ui_component_setparent(view, &self->paramviews);	
	psy_ui_component_setalign(view, psy_ui_ALIGN_LEFT);	
	psy_ui_component_align(&self->client);	
	psy_ui_component_align(&self->paramviews);
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
void mainframe_oncheckunsaved(MainFrame* self, CheckUnsavedBox* sender,
	int option, int mode)
{	
	bool load;

	load = FALSE;
	switch (option) {			
		case CHECKUNSAVE_SAVE: {
			psy_ui_SaveDialog dialog;

			psy_ui_savedialog_init_all(&dialog, 0, "Save Song",
				psy_audio_songfile_savefilter(), "PSY",
				workspace_songs_directory(&self->workspace));
			if (psy_ui_savedialog_execute(&dialog)) {
				workspace_savesong(&self->workspace,
					psy_ui_savedialog_filename(&dialog));
				if (mode == CHECKUNSAVE_CLOSE) {
					psy_ui_app_close(&app);
				} else if (mode == CHECKUNSAVE_LOAD) {
					load = TRUE;
				} else if (mode == CHECKUNSAVE_NEW) {										
					workspace_newsong(&self->workspace);					
				}
			}
			psy_ui_savedialog_dispose(&dialog);
			break; }
		case CHECKUNSAVE_NOSAVE: {
			extern psy_ui_App app;

			self->workspace.undosavepoint = psy_list_size(
				self->workspace.undoredo.undo);
			self->workspace.machines_undosavepoint = psy_list_size(
				self->workspace.song->machines.undoredo.undo);
			if (mode == CHECKUNSAVE_CLOSE) {
				psy_ui_app_close(&app);
			} else if (mode == CHECKUNSAVE_LOAD) {
				load = TRUE;
			} else if (mode == CHECKUNSAVE_NEW) {
				workspace_newsong(&self->workspace);
			}
			break; }
		case CHECKUNSAVE_CONTINUE:
			workspace_updatecurrview(&self->workspace);
			break;
		default:		
			break;
	}
	if (load) {
		psy_ui_OpenDialog dialog;		

		psy_ui_opendialog_init_all(&dialog, 0, "Load Song",
			psy_audio_songfile_loadfilter(), "PSY",
			workspace_songs_directory(&self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			workspace_loadsong(&self->workspace,
				psy_ui_opendialog_filename(&dialog),
				workspace_playsongafterload(&self->workspace));
		}
		psy_ui_opendialog_dispose(&dialog);		
	}
}

bool mainframe_onclose(MainFrame* self)
{
	if (workspace_songmodified(&self->workspace)) {
		workspace_selectview(&self->workspace, TABPAGE_CHECKUNSAVED, 0,
			CHECKUNSAVE_CLOSE);
		return FALSE;
	}
	return TRUE;
}

void mainframe_ontoggleseqeditor(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(seqeditor_base(&self->seqeditor))) {
		psy_ui_button_seticon(&self->sequenceview.options.toggleseqediticon,
			psy_ui_ICON_MORE);
		psy_ui_button_settext(&self->sequenceview.options.toggleseqedit,
			"Show SequenceEditor");		
		psy_ui_component_hide(&self->splitseqeditor.component);
		psy_ui_button_disablehighlight(
			&self->sequenceview.options.toggleseqedit);
	} else {
		psy_ui_component_show(&self->splitseqeditor.component);		
		psy_ui_button_seticon(&self->sequenceview.options.toggleseqediticon,
			psy_ui_ICON_LESS);
		psy_ui_button_settext(&self->sequenceview.options.toggleseqedit,
			"Hide SequenceEditor");
		psy_ui_button_highlight(&self->sequenceview.options.toggleseqedit);
	}
	psy_ui_component_togglevisibility(seqeditor_base(&self->seqeditor));
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
				psy_ui_value_makeeh(10)));		
	}
	psy_ui_component_align(mainframe_base(self));
}

void mainframe_ontogglekbdhelp(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_component_togglevisibility(kbdhelp_base(&self->kbdhelp));	
}

void mainframe_onselectpatterndisplay(MainFrame* self, psy_ui_Component* sender,
	PatternDisplayType display)
{	
	patternview_selectdisplay(&self->patternview,
		workspace_patterndisplaytype(&self->workspace));
}


#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
void mainframe_onfileload(MainFrame* self, FileView* sender)
{
	const char* path;

	path = fileview_path(sender);
	workspace_loadsong(&self->workspace, path,
		workspace_playsongafterload(&self->workspace));
}
#endif

void mainframe_onskinchanged(MainFrame* self, Workspace* sender)
{
	// Nothing todo here. The psycle skin sheme only configures the Pattern,
	// Machine and Parameterview that are set there.
}

void mainframe_onkeydown(MainFrame* self, psy_ui_KeyEvent* ev)
{	
	mainframe_delegatekeyboard(self, psy_EVENTDRIVER_KEYDOWN, ev);
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
			psy_audio_encodeinput(ev->keycode, ev->shift, ev->ctrl),
			0));
}

// eventdriver callback to handle chordmode, patternedit noterelease
int mainframe_eventdrivercallback(MainFrame* self, int msg, int param1,
	int param2)
{
	switch (msg) {
		case PSY_EVENTDRIVER_PATTERNEDIT:
			return psy_ui_component_hasfocus(
				&self->patternview.trackerview.grid.component);
			break;
		case PSY_EVENTDRIVER_NOTECOLUMN:
			return self->patternview.trackerview.grid.cursor.column == 0;
			break;
		case PSY_EVENTDRIVER_SETCHORDMODE:
			if (param1 == 1) {
				self->patternview.trackerview.grid.chordbegin =
					self->patternview.trackerview.grid.cursor.track;
				self->patternview.trackerview.grid.chordmode = TRUE;
			}
			break;
		case PSY_EVENTDRIVER_INSERTNOTEOFF:
			trackergrid_inputnote(&self->patternview.trackerview.grid,
				psy_audio_NOTECOMMANDS_RELEASE,
				self->patternview.trackerview.grid.chordmode);
			break;
	}
	return 0;
}
