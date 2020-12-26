// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequenceview.h"
// audio
#include <exclusivelock.h>
#include <patterns.h>
#include <songio.h>
// ui
#include <uialigner.h>
// std
#include <assert.h>
#include <stdio.h>
#include <string.h>
// pltform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// SequenceButtons
// prototypes
static void sequencebuttons_onalign(SequenceButtons*);
static void sequencebuttons_onpreferredsize(SequenceButtons*, psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable sequencebuttons_vtable;
static bool sequencebuttons_vtable_initialized = FALSE;

static void sequencebuttons_vtable_init(SequenceButtons* self)
{
	if (!sequencebuttons_vtable_initialized) {
		sequencebuttons_vtable = *(self->component.vtable);
		sequencebuttons_vtable.onalign = (psy_ui_fp_component_onalign)
			sequencebuttons_onalign;
		sequencebuttons_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			sequencebuttons_onpreferredsize;
		sequencebuttons_vtable_initialized = TRUE;
	}
}
// implementation
void sequencebuttons_init(SequenceButtons* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	sequencebuttons_vtable_init(self);
	self->component.vtable = &sequencebuttons_vtable;
	psy_ui_button_init_text(&self->incpattern, &self->component,
		"+");
	psy_ui_button_init_text(&self->insertentry, &self->component,
		"sequencerview.ins");
	psy_ui_button_init_text(&self->decpattern, &self->component,
		"-");
	psy_ui_button_init_text(&self->newentry, &self->component,
		"sequencerview.new");
	psy_ui_button_init_text(&self->cloneentry, &self->component,
		"sequencerview.clone");
	psy_ui_button_init_text(&self->delentry, &self->component,
		"sequencerview.del");	
	psy_ui_button_init_text(&self->clear, &self->component,
		"sequencerview.clear");
	psy_ui_button_init_text(&self->rename, &self->component,
		"sequencerview.rename");
	// psy_ui_button_init(&self->cut, &self->component);
	// psy_ui_button_settext(&self->cut, "");
	psy_ui_button_init_text(&self->copy, &self->component,
		"sequencerview.copy");
	psy_ui_button_init_text(&self->paste, &self->component,
		"sequencerview.paste");
	psy_ui_button_init_text(&self->singlesel, &self->component,
		"sequencerview.singlesel");
	psy_ui_button_init_text(&self->multisel, &self->component,
		"sequencerview.multisel");
	psy_ui_button_highlight(&self->singlesel);
	psy_ui_button_disablehighlight(&self->multisel);
}

void sequencebuttons_onalign(SequenceButtons* self)
{
	int numparametercols = 3;
	int numrows = 0;
	int colwidth = 0;
	int rowheight = 0;
	int cpx = 0;
	int cpy = 0;
	int c = 0;
	int margin = 5;
	int ident;
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_List* p;
	psy_List* q;

	size = psy_ui_component_size(&self->component);
	size = psy_ui_component_preferredsize(&self->component, &size);
	tm = psy_ui_component_textmetric(&self->component);
	ident = 0; // tm.tmAveCharWidth * 0.5;
	cpx = ident;
	colwidth = (psy_ui_value_px(&size.width, &tm) - ident) / numparametercols;
	p = q = psy_ui_component_children(&self->component, 0);
	numrows = (psy_list_size(p) / numparametercols) + 1;
	rowheight = psy_ui_value_px(&size.height, &tm) / numrows;
	for (; p != NULL; p = p->next, ++c, cpx += colwidth + margin) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)psy_list_entry(p);
		if (c >= numparametercols) {
			cpx = ident;
			cpy += rowheight + margin;
			c = 0;
		}
		psy_ui_component_setposition(component,
			psy_ui_point_make(
				psy_ui_value_makepx(cpx),
				psy_ui_value_makepx(cpy)),
			psy_ui_size_make(
				psy_ui_value_makepx(colwidth),
				psy_ui_value_makepx(rowheight)));
	}
	psy_list_free(q);
}

void sequencebuttons_onpreferredsize(SequenceButtons* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (rv) {
		int numparametercols = 3;
		int margin = 5;
		int ident = 0;
		int c = 0;
		int cpx = 0;
		int cpy = 0;
		int cpxmax = 0;
		int cpymax = 0;
		int colmax = 0;
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		psy_List* p;
		psy_List* q;

		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		ident = tm.tmAveCharWidth * 1;
		cpx = ident;
		for (p = q = psy_ui_component_children(&self->component, 0); p != NULL;
			p = p->next, ++c) {
			psy_ui_Component* component;
			psy_ui_Size componentsize;
			if (c >= numparametercols) {
				cpx = ident;
				cpy = cpymax;
				c = 0;
			}
			component = (psy_ui_Component*)p->entry;
			componentsize = psy_ui_component_preferredsize(component, &size);
			if (colmax < psy_ui_value_px(&componentsize.width, &tm) + margin) {
				colmax = psy_ui_value_px(&componentsize.width, &tm) + margin;
			}
			cpx += psy_ui_value_px(&componentsize.width, &tm) + margin;
			if (cpymax < cpy + psy_ui_value_px(&componentsize.height, &tm) + margin) {
				cpymax = cpy + psy_ui_value_px(&componentsize.height, &tm) + margin;
			}
		}
		psy_list_free(q);
		cpxmax = numparametercols * colmax;
		rv->width = psy_ui_value_makepx(cpxmax + 5);
		rv->height = psy_ui_value_makepx(cpymax);
	}
}

// SequenceListViewState
void sequencelistviewstate_init(SequenceListViewState* self)
{
	self->margin = 5;
	self->trackwidth = 100;
	self->selectedtrack = 0;
}

// SequenceViewTrackHeader
// prototypes
static void sequenceviewtrackheader_ondestroy(SequenceViewTrackHeader*);
static void sequenceviewtrackheader_ondraw(SequenceViewTrackHeader*,
	psy_ui_Graphics*);
static void sequenceviewtrackheader_drawtext(SequenceViewTrackHeader*,
	psy_ui_Graphics*, int x, int y, const char* text);
