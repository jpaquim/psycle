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
static void InitBars(MainFrame*, ui_component* parent);
static void Destroy(MainFrame*, ui_component* component);
static void OnSize(MainFrame*, ui_component* sender, int width, int height);
static void OnKeyDown(MainFrame*, ui_component* component, int keycode, int keydata);
static void OnTimer(MainFrame*, ui_component* sender, int timerid);
static void OnSequenceSelChange(MainFrame* , SequenceEntry* entry);
static void OnPlay(MainFrame*, ui_component* sender);
static void OnStop(MainFrame*, ui_component* sender);
static void OnLoadSong(MainFrame*, ui_component* sender);
static void OnNewSong(MainFrame*, ui_component* sender);
static void OnAlign(MainFrame*, ui_component* sender);
static void OnMouseDown(MainFrame*, ui_component* sender, int x, int y, int button);
static void OnMouseMove(MainFrame*, ui_component* sender, int x, int y, int button);
static void OnMouseUp(MainFrame*, ui_component* sender, int x, int y, int button);
static void OnGear(MainFrame*, ui_component* sender);
static void OnGearCreate(MainFrame*, ui_component* sender);
static void OnAboutOk(MainFrame*, ui_component* sender);
static void SetStartPage(MainFrame*);
static void OnSettingsViewChanged(MainFrame*, ui_component* sender, Properties*);

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

	self->toolbarheight = 80;	
	self->resize = 0;
	workspace_init(&self->workspace);	
	workspace_load_configuration(&self->workspace);
	self->player = &self->workspace.player;
	self->firstshow = 1;	

	ui_frame_init(&self->component, 0);	
	ui_component_init(&self->top, &self->component);
	ui_component_setbackgroundmode(&self->top, BACKGROUND_SET);		
	InitBars(self, &self->top);	
	// ui_component_init(&self->client, &self->component);	
	// ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);		
	// ui_component_resize(&self->component, 800, 600);
	ui_component_settitle(&self->component, "Psycle");
	signal_connect(&self->component.signal_destroy, self, Destroy);
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_align, self, OnAlign);
	signal_connect(&self->component.signal_timer, self, OnTimer);	
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
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
	ui_component_setbackgroundcolor(&self->splitbar, 0x00363636);	
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
		self->workspace.config);	
	signal_connect(&self->settingsview.signal_changed, self, OnSettingsViewChanged);
	InitHelpView(&self->helpview, &self->notebook.component, &self->tabbars,
		&self->workspace);	
	signal_connect(&self->helpview.about.okbutton.signal_clicked, self, OnAboutOk);
	InitNoteInputs(&self->noteinputs);	
	InitSequenceView(&self->sequenceview, &self->component, &self->workspace);
	ui_component_move(&self->sequenceview.component, 0, self->toolbarheight);	
	InitGear(&self->gear, &self->component, &self->workspace);
	ui_component_resize(&self->gear.component, 300, 200);
	ui_component_hide(&self->gear.component);
	signal_connect(&self->machinebar.gear.signal_clicked, self, OnGear);	
	signal_connect(&self->gear.buttons.createreplace.signal_clicked, self, OnGearCreate);	
	/*ui_menu_init(&menu, "", 0);
	ui_menu_init(&menu_file, "File", OnFileMenu);		
	ui_menu_append(&menu, &menu_file, 0);
	ui_component_setmenu(&self->component, &menu);
	*/
		
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
			char* titlestr = 0;
			properties_readstring(title, "title", &titlestr, "Untitled");
			ui_statusbar_settext(&self->statusbar, 0, titlestr);
		}
	}
}

