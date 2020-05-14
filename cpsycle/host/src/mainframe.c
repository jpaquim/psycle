// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "mainframe.h"
#include "cmdsgeneral.h"
#include "settingsview.h"
#include "resources/resource.h"
#include "paramview.h"

#include <dir.h>
#include <uiapp.h>

#include <songio.h>

#include "../../detail/portable.h"

#define TIMERID_MAINFRAME 20

static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_setstatusbartext(MainFrame*, const char* text);
static const char* mainframe_statusbaridletext(MainFrame*);
static void mainframe_destroy(MainFrame*, psy_ui_Component* component);
static void mainframe_onkeydown(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_onkeyup(MainFrame*, psy_ui_KeyEvent*);
static void mainframe_onmousedown(MainFrame*, psy_ui_MouseEvent*);
static void mainframe_onsequenceselchange(MainFrame* , SequenceEntry*);
static void mainframe_ongear(MainFrame*, psy_ui_Component* sender);
static void mainframe_oncpu(MainFrame*, psy_ui_Component* sender);
static void mainframe_onmidi(MainFrame*, psy_ui_Component* sender);
static void mainframe_onrecentsongs(MainFrame*, psy_ui_Component* sender);
static void mainframe_onplugineditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ongearcreate(MainFrame*, psy_ui_Component* sender);
static void mainframe_onaboutok(MainFrame*, psy_ui_Component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, SettingsView* sender,
	psy_Properties*);
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
static void mainframe_ontimer(MainFrame*, int timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, psy_EventDriver* sender);

static void mainframe_onterminaloutput(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalwarning(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalerror(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onzoomboxchanged(MainFrame*, ZoomBox* sender);
static void mainframe_onsongtrackschanged(MainFrame*, psy_audio_Player* sender,
	unsigned int numsongtracks);
static void mainframe_onchangecontrolskin(MainFrame*, Workspace* sender,
	const char* path);
static void mainframe_ondockview(MainFrame*, Workspace* sender,
	psy_ui_Component* view);

#define GEARVIEW 10

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(MainFrame* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onkeydown = (psy_ui_fp_onkeydown) mainframe_onkeydown;
		vtable.onkeyup = (psy_ui_fp_onkeyup) mainframe_onkeyup;
		vtable.onmousedown = (psy_ui_fp_onmousedown) mainframe_onmousedown;
		vtable.ontimer = (psy_ui_fp_ontimer) mainframe_ontimer;
	}
}

void mainframe_init(MainFrame* self)
{			
	psy_ui_Margin tabbardividemargin;
	
	psy_ui_margin_init(&tabbardividemargin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(4.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_frame_init(&self->component, 0);
	vtable_init(self);
	self->component.vtable = &vtable;
	
	psy_ui_component_seticonressource(&self->component, IDI_PSYCLEICON);
	psy_ui_component_enablealign(&self->component);	
	workspace_init(&self->workspace, &self->component);	
	workspace_load_configuration(&self->workspace);
	workspace_load_recentsongs(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}	
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);	
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
	psy_ui_component_resize(&self->terminal.component, 0, 0);
	kbdhelp_init(&self->kbdhelp, &self->component, &self->workspace);
	psy_ui_component_setalign(kbdhelp_base(&self->kbdhelp),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
	psy_ui_splitbar_init(&self->splitbarterminal, &self->component);
	psy_ui_component_setalign(&self->splitbarterminal.component,
		psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&self->component.signal_destroy, self,
		mainframe_destroy);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_enablealign(&self->top);	
	// client
	psy_ui_component_init(&self->client, &self->component);	
	psy_ui_component_setbackgroundmode(&self->client, BACKGROUND_NONE);
	psy_ui_component_enablealign(&self->client);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);		
	psy_ui_component_init(&self->paramviews, &self->client);
	psy_ui_component_enablealign(&self->paramviews);
	psy_ui_component_setalign(&self->paramviews, psy_ui_ALIGN_BOTTOM);
	//psy_ui_component_resize(&self->paramviews, 100, 100);
	// tabbars			
	{
		psy_ui_Margin spacing;

		psy_ui_margin_init(&spacing, psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
			psy_ui_value_makepx(0));
		psy_ui_component_init(&self->tabbars, &self->client);		
		psy_ui_component_enablealign(&self->tabbars);
		psy_ui_component_setspacing(&self->tabbars, &spacing);
		mainframe_initbars(self);
		psy_signal_connect(&self->filebar.recentbutton.signal_clicked, self,
			mainframe_onrecentsongs);
	}
	// tabbars
	psy_ui_component_setalign(&self->tabbars, psy_ui_ALIGN_TOP);
	psy_ui_component_enablealign(&self->tabbars);
	navigation_init(&self->navigation, &self->tabbars, &self->workspace);
	self->navigation.component.margin.right = psy_ui_value_makeew(2.0);
	psy_ui_component_setalign(&self->navigation.component, psy_ui_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->tabbars);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	psy_ui_component_setalignexpand(tabbar_base(&self->tabbar),	
		psy_ui_HORIZONTALEXPAND);	
	tabbar_append_tabs(&self->tabbar, "Machines", "Patterns", "Samples",
		"Instruments", "Properties", NULL);
	tabbar_append(&self->tabbar, "Settings")->margin.left =
		psy_ui_value_makeew(4.0);
	tabbar_append(&self->tabbar, "Help")->margin.right =
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
	settingsview_init(&self->settingsview,
		psy_ui_notebook_base(&self->notebook),
		&self->viewtabbars.component,
		self->workspace.config);
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
	// gear
	gear_init(&self->gear, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->gear.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->gear.component);
	psy_signal_connect(&self->machinebar.gear.signal_clicked, self,
		mainframe_ongear);
	// cpuview
	cpuview_init(&self->cpuview, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->cpuview.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->cpuview.component);
	psy_signal_connect(&self->machinebar.cpu.signal_clicked, self,
		mainframe_oncpu);
	// midiview
	midiview_init(&self->midiview, &self->client, &self->workspace);
	psy_ui_component_setalign(&self->midiview.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->midiview.component);	
	psy_signal_connect(&self->machinebar.midi.signal_clicked, self,
		mainframe_onmidi);
	plugineditor_init(&self->plugineditor, &self->component, &self->workspace);
	psy_ui_component_setalign(&self->plugineditor.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_resize(&self->plugineditor.component, 400, 0);
	psy_ui_component_hide(&self->plugineditor.component);
	psy_signal_connect(&self->machinebar.editor.signal_clicked, self,
		mainframe_onplugineditor);
	psy_signal_connect(&self->gear.buttons.createreplace.signal_clicked, self,
		mainframe_ongearcreate);
	psy_signal_emit(&self->workspace.signal_configchanged,
		&self->workspace, 1, self->workspace.config);
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
	// recent song view
	recentview_init(&self->recentview, &self->component,
		&self->viewtabbars.component,
		&self->workspace);
	psy_ui_component_setalign(&self->recentview.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->recentview.component);
	// sequenceview
	sequenceview_init(&self->sequenceview, &self->component, &self->workspace);
	psy_ui_component_setalign(&self->sequenceview.component,
		psy_ui_ALIGN_LEFT);
	// splitbar
	psy_ui_splitbar_init(&self->splitbar, &self->component);
	// create statusbar components
	mainframe_initstatusbar(self);
	psy_ui_component_starttimer(&self->component, TIMERID_MAINFRAME, 50);
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
		
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));	
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
	psy_ui_notebook_setpageindex(&self->viewstatusbars, 0);
//	psy_ui_notebook_connectcontroller(&self->viewstatusbars,
	//	&self->tabbar.signal_change);
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

	psy_ui_margin_init(&row0margin, psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeeh(0.5));
	psy_ui_margin_init(&rowmargin, psy_ui_value_makepx(0),
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
	playposbar_init(&self->playposbar, &self->toprow0, &self->workspace.player);	
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->toprow0, 0),
		psy_ui_ALIGN_LEFT, &margin));
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
}

void mainframe_destroy(MainFrame* self, psy_ui_Component* component)
{
	extern psy_ui_App app;

	workspace_save_configuration(&self->workspace);
	workspace_dispose(&self->workspace);
	psy_ui_app_stop(&app);
}

void mainframe_oneventdriverinput(MainFrame* self, psy_EventDriver* sender)
{
	EventDriverCmd cmd;
	// 	psy_Properties* section;

	// section = psy_properties_find(sender->properties, "general");
	cmd = sender->getcmd(sender, 0);
	if (cmd.id == CMD_IMM_HELP) {
		tabbar_select(&self->helpview.tabbar, 0);
		tabbar_select(&self->tabbar, TABPAGE_HELPVIEW);		
	} else
	if (cmd.id == CMD_IMM_HELPSHORTCUT) {
		if (psy_ui_component_visible(kbdhelp_base(&self->kbdhelp))) {
			psy_ui_component_hide(kbdhelp_base(&self->kbdhelp));
			psy_ui_component_align(&self->component);
		} else {
			psy_ui_component_show(kbdhelp_base(&self->kbdhelp));
			psy_ui_component_align(&self->component);
		}
	} else
	if (cmd.id == CMD_IMM_EDITMACHINE) {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	} else
	if (cmd.id == CMD_IMM_EDITPATTERN) {
		tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
	} else
	if (cmd.id == CMD_IMM_ADDMACHINE) {
		self->machineview.newmachine.pluginsview.calledby =
			self->tabbar.selected;
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		tabbar_select(&self->machineview.tabbar, 1);
	} else
	if (cmd.id == CMD_IMM_PLAYSONG) {
		psy_audio_player_setposition(&self->workspace.player, 0);
		psy_audio_player_start(&self->workspace.player);
	} else
	if (cmd.id == CMD_IMM_PLAYSTART) {
		SequenceEntry* entry;

		entry = sequenceposition_entry(&self->workspace.sequenceselection.editposition);		
		psy_audio_player_setposition(&self->workspace.player,
			(entry) ? entry->offset : 0);
		psy_audio_player_start(&self->workspace.player);
	} else
	if (cmd.id == CMD_IMM_PLAYFROMPOS) {
		psy_dsp_beat_t playposition = 0;
		SequenceEntry* entry;

		entry = sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) + 
			(psy_dsp_beat_t) self->workspace.patterneditposition.offset;
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
	} else
	if (cmd.id == CMD_IMM_PLAYSTOP) {
		psy_audio_player_stop(&self->workspace.player);
	} else
	if (cmd.id == CMD_IMM_SONGPOSDEC) {
		if (self->workspace.song) {		
			if (self->workspace.sequenceselection.editposition.trackposition.tracknode &&
					self->workspace.sequenceselection.editposition.trackposition.tracknode->prev) {
				sequenceselection_seteditposition(
					&self->workspace.sequenceselection,
					sequence_makeposition(&self->workspace.song->sequence,
						self->workspace.sequenceselection.editposition.track,
						self->workspace.sequenceselection.editposition.trackposition.tracknode->prev));
				workspace_setsequenceselection(&self->workspace, 
					self->workspace.sequenceselection);
			}
		}
	} else
	if (cmd.id == CMD_IMM_SONGPOSINC) {
		if (self->workspace.song) {						
			if (self->workspace.sequenceselection.editposition.trackposition.tracknode &&
					self->workspace.sequenceselection.editposition.trackposition.tracknode->next) {
				sequenceselection_seteditposition(
					&self->workspace.sequenceselection,
					sequence_makeposition(&self->workspace.song->sequence,
						self->workspace.sequenceselection.editposition.track,
						self->workspace.sequenceselection.editposition.trackposition.tracknode->next));
				workspace_setsequenceselection(&self->workspace, 
					self->workspace.sequenceselection);
			}
		}
	} else
	if (cmd.id == CMD_IMM_MAXPATTERN) {
		mainframe_maximizeorminimizeview(self);
	} else
	if (cmd.id == CMD_IMM_INFOMACHINE) {
		workspace_showparameters(&self->workspace,
			machines_slot(&self->workspace.song->machines));
	} else
	if (cmd.id == CMD_IMM_EDITINSTR) {
		workspace_selectview(&self->workspace, TABPAGE_INSTRUMENTSVIEW, 0, 0);
	} else
	if (cmd.id == CMD_IMM_EDITSAMPLE) {
		workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW, 0, 0);
		tabbar_select(&self->samplesview.clienttabbar, 0);
	} else
	if (cmd.id == CMD_IMM_EDITWAVE) {
		workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW, 0, 0);
		tabbar_select(&self->samplesview.clienttabbar, 2);
	} else
	if (cmd.id == CMD_IMM_TERMINAL) {
		psy_ui_Size size = psy_ui_component_size(&self->terminal.component);
		if (size.height > 0) {
			psy_ui_component_resize(&self->terminal.component, 0, 0);
			psy_ui_component_align(&self->component);
		} else {								
			psy_ui_component_resize(&self->terminal.component, 0, 100);
			psy_ui_component_align(&self->component);		
		}
	}
}

