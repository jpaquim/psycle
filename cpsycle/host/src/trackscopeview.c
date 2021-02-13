// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackscopeview.h"
// host
#include "styles.h"
// ui
#include <uiapp.h>
// std
#include <math.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

// prototypes
static void trackscopes_ondraw(TrackScopes*, psy_ui_Graphics*);
static void trackscopes_onmousedown(TrackScopes*, psy_ui_MouseEvent*);
static void trackscopes_drawtrack(TrackScopes*, psy_ui_Graphics*,
	double x, double y, uintptr_t track);
void trackscopes_drawtrackindex(TrackScopes*, psy_ui_Graphics*,
	double x, double y, uintptr_t track);
void trackscopes_drawtrackmuted(TrackScopes*, psy_ui_Graphics*, double x,
	double y, uintptr_t track);
static void trackscopes_ontimer(TrackScopes*, uintptr_t timerid);
static void trackscopes_onalign(TrackScopes*);
static void trackscopes_onpreferredsize(TrackScopes*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static uintptr_t trackscopes_numrows(const TrackScopes*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(TrackScopes* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)trackscopes_ontimer;
		vtable.onalign = (psy_ui_fp_component_onalign)trackscopes_onalign;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackscopes_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_component_ondraw)trackscopes_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			trackscopes_onmousedown;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void trackscopes_init(TrackScopes* self, psy_ui_Component* parent,
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

void trackscopes_ondraw(TrackScopes* self, psy_ui_Graphics* g)
{
	if (workspace_song(self->workspace)) {
		uintptr_t numtracks = psy_audio_song_numsongtracks(
			workspace_song(self->workspace));
		uintptr_t c;
		intptr_t rows = 1;
		uintptr_t currtrack;
		double cpx;
		double cpy;
				
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00444444));
		psy_ui_setcolour(g, psy_ui_colour_make(0x00777777));
		currtrack = 0;
		for (c = 0, cpx = cpy = 0; c < numtracks; ++c) {
			trackscopes_drawtrackindex(self, g, cpx, cpy, c);
			if (!psy_audio_trackstate_istrackmuted(
					&workspace_song(self->workspace)->patterns.trackstate, c)) {
				trackscopes_drawtrack(self, g, cpx, cpy, c);
			} else {
				trackscopes_drawtrackmuted(self, g, cpx, cpy, c);
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

void trackscopes_drawtrackindex(TrackScopes* self, psy_ui_Graphics* g,
	double x, double y, uintptr_t track)
{
	char text[40];
		
	psy_snprintf(text, 40, "%X", (int)track);
	psy_ui_textout(g, x + 3, y + 2, text, strlen(text));
}


void trackscopes_drawtrack(TrackScopes* self, psy_ui_Graphics* g,
	double x, double y, uintptr_t track)
{
	uintptr_t lastmachine;
	double width;
	double height;

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
		lastmachine = psy_INDEX_INVALID;
	}	
	if (workspace_song(self->workspace)) {
		psy_audio_Machine* machine;
		double centery;
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
						px = (float)width / (float)numsamples;
						py = (float)height * psy_audio_buffer_rangefactor(memory,
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
								psy_ui_drawline(g,
									psy_ui_realpoint_make(x + x1, centery + y1),
									psy_ui_realpoint_make(x + x2, centery + y2));
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
			psy_ui_drawline(g, psy_ui_realpoint_make(x, centery),
				psy_ui_realpoint_make(x + width, centery));
		}
	}	
}

void trackscopes_drawtrackmuted(TrackScopes* self, psy_ui_Graphics* g, double x,
	double y, uintptr_t track)
{	
	double width;
	double height;
	double ident;

	width = self->trackwidth;
	height = self->trackheight;
	ident = width * 0.25;
	psy_ui_setcolour(g, psy_ui_style(psy_ui_STYLE_COMMON)->colour);
	psy_ui_moveto(g, psy_ui_realpoint_make(x + ident, y + (int)(height * 0.2)));
	psy_ui_curveto(g,
		psy_ui_realpoint_make(x + width - ident * 2, y + (int)(height * 0.3)),
		psy_ui_realpoint_make(x + width - ident, y + (int)(height * 0.6)),
		psy_ui_realpoint_make(x + width - (int)(ident * 0.5), y + (int)(height * 0.9)));
	psy_ui_moveto(g,
		psy_ui_realpoint_make(x + ident + (int)(width * 0.1), y + (int)(height * 0.8)));
	psy_ui_curveto(g,
		psy_ui_realpoint_make(x + ident + (int)(width * 0.3), y + (int)(height * 0.4)),
		psy_ui_realpoint_make(x + width - ident * 2, y + (int)(height * 0.2)),
		psy_ui_realpoint_make(x + width - (int)(ident * 0.5), y + (int)(height * 0.25)));
}

void trackscopes_ontimer(TrackScopes* self, uintptr_t timerid)
{
	psy_ui_component_invalidate(&self->component);	
}

void trackscopes_onalign(TrackScopes* self)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Size size;	
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);	
	self->trackheight = (int)(tm->tmHeight * 2.75f);
	self->textheight = tm->tmHeight;
	if (workspace_song(self->workspace)) {
		uintptr_t numtracks;

		numtracks = psy_audio_song_numsongtracks(workspace_song(self->workspace));
		if (numtracks <= 32) {
			self->maxcolumns = 16;
		} else {
			self->maxcolumns = 32;
		}		
	} else {
		self->maxcolumns = 16;
	}
	self->trackwidth = psy_ui_value_px(&size.width, tm) / self->maxcolumns;
}

void trackscopes_onpreferredsize(TrackScopes* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{			
	
	rv->width = psy_ui_value_makeew(2 * 30);
	rv->height = psy_ui_value_makeeh(trackscopes_numrows(self) * 2.75);
}

uintptr_t trackscopes_numrows(const TrackScopes* self)
{	
	if (workspace_song_const(self->workspace) &&
			psy_audio_song_numsongtracks(workspace_song_const(self->workspace))
			> 16) {
		return 2;
	}
	return 1;
}

void trackscopes_onmousedown(TrackScopes* self, psy_ui_MouseEvent* ev)
{
	if (workspace_song(self->workspace)) {
		intptr_t columns;
		psy_ui_Size size;
		const psy_ui_TextMetric* tm;
		uintptr_t track;
		double trackwidth;
		uintptr_t numtracks;
		
		numtracks = psy_audio_song_numsongtracks(workspace_song(self->workspace));
		columns = numtracks < self->maxcolumns ? numtracks : self->maxcolumns;
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		trackwidth = psy_ui_value_px(&size.width, tm) / columns;		
		track = (uintptr_t)((ev->pt.x / trackwidth) + floor(ev->pt.y / self->trackheight) * columns);
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

void trackscopes_start(TrackScopes* self)
{
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackscopes_stop(TrackScopes* self)
{
	psy_ui_component_stoptimer(&self->component, 0);
}


// TrackScopeView
void trackscopeview_init(TrackScopeView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setstyletypes(&self->component, STYLE_TRACKSCOPES,
		STYLE_TRACKSCOPES, STYLE_TRACKSCOPES);
	trackscopes_init(&self->scopes, &self->component, workspace);
	psy_ui_component_setalign(&self->scopes.component, psy_ui_ALIGN_CLIENT);
}
