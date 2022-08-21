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
	intptr_t line, double seqoffset);
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
			(psy_ui_fp_component_event)
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
	psy_ui_realsize_init(&self->line_size);	
	self->cursor_line_abs = psy_INDEX_INVALID;
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
	static const char* format_hex_beat_seqstart = "%.2X %.2X %.3f";
	static const char* format_hex_beat = "%X %.3f";
	static const char* format_hex_seqstart = "%.2X %.2X";
	static const char* format_hex = "%X";

	static const char* format_beat_seqstart = "%X %i %.3f";
	static const char* format_beat = "%i %.3f";
	static const char* format_seqstart = "%X %i";
	static const char* format = "%i";

	if (self->show_in_hex) {
		if (self->show_beat) {
			self->format = format_hex_beat;
			self->format_seqstart = format_hex_beat_seqstart;
		} else {
			self->format = format_hex;
			self->format_seqstart = format_hex_seqstart;			
		}
	} else {
		if (self->show_beat) {
			self->format = format_beat;
			self->format_seqstart = format_beat_seqstart;
		} else {
			self->format = format;
			self->format_seqstart = format_seqstart;
		}
	}
}

void trackerlinenumbers_on_draw(TrackerLineNumbers* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;
	psy_ui_RealRectangle g_clip;
	double cpy;
	uintptr_t line_start;
	uintptr_t line_end;
	uintptr_t line;
	intptr_t seqline;
	double offset;			
	double seqoffset;	
	double clip_offset_top;
	double clip_offset_bottom;
	uintptr_t maxlines;
	uintptr_t patidx;	
	psy_audio_SequenceTrackIterator ite;		
	uintptr_t lpb;	
	
	assert(self);
			
	if (!patternviewstate_sequence(self->state->pv)) {
		return;
	}	
	/* prepare clipping */
	g_clip = psy_ui_graphics_cliprect(g);	
	/*
	** single mode: offsets 0.0 at seq_entry begin (relative to seq_entry)
	** all        : offsets 0.0 at sequence begin (absolute to sequence)
	**/
	clip_offset_top = trackerstate_px_to_beat(self->state,
		psy_max(0.0, g_clip.top), self->line_size.height);
	clip_offset_bottom = trackerstate_px_to_beat(self->state,
		psy_max(0.0, g_clip.bottom), self->line_size.height);		
	clip_offset_bottom = psy_min(clip_offset_bottom,
		patternviewstate_length(self->state->pv));
	lpb = self->state->pv->cursor.lpb;	
	line_start = floor(clip_offset_top * lpb);
	line_end = floor(clip_offset_bottom * lpb);
	cpy = line_start * self->line_size.height;	
	if (!patternviewstate_single_mode(self->state->pv)) {
		psy_audio_sequencetrackiterator_init(&ite);
		psy_audio_sequence_begin(self->state->pv->sequence,			
			self->state->pv->cursor.order_index.track, clip_offset_top, &ite);				
		patidx = psy_audio_sequencetrackiterator_patidx(&ite);		
		seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);		
		seqline = patternviewstate_beat_to_line(self->state->pv, seqoffset);		
	} else {
		psy_audio_SequenceEntry* seq_entry;
		
		seq_entry = psy_audio_sequence_entry(self->state->pv->sequence,
			psy_audio_sequencecursor_order_index(&self->state->pv->cursor));
		if (seq_entry) {
			seqoffset = psy_audio_sequenceentry_offset(seq_entry);
		} else {
			seqoffset = 0.0;
		}
	}
	offset = clip_offset_top;	
	self->cursor_line_abs = psy_audio_sequencecursor_line_abs(
		&self->state->pv->cursor, self->state->pv->sequence);
	self->play_line_abs = psy_audio_sequencecursor_line_abs(
		&self->workspace->player.sequencer.hostseqtime.currplaycursor,
		self->state->pv->sequence);
	/* prepare draw */
	self->draw_restore_fg_colour = psy_ui_component_colour(&self->component);
	self->draw_restore_bg_colour = psy_ui_component_background_colour(
		&self->component);
	size = psy_ui_component_scroll_size_px(&self->component);
	/* draw lines */
	for (line = line_start; line <= line_end; ++line) {
		char text[64];
		psy_ui_Style* style;
		
		if (!patternviewstate_single_mode(self->state->pv)) {
			if (seqline > line) {
				text[0] = '\0';
			} else if (seqline == line) {
				psy_snprintf(text, 64, self->format_seqstart, (int)patidx,
					(int)line - seqline, (float)offset);
			} else {
				psy_snprintf(text, 64, self->format, (int)(line - seqline),
					(float)offset);
			}
			style = trackerstate_column_style(self->state,
			trackerlinennumbers_column_flags(self, line - seqline, seqoffset),
			0);
		} else {
			psy_snprintf(text, 64, self->format, (int)(line), (float)offset);
			style = trackerstate_column_style(self->state,
			trackerlinennumbers_column_flags(self, line, seqoffset), 0);
		}		
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
		trackerlinennumbers_draw_text(self, g, text, cpy, size.width, text);
		cpy += self->line_size.height;
		offset += psy_audio_sequencecursor_bpl(&self->state->pv->cursor);
		if (!patternviewstate_single_mode(self->state->pv)) {
			if (offset >= seqoffset + psy_audio_sequencetrackiterator_entry_length(&ite)) {
				if (psy_audio_sequencetrackiterator_has_next_entry(&ite)) {
					psy_audio_sequencetrackiterator_inc_entry(&ite);					
					seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
					seqline = patternviewstate_beat_to_line(self->state->pv,
						seqoffset);
					patidx = psy_audio_sequencetrackiterator_patidx(&ite);
					offset = seqoffset;								
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

TrackerColumnFlags trackerlinennumbers_column_flags(TrackerLineNumbers* self,
	intptr_t line, double seqoffset)
{
	TrackerColumnFlags rv;	
	uintptr_t line_abs;
	uintptr_t lpb;
	
	lpb = psy_audio_sequencecursor_lpb(&self->state->pv->cursor);	
	line_abs = (uintptr_t)(seqoffset * (double)lpb) + line;		
	rv.playbar = psy_audio_player_playing(workspace_player(self->workspace)) &&
		(self->play_line_abs == line_abs);
	rv.mid = 0;	
	rv.cursor = (self->cursor_line_abs == line_abs);
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
					self->line_size.height - 1));
		} else {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(cpx, y),
				psy_ui_realsize_make(flat_size,
					self->line_size.height - 1));
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
				blankspace, self->line_size.height - 1));
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
			
	line = patternviewstate_beat_to_line(self->state->pv,
		(patternviewstate_single_mode(self->state->pv))
		? psy_audio_sequencecursor_offset(cursor)
		: psy_audio_sequencecursor_offset_abs(cursor, self->state->pv->sequence));		
	psy_ui_component_invalidate_rect(&self->component,
		psy_ui_realrectangle_make(psy_ui_realpoint_make(
			0.0, self->line_size.height * line),
			self->line_size));
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
	last = trackerstate_beat_to_px(self->state, last, self->line_size.height);
	curr = self->workspace->player.sequencer.hostseqtime.currplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		curr += psy_audio_sequencecursor_seqoffset(
			&self->workspace->player.sequencer.hostseqtime.currplaycursor,
			self->state->pv->sequence);
	}
	curr = trackerstate_beat_to_px(self->state, curr, self->line_size.height);		
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
				maxval - minval + self->line_size.height)));		
		
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
		self->line_size.height);
}

void trackerlinenumbers_update_size(TrackerLineNumbers* self)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->size = psy_ui_component_scroll_size_px(&self->component);	
	self->line_size = psy_ui_realsize_make(self->size.width,
		psy_ui_value_px(&self->state->line_height, tm, NULL));	
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
