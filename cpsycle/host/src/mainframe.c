// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
#include "cmdsgeneral.h"
#include "settingsview.h"
#include "resources/resource.h"

#include "../../detail/portable.h"
#include <dir.h>
#include <uiapp.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


#define TIMERID_MAINFRAME 20

static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_setstatusbartext(MainFrame*, const char* text);
static const char* mainframe_statusbaridletext(MainFrame*);
static void mainframe_destroy(MainFrame*, psy_ui_Component* component);
static void mainframe_onkeydown(MainFrame*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void mainframe_onkeyup(MainFrame*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void mainframe_onsequenceselchange(MainFrame* , SequenceEntry*);
static void mainframe_ongear(MainFrame*, psy_ui_Component* sender);
static void mainframe_onplugineditor(MainFrame*, psy_ui_Component* sender);
static void mainframe_ongearcreate(MainFrame*, psy_ui_Component* sender);
static void mainframe_onaboutok(MainFrame*, psy_ui_Component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, SettingsView* sender,
	psy_Properties*);
static void mainframe_ontabbarchanged(MainFrame*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void mainframe_onsongchanged(MainFrame*, psy_ui_Component* sender,
	int flag);
static void mainframe_onsongloadprogress(MainFrame*, Workspace*, int progress);
static void mainframe_onpluginscanprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onviewselected(MainFrame*, Workspace*, int view);
static void mainframe_onrender(MainFrame*, psy_ui_Component* sender);
static void mainframe_updatetitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, psy_ui_Component* sender,
	int timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, psy_EventDriver* sender);

static void mainframe_onterminaloutput(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalwarning(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onterminalerror(MainFrame*, Workspace* sender,
	const char* text);
static void mainframe_onzoomboxchanged(MainFrame*, ZoomBox* sender);

#define GEARVIEW 10

enum {
	TABPAGE_MACHINEVIEW		= 0,
	TABPAGE_PATTERNVIEW		= 1,
	TABPAGE_SAMPLESVIEW		= 2,
	TABPAGE_INSTRUMENTSVIEW = 3,
	TABPAGE_PROPERTIESVIEW	= 4,
	TABPAGE_SETTINGSVIEW	= 5,
	TABPAGE_HELPVIEW		= 6,
	TABPAGE_RENDERVIEW		= 7
};

void mainframe_init(MainFrame* self)
{			
	psy_ui_Margin tabbardividemargin;
	
	psy_ui_margin_init(&tabbardividemargin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(4.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_frame_init(&self->component, 0);
	
	psy_ui_component_seticonressource(&self->component, IDI_PSYCLEICON);
	psy_ui_component_enablealign(&self->component);
	
	workspace_init(&self->workspace, &self->component);	
	workspace_load_configuration(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}	
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
	mainframe_updatetitle(self);
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
	psy_ui_component_hide(&self->terminal.component);
	psy_ui_component_resize(&self->terminal.component, 0, 100);
	psy_signal_connect(&self->component.signal_destroy, self, mainframe_destroy);	
	psy_signal_connect(&self->component.signal_keydown, self, mainframe_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self, mainframe_onkeyup);
	psy_signal_connect(&self->component.signal_timer, self, mainframe_ontimer);
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_enablealign(&self->top);
	// sequenceview
	// psy_ui_component_init(&self->left, &self->component);
	// psy_ui_component_setalign(&self->left, UI_ALIGN_LEFT);
	// psy_ui_component_enablealign(&self->left);	
	// self->left.preventpreferredsize = 1;
	sequenceview_init(&self->sequenceview, &self->component, &self->workspace);	
	psy_ui_component_setalign(&self->sequenceview.component, psy_ui_ALIGN_LEFT);
	// client
	psy_ui_component_init(&self->client, &self->component);	
	psy_ui_component_setbackgroundmode(&self->client, BACKGROUND_NONE);
	psy_ui_component_enablealign(&self->client);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);	
	stepsequencerview_init(&self->stepsequencerview, &self->client,
		&self->workspace);
	psy_ui_component_setalign(&self->stepsequencerview.component,
		psy_ui_ALIGN_BOTTOM);
	if (!workspace_showstepsequencer(&self->workspace)) {
		psy_ui_component_hide(&self->stepsequencerview.component);
	}	
	// tabbars			
	{
		psy_ui_Margin spacing;

		psy_ui_margin_init(&spacing, psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
			psy_ui_value_makepx(0));
		psy_ui_component_init(&self->tabbars, &self->client);
		psy_ui_component_setspacing(&self->tabbars, &spacing);
		mainframe_initbars(self);
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
	tabbar_append(&self->tabbar, "Machines");
	tabbar_append(&self->tabbar, "Patterns");	
	tabbar_append(&self->tabbar, "Samples");
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Properties");
	tabbar_append(&self->tabbar, "Settings")->margin.left =
		psy_ui_value_makeew(4.0);	;
	tabbar_append(&self->tabbar, "Help")->margin.right =
		psy_ui_value_makeew(4.0);
	// splitbar
	psy_ui_splitbar_init(&self->splitbar, &self->component);	
	/// init notebook views
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);	
	machineview_init(&self->machineview, psy_ui_notebook_base(&self->notebook), 
		&self->tabbars, &self->workspace);
	patternview_init(&self->patternview, psy_ui_notebook_base(&self->notebook),
		&self->tabbars, &self->workspace);	
	samplesview_init(&self->samplesview, psy_ui_notebook_base(&self->notebook),
		&self->tabbars, &self->workspace);	
	instrumentview_init(&self->instrumentsview,
		psy_ui_notebook_base(&self->notebook),
		&self->workspace);
	songpropertiesview_init(&self->songpropertiesview,
		&self->notebook.component, &self->workspace);	
	settingsview_init(&self->settingsview, psy_ui_notebook_base(&self->notebook),
		&self->tabbars, self->workspace.config);
	psy_signal_connect(&self->settingsview.signal_changed, self,
		mainframe_onsettingsviewchanged);
	helpview_init(&self->helpview, psy_ui_notebook_base(&self->notebook),
		&self->tabbars, &self->workspace);	
	psy_signal_connect(&self->helpview.about.okbutton.signal_clicked, self,
		mainframe_onaboutok);	
	renderview_init(&self->renderview, psy_ui_notebook_base(&self->notebook),
		&self->tabbars, &self->workspace);
	psy_signal_connect(&self->filebar.renderbutton.signal_clicked, self,
		mainframe_onrender);
	psy_signal_connect(&self->workspace.signal_viewselected, self,
		mainframe_onviewselected);
	gear_init(&self->gear, &self->component, &self->workspace);
	psy_ui_component_setalign(&self->gear.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->gear.component);
	psy_signal_connect(&self->machinebar.gear.signal_clicked, self,
		mainframe_ongear);
	plugineditor_init(&self->plugineditor, &self->component, &self->workspace);
	psy_ui_component_setalign(&self->plugineditor.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_resize(&self->plugineditor.component, 400, 0);
	psy_ui_component_hide(&self->plugineditor.component);
	psy_signal_connect(&self->machinebar.editor.signal_clicked, self,
		mainframe_onplugineditor);
	psy_signal_connect(&self->gear.buttons.createreplace.signal_clicked, self,
		mainframe_ongearcreate);
	mainframe_setstartpage(self);
	psy_signal_emit(&self->workspace.signal_configchanged,
		&self->workspace, 1, self->workspace.config);
	psy_signal_connect(&self->tabbar.signal_change, self,
		mainframe_ontabbarchanged);
	workspace_addhistory(&self->workspace);	
	psy_signal_connect(&self->workspace.player.eventdrivers.signal_input, self,
		mainframe_oneventdriverinput);
	// create statusbar components
	mainframe_initstatusbar(self);
	psy_ui_component_starttimer(&self->component, TIMERID_MAINFRAME, 50);
	if (self->workspace.song) {
		mainframe_setstatusbartext(self,
			self->workspace.song->properties.title);
	}
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
		
	psy_ui_margin_init(&margin, psy_ui_value_makeeh(0.5),
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
	psy_ui_notebook_init(&self->viewbars, &self->statusbar);
	psy_ui_component_setalign(&self->viewbars.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_enablealign(&self->viewbars.component);		
	machineviewbar_init(&self->machineviewbar, &self->viewbars.component,
		&self->workspace);
	psy_ui_component_setalign(&self->machineviewbar.component,
		psy_ui_ALIGN_LEFT);
	self->machineview.wireview.statusbar = &self->machineviewbar;
	patternviewbar_init(&self->patternbar, &self->viewbars.component,
		&self->workspace);
	psy_ui_component_setalign(&self->patternbar.component, psy_ui_ALIGN_LEFT);
	psy_ui_notebook_setpageindex(&self->viewbars, 0);
	psy_ui_notebook_connectcontroller(&self->viewbars,
		&self->tabbar.signal_change);
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
	// Vugroup
	vubar_init(&self->vubar, &self->top, &self->workspace);	
	psy_ui_component_setalign(&self->vubar.component, psy_ui_ALIGN_RIGHT);
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
		tabbar_select(&self->tabbar, TABPAGE_HELPVIEW);
		psy_ui_component_show(&self->helpview.tabbar.component);
	} else {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		psy_ui_component_show(&self->machineview.tabbar.component);
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
	if (cmd.id == CMD_IMM_EDITMACHINE) {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	} else
	if (cmd.id == CMD_IMM_EDITPATTERN) {
		tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
	} else
	if (cmd.id == CMD_IMM_ADDMACHINE) {
		self->machineview.newmachine.pluginsview.calledby = self->tabbar.selected;
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		tabbar_select(&self->machineview.tabbar, 1);
	} else
	if (cmd.id == CMD_IMM_PLAYSONG) {
		player_setposition(&self->workspace.player, 0);
		player_start(&self->workspace.player);
	} else
	if (cmd.id == CMD_IMM_PLAYSTART) {
		SequenceEntry* entry;

		entry = sequenceposition_entry(&self->workspace.sequenceselection.editposition);		
		player_setposition(&self->workspace.player, entry ? entry->offset : 0);		
		player_start(&self->workspace.player);		
	} else
	if (cmd.id == CMD_IMM_PLAYFROMPOS) {
		psy_dsp_beat_t playposition = 0;
		SequenceEntry* entry;

		entry = sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) + 
			(psy_dsp_beat_t) self->workspace.patterneditposition.offset;
		player_setposition(&self->workspace.player, playposition);
		player_start(&self->workspace.player);
	} else
	if (cmd.id == CMD_IMM_PLAYSTOP) {
		player_stop(&self->workspace.player);
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
		workspace_selectview(&self->workspace, TABPAGE_INSTRUMENTSVIEW);
	} else
	if (cmd.id == CMD_IMM_EDITSAMPLE) {
		workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW);
		tabbar_select(&self->samplesview.clienttabbar, 0);
	} else
	if (cmd.id == CMD_IMM_EDITWAVE) {
		workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW);
		tabbar_select(&self->samplesview.clienttabbar, 2);
	} else
	if (cmd.id == CMD_IMM_TERMINAL) {
		if (psy_ui_component_visible(&self->terminal.component)) {
			psy_ui_component_hide(&self->terminal.component);
			psy_ui_component_align(&self->component);
		} else {								
			psy_ui_component_show(&self->terminal.component);
			psy_ui_component_align(&self->component);		
		}
	}
}

void mainframe_onkeydown(MainFrame* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
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

void mainframe_onkeyup(MainFrame* self, psy_ui_Component* component,
	psy_ui_KeyEvent* ev)
{
	if (ev->keycode != psy_ui_KEY_CONTROL && ev->keycode != psy_ui_KEY_SHIFT) {
		psy_EventDriver* kbd;
		EventDriverData input;			
		
		input.message = EVENTDRIVER_KEYUP;
		input.param1 = encodeinput(ev->keycode, GetKeyState(psy_ui_KEY_SHIFT) < 0,
			GetKeyState(psy_ui_KEY_CONTROL) < 0);
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

void mainframe_onsongchanged(MainFrame* self, psy_ui_Component* sender, int flag)
{		
	if (flag == WORKSPACE_LOADSONG) {
		if (workspace_showsonginfoonload(&self->workspace)) {
			tabbar_select(&self->tabbar, TABPAGE_PROPERTIESVIEW);
		}
		if (self->workspace.song) {			
			mainframe_setstatusbartext(self,
				self->workspace.song->properties.title);
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
}


void mainframe_ongear(MainFrame* self, psy_ui_Component* sender)
{
	if (psy_ui_component_visible(&self->gear.component)) {
		psy_ui_button_disablehighlight(&self->machinebar.gear);
		psy_ui_component_hide(&self->gear.component);
		psy_ui_component_align(&self->component);
	} else {						
		psy_ui_button_highlight(&self->machinebar.gear);
		psy_ui_component_show(&self->gear.component);
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

void mainframe_ontimer(MainFrame* self, psy_ui_Component* sender, int timerid)
{
	workspace_idle(&self->workspace);
}

void mainframe_onviewselected(MainFrame* self, Workspace* sender, int view)
{
	if (view != GEARVIEW) {
		tabbar_select(&self->tabbar, view);
	}
}

void mainframe_ontabbarchanged(MainFrame* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	psy_ui_Component* component;
	workspace_onviewchanged(&self->workspace, tabindex);
	component = psy_ui_notebook_activepage(&self->notebook);
	psy_ui_component_align(&self->component);
	psy_ui_component_setfocus(component);
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
		zoombox_rate(sender) * 80));	
}
