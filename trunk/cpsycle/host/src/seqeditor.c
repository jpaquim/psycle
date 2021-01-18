// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "seqeditor.h"
// host
#include "cmdsgeneral.h"
#include "sequencetrackbox.h"
#include "pianoroll.h"
// audio
#include <exclusivelock.h>
#include <patterns.h>
#include <songio.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define DEFAULT_PXPERBEAT 5.0

void seqeditortrackstate_init(SeqEditorTrackState* self)
{
	psy_signal_init(&self->signal_cursorchanged);
	self->pxperbeat = DEFAULT_PXPERBEAT;
	self->defaultlineheight = psy_ui_value_makeeh(1.5);
	self->trackssize = psy_ui_size_makeem(80.0, 1.5);
	self->lineheight = self->defaultlineheight;
	self->linemargin = psy_ui_value_makeeh(0.2);	
	self->cursorposition = (psy_dsp_big_beat_t)0.0;
	self->drawcursor = FALSE;
}

void seqeditortrackstate_dispose(SeqEditorTrackState* self)
{
	psy_signal_dispose(&self->signal_cursorchanged);
}

// SeqEditorRuler
static void seqeditorruler_ondraw(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_drawruler(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler*, psy_audio_SequenceSelection* sender);
static void seqeditorruler_onpreferredsize(SeqEditorRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static psy_dsp_big_beat_t seqeditorruler_step(SeqEditorRuler*);
static void seqeditorruler_oncursorchanged(SeqEditorRuler*, SeqEditorTrackState*);
// vtable
static psy_ui_ComponentVtable seqeditorruler_vtable;
static bool seqeditorruler_vtable_initialized = FALSE;

static void seqeditorruler_vtable_init(SeqEditorRuler* self)
{
	if (!seqeditorruler_vtable_initialized) {
		seqeditorruler_vtable = *(self->component.vtable);
		seqeditorruler_vtable.ondraw = (psy_ui_fp_component_ondraw)
			seqeditorruler_ondraw;
		seqeditorruler_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			seqeditorruler_onpreferredsize;
		seqeditorruler_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditorruler_init(SeqEditorRuler* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, PatternViewSkin* skin,
	Workspace* workspace)
{
	assert(self);
	assert(trackstate);
	assert(workspace);
	assert(skin);

	psy_ui_component_init(&self->component, parent);
	seqeditorruler_vtable_init(self);
	self->component.vtable = &seqeditorruler_vtable;
	psy_ui_component_doublebuffer(&self->component);	
	self->trackstate = trackstate;
	self->skin = skin;
	self->workspace = workspace;	
	psy_signal_connect(&workspace->sequenceselection.signal_changed, self,
		seqeditorruler_onsequenceselectionchanged);	
	psy_signal_connect(&self->trackstate->signal_cursorchanged, self,
		seqeditorruler_oncursorchanged);
}

void seqeditorruler_ondraw(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	seqeditorruler_drawruler(self, g);
}

void seqeditorruler_drawruler(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	double baseline;
	double linewidth;
	psy_dsp_big_beat_t duration;
	psy_dsp_big_beat_t clipstart;
	psy_dsp_big_beat_t clipend;
	psy_dsp_big_beat_t currbeat;
	psy_dsp_big_beat_t step;	
	
	size = psy_ui_component_sizepx(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = size.height - 1;
	duration = (size.width + psy_ui_component_scrollleftpx(&self->component)) /
		(psy_dsp_big_beat_t)self->trackstate->pxperbeat;
	//psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence);
	linewidth = duration * self->trackstate->pxperbeat;
	psy_ui_setcolour(g, self->skin->row);
	psy_ui_drawline(g, 0, baseline, linewidth, baseline);
	clipstart = 0;
	clipend = duration;
	step = seqeditorruler_step(self);
	for (currbeat = clipstart; currbeat <= clipend; currbeat += step) {
		double cpx;
		char txt[40];

		cpx = currbeat * self->trackstate->pxperbeat;
		if (((intptr_t)(currbeat / step) % 8) == 0) {
			psy_ui_settextcolour(g, self->skin->row4beat);
			psy_ui_setcolour(g, self->skin->row4beat);
		} else if (((intptr_t)(currbeat /step) % 4) == 0) {
			psy_ui_settextcolour(g, self->skin->rowbeat);
			psy_ui_setcolour(g, self->skin->rowbeat);
		} else {
			psy_ui_settextcolour(g, self->skin->row);
			psy_ui_setcolour(g, self->skin->row);
		}
		psy_ui_drawline(g, cpx, baseline, cpx, baseline - tm->tmHeight / 3);
		if (self->trackstate->drawcursor) {
			psy_dsp_big_beat_t cursor;

			cursor = floor(self->trackstate->cursorposition / step) * step;
			if (currbeat >= cursor && currbeat <= cursor + step) {
				double start;
				double end;

				if (cursor == currbeat &&
						self->trackstate->cursorposition - cursor < step / 2) {
					psy_ui_settextcolour(g, self->skin->row4beat);
				} else if (cursor + step == currbeat &&
					self->trackstate->cursorposition - cursor > step / 2) {
					psy_ui_settextcolour(g, self->skin->row4beat);
				}
				psy_ui_setcolour(g, self->skin->row4beat);
				start = cursor * self->trackstate->pxperbeat;
				end = (cursor + step) * self->trackstate->pxperbeat;
				psy_ui_drawline(g, start, baseline, end, baseline);
			}
		}		
		psy_snprintf(txt, 40, "%d", (int)(currbeat));
		psy_ui_textout(g, cpx + 3, baseline - tm->tmHeight, txt, strlen(txt));
	}
	if (self->trackstate->drawcursor) {
		double cpx;
		
		psy_ui_setcolour(g, self->skin->cursor);
		cpx = self->trackstate->cursorposition * self->trackstate->pxperbeat;
		psy_ui_drawline(g, cpx, baseline, cpx, baseline - tm->tmHeight / 3);
	}
}

psy_dsp_big_beat_t seqeditorruler_step(SeqEditorRuler* self)
{
	psy_dsp_big_beat_t rv;
	
	rv = (psy_dsp_big_beat_t)(80.0 / self->trackstate->pxperbeat);
	if (rv > 16.0) {
		rv = (psy_dsp_big_beat_t)(floor((rv / 16) * 16.0));
	} else if (rv > 4.0) {
		rv = (psy_dsp_big_beat_t)(floor((rv / 4.0) * 4.0));
	} else if (rv > 2.0) {
		rv = (psy_dsp_big_beat_t)(floor((rv / 2.0) * 2.0));
	} else {
		rv = (psy_dsp_big_beat_t)1.0;
	}
	return rv;
}

void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler* self,
	psy_audio_SequenceSelection* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditorruler_oncursorchanged(SeqEditorRuler* self,
	SeqEditorTrackState* trackstate)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditorruler_onpreferredsize(SeqEditorRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (workspace_song(self->workspace)) {			
		double linewidth;
		psy_dsp_big_beat_t duration;
		
		duration = psy_audio_sequence_duration(
			&workspace_song(self->workspace)->sequence);
		linewidth = duration * self->trackstate->pxperbeat;
		rv->width = psy_ui_value_makepx(linewidth);
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
	rv->height = psy_ui_value_makeeh(1.0);
}

// SeqEditorTrack
// prototypes
static void seqeditortrack_ondraw_virtual(SeqEditorTrack* self,
	psy_ui_Graphics* g, double x, double y);
static void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static bool seqeditortrack_onmousedown_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static bool seqeditortrack_onmousemove_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static bool seqeditortrack_onmouseup_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static void seqeditortracks_invalidatebitmap(SeqEditorTracks*);
static void seqeditortrack_outputstatusposition(SeqEditorTrack*, double x);
// vtable
static SeqEditorTrackVTable seqeditortrack_vtable;
static bool seqeditortrack_vtable_initialized = FALSE;

static void seqeditortrack_vtable_init(void)
{
	if (!seqeditortrack_vtable_initialized) {
		seqeditortrack_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrack_ondraw_virtual;
		seqeditortrack_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrack_onpreferredsize_virtual;
		seqeditortrack_vtable.onmousedown = (seqeditortrack_fp_onmousedown)
			seqeditortrack_onmousedown_virtual;
		seqeditortrack_vtable.onmousemove = (seqeditortrack_fp_onmousemove)
			seqeditortrack_onmousemove_virtual;
		seqeditortrack_vtable.onmouseup = (seqeditortrack_fp_onmouseup)
			seqeditortrack_onmouseup_virtual;
		seqeditortrack_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortrack_init(SeqEditorTrack* self, SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_vtable_init();
	self->vtable = &seqeditortrack_vtable;
	self->trackstate = trackstate;
	self->workspace = workspace;
	self->parent = parent;
	self->currtrack = NULL;
	self->trackindex = 0;
	self->drag_sequenceitem_node = NULL;
	self->dragstarting = FALSE;
	self->bitmapvalid = FALSE;	
	psy_ui_bitmap_init(&self->bitmap);
}

void seqeditortrack_dispose(SeqEditorTrack* self)
{
	psy_ui_bitmap_dispose(&self->bitmap);
}

SeqEditorTrack* seqeditortrack_alloc(void)
{
	return (SeqEditorTrack*)malloc(sizeof(SeqEditorTrack));
}

SeqEditorTrack* seqeditortrack_allocinit(SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	SeqEditorTrack* rv;

	rv = seqeditortrack_alloc();
	if (rv) {
		seqeditortrack_init(rv, parent, trackstate, workspace);
	}
	return rv;
}

void seqeditortrack_updatetrack(SeqEditorTrack* self, 
	psy_audio_SequenceTrackNode* tracknode,
	psy_audio_SequenceTrack* track,
	uintptr_t trackindex)
{
	self->currtrack = track;
	self->currtracknode = tracknode;
	self->trackindex = trackindex;
}

void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g,
	double x, double y)
{
	psy_List* p;
	intptr_t c;	
	const psy_ui_TextMetric* tm;
	double lineheight;
	psy_ui_Rectangle bg;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {		
		return;
	}	
	tm = psy_ui_component_textmetric(&self->parent->component);
	lineheight = floor(psy_ui_value_px(&self->trackstate->lineheight, tm));
	bg = psy_ui_rectangle_make(0, y,		
		psy_ui_component_sizepx(&self->parent->component).width, lineheight);
	psy_ui_drawsolidrectangle(g, bg,
		psy_ui_component_backgroundcolour(&self->parent->component));	
	if (!self->bitmapvalid && self->currtrack->entries) {
		psy_ui_Graphics gr;			
		psy_ui_RealSize size;		
		psy_ui_Size preferredsize;		

		psy_ui_bitmap_dispose(&self->bitmap);
		seqeditortrack_onpreferredsize_virtual(self, NULL, &preferredsize);
		bg = psy_ui_rectangle_make(0, 0,
			psy_ui_value_px(&preferredsize.width, tm),
			lineheight);
		size = psy_ui_realsize_make(bg.right, lineheight);
		psy_ui_bitmap_init_size(&self->bitmap, size);
		psy_ui_graphics_init_bitmap(&gr, &self->bitmap);
		psy_ui_setfont(&gr, psy_ui_component_font(&self->parent->component));
		psy_ui_setbackgroundmode(&gr, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(&gr, psy_ui_component_colour(&self->parent->component));				
		psy_ui_drawsolidrectangle(&gr, bg,
			psy_ui_component_backgroundcolour(&self->parent->component));
		for (p = self->currtrack->entries, c = 0; p != NULL;
			psy_list_next(&p), ++c) {
			psy_audio_SequenceEntry* sequenceentry;
			psy_audio_Pattern* pattern;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
				psy_audio_sequenceentry_patternslot(sequenceentry));
			if (pattern) {				
				double patternwidth;
				bool selected;
				char text[256];				
				psy_ui_Rectangle r;
				double centery;
				
				centery = (size.height - tm->tmHeight) / 2;
				patternwidth = (psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);				
				r = psy_ui_rectangle_make(sequenceentry->offset * self->trackstate->pxperbeat,
					0, patternwidth, lineheight);
				selected =
					self->workspace->sequenceselection.editposition.track == self->trackindex &&
					self->workspace->sequenceselection.editposition.order == c;
				if (selected) {
					psy_ui_drawsolidrectangle(&gr, r, psy_ui_colour_make(0x00514536));
					psy_ui_setcolour(&gr, psy_ui_colour_make(0x00555555));
					psy_ui_drawrectangle(&gr, r);
				} else {
					psy_ui_drawsolidrectangle(&gr, r, psy_ui_colour_make(0x00333333));
					psy_ui_setcolour(&gr, psy_ui_colour_make(0x00444444));
					psy_ui_drawrectangle(&gr, r);
				}
				r = psy_ui_rectangle_make(sequenceentry->offset * self->trackstate->pxperbeat,
					centery, patternwidth, tm->tmHeight);
				if (generalconfig_showingpatternnames(psycleconfig_general(
					workspace_conf(self->workspace)))) {
					psy_audio_Pattern* pattern;

					pattern = psy_audio_sequenceentry_pattern(sequenceentry,
						&workspace_song(self->workspace)->patterns);
					if (pattern) {
						psy_snprintf(text, 20, "%02X: %s", c,
							psy_audio_pattern_name(pattern));
					} else {
						psy_snprintf(text, 20, "%02X:%02X(ERR)", c,
							(int)psy_audio_sequenceentry_patternslot(sequenceentry));
					}
					psy_ui_textoutrectangle(&gr, r.left + 2, r.top, psy_ui_ETO_CLIPPED, r,
						text, strlen(text));
				} else {
					psy_snprintf(text, 256, "%02X",
						(int)psy_audio_sequenceentry_patternslot(sequenceentry));
					psy_ui_textoutrectangle(&gr, r.left + 2, r.top, psy_ui_ETO_CLIPPED, r,
						text, strlen(text));
				}
				if (self->parent->drawpatternevents) {
					PianoGridDraw griddraw;
					PianoGridState gridstate;
					KeyboardState keyboardstate;
					psy_audio_PatternSelection selection;
					double lh;							
					
					keyboardstate_init(&keyboardstate, self->parent->skin);
					lh = psy_ui_value_px(&self->trackstate->lineheight, tm);
					lh = lh / tm->tmHeight;
					keyboardstate.defaultkeyheight = psy_ui_value_makeeh(
						 lh / keyboardstate_numkeys(&keyboardstate));
					keyboardstate.keyheight = keyboardstate.defaultkeyheight;
					pianogridstate_init(&gridstate, self->parent->skin);
					pianogridstate_setpattern(&gridstate, pattern);					
					pianogridstate_setlpb(&gridstate, psy_audio_player_lpb(
						workspace_player(self->workspace)));
					gridstate.pxperbeat = self->trackstate->pxperbeat;
					psy_audio_patternselection_init(&selection);					
					pianogriddraw_init(&griddraw,						
						&keyboardstate, &gridstate,
						psy_ui_value_zero(), psy_ui_value_zero(),
						0.0,
						NULL,
						PIANOROLL_TRACK_DISPLAY_ALL,
						FALSE, FALSE,
						selection,							
						size, tm, self->workspace);
					pianogriddraw_preventclip(&griddraw);
					pianogriddraw_preventgrid(&griddraw);
					pianogriddraw_preventcursor(&griddraw);
					pianogriddraw_preventplaybar(&griddraw);					
					psy_ui_setorigin(&gr, -r.left, 0);
					pianogriddraw_ondraw(&griddraw, &gr);
					psy_ui_setorigin(&gr, 0, 0);					
				}
			}			
		}	
		psy_ui_graphics_dispose(&gr);
		self->bitmapvalid = TRUE;
	}
	if (self->bitmapvalid) {
		psy_ui_Size bitmapsize;

		bitmapsize = psy_ui_bitmap_size(&self->bitmap);
		psy_ui_drawbitmap(g, &self->bitmap, 0, y,
			psy_ui_value_px(&bitmapsize.width, tm),
			psy_ui_value_px(&bitmapsize.height, tm),
			0, 0);
	}
	if (self->drag_sequenceitem_node && !self->dragstarting) {
		psy_ui_Rectangle r;
		const psy_ui_TextMetric* tm;
		psy_ui_IntSize size;
		psy_audio_SequenceEntry* sequenceentry;
		double cpx;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(
			self->drag_sequenceitem_node);
		tm = psy_ui_component_textmetric(&self->parent->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->parent->component), tm);
		cpx = self->itemdragposition * self->trackstate->pxperbeat;
		r = psy_ui_rectangle_make(cpx, y, 2, lineheight);
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00CACACA));		
	}
}

void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&workspace_song(self->workspace)->patterns);
	}
	rv->width = psy_ui_value_makepx(self->trackstate->pxperbeat *
		trackduration);
	rv->height = self->trackstate->lineheight;
}

bool seqeditortrack_onmousedown_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	psy_List* sequenceitem_node;
	uintptr_t selected;
	
	psy_ui_component_capture(&self->parent->component);
	self->drag_sequenceitem_node = NULL;
	self->dragstartpx = ev->x;
	for (sequenceitem_node = self->currtrack->entries, selected = 0;
		sequenceitem_node != NULL;
		psy_list_next(&sequenceitem_node), ++selected) {
		psy_audio_SequenceEntry* entry;
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*)psy_list_entry(sequenceitem_node);
		pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
			entry->patternslot);
		if (pattern) {
			psy_ui_Rectangle r;
			double patternwidth;
			const psy_ui_TextMetric* tm;
			double lineheight;

			tm = psy_ui_component_textmetric(&self->parent->component);
			lineheight = psy_ui_value_px(&self->trackstate->lineheight, tm);
			patternwidth = psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat;
			r = psy_ui_rectangle_make(entry->offset * self->trackstate->pxperbeat, 0,
				patternwidth, lineheight);
			if (psy_ui_rectangle_intersect(&r, ev->x, ev->y)) {
				if (self->currtrack) {
					uintptr_t trackindex;

					trackindex = psy_list_entry_index(workspace_song(self->workspace)->sequence.tracks,
						self->currtrack);
					if (trackindex != psy_INDEX_INVALID) {
						psy_audio_OrderIndex seqeditpos;
						psy_audio_PatternCursor cursor;
						psy_dsp_big_beat_t position;
						
						seqeditpos = workspace_sequenceeditposition(self->workspace);
						if (trackindex != seqeditpos.track || selected != seqeditpos.order) {
							workspace_setsequenceeditposition(
								self->workspace,
								psy_audio_orderindex_make(trackindex, selected));
						}						
						cursor = self->workspace->patterneditposition;
						position = seqeditortrackstate_pxtobeat(self->trackstate, ev->x);
						position =
							(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->workspace))) /
							(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));							
						cursor.offset = position - entry->offset;
						workspace_setpatterncursor(self->workspace, cursor);
						cursor = self->workspace->patterneditposition;
						workspace_gotocursor(self->workspace, cursor);						
					}
				}
				self->drag_sequenceitem_node = sequenceitem_node;
				self->parent->capture = self;
				self->itemdragposition = entry->offset;
				self->dragstarting = TRUE;
				seqeditortrack_outputstatusposition(self, ev->x);				
			}
		}		
	}
	if (ev->button == 2) {
		self->drag_sequenceitem_node = NULL;
	}
	return TRUE;
}