void mainframe_onkeydown(MainFrame* self, psy_ui_KeyEvent* ev)
{	
	if (ev->keycode != psy_ui_KEY_CONTROL && ev->keycode != psy_ui_KEY_SHIFT) {
		psy_EventDriver* kbd;
		EventDriverData input;			
		
		input.message = EVENTDRIVER_KEYDOWN;
		kbd = workspace_kbddriver(&self->workspace);		
		input.param1 = encodeinput(ev->keycode, 
			self->workspace.chordmode ? 0 : ev->shift, ev->ctrl);		
		input.param2 = workspace_octave(&self->workspace) * 12;
		kbd->write(kbd, input);
	}
}

void mainframe_onkeyup(MainFrame* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode != psy_ui_KEY_CONTROL && ev->keycode != psy_ui_KEY_SHIFT) {
		psy_EventDriver* kbd;
		EventDriverData input;			
		
		input.message = EVENTDRIVER_KEYUP;
#if defined DIVERSALIS__OS__MICROSOFT_        
		input.param1 = encodeinput(ev->keycode, GetKeyState(psy_ui_KEY_SHIFT) < 0,
			GetKeyState(psy_ui_KEY_CONTROL) < 0);
#else
        input.param1 = encodeinput(ev->keycode, ev->shift, ev->ctrl);
#endif          
		input.param2 = 48;
		kbd = workspace_kbddriver(&self->workspace);
		kbd->write(kbd, input);
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
		psy_ui_component_resize(&self->sequenceview.component, 
			self->workspace.maximizeview.sequenceviewrestorewidth, 0);
	} else {
		self->workspace.maximizeview.maximized = 1;
		self->workspace.maximizeview.row0 = self->toprow0.visible;
		self->workspace.maximizeview.row1 = self->toprow1.visible;
		self->workspace.maximizeview.row2 = self->toprow2.visible;
		self->workspace.maximizeview.sequenceviewrestorewidth =
			psy_ui_component_size(&self->sequenceview.component).width;
		psy_ui_component_hide(&self->toprow0);
		psy_ui_component_hide(&self->toprow1);
		psy_ui_component_resize(&self->sequenceview.component, 0, 0);			
	}
	psy_ui_component_align(&self->component);
}