static void sequenceviewtrackheader_onmousedown(SequenceViewTrackHeader*,
	psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable trackheaderview_vtable;
static bool trackheaderview_vtable_initialized = FALSE;

static void trackheaderview_vtable_init(SequenceViewTrackHeader* self)
{
	if (!trackheaderview_vtable_initialized) {
		trackheaderview_vtable = *(self->component.vtable);
		trackheaderview_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			sequenceviewtrackheader_ondestroy;
		trackheaderview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			sequenceviewtrackheader_ondraw;
		trackheaderview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			sequenceviewtrackheader_onmousedown;
		trackheaderview_vtable_initialized = TRUE;
	}
}

// implemenetation
void sequenceviewtrackheader_init(SequenceViewTrackHeader* self,
	psy_ui_Component* parent, SequenceListViewState* state)
{	
	psy_ui_component_init(&self->component, parent);
	trackheaderview_vtable_init(self);
	self->component.vtable = &trackheaderview_vtable;
	self->state = state;	
	//psy_ui_component_setbackgroundcolour(&self->component,
		//psy_ui_colour_make(0x00262626));
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makepx(0),
			psy_ui_value_makeeh(1)));
	psy_ui_component_preventalign(&self->component);
	self->colour = psy_ui_colour_make(0x00303030);
	self->colour_highlight = psy_ui_colour_make(0x00545454);
	self->colour_font = psy_ui_colour_make(0x00B2B2B2);	
	psy_signal_init(&self->signal_newtrack);
	psy_signal_init(&self->signal_deltrack);
	psy_signal_init(&self->signal_trackselected);
}

void sequenceviewtrackheader_ondestroy(SequenceViewTrackHeader* self)
{
	psy_signal_dispose(&self->signal_newtrack);
	psy_signal_dispose(&self->signal_deltrack);
	psy_signal_dispose(&self->signal_trackselected);
}

void sequenceviewtrackheader_ondraw(SequenceViewTrackHeader* self,
	psy_ui_Graphics* g)
{
	if (self->state->sequence) {
		psy_audio_SequenceTrackNode* p;
		int cpx = 0;
		int centery;
		int lineheight = 1;		
		int c = 0;
		psy_ui_Rectangle r;
		psy_ui_IntSize size;
		psy_ui_TextMetric tm;

		size = psy_ui_component_intsize(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		centery = size.height - 1; // lineheight) / 2;
		cpx = self->state->margin;		
		psy_ui_settextcolour(g, self->colour_font);
		for (p = self->state->sequence->tracks; p != NULL; p = p->next,
			cpx += self->state->trackwidth, ++c) {
			char text[64];

			psy_ui_setrectangle(&r,
				cpx, 0, self->state->trackwidth - 5, size.height);			
			if (self->state->selectedtrack != c) {
				psy_ui_setcolour(g, self->colour);				
			} else {
				psy_ui_setcolour(g, self->colour_highlight);				
			}
			psy_ui_drawrectangle(g, r);
			psy_snprintf(text, 64, "%.2X", (int)c);			
			sequenceviewtrackheader_drawtext(self, g, cpx + (int)(tm.tmAveCharWidth * 0.2), 0, text);
			sequenceviewtrackheader_drawtext(self, g, cpx + (int)(tm.tmAveCharWidth * 5), 0, "S");
			sequenceviewtrackheader_drawtext(self, g, cpx + (int)(tm.tmAveCharWidth * 8), 0, "M");
			if (c != 0) {
				sequenceviewtrackheader_drawtext(self, g, cpx + self->state->trackwidth -
					(int)(tm.tmAveCharWidth * 3.5), 0, "X");
			}
		}
		psy_ui_setrectangle(&r,
			cpx, 0, self->state->trackwidth - 5, size.height);
		psy_ui_setcolour(g, self->colour);
		psy_ui_drawrectangle(g, r);
		sequenceviewtrackheader_drawtext(self, g,
			cpx + (int)((self->state->trackwidth - tm.tmAveCharWidth * 6) / 2), 0,
			"Add");
	}	
}

void sequenceviewtrackheader_drawtext(SequenceViewTrackHeader* self,
	psy_ui_Graphics* g, int x, int y, const char* text)
{	
	psy_ui_textout(g, x + 3, y, text, psy_strlen(text));
}

void sequenceviewtrackheader_onmousedown(SequenceViewTrackHeader* self,
	psy_ui_MouseEvent* ev)
{	
	uintptr_t selectedtrack;

	selectedtrack = ev->x / self->state->trackwidth;
	if (selectedtrack >= psy_audio_sequence_sizetracks(self->state->sequence)) {
		psy_signal_emit(&self->signal_newtrack, self, 1,
			(int)psy_audio_sequence_sizetracks(self->state->sequence));
	} else {
		psy_ui_TextMetric tm;
		
		tm = psy_ui_component_textmetric(&self->component);
		if (selectedtrack != 0 && ev->x >= selectedtrack * self->state->trackwidth + self->state->trackwidth -
				(int)(tm.tmAveCharWidth * 3.5)) {
			psy_signal_emit(&self->signal_deltrack, self, 1, (int)selectedtrack);
		} else {
			psy_signal_emit(&self->signal_trackselected, self, 1, (int)selectedtrack);
		}
	}
}

// SequenceListView
// prototypes
static void sequencelistview_onpreferredsize(SequenceListView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sequencelistview_ondraw(SequenceListView*, psy_ui_Graphics*);
static void sequencelistview_drawtrack(SequenceListView*, psy_ui_Graphics*,
	psy_audio_SequenceTrack*, int trackindex, int x);
static void sequencelistview_drawprogressbar(SequenceListView*,
	psy_ui_Graphics*, int x, int y,
	psy_audio_SequenceEntry*);
static void sequencelistview_onmousedown(SequenceListView*,
	psy_ui_MouseEvent*);
static void sequencelistview_onmousedoubleclick(SequenceListView*,
	psy_ui_MouseEvent*);
static void sequencelistview_ontimer(SequenceListView*, uintptr_t timerid);
static void sequencelistview_onpatternnamechanged(SequenceListView*,
	psy_audio_Patterns*, uintptr_t slot);
static psy_ui_Rectangle sequencelistview_rowrectangle(SequenceListView*,
	uintptr_t row);
static void sequencelistview_invalidaterow(SequenceListView*, uintptr_t row);
static void sequencelistview_oneditkeydown(SequenceListView*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);
// vtable
static psy_ui_ComponentVtable sequencelistview_vtable;
static bool sequencelistview_vtable_initialized = FALSE;

static void sequencelistview_vtable_init(SequenceListView* self)
{
	if (!sequencelistview_vtable_initialized) {
		sequencelistview_vtable = *(self->component.vtable);
		sequencelistview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			sequencelistview_ondraw;
		sequencelistview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			sequencelistview_onmousedown;
		sequencelistview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmousedoubleclick)
			sequencelistview_onmousedoubleclick;
		sequencelistview_vtable.ontimer = (psy_ui_fp_component_ontimer)
			sequencelistview_ontimer;
		sequencelistview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			sequencelistview_onpreferredsize;
		sequencelistview_vtable_initialized = TRUE;
	}
}
// implementation
void sequencelistview_init(SequenceListView* self, psy_ui_Component* parent,
	SequenceListViewState* state, SequenceView* view,
	psy_audio_Patterns* patterns, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	sequencelistview_vtable_init(self);
	self->component.vtable = &sequencelistview_vtable;
	self->state = state;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 1);
	psy_ui_edit_init(&self->rename, &self->component);
	psy_signal_connect(&self->rename.component.signal_keydown, self,
		sequencelistview_oneditkeydown);
	psy_ui_component_hide(&self->rename.component);
	self->view = view;	
	self->patterns = patterns;
	self->workspace = workspace;
	self->selection = &workspace->sequenceselection;
	self->selected = 0;	
	self->lineheight = 12;
	self->textoffsety = 0;	
	self->lastplayposition = -1.f;
	self->lastplayrow = UINTPTR_MAX;
	self->showpatternnames = generalconfig_showingpatternnames(psycleconfig_general(
		workspace_conf(self->workspace)));
	self->refreshcount = 0;
	if (self->state->sequence && self->state->sequence->patterns) {
		psy_signal_connect(&self->state->sequence->patterns->signal_namechanged, self,
			sequencelistview_onpatternnamechanged);
	}
	sequencelistview_computetextsizes(self);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	psy_ui_component_starttimer(&self->component, 0, 200);
}