bool seqeditortrack_onmousemove_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{		
	if (self->drag_sequenceitem_node) {
		psy_audio_SequenceEntry* sequenceentry;	
		psy_dsp_big_beat_t dragposition;

		sequenceentry = (psy_audio_SequenceEntry*)self->drag_sequenceitem_node->entry;
		dragposition = seqeditortrackstate_pxtobeat(self->trackstate, ev->x);
		if (self->dragstarting && ((abs((int)(self->dragstartpx - ev->x)) < 2) ||
			dragposition - (sequenceentry->offset - sequenceentry->repositionoffset) == 0.0)) {
			// just select the pattern
			return TRUE;
		}
		if (self->parent->dragmode == SEQEDITORDRAG_MOVE) {
			if (dragposition - (sequenceentry->offset - sequenceentry->repositionoffset) >= 0) {
				self->itemdragposition = dragposition;
				self->itemdragposition =
					(intptr_t)(dragposition * psy_audio_player_lpb(workspace_player(self->workspace))) /
					(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));
			} else {
				self->itemdragposition = sequenceentry->offset - sequenceentry->repositionoffset;
			}
		} else {
			self->itemdragposition = dragposition;
			if (self->itemdragposition < 0.0) {
				self->itemdragposition = 0.0;
			}
			self->itemdragposition =
				(intptr_t)(dragposition * psy_audio_player_lpb(workspace_player(self->workspace))) /
				(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));
		}
		self->dragstarting = FALSE;
		psy_ui_component_invalidate(&self->parent->component);
	}
	seqeditortrack_outputstatusposition(self, ev->x);	
	return TRUE;
}

