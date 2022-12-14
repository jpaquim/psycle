// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "transformpatternview.h"
/* audio */
#include <sequencecursor.h>
/* platform */
#include "../../detail/portable.h"

static const char notes[12][3] = { "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-" };
static const char* empty = "Empty";
static const char* nonempty = "Nonempty";
static const char* all = "All";
static const char* same = "Same";
static const char* off = "off";
static const char* twk = "twk";
static const char* tws = "tws";
static const char* mcm = "mcm";

// prototypes
static void transformpatternview_on_destroyed(TransformPatternView*);
static void transformpatternview_init_search(TransformPatternView*);
static void transformpatternview_init_replace(TransformPatternView*);
static void transformpatternview_init_searchon(TransformPatternView*);
static void transformpatternview_init_actions(TransformPatternView*);
static void transformpatternview_initselection(TransformPatternView*);
static void transformpatternview_applyto(TransformPatternView*,
	int index);
static void transformpatternview_onsearchon_mouse_down(TransformPatternView*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void transformpatternview_onsearch(TransformPatternView*);
static void transformpatternview_searchentiresong(TransformPatternView*,
	psy_audio_PatternSearchReplaceMode);
static void transformpatternview_searchpattern(TransformPatternView*,
	psy_audio_PatternSearchReplaceMode);
static void transformpatternview_searchcurrentselection(TransformPatternView*,
	psy_audio_PatternSearchReplaceMode);
static void transformpatternview_onreplace(TransformPatternView*);
static void transformpatternview_onhide(TransformPatternView*);
static psy_audio_PatternSearchReplaceMode setupsearchreplacemode(
	int searchnote, int searchinst, int searchmach,
	int replnote, int replinst, int replmach, bool repltweak);
static psy_audio_Pattern* transformpatternview_currpattern(TransformPatternView*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(TransformPatternView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			transformpatternview_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void transformpatternview_init(TransformPatternView* self, psy_ui_Component*
	parent, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(transformpatternview_base(self), parent, NULL);
	vtable_init(self);
	self->workspace = workspace;
	self->applyto = 0;
	psy_audio_blockselection_init(&self->patternselection);
	psy_ui_margin_init_em(&self->sectionmargin, 0.0, 0.0, 0.0, 2.0);
	psy_ui_component_set_default_align(transformpatternview_base(self),
		psy_ui_ALIGN_TOP, psy_ui_margin_make(psy_ui_value_make_px(0),
			psy_ui_value_make_ew(0.0), psy_ui_value_make_eh(0.5),
			psy_ui_value_make_ew(2.0)));
	transformpatternview_init_search(self);
	transformpatternview_init_replace(self);
	transformpatternview_init_searchon(self);
	transformpatternview_init_actions(self);
	transformpatternview_initselection(self);	
	psy_ui_component_set_align(transformpatternview_base(self),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(transformpatternview_base(self));
}

void transformpatternview_on_destroyed(TransformPatternView* self)
{
}

void transformpatternview_init_search(TransformPatternView* self)
{
	assert(self);
	
	psy_ui_component_init(&self->search, transformpatternview_base(self), NULL);
	psy_ui_component_set_default_align(&self->search,
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->searchtop, &self->search, NULL);
	psy_ui_component_set_align(&self->searchtop, psy_ui_ALIGN_TOP);
	/* title */
	psy_ui_label_init(&self->searchtitle, &self->searchtop);
	psy_ui_label_set_text(&self->searchtitle,
		"transformpattern.searchpattern");	
	psy_ui_component_set_align(psy_ui_label_base(&self->searchtitle), psy_ui_ALIGN_LEFT);
	/* hide button */
	psy_ui_button_init_connect(&self->hide, &self->searchtop,
		self, transformpatternview_onhide);
	psy_ui_button_set_text(&self->hide, "X");
	psy_ui_component_set_align(psy_ui_button_base(&self->hide), psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_default_align(&self->search, psy_ui_ALIGN_TOP,
		self->sectionmargin);
	/* Note */
	psy_ui_label_init(&self-> searchnotedesc, &self->search);
	psy_ui_label_set_text(&self->searchnotedesc, "transformpattern.note");
	psy_ui_combobox_init(&self->searchnote, &self->search);	
	/* Inst */
	psy_ui_label_init(&self->searchinstdesc, &self->search);
	psy_ui_label_set_text(&self->searchinstdesc, "transformpattern.instr");
	psy_ui_combobox_init(&self->searchinst, &self->search);
	/* Mach */
	psy_ui_label_init(&self-> searchmachdesc, &self->search);
	psy_ui_label_set_text(&self->searchmachdesc, "transformpattern.mac");	
	psy_ui_combobox_init(&self->searchmach, &self->search);
}

void transformpatternview_init_replace(TransformPatternView* self)
{
	assert(self);

	psy_ui_component_init(&self->replace, transformpatternview_base(self), NULL);
	psy_ui_component_set_default_align(&self->replace,
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_label_init(&self->replacetitle, &self->replace);
	psy_ui_label_set_text(&self->replacetitle, "transformpattern.replacepattern");	
	psy_ui_component_set_default_align(&self->replace, psy_ui_ALIGN_TOP,
		self->sectionmargin);
	/* Note */
	psy_ui_label_init(&self->replacenotedesc, &self->replace);
	psy_ui_label_set_text(&self->replacenotedesc, "transformpattern.note");
	psy_ui_combobox_init(&self->replacenote, &self->replace);
	/* Inst */
	psy_ui_label_init(&self->replaceinstdesc, &self->replace);
	psy_ui_label_set_text(&self->replaceinstdesc, "transformpattern.instr");
	psy_ui_combobox_init(&self->replaceinst, &self->replace);
	/* Mach */
	psy_ui_label_init(&self->replacemachdesc, &self->replace);
	psy_ui_label_set_text(&self->replacemachdesc, "transformpattern.mac");
	psy_ui_combobox_init(&self->replacemach, &self->replace);
}

void transformpatternview_init_searchon(TransformPatternView* self)
{
	assert(self);

	psy_ui_component_init(&self->searchon, transformpatternview_base(self), NULL);
	psy_ui_component_set_default_align(&self->searchon,
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->searchontitle, &self->searchon,
		"transformpattern.searchon");
	psy_ui_component_init(&self->searchonchoice, &self->searchon, NULL);
	psy_signal_connect(&self->searchonchoice.signal_mousedown, self,
		transformpatternview_onsearchon_mouse_down);
	psy_ui_component_set_default_align(&self->searchonchoice,
		psy_ui_ALIGN_TOP, self->sectionmargin);
	psy_ui_label_init_text(&self->entire, &self->searchonchoice,
		"transformpattern.entiresong");
	psy_ui_label_init_text(&self->currpattern, &self->searchonchoice,
		"transformpattern.currentpattern");
	psy_ui_label_init_text(&self->currselection, &self->searchonchoice,
		"transformpattern.currentselection");
	psy_ui_component_prevent_input(psy_ui_label_base(&self->currselection),
		psy_ui_NONE_RECURSIVE);
	transformpatternview_applyto(self, 1);
}

void transformpatternview_init_actions(TransformPatternView* self)
{
	psy_ui_Margin margin;

	assert(self);

	psy_ui_component_init(&self->actions, transformpatternview_base(self), NULL);
	psy_ui_margin_init_em(&margin, 0.5, 2.0, 0.0, 2.0);		
	psy_ui_component_set_margin(&self->actions, margin);
	psy_ui_component_set_default_align(&self->actions,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	/* search */
	psy_ui_button_init_connect(&self->dosearch, &self->actions,
		self, transformpatternview_onsearch);
	psy_ui_button_set_text(&self->dosearch, "transformpattern.search");
	/* replace */
	psy_ui_button_init_connect(&self->doreplace, &self->actions,
		self, transformpatternview_onreplace);
	psy_ui_button_set_text(&self->doreplace, "transformpattern.replaceall");
	/* cancel */
	psy_ui_button_init_connect(&self->docancel, &self->actions,
		self, transformpatternview_onhide);	
	psy_ui_button_set_text(&self->docancel, "transformpattern.cancel");
	psy_ui_component_set_align(psy_ui_button_base(&self->docancel), psy_ui_ALIGN_RIGHT);
}

void transformpatternview_initselection(TransformPatternView* self)
{
	int i;
	bool is440;

	assert(self);

	// init maps
	//Note (search and replace)
	psy_ui_combobox_add_text(&self->searchnote, all);		psy_ui_combobox_setitemdata(&self->searchnote, 0, 1003);
	psy_ui_combobox_add_text(&self->searchnote, empty);		psy_ui_combobox_setitemdata(&self->searchnote, 1, 1001);
	psy_ui_combobox_add_text(&self->searchnote, nonempty);	psy_ui_combobox_setitemdata(&self->searchnote, 2, 1002);
	psy_ui_combobox_add_text(&self->replacenote, same);		psy_ui_combobox_setitemdata(&self->replacenote, 0, 1002);
	psy_ui_combobox_add_text(&self->replacenote, empty);	psy_ui_combobox_setitemdata(&self->replacenote, 1, 1001);
	is440 = patternviewconfig_notetabmode(&self->workspace->config.visual.patview) ==
		psy_dsp_NOTESTAB_A440;
	for (i = psy_audio_NOTECOMMANDS_C0; i <= psy_audio_NOTECOMMANDS_B9; i++) {
		char text[256];

		psy_snprintf(text, 256, "%s%d", notes[i % 12],
			(i / 12) + ((is440) ? - 1 : 0));
		psy_ui_combobox_add_text(&self->searchnote, text);	psy_ui_combobox_setitemdata(&self->searchnote, 3 + i, i);
		psy_ui_combobox_add_text(&self->replacenote, text);	psy_ui_combobox_setitemdata(&self->replacenote, 2 + i, i);
	}
	psy_ui_combobox_add_text(&self->searchnote, off);		psy_ui_combobox_setitemdata(&self->searchnote, 123, psy_audio_NOTECOMMANDS_RELEASE);
	psy_ui_combobox_add_text(&self->searchnote, twk);		psy_ui_combobox_setitemdata(&self->searchnote, 124, psy_audio_NOTECOMMANDS_TWEAK);
	psy_ui_combobox_add_text(&self->searchnote, tws);		psy_ui_combobox_setitemdata(&self->searchnote, 125, psy_audio_NOTECOMMANDS_TWEAKSLIDE);
	psy_ui_combobox_add_text(&self->searchnote, mcm);		psy_ui_combobox_setitemdata(&self->searchnote, 126, psy_audio_NOTECOMMANDS_MIDICC);
	psy_ui_combobox_add_text(&self->replacenote, off);		psy_ui_combobox_setitemdata(&self->replacenote, 122, psy_audio_NOTECOMMANDS_RELEASE);
	psy_ui_combobox_add_text(&self->replacenote, twk);		psy_ui_combobox_setitemdata(&self->replacenote, 123, psy_audio_NOTECOMMANDS_TWEAK);
	psy_ui_combobox_add_text(&self->replacenote, tws);		psy_ui_combobox_setitemdata(&self->replacenote, 124, psy_audio_NOTECOMMANDS_TWEAKSLIDE);
	psy_ui_combobox_add_text(&self->replacenote, mcm);		psy_ui_combobox_setitemdata(&self->replacenote, 125, psy_audio_NOTECOMMANDS_MIDICC);

	psy_ui_combobox_select(&self->searchnote, 0);
	psy_ui_combobox_select(&self->replacenote, 0);

	//Inst (search and replace)
	psy_ui_combobox_add_text(&self->searchinst, all);		psy_ui_combobox_setitemdata(&self->searchinst, 0, 1003);
	psy_ui_combobox_add_text(&self->searchinst, empty);		psy_ui_combobox_setitemdata(&self->searchinst, 1, 1001);
	psy_ui_combobox_add_text(&self->searchinst, nonempty);	psy_ui_combobox_setitemdata(&self->searchinst, 2, 1002);
	psy_ui_combobox_add_text(&self->replaceinst, same);		psy_ui_combobox_setitemdata(&self->replaceinst, 0, 1002);
	psy_ui_combobox_add_text(&self->replaceinst, empty);		psy_ui_combobox_setitemdata(&self->replaceinst, 1, 1001);
	for (i = 0; i < 0xFF; i++) {
		char text[256];
		
		psy_snprintf(text, 256, "%02X", i);				
		psy_ui_combobox_add_text(&self->searchinst, text);	psy_ui_combobox_setitemdata(&self->searchinst, 3 + i, i);
		psy_ui_combobox_add_text(&self->replaceinst, text);	psy_ui_combobox_setitemdata(&self->replaceinst, 2 + i, i);
	}
	psy_ui_combobox_select(&self->searchinst, 0);
	psy_ui_combobox_select(&self->replaceinst, 0);

	//Mach (search and replace)
	psy_ui_combobox_add_text(&self->searchmach, all);		psy_ui_combobox_setitemdata(&self->searchmach, 0, 1003);
	psy_ui_combobox_add_text(&self->searchmach, empty);		psy_ui_combobox_setitemdata(&self->searchmach, 1, 1001);
	psy_ui_combobox_add_text(&self->searchmach, nonempty);	psy_ui_combobox_setitemdata(&self->searchmach, 2, 1002);
	psy_ui_combobox_add_text(&self->replacemach, same);		psy_ui_combobox_setitemdata(&self->replacemach, 0, 1002);
	psy_ui_combobox_add_text(&self->replacemach, empty);		psy_ui_combobox_setitemdata(&self->replacemach, 1, 1001);
	for (i = 0; i < 0xFF; i++) {
		char text[256];

		psy_snprintf(text, 256, "%02X", i);
		psy_ui_combobox_add_text(&self->searchmach, text);	psy_ui_combobox_setitemdata(&self->searchmach, 3 + i, i);
		psy_ui_combobox_add_text(&self->replacemach, text);	psy_ui_combobox_setitemdata(&self->replacemach, 2 + i, i);
	}
	psy_ui_combobox_select(&self->searchmach, 0);
	psy_ui_combobox_select(&self->replacemach, 0);
}

void transformpatternview_onsearchon_mouse_down(TransformPatternView* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	psy_List* q;

	q = psy_ui_component_children(&self->searchonchoice, psy_ui_NONE_RECURSIVE);
	transformpatternview_applyto(self, (int)psy_list_entry_index(q,
		psy_ui_event_target(&ev->event)));
	psy_list_free(q);
}

void transformpatternview_applyto(TransformPatternView* self, int index)
{
	psy_List* p;
	psy_List* q;
	int c;

	if (index != -1 && index != 2 || self->patternselection.valid) {
		self->applyto = index;
	}	
	c = 0;
	for (p = q = psy_ui_component_children(&self->searchonchoice, psy_ui_NONE_RECURSIVE); p != NULL;
		psy_list_next(&p), ++c) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)psy_list_entry(p);		
		if (c == self->applyto) {			
			psy_ui_component_set_colour(component,
				psy_ui_colour_make(0x00B1C8B0));
		} else {
			psy_ui_component_set_colour(component,
				psy_ui_colour_make(0x00BDBDBD));
		}
		if (component == &self->currselection.component && !self->patternselection.valid) {
			psy_ui_component_set_colour(component,
				psy_ui_colour_make(0x00666666));
		}		
	}
	psy_list_free(q);
	psy_ui_component_invalidate(&self->searchonchoice);
}

void transformpatternview_onsearch(TransformPatternView* self)
{
	psy_audio_PatternSearchReplaceMode searchreplacemode;
	
	searchreplacemode = setupsearchreplacemode(
		(int)psy_ui_combobox_itemdata(&self->searchnote,
			psy_ui_combobox_cursel(&self->searchnote)),
		(int)psy_ui_combobox_itemdata(&self->searchinst,
			psy_ui_combobox_cursel(&self->searchinst)),
		(int)psy_ui_combobox_itemdata(&self->searchmach,
			psy_ui_combobox_cursel(&self->searchmach)),
		-1, -1, -1, FALSE);
	switch (self->applyto) {
	case 0:
		transformpatternview_searchentiresong(self, searchreplacemode);
		break;
	case 1:
		transformpatternview_searchpattern(self, searchreplacemode);
		break;
	case 2:
		transformpatternview_searchcurrentselection(self, searchreplacemode);
		break;
	default:
		break;
	}	
}

void transformpatternview_searchentiresong(TransformPatternView* self,
	psy_audio_PatternSearchReplaceMode searchreplacemode)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		psy_TableIterator it;

		for (it = psy_audio_patterns_begin(
					&workspace_song(self->workspace)->patterns);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_Pattern* currpattern;

			currpattern = (psy_audio_Pattern*)psy_tableiterator_value(&it);
			if (psy_audio_sequence_patternused(&workspace_song(self->workspace)->sequence,
					psy_tableiterator_key(&it))) {
				psy_audio_SequenceCursor cursor;

				cursor = psy_audio_pattern_searchinpattern(currpattern,
					psy_audio_blockselection_make(
						psy_audio_sequencecursor_make(
							psy_audio_orderindex_make(0, 0),
							0, (psy_dsp_big_beat_t)0.0),
						psy_audio_sequencecursor_make(
							psy_audio_orderindex_make(0, 0),
							MAX_TRACKS,
							psy_audio_pattern_length(currpattern))),
					searchreplacemode);				
				{
					psy_audio_OrderIndex orderindex;
									
					orderindex = psy_audio_sequence_patternfirstused(
						&workspace_song(self->workspace)->sequence,
						psy_tableiterator_key(&it));					
					psy_audio_sequenceselection_select_first(
						&self->workspace->song->sequence.selection,
						orderindex);					
					// workspace_setcursor(self->workspace, cursor);
					break;
				}
			}
		}
	}
}

void transformpatternview_searchpattern(TransformPatternView* self,
	psy_audio_PatternSearchReplaceMode searchreplacemode)
{
	psy_audio_Pattern* currpattern;

	assert(self);

	currpattern = transformpatternview_currpattern(self);
	if (currpattern) {
		psy_audio_SequenceCursor cursor;

		cursor = psy_audio_pattern_searchinpattern(currpattern,
			psy_audio_blockselection_make(
				psy_audio_sequencecursor_make(
					psy_audio_orderindex_make(0, 0),
					0, (psy_dsp_big_beat_t)0.0),
				psy_audio_sequencecursor_make(
					psy_audio_orderindex_make(0, 0),
					MAX_TRACKS,
					psy_audio_pattern_length(currpattern))),
			searchreplacemode);		
		psy_audio_sequence_set_cursor(&self->workspace->song->sequence,
				cursor);		
	}
}

void transformpatternview_searchcurrentselection(TransformPatternView* self,
	psy_audio_PatternSearchReplaceMode searchreplacemode)
{
	psy_audio_Pattern* currpattern;

	assert(self);

	currpattern = transformpatternview_currpattern(self);
	if (currpattern) {
		psy_audio_SequenceCursor cursor;

		cursor = psy_audio_pattern_searchinpattern(currpattern,
			self->patternselection, searchreplacemode);		
	}	
}

void transformpatternview_set_pattern_selection(TransformPatternView* self,
	const psy_audio_BlockSelection* selection)
{
	assert(self && selection);
	
	if (self->patternselection.valid != selection->valid) {
		self->patternselection = *selection;
		if (selection->valid) {
			psy_ui_component_enable_input(psy_ui_label_base(&self->currselection),
				psy_ui_NONE_RECURSIVE);
			transformpatternview_applyto(self, 2);
		} else {
			psy_ui_component_prevent_input(psy_ui_label_base(&self->currselection),
				psy_ui_NONE_RECURSIVE);
			if (self->applyto == 2) {
				transformpatternview_applyto(self, 1);
			}
		}				
	}
}

psy_audio_Pattern* transformpatternview_currpattern(TransformPatternView* self)
{
	psy_audio_Pattern* pattern;	

	if (self->workspace->song) {
		return pattern = psy_audio_sequence_pattern(&self->workspace->song->sequence,
			psy_audio_sequenceselection_first(&self->workspace->song->sequence.selection));
	}
	return NULL;
}

void transformpatternview_onreplace(TransformPatternView* self)
{
	assert(self);
}

void transformpatternview_onhide(TransformPatternView* self)
{
	assert(self);

	psy_ui_component_hide_align(&self->component);
}

psy_audio_PatternSearchReplaceMode setupsearchreplacemode(int searchnote,
	int searchinst, int searchmach, int replnote, int replinst, int replmach,
	bool repltweak)
{
	psy_audio_PatternSearchReplaceMode mode;

	mode.notereference = (uintptr_t)(searchnote & 0xFF);
	mode.instreference = (uintptr_t)(searchinst & 0xFF);
	mode.machreference = (uintptr_t)(searchmach & 0xFF);
	mode.notereplace = (uintptr_t)(replnote & 0xFF);
	mode.instreplace = (uintptr_t)(replinst & 0xFF);
	mode.machreplace = (uintptr_t)(replmach & 0xFF);
	mode.tweakreplace = psy_audio_NOTECOMMANDS_EMPTY;

	// In search: 1001 empty, 1002 non-empty, 1003 all, other -> exact match
	switch (searchnote) {
	case 1001: mode.notematcher = psy_audio_patternsearchreplacemode_matchesempty; break;
	case 1002: mode.notematcher = psy_audio_patternsearchreplacemode_matchesnonempty; break;
	case 1003: mode.notematcher = psy_audio_patternsearchreplacemode_matchesall; break;
	default: mode.notematcher = psy_audio_patternsearchreplacemode_matchesequal; break;
	}
	switch (searchinst) {
	case 1001: mode.instmatcher = psy_audio_patternsearchreplacemode_matchesempty; break;
	case 1002: mode.instmatcher = psy_audio_patternsearchreplacemode_matchesnonempty; break;
	case 1003: mode.instmatcher = psy_audio_patternsearchreplacemode_matchesall; break;
	default: mode.instmatcher = psy_audio_patternsearchreplacemode_matchesequal; break;
	}
	switch (searchmach) {
	case 1001: mode.machmatcher = psy_audio_patternsearchreplacemode_matchesempty; break;
	case 1002: mode.machmatcher = psy_audio_patternsearchreplacemode_matchesnonempty; break;
	case 1003: mode.machmatcher = psy_audio_patternsearchreplacemode_matchesall; break;
	default: mode.machmatcher = psy_audio_patternsearchreplacemode_matchesequal; break;
	}
	// in replace: 1001 set empty, 1002 -> keep existing, other -> replace value
	switch (replnote) {
	case 1001: mode.notereplacer = psy_audio_patternsearchreplacemode_replacewithempty; break;
	case 1002: mode.notereplacer = psy_audio_patternsearchreplacemode_replacewithcurrent; break;
	default: mode.notereplacer = psy_audio_patternsearchreplacemode_replacewithnewval; break;
	}
	switch (replinst) {
	case 1001: mode.instreplacer = psy_audio_patternsearchreplacemode_replacewithempty; break;
	case 1002: mode.instreplacer = psy_audio_patternsearchreplacemode_replacewithcurrent; break;
	default: mode.instreplacer = psy_audio_patternsearchreplacemode_replacewithnewval; break;
	}
	switch (replmach) {
	case 1001: mode.machreplacer = psy_audio_patternsearchreplacemode_replacewithempty; break;
	case 1002: mode.machreplacer = psy_audio_patternsearchreplacemode_replacewithcurrent; break;
	default: mode.machreplacer = psy_audio_patternsearchreplacemode_replacewithnewval; break;
	}
	mode.tweakreplacer = (repltweak)
		? psy_audio_patternsearchreplacemode_replacewithempty
		: psy_audio_patternsearchreplacemode_replacewithcurrent;
	return mode;
}