void sequencelistview_ondraw(SequenceListView* self, psy_ui_Graphics* g)
{
	psy_audio_SequenceTrackNode* p;
	int cpx = 0;
	int trackindex = 0;
	self->foundselected = 0;

	sequencelistview_computetextsizes(self);
	for (p = self->state->sequence->tracks; p != NULL; p = p->next,
		cpx += self->state->trackwidth, ++trackindex) {
		sequencelistview_drawtrack(self, g, (psy_audio_SequenceTrack*)
			psy_list_entry(p), trackindex, cpx);
	}
	if (!self->foundselected) {
		int cpy;
		psy_ui_Rectangle r;

		cpx = self->state->selectedtrack * self->state->trackwidth + self->state->margin + 5;
		cpy = self->selected * self->lineheight + self->state->margin;
		psy_ui_setrectangle(&r, cpx,
			cpy, self->state->trackwidth - 5 - 2 * self->state->margin,
			self->textheight);
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x009B7800));
	}
}

void sequencelistview_onpreferredsize(SequenceListView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->state->sequence) {
		rv->width = psy_ui_value_makepx(self->state->margin +
			psy_audio_sequence_sizetracks(self->state->sequence) *
				self->component.scrollstepx);
		rv->height = psy_ui_value_makepx(
			psy_audio_sequence_maxtracksize(self->state->sequence) *
			self->component.scrollstepy);
	} else {
		*rv = psy_ui_size_zero();
	}
}

void sequencelistview_computetextsizes(SequenceListView* self)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int)(tm.tmHeight * 1.2);
	self->textoffsety = (self->lineheight - tm.tmHeight) / 2;
	self->textheight = tm.tmHeight;
	self->state->trackwidth = tm.tmAveCharWidth * 16;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->component.scrollstepy = self->lineheight;
	self->component.scrollstepx = self->state->trackwidth;
}

void sequencelistview_drawtrack(SequenceListView* self, psy_ui_Graphics* g, psy_audio_SequenceTrack* track, int trackindex, int x)
{
	psy_List* p;
	uintptr_t c = 0;
	int cpy = 0;
	char text[20];
	psy_ui_Rectangle r;
	psy_ui_IntSize size;
	int startrow;
	int endrow;

	startrow = max(0, (g->clip.top - self->state->margin) / self->lineheight);
	endrow = (int)((g->clip.bottom - self->state->margin) / (double)self->lineheight);
	size = psy_ui_component_intsize(&self->component);
	psy_ui_setrectangle(&r, x, 0, self->state->trackwidth - 5, size.height);
	psy_ui_settextcolour(g, psy_ui_colour_make(0));
	p = track->entries;
	for (; p != NULL; psy_list_next(&p), ++c, cpy += self->lineheight) {
		psy_audio_SequenceEntry* sequenceentry;
		bool rowplaying = FALSE;

		sequenceentry = (psy_audio_SequenceEntry*)p->entry;
		if ((int)c < startrow) {
			continue;
		} else if ((int)c > endrow) {
			// todo: should be >=, but that produces at scroll artifacts
			break;
		}
		rowplaying = psy_audio_player_playing(self->player) &&
			psy_audio_player_playlist_position(self->player) == c;
		if (self->showpatternnames) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_sequenceentry_pattern(sequenceentry,
				self->patterns);
			if (pattern) {
				psy_snprintf(text, 20, "%02X: %s %4.2f", c,
					psy_audio_pattern_name(pattern),
					sequenceentry->offset);
			} else {
				psy_snprintf(text, 20, "%02X:%02X(ERR) %4.2f", c,
					(int)psy_audio_sequenceentry_patternslot(sequenceentry),
					sequenceentry->offset);
			}
		} else {
			psy_snprintf(text, 20, "%02X:%02X  %4.2f", c,
				(int)psy_audio_sequenceentry_patternslot(sequenceentry),
				sequenceentry->offset);
		}
		if (rowplaying) {
			sequencelistview_drawprogressbar(self, g, x, cpy, sequenceentry);
		}
		if (self->state->selectedtrack == trackindex &&
			(self->selection->editposition.trackposition.sequencentrynode == p
				|| (psy_list_findentry(self->selection->entries, sequenceentry))
				)) {
			if (!rowplaying) {
				psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));
			} else {
				psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			}
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
			self->foundselected = 1;
		} else if (rowplaying) {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
		} else {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
		}
		psy_ui_textout(g, x + 5, cpy + self->state->margin + self->textoffsety, text,
			strlen(text));
	}
}