bool seqeditortrack_onmouseup_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	bool rv;

	rv = TRUE;
	psy_ui_component_releasecapture(&self->parent->component);
	self->parent->capture = NULL;
	if (ev->button == 1) {
		self->parent->capture = NULL;
		if (self->drag_sequenceitem_node && !self->dragstarting) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)
				self->drag_sequenceitem_node->entry;
			if (self->parent->dragmode == SEQEDITORDRAG_MOVE) {
				sequenceentry->repositionoffset = self->itemdragposition -
					(sequenceentry->offset - sequenceentry->repositionoffset);
				sequenceentry->offset = self->itemdragposition;
				psy_audio_sequence_reposition_track(
					&workspace_song(self->workspace)->sequence, self->currtrack);			
			} else {
				psy_audio_sequence_reorder(
					&self->parent->workspace->song->sequence,
					psy_audio_orderindex_make(
						self->trackindex,
						sequenceentry->row),
					self->itemdragposition);				
			}			
		}
		self->drag_sequenceitem_node = NULL;
		self->dragstarting = FALSE;
	} else if (ev->button == 2) {		
		psy_audio_player_sendcmd(workspace_player(self->workspace),
			"general", psy_eventdrivercmd_makeid(CMD_IMM_INFOPATTERN));	
	}	
	return rv;
}

