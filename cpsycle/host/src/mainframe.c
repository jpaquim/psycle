// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
#include "cmdsgeneral.h"
#include "settingsview.h"
#include "resources/resource.h"

#include <portable.h>
#include <dir.h>
#include <uiapp.h>

#define TIMERID_MAINFRAME 20

static void mainframe_initmenu(MainFrame*);
static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_setstatusbartext(MainFrame*, const char* text);
static const char* mainframe_statusbaridletext(MainFrame* self);
static void mainframe_destroy(MainFrame*, ui_component* component);
static void mainframe_onkeydown(MainFrame*, ui_component* sender, KeyEvent*);
static void mainframe_onkeyup(MainFrame*, ui_component* sender, KeyEvent*);
static void mainframe_onsequenceselchange(MainFrame* , SequenceEntry* entry);
static void mainframe_onalign(MainFrame*, ui_component* sender);
static void mainframe_onmousedown(MainFrame*, ui_component* sender,
	MouseEvent*);
static void mainframe_onmousemove(MainFrame*, ui_component* sender,
	MouseEvent*);
static void mainframe_onmouseup(MainFrame*, ui_component* sender,
	MouseEvent*);
static void mainframe_ongear(MainFrame*, ui_component* sender);
static void mainframe_onplugineditor(MainFrame*, ui_component* sender);
static void mainframe_ongearcreate(MainFrame*, ui_component* sender);
static void mainframe_onaboutok(MainFrame*, ui_component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, SettingsView* sender,
	Properties*);
static void mainframe_ontabbarchanged(MainFrame*, ui_component* sender, uintptr_t tabindex);
static void mainframe_onmouseentersplitbar(MainFrame*, ui_component* sender);
static void mainframe_onmouseleavesplitbar(MainFrame*, ui_component* sender);
static void mainframe_onsongchanged(MainFrame*, ui_component* sender,
	int flag);