void mainframe_onsongloadprogress(MainFrame* self, Workspace* workspace, int progress)
{	
	if (progress == -1) {
		psy_ui_terminal_clear(&self->terminal);
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
		psy_ui_button_highlight(&self->machinebar.gear);
		psy_ui_component_show(&self->gear.component);
		psy_ui_component_align(&self->client);
	}
}

void mainframe_ongear(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->gear.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.gear);
		psy_ui_component_hide(&self->gear.component);
		psy_ui_component_align(&self->client);
	} else {						
		psy_ui_button_highlight(&self->machinebar.gear);
		psy_ui_component_show(&self->gear.component);
		psy_ui_component_align(&self->client);
	}	
}

void mainframe_oncpu(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->cpuview.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.cpu);
		psy_ui_component_hide(&self->cpuview.component);
		psy_ui_component_align(&self->client);
	}
	else {
		psy_ui_button_highlight(&self->machinebar.cpu);
		psy_ui_component_show(&self->cpuview.component);
		psy_ui_component_align(&self->client);
	}
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

void mainframe_onplugineditor(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->plugineditor.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.editor);
		psy_ui_component_hide(&self->plugineditor.component);
		psy_ui_component_align(&self->component);
	} else {						
		psy_ui_button_highlight(&self->machinebar.editor);
		psy_ui_component_show(&self->plugineditor.component);
		psy_ui_component_align(&self->component);		
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

void mainframe_onsettingsviewchanged(MainFrame* self, SettingsView* sender,
	psy_Properties* property)
{	
	if (strcmp(psy_properties_key(property), "showstepsequencer") == 0) {
		if (workspace_showstepsequencer(&self->workspace)) {
			psy_ui_component_show(&self->stepsequencerview.component);
		} else {
			psy_ui_component_hide(&self->stepsequencerview.component);
		}
		psy_ui_component_align(&self->client);
	}
	if (strcmp(psy_properties_key(property), "trackscopes") == 0) {
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
		workspace_configchanged(&self->workspace, property,
			sender->choiceproperty);
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

void mainframe_ontimer(MainFrame* self, int timerid)
{
	workspace_idle(&self->workspace);
}

void mainframe_onviewselected(MainFrame* self, Workspace* sender, int index,
	uintptr_t section, int option)
{
	if (index != GEARVIEW) {
		psy_ui_Component* view;

		tabbar_select(&self->tabbar, index);
		view = psy_ui_notebook_activepage(&self->notebook);
		if (view) {
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
	psy_ui_terminal_output(&self->terminal, text);
}

void mainframe_onterminalwarning(MainFrame* self, Workspace* sender,
	const char* text)
{
	if (!psy_ui_component_visible(&self->terminal.component)) {
		psy_ui_component_show(&self->terminal.component);
		psy_ui_component_align(&self->component);		
	}	
	psy_ui_terminal_output(&self->terminal, text);
}

void mainframe_onterminalerror(MainFrame* self, Workspace* sender,
	const char* text)
{
	if (!psy_ui_component_visible(&self->terminal.component)) {		
		psy_ui_component_show(&self->terminal.component);
		psy_ui_component_align(&self->component);		
	}	
	psy_ui_terminal_output(&self->terminal, text);
}

void mainframe_onzoomboxchanged(MainFrame* self, ZoomBox* sender)
{	
	workspace_changedefaultfontsize(&self->workspace, (int)(
		zoombox_rate(sender) * self->workspace.fontheight));
}

void mainframe_onsongtrackschanged(MainFrame* self, psy_audio_Player* sender,
	unsigned int numsongtracks)
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
	psy_ui_component_resize(view, 0, 0);
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
