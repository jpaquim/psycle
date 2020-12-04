// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternview.h"
// audio
#include <songio.h>
// std
#include <string.h>
// platform
#include "../../detail/portable.h"

// PatternViewBar
// prototypes
static void patternviewbar_ondefaultline(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_initalign(PatternViewBar*);
static void patternviewbar_onconfigchanged(PatternViewBar*, Workspace*,
	psy_Property*);
static void patternviewbar_onmovecursorwhenpaste(PatternViewBar*,
	psy_ui_Component* sender);
static void patternviewbar_updatetext(PatternViewBar*, psy_Translator*);
static void patternviewbar_onlanguagechanged(PatternViewBar*, psy_Translator* sender);
static void patternviewbar_onpatterncursorchanged(PatternViewBar*,
	Workspace* sender);
static void patternviewbar_onsequenceselectionchanged(PatternViewBar*,
	Workspace* sender);
static void patternviewbar_onsongchanged(PatternViewBar*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void patternviewbar_updatestatus(PatternViewBar*);
// implementation
void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);		
	stepbox_init(&self->cursorstep, &self->component, workspace);
	psy_ui_checkbox_init(&self->movecursorwhenpaste, &self->component);	
	psy_signal_connect(&self->movecursorwhenpaste.signal_clicked, self,
		patternviewbar_onmovecursorwhenpaste);
	psy_ui_checkbox_init(&self->defaultentries, &self->component);
	if (workspace_showgriddefaults(self->workspace)) {
		psy_ui_checkbox_check(&self->defaultentries);
	}
	psy_signal_connect(&self->defaultentries.signal_clicked, self,
		patternviewbar_ondefaultline);
	psy_ui_label_init(&self->status, &self->component);
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 40);
	patternviewbar_initalign(self);
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		patternviewbar_onconfigchanged);
	psy_signal_connect(&workspace->signal_patterncursorchanged, self,
		patternviewbar_onpatterncursorchanged);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternviewbar_onsequenceselectionchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternviewbar_onsongchanged);
	if (workspace_ismovecursorwhenpaste(workspace)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}
	psy_signal_connect(&workspace->signal_languagechanged, self,
		patternviewbar_onlanguagechanged);
	patternviewbar_updatestatus(self);
	patternviewbar_updatetext(self, workspace_translator(workspace));
}

void patternviewbar_updatetext(PatternViewBar* self, psy_Translator* translator)
{
	psy_ui_checkbox_settext(&self->movecursorwhenpaste,
		psy_translator_translate(translator, "settingsview.move-cursor-when-paste"));
	psy_ui_checkbox_settext(&self->defaultentries,
		psy_translator_translate(translator, "settingsview.default-line"));
}

void patternviewbar_onlanguagechanged(PatternViewBar* self, psy_Translator* sender)
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
			psy_property_set_bool(pv, "griddefaults", !psy_property_item_int(p));
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

void patternviewbar_onpatterncursorchanged(PatternViewBar* self,
	Workspace* sender)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_onsequenceselectionchanged(PatternViewBar* self,
	Workspace* sender)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_onsongchanged(PatternViewBar* self, Workspace* sender,
	int flag, psy_audio_SongFile* songfile)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_updatestatus(PatternViewBar* self)
{
	char text[256];
	psy_audio_PatternCursor cursor;
	psy_audio_SequencePosition sequenceposition;
	int patternid;

	cursor = workspace_patterncursor(self->workspace);
	sequenceposition = workspace_sequenceselection(self->workspace).editposition;
	patternid = psy_audio_sequenceposition_patternid(&sequenceposition);
	if (patternid == UINTPTR_MAX) {
		psy_snprintf(text, 256, "Pat --  Ln --  Trk --  Col --:-- Edit");
	} else {
		psy_snprintf(text, 256, "Pat %d  Ln %d  Trk %d  Col %d:%d Edit",
			patternid, cursor.line, cursor.track, cursor.column, cursor.digit);
	}
	psy_ui_label_settext(&self->status, text);
}

