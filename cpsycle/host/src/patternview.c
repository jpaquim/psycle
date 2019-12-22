// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternview.h"
#include <portable.h>

static void patternview_ontabbarchange(PatternView*, ui_component* sender, int tabindex);
static void patternview_onsize(PatternView*, ui_component* sender, ui_size*);
static void patternview_onshow(PatternView*, ui_component* sender);
static void patternview_onhide(PatternView*, ui_component* sender);
static void patternview_onlpbchanged(PatternView*, psy_audio_Player* sender, uintptr_t lpb);
static void patternview_onsongchanged(PatternView*, Workspace* sender);
static void patternview_oneditpositionchanged(PatternView*, Workspace* sender);
static void patternview_onsequenceselectionchanged(PatternView*, Workspace* sender);
static void patternview_onpropertiesclose(PatternView*, ui_component* sender);
static void patternview_onpropertiesapply(PatternView*, ui_component* sender);
static void patternview_onkeydown(PatternView*, ui_component* sender, KeyEvent*);
static void patternview_onkeyup(PatternView*, ui_component* sender, KeyEvent*);
static void patternview_onfocus(PatternView*, ui_component* sender);
static void patternviewstatus_ondraw(PatternViewStatus*, ui_component* sender, ui_graphics* g);
static void patternviewstatus_onpreferredsize(PatternViewStatus* self,
	ui_component* sender, ui_size* limit, ui_size* rv);
static void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewstatus_onsequenceselectionchanged(PatternViewStatus*,
	Workspace* sender);
void patternviewbar_initalign(PatternViewBar*);

void patternviewstatus_init(PatternViewStatus* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	self->component.doublebuffered = 1;	
	ui_component_resize(&self->component, 300, 20);
	psy_signal_connect(&self->component.signal_draw, self,
		patternviewstatus_ondraw);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		patternviewstatus_onpatterneditpositionchanged);
	psy_signal_disconnectall(&self->component.signal_preferredsize);
	psy_signal_connect(&self->component.signal_preferredsize, self,
		patternviewstatus_onpreferredsize);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternviewstatus_onsequenceselectionchanged);
}

void patternviewstatus_onsequenceselectionchanged(PatternViewStatus* self,
	Workspace* sender)
{
	ui_component_invalidate(&self->component);
}

void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus* self,
	Workspace* sender)
{
	ui_component_invalidate(&self->component);
}

void patternviewstatus_ondraw(PatternViewStatus* self, ui_component* sender,
	ui_graphics* g)
{
	char text[256];
	PatternEditPosition editposition;
	SequencePosition sequenceposition;
	SequenceEntry* sequenceentry;
	int pattern;
	ui_size size;
	ui_textmetric tm;

	size = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	editposition = workspace_patterneditposition(self->workspace);
	sequenceposition = workspace_sequenceselection(self->workspace).editposition;		
	sequenceentry = sequenceposition_entry(&sequenceposition);	
	if (sequenceentry) {
		pattern = sequenceentry->pattern;
	} else {
		pattern = -1;
	}	
	ui_settextcolor(g, 0x00D1C5B6);
	ui_setbackgroundmode(g, TRANSPARENT);
	psy_snprintf(text, 256, "          Pat  %2d   Ln   %d   Track   %d   Col  %d         Edit ON",
		pattern,
		editposition.line,
		editposition.track,
		editposition.col);
	ui_textout(g, 0, (size.height - tm.tmHeight) / 2, text, strlen(text));
}

void patternviewstatus_onpreferredsize(PatternViewStatus* self,
	ui_component* sender, ui_size* limit, ui_size* rv)
{				
	if (rv) {
		ui_textmetric tm;
	
		tm = ui_component_textmetric(&self->component);
		rv->width = tm.tmAveCharWidth * 70;
		rv->height = (int)(tm.tmHeight * 1.5);
	}
}

void patternviewbar_init(PatternViewBar* self, ui_component* parent,
	Workspace* workspace)
{		
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);	
	stepbox_init(&self->step, &self->component, workspace);
	ui_checkbox_init(&self->movecursorwhenpaste, &self->component);
	ui_checkbox_settext(&self->movecursorwhenpaste, "Move Cursor When Paste");
	patternviewstatus_init(&self->status, &self->component, workspace);
	patternviewbar_initalign(self);
}

void patternviewbar_initalign(PatternViewBar* self)
{		
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2.0),
		ui_value_makepx(0), ui_value_makepx(0));			
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));		
}
void patternview_init(PatternView* self, 
		ui_component* parent,
		ui_component* tabbarparent,		
		Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_keydown, self,
		patternview_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self,
		patternview_onkeyup);
	psy_signal_connect(&self->component.signal_focus, self, patternview_onfocus);
	ui_notebook_init(&self->notebook, &self->component);
	ui_component_setbackgroundmode(&self->notebook.component, BACKGROUND_NONE);
	ui_notebook_init(&self->editnotebook, &self->notebook.component);
	ui_component_setbackgroundmode(&self->editnotebook.component, BACKGROUND_NONE);
	ui_notebook_setpageindex(&self->editnotebook, 0);
	trackerview_init(&self->trackerview, &self->editnotebook.component, workspace);
	psy_signal_connect(&self->component.signal_size, self, patternview_onsize);	
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, workspace);
	InitPatternProperties(&self->properties, &self->notebook.component, 0);
	patternview_setpattern(self, patterns_at(&workspace->song->patterns, 0));		
	psy_signal_connect(&self->properties.closebutton.signal_clicked, self, patternview_onpropertiesclose);
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self, patternview_onpropertiesapply);	
	// Tabbar
	tabbar_init(&self->tabbar, tabbarparent);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_LEFT);	
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "Tracker");
	tabbar_append(&self->tabbar, "Pianoroll");	
	tabbar_append(&self->tabbar, "Split");
	tabbar_append(&self->tabbar, "Properties");	
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);
	// ui_notebook_connectcontroller(&self->editnotebook, &self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);	
	psy_signal_connect(&self->component.signal_show, self, patternview_onshow);
	psy_signal_connect(&self->component.signal_hide, self, patternview_onhide);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		patternview_onlpbchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);
	//signal_connect(&workspace->signal_editpositionchanged,
	//	self, OnEditPositionChanged);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternview_onsequenceselectionchanged);
	self->lpb = player_lpb(&workspace->player);
}

