// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "mainframe.h"
#include "cmdsgeneral.h"
#include "settingsview.h"
#include "resources/resource.h"
#include "paramview.h"
#include <uiopendialog.h>
#include <uisavedialog.h>

#include <dir.h>
#include <uiapp.h>

#include <songio.h>

#include "../../detail/portable.h"

#define TERMINALMESSAGECOLOR 0x0000FF00
#define TERMINALWARNINGCOLOR 0x0000FFFF
#define TERMINALERRORCOLOR 0x000000FF

static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_updatetext(MainFrame*);
static void mainframe_updatetheme(MainFrame*);
static void mainframe_onskinchanged(MainFrame*, Workspace*);
static void mainframe_setstatusbartext(MainFrame*, const char* text);
static const char* mainframe_statusbaridletext(MainFrame*);
static void mainframe_destroyed(MainFrame*, psy_ui_Component* sender);
static void mainframe_onkeydown(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_onkeyup(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_onmousedown(MainFrame*, psy_ui_MouseEvent*);
static void mainframe_onsequenceselchange(MainFrame* , psy_audio_SequenceEntry*);
static void mainframe_ongear(MainFrame*, psy_ui_Component* sender);
static void mainframe_onhidegear(MainFrame*, psy_ui_Component* sender);
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
static void mainframe_onshowgear(MainFrame*, Workspace* sender);
static void mainframe_updatetitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, uintptr_t timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, psy_EventDriver* sender);
static void mainframe_ontoggleseqeditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontoggleterminal(MainFrame*, psy_ui_Component* sender);
static void mainframe_ontogglekbdhelp(MainFrame*, psy_ui_Component* sender);

static void mainframe_onterminaloutput(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalwarning(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalerror(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onzoomboxchanged(MainFrame*, ZoomBox* sender);
static void mainframe_onsongtrackschanged(MainFrame*, psy_audio_Player* sender,
	uintptr_t numsongtracks);
static void mainframe_onchangecontrolskin(MainFrame*, Workspace* sender,
	const char* path);
static void mainframe_ondockview(MainFrame*, Workspace* sender,
	psy_ui_Component* view);
static void mainframe_onlanguagechanged(MainFrame*, Translator* sender);
static bool mainframe_onclose(MainFrame*);
static void mainframe_oncheckunsaved(MainFrame*, CheckUnsavedBox* sender, int option, int mode);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
static void mainframe_onfileload(MainFrame*, FileView* sender);
#endif

#define GEARVIEW 10

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainFrame* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onkeydown = (psy_ui_fp_onkeydown)mainframe_onkeydown;
		vtable.onkeyup = (psy_ui_fp_onkeyup)mainframe_onkeyup;
		vtable.onmousedown = (psy_ui_fp_onmousedown)mainframe_onmousedown;
		vtable.ontimer = (psy_ui_fp_ontimer)mainframe_ontimer;
		vtable.onclose = (psy_ui_fp_onclose)mainframe_onclose;
		vtable_initialized = TRUE;
	}
}

void mainframe_init(MainFrame* self)
{			
	psy_ui_Margin tabbardividemargin;
	
	psy_ui_margin_init_all(&tabbardividemargin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(4.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_frame_init(&self->component, 0);
	vtable_init(self);
	self->component.vtable = &vtable;	
	self->startpage = FALSE;
	psy_ui_component_seticonressource(&self->component, IDI_PSYCLEICON);		
	workspace_init(&self->workspace, &self->component);	
	workspace_load_configuration(&self->workspace);
	workspace_load_recentsongs(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}	
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
	self->terminalhasmessage = FALSE;
	self->terminalhaswarning = FALSE;
	self->terminalhaserror = FALSE;
	// create empty status bar
	psy_ui_component_init(&self->statusbar, &self->component);
	psy_ui_component_setalign(&self->statusbar, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_enablealign(&self->statusbar);	
	psy_ui_terminal_init(&self->terminal, &self->component);
	psy_signal_connect(&self->workspace.signal_terminal_warning, self,
		mainframe_onterminalwarning);
	psy_signal_connect(&self->workspace.signal_terminal_out, self,
		mainframe_onterminaloutput);
	psy_signal_connect(&self->workspace.signal_terminal_error, self,
		mainframe_onterminalerror);
	psy_ui_component_setalign(&self->terminal.component, psy_ui_ALIGN_BOTTOM);	
	psy_ui_component_resize(&self->terminal.component,
		psy_ui_size_make(
		psy_ui_value_makepx(0), psy_ui_value_makepx(0)));	
	psy_ui_splitbar_init(&self->splitbarterminal, &self->component);
	psy_ui_component_setalign(&self->splitbarterminal.component,
		psy_ui_ALIGN_BOTTOM);
	kbdhelp_init(&self->kbdhelp, &self->component, &self->workspace);
	psy_ui_component_setalign(kbdhelp_base(&self->kbdhelp),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
	psy_signal_connect(&self->component.signal_destroyed, self,
		mainframe_destroyed);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_enablealign(&self->top);	
	// client
	psy_ui_component_init(&self->client, &self->component);	
	psy_ui_component_setbackgroundmode(&self->client,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_enablealign(&self->client);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);		
	psy_ui_component_init(&self->paramviews, &self->client);
	psy_ui_component_enablealign(&self->paramviews);
	psy_ui_component_setalign(&self->paramviews, psy_ui_ALIGN_BOTTOM);
	// tabbars			
	{
		psy_ui_Margin spacing;

		psy_ui_margin_init_all(&spacing, psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
			psy_ui_value_makepx(0));
		psy_ui_component_init(&self->tabbars, &self->client);		
		psy_ui_component_enablealign(&self->tabbars);
		psy_ui_component_setspacing(&self->tabbars, &spacing);
		mainframe_initbars(self);
		psy_signal_connect(&self->filebar.recentbutton.signal_clicked, self,
			mainframe_onrecentsongs);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
		psy_signal_connect(&self->filebar.loadbutton.signal_clicked, self,
			mainframe_onfileloadview);
#endif
	}
	// tabbars
	psy_ui_component_setalign(&self->tabbars, psy_ui_ALIGN_TOP);
	psy_ui_component_enablealign(&self->tabbars);
	navigation_init(&self->navigation, &self->tabbars, &self->workspace);
	self->navigation.component.margin.left = psy_ui_value_makeew(2.0);
	self->navigation.component.margin.right = psy_ui_value_makeew(2.0);
	psy_ui_component_setalign(&self->navigation.component, psy_ui_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->tabbars);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	psy_ui_component_setalignexpand(tabbar_base(&self->tabbar),	
		psy_ui_HORIZONTALEXPAND);	
	tabbar_append_tabs(&self->tabbar, 
		"", // Machines
		"", // Patterns
		"", // Samples
		"", // Instruments
		"", // Properties
		NULL);
	tabbar_append(&self->tabbar,
		"" /* Settings */)->margin.left =
		psy_ui_value_makeew(4.0);
	tabbar_append(&self->tabbar,
		"" /* Help */)->margin.right =
		psy_ui_value_makeew(4.0);
	psy_ui_notebook_init(&self->viewtabbars, &self->tabbars);
	psy_ui_component_enablealign(&self->viewtabbars.component);
	psy_ui_component_setalign(&self->viewtabbars.component, psy_ui_ALIGN_LEFT);	
	/// init notebook views
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
	// gear
	gear_init(&self->gear, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->gear.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->gear.component);
	psy_signal_connect(&self->machinebar.gear.signal_clicked, self,
		mainframe_ongear);
	psy_signal_connect(&self->gear.component.signal_hide, self,
		mainframe_onhidegear);
	// cpuview	
	cpuview_init(&self->cpuview, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->cpuview.component, psy_ui_ALIGN_RIGHT);				
	psy_ui_component_hide(&self->cpuview.component);
	psy_signal_connect(&self->machinebar.cpu.signal_clicked, self,
		mainframe_oncpu);
	psy_signal_connect(&self->cpuview.component.signal_hide, self,
		mainframe_onhidecpu);
	// midiview
	midiview_init(&self->midiview, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->midiview.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->midiview.component);	
	psy_signal_connect(&self->machinebar.midi.signal_clicked, self,
		mainframe_onmidi);	
	psy_signal_connect(&self->gear.buttons.createreplace.signal_clicked, self,
		mainframe_ongearcreate);
	psy_signal_emit(&self->workspace.signal_configchanged,
		&self->workspace, 1, &self->workspace.config);
	psy_signal_connect(&self->tabbar.signal_change, self,
		mainframe_ontabbarchanged);
	workspace_addhistory(&self->workspace);	
	psy_signal_connect(&self->workspace.player.eventdrivers.signal_input, self,
		mainframe_oneventdriverinput);	
	// stepsequencerview
	stepsequencerview_init(&self->stepsequencerview, &self->client,
		&self->workspace);
	psy_ui_component_setalign(&self->stepsequencerview.component,
		psy_ui_ALIGN_BOTTOM);
	if (!workspace_showstepsequencer(&self->workspace)) {
		psy_ui_component_hide(&self->stepsequencerview.component);
	}
	// seqeditor
	seqeditor_init(&self->seqeditor, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->seqeditor.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->seqeditor.component);
	// recent song view
	recentview_init(&self->recentview, &self->component,
		&self->viewtabbars.component,
		&self->workspace);
	psy_ui_component_setalign(&self->recentview.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->recentview.component);
#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
	// file load view
	fileview_init(&self->fileloadview, &self->component,
		&self->workspace);
	psy_ui_component_setalign(&self->fileloadview.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->fileloadview.component);
	psy_signal_connect(&self->fileloadview.signal_selected,
		self, mainframe_onfileload);
#endif
	// sequenceview
	sequenceview_init(&self->sequenceview, &self->component, &self->workspace);
	psy_ui_component_setalign(&self->sequenceview.component,
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->sequenceview.options.toggleseqediticon.signal_clicked, self,
		mainframe_ontoggleseqeditor);
	psy_signal_connect(&self->sequenceview.options.toggleseqedit.signal_clicked, self,
		mainframe_ontoggleseqeditor);
	// splitbar
	psy_ui_splitbar_init(&self->splitbar, &self->component);
	// plugineditor
	plugineditor_init(&self->plugineditor, &self->component, &self->workspace);
	psy_ui_component_setalign(&self->plugineditor.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->plugineditor.component);
	psy_signal_connect(&self->machinebar.editor.signal_clicked, self,
		mainframe_onplugineditor);
	psy_ui_splitbar_init(&self->splitbarplugineditor, &self->component);
	psy_ui_component_setalign(&self->splitbarplugineditor.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->splitbarplugineditor.component);
	// create statusbar components
	mainframe_initstatusbar(self);
	psy_ui_component_starttimer(&self->component, 0, 50);
	if (self->workspace.song) {
		mainframe_setstatusbartext(self,
			self->workspace.song->properties.title);
	}
	psy_signal_connect(&self->workspace.player.signal_numsongtrackschanged,
		self, mainframe_onsongtrackschanged);	
	psy_signal_connect(&self->workspace.signal_changecontrolskin, self,
		mainframe_onchangecontrolskin);
	psy_signal_connect(&self->workspace.signal_dockview, self,
		mainframe_ondockview);
	psy_signal_connect(&self->workspace.signal_showgear, self,
		mainframe_onshowgear);
	mainframe_setstartpage(self);
	mainframe_updatetitle(self);
	mainframe_updatetext(self);
	psy_signal_connect(&self->workspace.signal_languagechanged, self,
		mainframe_onlanguagechanged);
	psy_signal_connect(&self->checkunsavedbox.signal_execute, self,
		mainframe_oncheckunsaved);
	interpreter_init(&self->interpreter, &self->workspace);
	interpreter_start(&self->interpreter);
	psy_signal_connect(&self->workspace.signal_skinchanged, self,
		mainframe_onskinchanged);	
	mainframe_updatetheme(self);
}

void mainframe_updatetheme(MainFrame* self)
{
	/*psy_ui_component_setbackgroundcolor(&self->settingsview.component,
		self->machineview.wireview.skin.colour, psy_ui_RECURSIVE);
	psy_ui_component_setcolor(&self->settingsview.component,
		self->patternview.skin.font, psy_ui_RECURSIVE);
	psy_ui_component_setbackgroundcolor(&self->helpview.component,
		self->machineview.wireview.skin.colour, psy_ui_RECURSIVE);
	psy_ui_component_setbackgroundcolor(&self->samplesview.component,
		self->patternview.skin.background, psy_ui_RECURSIVE);
	psy_ui_component_setbackgroundcolor(&self->instrumentsview.component,
		self->patternview.skin.background, psy_ui_RECURSIVE);
	psy_ui_component_setcolor(&self->instrumentsview.component,
		self->patternview.skin.font, psy_ui_RECURSIVE);
	psy_ui_component_setbackgroundcolor(&self->songpropertiesview.component,
		self->patternview.skin.background, psy_ui_RECURSIVE);*/
}

void mainframe_onskinchanged(MainFrame* self, Workspace* sender)
{
	mainframe_updatetheme(self);
}

void mainframe_updatetext(MainFrame* self)
{
	tabbar_rename_tabs(&self->tabbar,
		workspace_translate(&self->workspace, "main.machines"),
		workspace_translate(&self->workspace, "main.patterns"),
		workspace_translate(&self->workspace, "main.samples"),
		workspace_translate(&self->workspace, "main.instruments"),
		workspace_translate(&self->workspace, "main.properties"),
		workspace_translate(&self->workspace, "main.settings"),
		workspace_translate(&self->workspace, "help.help"),
		NULL);
	psy_ui_button_settext(&self->toggleterminal,
		workspace_translate(&self->workspace, "main.terminal"));
	psy_ui_button_settext(&self->togglekbdhelp,
		workspace_translate(&self->workspace, "main.kbd"));
	psy_ui_component_invalidate(&self->component);
}

void mainframe_setstatusbartext(MainFrame* self, const char* text)
{	
	psy_ui_label_settext(&self->statusbarlabel, text ? text : "");
}

const char* mainframe_statusbaridletext(MainFrame* self)
{	
	return self->workspace.song
		? self->workspace.song->properties.title
		: 0;	
}

void mainframe_initstatusbar(MainFrame* self)
{	
	psy_ui_Margin margin;
		
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.25),
		psy_ui_value_makeew(1.0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(1.0));	
	// zoom
	zoombox_init(&self->zoombox, &self->statusbar);
	psy_ui_component_setalign(&self->zoombox.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->zoombox.component, &margin);
	psy_signal_connect(&self->zoombox.signal_changed, self,
		mainframe_onzoomboxchanged);
	// statusbar label	
	psy_ui_label_init(&self->statusbarlabel, &self->statusbar);
	psy_ui_label_settext(&self->statusbarlabel, "Ready");
	psy_ui_label_setcharnumber(&self->statusbarlabel, 29);
	psy_ui_component_setmargin(&self->statusbarlabel.component, &margin);
	psy_ui_component_setalign(&self->statusbarlabel.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_notebook_init(&self->viewstatusbars, &self->statusbar);
	psy_ui_component_setmargin(&self->viewstatusbars.component, &margin);
	psy_ui_component_setalign(&self->viewstatusbars.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_enablealign(&self->viewstatusbars.component);		
	machineviewbar_init(&self->machineviewbar, &self->viewstatusbars.component,
		&self->workspace);
	psy_ui_component_setalign(&self->machineviewbar.component,
		psy_ui_ALIGN_LEFT);
	self->machineview.wireview.statusbar = &self->machineviewbar;
	patternviewbar_init(&self->patternbar, &self->viewstatusbars.component,
		&self->workspace);
	psy_ui_component_setalign(&self->patternbar.component, psy_ui_ALIGN_LEFT);
	sampleeditorbar_init(&self->samplesview.sampleeditor.sampleeditortbar, &self->viewstatusbars.component,
		&self->samplesview.sampleeditor,
		&self->workspace);	
	psy_ui_component_setalign(&self->samplesview.sampleeditor.sampleeditortbar.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_notebook_setpageindex(&self->viewstatusbars, 0);
//	psy_ui_notebook_connectcontroller(&self->viewstatusbars,
	//	&self->tabbar.signal_change);
	psy_ui_button_init(&self->togglekbdhelp, &self->statusbar);
	psy_ui_button_settext(&self->togglekbdhelp, "Kbd");
	psy_ui_component_setalign(&self->togglekbdhelp.component,
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->togglekbdhelp.signal_clicked, self,
		mainframe_ontogglekbdhelp);
	psy_ui_button_init(&self->toggleterminal, &self->statusbar);
	psy_ui_button_settext(&self->toggleterminal, "  Terminal");	
	psy_ui_component_setalign(&self->toggleterminal.component,
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->toggleterminal.signal_clicked, self,
		mainframe_ontoggleterminal);	
	psy_ui_progressbar_init(&self->progressbar, &self->statusbar);
	psy_ui_component_setalign(&self->progressbar.component,
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
	psy_ui_component_enablealign(&self->topright);
	psy_ui_component_setalign(&self->topright, psy_ui_ALIGN_RIGHT);
	vubar_init(&self->vubar, &self->topright, &self->workspace);	
	psy_ui_component_setalign(&self->vubar.component, psy_ui_ALIGN_TOP);
	// row0
	psy_ui_component_init(&self->toprow0, &self->top);
	psy_ui_component_enablealign(&self->toprow0);	
	psy_ui_component_setalign(&self->toprow0, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->toprow0, &row0margin);
	// row1
	psy_ui_component_init(&self->toprow1, &self->top);
	psy_ui_component_enablealign(&self->toprow1);	
	psy_ui_component_setalign(&self->toprow1, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->toprow1, &rowmargin);
	// row2
	psy_ui_component_init(&self->toprow2, &self->top);
	psy_ui_component_enablealign(&self->toprow2);	
	psy_ui_component_setalign(&self->toprow2, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->toprow2, &rowmargin);
	// scopebar
	trackscopeview_init(&self->trackscopeview, &self->top, &self->workspace);
	psy_ui_component_setalign(&self->trackscopeview.component,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->trackscopeview.component, &rowmargin);
	if (!workspace_showtrackscopes(&self->workspace)) {
		psy_ui_component_hide(&self->trackscopeview.component);
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
	psy_ui_component_setmargin(&self->playposbar.component, &margin);	
	// row1
	// Songbar	
	songbar_init(&self->songbar, &self->toprow1, &self->workspace);
	psy_ui_component_setalign(&self->songbar.component, psy_ui_ALIGN_LEFT);
	// row2
	// Machinebar
	machinebar_init(&self->machinebar, &self->toprow2, &self->workspace);	
	psy_ui_component_setalign(&self->machinebar.component, psy_ui_ALIGN_LEFT);	
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

void mainframe_destroyed(MainFrame* self, psy_ui_Component* component)
{
	extern psy_ui_App app;

	workspace_save_configuration(&self->workspace);
	workspace_dispose(&self->workspace);
	interpreter_dispose(&self->interpreter);
	psy_ui_app_stop(&app);
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
			mainframe_ontogglekbdhelp(self, &self->component);
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
			if (self->workspace.song) {
				if (self->workspace.sequenceselection.editposition.trackposition.sequencentrynode &&
					self->workspace.sequenceselection.editposition.trackposition.sequencentrynode->prev) {
					psy_audio_sequenceselection_seteditposition(
						&self->workspace.sequenceselection,
						psy_audio_sequence_makeposition(&self->workspace.song->sequence,
							self->workspace.sequenceselection.editposition.tracknode,
							self->workspace.sequenceselection.editposition.trackposition.sequencentrynode->prev));
					workspace_setsequenceselection(&self->workspace,
						self->workspace.sequenceselection);
				}
			}
			break;
		case CMD_IMM_SONGPOSINC:
			if (self->workspace.song) {
				if (self->workspace.sequenceselection.editposition.trackposition.sequencentrynode &&
					self->workspace.sequenceselection.editposition.trackposition.sequencentrynode->next) {
					psy_audio_sequenceselection_seteditposition(
						&self->workspace.sequenceselection,
						psy_audio_sequence_makeposition(&self->workspace.song->sequence,
							self->workspace.sequenceselection.editposition.tracknode,
							self->workspace.sequenceselection.editposition.trackposition.sequencentrynode->next));
					workspace_setsequenceselection(&self->workspace,
						self->workspace.sequenceselection);
				}
			}
			break;
		case CMD_IMM_MAXPATTERN:
			mainframe_maximizeorminimizeview(self);
			break;
		case CMD_IMM_INFOMACHINE:
			workspace_showparameters(&self->workspace,
				psy_audio_machines_slot(&self->workspace.song->machines));
			break;
		case CMD_IMM_EDITINSTR:
			workspace_selectview(&self->workspace, TABPAGE_INSTRUMENTSVIEW, 0, 0);
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
			mainframe_ontoggleterminal(self, &self->component);
			break;
		case CMD_IMM_INSTRDEC:
			if (self->workspace.song) {
				psy_audio_InstrumentIndex index;

				index = psy_audio_instruments_selected(&self->workspace.song->instruments);
				if (index.subslot > 0) {
					--index.subslot;
					psy_audio_instruments_select(&self->workspace.song->instruments,
						index);
				}
			}
			break;
		case CMD_IMM_INSTRINC:
			if (self->workspace.song) {
				psy_audio_InstrumentIndex index;

				index = psy_audio_instruments_selected(&self->workspace.song->instruments);
					++index.subslot;
				psy_audio_instruments_select(&self->workspace.song->instruments,
					index);
			}
			break;
		default:
			break;
	}
}

void mainframe_onkeydown(MainFrame* self, psy_ui_KeyEvent* ev)
{	
	if (ev->keycode != psy_ui_KEY_CONTROL && ev->keycode != psy_ui_KEY_SHIFT) {
		psy_EventDriver* kbd;
		psy_EventDriverData input;			
		
		input.message = EVENTDRIVER_KEYDOWN;
		kbd = workspace_kbddriver(&self->workspace);		
		input.param1 = psy_audio_encodeinput(ev->keycode, 
			self->patternview.trackerview.grid.chordmode ? 0 : ev->shift, ev->ctrl);		
		input.param2 = workspace_octave(&self->workspace) * 12;
		psy_eventdriver_write(kbd, input);
	}
}

void mainframe_onkeyup(MainFrame* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode != psy_ui_KEY_CONTROL && ev->keycode != psy_ui_KEY_SHIFT) {
		psy_EventDriver* kbd;
		psy_EventDriverData input;			
		
		input.message = EVENTDRIVER_KEYUP;
        input.param1 = psy_audio_encodeinput(ev->keycode, ev->shift, ev->ctrl);
		input.param2 = 48;
		kbd = workspace_kbddriver(&self->workspace);
		psy_eventdriver_write(kbd, input);
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
		self->workspace.maximizeview.trackscopes = self->trackscopeview.component.visible;
		sequenceviewsize = psy_ui_component_size(&self->sequenceview.component);
		self->workspace.maximizeview.sequenceviewrestorewidth =
			sequenceviewsize.width;
		psy_ui_component_hide(&self->toprow0);
		psy_ui_component_hide(&self->toprow1);
		psy_ui_component_hide(&self->trackscopeview.component);
		self->sequenceview.component.preventpreferredsizeatalign = TRUE;
		psy_ui_component_resize(&self->sequenceview.component,
			psy_ui_size_zero());
	}
	psy_ui_component_align(&self->component);
}

void mainframe_onsongloadprogress(MainFrame* self, Workspace* workspace, int progress)
{	
	if (progress == -1) {
		//psy_ui_terminal_clear(&self->terminal);
		psy_ui_terminal_output(&self->terminal, "\n");
	}
	psy_ui_progressbar_setprogress(&self->progressbar, progress / 100.f);
}

void mainframe_onpluginscanprogress(MainFrame* self, Workspace* workspace, int progress)
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
	mainframe_updatetitle(self);
	psy_ui_component_invalidate(&self->component);	
}

void mainframe_updatetitle(MainFrame* self)
{	
	char txt[512];
	char prefix[4096];
	char name[4096];
	char ext[4096];

	psy_dir_extract_path(self->workspace.filename, prefix, name, ext);
	psy_snprintf(txt, 512, "[%s.%s]  Psycle Modular Music Creation Studio ",
		name, ext);			
	psy_ui_component_settitle(&self->component, txt);
	if (workspace_song(&self->workspace)) {
		mainframe_setstatusbartext(self, psy_audio_song_title(
			workspace_song(&self->workspace)));
	}
}

void mainframe_onshowgear(MainFrame* self, Workspace* sender)
{
	if (!psy_ui_component_visible(&self->gear.component)) {
		self->gear.component.visible = 1;
		psy_ui_button_highlight(&self->machinebar.gear);		
		psy_ui_component_align(&self->client);
		psy_ui_component_show(&self->gear.component);
	}
}

void mainframe_ongear(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->gear.component)) {		
		psy_ui_component_hide(&self->gear.component);
		psy_ui_component_align(&self->client);
	} else {						
		psy_ui_button_highlight(&self->machinebar.gear);
		self->gear.component.visible = 1;		
		psy_ui_component_align(&self->client);
		psy_ui_component_show(&self->gear.component);
	}	
}

void mainframe_onhidegear(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.gear);
}

void mainframe_oncpu(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->cpuview.component)) {		
		psy_ui_component_hide(&self->cpuview.component);
		psy_ui_component_align(&self->client);
	}
	else {
		psy_ui_button_highlight(&self->machinebar.cpu);
		psy_ui_component_show(&self->cpuview.component);
		psy_ui_component_align(&self->client);
	}
}

void mainframe_onhidecpu(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_button_disablehighlight(&self->machinebar.cpu);
}

void mainframe_onmidi(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->midiview.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.midi);
		psy_ui_component_hide(&self->midiview.component);
		psy_ui_component_align(&self->client);
	}
	else {
		psy_ui_button_highlight(&self->machinebar.midi);
		psy_ui_component_show(&self->midiview.component);
		psy_ui_component_align(&self->client);
	}
}

void mainframe_onrecentsongs(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->recentview.component)) {
		psy_ui_button_seticon(&self->filebar.recentbutton, psy_ui_ICON_MORE);
		psy_ui_component_hide(&self->recentview.component);
		psy_ui_component_align(&self->component);
	}
	else {
		psy_ui_button_seticon(&self->filebar.recentbutton, psy_ui_ICON_LESS);
		psy_ui_component_show(&self->recentview.component);
		psy_ui_component_align(&self->component);
	}
}