// PatternView
// prototypes
static void patternview_inittabbar(PatternView*, psy_ui_Component* tabbarparent);
static void patternview_ontabbarchange(PatternView*, psy_ui_Component* sender,
	int tabindex);
static void patternview_onsongchanged(PatternView*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void patternview_onsequenceselectionchanged(PatternView*,
	Workspace* sender);
static void patternview_onskinchanged(PatternView*, Workspace*);
static void patternview_onconfigurationchanged(PatternView*, Workspace*,
	psy_Property*);
static void patternview_configure(PatternView*);
static void patternview_readfont(PatternView*);
static void patternview_onpropertiesapply(PatternView*,
	psy_ui_Component* sender);
static void patternview_onfocus(PatternView*, psy_ui_Component* sender);
static void patternview_oncontextmenu(PatternView*,
	psy_ui_Component* sender);
static void patternview_ondestroy(PatternView* self, psy_ui_Component* sender);
static void patternview_onpreferredsize(PatternView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void patternview_onsequencechanged(PatternView*, psy_audio_Sequence*
	sender);
static void patternview_onpatterncursorchanged(PatternView* self,
	Workspace* sender);
static void patternview_onalign(PatternView*);
static void patternview_computemetrics(PatternView*);
static void patternview_ongridscroll(PatternView*, psy_ui_Component* sender);
static void patternview_onzoomboxchanged(PatternView*, ZoomBox* sender);
static void patternview_setfont(PatternView*, psy_ui_Font* font, bool iszoombase);
static void patternview_onlpbchanged(PatternView*, psy_audio_Player* sender,
	uintptr_t lpb);
static void patternview_ontimer(PatternView*, uintptr_t timerid);
static void patternview_updateksin(PatternView*);
static void patternview_readtheme(PatternView*);
static void patternview_oncolresize(PatternView*, TrackerGrid* sender);
static void patternview_numtrackschanged(PatternView*, psy_audio_Player* player,
	uintptr_t numsongtracks);

// vtable
static psy_ui_ComponentVtable patternview_vtable;
static int patternview_vtable_initialized = 0;

static void patternview_vtable_init(PatternView* self)
{
	if (!patternview_vtable_initialized) {
		patternview_vtable = *(self->component.vtable);
		patternview_vtable.ontimer = (psy_ui_fp_component_ontimer)
			patternview_ontimer;
		patternview_vtable.onalign = (psy_ui_fp_component_onalign)
			patternview_onalign;
		patternview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			patternview_onpreferredsize;
		patternview_vtable_initialized = 1;
	}
}
// implementation
void patternview_init(PatternView* self, 
		psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,		
		Workspace* workspace)
{
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent);
	patternview_vtable_init(self);
	self->component.vtable = &patternview_vtable;	
	self->workspace = workspace;
	self->showlinenumbers = TRUE;
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
	patternviewskin_settheme(&self->skin,
		workspace_patternviewtheme(workspace),
		workspace_skins_directory(workspace));
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		patternview_onskinchanged);
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		patternview_onconfigurationchanged);
	// shared states
	trackconfig_init(&self->trackconfig, workspace_showwideinstcolumn(workspace) != FALSE);
	trackerlinestate_init(&self->linestate);
	self->linestate.skin = &self->skin;
	trackergridstate_init(&self->gridstate, &self->trackconfig);
	self->gridstate.skin = &self->skin;
	// linenumbers
	trackerlinenumberbar_init(&self->left, &self->component, &self->linestate, &self->trackerview,
		self->workspace);
	psy_ui_component_setalign(&self->left.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->left.zoombox.signal_changed, self,
		patternview_onzoomboxchanged);	
	// header
	trackerheader_init(&self->header, &self->component,
		&self->trackconfig, &self->gridstate, self->workspace);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// pattern default line
	trackergrid_init(&self->griddefaults, &self->component, &self->trackconfig,
		NULL, &self->linestate, TRACKERGRID_EDITMODE_LOCAL, workspace);
	self->griddefaults.defaultgridstate.skin = &self->skin;
	psy_ui_component_setalign(&self->griddefaults.component, psy_ui_ALIGN_TOP);
	self->griddefaults.columnresize = 1;
	trackergrid_setpattern(&self->griddefaults, &self->workspace->player.patterndefaults);
	psy_signal_connect(&self->griddefaults.signal_colresize, self,
		patternview_oncolresize);
	// TrackerView	
	trackerview_init(&self->trackerview, &self->editnotebook.component,
		&self->gridstate, &self->linestate, &self->trackconfig, self, workspace);
	psy_signal_connect(&self->trackerview.grid.signal_colresize, self,
		patternview_oncolresize);
	// PianoRoll
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, &self->skin,
		workspace);	
	patternview_setpattern(self, psy_audio_patterns_at(&workspace->song->patterns, 0));	
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self,
		patternview_onpropertiesapply);	
	// Tabbar
	patternview_inittabbar(self, tabbarparent);
	patternview_selectdisplay(self, PATTERNDISPLAY_TRACKER);
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
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		patternview_onpatterncursorchanged);
	psy_signal_connect(&self->trackerview.grid.component.signal_scroll, self,
		patternview_ongridscroll);
	psy_signal_connect(&self->workspace->player.signal_lpbchanged, self,
		patternview_onlpbchanged);
	psy_signal_connect(&self->workspace->player.signal_numsongtrackschanged, self,
		patternview_numtrackschanged);
	if (workspace->song) {
		psy_signal_connect(&workspace->song->sequence.sequencechanged,
			self, patternview_onsequencechanged);
	}
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void patternview_ondestroy(PatternView* self, psy_ui_Component* sender)
{
	trackerlinestate_dispose(&self->linestate);
	trackergridstate_dispose(&self->gridstate);
	trackconfig_dispose(&self->trackconfig);
	patternviewskin_dispose(&self->skin);
}

