// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "mainframe.h"
#include <uimenu.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uilabel.h>
#include "settingsview.h"
#include "cmdsnotes.h"
#include "skinio.h"
#include "ui_app.h"

static void Create(MainFrame*);
static void InitMenu(MainFrame*);
static void InitBars(MainFrame*);
static void InitVuBar(MainFrame*);
static void Destroy(MainFrame*, ui_component* component);
static void OnKeyDown(MainFrame*, ui_component* component, int keycode, int keydata);
static void OnTimer(MainFrame*, ui_component* sender, int timerid);
static void OnSequenceSelChange(MainFrame* , SequenceEntry* entry);
static void OnPlay(MainFrame*, ui_component* sender);
static void OnStop(MainFrame*, ui_component* sender);
static void OnAlign(MainFrame*, ui_component* sender);
static void OnMouseDown(MainFrame*, ui_component* sender, int x, int y, int button);
static void OnMouseMove(MainFrame*, ui_component* sender, int x, int y, int button);
static void OnMouseUp(MainFrame*, ui_component* sender, int x, int y, int button);
static void OnGear(MainFrame*, ui_component* sender);
static void OnGearCreate(MainFrame*, ui_component* sender);
static void OnAboutOk(MainFrame*, ui_component* sender);
static void OnUpdateDriver(MainFrame*, ui_component* sender);
static void SetStartPage(MainFrame*);
static void OnSettingsViewChanged(MainFrame*, SettingsView* sender, Properties*);
static void OnMouseEnterSplitBar(MainFrame*, ui_component* sender);
static void OnMouseLeaveSplitBar(MainFrame*, ui_component* sender);
static void OnSongChanged(MainFrame*, ui_component* sender, int flag);

HWND hwndmain;

enum {
	TABPAGE_MACHINEVIEW		= 0,
	TABPAGE_PATTERNVIEW		= 1,
	TABPAGE_SAMPLESVIEW		= 2,
	TABPAGE_INSTRUMENTSVIEW = 3,
	TABPAGE_PROPERTIESVIEW	= 4,
	TABPAGE_SETTINGSVIEW	= 5,
	TABPAGE_HELPVIEW		= 6
};

void InitMainFrame(MainFrame* self)
{	
	int statuswidths[] = { 100, 200, -1 };	
	ui_margin tabbardividemargin = { 0, 30, 0, 0};

	ui_frame_init(&self->component, 0);			
	ui_component_enablealign(&self->component);	
	self->resize = 0;
	workspace_init(&self->workspace);
	self->workspace.mainhandle = &self->component;
	workspace_initplayer(&self->workspace);
	workspace_load_configuration(&self->workspace);
	signal_connect(&self->workspace.signal_songchanged, self, OnSongChanged);	
	self->firstshow = 1;	
	InitBars(self);	
	// ui_component_init(&self->client, &self->component);	
	// ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);		
	// ui_component_resize(&self->component, 800, 600);
	ui_component_settitle(&self->component, "Psycle");
	signal_connect(&self->component.signal_destroy, self, Destroy);
	signal_connect(&self->component.signal_align, self, OnAlign);
	signal_connect(&self->component.signal_timer, self, OnTimer);	
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	signal_connect(&self->component.signal_align, self, OnAlign);
	ui_component_init(&self->tabbars, &self->component);
	ui_component_setbackgroundmode(&self->tabbars, BACKGROUND_SET);
	InitTabBar(&self->tabbar, &self->tabbars);	
	ui_component_resize(&self->tabbar.component,
		430, ui_component_size(&self->tabbar.component).height);	
	tabbar_append(&self->tabbar, "Machines");
	tabbar_append(&self->tabbar, "Pattern");	
	tabbar_append(&self->tabbar, "Samples");
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Properties");
	tabbar_append(&self->tabbar, "Settings");
	tabbar_append(&self->tabbar, "Help");	
	tabbar_settabmargin(&self->tabbar, 4, &tabbardividemargin);	
	// splitbar
	ui_component_init(&self->splitbar, &self->component);
	ui_component_setbackgroundmode(&self->splitbar, BACKGROUND_SET);
	signal_connect(&self->splitbar.signal_mouseenter, self, OnMouseEnterSplitBar);
	signal_connect(&self->splitbar.signal_mouseleave, self, OnMouseLeaveSplitBar);
	/// init notebook views
	ui_notebook_init(&self->notebook, &self->component);	
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	InitMachineView(&self->machineview, &self->notebook.component, &self->tabbars, &self->workspace);
	InitPatternView(&self->patternview, &self->notebook.component, &self->tabbars, &self->workspace);	
	self->patternview.trackerview.grid.noteinputs = &self->noteinputs;		
	InitSamplesView(&self->samplesview, &self->notebook.component, &self->tabbars, &self->workspace);	
	InitInstrumentsView(&self->instrumentsview, &self->notebook.component,
		&self->tabbars, &self->workspace);
	self->instrumentsview.sampulseview.notemapedit.noteinputs = &self->noteinputs;	
	InitSongProperties(&self->songproperties, &self->notebook.component, &self->workspace);	
	InitSettingsView(&self->settingsview, &self->notebook.component,
		&self->tabbars, self->workspace.config);	
	signal_connect(&self->settingsview.signal_changed, self, OnSettingsViewChanged);
	InitHelpView(&self->helpview, &self->notebook.component, &self->tabbars,
		&self->workspace);	
	signal_connect(&self->helpview.about.okbutton.signal_clicked, self, OnAboutOk);
	InitNoteInputs(&self->noteinputs);	
	InitSequenceView(&self->sequenceview, &self->component, &self->workspace);	
	InitGear(&self->gear, &self->component, &self->workspace);	
	ui_component_resize(&self->gear.component, 300, 200);
	ui_component_hide(&self->gear.component);
	signal_connect(&self->machinebar.gear.signal_clicked, self, OnGear);	
	signal_connect(&self->gear.buttons.createreplace.signal_clicked, self, OnGearCreate);	
		
	ui_statusbar_init(&self->statusbar, &self->component);
	ui_statusbar_setfields(&self->statusbar, 3, statuswidths);
	ui_statusbar_settext(&self->statusbar, 0, "");
	ui_statusbar_settext(&self->statusbar, 1, "");
	ui_statusbar_settext(&self->statusbar, 2, "");

	signal_connect(&self->splitbar.signal_mousedown, self, OnMouseDown);	
	signal_connect(&self->splitbar.signal_mousemove, self, OnMouseMove);	
	signal_connect(&self->splitbar.signal_mouseup, self, OnMouseUp);
	
	SetTimer(self->component.hwnd, 100, 50, 0);	
	SetStartPage(self);
	if (self->workspace.song->properties) {
		Properties* title;
		title = properties_find(self->workspace.song->properties, "title");
		if (title) {						
			ui_statusbar_settext(&self->statusbar, 0, title->item.value.s);
		}
	}
}

