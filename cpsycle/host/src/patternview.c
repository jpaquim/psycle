// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

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
static void OnStatusDraw(PatternViewStatus*, ui_component* sender, ui_graphics* g);
static void onstatuspreferredsize(PatternViewStatus* self, ui_component* sender, ui_size* limit, int* width, int* height);
static void OnPatternEditPositionChanged(PatternViewStatus*, Workspace* sender);
static void OnStatusSequencePositionChanged(PatternViewStatus*, Sequence* sender);
static void OnStatusSongChanged(PatternViewStatus*, Workspace* sender);

void InitPatternViewStatus(PatternViewStatus* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	self->component.doublebuffered = 1;
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_resize(&self->component, 300, 20);
	signal_connect(&self->component.signal_draw, self, OnStatusDraw);
	signal_connect(&workspace->signal_editpositionchanged, self, OnPatternEditPositionChanged);
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self, onstatuspreferredsize);
	signal_connect(&workspace->signal_songchanged, self, OnStatusSongChanged);
	signal_connect(&workspace->song->sequence.signal_editpositionchanged,
		self, OnStatusSequencePositionChanged);
}

void OnStatusSequencePositionChanged(PatternViewStatus* self, Sequence* sender)
{
	ui_invalidate(&self->component);
}

void OnStatusSongChanged(PatternViewStatus* self, Workspace* workspace)
{
	if (workspace->song) {
		signal_connect(&workspace->song->sequence.signal_editpositionchanged,
			self, OnStatusSequencePositionChanged);
	}
}

void OnPatternEditPositionChanged(PatternViewStatus* self, Workspace* sender)
{
	ui_invalidate(&self->component);
}

void OnStatusDraw(PatternViewStatus* self, ui_component* sender, ui_graphics* g)
{
	char text[256];
	EditPosition editposition;
	SequencePosition sequenceposition;
	SequenceEntry* sequenceentry;
	int pattern;

	editposition = workspace_editposition(self->workspace);
	sequenceposition = 
		sequence_editposition(&self->workspace->song->sequence);
	sequenceentry = sequenceposition_entry(&sequenceposition);	
	if (sequenceentry) {
		pattern = sequenceentry->pattern;
	} else {
		pattern = 0;
	}	
	ui_settextcolor(g, 0x00D1C5B6);
	ui_setbackgroundmode(g, TRANSPARENT);
	_snprintf(text, 256, "          Pat  %2d   Ln   %d   Track   %d   Col  %d         Edit ON",
		pattern,
		editposition.line,
		editposition.track,
		editposition.col);
	ui_textout(g, 0, 0, text, strlen(text));
}

void onstatuspreferredsize(PatternViewStatus* self, ui_component* sender, ui_size* limit, int* width, int* height)
{				
	TEXTMETRIC tm;
	
	tm = ui_component_textmetric(&self->component);
	*width = tm.tmAveCharWidth * 50;
	*height = (int)(tm.tmHeight * 1.5);
}

void InitPatternViewBar(PatternViewBar* self, ui_component* parent, Workspace* workspace)
{		
	ui_component_init(&self->component, parent);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);		
	ui_component_enablealign(&self->component);	
	stepbox_init(&self->step, &self->component, workspace);
	InitPatternViewStatus(&self->status, &self->component, workspace);
	{		
		ui_margin margin = { 2, 10, 2, 0 };
				
		list_free(ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));		
	}
}

void InitPatternView(PatternView* self, 
		ui_component* parent,
		ui_component* tabbarparent,		
		Workspace* workspace)
{
	self->workspace = workspace;
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
	// Tabbar
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
	self->lpb = player_lpb(&workspace->player);
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
	// Sequence* sequence;	
	// SequenceTrackIterator iterator;
	// SequenceEntry* seqentry; 
	// Pattern* pattern;
	//    		
	// sequence = &self->workspace->song->sequence;	
	// iterator = sequence_begin(sequence, sequence->tracks, 0.0f);
	// seqentry = sequencetrackiterator_entry(&iterator);
	// pattern = patterns_at(sequence->patterns, seqentry->pattern);
	// pattern_scale(pattern, self->lpb / (beat_t)lpb);
	// self->lpb = lpb;
	//

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