void patternview_inittabbar(PatternView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_component_init(&self->sectionbar, tabbarparent);
	psy_ui_component_enablealign(&self->sectionbar);
	psy_ui_component_setalign(&self->sectionbar, psy_ui_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->sectionbar);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "Tracker", "Pianoroll", "Split", "Vertical", "Horizontal",
		"Properties", NULL);
	tabbar_tab(&self->tabbar, 2)->mode = TABMODE_LABEL;
	tabbar_tab(&self->tabbar, 5)->istoggle = TRUE;
	tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);
}

void patternview_ontabbarchange(PatternView* self, psy_ui_Component* sender,
	int tabindex)
{	
	if (tabindex == 5) {
		// Properties
		if (psy_ui_component_visible(&self->properties.component)) {
			psy_ui_component_hide(&self->properties.component);
		} else {
			psy_ui_component_show(&self->properties.component);
		}
		psy_ui_component_align(&self->component);
	} else {
		PatternDisplayType display;

		switch (tabindex) {
		case 0:
			display = PATTERNDISPLAY_TRACKER;
			break;
		case 1:
			display = PATTERNDISPLAY_PIANOROLL;
			break;
		case 3:
			display = PATTERNDISPLAY_TRACKER_PIANOROLL_VERTICAL;
			break;
		case 4:
			display = PATTERNDISPLAY_TRACKER_PIANOROLL_HORIZONTAL;
			break;
		default:
			display = PATTERNDISPLAY_NUM;
			break;
		}
		if (display != PATTERNDISPLAY_NUM) {
			workspace_selectpatterndisplay(self->workspace, display);
		}
	}
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{		
	trackerview_setpattern(&self->trackerview, pattern);
	pianoroll_setpattern(&self->pianoroll, pattern);
	patternproperties_setpattern(&self->properties, pattern);
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_invalidate(&self->griddefaults.component);
}

void patternview_onsongchanged(PatternView* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	psy_audio_Pattern* pattern;
	psy_audio_SequenceSelection selection;	
	
	selection = workspace_sequenceselection(workspace);	
	if (selection.editposition.trackposition.sequencentrynode) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)
			selection.editposition.trackposition.sequencentrynode->entry;
		pattern = psy_audio_patterns_at(&workspace->song->patterns, entry->patternslot);
	} else {
		pattern = 0;
	}
	patternview_setpattern(self, pattern);
	self->linestate.sequenceentryoffset = 0.f;
	self->pianoroll.grid.sequenceentryoffset = 0.f;
	pianoroll_setpattern(&self->pianoroll, pattern);
	if (workspace->song) {
		psy_signal_connect(&workspace->song->sequence.sequencechanged,
			self, patternview_onsequencechanged);
	}
	psy_ui_component_invalidate(&self->component);
}