void sequencelistview_drawprogressbar(SequenceListView* self,
	psy_ui_Graphics* g, int x, int y,
	psy_audio_SequenceEntry* sequenceentry)
{
	psy_ui_Rectangle r;

	r = psy_ui_rectangle_make(x + 5, y + self->state->margin,
		(int)((psy_audio_player_playlist_rowprogress(workspace_player(self->workspace))) *
			(self->state->trackwidth - 5)), self->lineheight);
	psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00514536));
}

void sequencelistview_showpatternnames(SequenceListView* self)
{
	self->showpatternnames = TRUE;
	generalconfig_showpatternnames(psycleconfig_general(workspace_conf(self->workspace)));
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_showpatternslots(SequenceListView* self)
{
	self->showpatternnames = FALSE;
	generalconfig_showpatternids(psycleconfig_general(workspace_conf(self->workspace)));
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_rename(SequenceListView* self)
{
	psy_audio_SequenceEntry* sequenceentry;

	sequenceentry = psy_audio_sequenceposition_entry(
		&self->selection->editposition);
	if (sequenceentry) {
		psy_audio_Pattern* pattern;

		pattern = psy_audio_sequenceentry_pattern(sequenceentry,
			self->patterns);
		if (pattern) {
			psy_ui_component_setposition(&self->rename.component,
				psy_ui_point_make(psy_ui_value_makepx(0),
					psy_ui_value_makepx(self->state->margin)),
				psy_ui_size_make(psy_ui_value_makeew(20), psy_ui_value_makeeh(1)));
			psy_ui_edit_settext(&self->rename, psy_audio_pattern_name(
				pattern));
			psy_ui_component_show(&self->rename.component);
			psy_ui_component_setfocus(&self->rename.component);
		}
	}
}

void sequencelistview_oneditkeydown(SequenceListView* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		psy_audio_SequenceEntry* sequenceentry;

		sequenceentry = psy_audio_sequenceposition_entry(
			&self->selection->editposition);
		if (sequenceentry) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_sequenceentry_pattern(sequenceentry,
				self->patterns);
			if (pattern) {
				psy_audio_pattern_setname(pattern,
					psy_ui_edit_text(&self->rename));
			}
		}
		psy_ui_component_hide(&self->rename.component);
		psy_ui_component_setfocus(&self->component);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_component_hide(&self->rename.component);
		psy_ui_component_setfocus(&self->component);
	}
}

void sequencelistview_onmousedown(SequenceListView* self,
	psy_ui_MouseEvent* ev)
{
	uintptr_t selected;
	uintptr_t selectedtrack;

	sequencelistview_computetextsizes(self);
	selected = (ev->y - self->state->margin) / self->lineheight;
	selectedtrack = ev->x / self->state->trackwidth;
	if (selectedtrack < psy_audio_sequence_sizetracks(self->state->sequence)) {
		sequencelistview_select(self, selectedtrack, selected);
	}
}

void sequencelistview_select(SequenceListView* self,
	uintptr_t track, uintptr_t row)
{
	if (track >= psy_audio_sequence_sizetracks(self->state->sequence)) {
		if (psy_audio_sequence_sizetracks(self->state->sequence) > 0) {
			track = psy_audio_sequence_sizetracks(self->state->sequence) - 1;
		} else {
			track = 0;
		}
	}
	psy_audio_sequenceselection_seteditposition(self->selection,
		psy_audio_sequence_at(self->state->sequence, track, row));
	workspace_setsequenceselection(self->workspace, *self->selection);
	sequenceview_updateplayposition(self->view);
}

void sequencelistview_onmousedoubleclick(SequenceListView* self,
	psy_ui_MouseEvent* ev)
{
	uintptr_t selected;
	uintptr_t selectedtrack;

	sequencelistview_computetextsizes(self);
	selected = (ev->y - self->state->margin) / self->lineheight;
	selectedtrack = ev->x / self->state->trackwidth;
	if (selectedtrack < psy_audio_sequence_sizetracks(self->state->sequence)) {
		psy_audio_sequenceselection_seteditposition(self->selection,
			psy_audio_sequence_at(self->state->sequence, selectedtrack, selected));
		workspace_setsequenceselection(self->workspace, *self->selection);
		sequenceview_changeplayposition(self->view);
	}
}

void sequencelistview_ontimer(SequenceListView* self, uintptr_t timerid)
{
	if (psy_audio_player_playing(self->player)) {
		if (psy_audio_player_playlist_position(workspace_player(self->workspace)) != self->lastplayrow) {
			// invalidate previous row
			sequencelistview_invalidaterow(self, self->lastplayrow);
			self->lastplayrow = psy_audio_player_playlist_position(workspace_player(self->workspace));
			// next(curr) row is invalidated with row progress bar
		}
		if (self->refreshcount == 2) { // saves cpu not updating at every intervall
									   // todo: better check for player line change
			// invalidate row progress bar
			// takes care, too, that at row change the new row is invalidated
			if (psy_audio_player_playlist_position(workspace_player(self->workspace))
				!= UINTPTR_MAX) {
				sequencelistview_invalidaterow(self,
					psy_audio_player_playlist_position(workspace_player(self->workspace)));
			}
			self->refreshcount = 0;
		}
		++self->refreshcount;
	} else if (self->lastplayrow != UINTPTR_MAX) {
		// invalidate if player is stopping to remove the progress bar
		sequencelistview_invalidaterow(self, self->lastplayrow);
		self->lastplayrow = UINTPTR_MAX;
	}
}

void sequencelistview_onpatternnamechanged(SequenceListView* self, psy_audio_Patterns* patterns,
	uintptr_t slot)
{
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_invalidaterow(SequenceListView* self, uintptr_t row)
{
	if (row != UINTPTR_MAX) {
		psy_ui_component_invalidaterect(&self->component,
			sequencelistview_rowrectangle(self, row));
	}
}

psy_ui_Rectangle sequencelistview_rowrectangle(SequenceListView* self,
	uintptr_t row)
{
	psy_ui_IntSize size;

	size = psy_ui_component_intsize(&self->component);
	return psy_ui_rectangle_make(0, self->lineheight * row + self->state->margin,
		size.width, self->lineheight);
}

// SequenceViewDuration
// implementation
void sequenceduration_init(SequenceViewDuration* self, psy_ui_Component* parent,
	psy_audio_Sequence* sequence, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(2.0));
	self->sequence = sequence;
	self->workspace = workspace;
	self->duration_ms = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_label_init(&self->desc, &self->component);
	psy_ui_label_setcharnumber(&self->desc, 9);
	psy_ui_label_settext(&self->desc, "sequencerview.duration");
	psy_ui_label_settextalignment(&self->desc, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->duration, &self->component);
	psy_ui_label_settextalignment(&self->duration, psy_ui_ALIGNMENT_LEFT);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_CLIENT);
	psy_ui_label_setcharnumber(&self->duration, 10);
	psy_ui_label_preventtranslation(&self->duration);
	psy_list_free(psy_ui_components_setmargin(
		psy_ui_component_children(&self->component, 0),
		&margin));	
	sequenceduration_update(self);	
}