void InitBars(MainFrame* self)
{
	ui_component_init(&self->top, &self->component);
	ui_component_setbackgroundmode(&self->top, BACKGROUND_SET);
	ui_component_resize(&self->top, 500, 400);
	ui_component_enablealign(&self->top);	
	// row0
	ui_component_init(&self->toprow0, &self->top);
	ui_component_enablealign(&self->toprow0);
	ui_component_setbackgroundmode(&self->toprow0, BACKGROUND_SET);	
	ui_component_setalign(&self->toprow0, UI_ALIGN_TOP);	
	// row1
	ui_component_init(&self->toprow1, &self->top);
	ui_component_enablealign(&self->toprow1);
	ui_component_setbackgroundmode(&self->toprow1, BACKGROUND_SET);	
	ui_component_setalign(&self->toprow1, UI_ALIGN_TOP);	
	// row2
	ui_component_init(&self->toprow2, &self->top);
	ui_component_enablealign(&self->toprow2);
	ui_component_setbackgroundmode(&self->toprow2, BACKGROUND_SET);
	ui_component_setalign(&self->toprow2, UI_ALIGN_TOP);	
	// add bars to rows
	// row0
	InitFileBar(&self->filebar, &self->toprow0, &self->workspace);
	ui_component_setalign(&self->filebar.component, UI_ALIGN_LEFT);
	InitUndoRedoBar(&self->undoredobar, &self->toprow0, &self->workspace);
	ui_component_setalign(&self->undoredobar.component, UI_ALIGN_LEFT);
	InitPlayBar(&self->playbar, &self->toprow0);
	ui_component_setalign(&self->playbar.component, UI_ALIGN_LEFT);	
	signal_connect(&self->playbar.signal_play, self, OnPlay);
	signal_connect(&self->playbar.signal_stop, self, OnStop);
	// row1
	// Songbar	
	InitSongBar(&self->songbar, &self->toprow1, &self->workspace);
	ui_component_setalign(&self->songbar.component, UI_ALIGN_LEFT);	
	// row2
	// Machinebar
	InitMachineBar(&self->machinebar, &self->toprow2, &self->workspace);	
	ui_component_setalign(&self->machinebar.component, UI_ALIGN_LEFT);		
	// Vugroup
	InitVuBar(self);
	ui_component_resize(&self->vubar, 200, 50);
	// ui_button_init(&self->updatedriver, &self->top);
	// ui_button_settext(&self->updatedriver, "Restart Driver");
	// ui_component_setposition(&self->updatedriver.component, 210, 0, 100, 20);	
	// signal_connect(&self->updatedriver.signal_clicked, self, OnUpdateDriver);	
}