void patternview_onsequenceselectionchanged(PatternView* self,
	Workspace* workspace)
{	
	psy_audio_SequenceSelection selection;
	psy_audio_SequenceEntry* entry;

	selection = workspace_sequenceselection(workspace);
	entry = psy_audio_sequenceposition_entry(&selection.editposition);
	if (entry) {
		psy_audio_Pattern* pattern;

		pattern = psy_audio_patterns_at(&workspace->song->patterns, 
			entry->patternslot);
		patternview_setpattern(self, pattern);
		self->linestate.sequenceentryoffset = entry->offset;
		self->pianoroll.grid.sequenceentryoffset = entry->offset;
	} else {
		patternview_setpattern(self, NULL);		
		self->linestate.sequenceentryoffset = 0.f;
		self->pianoroll.grid.sequenceentryoffset = 0.f;
	}
	psy_ui_component_invalidate(&self->component);
}

void patternview_onsequencechanged(PatternView* self, psy_audio_Sequence* sender)
{
	psy_ui_component_updateoverflow(&self->left.linenumbers.component);
	psy_ui_component_updateoverflow(&self->trackerview.grid.component);
	psy_ui_component_updateoverflow(&self->pianoroll.grid.component);
	psy_ui_component_invalidate(&self->trackerview.grid.component);
	psy_ui_component_invalidate(&self->pianoroll.grid.component);
	psy_ui_component_invalidate(&self->left.linenumbers.component);
}

void patternview_onpropertiesapply(PatternView* self, psy_ui_Component* sender)
{
	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onfocus(PatternView* self, psy_ui_Component* sender)
{
	if (tabbar_selected(&self->tabbar) == 1) { // Pianoroll
		psy_ui_component_setfocus(&self->pianoroll.grid.component);
		return;
	}
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

void patternview_selectdisplay(PatternView* self, PatternDisplayType display)
{	
	int tabindex;

	switch (display) {
		case PATTERNDISPLAY_TRACKER:
			tabindex = 0;
			break;
		case PATTERNDISPLAY_PIANOROLL:
			tabindex = 1;
			break;
		case PATTERNDISPLAY_TRACKER_PIANOROLL_VERTICAL:
			tabindex = 3;
			break;
		case PATTERNDISPLAY_TRACKER_PIANOROLL_HORIZONTAL:
			tabindex = 4;
			break;
		default:
			tabindex = 0;
			break;
	}
	if (tabindex < 2) {
		if (psy_ui_notebook_splitactivated(&self->editnotebook)) {
			psy_ui_notebook_full(&self->editnotebook);
		}		
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_ui_notebook_setpageindex(&self->editnotebook, tabindex);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, tabindex);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);		
	} else if (tabindex == 3) {
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, 0);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {
			if (workspace_showlinenumbers(self->workspace)) {
				psy_ui_component_show(&self->left.component);
				psy_ui_component_align(&self->component);
			}
		}
		psy_ui_notebook_split(&self->editnotebook, psy_ui_VERTICAL);
	} else if (tabindex == 4) {
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, 0);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {
			if (workspace_showlinenumbers(self->workspace)) {
				psy_ui_component_show(&self->left.component);
				psy_ui_component_align(&self->component);
			}
		}
		psy_ui_notebook_split(&self->editnotebook, psy_ui_HORIZONTAL);
	} else {
		psy_ui_notebook_setpageindex(&self->notebook, 1);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, 1);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
	}
}