void seqeditortrack_outputstatusposition(SeqEditorTrack* self, double x)
{
	psy_dsp_big_beat_t position;
	char text[256];

	position = seqeditortrackstate_pxtobeat(self->trackstate, x);
	// quantize to lpb raster
	seqeditortrackstate_setcursor(self->trackstate,	
		position * psy_audio_player_lpb(workspace_player(self->workspace)) /
		(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace)));
	psy_snprintf(text, 256, "Sequence Position %.3fb",
		(float)self->trackstate->cursorposition);
	workspace_outputstatus(self->workspace, text);
}


// SeqEditorTrackHeader
// prototypes
static void seqeditortrackheader_ondraw(SeqEditorTrackHeader* self, psy_ui_Graphics* g, double x, double y);
static void seqeditortrackheader_onpreferredsize(SeqEditorTrackHeader*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static bool seqeditortrackheader_onmousedown(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
static bool seqeditortrackheader_onmousemove(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
static bool seqeditortrackheader_onmouseup(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
// vtable
static SeqEditorTrackVTable seqeditortrackheader_vtable;
static bool seqeditortrackheader_vtable_initialized = FALSE;

static void seqeditortrackheader_vtable_init(SeqEditorTrackHeader* self)
{
	if (!seqeditortrackheader_vtable_initialized) {
		seqeditortrackheader_vtable = *(self->base.vtable);
		seqeditortrackheader_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrackheader_ondraw;
		seqeditortrackheader_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrackheader_onpreferredsize;
		seqeditortrackheader_vtable.onmousedown = (seqeditortrack_fp_onmousedown)
			seqeditortrackheader_onmousedown;
		seqeditortrackheader_vtable.onmousemove = (seqeditortrack_fp_onmousemove)
			seqeditortrackheader_onmousemove;
		seqeditortrackheader_vtable.onmouseup = (seqeditortrack_fp_onmouseup)
			seqeditortrackheader_onmouseup;
		seqeditortrackheader_vtable_initialized = TRUE;
	}
}
void seqeditortrackheader_init(SeqEditorTrackHeader* self,
	struct SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_init(&self->base, parent, trackstate, workspace);
	seqeditortrackheader_vtable_init(self);
	self->base.vtable = &seqeditortrackheader_vtable;
}

SeqEditorTrackHeader* seqeditortrackheader_alloc(void)
{
	return (SeqEditorTrackHeader*)malloc(sizeof(SeqEditorTrackHeader));
}

SeqEditorTrackHeader* seqeditortrackheader_allocinit(SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	SeqEditorTrackHeader* rv;

	rv = seqeditortrackheader_alloc();
	if (rv) {
		seqeditortrackheader_init(rv, parent, trackstate, workspace);
	}
	return rv;
}

void seqeditortrackheader_ondraw(SeqEditorTrackHeader* self,
	psy_ui_Graphics* g, double x, double y)
{	
	SequenceTrackBox trackbox;
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;	
		
	tm = psy_ui_component_textmetric(&self->base.parent->component);
	size = psy_ui_component_sizepx(&self->base.parent->component);	
	sequencetrackbox_init(&trackbox,
		psy_ui_rectangle_make(
			x, y, size.width,
			psy_ui_value_px(&self->base.trackstate->lineheight, tm)),
		tm, self->base.currtrack,
		(self->base.workspace->song) ? &self->base.workspace->song->sequence : NULL,
		self->base.trackindex,
		self->base.workspace->sequenceselection.editposition.track == self->base.trackindex);
	trackbox.showname = TRUE;
	sequencetrackbox_draw(&trackbox, g);
}

void seqeditortrackheader_onpreferredsize(SeqEditorTrackHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(rv);

	*rv = psy_ui_size_make(psy_ui_value_makeew(40.0),
		self->base.trackstate->lineheight);
}

bool seqeditortrackheader_onmousedown(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	if (self->base.currtrack) {		
		SequenceTrackBox trackbox;
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		psy_audio_Sequence* sequence;

		sequence = &self->base.workspace->song->sequence;
		tm = psy_ui_component_textmetric(&self->base.parent->component);
		size = psy_ui_component_sizepx(&self->base.parent->component);		
		sequencetrackbox_init(&trackbox,
			psy_ui_rectangle_make(
				0, 0, size.width,
				psy_ui_value_px(&self->base.trackstate->lineheight, tm)),
			tm,
			self->base.currtrack,
			(self->base.workspace->song) ? &self->base.workspace->song->sequence : NULL,
			self->base.trackindex,
			self->base.workspace->sequenceselection.editposition.track == self->base.trackindex);
		switch (sequencetrackbox_hittest(&trackbox, ev->x, 0)) {
		case SEQUENCETRACKBOXEVENT_MUTE:
			if (self->base.currtrack) {
				if (psy_audio_sequence_istrackmuted(sequence, self->base.trackindex)) {
					psy_audio_sequence_unmutetrack(sequence, self->base.trackindex);
				} else {
					psy_audio_sequence_mutetrack(sequence, self->base.trackindex);
				}				
			}
			break;
		case SEQUENCETRACKBOXEVENT_SOLO:
			if (self->base.currtrack) {
				if (psy_audio_sequence_istracksoloed(sequence, self->base.trackindex)) {
					psy_audio_sequence_deactivatesolotrack(sequence);
				} else {
					psy_audio_sequence_activatesolotrack(sequence, self->base.trackindex);
				}
			}
			break;
		case SEQUENCETRACKBOXEVENT_DEL: {
			psy_audio_SequencePosition position;
			position = psy_audio_sequence_at(&self->base.workspace->song->sequence,
				self->base.trackindex, 0);
			psy_audio_exclusivelock_enter();
			self->base.currtrack = NULL;
			psy_audio_sequence_removetrack(&self->base.workspace->song->sequence,
				position.tracknode);
			psy_audio_exclusivelock_leave();
			return FALSE;
			break; }
		// fallthrough
		case SEQUENCETRACKBOXEVENT_SELECT:
		default: {
			uintptr_t track;

			track = self->base.trackindex;
			if (track >= psy_audio_sequence_sizetracks(
					&self->base.workspace->song->sequence)) {
				if (psy_audio_sequence_sizetracks(&self->base.workspace->song->sequence) > 0) {
					track = psy_audio_sequence_sizetracks(&self->base.workspace->song->sequence) - 1;
				} else {
					track = 0;
				}
			}
			workspace_setsequenceeditposition(self->base.workspace,
				psy_audio_orderindex_make(track, 0));
			break; }
		}
		psy_ui_component_invalidate(&self->base.parent->component);
	} else {
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_appendtrack(&self->base.workspace->song->sequence,
			psy_audio_sequencetrack_allocinit());
		psy_audio_exclusivelock_leave();		
		return FALSE;
	}
	return TRUE;
}

bool seqeditortrackheader_onmousemove(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	return TRUE;
}

bool seqeditortrackheader_onmouseup(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	return TRUE;
}

// SeqEditorTracks
// prototypes
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_ondraw(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_drawplayline(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_drawcursorline(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_onpreferredsize(SeqEditorTracks*, psy_ui_Size*
	limit, psy_ui_Size* rv);
static void seqeditortracks_onmousedown(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmousemove(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmouseup(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmouseenter(SeqEditorTracks*);
static void seqeditortracks_onmouseleave(SeqEditorTracks*);
static bool seqeditortracks_notifymouse(SeqEditorTracks*, psy_ui_MouseEvent*,
	bool (*fp_notify)(SeqEditorTrack*, psy_ui_MouseEvent*));
static void seqeditortracks_build(SeqEditorTracks*);
static psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks*);
static void seqeditortracks_onsequenceselectionclear(SeqEditorTracks*,
	psy_audio_SequenceSelection*);
static void seqeditortracks_onsequenceselectionselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_onsequenceselectiondeselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks*,
	psy_audio_SequenceSelection*);
static void seqeditortracks_onsequencetrackreposition(SeqEditorTracks*,
	psy_audio_Sequence* sender, uintptr_t trackidx);

static void seqeditortracks_ontimer(SeqEditorTracks*, uintptr_t timerid);
static void seqeditortracks_oncursorchanged(SeqEditorTracks*, SeqEditorTrackState*);
// vtable
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);
		seqeditortracks_vtable.ondraw = (psy_ui_fp_component_ondraw)
			seqeditortracks_ondraw;
		seqeditortracks_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			seqeditortracks_onpreferredsize;
		seqeditortracks_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			seqeditortracks_onmousemove;
		seqeditortracks_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			seqeditortracks_onmouseup;
		seqeditortracks_vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)
			seqeditortracks_onmouseenter;
		seqeditortracks_vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)
			seqeditortracks_onmouseleave;
		seqeditortracks_vtable.ontimer = (psy_ui_fp_component_ontimer)
			seqeditortracks_ontimer;
		seqeditortracks_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, int mode, Workspace* workspace)
{
	self->workspace = workspace;
	self->trackstate = trackstate;
	self->mode = mode;
	self->drawpatternevents = TRUE;	
	self->dragmode = SEQEDITORDRAG_MOVE;
	psy_ui_component_init(&self->component, parent);	
	seqeditortracks_vtable_init(self);
	self->component.vtable = &seqeditortracks_vtable;
	self->tracks = NULL;
	self->lastplaylinepx = -1;
	self->capture = NULL;
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	seqeditortracks_build(self);
	psy_signal_connect(&workspace->sequenceselection.signal_clear, self,
		seqeditortracks_onsequenceselectionclear);
	psy_signal_connect(&workspace->sequenceselection.signal_select, self,
		seqeditortracks_onsequenceselectionselect);
	psy_signal_connect(&workspace->sequenceselection.signal_deselect, self,
		seqeditortracks_onsequenceselectiondeselect);
	psy_signal_connect(&workspace->sequenceselection.signal_update, self,
		seqeditortracks_onsequenceselectionupdate);
	if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
		psy_ui_component_starttimer(&self->component, 0, 50);
	}
	psy_signal_connect(&self->trackstate->signal_cursorchanged, self,
		seqeditortracks_oncursorchanged);
}

void seqeditortracks_ondestroy(SeqEditorTracks* self, psy_ui_Component* sender)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;

	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
	sequence = seqeditortracks_sequence(self);
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		
		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			SeqEditorTrack* seqedittrack;

			if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
				seqedittrack = seqeditortrack_allocinit(self, self->trackstate,
					self->workspace);
			} else if (self->mode == SEQEDITOR_TRACKMODE_HEADER) {
				seqedittrack = seqeditortrackheader_base(
					seqeditortrackheader_allocinit(self, self->trackstate,
						self->workspace));
			} else {
				seqedittrack = NULL;
			}
			if (seqedittrack) {
				psy_list_append(&self->tracks, seqedittrack);
				seqeditortrack_updatetrack(seqedittrack,
					t, (psy_audio_SequenceTrack*)t->entry, c);
			}
		}
		// add an empty track for the add track button			
		SeqEditorTrack* seqedittrack;

		seqedittrack = seqeditortrackheader_base(
			seqeditortrackheader_allocinit(self, self->trackstate,
				self->workspace));
		if (seqedittrack) {
			psy_list_append(&self->tracks, seqedittrack);
			seqeditortrack_updatetrack(seqedittrack,
				NULL, NULL, c + 1);
		}		
	}
	self->capture = NULL;
}