void InitBars(MainFrame* self, ui_component* parent)
{
	InitPlayBar(&self->playbar, parent);
	ui_component_setposition(&self->playbar.component, 320, 2, 160, 25);		
	signal_connect(&self->playbar.signal_play, self, OnPlay);
	signal_connect(&self->playbar.signal_stop, self, OnStop);
	
	ui_button_init(&self->newsongbutton, parent);
	ui_button_settext(&self->newsongbutton, "New Song");
	ui_component_setposition(&self->newsongbutton.component, 0, 0, 100, 20);	
	signal_connect(&self->newsongbutton.signal_clicked, self, OnNewSong);
	ui_button_init(&self->loadsongbutton, parent);
	ui_button_settext(&self->loadsongbutton, "Load Song");
	ui_component_setposition(&self->loadsongbutton.component, 105, 0, 100, 20);	
	signal_connect(&self->loadsongbutton.signal_clicked, self, OnLoadSong);	
	InitMachineBar(&self->machinebar, parent, &self->workspace);	
	ui_component_move(&self->machinebar.component, 3, 50);
	ui_component_resize(&self->machinebar.component, 530, 25);
	// Songbar
	InitSongTrackBar(&self->songtrackbar, parent, &self->workspace);
	ui_component_setposition(&self->songtrackbar.component, 0, 25, 100, 20);
	InitTimeBar(&self->timebar, parent, &self->workspace.player);
	ui_component_setposition(&self->timebar.component, 100, 25, 190, 20);	
	InitLinesPerBeatBar(&self->linesperbeatbar, parent, &self->workspace.player);
	ui_component_setposition(&self->linesperbeatbar.component, 290, 25, 180, 20);	
	InitOctaveBar(&self->octavebar, parent, &self->workspace);
	ui_component_setposition(&self->octavebar.component, 445, 25, 100, 20);
	// Vugroup
	InitVumeter(&self->vumeter, parent, &self->workspace);
	ui_component_setposition(&self->vumeter.component, 540, 0, 200, 20);	
	InitVolSlider(&self->volslider, parent, &self->workspace.player);
	ui_component_setposition(&self->volslider.component, 540, 20, 200, 20);	
	InitClipBox(&self->clipbox, parent, &self->workspace);
	ui_component_setposition(&self->clipbox.component, 745, 5, 10, 35);
}

void SetStartPage(MainFrame* self)
{
	int showabout;

	properties_readbool(self->workspace.config, "showaboutatstart", &showabout, 1);
	if (showabout) {
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

	size = ui_component_size(&self->component);
	statusbarsize = ui_component_size(&self->statusbar.component);
	tabbarsize = ui_component_size(&self->tabbar.component);
	sequenceviewsize = ui_component_size(&self->sequenceview.component);
	
	if (self->gear.component.visible) {
		gearsize = ui_component_size(&self->gear.component);		
	} else {
		gearsize.width = 0;
	}
	ui_component_setposition(&self->top, 0, 0, size.width, self->toolbarheight);
	ui_component_setposition(&self->tabbars,
		sequenceviewsize.width + 4,
		self->toolbarheight,
		size.width - sequenceviewsize.width - 4,
		tabbarsize.height);
	ui_component_setposition(&self->splitbar,
		sequenceviewsize.width,
		self->toolbarheight, 4,
		size.height - statusbarsize.height - self->toolbarheight);
	ui_component_resize(&self->sequenceview.component,
		sequenceviewsize.width,
		size.height - statusbarsize.height - self->toolbarheight);	
	ui_component_setposition(&self->notebook.component,
		sequenceviewsize.width + 4,
		self->toolbarheight + tabbarsize.height,
		size.width - sequenceviewsize.width - 3 - gearsize.width,
		size.height - statusbarsize.height - self->toolbarheight - tabbarsize.height);
	ui_component_resize(&self->statusbar.component, 0, 0);
	if (ui_component_visible(&self->gear.component)) {
		ui_component_move(&self->gear.component,
			size.width - gearsize.width,
			self->toolbarheight + tabbarsize.height);
		ui_component_resize(&self->gear.component, gearsize.width,
			size.height - statusbarsize.height - self->toolbarheight - tabbarsize.height);
	}
	if (self->firstshow) {
		machineview_align(&self->machineview);
		self->firstshow = 0;
	}
}

void OnSize(MainFrame* self, ui_component* sender, int width, int height)
{	
	ui_component_align(&self->component);
}

void OnKeyDown(MainFrame* self, ui_component* component, int keycode, int keydata)
{	
	if (keycode == VK_F2) {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	} else
	if (keycode == VK_F5) {
		player_start(self->player);		
	} else
	if (keycode == VK_F3) {
		tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
	} else	 
	if (keycode == VK_F6) {
		tabbar_select(&self->tabbar, TABPAGE_SETTINGSVIEW);
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
	ui_statusbar_settext(&self->statusbar, 1, buffer);	
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
	int showsonginfo = 0;	
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, path)) {
		workspace_loadsong(&self->workspace, path);
		properties_readbool(self->workspace.config, "showsonginfoonload",
			&showsonginfo, 1);
		if (showsonginfo) {
			tabbar_select(&self->tabbar, TABPAGE_PROPERTIESVIEW);
		} else {
			ui_invalidate(&self->component);	
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
	
		GetWindowRect(sender->hwnd, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient(GetParent(sender->hwnd), &pt);
		ui_component_move(sender, pt.x + x, self->toolbarheight);
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
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	tabbar_select(&self->machineview.tabbar, 1);
}

void OnSettingsViewChanged(MainFrame* self, ui_component* sender, Properties* property)
{
	signal_emit(&self->workspace.signal_configchanged,
		&self->workspace, 1, property);
}