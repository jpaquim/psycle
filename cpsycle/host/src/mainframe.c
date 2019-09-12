// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "mainframe.h"
#include "uistatusbar.h"
#include <uimenu.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uilabel.h>
#include "settingsview.h"
#include "songproperties.h"
#include "cmdsnotes.h"
#include "skinio.h"

static void Create(MainFrame* self);
static void InitMenu(MainFrame* self);
static void OnFileMenu(ui_menu* menu);
static void OnDraw(MainFrame* self, ui_component* sender, ui_graphics* g);
static void Destroy(MainFrame* self, ui_component* component);
static void OnSize(MainFrame* self, ui_component* sender, int width, int height);
static void OnKeyDown(MainFrame* self, ui_component* component, int keycode, int keydata);
static void OnTimer(MainFrame* self, ui_component* sender, int timerid);
static void OnSequenceSelChange(MainFrame* self, SequenceEntry* entry);
static void OnPlay(MainFrame* self, ui_component* sender);
static void OnStop(MainFrame* self, ui_component* sender);
static void OnLoadSong(MainFrame* self, ui_component* sender);
static void OnNewSong(MainFrame* self, ui_component* sender);

static ui_statusbar statusbar;
static ui_menu menu;    
static ui_menu menu_file;
static ui_edit edit;
static ui_button button;
static ui_label label;
static SongProperties songproperties;

static int toolbarheight = 75;
static int tabbarheight = 20;

void InitMainFrame(MainFrame* self)
{	
	int iStatusWidths[] = {100, 200, -1};	
	
	workspace_init(&self->workspace);	
	self->player = &self->workspace.player;
	self->firstshow = 1;
	ui_frame_init(&self->component, 0);
	// ui_component_resize(&self->component, 800, 600);
	ui_component_settitle(&self->component, "Psycle");
	signal_connect(&self->component.signal_destroy, self, Destroy);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_timer, self, OnTimer);	
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	ui_button_init(&self->newsongbutton, &self->component);
	ui_button_settext(&self->newsongbutton, "New Song");
	ui_component_setposition(&self->newsongbutton.component, 0, 0, 100, 20);	
	signal_connect(&self->newsongbutton.signal_clicked, self, OnNewSong);
	ui_button_init(&self->loadsongbutton, &self->component);
	ui_button_settext(&self->loadsongbutton, "Load Song");
	ui_component_setposition(&self->loadsongbutton.component, 105, 0, 100, 20);	
	signal_connect(&self->loadsongbutton.signal_clicked, self, OnLoadSong);
	InitMachineBar(&self->machinebar, &self->component, &self->workspace);
	ui_component_move(&self->machinebar.component, 3, 50);
	ui_component_resize(&self->machinebar.component, 410, 25);
	InitTabBar(&self->tabbar, &self->component);
	ui_component_move(&self->tabbar.component, 150, toolbarheight);
	ui_component_resize(&self->tabbar.component, 430, 20);	
	tabbar_append(&self->tabbar, "Machines");
	tabbar_append(&self->tabbar, "Pattern");	
	tabbar_append(&self->tabbar, "Samples");
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Settings");
	tabbar_select(&self->tabbar, 0);	
	InitPlayBar(&self->playbar, &self->component);
	ui_component_move(&self->playbar.component, 320, 2);
	ui_component_resize(&self->playbar.component, 160, 25);	
	signal_connect(&self->playbar.signal_play, self, OnPlay);
	signal_connect(&self->playbar.signal_stop, self, OnStop);
	InitVumeter(&self->vumeter, &self->component, &self->workspace.player);
	ui_component_move(&self->vumeter.component, 520, 0);
	ui_component_resize(&self->vumeter.component, 200, 20);	
	InitTimeBar(&self->timebar, &self->component, &self->workspace.player);
	ui_component_move(&self->timebar.component, 0, 25);
	ui_component_resize(&self->timebar.component, 250, 20);	
	InitLinesPerBeatBar(&self->linesperbeatbar, &self->component, &self->workspace.player);
	ui_component_move(&self->linesperbeatbar.component, 220, 25);
	ui_component_resize(&self->linesperbeatbar.component, 250, 20);	
	ui_notebook_init(&self->notebook, &self->component);
	ui_component_move(&self->notebook.component, 150, toolbarheight + tabbarheight);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	/// init notebook views
	InitMachineView(&self->machineview, &self->notebook.component, &self->component, &self->workspace);
	InitPatternView(&self->patternview, &self->notebook.component, &self->component, &self->workspace);	
	self->patternview.trackerview.grid.noteinputs = &self->noteinputs;		
	InitSamplesView(&self->samplesview, &self->notebook.component, &self->component, &self->workspace);	
	InitInstrumentsView(&self->instrumentsview, &self->notebook.component,
		&self->component, &self->workspace);
	self->instrumentsview.sampulseview.notemapedit.noteinputs = &self->noteinputs;	
	InitSettingsView(&self->settingsview, &self->notebook.component,
		self->workspace.config);	
	greet_init(&self->greet, &self->component);
	ui_component_hide(&self->greet.component);	
	InitNoteInputs(&self->noteinputs);	
	InitSequenceView(&self->sequenceview, &self->component, &self->workspace);	
	ui_component_move(&self->sequenceview.component, 0, toolbarheight);
	ui_notebook_setpage(&self->notebook, 0);
	/*InitSongProperties(&songproperties, &self->component);	
	ui_menu_init(&menu, "", 0);
	ui_menu_init(&menu_file, "File", OnFileMenu);		
	ui_menu_append(&menu, &menu_file, 0);
	ui_component_setmenu(&self->component, &menu);
	*/	
	ui_statusbar_init(&statusbar, &self->component);
	ui_statusbar_setfields(&statusbar, 3, iStatusWidths);
	ui_statusbar_settext(&statusbar, 0, "Field1");
	ui_statusbar_settext(&statusbar, 1, "Field2");
	ui_statusbar_settext(&statusbar, 2, "Field3");	

	SetTimer(self->component.hwnd, 100, 50, 0);	
}

