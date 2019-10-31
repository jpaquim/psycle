// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
#include <uimenu.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uilabel.h>
#include "settingsview.h"
#include "cmdsnotes.h"
#include "skinio.h"
#include "inputmap.h"
#include <uiapp.h>
#include <dir.h>
#include "resources/resource.h"

static void InitMenu(MainFrame*);
static void InitStatusBar(MainFrame*);
static void InitBars(MainFrame*);
static void InitVuBar(MainFrame*);
static void SetStatusBarText(MainFrame*, const char* text);
static const char* StatusBarIdleText(MainFrame* self);
static void Destroy(MainFrame*, ui_component* component);
static void OnKeyDown(MainFrame*, ui_component* component, int keycode, int keydata);
static void OnSequenceSelChange(MainFrame* , SequenceEntry* entry);
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
static void onsongloadprogress(MainFrame*, Workspace*, int progress);
static void onpluginscanprogress(MainFrame*, Workspace*, int progress);
static void UpdateTitle(MainFrame*);

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

void mainframe_init(MainFrame* self)
{			
	ui_margin tabbardividemargin = { 0, 30, 0, 0};

	ui_frame_init(&self->component, 0);				
	ui_component_seticonressource(&self->component, IDI_PSYCLEICON);	
	ui_component_enablealign(&self->component);
	self->resize = 0;
	workspace_init(&self->workspace, &self->component);	
	workspace_load_configuration(&self->workspace);
	if (!workspace_hasplugincache(&self->workspace)) {
		workspace_scanplugins(&self->workspace);
	}	
	signal_connect(&self->workspace.signal_songchanged, self, OnSongChanged);
	UpdateTitle(self);
	self->firstshow = 1;	
	InitBars(self);	
	signal_connect(&self->component.signal_destroy, self, Destroy);
	signal_connect(&self->component.signal_align, self, OnAlign);	
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	signal_connect(&self->component.signal_align, self, OnAlign);
	ui_component_init(&self->tabbars, &self->component);		
	tabbar_init(&self->tabbar, &self->tabbars);	
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
	signal_connect(&self->splitbar.signal_mouseenter, self, OnMouseEnterSplitBar);
	signal_connect(&self->splitbar.signal_mouseleave, self, OnMouseLeaveSplitBar);
	/// init notebook views
	ui_notebook_init(&self->notebook, &self->component);	
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	machineview_init(&self->machineview, &self->notebook.component, &self->tabbars, &self->workspace);
	patternview_init(&self->patternview, &self->notebook.component, &self->tabbars, &self->workspace);	
	InitSamplesView(&self->samplesview, &self->notebook.component, &self->tabbars, &self->workspace);	
	InitInstrumentsView(&self->instrumentsview, &self->notebook.component,
		&self->tabbars, &self->workspace);
	InitSongProperties(&self->songproperties, &self->notebook.component, &self->workspace);	
	InitSettingsView(&self->settingsview, &self->notebook.component,
		&self->tabbars, self->workspace.config);	
	signal_connect(&self->settingsview.signal_changed, self, OnSettingsViewChanged);
	InitHelpView(&self->helpview, &self->notebook.component, &self->tabbars,
		&self->workspace);	
	signal_connect(&self->helpview.about.okbutton.signal_clicked, self, OnAboutOk);	
	InitSequenceView(&self->sequenceview, &self->component, &self->workspace);	
	InitGear(&self->gear, &self->component, &self->workspace);
	ui_component_hide(&self->gear.component);
	signal_connect(&self->machinebar.gear.signal_clicked, self, OnGear);	
	signal_connect(&self->gear.buttons.createreplace.signal_clicked, self, OnGearCreate);	
		
	InitStatusBar(self);
	signal_connect(&self->splitbar.signal_mousedown, self, OnMouseDown);	
	signal_connect(&self->splitbar.signal_mousemove, self, OnMouseMove);	
	signal_connect(&self->splitbar.signal_mouseup, self, OnMouseUp);
	self->splitbar.debugflag = 10000;	
	SetStartPage(self);
	if (self->workspace.song->properties) {
		Properties* title;
		title = properties_find(self->workspace.song->properties, "title");
		if (title) {						
			SetStatusBarText(self, properties_valuestring(title));
		}
	}
	SetStatusBarText(self, StatusBarIdleText(self));
	signal_emit(&self->workspace.signal_configchanged, &self->workspace, 1,
		self->workspace.config);	
}

