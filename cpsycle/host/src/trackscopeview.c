// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackscopeview.h"
// ui
#include <uiapp.h>
// std
#include <math.h>
#include <string.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

// prototypes
static void trackscopeview_ondraw(TrackScopeView*, psy_ui_Graphics*);
static void trackscopeview_onmousedown(TrackScopeView*, psy_ui_MouseEvent*);
static void trackscopeview_drawtrack(TrackScopeView*, psy_ui_Graphics*,
	intptr_t x, intptr_t y, intptr_t track);
void trackscopeview_drawtrackindex(TrackScopeView*, psy_ui_Graphics*,
	intptr_t x, intptr_t y, intptr_t track);
void trackscopeview_drawtrackmuted(TrackScopeView*, psy_ui_Graphics*, intptr_t x,
	intptr_t y, intptr_t track);
static void trackscopeview_ontimer(TrackScopeView*, uintptr_t timerid);
static void trackscopeview_onalign(TrackScopeView*);
static void trackscopeview_onpreferredsize(TrackScopeView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(TrackScopeView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)trackscopeview_ontimer;
		vtable.onalign = (psy_ui_fp_component_onalign)trackscopeview_onalign;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackscopeview_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_component_ondraw)trackscopeview_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			trackscopeview_onmousedown;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void trackscopeview_init(TrackScopeView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	psy_ui_component_doublebuffer(&self->component);
	self->workspace = workspace;
	self->trackwidth = 90;
	self->trackheight = 30;
	self->textheight = 12;
	self->maxcolumns = 16;
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackscopeview_ondraw(TrackScopeView* self, psy_ui_Graphics* g)
{
	if (workspace_song(self->workspace)) {
		uintptr_t numtracks = psy_audio_player_numsongtracks(workspace_player(self->workspace));
		uintptr_t c;
		intptr_t rows = 1;
		uintptr_t currtrack;
		intptr_t cpx;
		intptr_t cpy;
				
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00444444));
		psy_ui_setcolour(g, psy_ui_colour_make(0x00777777));
		currtrack = 0;
		for (c = 0, cpx = cpy = 0; c < numtracks; ++c) {
			trackscopeview_drawtrackindex(self, g, cpx, cpy, c);
			if (!psy_audio_trackstate_istrackmuted(
					&workspace_song(self->workspace)->patterns.trackstate, c)) {
				trackscopeview_drawtrack(self, g, cpx, cpy, c);
			} else {
				trackscopeview_drawtrackmuted(self, g, cpx, cpy, c);
			}
			if (currtrack < self->maxcolumns - 1) {
				++currtrack;
				cpx += self->trackwidth;
			} else {
				currtrack = 0;
				cpx = 0;
				cpy += self->trackheight;			
			}
		}
	}
}

void trackscopeview_drawtrackindex(TrackScopeView* self, psy_ui_Graphics* g,
	intptr_t x, intptr_t y, intptr_t track)
{
	char text[40];
		
	psy_snprintf(text, 40, "%X", track);
	psy_ui_textout(g, x + 3, y, text, strlen(text));
}


void trackscopeview_drawtrack(TrackScopeView* self, psy_ui_Graphics* g,
	intptr_t x, intptr_t y, intptr_t track)
{
	uintptr_t lastmachine;
	intptr_t width;
	intptr_t height;

	width = self->trackwidth;
	height = self->trackheight;
	if (psy_table_exists(&workspace_player(self->workspace)->sequencer.lastmachine,
			track)) {
		char text[40];

		lastmachine = (uintptr_t)
			psy_table_at(&workspace_player(self->workspace)->sequencer.lastmachine,
				track);
		psy_snprintf(text, 40, "%X", lastmachine);
		psy_ui_textout(g, x + width - 10, y + height - self->textheight, text,
			strlen(text));
	} else {
		lastmachine = UINTPTR_MAX;
	}	
	if (workspace_song(self->workspace)) {
		psy_audio_Machine* machine;
		intptr_t centery;
		bool active = FALSE;
		
		centery = height / 2 + y;
		machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
			lastmachine);
		if (machine) {
			psy_audio_Buffer* memory;
			
			memory = psy_audio_machine_buffermemory(machine);
			if (memory) {
				uintptr_t numsamples;
				uintptr_t frame;
				float px;
				float py;
				float cpx = 0;
				int x1, y1, x2, y2;
				static float epsilon = 0.01f;
				
				numsamples = psy_audio_machine_buffermemorysize(machine);
				numsamples = psy_min(numsamples, psy_audio_MAX_STREAM_SIZE);
				if (numsamples > 0) {
					bool zero;
					uintptr_t writepos;
					psy_dsp_amp_t rms;

					rms = psy_audio_buffer_rmsdisplay(memory);
					zero = rms < epsilon;
					if (!zero) {
						uintptr_t i;
						uintptr_t step;
						
						active = TRUE;
						step = 1;
						px = width / (float) numsamples;
						py = height * psy_audio_buffer_rangefactor(memory,
							PSY_DSP_AMP_RANGE_VST) / 3;						
						writepos = memory->writepos;
						if (writepos >= numsamples) {
							frame = numsamples - writepos;
						} else {
							frame = writepos - numsamples;
						}
						frame = psy_min(frame, numsamples - 1);						
						x1 = x2 = 0;
						y1 = y2 = (int) (memory->samples[0][frame] * py);
						for (i = 1; i < numsamples; i += step) {
							x1 = x2;
							x2 = (int) (i * px);
							if (x1 != x2) {
								y1 = y2;							
								y2 = (int) (memory->samples[0][frame] * py);
								if (y2 > height / 2 || y2 < -height / 2) {
									continue;
								}
								psy_ui_drawline(g, x + x1, centery + y1, x + x2,
									centery + y2);								
							}
							++frame;
							if (frame >= numsamples) {
								frame = 0;
							}
						}
					}
				}
			}			
		}
		if (!active) {
			psy_ui_drawline(g, x, centery, x + width, centery);
		}
	}	
}