static void mainframe_onsongloadprogress(MainFrame*, Workspace*, int progress);
static void mainframe_onpluginscanprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onviewselected(MainFrame*, Workspace*, int view);
static void mainframe_onrender(MainFrame*, ui_component* sender);
static void mainframe_updatetitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, ui_component* sender, int timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);
static void mainframe_oneventdriverinput(MainFrame*, EventDriver* sender);

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
	ui_margin tabbardividemargin;
	
	ui_margin_init(&tabbardividemargin, ui_value_makepx(0), ui_value_makeew(4.0),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_frame_init(&self->component, 0);
	ui_component_seticonressource(&self->component, IDI_PSYCLEICON);
	ui_component_enablealign(&self->component);
	self->resize = 0;
	workspace_init(&self->workspace, &self->component);	
	workspace_load_configuration(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}	
	psy_signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
	mainframe_updatetitle(self);
	self->firstshow = 1;			
	mainframe_initbars(self);
	ui_terminal_init(&self->terminal, &self->component);
	ui_component_hide(&self->terminal.component);
	ui_component_resize(&self->terminal.component, 0, 100);
	psy_signal_connect(&self->component.signal_destroy, self, mainframe_destroy);
	psy_signal_connect(&self->component.signal_align, self, mainframe_onalign);	
	psy_signal_connect(&self->component.signal_keydown, self, mainframe_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self, mainframe_onkeyup);
	psy_signal_connect(&self->component.signal_timer, self, mainframe_ontimer);
	ui_component_init(&self->tabbars, &self->component);
	ui_component_enablealign(&self->tabbars);
	navigation_init(&self->navigation, &self->tabbars, &self->workspace);
	self->navigation.component.margin.right = ui_value_makeew(4.0);
	ui_component_setalign(&self->navigation.component, UI_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->tabbars);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_LEFT);
	ui_component_setalignexpand(&self->tabbar.component, UI_HORIZONTALEXPAND);	
	tabbar_append(&self->tabbar, "Machines");
	tabbar_append(&self->tabbar, "Pattern");	
	tabbar_append(&self->tabbar, "Samples");
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Properties");
	tabbar_append(&self->tabbar, "Settings")->margin.left = ui_value_makeew(4.0);	;
	tabbar_append(&self->tabbar, "Help")->margin.right = ui_value_makeew(4.0);
	// splitbar
	ui_component_init(&self->splitbar, &self->component);	
	psy_signal_connect(&self->splitbar.signal_mouseenter, self,
		mainframe_onmouseentersplitbar);
	psy_signal_connect(&self->splitbar.signal_mouseleave, self,
		mainframe_onmouseleavesplitbar);
	/// init notebook views
	ui_notebook_init(&self->notebook, &self->component);	
	ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);	
	machineview_init(&self->machineview, &self->notebook.component, 
		&self->tabbars, &self->workspace);
	patternview_init(&self->patternview, &self->notebook.component,
		&self->tabbars, &self->workspace);	
	samplesview_init(&self->samplesview, &self->notebook.component,
		&self->tabbars, &self->workspace);	
	instrumentview_init(&self->instrumentsview, &self->notebook.component,
		&self->workspace);
	songpropertiesview_init(&self->songpropertiesview,
		&self->notebook.component, &self->workspace);	
	settingsview_init(&self->settingsview, &self->notebook.component,
		&self->tabbars, self->workspace.config);
	psy_signal_connect(&self->settingsview.signal_changed, self,
		mainframe_onsettingsviewchanged);
	helpview_init(&self->helpview, &self->notebook.component, &self->tabbars,
		&self->workspace);	
	psy_signal_connect(&self->helpview.about.okbutton.signal_clicked, self,
		mainframe_onaboutok);
	sequenceview_init(&self->sequenceview, &self->component, &self->workspace);
	renderview_init(&self->renderview, &self->notebook.component,
		&self->tabbars, &self->workspace);
	psy_signal_connect(&self->filebar.renderbutton.signal_clicked, self,
		mainframe_onrender);
	psy_signal_connect(&self->workspace.signal_viewselected, self,
		mainframe_onviewselected);
	InitGear(&self->gear, &self->component, &self->workspace);
	ui_component_hide(&self->gear.component);
	psy_signal_connect(&self->machinebar.gear.signal_clicked, self,
		mainframe_ongear);
	plugineditor_init(&self->plugineditor, &self->component, &self->workspace);
	ui_component_hide(&self->plugineditor.component);
	psy_signal_connect(&self->machinebar.editor.signal_clicked, self,
		mainframe_onplugineditor);
	psy_signal_connect(&self->gear.buttons.createreplace.signal_clicked, self,
		mainframe_ongearcreate);
	mainframe_initstatusbar(self);
	psy_signal_connect(&self->splitbar.signal_mousedown, self,
		mainframe_onmousedown);
	psy_signal_connect(&self->splitbar.signal_mousemove, self,
		mainframe_onmousemove);
	psy_signal_connect(&self->splitbar.signal_mouseup, self,
		mainframe_onmouseup);
	mainframe_setstartpage(self);
	if (self->workspace.song) {
		mainframe_setstatusbartext(self,
			self->workspace.song->properties.title);
	}	
	psy_signal_emit(&self->workspace.signal_configchanged,
		&self->workspace, 1, self->workspace.config);
	psy_signal_connect(&self->tabbar.signal_change, self,
		mainframe_ontabbarchanged);
	workspace_addhistory(&self->workspace);
	ui_component_starttimer(&self->component, TIMERID_MAINFRAME, 50);
	psy_signal_connect(&self->workspace.player.eventdrivers.signal_input, self,
		mainframe_oneventdriverinput);
}

void mainframe_setstatusbartext(MainFrame* self, const char* text)
{	
	ui_label_settext(&self->statusbarlabel, text ? text : "");
}

const char* mainframe_statusbaridletext(MainFrame* self)
{	
	return self->workspace.song
		? self->workspace.song->properties.title
		: 0;	
}

