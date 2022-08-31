/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerlinenumbers.h"
/* local */
#include "patternview.h"
#include "patternviewconfig.h"
/* platform */
#include "../../detail/portable.h"

/* TrackerLineNumbers */

/* prototypes */
static void trackerlinenumbers_connect_configuration(TrackerLineNumbers*);
static void trackerlinenumbers_on_draw(TrackerLineNumbers*, psy_ui_Graphics*);
static TrackerColumnFlags trackerlinennumbers_column_flags(TrackerLineNumbers*,
	intptr_t line, intptr_t seqline);
static void trackerlinennumbers_draw_text(TrackerLineNumbers*, psy_ui_Graphics*,
	const char* format, double y, double width, const char* text);
static void trackerlinenumbers_invalidate_cursor_internal(TrackerLineNumbers*,
	const psy_audio_SequenceCursor*);
static void trackerlinenumbers_on_align(TrackerLineNumbers*);
static void trackerlinenumbers_on_preferred_size(TrackerLineNumbers*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void trackerlinenumbers_update_size(TrackerLineNumbers*);
static void trackerlinenumbers_configure(TrackerLineNumbers*);
static void trackerlinenumbers_on_show_beat_offset(TrackerLineNumbers*,
	psy_Property* sender);
static void trackerlinenumbers_on_show_cursor(TrackerLineNumbers*,
	psy_Property* sender);
static void trackerlinenumbers_on_show_in_hex(TrackerLineNumbers*,
	psy_Property* sender);
static void trackerlinennumbers_save_colours(TrackerLineNumbers*,
	psy_ui_Graphics*);	
static void trackerlinennumbers_set_colours(TrackerLineNumbers*,
	psy_ui_Graphics*, psy_ui_Style*);
static void trackerlinennumbers_text(TrackerLineNumbers*, int line,
	int pat_idx, char* rv);	

/* vtable */
static psy_ui_ComponentVtable trackerlinenumbers_vtable;
static bool trackerlinenumbers_vtable_initialized = FALSE;

static void trackerlinenumbers_vtable_init(TrackerLineNumbers* self)
{
	if (!trackerlinenumbers_vtable_initialized) {
		trackerlinenumbers_vtable = *(self->component.vtable);
		trackerlinenumbers_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackerlinenumbers_on_draw;
		trackerlinenumbers_vtable.onalign =
			(psy_ui_fp_component)
			trackerlinenumbers_on_align;
		trackerlinenumbers_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			trackerlinenumbers_on_preferred_size;
		trackerlinenumbers_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component,
		&trackerlinenumbers_vtable);
}

/* implementation */
void trackerlinenumbers_init(TrackerLineNumbers* self,
	psy_ui_Component* parent, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerlinenumbers_vtable_init(self);	
	self->state = state;
	self->workspace = workspace;
	self->show_in_hex = TRUE;
	self->show_beat = FALSE;
	self->draw_cursor = TRUE;
	self->draw_restore_fg_colour = psy_ui_colour_white();
	self->draw_restore_bg_colour = psy_ui_colour_black();
	psy_ui_realsize_init(&self->size);	
	psy_audio_sequencecursor_init(&self->old_cursor);
	trackerlinenumbers_update_format(self);		
	psy_ui_component_set_scroll_step_height(&self->component,
		state->line_height);
	self->component.blitscroll = TRUE;	
	trackerlinenumbers_connect_configuration(self);
	trackerlinenumbers_configure(self);	
}

void trackerlinenumbers_connect_configuration(TrackerLineNumbers* self)
{
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"beatoffset", 
		self, trackerlinenumbers_on_show_beat_offset);
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"linenumberscursor", 
		self, trackerlinenumbers_on_show_cursor);
	patternviewconfig_connect(
		&self->workspace->config.visual.patview,
		"linenumbersinhex", 
		self, trackerlinenumbers_on_show_in_hex);	
}

void trackerlinenumbers_update_format(TrackerLineNumbers* self)
{	
	static const char* format_hex_seqstart = "%.2X %.2X";
	static const char* format_hex = "%X";

	static const char* format_seqstart = "%X %i";
	static const char* format = "%i";

	if (self->show_in_hex) {		
		self->format = format_hex;
		self->format_seqstart = format_hex_seqstart;		
	} else {		
		self->format = format;
		self->format_seqstart = format_seqstart;		
	}
}

