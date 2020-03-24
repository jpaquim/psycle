// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternview.h"

#include <string.h>
#include <songio.h>

#include "../../detail/portable.h"

static void patternview_ontabbarchange(PatternView*, psy_ui_Component* sender,
	int tabindex);
static void patternview_onsongchanged(PatternView*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void patternview_onsequenceselectionchanged(PatternView*,
	Workspace* sender);
static void patternview_onpropertiesapply(PatternView*,
	psy_ui_Component* sender);
static void patternview_onfocus(PatternView*, psy_ui_Component* sender);
static void patternview_oncontextmenu(PatternView*,
	psy_ui_Component* sender);
static void patternviewstatus_ondraw(PatternViewStatus*, psy_ui_Graphics*);
static void patternviewstatus_onpreferredsize(PatternViewStatus* self,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewstatus_onsequenceselectionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewbar_ondefaultline(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_initalign(PatternViewBar*);
static void patternviewbar_onconfigchanged(PatternViewBar*, Workspace*,
	psy_Properties*);
static void patternviewbar_onmovecursorwhenpaste(PatternViewBar* self,
	psy_ui_Component* sender);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(PatternViewStatus* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			patternviewstatus_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) patternviewstatus_ondraw;
	}
}

void patternviewstatus_init(PatternViewStatus* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);		
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		patternviewstatus_onpatterneditpositionchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternviewstatus_onsequenceselectionchanged);
}

void patternviewstatus_onsequenceselectionchanged(PatternViewStatus* self,
	Workspace* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus* self,
	Workspace* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void patternviewstatus_ondraw(PatternViewStatus* self, psy_ui_Graphics* g)
{
	char text[256];
	PatternEditPosition editposition;
	SequencePosition sequenceposition;
	SequenceEntry* sequenceentry;
	int pattern;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	editposition = workspace_patterneditposition(self->workspace);
	sequenceposition = workspace_sequenceselection(self->workspace).editposition;		
	sequenceentry = sequenceposition_entry(&sequenceposition);	
	if (sequenceentry) {
		pattern = sequenceentry->pattern;
	} else {
		pattern = -1;
	}	
	psy_ui_settextcolor(g, 0x00D1C5B6);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_snprintf(text, 256, "Pat %d  Ln %d  Trk %d  Col %d:%d Edit",
		pattern,
		editposition.line,
		editposition.track,
		editposition.column,
		editposition.digit);
	psy_ui_textout(g, 0, (size.height - tm.tmHeight) / 2, text, strlen(text));
}

void patternviewstatus_onpreferredsize(PatternViewStatus* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{				
	if (rv) {
		psy_ui_TextMetric tm;
	
		tm = psy_ui_component_textmetric(&self->component);
		rv->width = tm.tmAveCharWidth * 40;
		rv->height = (int)(tm.tmHeight);
	}
}

void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);	
	stepbox_init(&self->step, &self->component, workspace);
	psy_ui_checkbox_init(&self->movecursorwhenpaste, &self->component);
	psy_ui_checkbox_settext(&self->movecursorwhenpaste,
		"Move Cursor When Paste");
	psy_signal_connect(&self->movecursorwhenpaste.signal_clicked, self,
		patternviewbar_onmovecursorwhenpaste);
	psy_ui_checkbox_init(&self->defaultentries, &self->component);
	psy_ui_checkbox_settext(&self->defaultentries, "Default Line");
	if (workspace_showgriddefaults(self->workspace)) {
		psy_ui_checkbox_check(&self->defaultentries);
	}
	psy_signal_connect(&self->defaultentries.signal_clicked, self,
		patternviewbar_ondefaultline);
	patternviewstatus_init(&self->status, &self->component, workspace);
	patternviewbar_initalign(self);
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		patternviewbar_onconfigchanged);
	if (workspace_ismovecursorwhenpaste(workspace)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	}
	else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}

}

void patternviewbar_ondefaultline(PatternViewBar* self, psy_ui_CheckBox* sender)
{
	psy_Properties* pv;

	pv = psy_properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {
		psy_Properties* p;
		
		p = psy_properties_read(pv, "griddefaults");
		if (p) {			
			psy_properties_write_bool(pv, "griddefaults", !psy_properties_value(p));
			psy_signal_emit(&self->workspace->signal_configchanged, self->workspace, 1, p);
		}
	}
}