void mainframe_initstatusbar(MainFrame* self)
{	
	ui_margin margin;
		
	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2.0),
		ui_value_makepx(0), ui_value_makepx(0));

	ui_component_init(&self->statusbar, &self->component);	
	ui_component_enablealign(&self->statusbar);
	// statusbar label	
	ui_label_init(&self->statusbarlabel, &self->statusbar);
	ui_label_settext(&self->statusbarlabel, "Ready");
	ui_label_setcharnumber(&self->statusbarlabel, 29);
	ui_component_setmargin(&self->statusbarlabel.component, &margin);
	ui_component_setalign(&self->statusbarlabel.component, UI_ALIGN_LEFT);	
	ui_notebook_init(&self->viewbars, &self->statusbar);
	ui_component_setalign(&self->viewbars.component, UI_ALIGN_LEFT);
	ui_component_enablealign(&self->viewbars.component);		
	machineviewbar_init(&self->machineviewbar, &self->viewbars.component, &self->workspace);
	ui_component_setalign(&self->machineviewbar.component, UI_ALIGN_LEFT);
	self->machineview.wireview.statusbar = &self->machineviewbar;
	patternviewbar_init(&self->patternbar, &self->viewbars.component, &self->workspace);
	ui_component_setalign(&self->patternbar.component, UI_ALIGN_LEFT);
	ui_notebook_setpageindex(&self->viewbars, 0);
	ui_notebook_connectcontroller(&self->viewbars, &self->tabbar.signal_change);
	ui_progressbar_init(&self->progressbar, &self->statusbar);
	ui_component_setalign(&self->progressbar.component, UI_ALIGN_RIGHT);	
	psy_signal_connect(&self->workspace.signal_loadprogress, self, 
		mainframe_onsongloadprogress);
	psy_signal_connect(&self->workspace.signal_scanprogress, self, 
		mainframe_onpluginscanprogress);
}

void mainframe_initbars(MainFrame* self)
{
	ui_margin row0margin;
	ui_margin rowmargin;	

	ui_margin_init(&row0margin, ui_value_makeeh(0.5), ui_value_makepx(0),
	ui_value_makeeh(0.5), ui_value_makeeh(0.5));
	ui_margin_init(&rowmargin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makeeh(0.5), ui_value_makeeh(0.5));	
	
	ui_component_init(&self->top, &self->component);	
	ui_component_resize(&self->top, 500, 400);
	ui_component_enablealign(&self->top);	
	// row0
	ui_component_init(&self->toprow0, &self->top);
	ui_component_enablealign(&self->toprow0);	
	ui_component_setalign(&self->toprow0, UI_ALIGN_TOP);
	ui_component_setmargin(&self->toprow0, &row0margin);
	// row1
	ui_component_init(&self->toprow1, &self->top);
	ui_component_enablealign(&self->toprow1);	
	ui_component_setalign(&self->toprow1, UI_ALIGN_TOP);
	ui_component_setmargin(&self->toprow1, &rowmargin);
	// row2
	ui_component_init(&self->toprow2, &self->top);
	ui_component_enablealign(&self->toprow2);	
	ui_component_setalign(&self->toprow2, UI_ALIGN_TOP);
	ui_component_setmargin(&self->toprow2, &rowmargin);
	// add bars to rows
	// row0
	filebar_init(&self->filebar, &self->toprow0, &self->workspace);	
	undoredobar_init(&self->undoredobar, &self->toprow0, &self->workspace);	
	playbar_init(&self->playbar, &self->toprow0, &self->workspace);	
	playposbar_init(&self->playposbar, &self->toprow0, &self->workspace.player);
	{
		ui_margin margin;
		
		ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2.0),
			ui_value_makepx(0), ui_value_makepx(0));

		list_free(ui_components_setalign(
			ui_component_children(&self->toprow0, 0),
			UI_ALIGN_LEFT, &margin));
		margin.right = ui_value_makepx(0);
		ui_component_setmargin(&self->playposbar.component, &margin);
	}
	// row1
	// Songbar	
	InitSongBar(&self->songbar, &self->toprow1, &self->workspace);
	ui_component_setalign(&self->songbar.component, UI_ALIGN_LEFT);	
	// row2
	// Machinebar
	InitMachineBar(&self->machinebar, &self->toprow2, &self->workspace);	
	ui_component_setalign(&self->machinebar.component, UI_ALIGN_LEFT);		
	// Vugroup
	vubar_init(&self->vubar, &self->component, &self->workspace);
	ui_component_resize(&self->vubar.component, 200, 50);	
}