#ifndef PSYCLE_USE_PLATFORM_FILEOPEN
void mainframe_onfileloadview(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->fileloadview.component)) {
		//psy_ui_button_seticon(&self->filebar.loadbutton, psy_ui_ICON_MORE);
		psy_ui_component_hide(&self->fileloadview.component);
		psy_ui_component_align(&self->component);
	} else {
		//psy_ui_button_seticon(&self->filebar.recentbutton, psy_ui_ICON_LESS);
		psy_ui_component_show(&self->fileloadview.component);
		psy_ui_component_align(&self->component);
	}
}
#endif

void mainframe_onplugineditor(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->plugineditor.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.editor);
		psy_ui_component_hide(&self->plugineditor.component);
		psy_ui_component_hide(&self->splitbarplugineditor.component);
		psy_ui_component_align(&self->component);		
	} else {						
		psy_ui_button_highlight(&self->machinebar.editor);
		self->plugineditor.component.visible = 1;
		self->splitbarplugineditor.component.visible = 1;
		psy_ui_component_align(&self->component);
		psy_ui_component_show(&self->splitbarplugineditor.component);
		psy_ui_component_show(&self->plugineditor.component);
	}	
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
	printf("mainframe_onsettingsviewchanged\n");
	if (strcmp(psy_property_key(property), "showstepsequencer") == 0) {
		if (workspace_showstepsequencer(&self->workspace)) {
			psy_ui_component_show(&self->stepsequencerview.component);
		} else {
			psy_ui_component_hide(&self->stepsequencerview.component);
		}
		psy_ui_component_align(&self->client);
	}
	if (strcmp(psy_property_key(property), "trackscopes") == 0) {
		if (workspace_showtrackscopes(&self->workspace)) {
			psy_ui_component_show(&self->trackscopeview.component);
			psy_ui_component_align(&self->component);
			trackscopeview_start(&self->trackscopeview);
		} else {
			psy_ui_component_hide(&self->trackscopeview.component);
			psy_ui_component_align(&self->component);
			trackscopeview_stop(&self->trackscopeview);
		}
	} else {
		workspace_configchanged(&self->workspace, property);
	}
}

