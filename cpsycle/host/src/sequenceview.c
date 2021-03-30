// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequenceview.h"
// host
#include "styles.h"
// audio
#include <exclusivelock.h>
#include <patterns.h>
#include <songio.h>
#include <sequencecmds.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// SequenceButtons
// implementation
void sequencebuttons_init(SequenceButtons* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin spacing;

	psy_ui_Margin rowmargin;
	psy_ui_Button* buttons[] = {
		&self->incpattern, &self->insertentry, &self->decpattern,
		&self->newentry, &self->delentry, &self->cloneentry,
		&self->clear, &self->rename, &self->copy,
		&self->paste, &self->singlesel, &self->multisel
	};
	uintptr_t i;	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletypes(&self->component, STYLE_SEQVIEW_BUTTONS,
		STYLE_SEQVIEW_BUTTONS, STYLE_SEQVIEW_BUTTONS);
	psy_ui_margin_init_all_em(&spacing, 0.25, 0.5, 0.5, 0.5);
	psy_ui_component_setspacing(&self->component, &spacing);
	psy_ui_component_init(&self->standard, &self->component, NULL);
	psy_ui_component_setalign(&self->standard, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->row0, &self->standard, NULL);
	psy_ui_component_setalign(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row0, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_margin_init_all_em(&rowmargin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(&self->row0, &rowmargin);
	psy_ui_button_init(&self->incpattern, &self->row0, NULL);
	psy_ui_button_preventtranslation(&self->incpattern);
	psy_ui_button_settext(&self->incpattern, "+");
	psy_ui_button_init_text(&self->insertentry, &self->row0, NULL,
		"sequencerview.ins");	
	psy_ui_button_init(&self->decpattern, &self->row0, NULL);
	psy_ui_button_preventtranslation(&self->decpattern);
	psy_ui_button_settext(&self->decpattern, "-");
	psy_ui_component_init(&self->row1, &self->standard, NULL);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row1, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_text(&self->newentry, &self->row1, NULL,
		"sequencerview.new");
	psy_ui_button_init_text(&self->cloneentry, &self->row1, NULL,
		"sequencerview.clone");
	psy_ui_button_init_text(&self->delentry, &self->row1, NULL,
		"sequencerview.del");
	psy_ui_component_init(&self->expand, &self->component, NULL);
	psy_ui_component_setalign(&self->expand, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->toggle, &self->expand, NULL);
	psy_ui_button_preventtranslation(&self->toggle);
	psy_ui_button_settext(&self->toggle, ". . .");
	psy_ui_component_setalign(psy_ui_button_base(&self->toggle),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->block, &self->component, NULL);
	psy_ui_component_setalign(&self->block, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->row2, &self->block, NULL);
	psy_ui_margin_init_all_em(&rowmargin, 0.5, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(&self->row2, &rowmargin);
	psy_ui_component_setalign(&self->row2, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row2, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->clear, &self->row2, NULL,
		"sequencerview.clear");	
	psy_ui_button_init_text(&self->rename, &self->row2, NULL,
		"sequencerview.rename");
	// psy_ui_button_init(&self->cut, &self->component);
	// psy_ui_button_settext(&self->cut, "");
	psy_ui_button_init_text(&self->copy, &self->row2, NULL,
		"sequencerview.copy");
	psy_ui_component_init(&self->row3, &self->block, NULL);
	psy_ui_component_setalign(&self->row3, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row3, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_text(&self->paste, &self->row3, NULL,
		"sequencerview.paste");
	psy_ui_button_init_text(&self->singlesel, &self->row3, NULL,
		"sequencerview.singlesel");
	psy_ui_button_init_text(&self->multisel, &self->row3, NULL,
		"sequencerview.multisel");
	psy_ui_button_highlight(&self->singlesel);
	psy_ui_button_disablehighlight(&self->multisel);
	psy_ui_component_hide(&self->block);

	for (i = 0; i < sizeof(buttons) / sizeof(psy_ui_Button*); ++i) {
		double colwidth;

		colwidth = 10.0;
		psy_ui_button_setcharnumber(buttons[i], colwidth);
		psy_ui_component_setstyletypes(psy_ui_button_base(buttons[i]),
			STYLE_SEQVIEW_BUTTON, STYLE_SEQVIEW_BUTTON_HOVER,
			STYLE_SEQVIEW_BUTTON_SELECT);
		psy_ui_margin_init_all_em(&spacing, 0.5, 0.5, 0.5, 0.5);
		psy_ui_button_setlinespacing(buttons[i], 1.4);
	}
}

void sequencebuttons_onalign(SequenceButtons* self)
{
	uintptr_t numparametercols = 3;
	uintptr_t numrows = 0;
	double colwidth = 0;
	double rowheight = 0;
	double cpx = 0;
	double cpy = 0;
	uintptr_t c = 0;
	intptr_t margin;
	double ident;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	psy_List* p;
	psy_List* q;

	size = psy_ui_component_size(&self->component);
	size = psy_ui_component_preferredsize(&self->component, &size);
	tm = psy_ui_component_textmetric(&self->component);
	ident = 0; // tm->tmAveCharWidth * 0.5;
	margin = 5;
	cpx = ident;
	colwidth = floor((psy_ui_value_px(&size.width, tm) - ident) / numparametercols);
	p = q = psy_ui_component_children(&self->component, 0);
	numrows = (psy_list_size(p) / numparametercols) + 1;
	rowheight = floor(psy_ui_value_px(&size.height, tm) / (double)numrows);
	for (; p != NULL; p = p->next, ++c, cpx += colwidth + margin) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)psy_list_entry(p);
		if (c >= numparametercols) {
			cpx = ident;
			cpy += rowheight + margin;
			c = 0;
		}
		psy_ui_component_setposition(component,
			psy_ui_rectangle_make(
				psy_ui_point_makepx(cpx, cpy),
				psy_ui_size_makepx(colwidth, rowheight)));
	}
	psy_list_free(q);
}

void sequencebuttons_onpreferredsize(SequenceButtons* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	uintptr_t numparametercols = 3;
	double margin = 5;
	double ident = 0;
	uintptr_t c = 0;
	double cpx = 0;
	double cpy = 0;
	double cpxmax = 0;
	double cpymax = 0;
	double colmax = 0;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	psy_List* p;
	psy_List* q;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	ident = tm->tmAveCharWidth * 1;
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
		if (colmax < psy_ui_value_px(&componentsize.width, tm) + margin) {
			colmax = (intptr_t)psy_ui_value_px(&componentsize.width, tm) + margin;
		}
		cpx += (intptr_t)psy_ui_value_px(&componentsize.width, tm) + margin;
		if (cpymax < cpy + psy_ui_value_px(&componentsize.height, tm) + margin) {
			cpymax = floor(cpy + psy_ui_value_px(&componentsize.height, tm) + margin);
		}
	}
	psy_list_free(q);
	cpxmax = numparametercols * colmax;
	*rv = psy_ui_size_makepx(cpxmax + tm->tmAveCharWidth * 2, cpymax);
}

// SequenceListViewState
void sequencelistviewstate_init(SequenceListViewState* self)
{
	self->margin = 5;
	self->trackwidth = 100;	
	self->sequence = NULL;
	self->selection = NULL;
	self->cmd = SEQLVCMD_NONE;
	self->cmdtrack = psy_INDEX_INVALID;
}

// SequenceTrackHeaders
// prototypes
static void sequencetrackheaders_ondestroy(SequenceTrackHeaders*);
static void sequencetrackheaders_onmouseup(SequenceTrackHeaders*,
	psy_ui_MouseEvent*);
static void sequencetrackheaders_onnewtrack(SequenceTrackHeaders*,
	psy_ui_Button* sender);
static void sequencetrackheaders_ondeltrack(SequenceTrackHeaders*,
	TrackBox* sender);
// vtable
static psy_ui_ComponentVtable trackheaderviews_vtable;
static bool trackheaderviews_vtable_initialized = FALSE;

static void trackheaderview_vtable_init(SequenceTrackHeaders* self)
{
	if (!trackheaderviews_vtable_initialized) {
		trackheaderviews_vtable = *(self->component.vtable);
		trackheaderviews_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			sequencetrackheaders_ondestroy;		
		trackheaderviews_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			sequencetrackheaders_onmouseup;
		trackheaderviews_vtable_initialized = TRUE;
	}
}
// implemenetation
void sequencetrackheaders_init(SequenceTrackHeaders* self,
	psy_ui_Component* parent, SequenceListViewState* state)
{	
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all(&spacing,
		psy_ui_value_zero(), psy_ui_value_zero(), psy_ui_value_zero(),
		psy_ui_value_makepx(state->margin));
	psy_ui_component_setspacing(&self->component, &spacing);
	trackheaderview_vtable_init(self);
	self->component.vtable = &trackheaderviews_vtable;
	self->state = state;	
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_makeem(0.0, 2.0));
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setoverflow(&self->component,
		psy_ui_OVERFLOW_HSCROLL);
	psy_ui_component_setmode(&self->component,
		psy_ui_SCROLL_COMPONENTS);
	self->hovertrack = psy_INDEX_INVALID;
	psy_signal_init(&self->signal_newtrack);
	psy_signal_init(&self->signal_deltrack);
	psy_signal_init(&self->signal_trackselected);
	sequencetrackheaders_build(self);
}