void mainframe_setstartpage(MainFrame* self)
{		
	if (workspace_showaboutatstart(&self->workspace)) {
		tabbar_select(&self->tabbar, TABPAGE_HELPVIEW);
		ui_component_show(&self->helpview.tabbar.component);
	} else {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		ui_component_show(&self->machineview.tabbar.component);
	}
}

void mainframe_initmenu(MainFrame* self)
{
}

void mainframe_destroy(MainFrame* self, ui_component* component)
{
	workspace_save_configuration(&self->workspace);
	workspace_dispose(&self->workspace);
	ui_quit();
}

void mainframe_onalign(MainFrame* self, ui_component* sender)
{
	ui_size size;
	ui_size statusbarsize;
	ui_size tabbarssize;
	ui_size sequenceviewsize;
	ui_size gearsize;
	ui_size plugineditorsize;
	ui_size terminalsize;
	ui_size vusize;
	ui_size topsize;	
	ui_size limit;
	int splitbarwidth = 4;
	ui_textmetric tm;
	
	size = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	statusbarsize = ui_component_preferredsize(&self->statusbar, &size);	
	sequenceviewsize = ui_component_size(&self->sequenceview.component);	
	limit.width = size.width - sequenceviewsize.width - splitbarwidth;
	limit.height = size.height;
	tabbarssize = ui_component_preferredsize(&self->tabbars, &limit);
	tabbarssize.height += (int)(tm.tmHeight * 0.5);
	vusize = ui_component_size(&self->vubar.component);	
	if (self->gear.component.visible) {
		gearsize = ui_component_size(&self->gear.component);		
	} else {
		gearsize.width = 0;
	}
	if (self->plugineditor.component.visible) {
		plugineditorsize = ui_component_size(&self->plugineditor.component);
	} else {
		plugineditorsize.width = 0;
	}
	if (self->terminal.component.visible) {
		terminalsize.height = 
			ui_component_size(&self->terminal.component).height;
	} else {
		terminalsize.height = 0;
	}
	limit.width = size.width - vusize.width;
	limit.height = size.height;
	topsize = ui_component_preferredsize(&self->top, &limit);
	topsize.height += ui_component_textmetric(&self->component).tmHeight;
	ui_component_setposition(&self->top, 0, 0, size.width - vusize.width,
		topsize.height);
	ui_component_setposition(&self->vubar.component, 
		size.width - vusize.width,
		0,
		vusize.width,
		topsize.height);
	ui_component_setposition(&self->tabbars,
		sequenceviewsize.width + splitbarwidth,
		topsize.height,
		size.width - sequenceviewsize.width - splitbarwidth,
		tabbarssize.height);	
	ui_component_setposition(&self->splitbar,
		sequenceviewsize.width,
		topsize.height, splitbarwidth,
		size.height - statusbarsize.height - topsize.height -
			terminalsize.height);
	ui_component_setposition(&self->sequenceview.component,
		0,
		topsize.height,
		sequenceviewsize.width,
		size.height - statusbarsize.height - topsize.height
			- terminalsize.height);	
	ui_component_setposition(&self->notebook.component,
		sequenceviewsize.width + splitbarwidth + plugineditorsize.width,
		topsize.height + tabbarssize.height,
		size.width - sequenceviewsize.width - 3 - gearsize.width
			- plugineditorsize.width,
		size.height - statusbarsize.height - topsize.height -
			tabbarssize.height - terminalsize.height);
	ui_component_setposition(&self->statusbar,
		0,
		size.height - statusbarsize.height,		
		size.width,
		statusbarsize.height);
	if (ui_component_visible(&self->plugineditor.component)) {
		ui_component_setposition(&self->plugineditor.component,
			sequenceviewsize.width + splitbarwidth,
			topsize.height + tabbarssize.height,
			plugineditorsize.width,
			size.height - statusbarsize.height - topsize.height -
				tabbarssize.height - terminalsize.height);
	}
	if (ui_component_visible(&self->gear.component)) {
		ui_component_setposition(&self->gear.component,
			size.width - gearsize.width,
			topsize.height + tabbarssize.height,
			gearsize.width,
			size.height - statusbarsize.height - topsize.height -
				tabbarssize.height - terminalsize.height);
	}
	if (ui_component_visible(&self->terminal.component)) {
		ui_component_setposition(&self->terminal.component,
			0,
			size.height - terminalsize.height - statusbarsize.height,
			size.width,
			terminalsize.height);
	}
	if (self->firstshow) {
		machineview_align(&self->machineview);
		self->firstshow = 0;
	}	
}