void seqeditortracks_ondraw(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence = seqeditortracks_sequence(self);	
	if (sequence) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		double cpx, cpy;
		uintptr_t c;
		const psy_ui_TextMetric* tm;
		double linemargin;
		double lineheight;

		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
		cpx = 0;		
		cpy = 0;
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, tm);		
		lineheight = psy_ui_value_px(&self->trackstate->lineheight, tm);
		for (seqeditnode = self->tracks, seqnode = sequence->tracks, c = 0;
				seqeditnode != NULL;
				seqnode = (seqnode) ? seqnode->next : seqnode,
				psy_list_next(&seqeditnode),
				++c) {
			if (cpy > g->clip.top - lineheight - linemargin) {
				SeqEditorTrack* seqedittrack;
				psy_audio_SequenceTrack* seqtrack;

				seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
				if (seqnode) {
					seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
				} else {
					seqtrack = NULL;
				}
				seqeditortrack_updatetrack(seqedittrack, seqnode, seqtrack, c);
				seqeditortrack_ondraw(seqedittrack, g, cpx, cpy);				
			}
			cpy += lineheight + linemargin;
			if (cpy > g->clip.bottom) {
				break;
			}
		}		
		if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
			seqeditortracks_drawplayline(self, g);
			seqeditortracks_drawcursorline(self, g);
		}
	}
}