void InitVuBar(MainFrame* self)
{
	ui_component_init(&self->vubar, &self->component);
	ui_component_setbackgroundmode(&self->vubar, BACKGROUND_SET);
	InitVumeter(&self->vumeter, &self->vubar, &self->workspace);
	ui_component_setposition(&self->vumeter.component, 0, 0, 200, 20);	
	InitVolSlider(&self->volslider, &self->vubar, &self->workspace);
	ui_component_setposition(&self->volslider.component, 0, 20, 200, 20);	
	InitClipBox(&self->clipbox, &self->vubar, &self->workspace);
	ui_component_setposition(&self->clipbox.component, 205, 5, 10, 35);
}

void SetStartPage(MainFrame* self)
{		
	if (workspace_showaboutatstart(&self->workspace)) {
		tabbar_select(&self->tabbar, TABPAGE_HELPVIEW);
		ui_component_show(&self->helpview.tabbar.component);
	} else {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		ui_component_show(&self->machineview.tabbar.component);
	}
}

void InitMenu(MainFrame* self)
{
}

void Destroy(MainFrame* self, ui_component* component)
{
	workspace_save_configuration(&self->workspace);
	workspace_dispose(&self->workspace);
	ui_quit();
}

void OnAlign(MainFrame* self, ui_component* sender)
{
	ui_size size;
	ui_size statusbarsize;
	ui_size tabbarsize;
	ui_size sequenceviewsize;
	ui_size gearsize;
	ui_size vusize;
	int tmp;
	int splitbarwidth = 4;
	int toolbarheight;	
	ui_size limit;
	
	size = ui_component_size(&self->component);
	statusbarsize = ui_component_size(&self->statusbar.component);
	limit.width = size.width;
	limit.height = size.height;
	signal_emit(&self->tabbar.component.signal_preferredsize, &self->tabbar,
		3, &limit, &tabbarsize.width, &tabbarsize.height);					 	
	sequenceviewsize = ui_component_size(&self->sequenceview.component);
	vusize = ui_component_size(&self->vubar);
	
	if (self->gear.component.visible) {
		gearsize = ui_component_size(&self->gear.component);		
	} else {
		gearsize.width = 0;
	}		
	limit.width = size.width - vusize.width;
	limit.height = size.height;
	signal_emit(&self->top.signal_preferredsize, &self->top, 3, &limit, &tmp,
		&toolbarheight);		
	ui_component_setposition(&self->top, 0, 0, size.width - vusize.width,
		toolbarheight);
	ui_component_setposition(&self->vubar, 
		size.width - vusize.width,
		0,
		vusize.width,
		toolbarheight);
	ui_component_setposition(&self->tabbars,
		sequenceviewsize.width + splitbarwidth,
		toolbarheight,
		size.width - sequenceviewsize.width - splitbarwidth,
		tabbarsize.height);
	ui_component_resize(&self->tabbar.component, tabbarsize.width, tabbarsize.height);		
	ui_component_setposition(&self->splitbar,
		sequenceviewsize.width,
		toolbarheight, splitbarwidth,
		size.height - statusbarsize.height - toolbarheight);
	ui_component_setposition(&self->sequenceview.component,
		0,
		toolbarheight,
		sequenceviewsize.width,
		size.height - statusbarsize.height - toolbarheight);	
	ui_component_setposition(&self->notebook.component,
		sequenceviewsize.width + splitbarwidth,
		toolbarheight + tabbarsize.height,
		size.width - sequenceviewsize.width - 3 - gearsize.width,
		size.height - statusbarsize.height - toolbarheight - tabbarsize.height);
	ui_component_resize(&self->statusbar.component, 0, 0);
	if (ui_component_visible(&self->gear.component)) {
		ui_component_move(&self->gear.component,
			size.width - gearsize.width, toolbarheight + tabbarsize.height);
		ui_component_resize(&self->gear.component, gearsize.width,
			size.height - statusbarsize.height - toolbarheight - tabbarsize.height);
	}
	if (self->firstshow) {
		machineview_align(&self->machineview);
		self->firstshow = 0;
	}
}