void mainframe_oneventdriverinput(MainFrame* self, EventDriver* sender)
{
	int cmd;
	// 	Properties* section;

	// section = properties_find(sender->properties, "general");
	cmd = sender->getcmd(sender, 0);
	if (cmd == CMD_IMM_HELP) {
		tabbar_select(&self->helpview.tabbar, 0);
		tabbar_select(&self->tabbar, TABPAGE_HELPVIEW);		
	} else
	if (cmd == CMD_IMM_EDITMACHINE) {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	} else
	if (cmd == CMD_IMM_EDITPATTERN) {
		tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
	} else
	if (cmd == CMD_IMM_ADDMACHINE) {
		self->machineview.newmachine.pluginsview.calledby = self->tabbar.selected;
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		tabbar_select(&self->machineview.tabbar, 1);
	} else
	if (cmd == CMD_IMM_PLAYSONG) {
		player_setposition(&self->workspace.player, 0);
		player_start(&self->workspace.player);
	} else
	if (cmd == CMD_IMM_PLAYSTART) {
		SequenceEntry* entry;

		entry = sequenceposition_entry(&self->workspace.sequenceselection.editposition);		
		player_setposition(&self->workspace.player, entry ? entry->offset : 0);		
		player_start(&self->workspace.player);		
	} else
	if (cmd == CMD_IMM_PLAYFROMPOS) {
		psy_dsp_beat_t playposition = 0;
		SequenceEntry* entry;

		entry = sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) + 
			(psy_dsp_beat_t) self->workspace.patterneditposition.offset;
		player_setposition(&self->workspace.player, playposition);
		player_start(&self->workspace.player);
	} else
	if (cmd == CMD_IMM_PLAYSTOP) {
		player_stop(&self->workspace.player);
	} else
	if (cmd == CMD_IMM_SONGPOSDEC) {
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
	if (cmd == CMD_IMM_SONGPOSINC) {
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
	if (cmd == CMD_IMM_MAXPATTERN) {
		mainframe_maximizeorminimizeview(self);
	} else
	if (cmd == CMD_IMM_INFOMACHINE) {
		workspace_showparameters(&self->workspace,
			machines_slot(&self->workspace.song->machines));
	} else
	if (cmd == CMD_IMM_EDITINSTR) {
		workspace_selectview(&self->workspace, TABPAGE_INSTRUMENTSVIEW);
	} else
	if (cmd == CMD_IMM_EDITSAMPLE) {
		workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW);
		tabbar_select(&self->samplesview.clienttabbar, 0);
	} else
	if (cmd == CMD_IMM_EDITWAVE) {
		workspace_selectview(&self->workspace, TABPAGE_SAMPLESVIEW);
		tabbar_select(&self->samplesview.clienttabbar, 2);
	} else
	if (cmd == CMD_IMM_TERMINAL) {
		if (ui_component_visible(&self->terminal.component)) {		
		ui_component_hide(&self->terminal.component);
		ui_component_align(&self->component);
	} else {								
		ui_component_show(&self->terminal.component);
		ui_component_align(&self->component);		
	}
	}
}