void seqeditortracks_drawplayline(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	if (psy_audio_player_playing(workspace_player(self->workspace))) {		
		psy_ui_Rectangle position;		
		
		position = psy_ui_component_scrolledposition(&self->component);		
		psy_ui_setcolour(g, self->skin->playbar);
		psy_ui_drawline(g, self->lastplaylinepx, position.top, self->lastplaylinepx,
			position.bottom - position.top);
	}
}

void seqeditortracks_drawcursorline(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	if (self->trackstate->drawcursor) {
		psy_ui_Rectangle position;
		const psy_ui_TextMetric* tm;				
		double cpx;

		tm = psy_ui_component_textmetric(&self->component);
		cpx = self->trackstate->cursorposition * self->trackstate->pxperbeat;
		position = psy_ui_component_scrolledposition(&self->component);
		psy_ui_setcolour(g, self->skin->playbar);
		psy_ui_drawline(g, cpx, position.top, cpx,
			(psy_ui_value_px(&self->trackstate->trackssize.height, tm) - position.top));
		psy_ui_setcolour(g, self->skin->cursor);
	}	
}

bool seqeditortracks_playlinechanged(SeqEditorTracks* self)
{	
	double cpx;
		
	cpx = psy_audio_player_position(workspace_player(self->workspace)) *
		self->trackstate->pxperbeat;
	return cpx != self->lastplaylinepx;
}

SeqEditorTrack* seqeditortracks_at(SeqEditorTracks* self, uintptr_t trackidx)
{
	psy_List* node;

	node = psy_list_at(self->tracks, trackidx);
	if (node) {
		return (SeqEditorTrack*)psy_list_entry(node);
	}
	return NULL;
}

psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->sequence;
	}
	return NULL;
}

void seqeditortracks_onpreferredsize(SeqEditorTracks* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_Point maxsize;

	maxsize.x = psy_ui_value_makeew(0.0);
	maxsize.y = psy_ui_value_makeeh(0.0);
	if (workspace_song(self->workspace)) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		const psy_ui_TextMetric* tm;
		uintptr_t c;		
	
		tm = psy_ui_component_textmetric(&self->component);		
		for (seqeditnode = self->tracks, c = 0,
				seqnode = workspace_song(self->workspace)->sequence.tracks;
				seqeditnode != NULL;
				psy_list_next(&seqeditnode),
				seqnode = (seqnode)
					? seqnode->next
					: NULL,
				++c) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;
			psy_ui_Size limit;
			psy_ui_Size preferredtracksize;

			limit = psy_ui_component_size(&self->component);
			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			if (seqnode) {
				seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			} else {
				seqtrack = NULL;
			}
			seqeditortrack_updatetrack(seqedittrack, seqnode, seqtrack, c);
			seqeditortrack_onpreferredsize(seqedittrack, &limit, &preferredtracksize);			
			maxsize.x = psy_ui_max_values(maxsize.x, preferredtracksize.width, tm);
			psy_ui_value_add(&maxsize.y, &preferredtracksize.height, tm);
			psy_ui_value_add(&maxsize.y, &self->trackstate->linemargin, tm);
		}		
	}
	rv->width = maxsize.x;
	rv->height = maxsize.y;
}

void seqeditortracks_onmousedown(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	if (!seqeditortracks_notifymouse(self, ev, seqeditortrack_onmousedown)) {
		self->capture = NULL;
		seqeditortracks_build(self);
		seqeditortracks_invalidatebitmap(self);
	}
}

void seqeditortracks_onmousemove(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	seqeditortracks_notifymouse(self, ev, seqeditortrack_onmousemove);
}

void seqeditortracks_onmouseup(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	if (!seqeditortracks_notifymouse(self, ev, seqeditortrack_onmouseup)) {
		self->capture = NULL;
		seqeditortracks_build(self);
		seqeditortracks_invalidatebitmap(self);
	}
}

void seqeditortracks_onmouseenter(SeqEditorTracks* self)
{
	assert(self->trackstate);

	if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
		self->trackstate->drawcursor = TRUE;
		psy_signal_emit(&self->trackstate->signal_cursorchanged, self->trackstate, 0);
	}
}

void seqeditortracks_onmouseleave(SeqEditorTracks* self)
{
	assert(self->trackstate);

	if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
		self->trackstate->drawcursor = FALSE;
		psy_signal_emit(&self->trackstate->signal_cursorchanged, self->trackstate, 0);
	}
}

bool seqeditortracks_notifymouse(SeqEditorTracks* self, psy_ui_MouseEvent* ev,
	bool (*fp_notify)(SeqEditorTrack*, psy_ui_MouseEvent*))
{
	bool rv;
	psy_audio_Sequence* sequence;
	
	rv = TRUE;
	sequence = seqeditortracks_sequence(self);
	if (sequence) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		double cpy;
		intptr_t c;
		const psy_ui_TextMetric* tm;
		double linemargin;
		double lineheight;

		cpy = 0;
		tm = psy_ui_component_textmetric(&self->component);
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, tm);		
		lineheight = psy_ui_value_px(&self->trackstate->lineheight, tm);
		if (self->capture) {
			psy_ui_MouseEvent trackev;

			trackev = *ev;
			trackev.y = ev->y - (self->capture->trackindex *
				(lineheight + linemargin));
			rv = fp_notify(self->capture, &trackev);			
		} else {
			for (seqeditnode = self->tracks, seqnode = sequence->tracks, c = 0;
					seqeditnode != NULL;
					seqnode = (seqnode) ? seqnode->next : NULL,
					psy_list_next(&seqeditnode),
					++c) {
				SeqEditorTrack* seqedittrack;
				psy_audio_SequenceTrack* seqtrack;

				seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
				if (seqnode) {
					seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
				} else {
					seqtrack = NULL;
				}
				seqeditortrack_updatetrack(seqedittrack, seqnode, seqtrack, c);
				cpy += lineheight + linemargin;
				if (ev->y >= c * (lineheight + linemargin) &&
						ev->y < (c + 1) * (lineheight + linemargin)) {
					psy_ui_MouseEvent trackev;

					trackev = *ev;
					trackev.y = ev->y - (cpy - (lineheight + linemargin));
					rv = fp_notify(seqedittrack, &trackev);
					break;
				}
			}
		}
	}
	return rv;
}

