/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerlinenumbers.h"
/* local */
#include "patternview.h"
/* platform */
#include "../../detail/portable.h"

/* TrackerLineNumbers */

/* prototypes */
static void trackerlinenumbers_on_draw(TrackerLineNumbers*, psy_ui_Graphics*);
static TrackerColumnFlags trackerlinennumbers_column_flags(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset, intptr_t line, intptr_t seqline);
static void trackerlinennumbers_draw_text(TrackerLineNumbers*, psy_ui_Graphics*,
	const char* format, double y, double width, const char* text);
static void trackerlinenumbers_invalidate_cursor_internal(TrackerLineNumbers*,
	const psy_audio_SequenceCursor*);
static void trackerlinenumbers_on_align(TrackerLineNumbers*);
static void trackerlinenumbers_on_preferred_size(TrackerLineNumbers*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void trackerlinenumbers_update_size(TrackerLineNumbers*);
static void trackerlinenumbers_on_configure(TrackerLineNumbers*,
	PatternViewConfig*, psy_Property*);

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
			(psy_ui_fp_component_onpreferredsize)
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
	psy_ui_realsize_init(&self->size);
	psy_ui_realsize_init(&self->line_size);	
	psy_audio_sequencecursor_init(&self->old_cursor);
	trackerlinenumbers_update_format(self);		
	psy_ui_component_set_scroll_step_height(&self->component,
		state->line_height);
	self->component.blitscroll = TRUE;
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		trackerlinenumbers_on_configure);
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
	if (patternviewstate_sequence(self->state->pv)) {
		psy_ui_RealSize size;
		psy_ui_RealRectangle g_clip;
		double cpy;
		intptr_t line;
		intptr_t seqline;
		double offset;
		double maxabsoffset;				
		double seqoffset;
		double length;		
		uintptr_t maxlines;
		uintptr_t patidx;
		psy_audio_BlockSelection clip;
		psy_audio_SequenceTrackIterator ite;		
		
		assert(self);
				
		size = psy_ui_component_scroll_size_px(&self->component);
		g_clip = psy_ui_graphics_cliprect(g);
		trackerstate_line_clip(self->state, &g_clip, &clip, self->line_size.height);
		offset = clip.topleft.absoffset;
		psy_audio_sequencetrackiterator_init(&ite);
		psy_audio_sequence_begin(self->state->pv->sequence,
			psy_audio_sequence_track_at(self->state->pv->sequence,
				self->state->pv->cursor.orderindex.track),
			offset, &ite);		
		maxabsoffset = 0.0;
		if (ite.pattern) {
			if (patternviewstate_single_mode(self->state->pv)) {
				maxabsoffset = self->state->pv->cursor.absoffset +
					ite.pattern->length;
			} else {
				maxabsoffset = patternviewstate_length(self->state->pv);
			}
		} else {
			maxabsoffset = 0.0;
		}
		patidx = psy_audio_sequencetrackiterator_patidx(&ite);
		seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
		line = patternviewstate_beat_to_line(self->state->pv,
			clip.topleft.absoffset);
		seqline = patternviewstate_beat_to_line(self->state->pv, seqoffset);
		maxlines = patternviewstate_beat_to_line(self->state->pv, maxabsoffset);
		cpy = trackerstate_beat_to_px(self->state, clip.topleft.absoffset,
			self->line_size.height);
		if (patternviewstate_single_mode(self->state->pv)) {
			cpy -= trackerstate_beat_to_px(self->state, clip.topleft.seqoffset,
				self->line_size.height);
		}
		while (offset <= clip.bottomright.absoffset && line < (intptr_t)maxlines) {
			double ystart;			
			char text[64];
			psy_ui_Colour bg;
			psy_ui_Colour fore;
				
			if (!patternviewstate_single_mode(self->state->pv) && seqoffset == offset) {
				psy_snprintf(text, 64, self->format_seqstart, (int)patidx,
					(int)line, (float)offset);
			} else {
				psy_snprintf(text, 64, self->format, (int)(line - 
					((patternviewstate_single_mode(self->state->pv)) ? seqline : 0)),
					(float)offset);
			}			
			trackerstate_columncolours(self->state,
				trackerlinennumbers_column_flags(self, offset, line,
					seqline), 0, &bg, &fore);
			psy_ui_setbackgroundcolour(g, bg);
			psy_ui_settextcolour(g, fore);			
			trackerlinennumbers_draw_text(self, g, text, cpy, size.width, text);
			cpy += self->line_size.height;
			ystart = cpy;
			offset += patternviewstate_bpl(self->state->pv);
			if (ite.pattern && offset >= seqoffset + ite.pattern->length) {
				if (ite.sequencentrynode && ite.sequencentrynode->next) {
					psy_audio_sequencetrackiterator_inc_entry(&ite);
					if (!ite.pattern) {
						break;
					}
					seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
					seqline = patternviewstate_beat_to_line(self->state->pv, seqoffset);
					patidx = psy_audio_sequencetrackiterator_patidx(&ite);
					offset = seqoffset;					
					length = ite.pattern->length;			
				} else {
					break;
				}
			}
			++line;
		}
		psy_audio_sequencetrackiterator_dispose(&ite);
	}
}

TrackerColumnFlags trackerlinennumbers_column_flags(TrackerLineNumbers* self,
	psy_dsp_big_beat_t offset, intptr_t line, intptr_t seqline)
{
	TrackerColumnFlags rv;	
	rv.playbar = psy_audio_player_playing(workspace_player(self->workspace)) &&
		psy_dsp_testrange(self->workspace->host_sequencer_time.currplaycursor.absoffset,
			offset, patternviewstate_bpl(self->state->pv));
	rv.mid = 0;	
	rv.cursor = (!self->workspace->follow_song || !self->workspace->host_sequencer_time.currplaying) &&
		self->draw_cursor &&
		(psy_audio_sequencecursor_line(&self->state->pv->cursor) == line);
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
		? psy_audio_sequencecursor_pattern_offset(cursor)
		: psy_audio_sequencecursor_offset_abs(cursor));		
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

void trackerlinenumbers_invalidate_line(TrackerLineNumbers* self, intptr_t line)
{		
	psy_ui_component_invalidate_rect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, self->line_size.height *
				(line - patternviewstate_seqstartline(self->state->pv))),
			self->line_size));
}

void trackerlinenumbers_invalidate_playbar(TrackerLineNumbers* self)
{
	trackerlinenumbers_invalidate_line(self,
		self->workspace->host_sequencer_time.lastplaycursor.linecache);
	trackerlinenumbers_invalidate_line(self,
		self->workspace->host_sequencer_time.currplaycursor.linecache);
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
		width += 3.0;	
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
	if (!self->workspace->follow_song || !self->workspace->host_sequencer_time.currplaying) {
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self->state->pv);
		if (sequence) {
			trackerlinenumbers_invalidate_line(self,
				psy_audio_sequencecursor_line_abs(&sequence->lastcursor));
			trackerlinenumbers_invalidate_line(self,
				psy_audio_sequencecursor_line_abs(&sequence->cursor));
			self->old_cursor = sequence->cursor;
		}
	}
}

void trackerlinenumbers_on_configure(TrackerLineNumbers* self,
	PatternViewConfig* config, psy_Property* property)
{
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