void mainframe_onkeydown(MainFrame* self, ui_component* sender, KeyEvent* ev)
{	
	if (ev->keycode != VK_CONTROL && ev->keycode != VK_SHIFT) {
		EventDriver* kbd;
		int input;			
			
		kbd = workspace_kbddriver(&self->workspace);
		input = encodeinput(ev->keycode, ev->shift, ev->ctrl);
		kbd->write(kbd, EVENTDRIVER_KEYDOWN, (unsigned char*)&input, 4);			
	}
}

void mainframe_onkeyup(MainFrame* self, ui_component* component, KeyEvent* keyevent)
{
	if (keyevent->keycode != VK_CONTROL &&
			keyevent->keycode != VK_SHIFT) {
		EventDriver* kbd;
		int input;			
		
		input = encodeinput(keyevent->keycode, GetKeyState(VK_SHIFT) < 0,
			GetKeyState(VK_CONTROL) < 0);						
		kbd = workspace_kbddriver(&self->workspace);
		kbd->write(kbd,
			EVENTDRIVER_KEYUP,			
			(unsigned char*)&input,
			4);
	}
}

void mainframe_maximizeorminimizeview(MainFrame* self)
{
	if (self->workspace.maximizeview.maximized) {
		self->workspace.maximizeview.maximized = 0;
		if (self->workspace.maximizeview.row0) {
			ui_component_show(&self->toprow0);
		}
		if (self->workspace.maximizeview.row1) {
			ui_component_show(&self->toprow1);
		}
		if (self->workspace.maximizeview.row2) {
			ui_component_show(&self->toprow2);
		}
		ui_component_resize(&self->sequenceview.component, 
			self->workspace.maximizeview.sequenceviewrestorewidth, 0);
	} else {
		self->workspace.maximizeview.maximized = 1;
		self->workspace.maximizeview.row0 = self->toprow0.visible;
		self->workspace.maximizeview.row1 = self->toprow1.visible;
		self->workspace.maximizeview.row2 = self->toprow2.visible;
		self->workspace.maximizeview.sequenceviewrestorewidth =
			ui_component_size(&self->sequenceview.component).width;
		ui_component_hide(&self->toprow0);
		ui_component_hide(&self->toprow1);
		ui_component_resize(&self->sequenceview.component, 0, 0);			
	}
	ui_component_align(&self->component);
}

void mainframe_onsongloadprogress(MainFrame* self, Workspace* workspace, int progress)
{
	ui_progressbar_setprogress(&self->progressbar, progress / 100.f);
}

void mainframe_onpluginscanprogress(MainFrame* self, Workspace* workspace, int progress)
{	
	if (progress == 0) {
		ui_progressbar_setprogress(&self->progressbar, 0);
	} else {
		ui_progressbar_tick(&self->progressbar);
	}
}

void mainframe_onsongchanged(MainFrame* self, ui_component* sender, int flag)
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
	ui_component_invalidate(&self->component);	
}

void mainframe_updatetitle(MainFrame* self)
{	
	char txt[512];
	char prefix[4096];
	char name[4096];
	char ext[4096];

	extract_path(self->workspace.filename, prefix, name, ext);
	psy_snprintf(txt, 512, "[%s.%s]  Psycle Modular Music Creation Studio ",
		name, ext);			
	ui_component_settitle(&self->component, txt);
}

void mainframe_onmousedown(MainFrame* self, ui_component* sender, MouseEvent* ev)
{	
	ui_component_capture(sender);
	self->resize = 1;
	SetCursor(LoadCursor(NULL, IDC_SIZEWE));
}

