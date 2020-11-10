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
static void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewstatus_onsequenceselectionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewbar_ondefaultline(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_initalign(PatternViewBar*);
static void patternviewbar_onconfigchanged(PatternViewBar*, Workspace*,
	psy_Property*);
static void patternviewbar_onmovecursorwhenpaste(PatternViewBar*,
	psy_ui_Component* sender);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(PatternViewStatus* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondraw = (psy_ui_fp_ondraw) patternviewstatus_ondraw;
		vtable_initialized = 1;
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
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(40),
			psy_ui_value_makeeh(1)));
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
	psy_audio_PatternEditPosition editposition;
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
	psy_ui_settextcolor(g, psy_ui_color_make(0x00D1C5B6));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_snprintf(text, 256, "Pat %d  Ln %d  Trk %d  Col %d:%d Edit",
		pattern,
		editposition.line,
		editposition.track,
		editposition.column,
		editposition.digit);
	psy_ui_textout(g, 0, (psy_ui_value_px(&size.height, &tm) - tm.tmHeight) / 2, text, strlen(text));
}

// PatternViewBar
static void patternviewbar_updatetext(PatternViewBar*, Translator*);
static void patternviewbar_onlanguagechanged(PatternViewBar*, Translator* sender);

void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);		
	stepbox_init(&self->step, &self->component, workspace);
	psy_ui_checkbox_init(&self->movecursorwhenpaste, &self->component);	
	psy_signal_connect(&self->movecursorwhenpaste.signal_clicked, self,
		patternviewbar_onmovecursorwhenpaste);
	psy_ui_checkbox_init(&self->defaultentries, &self->component);
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
	psy_signal_connect(&workspace->signal_languagechanged, self,
		patternviewbar_onlanguagechanged);
	patternviewbar_updatetext(self, &self->workspace->translator);
}

void patternviewbar_updatetext(PatternViewBar* self, Translator* translator)
{
	psy_ui_checkbox_settext(&self->movecursorwhenpaste,
		translator_translate(translator, "settingsview.move-cursor-when-paste"));
	psy_ui_checkbox_settext(&self->defaultentries,
		translator_translate(translator, "settingsview.default-line"));
}

void patternviewbar_onlanguagechanged(PatternViewBar* self, Translator* sender)
{
	patternviewbar_updatetext(self, sender);
}

void patternviewbar_ondefaultline(PatternViewBar* self, psy_ui_CheckBox* sender)
{
	psy_Property* pv;

	pv = psy_property_findsection(&self->workspace->config, "visual.patternview");
	if (pv) {
		psy_Property* p;
		
		p = psy_property_at(pv, "griddefaults", PSY_PROPERTY_TYPE_NONE);
		if (p) {			
			psy_property_set_bool(pv, "griddefaults", !psy_property_as_int(p));
			psy_signal_emit(&self->workspace->signal_configchanged, self->workspace, 1, p);
		}
	}
}

void patternviewbar_initalign(PatternViewBar* self)
{		
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));			
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));		
}


// PatternView
static void patternview_ondestroy(PatternView* self, psy_ui_Component* sender);
static void patternview_onpreferredsize(PatternView*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable patternview_vtable;
static int patternview_vtable_initialized = 0;

static void patternview_vtable_init(PatternView* self)
{
	if (!patternview_vtable_initialized) {
		patternview_vtable = *(self->component.vtable);
		patternview_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			patternview_onpreferredsize;
		patternview_vtable_initialized = 1;
	}
}

void patternview_init(PatternView* self, 
		psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,		
		Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin leftmargin;
	
	psy_ui_component_init(&self->component, parent);
	patternview_vtable_init(self);
	self->component.vtable = &patternview_vtable;
	psy_ui_margin_init_all(&leftmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(3));
	self->workspace = workspace;	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_focus, self, patternview_onfocus);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(&self->notebook),
		psy_ui_BACKGROUND_NONE);
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(&self->notebook));
	psy_ui_component_setbackgroundmode(&self->editnotebook.component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_notebook_setpageindex(&self->editnotebook, 0);
	patternproperties_init(&self->properties, &self->component, NULL, workspace);
	psy_ui_component_setalign(&self->properties.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->properties.component);
	// Skin
	patternviewskin_init(&self->skin);
	// TrackerView
	trackerview_init(&self->trackerview, &self->editnotebook.component,
		&self->component, &self->skin, workspace);
	// PianoRoll
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, &self->skin,
		workspace);	
	patternview_setpattern(self, patterns_at(&workspace->song->patterns, 0));	
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self,
		patternview_onpropertiesapply);	
	// Tabbar
	psy_ui_component_init(&self->sectionbar, tabbarparent);
	psy_ui_component_enablealign(&self->sectionbar);
	psy_ui_component_setalign(&self->sectionbar, psy_ui_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->sectionbar);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);	
	tabbar_append_tabs(&self->tabbar, "Tracker", "Pianoroll", "Split", "Vertical", "Horizontal",
		"Properties", NULL);
	tabbar_tab(&self->tabbar, 2)->mode = TABMODE_LABEL;
	tabbar_tab(&self->tabbar, 5)->istoggle = TRUE;
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);	
	tabbar_select(&self->tabbar, 0);
	psy_ui_button_init(&self->contextbutton, &self->sectionbar);
	psy_ui_button_seticon(&self->contextbutton, psy_ui_ICON_MORE);
	psy_ui_component_setalign(psy_ui_button_base(&self->contextbutton), psy_ui_ALIGN_RIGHT);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(-1.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(1));
	psy_ui_component_setmargin(psy_ui_button_base(&self->contextbutton), &margin);
	psy_signal_connect(&self->component.signal_destroy, self,
		patternview_ondestroy);
	psy_signal_connect(&self->contextbutton.signal_clicked,
		self, patternview_oncontextmenu);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternview_onsequenceselectionchanged);
	psy_ui_component_setmargin(&self->trackerview.linenumbers.component,
		&leftmargin);
	psy_ui_component_setmargin(&self->trackerview.linenumberslabel.component,
		&leftmargin);
}