void trackerlinenumbers_on_draw(TrackerLineNumbers* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;
	psy_ui_RealRectangle g_clip;
	BeatClip clip;	
	uintptr_t line;
	uintptr_t num_clip_lines;
	intptr_t seqline;
	uintptr_t num_entry_lines;	
	uintptr_t count;	
	uintptr_t patidx;	
	psy_audio_SequenceTrackIterator ite;
	double cpy;	
		
	assert(self);
			
	if (!patternviewstate_sequence(self->state->pv)) {
		return;
	}	
	/* prepare clipping */
	g_clip = psy_ui_graphics_cliprect(g);
	beatclip_init(&clip, &self->state->beat_convert, g_clip.top,
		g_clip.bottom);
	clip.end = psy_min(clip.end, patternviewstate_length(
		self->state->pv));	
	line = beatline_beat_to_line(&self->state->pv->beat_line, clip.begin);
	num_clip_lines = beatline_beat_to_line(&self->state->pv->beat_line,
		clip.end) - line;
	cpy = beatconvert_beat_to_px(&self->state->beat_convert, clip.begin);		
	if (patternviewstate_single_mode(self->state->pv)) {		
		seqline = beatline_beat_to_line(&self->state->pv->beat_line,
			psy_audio_sequencecursor_seqoffset(&self->state->pv->cursor,
			self->state->pv->sequence));
		num_entry_lines = beatline_beat_to_line(&self->state->pv->beat_line,
			patternviewstate_length(self->state->pv));		
	} else {
		psy_audio_sequencetrackiterator_init(&ite);
		psy_audio_sequence_begin(self->state->pv->sequence,			
			self->state->pv->cursor.order_index.track, clip.begin, &ite);				
		patidx = psy_audio_sequencetrackiterator_patidx(&ite);		
		seqline = beatline_beat_to_line(&self->state->pv->beat_line,
			psy_audio_sequencetrackiterator_seqoffset(&ite));
		num_entry_lines = beatline_beat_to_line(&self->state->pv->beat_line,
			psy_audio_sequencetrackiterator_entry_length(&ite));
		line -= seqline;		
	}	
	/* prepare draw */
	trackerlinennumbers_save_colours(self, g);
	size = psy_ui_component_scroll_size_px(&self->component);
	/* draw lines */	
	count = 0;	
	while (count <= num_clip_lines) {
		char text[64];		
		
		if (line < num_entry_lines) {
			trackerlinennumbers_text(self, line, patidx, text);		
			trackerlinennumbers_set_colours(self, g, trackerstate_column_style(
				self->state, trackerlinennumbers_column_flags(self, line, seqline),
				0));
			trackerlinennumbers_draw_text(self, g, text, cpy, size.width, text);
		}
		cpy += self->state->beat_convert.line_px;
		++line;
		++count;		
		if (!patternviewstate_single_mode(self->state->pv)) {			
			if (line >= num_entry_lines) {
				if (psy_audio_sequencetrackiterator_has_next_entry(&ite)) {
					psy_audio_sequencetrackiterator_inc_entry(&ite);					
					seqline = beatline_beat_to_line(&self->state->pv->beat_line,
						psy_audio_sequencetrackiterator_seqoffset(&ite));
					line = 0;
					cpy = (line + seqline) * self->state->beat_convert.line_px;
					num_entry_lines = beatline_beat_to_line(
						&self->state->pv->beat_line,						
						psy_audio_sequencetrackiterator_entry_length(&ite));					
					patidx = psy_audio_sequencetrackiterator_patidx(&ite);					
				} else {
					break;
				}
			}
		}
	}
	if (!patternviewstate_single_mode(self->state->pv)) {
		psy_audio_sequencetrackiterator_dispose(&ite);
	}	
}

void trackerlinennumbers_text(TrackerLineNumbers* self, int line,
	int pat_idx, char* rv)
{
	if (line < 0) {
		rv[0] = '\0';
	} else if (!patternviewstate_single_mode(self->state->pv) && line == 0) {			
		psy_snprintf(rv, 64, self->format_seqstart, pat_idx, line);
	} else {
		psy_snprintf(rv, 64, self->format, line);
	}
}

void trackerlinennumbers_save_colours(TrackerLineNumbers* self,
	psy_ui_Graphics* g)
{
	self->draw_restore_fg_colour = psy_ui_component_colour(&self->component);
	self->draw_restore_bg_colour = psy_ui_component_background_colour(
		&self->component);
}

void trackerlinennumbers_set_colours(TrackerLineNumbers* self,
	psy_ui_Graphics* g, psy_ui_Style* style)
{
	if (style) {
		if (style->background.colour.mode.transparent) {
			psy_ui_set_background_colour(g, self->draw_restore_bg_colour);
		} else {
			psy_ui_set_background_colour(g, style->background.colour);
		}
		if (style->colour.mode.transparent) {
			psy_ui_set_text_colour(g, self->draw_restore_fg_colour);
		} else {
			psy_ui_set_text_colour(g, style->colour);
		}
	}
}