void sequenceduration_update(SequenceViewDuration* self)
{
	psy_dsp_big_beat_t songlength;
	char text[40];

	songlength = psy_audio_sequence_calcdurationinms(self->sequence);
	psy_snprintf(text, 40, "%02dm%02ds %.2fb",
		(int)(songlength / 60), ((int)songlength % 60),
		psy_audio_sequence_duration(self->sequence));
	psy_ui_label_settext(&self->duration, text);
}

// SequencerOptionsBar
// implementation
void sequenceroptionsbar_init(SequencerOptionsBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_checkbox_init_multiline(&self->followsong, &self->component);
	psy_ui_checkbox_settext(&self->followsong,
		"sequencerview.follow-song");
	psy_ui_checkbox_init_multiline(&self->shownames, &self->component);
	psy_ui_checkbox_settext(&self->shownames,
		"sequencerview.show-pattern-names");		
	psy_ui_checkbox_init_multiline(&self->recordnoteoff, &self->component);
	psy_ui_checkbox_settext(&self->recordnoteoff,
		"sequencerview.record-noteoff");
	psy_ui_checkbox_init_multiline(&self->recordtweak, &self->component);
	psy_ui_checkbox_settext(&self->recordtweak,
		"sequencerview.record-tweak");
	psy_ui_checkbox_init_multiline(&self->multichannelaudition, &self->component);
	psy_ui_checkbox_settext(&self->multichannelaudition,
		"sequencerview.multichannel-audition");		
	psy_ui_checkbox_init_multiline(&self->allownotestoeffect, &self->component);
	psy_ui_checkbox_settext(&self->allownotestoeffect,
		"sequencerview.allow-notes-to_effect");	
	{
		psy_ui_Margin margin;
		// seqedit buttons
		psy_ui_component_init(&self->seqedit, &self->component);
		psy_ui_component_setalign(&self->seqedit, psy_ui_ALIGN_BOTTOM);			
		psy_ui_button_init(&self->toggleseqediticon, &self->seqedit);
		psy_ui_button_seticon(&self->toggleseqediticon, psy_ui_ICON_MORE);
		psy_ui_component_setalign(&self->toggleseqediticon.component,
			psy_ui_ALIGN_LEFT);		
		psy_ui_button_init_text(&self->toggleseqedit, &self->seqedit,
			"Show Sequenceeditor");
		psy_ui_component_setalign(&self->toggleseqedit.component,
			psy_ui_ALIGN_LEFT);		
		// stepseq buttons
		psy_ui_component_init(&self->stepseq, &self->component);
		psy_ui_component_setalign(&self->stepseq, psy_ui_ALIGN_BOTTOM);
		psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
			psy_ui_value_makepx(0));
		psy_ui_button_init(&self->togglestepseqicon, &self->stepseq);
		psy_ui_button_seticon(&self->togglestepseqicon, psy_ui_ICON_MORE);
		psy_ui_component_setalign(&self->togglestepseqicon.component,
			psy_ui_ALIGN_LEFT);
		psy_ui_component_setmargin(&self->togglestepseqicon.component, &margin);
		psy_ui_button_init_text(&self->togglestepseq, &self->stepseq,
			"Show Stepsequencer");
		psy_ui_component_setalign(&self->togglestepseq.component,
			psy_ui_ALIGN_LEFT);		
	}	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.2),
		psy_ui_value_makeew(2.0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1.0),
		psy_ui_value_makeew(2.0));
	psy_ui_component_setmargin(&self->allownotestoeffect.component, &margin);
}

// SequenceView
// prototypes
static void sequenceview_onnewentry(SequenceView*);
static void sequenceview_oninsertentry(SequenceView*);
static void sequenceview_oncloneentry(SequenceView*);
static void sequenceview_ondelentry(SequenceView*);
static void sequenceview_onincpattern(SequenceView*);
static void sequenceview_ondecpattern(SequenceView*);
static void sequenceview_onnewtrack(SequenceView*, psy_ui_Component* sender,
	uintptr_t trackindex);
static void sequenceview_ontrackselected(SequenceView*, psy_ui_Component* sender,
	uintptr_t trackindex);
static void sequenceview_ondeltrack(SequenceView*, psy_ui_Component* sender,
	uintptr_t trackindex);