void patternview_onconfigurationchanged(PatternView* self, Workspace* sender,
	psy_Property* property)
{
	if (psy_property_insection(property, sender->patternviewtheme)) {
		patternviewskin_settheme(&self->skin,
			workspace_patternviewtheme(sender),
			workspace_skins_directory(sender));
		patternview_updateksin(self);
		if (strcmp(psy_property_key(property), "wideinstcolumn") == 0) {
			trackconfig_initcolumns(&self->trackconfig, psy_property_item_int(property));
			patternview_computemetrics(self);
		}
	}
	patternview_configure(self);
	if (strcmp(psy_property_key(property), "font") == 0) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_item_str(property));
		psy_ui_font_init(&font, &fontinfo);
		patternview_setfont(self, &font, TRUE);
		psy_ui_font_dispose(&font);
		psy_ui_component_align(&self->component);
	}
	if (strcmp(psy_property_key(property), "griddefaults") == 0) {
		self->trackerview.showdefaultline = psy_property_item_int(property);
		if (psy_ui_component_visible(&self->griddefaults.component) != self->trackerview.showdefaultline) {
			psy_ui_component_togglevisibility(&self->griddefaults.component);
			psy_ui_component_align(&self->left.component);
		}
	} else if (strcmp(psy_property_key(property), "notetab") == 0) {
		self->trackerview.grid.notestabmode = self->griddefaults.notestabmode =
			workspace_notetabmode(self->workspace);
	}
}

void patternview_configure(PatternView* self)
{
	psy_Property* p;

	p = psy_property_findsection(&self->workspace->config, "visual.patternview");
	if (p) {
		patternview_showlinenumbers(self, psy_property_at_bool(p, "linenumbers", 1));
		trackerlinenumbers_showlinenumbercursor(&self->left.linenumbers,
			psy_property_at_bool(p, "linenumberscursor", 1));
		trackerlinenumbers_showlinenumbersinhex(&self->left.linenumbers,
			psy_property_at_bool(p, "linenumbersinhex", 1));
		patternview_readfont(self);
		trackconfig_initcolumns(&self->trackconfig,
			workspace_showwideinstcolumn(self->workspace));
		patternview_computemetrics(self);
		if (psy_property_at_bool(p, "griddefaults", 1)) {
			self->trackerview.showdefaultline = 1;
			psy_ui_component_show(&self->griddefaults.component);
		} else {
			self->trackerview.showdefaultline = 0;
			psy_ui_component_hide(&self->griddefaults.component);
		}
		self->trackerview.grid.wraparound = psy_property_at_bool(p, "wraparound", 1);
		trackergrid_showemptydata(&self->trackerview.grid, psy_property_at_bool(p, "drawemptydata", 1));
		trackergrid_setcentermode(&self->trackerview.grid, psy_property_at_bool(p, "centercursoronscreen", 1));
		self->trackerview.grid.notestabmode = self->griddefaults.notestabmode =
			(psy_property_at_bool(p, "notetab", 0))
			? psy_dsp_NOTESTAB_A440
			: psy_dsp_NOTESTAB_A220;
	}
	patternview_readtheme(self);	
}

void patternview_readfont(PatternView* self)
{
	psy_Property* pv;

	pv = psy_property_findsection(&self->workspace->config, "visual.patternview");
	if (pv) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));
		psy_ui_font_init(&font, &fontinfo);
		patternview_setfont(self, &font, TRUE);
		psy_ui_font_dispose(&font);
	}
}

void patternview_onskinchanged(PatternView* self, Workspace* sender)
{
	patternviewskin_settheme(&self->skin,
		workspace_patternviewtheme(sender),
		workspace_skins_directory(sender));
	patternview_updateksin(self);
}

void patternview_onalign(PatternView* self)
{
	patternview_computemetrics(self);
}