TrackerColumnFlags trackerlinennumbers_column_flags(TrackerLineNumbers* self,
	intptr_t line, intptr_t seqline)
{
	TrackerColumnFlags rv;	
	uintptr_t line_abs;
	uintptr_t lpb;
	
	lpb = psy_audio_sequencecursor_lpb(&self->state->pv->cursor);	
	line_abs = seqline + line;		
	rv.playbar = psy_audio_player_playing(workspace_player(self->workspace)) &&
		(self->workspace->player.sequencer.hostseqtime.currplaycursor.abs_line ==
		line_abs);
	rv.mid = 0;	
	rv.cursor = (self->state->pv->cursor.abs_line == line_abs);
	// rv.cursor = (keyboardmiscconfig_following_song(&self->workspace->config.misc) ||
	//	!self->workspace->player.sequencer.hostseqtime.currplaying) &&
	//	self->draw_cursor &&
	//	(psy_audio_sequencecursor_line(&self->state->pv->cursor) == line);
	rv.beat = (line % self->state->pv->cursor.lpb) == 0;
	rv.beat4 = (line % (self->state->pv->cursor.lpb * 4)) == 0;
	rv.selection = 0;
	return rv;
}

void trackerlinennumbers_draw_text(TrackerLineNumbers* self, psy_ui_Graphics* g,
	const char* format, double y, double width, const char* text)
{
	uintptr_t numdigits;
	uintptr_t maxdigits;
	uintptr_t startdigit;
	double blankspace;	
	char digit[2];
	psy_ui_RealRectangle r;
	uintptr_t c;
	const psy_ui_TextMetric* tm;
	double flat_size;

	digit[1] = '\0';
	tm = psy_ui_component_textmetric(&self->component);
	numdigits = psy_strlen(text);
	if (numdigits == 0) {
		return;
	}
	flat_size = psy_ui_value_px(&self->state->track_config->flatsize, tm, NULL);
	maxdigits = (uintptr_t)(width / flat_size);
	if (maxdigits == 0) {
		return;
	}	
	startdigit = maxdigits - numdigits;
	if (numdigits < maxdigits) {
		--startdigit;
	}
	for (c = 0; c < maxdigits; ++c) {
		double cpx;

		cpx = c * flat_size;
		if (c >= startdigit && c < startdigit + numdigits) {
			digit[0] = text[c - startdigit];
		} else {
			digit[0] = ' ';
		}
		if (c == maxdigits - 1) {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(cpx, y),
				psy_ui_realsize_make(self->size.width - cpx - 1,
					self->state->beat_convert.line_px - 1));
		} else {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(cpx, y),
				psy_ui_realsize_make(flat_size,
					self->state->beat_convert.line_px - 1));
		}
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
			digit, psy_strlen(digit));
	}
	r.left += flat_size;
	blankspace = (width - r.left) - 4;
	if (blankspace > 0) {
		r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(r.left, y),
			psy_ui_realsize_make(
				blankspace, self->state->beat_convert.line_px - 1));
		digit[0] = ' ';
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
			digit, psy_strlen(digit));
	}
}

void trackerlinenumbers_invalidate_cursor_internal(TrackerLineNumbers* self,
	const psy_audio_SequenceCursor* cursor)
{		
	intptr_t line;
					
	line = beatline_beat_to_line(&self->state->pv->beat_line,
		(patternviewstate_single_mode(self->state->pv))
		? psy_audio_sequencecursor_offset(cursor)
		: psy_audio_sequencecursor_offset_abs(cursor, self->state->pv->sequence));		
	psy_ui_component_invalidate_rect(&self->component,
		psy_ui_realrectangle_make(psy_ui_realpoint_make(
			0.0, self->state->beat_convert.line_px * line),
			psy_ui_realsize_make(self->size.width,
			self->state->beat_convert.line_px)));
}

void trackerlinenumbers_invalidate_cursor(TrackerLineNumbers* self)
{
	trackerlinenumbers_invalidate_cursor_internal(self, &self->old_cursor);
	trackerlinenumbers_invalidate_cursor_internal(self, &self->state->pv->cursor);
	self->old_cursor = self->state->pv->cursor;
}

void trackerlinenumbers_invalidate_playbar(TrackerLineNumbers* self)
{
	double last;
	double curr;
	double minval;
	double maxval;
	psy_ui_RealSize size;

	if (!self->state->pv->sequence) {
		return;
	}	
	last = self->workspace->player.sequencer.hostseqtime.lastplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		last += psy_audio_sequencecursor_seqoffset(
			&self->workspace->player.sequencer.hostseqtime.lastplaycursor,
			self->state->pv->sequence);
	}
	last = trackerstate_beat_to_px(self->state, last);
	curr = self->workspace->player.sequencer.hostseqtime.currplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		curr += psy_audio_sequencecursor_seqoffset(
			&self->workspace->player.sequencer.hostseqtime.currplaycursor,
			self->state->pv->sequence);
	}
	curr = trackerstate_beat_to_px(self->state, curr);		
	minval = psy_min(last, curr);
	maxval = psy_max(last, curr);
	size = psy_ui_component_scroll_size_px(&self->component);
	psy_ui_component_invalidate_rect(&self->component, 
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(				
				0,
				minval),
			psy_ui_realsize_make(
				size.width,
				maxval - minval + self->state->beat_convert.line_px)));		
		
}