static void sequenceview_onclear(SequenceView*);
static void sequenceview_onrename(SequenceView*);
static void sequenceview_oncut(SequenceView*);
static void sequenceview_oncopy(SequenceView*);
static void sequenceview_onpaste(SequenceView*);
static void sequenceview_onsingleselection(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onmultiselection(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onshowpatternnames(SequenceView*, psy_ui_CheckBox* sender);
static void sequenceview_onfollowsong(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onfollowsongchanged(SequenceView*, Workspace* sender);
static void sequenceview_onrecordtweak(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onrecordnoteoff(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onmultichannelaudition(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onsongchanged(SequenceView*, Workspace*, int flag, psy_audio_SongFile* songfile);
static void sequenceview_onsequenceselectionchanged(SequenceView*, Workspace*);
static void sequenceview_onsequencechanged(SequenceView*,
	psy_audio_Sequence* sender);
static void sequenceview_onconfigure(SequenceView*, GeneralConfig*,
	psy_Property*);
static void sequenceview_onthemechanged(SequenceView*, MachineViewConfig*,
	psy_Property* theme);
static void sequenceview_updateskin(SequenceView*);
static void sequenceview_onalign(SequenceView*, psy_ui_Component* sender);
// implementation
void sequenceview_init(SequenceView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	if (workspace->song) {		
		self->patterns = &workspace->song->patterns;
		self->selection = &workspace->sequenceselection;
		self->state.sequence = &workspace->song->sequence;
	} else {		
		self->patterns = NULL;
		self->selection = NULL;
		self->state.sequence = NULL;
	}
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	// shared state
	sequencelistviewstate_init(&self->state);	
	// sequence listview
	sequencelistview_init(&self->listview, &self->component,
		&self->state, self, self->patterns, workspace);
#if 1
	// Use Custom Scrollbar
	psy_ui_scroller_init(&self->scroller, &self->listview.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
#else
	psy_ui_component_setalign(&self->listview.component, psy_ui_ALIGN_CLIENT);
#endif
	self->listview.player = &workspace->player;
	// button bar
	sequencebuttons_init(&self->buttons, &self->component, workspace);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);	
	// spacer
	psy_ui_component_init(&self->spacer, &self->component);
	psy_ui_component_setpreferredsize(&self->spacer, psy_ui_size_makeem(0.0, 0.3));
	psy_ui_component_preventalign(&self->spacer);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_TOP);
	// header
	sequenceviewtrackheader_init(&self->trackheader, &self->component,
		&self->state);
	psy_ui_component_setalign(&self->trackheader.component, psy_ui_ALIGN_TOP);
	// options
	sequenceroptionsbar_init(&self->options, &self->component, workspace);	
	psy_ui_component_setalign(&self->options.component, psy_ui_ALIGN_BOTTOM);
	// duration
	sequenceduration_init(&self->duration, &self->component, self->state.sequence, workspace);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->buttons.newentry.signal_clicked, self,
		sequenceview_onnewentry);
	psy_signal_connect(&self->buttons.insertentry.signal_clicked, self,
		sequenceview_oninsertentry);
	psy_signal_connect(&self->buttons.cloneentry.signal_clicked, self,
		sequenceview_oncloneentry);
	psy_signal_connect(&self->buttons.delentry.signal_clicked, self,
		sequenceview_ondelentry);
	psy_signal_connect(&self->buttons.incpattern.signal_clicked, self,
		sequenceview_onincpattern);
	psy_signal_connect(&self->buttons.decpattern.signal_clicked, self,
		sequenceview_ondecpattern);
	psy_signal_connect(&self->trackheader.signal_newtrack, self,
		sequenceview_onnewtrack);
	psy_signal_connect(&self->trackheader.signal_trackselected, self,
		sequenceview_ontrackselected);	
	psy_signal_connect(&self->trackheader.signal_deltrack, self,
		sequenceview_ondeltrack);
	psy_signal_connect(&self->buttons.clear.signal_clicked, self,
		sequenceview_onclear);
	psy_signal_connect(&self->buttons.rename.signal_clicked, self,
		sequenceview_onrename);
	// psy_signal_connect(&self->buttons.cut.signal_clicked, self,
	//	sequenceview_oncut);
	psy_signal_connect(&self->buttons.copy.signal_clicked, self,
		sequenceview_oncopy);
	psy_signal_connect(&self->buttons.paste.signal_clicked, self,
		sequenceview_onpaste);
	psy_signal_connect(&self->buttons.singlesel.signal_clicked, self,
		sequenceview_onsingleselection);
	psy_signal_connect(&self->buttons.multisel.signal_clicked, self,
		sequenceview_onmultiselection);
	psy_signal_connect(&self->options.followsong.signal_clicked, self,
		sequenceview_onfollowsong);
	if (self->listview.showpatternnames) {
		psy_ui_checkbox_check(&self->options.shownames);
	} else {
		psy_ui_checkbox_disablecheck(&self->options.shownames);
	}
	psy_signal_connect(&self->options.shownames.signal_clicked, self,
		sequenceview_onshowpatternnames);
	psy_signal_connect(&self->options.recordnoteoff.signal_clicked, self,
		sequenceview_onrecordnoteoff);
	psy_signal_connect(&self->options.recordtweak.signal_clicked, self,
		sequenceview_onrecordtweak);
	psy_signal_connect(&self->options.multichannelaudition.signal_clicked, self,
		sequenceview_onmultichannelaudition);
	psy_signal_connect(&workspace->signal_songchanged, self,
		sequenceview_onsongchanged);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged, self,
		sequenceview_onsequenceselectionchanged);
	psy_signal_connect(&workspace->signal_followsongchanged, self,
		sequenceview_onfollowsongchanged);	
	if (self->state.sequence && self->state.sequence->patterns) {
		psy_signal_connect(&self->state.sequence->patterns->signal_namechanged,
			&self->listview,
			sequencelistview_onpatternnamechanged);
		psy_signal_connect(&self->state.sequence->sequencechanged,
			self, sequenceview_onsequencechanged);
	}
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed, self,
		sequenceview_onconfigure);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_themechanged,
		self, sequenceview_onthemechanged);
	sequenceview_updateskin(self);
	psy_signal_connect(&self->component.signal_align, self,
		sequenceview_onalign);
}

void sequenceview_onnewentry(SequenceView* self)
{
	psy_List* tracknode;
	psy_audio_Pattern* newpattern;

	newpattern = psy_audio_pattern_allocinit();
	// change length to default lines
	psy_audio_pattern_setlength(newpattern,
		psy_audio_pattern_defaultlines() /
		(psy_audio_player_lpb(workspace_player(self->workspace))));
	tracknode = psy_audio_sequence_insert(self->state.sequence,
		self->selection->editposition,
		psy_audio_patterns_append(self->patterns,
			newpattern));
	psy_audio_sequenceselection_seteditposition(self->selection,
		psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			tracknode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->component);
}

void sequenceview_oninsertentry(SequenceView* self)
{
	psy_audio_SequencePosition editposition;
	psy_audio_SequenceEntry* entry;
	psy_List* tracknode;

	editposition = self->workspace->sequenceselection.editposition;
	entry = psy_audio_sequenceposition_entry(&editposition);
	tracknode = psy_audio_sequence_insert(self->state.sequence, editposition,
		entry ? entry->patternslot : 0);
	psy_audio_sequenceselection_seteditposition(self->selection,
		psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			tracknode));
	sequenceview_updateplayposition(self);
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->component);
}

void sequenceview_oncloneentry(SequenceView* self)
{
	psy_audio_SequencePosition editposition;
	psy_audio_SequenceEntry* entry;

	editposition = self->workspace->sequenceselection.editposition;
	entry = psy_audio_sequenceposition_entry(&editposition);
	if (entry) {
		psy_audio_Pattern* pattern;
		psy_List* tracknode;

		pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
		if (pattern) {
			tracknode = psy_audio_sequence_insert(self->state.sequence, editposition,
				psy_audio_patterns_append(self->patterns, psy_audio_pattern_clone(pattern)));
			psy_audio_sequenceselection_seteditposition(self->selection,
				psy_audio_sequence_makeposition(self->state.sequence,
					self->selection->editposition.tracknode,
					tracknode));
			workspace_setsequenceselection(self->workspace,
				self->workspace->sequenceselection);
			sequenceview_updateplayposition(self);
		}
	}
	sequenceduration_update(&self->duration);
}