void mainframe_onmousemove(MainFrame* self, ui_component* sender, MouseEvent* ev)
{
	if (self->resize == 1) {		
		ui_size toolbarsize;
		ui_rectangle position;
	
		toolbarsize = ui_component_size(&self->top);
		position = ui_component_position(sender);
		ui_component_move(sender, position.left + ev->x, toolbarsize.height);
		ui_component_invalidate(sender);
		ui_component_update(sender);		
	}
	SetCursor(LoadCursor(NULL, IDC_SIZEWE));
}

void mainframe_onmouseup(MainFrame* self, ui_component* sender, MouseEvent* ev)
{			
	ui_rectangle position;
	
	ui_component_releasecapture();
	self->resize = 0;
	position = ui_component_position(sender);
	ui_component_resize(&self->sequenceview.component,
		position.left, ui_component_size(&self->sequenceview.component).height);	
	ui_component_align(&self->component);
	SetCursor(LoadCursor(NULL, IDC_SIZEWE));
}

void mainframe_onmouseentersplitbar(MainFrame* self, ui_component* sender)
{	
	ui_component_setbackgroundcolor(sender, 0x00666666);
	ui_component_invalidate(sender);
	SetCursor(LoadCursor(NULL, IDC_SIZEWE));
}

void mainframe_onmouseleavesplitbar(MainFrame* self, ui_component* sender)
{			
	ui_component_setbackgroundcolor(sender, 0x00232323);
	ui_component_invalidate(sender);
}

void mainframe_ongear(MainFrame* self, ui_component* sender)
{
	if (ui_component_visible(&self->gear.component)) {
		ui_button_disablehighlight(&self->machinebar.gear);
		ui_component_hide(&self->gear.component);
		ui_component_align(&self->component);
	} else {						
		ui_button_highlight(&self->machinebar.gear);
		ui_component_show(&self->gear.component);
		ui_component_align(&self->component);		
	}	
}

void mainframe_onplugineditor(MainFrame* self, ui_component* sender)
{
	if (ui_component_visible(&self->plugineditor.component)) {
		ui_button_disablehighlight(&self->machinebar.editor);
		ui_component_hide(&self->plugineditor.component);
		ui_component_align(&self->component);
	} else {						
		ui_button_highlight(&self->machinebar.editor);
		ui_component_show(&self->plugineditor.component);
		ui_component_align(&self->component);		
	}	
}

void mainframe_onaboutok(MainFrame* self, ui_component* sender)
{
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
}

void mainframe_ongearcreate(MainFrame* self, ui_component* sender)
{
	self->machineview.newmachine.pluginsview.calledby = GEARVIEW;
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	tabbar_select(&self->machineview.tabbar, 1);
}

void mainframe_onsettingsviewchanged(MainFrame* self, SettingsView* sender,
	Properties* property)
{
	workspace_configchanged(&self->workspace, property, sender->choiceproperty);	
}

int mainframe_showmaximizedatstart(MainFrame* self)
{
	return workspace_showmaximizedatstart(&self->workspace);
}

void mainframe_onrender(MainFrame* self, ui_component* sender)
{
	ui_notebook_setpageindex(&self->notebook, TABPAGE_RENDERVIEW);
}

void mainframe_ontimer(MainFrame* self, ui_component* sender, int timerid)
{
	workspace_idle(&self->workspace);
}

void mainframe_onviewselected(MainFrame* self, Workspace* sender, int view)
{
	if (view != GEARVIEW) {
		tabbar_select(&self->tabbar, view);
	}
}

void mainframe_ontabbarchanged(MainFrame* self, ui_component* sender,
	uintptr_t tabindex)
{
	ui_component* component;
	workspace_onviewchanged(&self->workspace, tabindex);
	component = ui_notebook_activepage(&self->notebook);
	ui_component_align(&self->component);
	ui_component_setfocus(component);
}
