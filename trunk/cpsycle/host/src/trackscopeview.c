// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackscopeview.h"
#include <portable.h>
#include <math.h>
#include <uiapp.h>

#define TIMERID_TRACKSCOPEVIEW 6000

static void trackscopeview_ondraw(TrackScopeView*, psy_ui_Graphics*);
static void trackscopeview_onmousedown(TrackScopeView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void trackscopeview_drawtrack(TrackScopeView*, psy_ui_Graphics*,
	int x, int y, int width, int height, int track);
void trackscopeview_drawtrackindex(TrackScopeView*, psy_ui_Graphics*,
	int x, int y, int width, int height, int track);
void trackscopeview_drawtrackmuted(TrackScopeView*, psy_ui_Graphics*, int x,
	int y, int width, int height, int track);
static void trackscopeview_ontimer(TrackScopeView*, psy_ui_Component* sender,
	int timerid);
static void trackscopeview_preferredsize(TrackScopeView*, ui_size* limit,
	ui_size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(TrackScopeView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.preferredsize = (psy_ui_fp_preferredsize)
			trackscopeview_preferredsize;
		vtable.draw = (psy_ui_fp_draw) trackscopeview_ondraw;
	}
}

void trackscopeview_init(TrackScopeView* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	self->workspace = workspace;
	self->trackheight = 30;
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->component.doublebuffered = 1;	
	psy_signal_connect(&self->component.signal_mousedown, self,
		trackscopeview_onmousedown); 
	psy_signal_connect(&self->component.signal_timer, self,
		trackscopeview_ontimer);
	ui_component_starttimer(&self->component, TIMERID_TRACKSCOPEVIEW, 50);
}

void trackscopeview_ondraw(TrackScopeView* self, psy_ui_Graphics* g)
{
	if (self->workspace->song) {
		ui_size size;
		int numtracks = player_numsongtracks(&self->workspace->player);
		int c;
		int maxcolumns = 16;
		int columns = maxcolumns;
		int rows = 1;
		int track = 0;
		int line = 0;		
		int cpx = 0;
		int cpy = 0;
		int width;		

		columns = numtracks < maxcolumns ? numtracks : maxcolumns;
		size = ui_component_size(&self->component);
		width = size.width / columns;
		ui_setbackgroundmode(g, TRANSPARENT);
		ui_settextcolor(g, 0x00444444);
		for (c = 0; c < numtracks; ++c) {
			trackscopeview_drawtrackindex(self, g, cpx, cpy, width,
					self->trackheight, c);
			if (!patternstrackstate_istrackmuted(
					&self->workspace->song->patterns.trackstate, c)) {
				trackscopeview_drawtrack(self, g, cpx, cpy, width,
					self->trackheight, c);
			} else {
				trackscopeview_drawtrackmuted(self, g, cpx, cpy, width,
					self->trackheight, c);
			}
			if (track < columns) {
				++track;
				cpx += width;
			} else {
				++line;
				track = 0;
				cpx = 0;
				cpy += self->trackheight;			
			}
		}
	}
}

void trackscopeview_drawtrackindex(TrackScopeView* self, psy_ui_Graphics* g, int x, int y,
	int width, int height, int track)
{
	char text[40];
	extern psy_ui_App app;

	ui_setcolor(g, app.defaults.defaultcolor);
	psy_snprintf(text, 40, "%X", track);
	ui_textout(g, x + 3, y, text, strlen(text));
}


void trackscopeview_drawtrack(TrackScopeView* self, psy_ui_Graphics* g, int x, int y,
	int width, int height, int track)
{
	uintptr_t lastmachine;	

	if (psy_table_exists(&self->workspace->player.sequencer.lastmachine, track)) {
		lastmachine = (uintptr_t)
			psy_table_at(&self->workspace->player.sequencer.lastmachine, track);	
	} else {
		lastmachine = NOMACHINE_INDEX;
	}	
	if (lastmachine != NOMACHINE_INDEX) {
		char text[40];

		psy_snprintf(text, 40, "%X", lastmachine);
		ui_textout(g, x + width - 10, y + height - 12, text, strlen(text));		
	}
	ui_setcolor(g, 0x00888888);
	if (self->workspace->song) {
		psy_audio_Machine* machine;
		int centery;

		centery = height / 2 + y;
		machine = machines_at(&self->workspace->song->machines, lastmachine);
		if (machine) {
			psy_audio_Buffer* memory;
			
			memory = machine->vtable->buffermemory(machine);
			if (memory) {
				uintptr_t numsamples;
				uintptr_t frame;
				float px;
				float py;
				float cpx = 0;
				int x1, y1, x2, y2;
				static float epsilon = 0.1f;

				numsamples = machine->vtable->buffermemorysize(machine);				
				

				if (numsamples > 0) {
					int zero = 1;

					for (frame = 0; frame < numsamples; ++frame) {
						if (fabs(memory->samples[0][frame]) > epsilon) {
							zero = 0;
							break;
						}
					}
					if (!zero) {
						px = width / (float) numsamples;
						py = height / 32768.f / 2;
						x1 = 0;
						y1 = (int) (memory->samples[0][0] * py);
						x2 = 0;
						y2 = y1;						
						for (frame = 0; frame < numsamples; ++frame, cpx += px) {
							x1 = x2;
							x2 = (int) (frame * px);
							if (frame == 0 || x1 != x2) {
								y1 = y2;							
								y2 = (int) (memory->samples[0][frame] * py);
								ui_drawline(g, x + x1, centery + y1, x + x2, centery + y2);
							}
						}
					} else {
						ui_drawline(g, x, centery, x + width, centery);
					}
				}
			} else {
				ui_drawline(g, x, centery, x + width, centery);
			}
		} else {
			ui_drawline(g, x, centery, x + width, centery);
		}
	}	
}

void trackscopeview_drawtrackmuted(TrackScopeView* self, psy_ui_Graphics* g, int x,
	int y, int width, int height, int track)
{	
	int ident = (int)(width * 0.25);
	ui_setcolor(g, app.defaults.defaultcolor);

	ui_moveto(g, ui_point_make(x + ident, y + (int)(height * 0.2)));
	ui_devcurveto(g,
		ui_point_make(x + width - ident * 2, y + (int)(height * 0.3)),
		ui_point_make(x + width - ident, y + (int)(height * 0.6)),
		ui_point_make(x + width - (int)(ident * 0.5), y + (int)(height * 0.9)));
	ui_moveto(g,
		ui_point_make(x + ident + (int)(width * 0.1), y + (int)(height * 0.8)));
	ui_devcurveto(g,
		ui_point_make(x + ident + (int)(width * 0.3), y + (int)(height * 0.4)),
		ui_point_make(x + width - ident * 2, y + (int)(height * 0.2)),
		ui_point_make(x + width - (int)(ident * 0.5), (int)(height * 0.25)));
}

void trackscopeview_ontimer(TrackScopeView* self, psy_ui_Component* sender, int timerid)
{
	ui_component_invalidate(&self->component);	
}

void trackscopeview_preferredsize(TrackScopeView* self, ui_size* limit,
	ui_size* rv)
{
	int maxcolumns = 16;
	int columns = maxcolumns;
	int numtracks = player_numsongtracks(&self->workspace->player);
	columns = numtracks < maxcolumns ? numtracks : maxcolumns;

	rv->width = 16 * 30;
	rv->height = (numtracks / columns) * self->trackheight;
}

void trackscopeview_onmousedown(TrackScopeView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->workspace->song) {
		int columns;
		ui_size size;
		int track;
		int trackwidth;
		int maxcolumns = 16;
		int numtracks = player_numsongtracks(&self->workspace->player);

		columns = numtracks < maxcolumns ? numtracks : maxcolumns;
		size = ui_component_size(&self->component);
		trackwidth = size.width / columns;
		
		track = (ev->x / trackwidth) + (ev->y / self->trackheight) * columns;
		if (ev->button == 1) {
			if (!patternstrackstate_istrackmuted(
					&self->workspace->song->patterns.trackstate, track)) {
				patternstrackstate_mutetrack(
					&self->workspace->song->patterns.trackstate, track);
			} else {
				patternstrackstate_unmutetrack(
					&self->workspace->song->patterns.trackstate, track);
			}
		} else
		if (ev->button == 2) {				
			if (patterns_istracksoloed(&self->workspace->song->patterns,
						track)) {
				patterns_deactivatesolotrack(
					&self->workspace->song->patterns);
			} else {
				patterns_activatesolotrack(
					&self->workspace->song->patterns, track);
			}
			ui_component_invalidate(&self->component);
		}
	}
}

void trackscopeview_start(TrackScopeView* self)
{
	ui_component_starttimer(&self->component, TIMERID_TRACKSCOPEVIEW, 50);
}

void trackscopeview_stop(TrackScopeView* self)
{
	ui_component_stoptimer(&self->component, TIMERID_TRACKSCOPEVIEW);
}