void patternviewbar_initalign(PatternViewBar* self)
{		
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));			
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
		&margin));		
}

void patternview_init(PatternView* self, 
		psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,		
		Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_focus, self, patternview_onfocus);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(&self->notebook),
		BACKGROUND_NONE);
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(&self->notebook));
	psy_ui_component_setbackgroundmode(&self->editnotebook.component, BACKGROUND_NONE);
	psy_ui_notebook_setpageindex(&self->editnotebook, 0);
	trackerview_init(&self->trackerview, &self->editnotebook.component, workspace);	
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, workspace);
	patternproperties_init(&self->properties, psy_ui_notebook_base(&self->notebook), 0);
	patternview_setpattern(self, patterns_at(&workspace->song->patterns, 0));	
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self, patternview_onpropertiesapply);	
	// Tabbar
	psy_ui_component_init(&self->sectionbar, tabbarparent);
	psy_ui_component_enablealign(&self->sectionbar);
	psy_ui_component_setalign(&self->sectionbar, psy_ui_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->sectionbar);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);	
	tabbar_append_tabs(&self->tabbar, "Tracker", "Pianoroll", "Split",
		"Properties", NULL);	
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);	
	tabbar_select(&self->tabbar, 0);
	psy_ui_button_init(&self->contextbutton, &self->sectionbar);
	psy_ui_button_seticon(&self->contextbutton, psy_ui_ICON_MORE);
	psy_ui_component_setalign(psy_ui_button_base(&self->contextbutton), psy_ui_ALIGN_RIGHT);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init(&margin, psy_ui_value_makeeh(-1), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makeew(1));
		psy_ui_component_setmargin(psy_ui_button_base(&self->contextbutton), &margin);
	}
	psy_signal_connect(&self->contextbutton.signal_clicked,
		self, patternview_oncontextmenu);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternview_onsequenceselectionchanged);	
}

void patternview_ontabbarchange(PatternView* self, psy_ui_Component* sender,
	int tabindex)
{
	if (tabindex < 2) {
		if (psy_ui_notebook_splitactivated(&self->editnotebook)) {
			psy_ui_notebook_full(&self->editnotebook);						
		}
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_ui_notebook_setpageindex(&self->editnotebook, tabindex);
	} else 
	if (tabindex == 2) {
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {
			psy_ui_notebook_split(&self->editnotebook);			
		}
	} else {
		psy_ui_notebook_setpageindex(&self->notebook, 1);
	}
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{	
	trackerview_setpattern(&self->trackerview, pattern);
	pianoroll_setpattern(&self->pianoroll, pattern);
	patternproperties_setpattern(&self->properties, pattern);
}

void patternview_onsongchanged(PatternView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{
	psy_audio_Pattern* pattern;
	SequenceSelection selection;	
	
	selection = workspace_sequenceselection(workspace);	
	if (selection.editposition.trackposition.tracknode) {
		SequenceEntry* entry;

		entry = (SequenceEntry*)
			selection.editposition.trackposition.tracknode->entry;
		pattern = patterns_at(&workspace->song->patterns, entry->pattern);	
	} else {
		pattern = 0;
	}
	patternview_setpattern(self, pattern);
	self->trackerview.sequenceentryoffset = 0.f;
	self->pianoroll.sequenceentryoffset = 0.f;
	self->pianoroll.pattern = pattern;
	psy_ui_component_invalidate(&self->component);
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
	psy_ui_component_invalidate(&self->component);
}

void patternview_onpropertiesapply(PatternView* self, psy_ui_Component* sender)
{
	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onfocus(PatternView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->trackerview.grid.component);
}

void patternview_oncontextmenu(PatternView* self, psy_ui_Component* sender)
{
	if (trackerview_blockmenuvisible(&self->trackerview)) {
		trackerview_hideblockmenu(&self->trackerview);
	} else {
		trackerview_showblockmenu(&self->trackerview);
	}
}

void patternviewbar_onmovecursorwhenpaste(PatternViewBar* self, psy_ui_Component* sender)
{
	workspace_movecursorwhenpaste(self->workspace, psy_ui_checkbox_checked(&self->movecursorwhenpaste));
}

void patternviewbar_onconfigchanged(PatternViewBar* self, Workspace* workspace,
	psy_Properties* property)
{
	if (workspace_ismovecursorwhenpaste(workspace)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}
}