int mainframe_showmaximizedatstart(MainFrame* self)
{
	return workspace_showmaximizedatstart(&self->workspace);
}

void mainframe_onrender(MainFrame* self, psy_ui_Component* sender)
{
	psy_ui_notebook_setpageindex(&self->notebook, TABPAGE_RENDERVIEW);
}

void mainframe_ontimer(MainFrame* self, uintptr_t timerid)
{
	workspace_idle(&self->workspace);
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
			} else
			if (option == CHECKUNSAVE_NEW) {				
				self->checkunsavedbox.mode = option;
				checkunsavedbox_setlabels(&self->checkunsavedbox,
					"New Song Request, but your Song is not saved!",
					"Save and Create New Song",
					"Create New Song (no save)");
			} else
			if (option == CHECKUNSAVE_LOAD) {
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
	if (tabindex == TABPAGE_SAMPLESVIEW) {	
		psy_ui_notebook_setpageindex(&self->viewstatusbars, 2);
	} else {
		psy_ui_Component* page;
		psy_ui_notebook_setpageindex(&self->viewstatusbars, tabindex);
		page = psy_ui_notebook_activepage(&self->viewstatusbars);
		if (page) {
			psy_ui_component_align(page);
		}
	}
	psy_ui_notebook_setpageindex(&self->viewtabbars, tabindex);
	component = psy_ui_notebook_activepage(&self->notebook);
	if (component) {
		psy_ui_component_setfocus(component);
	}
}