void InitMenu(MainFrame* self)
{
}

void OnDraw(MainFrame* self, ui_component* sender, ui_graphics* g)
{
	ui_size size = ui_component_size(&self->component);    
	ui_rectangle r;
    ui_setrectangle(&r, 0, 0, size.width, size.height);
    ui_drawsolidrectangle(g, r, 0xFFFFFFFF);
}

void Destroy(MainFrame* self, ui_component* component)
{
	workspace_dispose(&self->workspace);
	PostQuitMessage (0) ;
}

void OnSize(MainFrame* self, ui_component* sender, int width, int height)
{
	int statusbarheight = 30;
	ui_component_resize(&self->sequenceview.component, 100, height - statusbarheight - toolbarheight);
	ui_component_resize(&self->notebook.component, width - 150, height - statusbarheight - toolbarheight - tabbarheight);
	SendMessage(statusbar.component.hwnd, WM_SIZE, 0, 0);
	if (self->firstshow) {
		machineview_align(&self->machineview);
		self->firstshow = 0;
	}
}

void OnFileMenu(ui_menu* menu)
{
   PostQuitMessage(0);  
}

void OnKeyDown(MainFrame* self, ui_component* component, int keycode, int keydata)
{	
	if (keycode == VK_F2) {
//		SetActiveView(self, &self->machineview.component);
	} else
	if (keycode == VK_F5) {
		player_start(self->player);		
	} else
	if (keycode == VK_F3) {
//		SetActiveView(self, &self->patternview.component);		
	} else	 
	if (keycode == VK_F6) {
//		SetActiveView(self, &self->settingsview.component);		
	} else
	if (keycode == VK_F8) {
		player_stop(self->player);		
	} else
	if (keycode == VK_F9) {
		OnLoadSong(self, &self->component);		
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
	ui_statusbar_settext(&statusbar, 0, buffer);	
}

void OnPlay(MainFrame* self, ui_component* sender)
{	
	player_start(self->player);		
}

void OnStop(MainFrame* self, ui_component* sender)
{
	player_stop(self->player);		
}

void OnNewSong(MainFrame* self, ui_component* sender)
{
	workspace_newsong(&self->workspace);
}

void OnLoadSong(MainFrame* self, ui_component* sender)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 					
	static char filter[] = "All Songs (*.psy *.xm *.it *.s3m *.mod)" "\0*.psy;*.xm;*.it;*.s3m;*.mod\0"
				"Songs (*.psy)"				        "\0*.psy\0"
				"FastTracker II Songs (*.xm)"       "\0*.xm\0"
				"Impulse Tracker Songs (*.it)"      "\0*.it\0"
				"Scream Tracker Songs (*.s3m)"      "\0*.s3m\0"
				"Original Mod Format Songs (*.mod)" "\0*.mod\0";
	char  defaultextension[] = "PSY"; 
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, path)) {
		workspace_loadsong(&self->workspace, path);
	}
}