void trackerlinenumbers_on_align(TrackerLineNumbers* self)
{
	trackerlinenumbers_update_size(self);
}

void trackerlinenumbers_on_preferred_size(TrackerLineNumbers* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	double width;

	assert(self);
		
	width = 5.0;
	if (self->show_in_hex) {
	}
	if (self->show_beat) {
		width += 5.0;
	}
	if (!patternviewstate_single_mode(self->state->pv)) {
		width += 1.0;	
	}
	rv->width =
		psy_ui_mul_values(
			psy_ui_value_make_ew(width),
			self->state->track_config->flatsize,
			psy_ui_component_textmetric(&self->component),
			NULL);
	rv->height = psy_ui_value_make_px(
		patternviewstate_numlines(self->state->pv) *
		self->state->beat_convert.line_px);
}

void trackerlinenumbers_update_size(TrackerLineNumbers* self)
{	
	self->size = psy_ui_component_scroll_size_px(&self->component);		
}

void trackerlinenumbers_show_cursor(TrackerLineNumbers* self)
{	
	self->draw_cursor = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_hide_cursor(TrackerLineNumbers* self)
{
	self->draw_cursor = FALSE;
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_show_in_hex(TrackerLineNumbers* self)
{
	self->show_in_hex = TRUE;
	trackerlinenumbers_update_format(self);
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_show_in_decimal(TrackerLineNumbers* self)
{
	self->show_in_hex = FALSE;
	trackerlinenumbers_update_format(self);
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_update_cursor(TrackerLineNumbers* self)
{
	if (!keyboardmiscconfig_following_song(&self->workspace->config.misc) ||
			!self->workspace->player.sequencer.hostseqtime.currplaying) {
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self->state->pv);
		if (sequence) {
			// trackerlinenumbers_invalidate_line(self,
			//	psy_audio_sequencecursor_line_abs(&sequence->lastcursor,
			//		sequence));
			//trackerlinenumbers_invalidate_line(self,
			//	psy_audio_sequencecursor_line_abs(&sequence->cursor,
			//		sequence));
			//self->old_cursor = sequence->cursor;
		}
	}
}

void trackerlinenumbers_configure(TrackerLineNumbers* self)
{
	PatternViewConfig* config;

	config = &self->workspace->config.visual.patview;
	if (patternviewconfig_showbeatoffset(config)) {
		trackerlinenumbers_show_beat(self);		
	} else {
		trackerlinenumbers_hide_beat(self);		
	}
	if (patternviewconfig_linenumberscursor(config)) {
		trackerlinenumbers_show_cursor(self);
	} else {
		trackerlinenumbers_hide_cursor(self);
	}
	if (patternviewconfig_linenumbersinhex(config)) {
		trackerlinenumbers_show_in_hex(self);
	} else {
		trackerlinenumbers_show_in_decimal(self);
	}
}

void trackerlinenumbers_on_show_beat_offset(TrackerLineNumbers* self,
	psy_Property* sender)
{
	psy_ui_Component* align_parent;

	if (psy_property_item_bool(sender)) {
		trackerlinenumbers_show_beat(self);
	} else {
		trackerlinenumbers_hide_beat(self);
	}
	align_parent = psy_ui_component_parent(psy_ui_component_parent(
		psy_ui_component_parent(&self->component)));
	if (align_parent) {
		psy_ui_component_align(align_parent);
		psy_ui_component_invalidate(align_parent);
	}
}

void trackerlinenumbers_on_show_in_hex(TrackerLineNumbers* self,
	psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		trackerlinenumbers_show_in_hex(self);
	} else {
		trackerlinenumbers_show_in_decimal(self);
	}	
}

void trackerlinenumbers_on_show_cursor(TrackerLineNumbers* self,
	psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		trackerlinenumbers_show_cursor(self);
	}
	else {
		trackerlinenumbers_hide_cursor(self);
	}
}

/* TrackerLineNumberView */
void trackerlinenumberview_init(TrackerLineNumberView* self, psy_ui_Component* parent,
	TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_init_align(&self->pane, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	trackerlinenumbers_init(&self->linenumbers, &self->pane, state, workspace);
	psy_ui_component_set_align(&self->linenumbers.component, psy_ui_ALIGN_FIXED);	
}

void trackerlinenumberview_set_scroll_top(TrackerLineNumberView* self,
	psy_ui_Value top)
{
	psy_ui_component_set_scroll_top(&self->linenumbers.component, top);
}