void mainframe_onterminaloutput(MainFrame* self, Workspace* sender,
	const char* text)
{
	self->terminalhasmessage = TRUE;
	psy_ui_terminal_output(&self->terminal, text);
	if (!self->terminalhaswarning || self->terminalhaserror) {
		psy_ui_button_settextcolor(&self->toggleterminal,
			psy_ui_color_make(TERMINALMESSAGECOLOR));
	}
}

void mainframe_onterminalwarning(MainFrame* self, Workspace* sender,
	const char* text)
{
	self->terminalhaswarning = TRUE;
	if (text) {
		psy_ui_terminal_output(&self->terminal, text);
	} else {
		psy_ui_terminal_output(&self->terminal, "unknown warning\n");
	}
	if (!self->terminalhaserror) {
		psy_ui_button_settextcolor(&self->toggleterminal,
			psy_ui_color_make(TERMINALWARNINGCOLOR));
	}
}

void mainframe_onterminalerror(MainFrame* self, Workspace* sender,
	const char* text)
{
	self->terminalhaserror = TRUE;
	if (text) {
		psy_ui_terminal_output(&self->terminal, text);
	} else {
		psy_ui_terminal_output(&self->terminal, "unknown error\n");
	}
	psy_ui_button_settextcolor(&self->toggleterminal,
		psy_ui_color_make(TERMINALERRORCOLOR));
}

