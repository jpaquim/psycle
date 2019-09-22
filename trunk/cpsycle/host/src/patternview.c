// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "patternview.h"

static void OnSize(PatternView* self, ui_component* sender, int width, int height);
static void OnShow(PatternView* self, ui_component* sender);
static void OnHide(PatternView* self, ui_component* sender);
static void OnLpbChanged(PatternView* self, Player* sender, unsigned int lpb);
static void OnSongChanged(PatternView* self, Workspace* sender);

void InitPatternView(PatternView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	ui_notebook_init(&self->notebook, &self->component);
	InitTrackerView(&self->trackerview, &self->notebook.component, workspace);
	signal_connect(&self->component.signal_size, self, OnSize);
	TrackerViewSetPattern(&self->trackerview, patterns_at(&workspace->song->patterns, 0));	
	InitPianoroll(&self->pianoroll, &self->notebook.component);	
	self->pianoroll.pattern = patterns_at(&workspace->song->patterns, 0);
	ui_notebook_setpage(&self->notebook, 0);
	InitTabBar(&self->tabbar, tabbarparent);
	ui_component_move(&self->tabbar.component, 450, 0);
	ui_component_resize(&self->tabbar.component, 160, 20);
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "Tracker");
	tabbar_append(&self->tabbar, "Pianoroll");
	self->tabbar.selected = 0;
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	signal_connect(&workspace->player.signal_lpbchanged, self, OnLpbChanged);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void OnSize(PatternView* self, ui_component* sender, int width, int height)
{	
	ui_component_resize(&self->notebook.component, width, height);
}

void OnShow(PatternView* self, ui_component* sender)
{	
	ui_component_show(&self->tabbar.component);
}

void OnHide(PatternView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void OnLpbChanged(PatternView* self, Player* sender, unsigned int lpb)
{
	ui_invalidate(&self->trackerview.component);
	ui_invalidate(&self->pianoroll.component);
}

void OnSongChanged(PatternView* self, Workspace* workspace)
{
	TrackerViewSetPattern(&self->trackerview, patterns_at(&workspace->song->patterns, 0));	
	self->pianoroll.pattern = self->trackerview.pattern;	
	TrackerViewSongChanged(&self->trackerview, workspace);	
	ui_invalidate(&self->component);
}