void SetStatusBarText(MainFrame* self, const char* text)
{	
	ui_label_settext(&self->statusbarlabel, text ? text : "");
}

const char* StatusBarIdleText(MainFrame* self)
{
	const char* rv;

	if (self->workspace.song->properties) {
		Properties* title;
		title = properties_find(self->workspace.song->properties, "title");
		if (title) {						
			rv = properties_valuestring(title);
		}
	}
	return rv;
}

void InitStatusBar(MainFrame* self)
{	
	ui_component_init(&self->statusbar, &self->component);	
	ui_component_enablealign(&self->statusbar);
	{ // statusbar label
		ui_margin margin = { 2, 0, 2, 0 };
		ui_label_init(&self->statusbarlabel, &self->statusbar);
		ui_label_settext(&self->statusbarlabel, "Ready");
		ui_label_setcharnumber(&self->statusbarlabel, 31);
		ui_component_setmargin(&self->statusbarlabel.component, &margin);
		ui_component_setalign(&self->statusbarlabel.component, UI_ALIGN_LEFT);
	}
	InitPatternViewBar(&self->patternbar, &self->statusbar, &self->workspace);		
	ui_component_setalign(&self->patternbar.component, UI_ALIGN_LEFT);
	ui_progressbar_init(&self->progressbar, &self->statusbar);
	ui_component_setalign(&self->progressbar.component, UI_ALIGN_RIGHT);	
	signal_connect(&self->workspace.signal_loadprogress, self, 
		onsongloadprogress);
	signal_connect(&self->workspace.signal_scanprogress, self, 
		onpluginscanprogress);
}

void InitBars(MainFrame* self)
{
	ui_component_init(&self->top, &self->component);	
	ui_component_resize(&self->top, 500, 400);
	ui_component_enablealign(&self->top);	
	// row0
	ui_component_init(&self->toprow0, &self->top);
	ui_component_enablealign(&self->toprow0);	
	ui_component_setalign(&self->toprow0, UI_ALIGN_TOP);	
	// row1
	ui_component_init(&self->toprow1, &self->top);
	ui_component_enablealign(&self->toprow1);	
	ui_component_setalign(&self->toprow1, UI_ALIGN_TOP);	
	// row2
	ui_component_init(&self->toprow2, &self->top);
	ui_component_enablealign(&self->toprow2);	
	ui_component_setalign(&self->toprow2, UI_ALIGN_TOP);	
	// add bars to rows
	// row0
	filebar_init(&self->filebar, &self->toprow0, &self->workspace);	
	undoredobar_init(&self->undoredobar, &self->toprow0, &self->workspace);	
	playbar_init(&self->playbar, &self->toprow0, &self->workspace);	
	playposbar_init(&self->playposbar, &self->toprow0, &self->workspace.player);		
	{
		ui_margin margin = { 0, 30, 0, 0 };

		list_free(ui_components_setalign(
			ui_component_children(&self->toprow0, 0),
			UI_ALIGN_LEFT, &margin));
		margin.right = 0;
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
	InitVuBar(self);
	ui_component_resize(&self->vubar, 200, 50);	
}

void InitVuBar(MainFrame* self)
{
	ui_component_init(&self->vubar, &self->component);	
	InitVumeter(&self->vumeter, &self->vubar, &self->workspace);
	ui_component_setposition(&self->vumeter.component, 0, 00, 200, 20);	
	InitVolSlider(&self->volslider, &self->vubar, &self->workspace);
	ui_component_setposition(&self->volslider.slider.component, 0, 20, 200, 20);	
	InitClipBox(&self->clipbox, &self->vubar, &self->workspace);
	ui_component_setposition(&self->clipbox.component, 205, 25, 10, 35);	
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
	ui_size topsize;	
	ui_size limit;
	int splitbarwidth = 4;	
	
	size = ui_component_size(&self->component);
	statusbarsize = ui_component_preferredsize(&self->statusbar, &size);
	tabbarsize = ui_component_preferredsize(&self->tabbar.component, &size);	
	sequenceviewsize = ui_component_size(&self->sequenceview.component);
	vusize = ui_component_size(&self->vubar);	
	if (self->gear.component.visible) {
		gearsize = ui_component_size(&self->gear.component);		
	} else {
		gearsize.width = 0;
	}		
	limit.width = size.width - vusize.width;
	limit.height = size.height;
	topsize = ui_component_preferredsize(&self->top, &limit);	
	ui_component_setposition(&self->top, 0, 0, size.width - vusize.width,
		topsize.height);
	ui_component_setposition(&self->vubar, 
		size.width - vusize.width,
		0,
		vusize.width,
		topsize.height);
	ui_component_setposition(&self->tabbars,
		sequenceviewsize.width + splitbarwidth,
		topsize.height,
		size.width - sequenceviewsize.width - splitbarwidth,
		tabbarsize.height);
	ui_component_resize(&self->tabbar.component, tabbarsize.width, tabbarsize.height);		
	ui_component_setposition(&self->splitbar,
		sequenceviewsize.width,
		topsize.height, splitbarwidth,
		size.height - statusbarsize.height - topsize.height);
	ui_component_setposition(&self->sequenceview.component,
		0,
		topsize.height,
		sequenceviewsize.width,
		size.height - statusbarsize.height - topsize.height);	
	ui_component_setposition(&self->notebook.component,
		sequenceviewsize.width + splitbarwidth,
		topsize.height + tabbarsize.height,
		size.width - sequenceviewsize.width - 3 - gearsize.width,
		size.height - statusbarsize.height - topsize.height - tabbarsize.height);
	ui_component_setposition(&self->statusbar,
		0,
		size.height - statusbarsize.height,		
		size.width,
		statusbarsize.height);
	if (ui_component_visible(&self->gear.component)) {
		ui_component_setposition(&self->gear.component,
			size.width - gearsize.width,
			topsize.height + tabbarsize.height,
			gearsize.width,
			size.height - statusbarsize.height - topsize.height - tabbarsize.height);
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
		Properties* properties;
		
		properties = properties_create();		
		skin_load(properties, "old Psycle.psv");
		TrackerViewApplyProperties(&self->patternview.trackerview, properties);
		machineview_applyproperties(&self->machineview, properties);
		properties_free(properties);
	} else {			
		EventDriver* kbd;
		int input;
		
		input = encodeinput(keycode, GetKeyState(VK_SHIFT) < 0,
			GetKeyState(VK_CONTROL) < 0);		
		kbd = workspace_kbddriver(&self->workspace);
		kbd->write(kbd, (unsigned char*)&input, 4);

		/*Machine* machine;
		int base;
		base = 48;
		machine = machines_at(&self->workspace.song->machines,
			self->workspace.song->machines.slot);
		if (machine) {				
			PatternEvent event = { cmd + base, 0, 0, 0, 0 };
			machine->seqtick(machine, 0, &event);
		}*/				
	}
}

void onsongloadprogress(MainFrame* self, Workspace* workspace, int progress)
{
	ui_progressbar_setprogress(&self->progressbar, progress / 100.f);
}

void onpluginscanprogress(MainFrame* self, Workspace* workspace, int progress)
{	
	if (progress == 0) {
		ui_progressbar_setprogress(&self->progressbar, 0);
	} else {
		ui_progressbar_tick(&self->progressbar);
	}
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
				SetStatusBarText(self, titlestr);				
			}						
		}
	}
	UpdateTitle(self);
	ui_invalidate(&self->component);	
}