void sequencetrackheaders_ondestroy(SequenceTrackHeaders* self)
{
	psy_signal_dispose(&self->signal_newtrack);
	psy_signal_dispose(&self->signal_deltrack);
	psy_signal_dispose(&self->signal_trackselected);
}

void sequencetrackheaders_build(SequenceTrackHeaders* self)
{
	psy_audio_Sequence* sequence;

	psy_ui_component_clear(&self->component);	
	sequence = self->state->sequence;
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Button* newtrack;

		for (t = sequence->tracks, c = 0; t != NULL;
			psy_list_next(&t), ++c) {
			SequenceTrackBox* sequencetrackbox;

			// sequencetrackbox = sequencetrackbox_allocinit(&self->component, &self->component,
			sequencetrackbox = sequencetrackbox_allocinit(&self->component, NULL,
				self->state->sequence, c);
			if (sequencetrackbox) {
				psy_ui_component_setminimumsize(
					sequencetrackbox_base(sequencetrackbox),
					psy_ui_size_make(
						psy_ui_value_makepx(self->state->trackwidth + self->state->margin),
						psy_ui_value_zero()));															
				psy_signal_connect(&sequencetrackbox->trackbox.signal_close, self,
					sequencetrackheaders_ondeltrack);
			}
		}
		// newtrack = psy_ui_button_allocinit(&self->component, &self->component);
		newtrack = psy_ui_button_allocinit(&self->component, NULL);
		if (newtrack) {
			psy_ui_button_settext(newtrack, "sequencerview.new-trk");			
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				sequencetrackheaders_onnewtrack);
		}
	}
	psy_ui_component_align(&self->component);
}

