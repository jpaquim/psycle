// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternview.h"

void patternview_onsplit(PatternView*, ui_component* sender);
static void OnSize(PatternView*, ui_component* sender, ui_size*);
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

void patternview_init(PatternView* self, 
		ui_component* parent,
		ui_component* tabbarparent,		
		Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	ui_notebook_init(&self->notebook, &self->component);	
	trackerview_init(&self->trackerview, &self->notebook.component, workspace);
	signal_connect(&self->component.signal_size, self, OnSize);	
	pianoroll_init(&self->pianoroll, &self->notebook.component, workspace);
	patternview_setpattern(self, patterns_at(&workspace->song->patterns, 0));	
	// InitPatternProperties(&self->properties, &self->notebook.component, 0);
	// signal_connect(&self->properties.closebutton.signal_clicked, self, OnPropertiesClose);
	// signal_connect(&self->properties.applybutton.signal_clicked, self, OnPropertiesApply);	
	// Tabbar
	tabbar_init(&self->tabbar, tabbarparent);
	ui_component_move(&self->tabbar.component, 450, 0);
	ui_component_resize(&self->tabbar.component, 100, 20);
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "Tracker");
	tabbar_append(&self->tabbar, "Pianoroll");
	// tabbar_append(&self->tabbar, "Properties");	
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);
	ui_button_init(&self->split, tabbarparent);
	ui_button_settext(&self->split, "Split");
	ui_component_setposition(&self->split.component, 450 + 100, 0, 50, 12);
	ui_component_hide(&self->split.component);
	signal_connect(&self->split.signal_clicked, self, patternview_onsplit);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	signal_connect(&workspace->player.signal_lpbchanged, self, OnLpbChanged);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	signal_connect(&workspace->song->sequence.signal_editpositionchanged,
		self, OnEditPositionChanged);
	self->lpb = player_lpb(&workspace->player);
}

void patternview_setpattern(PatternView* self, Pattern* pattern)
{	
	trackerview_setpattern(&self->trackerview, pattern);
	pianoroll_setpattern(&self->pianoroll, pattern);
	// PatternPropertiesSetPattern(&self->properties, pattern);
}

void OnSize(PatternView* self, ui_component* sender, ui_size* size)
{					
	ui_component_resize(&self->notebook.component, size->width, size->height);
}

void OnShow(PatternView* self, ui_component* sender)
{			
	ui_component_show(&self->tabbar.component);
	ui_component_show(&self->split.component);
}

void OnHide(PatternView* self, ui_component* sender)
{	
	ui_component_hide(&self->tabbar.component);
	ui_component_hide(&self->split.component);			
}

void patternview_onsplit(PatternView* self, ui_component* sender)
{
	if (self->notebook.splitbar.hwnd) {
		ui_notebook_full(&self->notebook);
		ui_button_settext(&self->split, "Split");
		ui_invalidate(&self->split.component);
	} else {
		ui_notebook_split(&self->notebook);
		ui_button_settext(&self->split, "Full");
		ui_invalidate(&self->split.component);
	}
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
	patternview_setpattern(self, patterns_at(&workspace->song->patterns, 0));	
	self->trackerview.sequenceentryoffset = 0.f;
	self->pianoroll.pattern = self->trackerview.pattern;	
	ui_invalidate(&self->component);
}

void OnEditPositionChanged(PatternView* self, Sequence* sender)
{	
	SequenceEntry* entry;
	
	entry = sequenceposition_entry(&sender->editposition);
	if (entry) {			
		patternview_setpattern(self,
			patterns_at(&self->trackerview.workspace->song->patterns,
			entry->pattern));
		self->trackerview.sequenceentryoffset = entry->offset;
	} else {
		patternview_setpattern(self, 0);		
		self->trackerview.sequenceentryoffset = 0.f;
	}
	ui_invalidate(&self->component);
}

void OnPropertiesClose(PatternView* self, ui_component* sender)
{	
}

void OnPropertiesApply(PatternView* self, ui_component* sender)
{
}

void OnKeyDown(PatternView* self, ui_component* sender, int keycode, int keydata)
{
	ui_component_propagateevent(sender);
}