void OnKeyDown(MainFrame* self, ui_component* component, int keycode, int keydata)
{	
	if (keycode == VK_F2) {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	} else
	if (keycode == VK_F5) {
		player_start(&self->workspace.player);		
	} else
	if (keycode == VK_F3) {
		tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
	} else	 
	if (keycode == VK_F6) {
		tabbar_select(&self->tabbar, TABPAGE_SETTINGSVIEW);
	} else
	if (keycode == VK_F8) {
		player_stop(&self->workspace.player);		
	} else	
	if (keycode == VK_F4) {
		Properties* properties = properties_create();
		properties_init(properties);
		skin_load(properties, "old Psycle.psv");
		TrackerViewApplyProperties(&self->patternview.trackerview, properties);
		MachineViewApplyProperties(&self->machineview, properties);
		properties_free(properties);
	} else {
		int cmd;		
		cmd = Cmd(&self->noteinputs.map, keycode);
		if (cmd != -1) {
			Machine* machine;
			int base;
			base = 48;
			machine = machines_at(&self->workspace.song->machines,
				self->workspace.song->machines.slot);
			if (machine) {				
				PatternEvent event = { cmd + base, 0, 0, 0, 0 };
				machine->seqtick(machine, 0, &event);
			}
		}
	}		
}

void OnTimer(MainFrame* self, ui_component* sender, int timerid)
{
	char buffer[20];

	_snprintf(buffer, 20, "%.4f", player_position(self->patternview.trackerview.grid.player)); 
	ui_statusbar_settext(&self->statusbar, 1, buffer);	
}

void OnPlay(MainFrame* self, ui_component* sender)
{	
	player_start(&self->workspace.player);		
}

void OnSongChanged(MainFrame* self, ui_component* sender, int flag)
{	
	if (flag == WORKSPACE_LOADSONG) {
		if (workspace_showsonginfoonload(&self->workspace)) {
			tabbar_select(&self->tabbar, TABPAGE_PROPERTIESVIEW);
		}
		if (self->workspace.song->properties) {
			Properties* title;
			title = properties_find(self->workspace.song->properties, "title");
			if (title) {
				char* titlestr = 0;
				properties_readstring(title, "title", &titlestr, "Untitled");
				ui_statusbar_settext(&self->statusbar, 0, titlestr);
			}
		}
	}	 
	ui_invalidate(&self->component);	
}

void OnStop(MainFrame* self, ui_component* sender)
{
	player_stop(&self->workspace.player);		
}

void OnMouseDown(MainFrame* self, ui_component* sender, int x, int y, int button)
{	
	ui_component_capture(sender);
	self->resize = 1;	
}

void OnMouseMove(MainFrame* self, ui_component* sender, int x, int y, int button)
{
	if (self->resize == 1) {		
		RECT rc;
		POINT pt;
		ui_size toolbarsize;
	
		toolbarsize = ui_component_size(&self->top);
		GetWindowRect(sender->hwnd, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient(GetParent(sender->hwnd), &pt);
		ui_component_move(sender, pt.x + x, toolbarsize.height);
		ui_invalidate(sender);
		UpdateWindow(sender->hwnd);
	}
}

void OnMouseUp(MainFrame* self, ui_component* sender, int x, int y, int button)
{		
	RECT rc;
	POINT pt;
	
	ui_component_releasecapture();
	self->resize = 0;
	GetWindowRect(sender->hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(sender->hwnd), &pt);
	ui_component_resize(&self->sequenceview.component,
		pt.x, ui_component_size(&self->sequenceview.component).height);	
	ui_component_align(&self->component);
	ui_invalidate(&self->tabbar.component);
}

void OnMouseEnterSplitBar(MainFrame* self, ui_component* sender)
{	
	ui_component_setbackgroundcolor(sender, 0x00666666);
	ui_invalidate(sender);
}

void OnMouseLeaveSplitBar(MainFrame* self, ui_component* sender)
{			
	ui_component_setbackgroundcolor(sender, 0x00232323);
	ui_invalidate(sender);
}

void OnGear(MainFrame* self, ui_component* sender)
{
	if (ui_component_visible(&self->gear.component)) {
		ui_component_hide(&self->gear.component);
		ui_component_align(&self->component);		
	} else {						
		ui_size size;
		ui_size gearsize;

		size = ui_component_size(&self->component);
		gearsize = ui_component_size(&self->gear.component);
		ui_component_move(&self->gear.component, size.width - gearsize.width, gearsize.height);
		ui_component_show(&self->gear.component);
		ui_component_align(&self->component);
	}	
	
}

void OnAboutOk(MainFrame* self, ui_component* sender)
{
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
}

void OnGearCreate(MainFrame* self, ui_component* sender)
{
	self->machineview.newmachine.calledbygear = 1;
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	tabbar_select(&self->machineview.tabbar, 1);
}

void OnSettingsViewChanged(MainFrame* self, SettingsView* sender, Properties* property)
{
	workspace_configchanged(&self->workspace, property, sender->choiceproperty);	
}

void OnUpdateDriver(MainFrame* self, ui_component* sender)
{
	workspace_updatedriver(&self->workspace);
}