void sequencetrackheaders_onmouseup(SequenceTrackHeaders* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->state->cmd == SEQLVCMD_NEWTRACK) {
		psy_signal_emit(&self->signal_newtrack, self, 1, 
			(uintptr_t)psy_audio_sequence_width(self->state->sequence));
	} else if (self->state->cmd == SEQLVCMD_DELTRACK) {
		psy_signal_emit(&self->signal_deltrack, self, 1,
			self->state->cmdtrack);
	}
	self->state->cmd = SEQLVCMD_NONE;
}

void sequencetrackheaders_onmousemove(SequenceTrackHeaders* self,
	psy_ui_MouseEvent* ev)
{
	uintptr_t selectedtrack;

	selectedtrack = (uintptr_t)(ev->pt.x / self->state->trackwidth);
	if (selectedtrack >= psy_audio_sequence_width(self->state->sequence)) {
		selectedtrack = psy_INDEX_INVALID;
	}
	if (self->hovertrack != selectedtrack) {
		self->hovertrack = selectedtrack;
		psy_ui_component_invalidate(&self->component);
	}
}

void sequencetrackheaders_onnewtrack(SequenceTrackHeaders* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQLVCMD_NEWTRACK;
}

void sequencetrackheaders_ondeltrack(SequenceTrackHeaders* self,
	TrackBox* sender)
{
	self->state->cmd = SEQLVCMD_DELTRACK;
	self->state->cmdtrack = trackbox_trackindex(sender);
}


// SequenceListView
// prototypes
static void sequencelistview_onpreferredsize(SequenceListView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sequencelistview_ondraw(SequenceListView*, psy_ui_Graphics*);
static void sequencelistview_drawtrack(SequenceListView*, psy_ui_Graphics*,
	psy_audio_SequenceTrack*, uintptr_t trackindex, double x);
static void sequencelistview_drawprogressbar(SequenceListView*,
	psy_ui_Graphics*, double x, double y, psy_audio_SequenceEntry*);
static void sequencelistview_onmousedown(SequenceListView*,
	psy_ui_MouseEvent*);
static void sequencelistview_onmousedoubleclick(SequenceListView*,
	psy_ui_MouseEvent*);
static void sequencelistview_ontimer(SequenceListView*, uintptr_t timerid);
static void sequencelistview_onpatternnamechanged(SequenceListView*,
	psy_audio_Patterns*, uintptr_t slot);
static psy_ui_RealRectangle sequencelistview_rowrectangle(SequenceListView*,
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
		sequencelistview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			sequencelistview_ondraw;
		sequencelistview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			sequencelistview_onmousedown;
		sequencelistview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			sequencelistview_onmousedoubleclick;
		sequencelistview_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			sequencelistview_ontimer;
		sequencelistview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			sequencelistview_onpreferredsize;		
		sequencelistview_vtable_initialized = TRUE;
	}
}
// implementation
void sequencelistview_init(SequenceListView* self, psy_ui_Component* parent,
	SequenceListViewState* state, SequenceView* view,
	psy_audio_Patterns* patterns, Workspace* workspace)
{
	// psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	sequencelistview_vtable_init(self);
	self->component.vtable = &sequencelistview_vtable;
	self->state = state;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_edit_init(&self->rename, &self->component);
	psy_signal_connect(&self->rename.component.signal_keydown, self,
		sequencelistview_oneditkeydown);
	psy_ui_component_hide(&self->rename.component);
	// spacing = psy_ui_margin_makeem(1.0, 0.0, 0.0, 0.0);
	// psy_ui_component_setspacing(&self->component, &spacing);
	self->view = view;	
	self->patterns = patterns;
	self->workspace = workspace;		
	self->lineheight = 12;
	self->textoffsety = 0;	
	self->lastplayposition = -1.f;
	self->lastplayrow = psy_INDEX_INVALID;
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
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQLISTVIEW, STYLE_SEQLISTVIEW, STYLE_SEQLISTVIEW_SELECT);
}

void sequencelistview_ondraw(SequenceListView* self, psy_ui_Graphics* g)
{
	psy_audio_SequenceTrackNode* p;
	double cpx = 0;
	uintptr_t trackindex = 0;
	self->foundselected = 0;

	sequencelistview_computetextsizes(self);
	for (p = self->state->sequence->tracks; p != NULL; p = p->next,
		cpx += self->state->trackwidth, ++trackindex) {
		sequencelistview_drawtrack(self, g, (psy_audio_SequenceTrack*)
			psy_list_entry(p), trackindex, cpx);
	}
	if (self->state->selection->editposition.order == 0 && 
			psy_audio_sequence_track_size(self->state->sequence,
			self->state->selection->editposition.track) == 0) {
		double cpy;
		psy_ui_RealRectangle r;

		cpx = self->state->selection->editposition.track * self->state->trackwidth + self->state->margin + 5;
		cpy = self->state->margin;
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
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		rv->width = psy_ui_value_makepx(self->state->margin +
			psy_audio_sequence_width(self->state->sequence) *
				psy_ui_value_px(&self->component.scrollstepx, tm));
		rv->height = psy_ui_value_makepx(
			psy_audio_sequence_maxtracksize(self->state->sequence) *
			psy_ui_value_px(&self->component.scrollstepy, tm));
	} else {
		*rv = psy_ui_size_zero();
	}
}