void mainframe_onzoomboxchanged(MainFrame* self, ZoomBox* sender)
{	
	workspace_changedefaultfontsize(&self->workspace, (int)(
		zoombox_rate(sender) * self->workspace.fontheight));
}

void mainframe_onsongtrackschanged(MainFrame* self, psy_audio_Player* sender,
	uintptr_t numsongtracks)
{
	psy_ui_component_align(&self->component);
}

void mainframe_onchangecontrolskin(MainFrame* self, Workspace* sender,
	const char* path)
{
	paramview_changecontrolskin(path);
}

void mainframe_ondockview(MainFrame* self, Workspace* sender,
	psy_ui_Component* view)
{
	psy_ui_Size size;

	size = psy_ui_component_size(view);
	psy_ui_component_resize(view,
		psy_ui_size_make(
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0)));
	psy_ui_component_setparent(view, &self->paramviews);	
	psy_ui_component_setalign(view, psy_ui_ALIGN_LEFT);	
	psy_ui_component_align(&self->client);	
	psy_ui_component_align(&self->paramviews);
}

void mainframe_onmousedown(MainFrame* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2 && psy_ui_mouseevent_target(ev) ==
			&self->recentview.component) {
		mainframe_onrecentsongs(self, &self->component);
	}
}

void mainframe_onlanguagechanged(MainFrame* self, Translator* sender)
{
	mainframe_updatetext(self);
	psy_ui_component_alignall(&self->component);
}

