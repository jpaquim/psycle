/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqlistview.h"
/* host */
#include "trackercmds.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define COLMAX 13

/* SeqViewTrack */
/* prototypes */
static void seqviewtrack_on_destroy(SeqViewTrack*);
static void seqviewtrack_on_preferred_size(SeqViewTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqviewtrack_on_draw(SeqViewTrack*, psy_ui_Graphics*);
static void seqviewtrack_draw_entry(SeqViewTrack*, psy_ui_Graphics*,
	psy_audio_SequenceEntry*, uintptr_t row, psy_ui_RealPoint,
	bool progress, bool sel);
static void seqviewtrack_textout_digit(SeqViewTrack*,
	psy_ui_Graphics*, const char* str, uintptr_t num, psy_ui_RealPoint,
	uintptr_t cursorcol);
static void seqviewtrack_draw_progress_bar(SeqViewTrack*,
	psy_ui_Graphics*, psy_ui_RealPoint);
void seqviewtrack_on_mouse_down(SeqViewTrack*, psy_ui_MouseEvent*);
void seqviewtrack_on_mouse_doubleclick(SeqViewTrack*, psy_ui_MouseEvent*);
static void seqviewtrack_on_sequence_select(SeqViewTrack*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqviewtrack_on_sequence_deselect(SeqViewTrack*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);

/* vtable */
static psy_ui_ComponentVtable seqviewtrack_vtable;
static bool seqviewtrack_vtable_initialized = FALSE;

static void seqviewtrack_vtable_init(SeqViewTrack* self)
{
	if (!seqviewtrack_vtable_initialized) {
		seqviewtrack_vtable = *(self->component.vtable);
		seqviewtrack_vtable.on_destroy =
			(psy_ui_fp_component_event)
			seqviewtrack_on_destroy;
		seqviewtrack_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqviewtrack_on_draw;
		seqviewtrack_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			seqviewtrack_on_preferred_size;
		seqviewtrack_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			seqviewtrack_on_mouse_down;
		seqviewtrack_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			seqviewtrack_on_mouse_doubleclick;		
		seqviewtrack_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqviewtrack_vtable;
}

/* implementation */
void seqviewtrack_init(SeqViewTrack* self, psy_ui_Component* parent,
	uintptr_t trackindex, psy_audio_SequenceTrack* track, SeqViewState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	seqviewtrack_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component,
		STYLE_SEQLISTVIEW_TRACK);
	psy_ui_component_set_style_type_select(&self->component,		
		STYLE_SEQLISTVIEW_TRACK_SELECT);	
	self->state = state;
	self->trackindex = trackindex;
	self->track = track;
	psy_signal_connect(
		&state->cmds->workspace->song->sequence.sequenceselection.signal_select,
		self, seqviewtrack_on_sequence_select);
	psy_signal_connect(
		&state->cmds->workspace->song->sequence.sequenceselection.signal_deselect,
		self, seqviewtrack_on_sequence_deselect);
	if (psy_audio_sequenceselection_first(&state->cmds->workspace->song->sequence.sequenceselection).track ==
			trackindex) {
		psy_ui_component_add_style_state(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

SeqViewTrack* seqviewtrack_alloc(void)
{
	return (SeqViewTrack*)malloc(sizeof(SeqViewTrack));
}

SeqViewTrack* seqviewtrack_allocinit(psy_ui_Component* parent,
	uintptr_t trackindex, psy_audio_SequenceTrack* track,
	SeqViewState* state)
{
	SeqViewTrack* rv;

	rv = seqviewtrack_alloc();
	if (rv) {
		seqviewtrack_init(rv, parent, trackindex, track, state);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void seqviewtrack_on_destroy(SeqViewTrack* self)
{
	if (&self->state->cmds->workspace->song) {
		psy_audio_Sequence* sequence;

		sequence = &self->state->cmds->workspace->song->sequence;
		psy_signal_disconnect(
			&sequence->sequenceselection.signal_select,
			self, seqviewtrack_on_sequence_select);
		psy_signal_disconnect(
			&sequence->sequenceselection.signal_deselect,
			self, seqviewtrack_on_sequence_deselect);
	}
}

void seqviewtrack_on_draw(SeqViewTrack* self, psy_ui_Graphics* g)
{
	psy_List* p;
	uintptr_t row;
	psy_ui_RealPoint cp;	
	uintptr_t startrow;
	uintptr_t endrow;		
	double lineheightpx;	
	const psy_ui_TextMetric* tm;
	psy_audio_OrderIndex editposition;	
	psy_ui_RealRectangle clip;
		
	clip = psy_ui_graphics_cliprect(g);
	tm = psy_ui_component_textmetric(&self->component);
	lineheightpx = psy_max(1.0, floor(psy_ui_value_px(&self->state->line_height,
		tm, NULL)));
	self->state->colwidth = floor(tm->tmAveCharWidth * 1.4);	
	self->state->digitsize = psy_ui_realsize_make(self->state->colwidth,
		lineheightpx);
	startrow = (uintptr_t)floor(psy_max(0, (clip.top / lineheightpx)));
	endrow = (uintptr_t)(floor(clip.bottom / lineheightpx + 0.5));		
	psy_ui_settextcolour(g, psy_ui_style(STYLE_SEQLISTVIEW_ITEM)->colour);
	psy_ui_realpoint_init_all(&cp, 0.0, lineheightpx * startrow);
	p = psy_list_at(self->track->nodes, startrow);
	editposition = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->song->sequence.sequenceselection);
	for (row = startrow; p != NULL; psy_list_next(&p), ++row,
			cp.y += lineheightpx) {
		psy_audio_SequenceEntry* seqentry;
		bool rowplaying;
		
		seqentry = (psy_audio_SequenceEntry*)p->entry;
		rowplaying = psy_audio_player_playing(self->state->cmds->player) &&
			psy_audio_player_playlist_position(self->state->cmds->player) == row;		
		if (rowplaying) {
			seqviewtrack_draw_progress_bar(self, g, cp);
		}				
		seqviewtrack_draw_entry(self, g, seqentry, row, cp, rowplaying,
			psy_audio_sequenceselection_is_selected(
			&self->state->cmds->workspace->song->sequence.sequenceselection,
			psy_audio_orderindex_make(self->trackindex, row)));
	}
}

void seqviewtrack_draw_entry(SeqViewTrack* self, psy_ui_Graphics* g,
	psy_audio_SequenceEntry* entry, uintptr_t row, psy_ui_RealPoint cp,
	bool rowplaying, bool sel)
{
	psy_ui_Colour bg;
	char text[256];
	bool drawcol;
	
	assert(entry);

	psy_ui_colour_init(&bg);
	drawcol = self->state->active && sel;
	switch (entry->type) {
	case psy_audio_SEQUENCEENTRY_PATTERN: {
		psy_audio_Pattern* pattern;
		psy_audio_SequencePatternEntry* seqpatternentry;

		seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
		pattern = psy_audio_sequencepatternentry_pattern(
			seqpatternentry,
			self->state->cmds->patterns);
		if (self->state->showpatternnames) {
			if (pattern) {
				psy_snprintf(text, 20, "%02X: %s %06.2f", row,
					psy_audio_pattern_name(pattern),
					(float)psy_audio_sequenceentry_offset(entry));
			} else {
				psy_snprintf(text, 20, "%02X:%02X(ERR) %06.2f", (int)row,
					(int)psy_audio_sequencepatternentry_patternslot(
						seqpatternentry),
					(float)psy_audio_sequenceentry_offset(entry));
			}
		} else {
			if (pattern) {
				psy_snprintf(text, 20, "%02X:%02X  %06.2f", row,
					(int)psy_audio_sequencepatternentry_patternslot(
						seqpatternentry),
					(float)psy_audio_sequenceentry_offset(entry));
			} else {
				psy_snprintf(text, 20, "%02X:%02XE %06.2f", row,
					(int)psy_audio_sequencepatternentry_patternslot(
						seqpatternentry),
					(float)psy_audio_sequenceentry_offset(entry));
			}
		}
		break; }
	case psy_audio_SEQUENCEENTRY_SAMPLE: {
		psy_audio_SequenceSampleEntry* seqsampleentry;

		seqsampleentry = (psy_audio_SequenceSampleEntry*)entry;
		psy_snprintf(text, 64, "%02X:%02X:%02X", row,
			(int)psy_audio_sequencesampleentry_samplesindex(seqsampleentry).slot,
			(int)psy_audio_sequencesampleentry_samplesindex(seqsampleentry).subslot);
		break; }
	case psy_audio_SEQUENCEENTRY_MARKER: {
		psy_audio_SequenceMarkerEntry* seqmarkerentry;

		seqmarkerentry = (psy_audio_SequenceMarkerEntry*)entry;
		if (seqmarkerentry->text) {
			psy_snprintf(text, 20, "%02X:%s", row,
				seqmarkerentry->text,
				(float)psy_audio_sequenceentry_offset(entry));
		} else {
			text[0] = '\0';
		}
		break; }
	default:
		text[0] = '\0';
		break;
	}

	if (sel) {
		if (!rowplaying) {
			bg = psy_ui_style(STYLE_SEQLISTVIEW_ITEM_SELECT)->background.colour;
		} else {
			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		}
		psy_ui_settextcolour(g, psy_ui_style(STYLE_SEQLISTVIEW_ITEM_SELECT)->colour);
	} else {
		psy_ui_settextcolour(g, psy_ui_style_const(STYLE_SEQLISTVIEW)->colour);
	}
	if (!bg.mode.transparent && !drawcol) {
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
			cp,
			psy_ui_realsize_make(
				psy_ui_value_px(&self->state->trackwidth,
					psy_ui_component_textmetric(&self->component), NULL),
				psy_ui_value_px(&self->state->line_height,
					psy_ui_component_textmetric(&self->component), NULL))),
			bg);
	}
	if (rowplaying) {
		double pos;
		uintptr_t col;
		
		pos = psy_audio_player_rowprogress(self->state->cmds->player, self->trackindex) * 
			psy_ui_value_px(&self->state->trackwidth, psy_ui_component_textmetric(
				&self->component), NULL);		
		col = (uintptr_t)(pos / self->state->colwidth + 0.5);
		col = psy_min(col, strlen(text));
		psy_ui_settextcolour(g, psy_ui_style_const(STYLE_SEQ_PROGRESS)->colour);
		seqviewtrack_textout_digit(self, g, text, col, cp,
			(drawcol) ? self->state->col : psy_INDEX_INVALID);
		psy_ui_settextcolour(g, psy_ui_style_const(STYLE_SEQLISTVIEW)->colour);
		seqviewtrack_textout_digit(self, g, text + col, col, 
			psy_ui_realpoint_make(col * self->state->colwidth, cp.y),
			(drawcol) ? self->state->col : psy_INDEX_INVALID);
	} else {		
		seqviewtrack_textout_digit(self, g, text, psy_strlen(text), cp,
			(drawcol) ? self->state->col : psy_INDEX_INVALID);
	}
}

void seqviewtrack_textout_digit(SeqViewTrack* self,
	psy_ui_Graphics* g, const char* str, uintptr_t num,
	psy_ui_RealPoint pt, uintptr_t cursorcol)
{
	uintptr_t numchars;
	uintptr_t digit;	
	psy_ui_RealPoint cp;	
	
	numchars = psy_strlen(str);
	cp = pt;	
	if (cursorcol != psy_INDEX_INVALID) {
		psy_ui_settextcolour(g,
			psy_ui_style(STYLE_SEQLISTVIEW_ITEM)->colour);
	}
	for (digit = 0; digit < numchars; ++digit) {
		char digitstr[2];

		digitstr[0] = str[digit];
		digitstr[1] = '\n';
		if (digit == cursorcol) {			
			psy_ui_setbackgroundcolour(g,
				psy_ui_style(STYLE_SEQLISTVIEW_ITEM_SELECT)->background.colour);			
			psy_ui_settextcolour(g,
				psy_ui_style(STYLE_SEQLISTVIEW_ITEM_SELECT)->colour);
			psy_ui_textoutrectangle(g,				
				cp, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
				psy_ui_realrectangle_make(cp, self->state->digitsize),
				digitstr, 1);
			psy_ui_settextcolour(g,
				psy_ui_style(STYLE_SEQLISTVIEW_ITEM)->colour);
		} else {			
			psy_ui_textout(g, cp, digitstr, 1);
		}
		cp.x += self->state->colwidth;
	}
}

void seqviewtrack_draw_progress_bar(SeqViewTrack* self, psy_ui_Graphics* g,
	psy_ui_RealPoint pt)
{	
	psy_ui_drawsolidrectangle(g,
		psy_ui_realrectangle_make(pt,
			psy_ui_realsize_make(psy_audio_player_rowprogress(
				self->state->cmds->player, self->trackindex) *
				psy_ui_value_px(&self->state->trackwidth,
				psy_ui_component_textmetric(&self->component), NULL),
			psy_ui_value_px(&self->state->line_height,
				psy_ui_component_textmetric(&self->component), NULL))),
		psy_ui_style_const(STYLE_SEQ_PROGRESS)->background.colour);
}

void seqviewtrack_on_preferred_size(SeqViewTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	rv->width = self->state->trackwidth;
	if (self->track) {
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		rv->height = psy_ui_value_make_px(
			psy_ui_value_px(&self->state->line_height, tm, NULL) *
				(double)psy_list_size(self->track->nodes));
	} else {
		rv->height = psy_ui_value_zero();		
	}			
}

void seqviewtrack_on_mouse_down(SeqViewTrack* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) != 1) {
		return;
	}
	if (self->track && self->state->cmds->workspace->song) {
		psy_audio_Sequence* sequence;
		psy_audio_SequenceCursor cursor;		
		uintptr_t order;

		sequence = &self->state->cmds->workspace->song->sequence;		
		cursor = sequence->cursor;
		if (psy_audio_sequencetrack_size(self->track) > 0) {
			order = psy_min((uintptr_t)((psy_ui_mouseevent_pt(ev).y) /
					psy_ui_value_px(&self->state->line_height,
						psy_ui_component_textmetric(&self->component), NULL)),
				psy_audio_sequencetrack_size(self->track) - 1);
		} else {
			order = psy_INDEX_INVALID;			
		}
		self->state->cmd_orderindex = psy_audio_orderindex_make(self->trackindex, order);
		if (self->state->active) {
			self->state->col = (uintptr_t)(
				psy_ui_mouseevent_pt(ev).x / self->state->colwidth);
			self->state->col = psy_min(self->state->col, COLMAX - 1);
		}
		if (order != psy_INDEX_INVALID) {
			if (psy_ui_mouseevent_ctrlkey(ev)) {
				if (!psy_audio_sequenceselection_is_selected(
						&sequence->sequenceselection,
						self->state->cmd_orderindex)) {
					psy_audio_sequenceselection_select_first(
						&sequence->sequenceselection,
						self->state->cmd_orderindex);
				} else {
					psy_audio_sequenceselection_deselect(
						&sequence->sequenceselection,
						self->state->cmd_orderindex);
				}
			} else {				
				cursor.orderindex = self->state->cmd_orderindex;
				cursor.absoffset -= cursor.seqoffset;
				cursor.seqoffset = psy_audio_sequence_seqoffset(sequence,
					cursor.orderindex);
				cursor.absoffset += cursor.seqoffset;
				psy_audio_sequence_set_cursor(sequence, cursor);
			}
		} else {						
			cursor = psy_audio_sequencecursor_make(0, 0.0);
			cursor.orderindex = self->state->cmd_orderindex;
			cursor.patternid = psy_INDEX_INVALID;
			psy_audio_sequence_set_cursor(sequence, cursor);			
		}
	}
}

void seqviewtrack_on_mouse_doubleclick(SeqViewTrack* self, psy_ui_MouseEvent* ev)
{	
	if (self->state->cmd_orderindex.track <
			psy_audio_sequence_width(self->state->cmds->sequence)) {
		psy_audio_SequenceCursor cursor;

		cursor = self->state->cmds->workspace->song->sequence.cursor;		
		self->state->cmd_orderindex.order = (uintptr_t)(
			psy_ui_mouseevent_pt(ev).y /
			psy_ui_value_px(&self->state->line_height,
				psy_ui_component_textmetric(&self->component), NULL));
		cursor.orderindex = self->state->cmd_orderindex;
		psy_audio_sequence_set_cursor(
			&self->state->cmds->workspace->song->sequence, cursor);		
		sequencecmds_changeplayposition(self->state->cmds);		
	}
}

void seqviewtrack_on_sequence_select(SeqViewTrack* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (index->track == self->trackindex) {
		psy_ui_component_add_style_state(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqviewtrack_on_sequence_deselect(SeqViewTrack* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (index->track == self->trackindex) {
		psy_ui_component_remove_style_state(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

/* SeqviewList */
/* prototypes */
static void seqviewlist_on_preferred_size(SeqviewList*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqviewlist_on_playline_changed(SeqviewList*, Workspace* sender);
static void seqviewlist_on_playstatus_changed(SeqviewList*, Workspace* sender);
static psy_ui_RealRectangle seqviewlist_rowrectangle(SeqviewList*,
	uintptr_t row);
static void seqviewlist_invalidaterow(SeqviewList*, uintptr_t row);
static void seqviewlist_changeplayposition(SeqviewList*);
static bool seqviewlist_oninput(SeqviewList*, InputHandler*);
static void seqviewlist_on_focus(SeqviewList*);
static void seqviewlist_on_focuslost(SeqviewList*);
static void seqviewlist_inputdigit(SeqviewList*, uint8_t value);
/* vtable */
static psy_ui_ComponentVtable seqviewlist_vtable;
static psy_ui_ComponentVtable seqviewlist_super_vtable;
static bool seqviewlist_vtable_initialized = FALSE;

static void seqviewlist_vtable_init(SeqviewList* self)
{
	if (!seqviewlist_vtable_initialized) {
		seqviewlist_vtable = *(self->component.vtable);
		seqviewlist_super_vtable = *(self->component.vtable);		
		seqviewlist_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			seqviewlist_on_preferred_size;		
		seqviewlist_vtable.on_focus =
			(psy_ui_fp_component_event)
			seqviewlist_on_focus;
		seqviewlist_vtable.on_focuslost =
			(psy_ui_fp_component_event)
			seqviewlist_on_focuslost;
		seqviewlist_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &seqviewlist_vtable);
}

/* implementation */
void seqviewlist_init(SeqviewList* self, psy_ui_Component* parent,
	SeqViewState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	seqviewlist_vtable_init(self);	
	self->state = state;	
	self->lastplayrow = psy_INDEX_INVALID;	
	self->showpatternnames = generalconfig_showingpatternnames(
		psycleconfig_general(workspace_conf(self->state->cmds->workspace)));	
	psy_ui_component_set_wheel_scroll(&self->component, 1);	
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	if (self->state->cmds->sequence && self->state->cmds->sequence->patterns) {
		psy_signal_connect(
			&self->state->cmds->sequence->patterns->signal_namechanged,
			self, seqviewlist_onpatternnamechanged);
	}
	psy_signal_connect(&self->state->cmds->workspace->signal_play_line_changed, self,
		seqviewlist_on_playline_changed);
	psy_signal_connect(&self->state->cmds->workspace->signal_play_status_changed, self,
		seqviewlist_on_playstatus_changed);
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make(self->state->trackwidth, self->state->line_height));
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_SCROLL);		
	seqviewlist_build(self);
	inputhandler_connect(&self->state->cmds->workspace->inputhandler,
		INPUTHANDLER_FOCUS, psy_EVENTDRIVER_CMD, "tracker",
		psy_INDEX_INVALID, self,
		(fp_inputhandler_input)
		seqviewlist_oninput);
	// psy_ui_component_start_timer(&self->component, 0, 200);
}

void seqviewlist_build(SeqviewList* self)
{	
	uintptr_t trackindex = 0;	
	
	psy_ui_component_clear(&self->component);
	for (; trackindex < psy_audio_sequence_width(self->state->cmds->sequence);
			++trackindex) {
		SeqViewTrack* track;

		track = seqviewtrack_allocinit(&self->component,
			trackindex,
			psy_audio_sequence_track_at(self->state->cmds->sequence, trackindex),
			self->state);		
	}
	psy_ui_component_align(&self->component);
}

void seqviewlist_on_preferred_size(SeqviewList* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	seqviewlist_super_vtable.onpreferredsize(&self->component, limit, rv);	
	psy_ui_value_add(&rv->width, &self->state->trackwidth,
		psy_ui_component_textmetric(&self->component), NULL);	
}

void seqviewlist_showpatternnames(SeqviewList* self)
{
	self->showpatternnames = TRUE;	
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_showpatternslots(SeqviewList* self)
{
	self->showpatternnames = FALSE;	
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_on_playline_changed(SeqviewList* self, Workspace* sender)
{
	uintptr_t row;

	row = psy_audio_player_playlist_position(
		self->state->cmds->player);
	if (row != self->lastplayrow) {
		/* invalidate previous sequence row */
		seqviewlist_invalidaterow(self, self->lastplayrow);
		self->lastplayrow = row;
	}	
	seqviewlist_invalidaterow(self, self->lastplayrow);	
}

void seqviewlist_on_playstatus_changed(SeqviewList* self, Workspace* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_onpatternnamechanged(SeqviewList* self,
	psy_audio_Patterns* patterns, uintptr_t slot)
{
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_invalidaterow(SeqviewList* self, uintptr_t row)
{
	if (row != psy_INDEX_INVALID) {
		psy_ui_component_invalidate_rect(&self->component,
			seqviewlist_rowrectangle(self, row));
	}
}

bool seqviewlist_oninput(SeqviewList* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {
		psy_audio_OrderIndex editposition;

		editposition = psy_audio_sequenceselection_first(
			&self->state->cmds->workspace->song->sequence.sequenceselection);
		switch (cmd.id) {
		case CMD_NAVLEFT:
			if (self->state->col > 0) {
				--self->state->col;
				seqviewlist_invalidaterow(self, editposition.order);
			} else {
				self->state->col = COLMAX - 1;
				if (self->state->cmds->workspace->song) {
					psy_audio_sequence_dec_seqpos(
						&self->state->cmds->workspace->song->sequence);
				}				
			}
			break;
		case CMD_NAVRIGHT:
			++self->state->col;
			if (self->state->col == COLMAX) {
				self->state->col = 0;
				if (self->state->cmds->workspace->song) {
					psy_audio_sequence_inc_seqpos(
						&self->state->cmds->workspace->song->sequence);
				}				
			}
			seqviewlist_invalidaterow(self, editposition.order);
			break;
		case CMD_NAVDOWN:
			if (self->state->cmds->workspace->song) {
				psy_audio_sequence_inc_seqpos(
					&self->state->cmds->workspace->song->sequence);
			}			
			break;
		case CMD_NAVUP:
			if (self->state->cmds->workspace->song) {
				psy_audio_sequence_dec_seqpos(
					&self->state->cmds->workspace->song->sequence);
			}			
			break;
		case CMD_COLUMNPREV:
			if (editposition.track > 0) {
				uintptr_t rows;
					
				--editposition.track;
				sequencecmds_update(self->state->cmds);
				rows = psy_audio_sequence_track_size(self->state->cmds->sequence,
					editposition.track);
				if (rows < editposition.order) {
					if (rows > 0) {
						editposition.order = rows - 1;
					} else {
						editposition.order = 0;
					}
				}
				psy_audio_sequenceselection_select_first(
					&self->state->cmds->workspace->song->sequence.sequenceselection,
					editposition);
			}
			break;
		case CMD_COLUMNNEXT:				
			sequencecmds_update(self->state->cmds);
			if (editposition.track + 1 < psy_audio_sequence_width(self->state->cmds->sequence)) {
				uintptr_t rows;

				++editposition.track;
				rows = psy_audio_sequence_track_size(self->state->cmds->sequence,
					editposition.track);
				if (rows < editposition.order) {
					if (rows > 0) {
						editposition.order = rows - 1;
					} else {
						editposition.order = 0;
					}
				}
				psy_audio_sequenceselection_select_first(
					&self->state->cmds->workspace->song->sequence.sequenceselection,
					editposition);
			}
			break;
		case CMD_DIGIT0:
		case CMD_DIGIT1:
		case CMD_DIGIT2:
		case CMD_DIGIT3:
		case CMD_DIGIT4:
		case CMD_DIGIT5:
		case CMD_DIGIT6:
		case CMD_DIGIT7:
		case CMD_DIGIT8:
		case CMD_DIGIT9:
		case CMD_DIGITA:
		case CMD_DIGITB:
		case CMD_DIGITC:
		case CMD_DIGITD:
		case CMD_DIGITE:
		case CMD_DIGITF: {
			int digit = (int)cmd.id - CMD_DIGIT0;
			if (digit != -1) {
				seqviewlist_inputdigit(self, digit);
			}
			break; }
		default:
			break;
		}
	}
	return cmd.id != -1;
}

void seqviewlist_inputdigit(SeqviewList* self, uint8_t digit)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_OrderIndex editposition;

	editposition = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->song->sequence.sequenceselection);
	sequencecmds_update(self->state->cmds);
	entry = psy_audio_sequence_entry(self->state->cmds->sequence,
			editposition);
	if (!entry) {
		return;
	}
	if (entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		psy_audio_SequencePatternEntry* seqpatternentry;

		seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
		/* pattern index */
		if (self->state->col == 3) {
			uintptr_t patidx;
			uint8_t curr;

			curr = seqpatternentry->patternslot & 0xF;
			patidx = seqpatternentry->patternslot & ~(0xFF);
			patidx = patidx | curr | (digit << 4);
			sequencecmds_changepattern(self->state->cmds,
				(int)patidx - (int)seqpatternentry->patternslot);
		} else if (self->state->col == 4) {
			uintptr_t patidx;
			uint8_t curr;

			curr = seqpatternentry->patternslot & 0xF0;
			patidx = seqpatternentry->patternslot & ~(0xFF);
			patidx = patidx | curr | (digit);
			sequencecmds_changepattern(self->state->cmds,
				(int)patidx - (int)seqpatternentry->patternslot);
		}
	}
}

void seqviewlist_on_focus(SeqviewList* self)
{
	psy_audio_OrderIndex editposition;
	
	seqviewlist_super_vtable.on_focus(&self->component);
	editposition = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->song->sequence.sequenceselection);
	self->state->active = TRUE;
	psy_ui_component_setborder(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		NULL);
	psy_ui_component_add_style_state(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		psy_ui_STYLESTATE_SELECT);
	seqviewlist_invalidaterow(self, editposition.order);	
}

void seqviewlist_on_focuslost(SeqviewList* self)
{
	psy_audio_OrderIndex editposition;

	seqviewlist_super_vtable.on_focuslost(&self->component);
	editposition = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->song->sequence.sequenceselection);
	self->state->active = FALSE;
	seqviewlist_invalidaterow(self, editposition.order);	
	psy_ui_component_setborder(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		psy_ui_component_border(&self->component));
	psy_ui_component_remove_style_state(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		psy_ui_STYLESTATE_SELECT);
}

psy_ui_RealRectangle seqviewlist_rowrectangle(SeqviewList* self,
	uintptr_t row)
{
	psy_ui_RealSize size;
	double lineheightpx;
	
	size = psy_ui_component_size_px(&self->component);
	lineheightpx = psy_ui_value_px(&self->state->line_height,
		psy_ui_component_textmetric(&self->component), NULL);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, lineheightpx * row),		
		psy_ui_realsize_make(size.width, lineheightpx + 1));
}
