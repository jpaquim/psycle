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
static void Draw(MainFrame* self, ui_graphics* g);
static void Destroy(MainFrame* self, ui_component* component);
static void OnSize(MainFrame* self, int width, int height);
static int OnKeyDown(MainFrame* self, int keycode, int keydata);
static void OnTimer(MainFrame* self, int timerid);
static void OnSequenceSelChange(MainFrame* self, SequenceEntry* entry);
static void SetActiveView(MainFrame* self, ui_component* view);

static ui_statusbar statusbar;
static ui_menu menu;    
static ui_menu menu_file;
static ui_edit edit;
static ui_button button;
static ui_label label;
static SongProperties songproperties;

extern Song song;

void InitMainFrame(MainFrame* self, Properties* properties, Player* player)
{	
	int iStatusWidths[] = {100, 200, -1};

	ui_frame_init(self, &self->component, 0);	 
	self->component.events.destroy = Destroy;
	self->component.events.draw = Draw;
	self->component.events.destroy = Destroy;	
	self->component.events.size = OnSize;
	self->component.events.timer = OnTimer;
	self->component.events.keydown = OnKeyDown;
	InitSettingsView(&self->settingsview, &self->component, properties);
	ui_component_move(&self->settingsview.component, 150, 0);
	ui_component_hide(&self->settingsview.component);
	InitMachineView(&self->machineview, &self->component, player, properties);	
	ui_component_move(&self->machineview.component, 150, 0);	
	ui_component_hide(&self->machineview.component);
	self->patternview.noteinputs = &self->noteinputs;
	self->patternview.pattern = patterns_at(&player->song->patterns, 0);
	InitPatternView(&self->patternview, &self->component, player);
	ui_component_hide(&self->machineview.component);
	ui_component_move(&self->patternview.component, 150, 0);
	self->activeview = &self->patternview.component;
	InitNoteInputs(&self->noteinputs);	
	InitSequenceView(&self->sequenceview, &self->component, player->sequencer.sequence, &player->song->patterns);	
	SequenceViewConnect(&self->sequenceview, self);	
	self->sequenceview.listview.selchanged = OnSequenceSelChange;
	SetActiveView(self, &self->machineview);
	/*InitSongProperties(&songproperties, &self->component);	
	ui_menu_init(&menu, "", 0);
	ui_menu_init(&menu_file, "File", OnFileMenu);		
	ui_menu_append(&menu, &menu_file, 0);
	ui_component_setmenu(&self->component, &menu);
	*/
	
	ui_statusbar_init(&statusbar, &statusbar, &self->component);
	ui_statusbar_setfields(&statusbar, 3, iStatusWidths);
	ui_statusbar_settext(&statusbar, 0, "Field1");
	ui_statusbar_settext(&statusbar, 1, "Field2");
	ui_statusbar_settext(&statusbar, 2, "Field3");	

	SetTimer(self->component.hwnd, 100, 50, 0);
}

void InitMenu(MainFrame* self)
{
}

void Draw(MainFrame* self, ui_graphics* g)
{
	ui_size size = ui_component_size(&self->component);    
	ui_rectangle r;
    ui_setrectangle(&r, 0, 0, size.width, size.height);
    ui_drawrectangle(g, r);    
}

void Destroy(MainFrame* self, ui_component* component)
{
	PostQuitMessage (0) ;
}

void OnSize(MainFrame* self, int width, int height)
{
	int statusbarheight = 30;
	self->cx = width;
	self->cy = height;
	ui_component_resize(&self->sequenceview.component, 150, height - statusbarheight);
	ui_component_resize(self->activeview, width - 150, height - statusbarheight);
	SendMessage(statusbar.component.hwnd, WM_SIZE, 0, 0);
}

void OnFileMenu(ui_menu* menu)
{
   PostQuitMessage(0);  
}

void SetActiveView(MainFrame* self, ui_component* view)
{
	if (self->activeview) {
		ui_component_hide(self->activeview);
	}
	self->activeview = view;
	ui_component_show(self->activeview);
	ui_component_resize(self->activeview, self->cx - 150, self->cy - 30);		
	ui_component_setfocus(self->activeview);
}

int OnKeyDown(MainFrame* self, int keycode, int keydata)
{	
	if (keycode == VK_F2) {
		SetActiveView(self, &self->machineview.component);
	} else
	if (keycode == VK_F3) {
		SetActiveView(self, &self->patternview.component);
	} else	 
	if (keycode == VK_F6) {
		SetActiveView(self, &self->settingsview.component);		
	} else
	if (keycode == VK_F9) {
		song_load(&song, "Example - mixerdemo.psy");
	} else 
	if (keycode == VK_F4) {
		Properties* properties = properties_create();
		properties_init(properties);
		skin_load(properties, "old Psycle.psv");
		PatternViewApplyProperties(&self->patternview, properties);
		MachineViewApplyProperties(&self->machineview, properties);
		properties_free(properties);
	} else {
		int cmd;
		
		cmd = Cmd(&self->noteinputs.map, keycode);
		if (cmd != -1) {
			Machine* machine;
			int base;
			base = 48;
			machine = machines_at(&self->machineview.player->song->machines, 1);
			if (machine) {
				machine->seqtick(machine, 0, cmd + base, 0, 0, 0);
			}
		}
	}	
	return 0;
}


void OnTimer(MainFrame* self, int timerid)
{
	char buffer[20];

	_snprintf(buffer, 20, "%.4f", self->patternview.player->pos); 
	ui_statusbar_settext(&statusbar, 0, buffer);	
}

void OnSequenceSelChange(MainFrame* self, SequenceEntry* entry)
{
	Pattern* pattern = patterns_at(&song.patterns, entry->pattern);
	if (pattern) {
		self->patternview.pattern = pattern;
		ui_invalidate(&self->patternview.component);
	}
}