// this is called if a button is clicked in the checkunsavedbox
// option : which button pressed
// mode : source of request(app close, song load, song new)
void mainframe_oncheckunsaved(MainFrame* self, CheckUnsavedBox* sender,
	int option, int mode)
{	
	bool load;

	load = FALSE;
	switch (option) {			
		case CHECKUNSAVE_SAVE: {
			psy_ui_SaveDialog dialog;

			psy_ui_savedialog_init_all(&dialog, 0,
				"Save Song",
				"Songs (*.psy)|*.psy", "PSY",
				workspace_songs_directory(&self->workspace));
			if (psy_ui_savedialog_execute(&dialog)) {
				workspace_savesong(&self->workspace,
					psy_ui_savedialog_filename(&dialog));
				if (mode == CHECKUNSAVE_CLOSE) {
					psy_ui_app_close(&app);
				} else
				if (mode == CHECKUNSAVE_LOAD) {
					load = TRUE;
				} else
				if (mode == CHECKUNSAVE_NEW) {										
					workspace_newsong(&self->workspace);					
				}
			}
			psy_ui_savedialog_dispose(&dialog);
			break;
		}
		case CHECKUNSAVE_NOSAVE: {
			extern psy_ui_App app;

			self->workspace.undosavepoint = psy_list_size(
				self->workspace.undoredo.undo);
			self->workspace.machines_undosavepoint = psy_list_size(
				self->workspace.song->machines.undoredo.undo);
			if (mode == CHECKUNSAVE_CLOSE) {
				psy_ui_app_close(&app);
			} else
			if (mode == CHECKUNSAVE_LOAD) {
				load = TRUE;
			} else
			if (mode == CHECKUNSAVE_NEW) {
				workspace_newsong(&self->workspace);
			}
			break;
		}
		case CHECKUNSAVE_CONTINUE:
			workspace_updatecurrview(&self->workspace);
			break;
		default:		
			break;
	}
	if (load) {
		psy_ui_OpenDialog dialog;
		static char filter[] =
			"All Songs (*.psy *.xm *.it *.s3m *.mod *.wav)" "|*.psy;*.xm;*.it;*.s3m;*.mod;*.wav|"
			"Songs (*.psy)"				        "|*.psy|"
			"FastTracker II Songs (*.xm)"       "|*.xm|"
			"Impulse Tracker Songs (*.it)"      "|*.it|"
			"Scream Tracker Songs (*.s3m)"      "|*.s3m|"
			"Original Mod Format Songs (*.mod)" "|*.mod|"
			"Wav Format Songs (*.wav)"			"|*.wav";

		psy_ui_opendialog_init_all(&dialog, 0, "Load Song", filter, "PSY",
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
		workspace_selectview(&self->workspace, TABPAGE_CHECKUNSAVED, 0, CHECKUNSAVE_CLOSE);
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
		psy_ui_component_hide(seqeditor_base(&self->seqeditor));
		psy_ui_component_align(&self->client);
	} else {
		psy_ui_component_show(seqeditor_base(&self->seqeditor));
		psy_ui_component_align(&self->client);
		psy_ui_button_seticon(&self->sequenceview.options.toggleseqediticon,
			psy_ui_ICON_LESS);
		psy_ui_button_settext(&self->sequenceview.options.toggleseqedit,
			"Hide SequenceEditor");
	}
}

void mainframe_ontoggleterminal(MainFrame* self, psy_ui_Component* sender)
{
	if (!psy_ui_isvaluezero(psy_ui_component_size(
		&self->terminal.component).height)) {
		psy_ui_component_resize(&self->terminal.component,
			psy_ui_size_zero());
		self->terminalhaserror = FALSE;
		self->terminalhaswarning = FALSE;
		self->terminalhasmessage = FALSE;
		psy_ui_button_settextcolor(&self->toggleterminal, psy_ui_color_make(0x00CACACA));
	} else {
		psy_ui_component_resize(&self->terminal.component,
			psy_ui_size_make(psy_ui_value_makepx(0),
				psy_ui_value_makeeh(10)));		
	}
	psy_ui_component_align(&self->component);
}

void mainframe_ontogglekbdhelp(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(kbdhelp_base(&self->kbdhelp))) {
		psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
		psy_ui_component_align(&self->component);
	} else {
		psy_ui_component_show(kbdhelp_base(&self->kbdhelp));
		psy_ui_component_align(&self->component);
	}
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