void trackscopeview_drawtrackmuted(TrackScopeView* self, psy_ui_Graphics* g, intptr_t x,
	intptr_t y, intptr_t track)
{	
	intptr_t width;
	intptr_t height;
	intptr_t ident;

	width = self->trackwidth;
	height = self->trackheight;
	ident = (intptr_t)(width * 0.25);
	psy_ui_setcolour(g, app.defaults.style_common.colour);
	psy_ui_moveto(g, psy_ui_intpoint_make(x + ident, y + (int)(height * 0.2)));
	psy_ui_curveto(g,
		psy_ui_intpoint_make(x + width - ident * 2, y + (int)(height * 0.3)),
		psy_ui_intpoint_make(x + width - ident, y + (int)(height * 0.6)),
		psy_ui_intpoint_make(x + width - (int)(ident * 0.5), y + (int)(height * 0.9)));
	psy_ui_moveto(g,
		psy_ui_intpoint_make(x + ident + (int)(width * 0.1), y + (int)(height * 0.8)));
	psy_ui_curveto(g,
		psy_ui_intpoint_make(x + ident + (int)(width * 0.3), y + (int)(height * 0.4)),
		psy_ui_intpoint_make(x + width - ident * 2, y + (int)(height * 0.2)),
		psy_ui_intpoint_make(x + width - (int)(ident * 0.5), y + (int)(height * 0.25)));
}

void trackscopeview_ontimer(TrackScopeView* self, uintptr_t timerid)
{
	psy_ui_component_invalidate(&self->component);	
}

void trackscopeview_onalign(TrackScopeView* self)
{
	psy_ui_TextMetric tm;
	psy_ui_Size size;	
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);	
	self->trackheight = (int)(tm.tmHeight * 2.75f);
	self->textheight = tm.tmHeight;
	self->trackwidth = psy_ui_value_px(&size.width, &tm) / self->maxcolumns;
}

void trackscopeview_onpreferredsize(TrackScopeView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	uintptr_t rows;

	rows = ((psy_audio_player_numsongtracks(workspace_player(self->workspace)) - 1)
		/ self->maxcolumns) + 1;
	rv->width = psy_ui_value_makeew(2 * 30);
	rv->height = psy_ui_value_makeeh(rows * 2.75);
}

void trackscopeview_onmousedown(TrackScopeView* self, psy_ui_MouseEvent* ev)
{
	if (workspace_song(self->workspace)) {
		intptr_t columns;
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		intptr_t track;
		intptr_t trackwidth;
		uintptr_t numtracks;
		
		numtracks = psy_audio_player_numsongtracks(workspace_player(self->workspace));
		columns = numtracks < self->maxcolumns ? numtracks : self->maxcolumns;
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		trackwidth = psy_ui_value_px(&size.width, &tm) / columns;		
		track = (ev->x / trackwidth) + (ev->y / self->trackheight) * columns;
		if (ev->button == 1) {
			if (!psy_audio_trackstate_istrackmuted(
					&workspace_song(self->workspace)->patterns.trackstate, track)) {
				psy_audio_trackstate_mutetrack(
					&workspace_song(self->workspace)->patterns.trackstate, track);
			} else {
				psy_audio_trackstate_unmutetrack(
					&workspace_song(self->workspace)->patterns.trackstate, track);
			}
		} else if (ev->button == 2) {				
			if (psy_audio_patterns_istracksoloed(&workspace_song(self->workspace)->patterns,
						track)) {
				psy_audio_patterns_deactivatesolotrack(
					&workspace_song(self->workspace)->patterns);
			} else {
				psy_audio_patterns_activatesolotrack(
					&workspace_song(self->workspace)->patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void trackscopeview_start(TrackScopeView* self)
{
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackscopeview_stop(TrackScopeView* self)
{
	psy_ui_component_stoptimer(&self->component, 0);
}