void sequencelistview_computetextsizes(SequenceListView* self)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm->tmAveCharWidth;
	self->lineheight = floor(tm->tmHeight * 1.2);
	self->textoffsety = (self->lineheight - tm->tmHeight) / 2;
	self->textheight = tm->tmHeight;
	self->state->trackwidth = tm->tmAveCharWidth * 16;
	self->identwidth = tm->tmAveCharWidth * 4;
	self->component.scrollstepy = psy_ui_value_makepx(self->lineheight);
	self->component.scrollstepx = psy_ui_value_makepx(self->state->trackwidth);
}

void sequencelistview_drawtrack(SequenceListView* self, psy_ui_Graphics* g,
	psy_audio_SequenceTrack* track, uintptr_t trackindex, double x)
{
	psy_List* p;
	uintptr_t c;
	double cpy = 0;
	char text[20];
	psy_ui_RealRectangle r;
	psy_ui_RealSize size;
	uintptr_t startrow;
	uintptr_t endrow;

	startrow = (uintptr_t)floor(psy_max(0, (g->clip.top - self->state->margin) / self->lineheight));
	endrow = (uintptr_t)(floor(g->clip.bottom - self->state->margin + 0.5) / self->lineheight);
	size = psy_ui_component_sizepx(&self->component);
	psy_ui_setrectangle(&r, x, 0, self->state->trackwidth - 5, size.height);
	psy_ui_settextcolour(g, psy_ui_colour_make(0));	
	cpy = self->lineheight * startrow;
	p = psy_list_at(p = track->entries, startrow);
	for (c = startrow; p != NULL; psy_list_next(&p), ++c, cpy += self->lineheight) {
		psy_audio_SequenceEntry* sequenceentry;
		bool rowplaying = FALSE;

		sequenceentry = (psy_audio_SequenceEntry*)p->entry;		
		rowplaying = psy_audio_player_playing(self->player) &&
			psy_audio_player_playlist_position(self->player) == c;
		if (self->showpatternnames) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_sequenceentry_pattern(sequenceentry,
				self->patterns);
			if (pattern) {
				psy_snprintf(text, 20, "%02X: %s %4.2f", c,
					psy_audio_pattern_name(pattern),
					(float)sequenceentry->offset);
			} else {
				psy_snprintf(text, 20, "%02X:%02X(ERR) %4.2f", c,
					(int)psy_audio_sequenceentry_patternslot(sequenceentry),
					(float)sequenceentry->offset);
			}
		} else {
			psy_snprintf(text, 20, "%02X:%02X  %4.2f", c,
				(int)psy_audio_sequenceentry_patternslot(sequenceentry),
				(float)sequenceentry->offset);
		}
		if (rowplaying) {
			sequencelistview_drawprogressbar(self, g, x, cpy, sequenceentry);
		}
		if (psy_audio_sequenceselection_isselected(self->state->selection,
				psy_audio_orderindex_make(trackindex, c))) {				
			if (!rowplaying) {
				psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));
			} else {
				psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			}
			psy_ui_settextcolour(g, self->component.style.select.colour);
			self->foundselected = 1;
		} else if (rowplaying) {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_style(STYLE_SEQ_PROGRESS)->colour);
		} else {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, self->component.style.style.colour);
		}
		psy_ui_textout(g, x + 5, cpy + self->state->margin + self->textoffsety, text,
			strlen(text));		
	}
}

void sequencelistview_drawprogressbar(SequenceListView* self,
	psy_ui_Graphics* g, double x, double y,
	psy_audio_SequenceEntry* sequenceentry)
{
	psy_ui_RealRectangle r;

	r = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(x + 5.0, y + self->state->margin),
		psy_ui_realsize_make(
			floor((psy_audio_player_playlist_rowprogress(workspace_player(self->workspace))) *
			(self->state->trackwidth - 5)), self->lineheight));
	psy_ui_drawsolidrectangle(g, r, psy_ui_style(STYLE_SEQ_PROGRESS)->backgroundcolour);
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
	psy_audio_Pattern* pattern;

	pattern = psy_audio_sequence_pattern(self->state->sequence,
		self->state->selection->editposition);			
	if (pattern) {
		psy_ui_component_setposition(&self->rename.component,
			psy_ui_rectangle_make(
				psy_ui_point_makepx(0, self->state->margin),
				psy_ui_size_makeem(20.0, 1.0)));
		psy_ui_edit_settext(&self->rename, psy_audio_pattern_name(pattern));
		psy_ui_component_show(&self->rename.component);
		psy_ui_component_setfocus(&self->rename.component);
	}	
}

