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
#include <portable.h>
#include "resources/resource.h"

#define TIMERID_MAINFRAME 20

static void mainframe_initmenu(MainFrame*);
static void mainframe_initstatusbar(MainFrame*);
static void mainframe_initbars(MainFrame*);
static void mainframe_initvubar(MainFrame*);
static void mainframe_setstatusbartext(MainFrame*, const char* text);
static const char* mainframe_statusbaridletext(MainFrame* self);
static void mainframe_destroy(MainFrame*, ui_component* component);
static void mainframe_onkeydown(MainFrame*, ui_component* component,
	KeyEvent*);
static void mainframe_onsequenceselchange(MainFrame* , SequenceEntry* entry);
static void mainframe_onalign(MainFrame*, ui_component* sender);
static void mainframe_onmousedown(MainFrame*, ui_component* sender,
	MouseEvent*);
static void mainframe_onmousemove(MainFrame*, ui_component* sender,
	MouseEvent*);
static void mainframe_onmouseup(MainFrame*, ui_component* sender,
	MouseEvent*);
static void mainframe_ongear(MainFrame*, ui_component* sender);
static void mainframe_ongearcreate(MainFrame*, ui_component* sender);
static void mainframe_onaboutok(MainFrame*, ui_component* sender);
static void mainframe_onupdatedriver(MainFrame*, ui_component* sender);
static void mainframe_setstartpage(MainFrame*);
static void mainframe_onsettingsviewchanged(MainFrame*, SettingsView* sender,
	Properties*);
static void mainframe_onmouseentersplitbar(MainFrame*, ui_component* sender);
static void mainframe_onmouseleavesplitbar(MainFrame*, ui_component* sender);
static void mainframe_onsongchanged(MainFrame*, ui_component* sender,
	int flag);
static void mainframe_onsongloadprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onpluginscanprogress(MainFrame*, Workspace*,
	int progress);
static void mainframe_onviewselected(MainFrame*, Workspace*,
	int view);
static void mainframe_onrender(MainFrame*, ui_component* sender);
static void mainframe_updatetitle(MainFrame*);
static void mainframe_ontimer(MainFrame*, ui_component* sender, int timerid);
static void mainframe_maximizeorminimizeview(MainFrame*);

HWND hwndmain;

enum {
	TABPAGE_MACHINEVIEW		= 0,
	TABPAGE_PATTERNVIEW		= 1,
	TABPAGE_SAMPLESVIEW		= 2,
	TABPAGE_INSTRUMENTSVIEW = 3,
	TABPAGE_PROPERTIESVIEW	= 4,
	TABPAGE_SETTINGSVIEW	= 5,
	TABPAGE_HELPVIEW		= 6,
	TABPAGE_RENDERVIEW		= 7,
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
	signal_connect(&self->workspace.signal_songchanged, self,
		mainframe_onsongchanged);
	mainframe_updatetitle(self);
	self->firstshow = 1;	
	mainframe_initbars(self);
	signal_connect(&self->component.signal_destroy, self, mainframe_destroy);
	signal_connect(&self->component.signal_align, self, mainframe_onalign);	
	signal_connect(&self->component.signal_keydown, self, mainframe_onkeydown);
	signal_connect(&self->component.signal_timer, self, mainframe_ontimer);
	ui_component_init(&self->tabbars, &self->component);
	ui_component_enablealign(&self->tabbars);
	tabbar_init(&self->tabbar, &self->tabbars);	
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_LEFT);
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
	signal_connect(&self->splitbar.signal_mouseenter, self,
		mainframe_onmouseentersplitbar);
	signal_connect(&self->splitbar.signal_mouseleave, self,
		mainframe_onmouseleavesplitbar);
	/// init notebook views
	ui_notebook_init(&self->notebook, &self->component);	
	ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	machineview_init(&self->machineview, &self->notebook.component, 
		&self->tabbars, &self->workspace);
	patternview_init(&self->patternview, &self->notebook.component,
		&self->tabbars, &self->workspace);	
	InitSamplesView(&self->samplesview, &self->notebook.component,
		&self->tabbars, &self->workspace);	
	InitInstrumentsView(&self->instrumentsview, &self->notebook.component,
		&self->tabbars, &self->workspace);
	songpropertiesview_init(&self->songpropertiesview,
		&self->notebook.component, &self->workspace);	
	InitSettingsView(&self->settingsview, &self->notebook.component,
		&self->tabbars, self->workspace.config);	
	signal_connect(&self->settingsview.signal_changed, self,
		mainframe_onsettingsviewchanged);
	InitHelpView(&self->helpview, &self->notebook.component, &self->tabbars,
		&self->workspace);	
	signal_connect(&self->helpview.about.okbutton.signal_clicked, self,
		mainframe_onaboutok);
	sequenceview_init(&self->sequenceview, &self->component, &self->workspace);
	renderview_init(&self->renderview, &self->notebook.component,
		&self->tabbars, &self->workspace);	
	signal_connect(&self->filebar.renderbutton.signal_clicked, self,
		mainframe_onrender);
	signal_connect(&self->workspace.signal_viewselected, self,
		mainframe_onviewselected);
	InitGear(&self->gear, &self->component, &self->workspace);
	ui_component_hide(&self->gear.component);
	signal_connect(&self->machinebar.gear.signal_clicked, self,
		mainframe_ongear);
	signal_connect(&self->gear.buttons.createreplace.signal_clicked, self,
		mainframe_ongearcreate);
	mainframe_initstatusbar(self);
	signal_connect(&self->splitbar.signal_mousedown, self,
		mainframe_onmousedown);
	signal_connect(&self->splitbar.signal_mousemove, self,
		mainframe_onmousemove);
	signal_connect(&self->splitbar.signal_mouseup, self,
		mainframe_onmouseup);
	mainframe_setstartpage(self);
	if (self->workspace.song) {
		mainframe_setstatusbartext(self,
			self->workspace.song->properties.title);
	}	
	signal_emit(&self->workspace.signal_configchanged, &self->workspace, 1,
		self->workspace.config);	
	ui_component_starttimer(&self->component, TIMERID_MAINFRAME, 50);
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
	ui_notebook_init(&self->viewbars, &self->statusbar);
	ui_component_setalign(&self->viewbars.component, UI_ALIGN_LEFT);
	ui_component_enablealign(&self->viewbars.component);		
	machineviewbar_init(&self->machineviewbar, &self->viewbars.component, &self->workspace);
	ui_component_setalign(&self->machineviewbar.component, UI_ALIGN_LEFT);
	self->machineview.wireview.statusbar = &self->machineviewbar;
	patternviewbar_init(&self->patternbar, &self->viewbars.component, &self->workspace);
	ui_component_setalign(&self->patternbar.component, UI_ALIGN_LEFT);
	ui_notebook_setpage(&self->viewbars, 0);
	ui_notebook_connectcontroller(&self->viewbars, &self->tabbar.signal_change);
	ui_progressbar_init(&self->progressbar, &self->statusbar);
	ui_component_setalign(&self->progressbar.component, UI_ALIGN_RIGHT);	
	signal_connect(&self->workspace.signal_loadprogress, self, 
		mainframe_onsongloadprogress);
	signal_connect(&self->workspace.signal_scanprogress, self, 
		mainframe_onpluginscanprogress);
}