void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection)
{
	seqeditortracks_invalidatebitmap(self);
}

void seqeditortracks_onsequencetrackreposition(SeqEditorTracks* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	SeqEditorTrack* track;

	track = seqeditortracks_at(self, trackidx);
	if (track) {
		track->bitmapvalid = FALSE;
		psy_ui_component_invalidate(&self->component);
	}
}

void seqeditortracks_onsequenceselectionclear(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection)
{
	seqeditortracks_invalidatebitmap(self);
}

void seqeditortracks_onsequenceselectionselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	SeqEditorTrack* track;

	track = seqeditortracks_at(self, index->track);
	if (track) {
		track->bitmapvalid = FALSE;
		psy_ui_component_invalidate(&self->component);
	}
}

void seqeditortracks_onsequenceselectiondeselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	SeqEditorTrack* track;

	track = seqeditortracks_at(self, index->track);
	if (track) {
		track->bitmapvalid = FALSE;
		psy_ui_component_invalidate(&self->component);
	}
}


void seqeditortracks_invalidatebitmap(SeqEditorTracks* self)
{
	psy_List* seqeditnode;

	for (seqeditnode = self->tracks; seqeditnode != NULL; psy_list_next(&seqeditnode)) {
		SeqEditorTrack* track;		

		track = (SeqEditorTrack*)psy_list_entry(seqeditnode);
		track->bitmapvalid = FALSE;
	}
	psy_ui_component_invalidate(&self->component);
}

void seqeditortracks_ontimer(SeqEditorTracks* self, uintptr_t timerid)
{		
	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		if (seqeditortracks_playlinechanged(self)) {
			double playlinepx;			
			psy_ui_Rectangle redrawrect;
						
			playlinepx = seqeditortrackstate_beattopx(self->trackstate,
				psy_audio_player_position(workspace_player(self->workspace)));
			redrawrect = psy_ui_component_scrolledposition(&self->component);
			if (self->lastplaylinepx == -1) {
				psy_ui_rectangle_setleft(&redrawrect, playlinepx);
				psy_ui_rectangle_setwidth(&redrawrect, 2);
			} else if (playlinepx > self->lastplaylinepx) {
				psy_ui_rectangle_setleft(&redrawrect, self->lastplaylinepx);
				psy_ui_rectangle_setright(&redrawrect, playlinepx + 1);
			} else {
				psy_ui_rectangle_setleft(&redrawrect, playlinepx);
				psy_ui_rectangle_setright(&redrawrect, self->lastplaylinepx + 1);				
			}
			self->lastplaylinepx = playlinepx;
			psy_ui_component_invalidaterect(&self->component, redrawrect);
		}
	} else if (self->lastplaylinepx != -1) {
		psy_ui_Rectangle redrawrect;

		redrawrect = psy_ui_component_scrolledposition(&self->component);		
		psy_ui_rectangle_setleft(&redrawrect, self->lastplaylinepx);
		psy_ui_rectangle_setwidth(&redrawrect, 2);
		self->lastplaylinepx = -1;
		psy_ui_component_invalidate(&self->component);
	}	
}

void seqeditortracks_oncursorchanged(SeqEditorTracks* self,
	SeqEditorTrackState* trackstate)
{
	psy_ui_component_invalidate(&self->component);
}

// SeqEditor
void seqeditorbar_init(SeqEditorBar* self, psy_ui_Component* parent)
{
	psy_ui_Margin topmargin;

	psy_ui_margin_init_all(&topmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	zoombox_init(&self->zoombox_beat, &self->component);	
	psy_ui_component_setmargin(&self->component, &topmargin);
	psy_ui_button_init_text(&self->move, &self->component,
		"Move");
	psy_ui_button_init_text(&self->reorder, &self->component,
		"Reorder");
}

void seqeditorbar_setdragmode(SeqEditorBar* self, SeqEditorDragMode mode)
{
	switch (mode) {
	case SEQEDITORDRAG_MOVE:
		psy_ui_button_highlight(&self->move);
		psy_ui_button_disablehighlight(&self->reorder);
		break;
	case SEQEDITORDRAG_REORDER:
		psy_ui_button_highlight(&self->reorder);
		psy_ui_button_disablehighlight(&self->move);
		break;
	default:
		break;
	}
}	

// SeqEditor
// prototypes
static void seqeditor_ondestroy(SeqEditor*);
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_SongFile*);
static void seqeditor_updatesong(SeqEditor*, psy_audio_Song*);
static void seqeditor_build(SeqEditor*);
static void seqeditor_onentryscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onheaderscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onzoomboxbeatchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_onzoomboxheightchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_updatescrollstep(SeqEditor*);
static void seqeditor_updateoverflow(SeqEditor*);
static void seqeditor_ondragmodemove(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_ondragmodereorder(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onsequenceclear(SeqEditor*, psy_audio_Sequence*);
static void seqeditor_onsequenceinsert(SeqEditor*, psy_audio_Sequence*,
	psy_audio_OrderIndex*);
static void seqeditor_onsequenceremove(SeqEditor*, psy_audio_Sequence*,
	psy_audio_OrderIndex*);
static void seqeditor_onsequencetrackinsert(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_onsequencetrackremove(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
// vtable
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);
		seqeditor_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			seqeditor_ondestroy;
		seqeditor_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	PatternViewSkin* skin,
	Workspace* workspace)
{		
	psy_ui_Margin topmargin;
	
	psy_ui_margin_init_all(&topmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	seqeditor_vtable_init(self);
	self->component.vtable = &seqeditor_vtable;
	psy_ui_component_doublebuffer(&self->component);
	seqeditortrackstate_init(&self->trackstate);
	self->workspace = workspace;
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	seqeditorbar_init(&self->bar, &self->left);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->bar.zoombox_beat.signal_changed, self,
		seqeditor_onzoomboxbeatchanged);
	// track header
	seqeditortracks_init(&self->trackheaders, &self->left,
		&self->trackstate, SEQEDITOR_TRACKMODE_HEADER, workspace);
	self->trackheaders.skin = skin;
	psy_ui_component_setalign(&self->trackheaders.component,
		psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_height, &self->left);
	psy_ui_component_setalign(&self->zoombox_height.component,
		psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->zoombox_height.signal_changed, self,
		seqeditor_onzoomboxheightchanged);
	// ruler
	seqeditorruler_init(&self->ruler, &self->component, &self->trackstate,
		skin, workspace);	
	psy_ui_component_setalign(seqeditorruler_base(&self->ruler),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(seqeditorruler_base(&self->ruler), &topmargin);
	// tracks
	seqeditortracks_init(&self->tracks, &self->component,		
		&self->trackstate, SEQEDITOR_TRACKMODE_ENTRY, workspace);	
	self->tracks.skin = skin;	
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	seqeditorbar_setdragmode(&self->bar, self->tracks.dragmode);	
	// align
	psy_ui_component_resize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20.0),
			psy_ui_value_makeeh(6 * 1.4 + 2.5)));
	// use splitbar
	psy_ui_component_preventpreferredsize(&self->component);
	seqeditor_updatesong(self, workspace->song);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_onentryscroll);
	psy_signal_connect(&self->trackheaders.component.signal_scroll, self,
		seqeditor_onheaderscroll);
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);
	psy_signal_connect(&self->bar.move.signal_clicked, self,
		seqeditor_ondragmodemove);
	psy_signal_connect(&self->bar.reorder.signal_clicked, self,
		seqeditor_ondragmodereorder);
}