void sequencelistview_oneditkeydown(SequenceListView* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		psy_audio_SequenceEntry* entry;

		entry = (self->state->sequence)
			? psy_audio_sequence_entry(self->state->sequence,
				self->state->selection->editposition)
			: NULL;		
		if (entry) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(&self->workspace->song->patterns,
				entry->patternslot);			
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
	if (ev->button == 1) {
		uintptr_t selected;
		uintptr_t selectedtrack;

		sequencelistview_computetextsizes(self);
		selected = (uintptr_t)((ev->pt.y - self->state->margin) / self->lineheight);
		selectedtrack = (uintptr_t)(ev->pt.x / self->state->trackwidth);
		if (selectedtrack < psy_audio_sequence_width(self->state->sequence)) {
			sequencelistview_select(self, selectedtrack, selected);
		}
	} else if (ev->button == 2) {
		sequenceview_toggleedit(self->view);		
	}
}

void sequencelistview_select(SequenceListView* self,
	uintptr_t track, uintptr_t row)
{
	if (track >= psy_audio_sequence_width(self->state->sequence)) {
		if (psy_audio_sequence_width(self->state->sequence) > 0) {
			track = psy_audio_sequence_width(self->state->sequence) - 1;
		} else {
			track = 0;
		}
	}
	workspace_setsequenceeditposition(self->workspace,
		psy_audio_orderindex_make(track, row));	
}

void sequencelistview_onmousedoubleclick(SequenceListView* self,
	psy_ui_MouseEvent* ev)
{
	uintptr_t selected;
	uintptr_t selectedtrack;

	sequencelistview_computetextsizes(self);
	selected = (uintptr_t)((ev->pt.y - self->state->margin) / self->lineheight);
	selectedtrack = (uintptr_t)(ev->pt.x / self->state->trackwidth);
	if (selectedtrack < psy_audio_sequence_width(self->state->sequence)) {
		workspace_setsequenceeditposition(self->workspace,
			psy_audio_orderindex_make(
				selectedtrack, selected));		
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
					!= psy_INDEX_INVALID) {
				sequencelistview_invalidaterow(self,
					psy_audio_player_playlist_position(workspace_player(self->workspace)));
			}
			self->refreshcount = 0;
		}
		++self->refreshcount;
	} else if (self->lastplayrow != psy_INDEX_INVALID) {
		// invalidate if player is stopping to remove the progress bar
		sequencelistview_invalidaterow(self, self->lastplayrow);
		self->lastplayrow = psy_INDEX_INVALID;
	}
}

void sequencelistview_onpatternnamechanged(SequenceListView* self, psy_audio_Patterns* patterns,
	uintptr_t slot)
{
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_invalidaterow(SequenceListView* self, uintptr_t row)
{
	if (row != psy_INDEX_INVALID) {
		psy_ui_component_invalidaterect(&self->component,
			sequencelistview_rowrectangle(self, row));
	}
}

psy_ui_RealRectangle sequencelistview_rowrectangle(SequenceListView* self,
	uintptr_t row)
{
	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, self->lineheight * row + self->state->margin),
		psy_ui_realsize_make(size.width, self->lineheight));
}

// SequenceViewDuration
// prototypes
static void sequenceduration_ontimer(SequenceViewDuration* self, psy_ui_Component* sender,
	uintptr_t id);
