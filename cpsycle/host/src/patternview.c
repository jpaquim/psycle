// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "patternview.h"

static void OnSize(PatternView*, ui_component* sender, int width, int height);
static void OnShow(PatternView*, ui_component* sender);
static void OnHide(PatternView*, ui_component* sender);
static void OnLpbChanged(PatternView*, Player* sender, unsigned int lpb);
static void OnSongChanged(PatternView*, Workspace* sender);
static void OnEditPositionChanged(PatternView*, Sequence* sender);
static void OnPropertiesClose(PatternView*, ui_component* sender);
static void OnPropertiesApply(PatternView*, ui_component* sender);
static void OnKeyDown(PatternView*, ui_component* sender, int keycode, int keydata);

void InitPatternView(PatternView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	ui_notebook_init(&self->notebook, &self->component);
	InitTrackerView(&self->trackerview, &self->notebook.component, workspace);
	signal_connect(&self->component.signal_size, self, OnSize);
	PatternViewSetPattern(self, patterns_at(&workspace->song->patterns, 0));	
	InitPianoroll(&self->pianoroll, &self->notebook.component);	
	self->pianoroll.pattern = patterns_at(&workspace->song->patterns, 0);
	ui_notebook_setpage(&self->notebook, 0);
	InitPatternProperties(&self->properties, &self->notebook.component, 0);	
	ui_component_hide(&self->properties.component);	
	signal_connect(&self->properties.closebutton.signal_clicked, self, OnPropertiesClose);
	signal_connect(&self->properties.applybutton.signal_clicked, self, OnPropertiesApply);

	InitTabBar(&self->tabbar, tabbarparent);
	ui_component_move(&self->tabbar.component, 450, 0);
	ui_component_resize(&self->tabbar.component, 160, 20);
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "Tracker");
	tabbar_append(&self->tabbar, "Pianoroll");
	tabbar_append(&self->tabbar, "Properties");
	self->tabbar.selected = 0;
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	signal_connect(&workspace->player.signal_lpbchanged, self, OnLpbChanged);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	signal_connect(&workspace->song->sequence.signal_editpositionchanged,
		self, OnEditPositionChanged);	
}

void PatternViewSetPattern(PatternView* self, Pattern* pattern)
{	
	self->trackerview.pattern = pattern;
	if (pattern) {
		self->trackerview.opcount = pattern->opcount;
	}
	self->trackerview.grid.dx = 0;
	self->trackerview.grid.dy = 0;
	self->trackerview.header.dx = 0;
	self->trackerview.linenumbers.dy = 0;
	PatternPropertiesSetPattern(&self->properties, pattern);
//	AdjustScrollranges(&self->trackerview.grid);
	ui_invalidate(&self->trackerview.linenumbers.component);
	ui_invalidate(&self->trackerview.header.component);	
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
	signal_connect(&workspace->song->sequence.signal_editpositionchanged,
		self, OnEditPositionChanged);
	PatternViewSetPattern(self, patterns_at(&workspace->song->patterns, 0));	
	self->pianoroll.pattern = self->trackerview.pattern;	
	ui_invalidate(&self->component);
}

void OnEditPositionChanged(PatternView* self, Sequence* sender)
{	
	SequenceEntry* entry;
	
	entry = sequenceposition_entry(&sender->editposition);
	if (entry) {			
		PatternViewSetPattern(self,
			patterns_at(&self->trackerview.workspace->song->patterns,
			entry->pattern));
	} else {
		PatternViewSetPattern(self, 0);		
	}
	ui_invalidate(&self->component);
}


void OnPropertiesClose(PatternView* self, ui_component* sender)
{
	// ui_size size = ui_component_size(&self->trackerview.component);
	// OnViewSize(self->t, &self->component, size.width, size.height);	
}

void OnPropertiesApply(PatternView* self, ui_component* sender)
{
	/*SCROLLINFO		si;	
	ui_size size = ui_component_size(&self->component);	
	OnViewSize(self, &self->grid.component, size.width, size.height);	
	size = ui_component_size(&self->grid.component);
	AdjustScrollranges(&self->grid);	
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (self->grid.component.hwnd, SB_VERT, &si) ;	
	self->grid.dy = -si.nPos * self->grid.lineheight;	
	self->linenumbers.dy = self->grid.dy;
	ui_invalidate(&self->component);*/
}

void OnKeyDown(PatternView* self, ui_component* sender, int keycode, int keydata)
{
	ui_component_propagateevent(sender);
}