void mainframe_initbars(MainFrame* self)
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
		ui_margin margin = { 0, 20, 0, 0 };

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
	vusize = ui_component_size(&self->vubar.component);	
	if (self->gear.component.visible) {
		gearsize = ui_component_size(&self->gear.component);		
	} else {
		gearsize.width = 0;
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
		tabbarsize.height);	
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

void mainframe_onkeydown(MainFrame* self, ui_component* component, KeyEvent* keyevent)
{	
	if (keyevent->keycode == VK_F2) {
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	} else
	if (keyevent->keycode == VK_F5) {
		player_start(&self->workspace.player);		
	} else
	if (keyevent->keycode == VK_F3) {
		tabbar_select(&self->tabbar, TABPAGE_PATTERNVIEW);
		ui_component_setfocus(&self->patternview.trackerview.grid.component);
	} else	 
	if (keyevent->keycode == VK_F6) {
		tabbar_select(&self->tabbar, TABPAGE_SETTINGSVIEW);
	} else
	if (keyevent->keycode == VK_F8) {
		player_stop(&self->workspace.player);		
	} else	
	if (keyevent->keycode == VK_F4) {
		Properties* properties;
		
		properties = properties_create();		
		skin_load(properties, "old Psycle.psv");
		TrackerViewApplyProperties(&self->patternview.trackerview, properties);
		machineview_applyproperties(&self->machineview, properties);
		properties_free(properties);
	} else 
	if (keyevent->keycode == VK_F9) {
		self->machineview.newmachine.pluginsview.calledby = self->tabbar.selected;
		tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
		tabbar_select(&self->machineview.tabbar, 1);
	} else
	if (keyevent->shift && keyevent->keycode == VK_RETURN) {
		workspace_showparameters(&self->workspace,
			machines_slot(&self->workspace.song->machines));
	} else
	if (keyevent->ctrl && keyevent->keycode == VK_TAB) {
		mainframe_maximizeorminimizeview(self);
	} else
	if (keyevent->shift && keyevent->keycode == VK_RIGHT) {
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
	if (keyevent->shift && keyevent->keycode == VK_LEFT) {
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
	} else {
		if (keyevent->keycode != VK_CONTROL &&
			keyevent->keycode != VK_SHIFT) {
			EventDriver* kbd;
			int input;
			
			input = encodeinput(keyevent->keycode, GetKeyState(VK_SHIFT) < 0,
				GetKeyState(VK_CONTROL) < 0);		
			kbd = workspace_kbddriver(&self->workspace);
			kbd->write(kbd, (unsigned char*)&input, 4);
		}

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
	char name[512];
	char ext[512];

	extract_path(self->workspace.filename, name, ext);
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
		ui_component_hide(&self->gear.component);
		ui_component_align(&self->component);
	} else {						
		ui_component_show(&self->gear.component);
		ui_component_align(&self->component);		
	}	
}

void mainframe_onaboutok(MainFrame* self, ui_component* sender)
{
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
}

void mainframe_ongearcreate(MainFrame* self, ui_component* sender)
{
	self->machineview.newmachine.pluginsview.calledby = 10;
	tabbar_select(&self->tabbar, TABPAGE_MACHINEVIEW);
	tabbar_select(&self->machineview.tabbar, 1);
}

void mainframe_onsettingsviewchanged(MainFrame* self, SettingsView* sender,
	Properties* property)
{
	workspace_configchanged(&self->workspace, property, sender->choiceproperty);	
}

void mainframe_onupdatedriver(MainFrame* self, ui_component* sender)
{
	workspace_updatedriver(&self->workspace);
}

int mainframe_showmaximizedatstart(MainFrame* self)
{
	return workspace_showmaximizedatstart(&self->workspace);
}

void mainframe_onrender(MainFrame* self, ui_component* sender)
{
	ui_notebook_setpage(&self->notebook, TABPAGE_RENDERVIEW);
}

void mainframe_ontimer(MainFrame* self, ui_component* sender, int timerid)
{
	workspace_idle(&self->workspace);
}

void mainframe_onviewselected(MainFrame* self, Workspace* sender, int view)
{
	if (view != 10) {
		tabbar_select(&self->tabbar, view);
	}
}