void seqeditor_ondestroy(SeqEditor* self)
{
	seqeditortrackstate_dispose(&self->trackstate);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	seqeditor_updatesong(self, workspace->song);
}

void seqeditor_updatesong(SeqEditor* self, psy_audio_Song* song)
{
	if (song) {
		psy_ui_Size size;
		
		size = psy_ui_component_size(&self->component);
		seqeditortracks_build(&self->tracks);
		seqeditortracks_build(&self->trackheaders);
		self->trackstate.trackssize =
			psy_ui_component_preferredsize(&self->tracks.component, &size);		
		psy_signal_connect(&song->sequence.signal_clear, self,
			seqeditor_onsequenceclear);
		psy_signal_connect(&song->sequence.signal_insert, self,
			seqeditor_onsequenceinsert);
		psy_signal_connect(&song->sequence.signal_remove, self,
			seqeditor_onsequenceremove);
		psy_signal_connect(&song->sequence.signal_trackinsert, self,
			seqeditor_onsequencetrackinsert);
		psy_signal_connect(&song->sequence.signal_trackremove, self,
			seqeditor_onsequencetrackremove);
		psy_signal_connect(&song->sequence.signal_trackreposition, &self->tracks,
			seqeditortracks_onsequencetrackreposition);
		seqeditor_updatescrollstep(self);
		seqeditor_updateoverflow(self);		
	}
}

void seqeditor_updatescrollstep(SeqEditor* self)
{	
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->tracks.component);
	self->tracks.component.scrollstepy = psy_ui_add_values(
		self->tracks.trackstate->lineheight,
		self->tracks.trackstate->linemargin,
		tm);
	self->trackheaders.component.scrollstepy =
		self->tracks.component.scrollstepy;
}

void seqeditor_updateoverflow(SeqEditor* self)
{
	psy_ui_component_updateoverflow(seqeditortracks_base(&self->tracks));
	psy_ui_component_updateoverflow(seqeditortracks_base(&self->trackheaders));
	psy_ui_component_invalidate(seqeditortracks_base(&self->tracks));
	psy_ui_component_invalidate(seqeditortracks_base(&self->trackheaders));
}

void seqeditor_onsequenceclear(SeqEditor* self, psy_audio_Sequence* sender)
{
	seqeditor_build(self);
}

void seqeditor_onsequenceinsert(SeqEditor* self, psy_audio_Sequence* sender,
	psy_audio_OrderIndex* index)
{
	SeqEditorTrack* track;
	
	track = seqeditortracks_at(&self->tracks, index->track);
	if (track) {
		track->bitmapvalid = FALSE;
		psy_ui_component_invalidate(&self->tracks.component);
	}
}

void seqeditor_onsequenceremove(SeqEditor* self, psy_audio_Sequence* sender,
	psy_audio_OrderIndex* index)
{
	SeqEditorTrack* track;

	track = seqeditortracks_at(&self->tracks, index->track);
	if (track) {
		track->bitmapvalid = FALSE;
		psy_ui_component_invalidate(&self->tracks.component);
	}
}

void seqeditor_onsequencetrackinsert(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);
}

void seqeditor_onsequencetrackremove(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);
}

void seqeditor_build(SeqEditor* self)
{
	psy_ui_Size size;

	seqeditortracks_build(&self->tracks);
	seqeditortracks_build(&self->trackheaders);
	size = psy_ui_component_size(&self->component);
	self->trackstate.trackssize =
		psy_ui_component_preferredsize(&self->tracks.component, &size);
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
}

void seqeditor_onentryscroll(SeqEditor* self, psy_ui_Component* sender)
{
	psy_ui_component_setscrollleft(&self->ruler.component,
		psy_ui_component_scrollleft(&self->tracks.component));
	psy_ui_component_setscrolltop(&self->trackheaders.component,
		psy_ui_component_scrolltop(&self->tracks.component));
}

void seqeditor_onheaderscroll(SeqEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrolltop(&self->tracks.component,
		psy_ui_component_scrolltop(&self->trackheaders.component));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{
	seqeditortracks_invalidatebitmap(&self->tracks);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onzoomboxbeatchanged(SeqEditor* self, ZoomBox* sender)
{
	self->trackstate.pxperbeat = sender->zoomrate * DEFAULT_PXPERBEAT;
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
	seqeditortracks_invalidatebitmap(&self->tracks);
	psy_ui_component_invalidate(&self->ruler.component);
}

void seqeditor_onzoomboxheightchanged(SeqEditor* self, ZoomBox* sender)
{
	self->trackstate.lineheight = psy_ui_mul_value_real(
		self->trackstate.defaultlineheight, zoombox_rate(sender));
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
	seqeditortracks_invalidatebitmap(&self->tracks);
}

void seqeditor_ondragmodemove(SeqEditor* self, psy_ui_Component* sender)
{
	self->tracks.dragmode = SEQEDITORDRAG_MOVE;
	seqeditorbar_setdragmode(&self->bar, self->tracks.dragmode);
}

void seqeditor_ondragmodereorder(SeqEditor* self, psy_ui_Component* sender)
{
	self->tracks.dragmode = SEQEDITORDRAG_REORDER;
	seqeditorbar_setdragmode(&self->bar, self->tracks.dragmode);
}