void patternview_computemetrics(PatternView* self)
{
	psy_ui_Size gridsize;
	psy_ui_TextMetric gridtm;
	int trackwidth;

	gridsize = psy_ui_component_size(&self->trackerview.grid.component);
	gridtm = psy_ui_component_textmetric(&self->trackerview.grid.component);
	self->gridstate.trackconfig->textwidth = (int)(gridtm.tmAveCharWidth * 1.5) + 2;
	self->linestate.lineheight = gridtm.tmHeight + 1;
	trackwidth = psy_max(
		trackergridstate_preferredtrackwidth(&self->gridstate),
		trackergridstate_preferredtrackwidth(&self->gridstate));
	self->trackconfig.patterntrackident = (trackwidth -
		trackergridstate_preferredtrackwidth(&self->gridstate)) / 2;
	if (self->trackconfig.patterntrackident < 0) {
		self->trackconfig.patterntrackident = 0;
	}
	self->trackconfig.headertrackident = 0;
	self->linestate.visilines = psy_ui_value_px(&gridsize.height, &gridtm) /
		self->linestate.lineheight;
}

void patternview_ongridscroll(PatternView* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleft(&self->trackerview.grid.component) !=
			psy_ui_component_scrollleft(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->trackerview.grid.component));
		psy_ui_component_setscrollleft(&self->griddefaults.component,
			psy_ui_component_scrollleft(&self->trackerview.grid.component));
	}
	if (psy_ui_component_scrolltop(&self->trackerview.grid.component) !=
			psy_ui_component_scrolltop(&self->left.linenumbers.component)) {
		psy_ui_component_setscrolltop(&self->left.linenumbers.component,
			psy_ui_component_scrolltop(&self->trackerview.grid.component));
	}
}

void patternview_showlinenumbers(PatternView* self, int showstate)
{	
	if (psy_ui_component_visible(&self->left.component) != showstate) {
		psy_ui_component_togglevisibility(&self->left.component);
		self->showlinenumbers = showstate;		
	}
}

void patternview_onzoomboxchanged(PatternView* self, ZoomBox* sender)
{
	psy_ui_Font* font;

	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font newfont;

		fontinfo = psy_ui_font_fontinfo(font);
		fontinfo.lfHeight = (int)(self->left.zoomheightbase * zoombox_rate(sender));
		psy_ui_font_init(&newfont, &fontinfo);
		patternview_setfont(self, &newfont, FALSE);
		psy_ui_font_dispose(&newfont);
		self->gridstate.zoom = zoombox_rate(sender);
		patternview_computemetrics(self);
		psy_ui_component_align(&self->left.component);
		psy_ui_component_align(&self->component);
		psy_ui_component_updateoverflow(&self->trackerview.grid.component);
		psy_ui_component_invalidate(&self->trackerview.grid.component);
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_invalidate(&self->left.linenumbers.component);
		psy_ui_component_invalidate(&self->left.linenumberslabel.component);
	}
}

void patternview_setfont(PatternView* self, psy_ui_Font* font, bool iszoombase)
{
	psy_ui_component_setfont(&self->trackerview.component, font);
	psy_ui_component_setfont(&self->griddefaults.component, font);
	psy_ui_component_setfont(&self->trackerview.grid.component, font);
	psy_ui_component_setfont(&self->left.linenumbers.component, font);
	if (iszoombase) {
		trackerlinenumberbar_computefontheight(&self->left);
	}
	patternview_computemetrics(self);
	trackerview_updatescrollstep(&self->trackerview);
	psy_ui_component_align(&self->component);
}