void patternview_ontabbarchange(PatternView* self, ui_component* sender,
	int tabindex)
{
	if (tabindex < 2) {
		if (self->editnotebook.splitbar.hwnd) {
			ui_notebook_full(&self->editnotebook);						
		}
		ui_notebook_setpageindex(&self->notebook, 0);
		ui_notebook_setpageindex(&self->editnotebook, tabindex);
	} else 
	if (tabindex == 2) {
		ui_notebook_setpageindex(&self->notebook, 0);
		if (self->editnotebook.splitbar.hwnd == 0) {		 
			ui_notebook_split(&self->editnotebook);			
		}
	} else {
		ui_notebook_setpageindex(&self->notebook, 1);
	}
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{	
	trackerview_setpattern(&self->trackerview, pattern);
	pianoroll_setpattern(&self->pianoroll, pattern);
	PatternPropertiesSetPattern(&self->properties, pattern);
}

void patternview_onsize(PatternView* self, ui_component* sender, ui_size* size)
{					
	ui_component_resize(&self->notebook.component, size->width, size->height);
}

void patternview_onshow(PatternView* self, ui_component* sender)
{			
	self->tabbar.component.visible = 1;	
	ui_component_align(ui_component_parent(&self->tabbar.component));
	ui_component_show(&self->tabbar.component);	
}

void patternview_onhide(PatternView* self, ui_component* sender)
{	
	ui_component_hide(&self->tabbar.component);				
}

void patternview_onlpbchanged(PatternView* self, psy_audio_Player* sender, uintptr_t lpb)
{
	// psy_audio_Sequence* sequence;	
	// SequenceTrackIterator iterator;
	// SequenceEntry* seqentry; 
	// psy_audio_Pattern* pattern;
	//    		
	// sequence = &self->workspace->song->sequence;	
	// iterator = sequence_begin(sequence, sequence->tracks, 0.0f);
	// seqentry = sequencetrackiterator_entry(&iterator);
	// pattern = patterns_at(sequence->patterns, seqentry->pattern);
	// pattern_scale(pattern, self->lpb / (beat_t)lpb);
	// self->lpb = lpb;
	//

	ui_component_invalidate(&self->trackerview.component);		
}

void patternview_onsongchanged(PatternView* self, Workspace* workspace)
{
	SequenceSelection selection;	
	
	selection = workspace_sequenceselection(workspace);	
	if (selection.editposition.trackposition.tracknode) {
		SequenceEntry* entry;

		entry = (SequenceEntry*) selection.editposition.trackposition.tracknode->entry;
		patternview_setpattern(self, patterns_at(&workspace->song->patterns,
			entry->pattern));	
	} else {
		patternview_setpattern(self, 0);
	}
	self->trackerview.sequenceentryoffset = 0.f;
	self->pianoroll.sequenceentryoffset = 0.f;
	self->pianoroll.pattern = self->trackerview.pattern;	
	ui_component_invalidate(&self->component);	
}

void patternview_oneditpositionchanged(PatternView* self, Workspace* sender)
{	
	/*psy_audio_Pattern* pattern;

	if (sender->song) {
		pattern = patterns_at(&sender->song->patterns, 
			sender->patterneditposition.pattern);
		patternview_setpattern(self, pattern);
		self->trackerview.sequenceentryoffset = 0; // entry->offset;
		self->pianoroll.sequenceentryoffset = 0; // entry->offset;
	} else {
		patternview_setpattern(self, 0);		
		self->trackerview.sequenceentryoffset = 0.f;
		self->pianoroll.sequenceentryoffset = 0.f;
	}
	ui_component_invalidate(&self->component);*/
}

void patternview_onsequenceselectionchanged(PatternView* self,
	Workspace* workspace)
{	
	SequenceSelection selection;
	SequenceEntry* entry;

	selection = workspace_sequenceselection(workspace);
	entry = sequenceposition_entry(&selection.editposition);
	if (entry) {
		psy_audio_Pattern* pattern;

		pattern = patterns_at(&workspace->song->patterns, 
			entry->pattern);
		patternview_setpattern(self, pattern);
		self->trackerview.sequenceentryoffset = entry->offset;
		self->pianoroll.sequenceentryoffset = entry->offset;
	} else {
		patternview_setpattern(self, 0);		
		self->trackerview.sequenceentryoffset = 0.f;
		self->pianoroll.sequenceentryoffset = 0.f;
	}
	ui_component_invalidate(&self->component);		
}

void patternview_onpropertiesclose(PatternView* self, ui_component* sender)
{	
}

void patternview_onpropertiesapply(PatternView* self, ui_component* sender)
{
	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onkeydown(PatternView* self, ui_component* sender,
	KeyEvent* keyevent)
{
	ui_component_propagateevent(sender);
}

void patternview_onkeyup(PatternView* self, ui_component* sender,
	KeyEvent* keyevent)
{
	ui_component_propagateevent(sender);
}

void patternview_onfocus(PatternView* self, ui_component* sender)
{
	ui_component_setfocus(&self->trackerview.grid.component);
}
