// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "patternview.h"

static void OnPatternViewSize(PatternView* self, ui_component* sender, int width, int height);
static void OnTabBarChange(PatternView* self, ui_component* sender, int tabindex);
static void OnShow(PatternView* self, ui_component* sender);
static void OnHide(PatternView* self, ui_component* sender);
static void OnLpbChanged(PatternView* self, Player* sender, unsigned int lpb);

void InitPatternView(PatternView* self, ui_component* parent, Player* player)
{
	ui_component_init(&self->component, parent);
	InitTrackerView(&self->trackerview, &self->component, player);
	signal_connect(&self->component.signal_size, self, OnPatternViewSize);
	TrackerViewSetPattern(&self->trackerview, patterns_at(&player->song->patterns, 0));	
	InitPianoroll(&self->pianoroll, &self->component);	
	self->pianoroll.pattern = patterns_at(&player->song->patterns, 0);
	ui_component_hide(&self->pianoroll.component);

	InitTabBar(&self->tabbar, parent);
	ui_component_move(&self->tabbar.component, 600, 50);
	ui_component_resize(&self->tabbar.component, 160, 20);
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "Tracker");
	tabbar_append(&self->tabbar, "Pianoroll");
	self->tabbar.selected = 0;
	signal_connect(&self->tabbar.signal_change, self, OnTabBarChange);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	signal_connect(&player->signal_lpbchanged, self, OnLpbChanged);
}

void OnPatternViewSize(PatternView* self, ui_component* sender, int width, int height)
{
	ui_size size = ui_component_size(&self->component);
	ui_component_resize(&self->trackerview.component, size.width, size.height);
	ui_component_resize(&self->pianoroll.component, size.width, size.height);
}

void OnTabBarChange(PatternView* self, ui_component* sender, int tabindex)
{
	ui_size size = ui_component_size(&self->component);
	switch (tabindex) {
		case 0:
			ui_component_hide(&self->pianoroll.component);
			ui_component_show(&self->trackerview.component);			
			ui_component_resize(&self->trackerview.component, size.width, size.height);	
			ui_component_setfocus(&self->trackerview.component);	
		break;
		case 1:			
			ui_component_hide(&self->trackerview.component);
			ui_component_show(&self->pianoroll.component);
			ui_component_resize(&self->pianoroll.component, size.width, size.height);	
			ui_component_setfocus(&self->pianoroll.component);	
		break;
		default:;
		break;
	};
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