static void sequenceduration_ondestroy(SequenceViewDuration* self, psy_ui_Component* sender);	
// implementation
void sequenceduration_init(SequenceViewDuration* self, psy_ui_Component* parent,
	psy_audio_Sequence* sequence, Workspace* workspace)
{
	psy_ui_Margin margin;

	self->sequence = sequence;
	self->workspace = workspace;
	self->duration_ms = 0;
	self->duration_bts = 0.0;
	psy_ui_margin_init_all_em(&margin, 0.5, 2.0, 0.5, 0.0);
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_label_init(&self->desc, &self->component, NULL);
	psy_ui_label_setcharnumber(&self->desc, 10);
	psy_ui_label_settext(&self->desc, "sequencerview.duration");
	psy_ui_label_settextalignment(&self->desc, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->desc.component, &margin);	
	psy_ui_label_init(&self->duration, &self->component, NULL);	
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_all_em(&margin, 0.5, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(&self->duration.component, &margin);
	psy_ui_label_setcharnumber(&self->duration, 18);
	psy_ui_label_preventtranslation(&self->duration);
	psy_ui_component_setstyletypes(psy_ui_label_base(&self->duration),
		STYLE_DURATION_TIME, STYLE_DURATION_TIME, STYLE_DURATION_TIME);
	self->calcduration = FALSE;
	psy_signal_connect(&self->component.signal_timer, self,
		sequenceduration_ontimer);	
	psy_signal_connect(&self->component.signal_destroy, self,
		sequenceduration_ontimer);
	psy_ui_component_starttimer(&self->component, 0, 50);
	sequenceduration_update(self);
}

void sequenceduration_ondestroy(SequenceViewDuration* self, psy_ui_Component* sender)
{	
	psy_ui_component_stoptimer(&self->component, 0);
	sequenceduration_stopdurationcalc(self);
}

void sequenceduration_stopdurationcalc(SequenceViewDuration* self)
{
	if (self->calcduration) {
		psy_audio_sequence_endcalcdurationinmsresult(self->sequence);
		self->calcduration = FALSE;
	}
}

void sequenceduration_update(SequenceViewDuration* self)
{			
	psy_dsp_big_beat_t duration_bts;
	
	duration_bts = psy_audio_sequence_duration(self->sequence);	
	if (self->duration_bts != duration_bts) {
		char text[64];

		self->duration_bts = duration_bts;
		if (self->calcduration) {
			self->duration_ms =
				psy_audio_sequence_endcalcdurationinmsresult(self->sequence);
			self->calcduration = FALSE;
		}
		if (!self->calcduration) {
			self->calcduration = TRUE;
			psy_audio_sequence_startcalcdurationinms(self->sequence);
			sequenceduration_ontimer(self, &self->component, 0);
		}
		psy_snprintf(text, 64, "--m--s %.2fb",	(float)self->duration_bts);
		psy_ui_label_settext(&self->duration, text);
	}
}

void sequenceduration_ontimer(SequenceViewDuration* self, psy_ui_Component* sender,
	uintptr_t id)
{
	if (self->calcduration) {
		uintptr_t i;

		for (i = 0; i < 20; ++i) {
			if (!psy_audio_sequence_calcdurationinms(self->sequence)) {
				char text[64];

				self->duration_ms = psy_audio_sequence_endcalcdurationinmsresult(self->sequence);
				psy_snprintf(text, 64, " %02dm%02ds %.2fb",
					(int)(self->duration_ms / 60), ((int)self->duration_ms % 60),
					(float)self->duration_bts);
				psy_ui_label_settext(&self->duration, text);
				self->calcduration = FALSE;
				break;
			}
		}
	}
}

// SequencerOptionsBar
// implementation
void sequenceroptionsbar_init(SequencerOptionsBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.25, 0.0);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		margin);
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
		// seqedit buttons
		psy_ui_component_init(&self->seqedit, &self->component, NULL);
		psy_ui_component_setalign(&self->seqedit, psy_ui_ALIGN_BOTTOM);		
		psy_ui_button_init_text(&self->toggleseqedit, &self->seqedit, NULL,
			"sequencerview.showseqeditor");
		psy_ui_component_setalign(&self->toggleseqedit.component,
			psy_ui_ALIGN_LEFT);
		psy_ui_button_seticon(&self->toggleseqedit, psy_ui_ICON_MORE);
		// stepseq buttons
		psy_ui_component_init(&self->stepseq, &self->component, NULL);
		psy_ui_component_setalign(&self->stepseq, psy_ui_ALIGN_BOTTOM);		
		psy_ui_button_init_text(&self->togglestepseq, &self->stepseq, NULL,
			"sequencerview.showstepsequencer");
		psy_ui_button_seticon(&self->togglestepseq, psy_ui_ICON_MORE);
		psy_ui_component_setalign(&self->togglestepseq.component,
			psy_ui_ALIGN_LEFT);		
	}	
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setmargin(&self->allownotestoeffect.component, &margin);
}

// SequenceView
// prototypes
static void sequenceview_ontoggleedit(SequenceView*, psy_ui_Component* sender);
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
static void sequenceview_onsongchanged(SequenceView*, Workspace*, int flag,
	psy_audio_Song* song);
static void sequenceview_onsequenceselectionchanged(SequenceView*, psy_audio_SequenceSelection*);
static void sequenceview_onsequencetrackreposition(SequenceView*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void sequenceview_onsequencechanged(SequenceView*,
	psy_audio_Sequence* sender);
static void sequenceview_onconfigure(SequenceView*, GeneralConfig*,
	psy_Property*);
static void sequenceview_onalign(SequenceView*, psy_ui_Component* sender);
// implementation
void sequenceview_init(SequenceView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	// shared state
	sequencelistviewstate_init(&self->state);
	if (workspace->song) {		
		self->patterns = &workspace->song->patterns;
		self->state.selection = &workspace->sequenceselection;
		self->state.sequence = &workspace->song->sequence;
	} else {		
		self->patterns = NULL;
		self->state.sequence = NULL;
	}
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);	
	// sequence listview
	sequencelistview_init(&self->listview, &self->component,
		&self->state, self, self->patterns, workspace);	
	psy_ui_scroller_init(&self->scroller, &self->listview.component,
		&self->component, NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	self->listview.player = &workspace->player;
	// button bar
	sequencebuttons_init(&self->buttons, &self->component, workspace);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	// spacer
	psy_ui_component_init(&self->spacer, &self->component, NULL);
	psy_ui_component_setpreferredsize(&self->spacer, psy_ui_size_makeem(0.0, 0.3));
	psy_ui_component_preventalign(&self->spacer);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_TOP);
	// header
	sequencetrackheaders_init(&self->trackheader, &self->component,
		&self->state);
	psy_ui_component_setalign(&self->trackheader.component, psy_ui_ALIGN_TOP);
	// options
	sequenceroptionsbar_init(&self->options, &self->component, workspace);	
	psy_ui_component_setalign(&self->options.component, psy_ui_ALIGN_BOTTOM);
	// duration
	sequenceduration_init(&self->duration, &self->component, self->state.sequence, workspace);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&self->buttons.toggle.signal_clicked, self,
		sequenceview_ontoggleedit);
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
	psy_signal_connect(&workspace->signal_followsongchanged, self,
		sequenceview_onfollowsongchanged);	
	psy_signal_connect(&workspace->sequenceselection.signal_changed, self,
		sequenceview_onsequenceselectionchanged);
	if (self->state.sequence && self->state.sequence->patterns) {
		psy_signal_connect(&self->state.sequence->patterns->signal_namechanged,
			&self->listview,
			sequencelistview_onpatternnamechanged);
		psy_signal_connect(&self->state.sequence->signal_changed,
			self, sequenceview_onsequencechanged);
		psy_signal_connect(&self->state.sequence->signal_trackreposition,
			self, sequenceview_onsequencetrackreposition);
	}
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed, self,
		sequenceview_onconfigure);	
	psy_signal_connect(&self->component.signal_align, self,
		sequenceview_onalign);
}