void patternview_ondestroy(PatternView* self, psy_ui_Component* sender)
{
	patternviewskin_dispose(&self->skin);
}

void patternview_ontabbarchange(PatternView* self, psy_ui_Component* sender,
	int tabindex)
{
	if (tabindex < 2) {
		if (psy_ui_notebook_splitactivated(&self->editnotebook)) {
			psy_ui_notebook_full(&self->editnotebook);						
		}		
		if (tabindex == 0) {
			if (workspace_showlinenumbers(self->workspace)) {
				psy_ui_component_show(&self->trackerview.left);
				psy_ui_component_align(&self->component);
			}
		} else {
			if (workspace_showlinenumbers(self->workspace)) {
				psy_ui_component_hide(&self->trackerview.left);
				psy_ui_component_align(&self->component);
			}
		}
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_ui_notebook_setpageindex(&self->editnotebook, tabindex);		
	} else 
	if (tabindex == 3) {		
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {			
			if (workspace_showlinenumbers(self->workspace)) {
				psy_ui_component_show(&self->trackerview.left);
				psy_ui_component_align(&self->component);
			}								
		}
		psy_ui_notebook_split(&self->editnotebook, psy_ui_VERTICAL);
	} else
	if (tabindex == 4) {
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {
			if (workspace_showlinenumbers(self->workspace)) {
				psy_ui_component_show(&self->trackerview.left);
				psy_ui_component_align(&self->component);
			}
		}
		psy_ui_notebook_split(&self->editnotebook, psy_ui_HORIZONTAL);
	} else
	if (tabindex == 5) {
		// Properties
		if (psy_ui_component_visible(&self->properties.component)) {
			psy_ui_component_hide(&self->properties.component);			
		} else {
			psy_ui_component_show(&self->properties.component);
		}
		psy_ui_component_align(&self->component);
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
	self->trackerview.linestate.sequenceentryoffset = 0.f;
	self->pianoroll.grid.sequenceentryoffset = 0.f;
	pianoroll_setpattern(&self->pianoroll, pattern);	
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
		self->trackerview.linestate.sequenceentryoffset = entry->offset;
		self->pianoroll.grid.sequenceentryoffset = entry->offset;
	} else {
		patternview_setpattern(self, NULL);		
		self->trackerview.linestate.sequenceentryoffset = 0.f;
		self->pianoroll.grid.sequenceentryoffset = 0.f;
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
	trackerview_toggleblockmenu(&self->trackerview);	
}

void patternviewbar_onmovecursorwhenpaste(PatternViewBar* self, psy_ui_Component* sender)
{
	workspace_movecursorwhenpaste(self->workspace, psy_ui_checkbox_checked(&self->movecursorwhenpaste));
}

void patternviewbar_onconfigchanged(PatternViewBar* self, Workspace* workspace,
	psy_Property* property)
{
	if (workspace_ismovecursorwhenpaste(workspace)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}
}

void patternview_onpreferredsize(PatternView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	*rv = psy_ui_component_size(&self->component);
}