void patternview_onpatterncursorchanged(PatternView* self, Workspace* sender)
{
	psy_audio_PatternCursor oldcursor;

	oldcursor = self->gridstate.cursor;
	self->gridstate.cursor = workspace_patterncursor(sender);
	if (!psy_audio_patterncursor_equal(&self->gridstate.cursor, &oldcursor)) {
		if (psy_audio_player_playing(&sender->player) && workspace_followingsong(sender)) {
			bool scrolldown;

			scrolldown = self->linestate.lastplayposition <
				psy_audio_player_position(&self->workspace->player);
			trackergrid_invalidateline(&self->trackerview.grid, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->left.linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = psy_audio_player_position(&self->workspace->player);
			trackergrid_invalidateline(&self->trackerview.grid, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->left.linenumbers,
				self->linestate.lastplayposition);
			if (self->linestate.lastplayposition >= self->linestate.sequenceentryoffset &&
				self->linestate.lastplayposition < self->linestate.sequenceentryoffset +
				self->gridstate.pattern->length) {
				if (scrolldown != FALSE) {
					trackergrid_scrolldown(&self->trackerview.grid, self->gridstate.cursor);
				} else {
					trackergrid_scrollup(&self->trackerview.grid, self->gridstate.cursor);
				}
			}
			trackergrid_storecursor(&self->trackerview.grid);
		} else if (self->trackerview.grid.midline) {
			trackergrid_centeroncursor(&self->trackerview.grid);
		}
	}
}

void patternview_onlpbchanged(PatternView* self, psy_audio_Player* sender, uintptr_t lpb)
{
	self->linestate.lpb = lpb;
	self->trackerview.grid.bpl = 1 / (psy_dsp_big_beat_t)lpb;
	psy_ui_component_updateoverflow(&self->trackerview.grid.component);
	psy_ui_component_invalidate(&self->trackerview.grid.component);
	psy_ui_component_invalidate(&self->left.linenumbers.component);
}

void patternview_ontimer(PatternView* self, uintptr_t timerid)
{
	if (self->gridstate.pattern) {
		if (psy_audio_player_playing(&self->workspace->player)) {
			if (!workspace_followingsong(self->workspace)) {
				trackergrid_invalidateline(&self->trackerview.grid, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->left.linenumbers,
					self->linestate.lastplayposition);
				self->linestate.lastplayposition =
					psy_audio_player_position(&self->workspace->player);
				trackergrid_invalidateline(&self->trackerview.grid, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->left.linenumbers,
					self->linestate.lastplayposition);
			}
			psy_ui_component_invalidate(&self->header.component);
		} else if (self->linestate.lastplayposition != -1) {
			trackergrid_invalidateline(&self->trackerview.grid,
				self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->left.linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = -1;
		}
		if (self->gridstate.pattern && self->gridstate.pattern->opcount != self->trackerview.grid.opcount &&
			self->trackerview.grid.syncpattern) {
			psy_ui_component_invalidate(&self->trackerview.grid.component);
			psy_ui_component_invalidate(&self->left.linenumbers.component);
		}
		self->trackerview.grid.opcount = (self->gridstate.pattern)
			? self->gridstate.pattern->opcount
			: 0;
	}
}

void patternview_updateksin(PatternView* self)
{
	psy_ui_component_setbackgroundcolour(&self->left.linenumbers.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerview.blockmenu.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerview.transformview.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerview.interpolatecurveview.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setcolour(&self->trackerview.interpolatecurveview.component,
		patternviewskin_fontcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerview.scroller.hscroll.sliderpane.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerview.scroller.vscroll.sliderpane.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_invalidate(&self->component);
}

void patternview_readtheme(PatternView* self)
{
	patternview_updateksin(self);
	if (self->trackerview.grid.midline) {
		trackergrid_centeroncursor(&self->trackerview.grid);
	}
}

void patternview_oncolresize(PatternView* self, TrackerGrid* sender)
{
	if (sender != &self->griddefaults) {
		psy_ui_component_invalidate(&self->griddefaults.component);
	}
	if (sender != &self->trackerview.grid) {
		psy_ui_component_invalidate(&self->trackerview.grid.component);
	}
	psy_ui_component_invalidate(&self->header.component);
}

void patternview_numtrackschanged(PatternView* self, psy_audio_Player* player,
	uintptr_t numsongtracks)
{
	self->griddefaults.gridstate->numtracks = numsongtracks;
	psy_ui_component_invalidate(&self->griddefaults.component);
}