void sequenceview_ontoggleedit(SequenceView* self, psy_ui_Component* sender)
{
	sequenceview_toggleedit(self);
}

void sequenceview_toggleedit(SequenceView* self)
{
	if (psy_ui_component_visible(&self->buttons.block)) {
		psy_ui_component_hide(&self->buttons.block);		
		psy_ui_button_seticon(&self->buttons.toggle, psy_ui_ICON_NONE);
		psy_ui_button_settext(&self->buttons.toggle, ". . .");
		psy_ui_component_align(&self->component);		
	} else {
		psy_ui_button_settext(&self->buttons.toggle, "-");		
		psy_ui_component_show(&self->buttons.block);
		psy_ui_component_align(&self->component);		
	}
}

void sequenceview_onnewentry(SequenceView* self)
{	
	psy_audio_Pattern* newpattern;
	uintptr_t patidx;

	newpattern = psy_audio_pattern_allocinit();
	// change length to default lines
	psy_audio_pattern_setlength(newpattern,
		(psy_dsp_big_beat_t)
		(psy_audio_pattern_defaultlines() /
		(psy_audio_player_lpb(workspace_player(self->workspace)))));
	patidx = psy_audio_patterns_append(self->patterns, newpattern);
	psy_undoredo_execute(&self->workspace->undoredo,
		&psy_audio_sequenceinsertcommand_alloc(self->state.sequence,
			&self->workspace->sequenceselection,
			self->state.selection->editposition,
			patidx)->command);
}

void sequenceview_oninsertentry(SequenceView* self)
{	
	psy_audio_SequenceEntry* entry;

	entry = psy_audio_sequence_entry(self->state.sequence,
		self->state.selection->editposition);
	if (entry) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceinsertcommand_alloc(self->state.sequence,
				&self->workspace->sequenceselection,
				self->state.selection->editposition,
				entry->patternslot)->command);
	}
}

void sequenceview_oncloneentry(SequenceView* self)
{
	psy_audio_Pattern* pattern;	
	
	pattern = psy_audio_sequence_pattern(self->state.sequence,
		self->state.selection->editposition);
	if (pattern) {
		psy_audio_Pattern* newpattern;
		uintptr_t patidx;

		newpattern = psy_audio_pattern_clone(pattern);
		patidx = psy_audio_patterns_append(self->patterns, newpattern);
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceinsertcommand_alloc(self->state.sequence,
				&self->workspace->sequenceselection,
				self->state.selection->editposition,
				patidx)->command);
	}
}

void sequenceview_ondelentry(SequenceView* self)
{
	psy_undoredo_execute(&self->workspace->undoredo,
		&psy_audio_sequenceremovecommand_alloc(self->state.sequence,
			&self->workspace->sequenceselection)->command);
}

void sequenceview_onincpattern(SequenceView* self)
{
	psy_undoredo_execute(&self->workspace->undoredo,
		&psy_audio_sequencechangepatterncommand_alloc(self->state.sequence,
			&self->workspace->sequenceselection, 1)->command);
}

void sequenceview_ondecpattern(SequenceView* self)
{
	psy_undoredo_execute(&self->workspace->undoredo,
		&psy_audio_sequencechangepatterncommand_alloc(self->state.sequence,
			&self->workspace->sequenceselection, -1)->command);
}

void sequenceview_onnewtrack(SequenceView* self, psy_ui_Component* sender,
	uintptr_t trackindex)
{
	psy_audio_exclusivelock_enter();
	psy_audio_sequence_appendtrack(self->state.sequence,
		psy_audio_sequencetrack_allocinit());
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
	position = psy_audio_sequence_at(self->state.sequence,
		self->state.selection->editposition.track,
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
	if (workspace_song(self->workspace)) {
		workspace_selectview(self->workspace, VIEW_ID_CHECKUNSAVED, 0,
			CONFIRM_SEQUENCECLEAR);
	}
}

void sequenceview_clear(SequenceView* self)
{
	psy_audio_exclusivelock_enter();
	workspace_clearsequencepaste(self->workspace);
	// no undo/redo
	psy_audio_patterns_clear(&workspace_song(self->workspace)->patterns);
	psy_audio_patterns_insert(&workspace_song(self->workspace)->patterns, 0,
		psy_audio_pattern_allocinit());
	// order can be restored but not patterndata
	// psycle mfc behaviour
	psy_undoredo_execute(&self->workspace->undoredo,
		&psy_audio_sequenceclearcommand_alloc(self->state.sequence,
			&self->workspace->sequenceselection)->command);
	psy_audio_exclusivelock_leave();
	workspace_setsequenceeditposition(self->workspace,
		psy_audio_orderindex_make(0, 0));
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
	psy_audio_sequencepaste_copy(
		&self->workspace->sequencepaste,
		self->state.sequence,
		&self->workspace->sequenceselection);	
}

void sequenceview_onpaste(SequenceView* self)
{
	psy_List* p;		

	assert(self);	;
	
	for (p = self->workspace->sequencepaste.entries; p != NULL; psy_list_next(&p)) {
		psy_audio_Order* order;
		psy_audio_OrderIndex insertposition;
		psy_audio_SequenceEntry* newentry;

		order = (psy_audio_Order*)psy_list_entry(p);
		insertposition = self->workspace->sequenceselection.editposition;		
		insertposition.order += order->index.order;
		insertposition.track += order->index.track;
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceinsertcommand_alloc(self->state.sequence,
				&self->workspace->sequenceselection,
				insertposition,
				order->entry.patternslot)->command);
		newentry = (psy_audio_SequenceEntry*)
			psy_audio_sequence_entry(self->state.sequence, insertposition);
		if (newentry) {
			*newentry = order->entry;
		}
	}	
}