void UpdateTitle(MainFrame* self)
{	
	char txt[512];
	char name[512];
	char ext[512];

	extract_path(self->workspace.filename, name, ext);
	_snprintf(txt, 512, "[%s.%s]  Psycle Modular Music Creation Studio ",
		name, ext);			
	ui_component_settitle(&self->component, txt);
}

void OnMouseDown(MainFrame* self, ui_component* sender, int x, int y, int button)
{	
	ui_component_capture(sender);
	self->resize = 1;	
}

void OnMouseMove(MainFrame* self, ui_component* sender, int x, int y, int button)
{
	if (self->resize == 1) {		
		ui_size toolbarsize;
		ui_rectangle position;
	
		toolbarsize = ui_component_size(&self->top);
		position = ui_component_position(sender);
		ui_component_move(sender, position.left + x, toolbarsize.height);
		ui_invalidate(sender);
		UpdateWindow((HWND)sender->hwnd);
	}
}

void OnMouseUp(MainFrame* self, ui_component* sender, int x, int y, int button)
{			
	ui_rectangle position;
	
	ui_component_releasecapture();
	self->resize = 0;
	position = ui_component_position(sender);
	ui_component_resize(&self->sequenceview.component,
		position.left, ui_component_size(&self->sequenceview.component).height);	
	ui_component_align(&self->component);	
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
	self->machineview.newmachine.pluginsview.calledbygear = 1;
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

int mainframe_showmaximizedatstart(MainFrame* self)
{
	return workspace_showmaximizedatstart(&self->workspace);
}