void sequenceview_ondelentry(SequenceView* self)
{
	psy_audio_SequencePosition editposition;
	psy_audio_SequenceEntryNode* tracknode;

	editposition = self->workspace->sequenceselection.editposition;
	tracknode = psy_audio_sequence_remove(self->state.sequence, editposition);
	if (tracknode) {
		editposition = psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			tracknode);
	} else {
		editposition.trackposition = psy_audio_sequence_last(self->state.sequence,
			editposition.tracknode);
	}
	if (editposition.tracknode == self->state.sequence->tracks &&
		psy_audio_sequence_size(self->state.sequence, editposition.tracknode) == 0) {
		psy_audio_SequencePosition position;

		position.trackposition = psy_audio_sequence_begin(self->state.sequence, editposition.tracknode, 0);
		position.tracknode = editposition.tracknode;
		tracknode = psy_audio_sequence_insert(self->state.sequence, position, 0);
		editposition = psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			tracknode);
	}
	psy_audio_sequenceselection_seteditposition(self->selection, editposition);
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->component);
}

void sequenceview_onincpattern(SequenceView* self)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_SequencePosition editposition;

	editposition = self->workspace->sequenceselection.editposition;
	entry = psy_audio_sequenceposition_entry(&editposition);
	if (entry) {
		psy_audio_sequence_setpatternslot(self->state.sequence, editposition,
			entry->patternslot + 1);
	}
	psy_audio_sequenceselection_seteditposition(self->selection,
		psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			editposition.trackposition.sequencentrynode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
}

void sequenceview_ondecpattern(SequenceView* self)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_SequencePosition editposition;

	editposition = self->workspace->sequenceselection.editposition;
	entry = psy_audio_sequenceposition_entry(&editposition);
	if (entry && entry->patternslot > 0) {
		psy_audio_sequence_setpatternslot(self->state.sequence, editposition,
			entry->patternslot - 1);
	}
	psy_audio_sequenceselection_seteditposition(self->selection,
		psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			editposition.trackposition.sequencentrynode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
}

void sequenceview_onnewtrack(SequenceView* self, psy_ui_Component* sender,
	uintptr_t trackindex)
{
	psy_audio_exclusivelock_enter();
	psy_audio_sequence_appendtrack(self->state.sequence, psy_audio_sequencetrack_allocinit());
	psy_audio_exclusivelock_leave();
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->component);
	sequencelistview_select(&self->listview, trackindex, 0);
}

void sequenceview_ontrackselected(SequenceView* self, psy_ui_Component* sender,
	uintptr_t trackindex)
{
	sequencelistview_select(&self->listview, trackindex, trackindex);
}

void sequenceview_ondeltrack(SequenceView* self, psy_ui_Component* sender,
	uintptr_t trackindex)
{
	psy_audio_SequencePosition position;
	position = psy_audio_sequence_at(self->state.sequence, self->state.selectedtrack,
		trackindex);
	psy_audio_exclusivelock_enter();
	psy_audio_sequence_removetrack(self->state.sequence, position.tracknode);
	psy_audio_exclusivelock_leave();
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->component);
	sequencelistview_select(&self->listview, trackindex, 0);
}

void sequenceview_onclear(SequenceView* self)
{
	psy_audio_SequencePosition sequenceposition;

	if (workspace_song(self->workspace)) {
		psy_audio_exclusivelock_enter();
		workspace_disposesequencepaste(self->workspace);
		psy_audio_sequence_clear(self->state.sequence);
		psy_audio_patterns_clear(&workspace_song(self->workspace)->patterns);
		psy_audio_patterns_insert(&workspace_song(self->workspace)->patterns, 0,
			psy_audio_pattern_allocinit());
		sequenceposition.tracknode =
			psy_audio_sequence_appendtrack(self->state.sequence, psy_audio_sequencetrack_allocinit());
		sequenceposition.trackposition =
			psy_audio_sequence_begin(self->state.sequence, sequenceposition.tracknode, 0);
		psy_audio_sequence_insert(self->state.sequence, sequenceposition, 0);
		psy_audio_sequenceselection_setsequence(self->selection,
			self->state.sequence);
		workspace_setsequenceselection(self->workspace, *self->selection);
		sequenceview_updateplayposition(self);
		psy_audio_exclusivelock_leave();
		psy_ui_component_updateoverflow(&self->listview.component);
		psy_ui_component_invalidate(&self->component);
	}
}

void sequenceview_onrename(SequenceView* self)
{
	sequencelistview_rename(&self->listview);
}

void sequenceview_oncut(SequenceView* self)
{
}

void sequenceview_oncopy(SequenceView* self)
{
	psy_List* p;

	workspace_disposesequencepaste(self->workspace);
	for (p = self->selection->entries; p != NULL; p = p->next) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)p->entry;
		psy_list_append(&self->workspace->sequencepaste,
			psy_audio_sequenceentry_allocinit(entry->patternslot, entry->offset));
	}
}

void sequenceview_onpaste(SequenceView* self)
{
	psy_audio_SequencePosition position;
	psy_List* p;

	position = self->selection->editposition;
	for (p = self->workspace->sequencepaste; p != NULL; psy_list_next(&p)) {
		psy_audio_SequenceEntry* sequenceentry;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		position = psy_audio_sequence_makeposition(self->state.sequence,
			self->selection->editposition.tracknode,
			psy_audio_sequence_insert(self->state.sequence, position,
				psy_audio_sequenceentry_patternslot(sequenceentry)));
	}
	psy_ui_component_invalidate(&self->component);
}

void sequenceview_onsingleselection(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->buttons.singlesel);
	psy_ui_button_disablehighlight(&self->buttons.multisel);
	self->listview.selection->selectionmode = psy_audio_SEQUENCE_SELECTIONMODE_SINGLE;
}

void sequenceview_onmultiselection(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->buttons.multisel);
	psy_ui_button_disablehighlight(&self->buttons.singlesel);
	self->listview.selection->selectionmode = psy_audio_SEQUENCE_SELECTIONMODE_MULTI;
}