void sequenceview_onsingleselection(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->buttons.singlesel);
	psy_ui_button_disablehighlight(&self->buttons.multisel);
	psy_audio_sequenceselection_setmode(self->state.selection,
		psy_audio_SEQUENCESELECTION_SINGLE);	
	psy_audio_sequenceselection_update(&self->workspace->sequenceselection);
}

void sequenceview_onmultiselection(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->buttons.multisel);
	psy_ui_button_disablehighlight(&self->buttons.singlesel);
	psy_audio_sequenceselection_setmode(self->state.selection,
		psy_audio_SEQUENCESELECTION_MULTI);	
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
	int flag, psy_audio_Song* song)
{
	if (&workspace->song) {		
		self->patterns = &workspace->song->patterns;		
		self->listview.patterns = &workspace->song->patterns;		
		self->state.sequence = &workspace->song->sequence;
		sequenceduration_stopdurationcalc(&self->duration);
		self->duration.sequence = &workspace->song->sequence;
		if (self->state.sequence && self->state.sequence->patterns) {
			psy_signal_connect(&self->state.sequence->patterns->signal_namechanged,
				&self->listview,
				sequencelistview_onpatternnamechanged);
			psy_signal_connect(&self->state.sequence->signal_changed,
				self, sequenceview_onsequencechanged);
			psy_signal_connect(&self->state.sequence->signal_trackreposition,
				self, sequenceview_onsequencetrackreposition);
		}
	} else {		
		self->patterns = NULL;		
		self->listview.patterns = NULL;		
		self->state.sequence = NULL;
	}
	sequencetrackheaders_build(&self->trackheader);
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
}

void sequenceview_onsequenceselectionchanged(SequenceView* self, psy_audio_SequenceSelection* sender)
{
	uintptr_t c = 0;
	double visilines;
	double listviewtop;
	psy_ui_RealSize listviewsize;

	sequenceview_updateplayposition(self);	
	c = sender->editposition.order;
	if (c == psy_INDEX_INVALID) {
		c = 0;
	}
	listviewsize = psy_ui_component_sizepx(&self->listview.component);
	visilines = (listviewsize.height - self->state.margin) / self->listview.lineheight;
	listviewtop = psy_ui_component_scrolltoppx(&self->listview.component) / self->listview.lineheight;
	if ((double)c < listviewtop) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_makepx(c * self->listview.lineheight));
	} else if ((double)c > listviewtop + visilines - 1) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_makepx((c - visilines + 1) * self->listview.lineheight));
	}
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void sequenceview_updateplayposition(SequenceView* self)
{
	if (workspace_followingsong(self->workspace) && psy_audio_player_playing(
			workspace_player(self->workspace))) {
		//sequenceview_changeplayposition(self);
	}
}

void sequenceview_changeplayposition(SequenceView* self)
{	
	psy_audio_SequenceEntry* entry;
	psy_dsp_big_beat_t startposition;

	entry = psy_audio_sequence_entry(self->state.sequence,
		self->state.selection->editposition);
	psy_audio_exclusivelock_enter();
	psy_audio_sequence_setplayselection(self->state.sequence,
		self->state.selection);
	psy_audio_exclusivelock_leave();
	if (entry) {
		startposition = entry->offset;
		psy_audio_exclusivelock_enter();
		psy_audio_player_stop(workspace_player(self->workspace));
		psy_audio_player_setposition(workspace_player(self->workspace),
			startposition);		
		psy_audio_player_start(workspace_player(self->workspace));
		psy_audio_exclusivelock_leave();
	}
}

void sequenceview_onsequencechanged(SequenceView* self,
	psy_audio_Sequence* sender)
{		
	sequenceduration_stopdurationcalc(&self->duration);
	sequencetrackheaders_build(&self->trackheader);
	sequenceduration_update(&self->duration);	
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
}

void sequenceview_onsequencetrackreposition(SequenceView* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	sequenceduration_update(&self->duration);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
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

void sequenceview_onalign(SequenceView* self, psy_ui_Component* sender)
{
	sequencelistview_computetextsizes(&self->listview);
}