void sequenceview_onfollowsong(SequenceView* self, psy_ui_Button* sender)
{
	if (workspace_followingsong(self->workspace)) {
		workspace_stopfollowsong(self->workspace);
	} else {
		workspace_followsong(self->workspace);
	}
}

void sequenceview_onfollowsongchanged(SequenceView* self, Workspace* sender)
{
	if (workspace_followingsong(sender)) {
		psy_ui_checkbox_check(&self->options.followsong);
	} else {
		psy_ui_checkbox_disablecheck(&self->options.followsong);
	}
}

void sequenceview_onshowpatternnames(SequenceView* self, psy_ui_CheckBox* sender)
{
	if (psy_ui_checkbox_checked(sender) != 0) {
		sequencelistview_showpatternnames(&self->listview);
	} else {
		sequencelistview_showpatternslots(&self->listview);
	}
}

void sequenceview_onrecordtweak(SequenceView* self, psy_ui_Button* sender)
{
	if (workspace_recordingtweaks(self->workspace)) {
		workspace_stoprecordtweaks(self->workspace);
	} else {
		workspace_recordtweaks(self->workspace);
	}
}

void sequenceview_onrecordnoteoff(SequenceView* self, psy_ui_Button* sender)
{
	if (psy_audio_player_recordingnoteoff(workspace_player(self->workspace))) {
		psy_audio_player_preventrecordnoteoff(workspace_player(self->workspace));
	} else {
		psy_audio_player_recordnoteoff(workspace_player(self->workspace));
	}
}

void sequenceview_onmultichannelaudition(SequenceView* self, psy_ui_Button* sender)
{
	workspace_player(self->workspace)->multichannelaudition =
		!workspace_player(self->workspace)->multichannelaudition;
}

void sequenceview_onsongchanged(SequenceView* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	if (&workspace->song) {		
		self->patterns = &workspace->song->patterns;		
		self->listview.patterns = &workspace->song->patterns;
		self->listview.selected = 0;
		self->state.sequence = &workspace->song->sequence;
		self->duration.sequence = &workspace->song->sequence;
		if (self->state.sequence && self->state.sequence->patterns) {
			psy_signal_connect(&self->state.sequence->patterns->signal_namechanged,
				&self->listview,
				sequencelistview_onpatternnamechanged);
			psy_signal_connect(&self->state.sequence->sequencechanged,
				self, sequenceview_onsequencechanged);
			psy_signal_connect(&self->state.sequence->sequencechanged,
				self, sequenceview_onsequencechanged);
		}
	} else {		
		self->patterns = NULL;		
		self->listview.patterns = NULL;
		self->listview.selected = 0;
		self->state.sequence = NULL;
	}
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
}

void sequenceview_onsequenceselectionchanged(SequenceView* self, Workspace* sender)
{
	psy_audio_SequencePosition position;
	psy_audio_SequenceTrackNode* p;
	psy_List* q;
	int c = 0;
	int visilines;
	int listviewtop;
	psy_ui_IntSize listviewsize;

	position = sender->sequenceselection.editposition;
	p = sender->song->sequence.tracks;
	while (p != NULL) {
		if (position.tracknode == p) {
			break;
		}
		++c;
		psy_list_next(&p);
	}
	self->state.selectedtrack = c;
	c = 0;
	if (p) {
		q = ((psy_audio_SequenceTrack*)p->entry)->entries;
		while (q) {
			if (q == position.trackposition.sequencentrynode) {
				break;
			}
			++c;
			psy_list_next(&q);
		}
	}
	self->listview.selected = c;
	listviewsize = psy_ui_component_intsize(&self->listview.component);
	visilines = (listviewsize.height - self->state.margin) / self->listview.lineheight;
	listviewtop = psy_ui_component_scrolltop(&self->listview.component) / self->listview.lineheight;
	if (c < listviewtop) {
		psy_ui_component_setscrolltop(&self->listview.component, c * self->listview.lineheight);
	} else if (c > listviewtop + visilines - 1) {
		psy_ui_component_setscrolltop(&self->listview.component, (c - visilines + 1) * self->listview.lineheight);
	}
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void sequenceview_updateplayposition(SequenceView* self)
{
	if (workspace_followingsong(self->workspace) && psy_audio_player_playing(
			workspace_player(self->workspace))) {
		sequenceview_changeplayposition(self);
	}
}

void sequenceview_changeplayposition(SequenceView* self)
{
	psy_audio_SequencePosition editposition;
	psy_audio_SequenceEntry* sequenceentry;
	psy_dsp_big_beat_t startposition;

	editposition = self->workspace->sequenceselection.editposition;
	sequenceentry = psy_audio_sequenceposition_entry(&editposition);
	startposition = sequenceentry->offset;
	psy_audio_exclusivelock_enter();
	psy_audio_player_stop(workspace_player(self->workspace));
	psy_audio_player_setposition(workspace_player(self->workspace),
		startposition);
	psy_audio_player_start(workspace_player(self->workspace));
	psy_audio_exclusivelock_leave();
}

void sequenceview_onsequencechanged(SequenceView* self,
	psy_audio_Sequence* sender)
{
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->component);
	sequenceduration_update(&self->duration);
}

void sequenceview_onconfigure(SequenceView* self, GeneralConfig* config,
	psy_Property* property)
{
	if (self->listview.showpatternnames !=
			generalconfig_showingpatternnames(config)) {
		self->listview.showpatternnames =
			generalconfig_showingpatternnames(config);
		psy_ui_component_invalidate(&self->listview.component);
		if (self->listview.showpatternnames) {
			psy_ui_checkbox_check(&self->options.shownames);
		} else {
			psy_ui_checkbox_disablecheck(&self->options.shownames);
		}
	}
}

void sequenceview_onthemechanged(SequenceView* self, MachineViewConfig* config,
	psy_Property* theme)
{
	sequenceview_updateskin(self);
}

void sequenceview_updateskin(SequenceView* self)
{
	// psy_ui_Colour bg;	
	
	// bg = psy_ui_colour_make(psy_property_at_int(
	//	self->workspace->config.macview.theme, "mv_colour", 0x00232323));
	// psy_ui_component_setbackgroundcolour(&self->component, bg);
}

void sequenceview_onalign(SequenceView* self, psy_ui_Component* sender)
{
	sequencelistview_computetextsizes(&self->listview);
}
