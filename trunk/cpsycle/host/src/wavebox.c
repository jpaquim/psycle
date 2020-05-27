// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavebox.h"

#include <stdlib.h>
#include <string.h>

static void wavebox_ondraw(WaveBox*, psy_ui_Graphics*);
static void wavebox_ondestroy(WaveBox*, psy_ui_Component* sender);
static void wavebox_onmousedown(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmousemove(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmouseup(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static int wavebox_hittest(WaveBox*, uintptr_t frame, int x, int epsilon);
static psy_ui_Rectangle wavebox_framerangetoscreen(WaveBox*, uintptr_t framebegin,
	uintptr_t frameend);
static int wavebox_hittest_range(WaveBox*, uintptr_t framemin, uintptr_t framemax, 
	int x);
static uintptr_t wavebox_screentoframe(WaveBox*, int x);
static int wavebox_frametoscreen(WaveBox*, uintptr_t frame);
static void wavebox_swapselection(WaveBox*);
static void  wavebox_onsize(WaveBox*, psy_ui_Size*);
static uintptr_t wavebox_numframes(WaveBox*);
static uintptr_t wavebox_numloopframes(WaveBox*);
static psy_dsp_amp_t wavebox_amp(WaveBox*, uintptr_t frame);



static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(WaveBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondraw = (psy_ui_fp_ondraw) wavebox_ondraw;
		vtable.onsize = (psy_ui_fp_onsize) wavebox_onsize;
		vtable_initialized = 1;
	}
}

void wavebox_init(WaveBox* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->sample = 0;
	self->channel = 0;
	self->hasselection = 0;
	self->selectionstart = 0;
	self->selectionend = 0;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	self->offsetstep = 0;
	self->nowavetext = 0;
	self->doubleloop = FALSE;
	self->preventdrawonselect = FALSE;
	wavebox_setnowavetext(self, "No wave loaded");
	psy_signal_init(&self->selectionchanged);
	psy_signal_connect(&self->component.signal_destroy, self, wavebox_ondestroy);
	psy_signal_connect(&self->component.signal_mousedown, self,
		wavebox_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		wavebox_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		wavebox_onmouseup);
}

void wavebox_ondestroy(WaveBox* self, psy_ui_Component* sender)
{	
	psy_signal_dispose(&self->selectionchanged);
	free(self->nowavetext);
}

void wavebox_setnowavetext(WaveBox* self, const char* text)
{
	free(self->nowavetext);
	self->nowavetext = strdup(text);
}

void wavebox_setsample(WaveBox* self, psy_audio_Sample* sample, uintptr_t channel)
{
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	self->sample = sample;
	self->channel = channel;
	self->offsetstep = sample ? (double)wavebox_numframes(self) / size.width *
		((double)self->zoomright - (double)self->zoomleft) : 0.0;
	psy_ui_component_invalidate(&self->component);
}

void wavebox_ondraw(WaveBox* self, psy_ui_Graphics* g)
{	
	psy_ui_Rectangle r;
	psy_ui_Size size = psy_ui_component_size(&self->component);	
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_setcolor(g, 0x00B1C8B0);
	if (!self->sample) {
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, 0x00D1C5B6);
		psy_ui_textout(g,
			(size.width - tm.tmAveCharWidth * strlen(self->nowavetext)) / 2,
			(size.height - tm.tmHeight) / 2,
			self->nowavetext, strlen(self->nowavetext));
	} else {
		psy_dsp_amp_t scaley;		
		int x;
		int centery = size.height / 2;
		psy_ui_Rectangle cont_loop_rc;
		psy_ui_Rectangle cont_doubleloop_rc;
		psy_ui_Rectangle sustain_loop_rc;		

		if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			cont_loop_rc = wavebox_framerangetoscreen(self,
				self->sample->loop.start,
				self->sample->loop.end);
			if (self->doubleloop) {
				cont_doubleloop_rc =
					wavebox_framerangetoscreen(self,
						self->sample->loop.end + 1,
						self->sample->loop.end + 1 +
							wavebox_numloopframes(self));
			}			
		}
		if (self->sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			sustain_loop_rc = wavebox_framerangetoscreen(self,
				self->sample->sustainloop.start,
				self->sample->sustainloop.end);
		}
		scaley = (size.height / 2) / (psy_dsp_amp_t)32768;	

		if (self->sample && self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {			
			psy_ui_drawsolidrectangle(g, cont_loop_rc, 0x00333333);
			if (self->doubleloop) {
				psy_ui_drawsolidrectangle(g, cont_doubleloop_rc, 0x00292929);
			}
		}		
		if (self->hasselection) {			
			psy_ui_drawsolidrectangle(g, wavebox_framerangetoscreen(self,
				self->selectionend, self->selectionend), 0x00B1C8B0);			
		}		
		for (x = 0; x < size.width; ++x) {			
			uintptr_t frame = (int)(self->offsetstep * x + 
				(int)(wavebox_numframes(self) * self->zoomleft));
			float framevalue;
			
			if (frame >= wavebox_numframes(self)) {
				break;
			}
			framevalue = wavebox_amp(self, frame);
			if (self->hasselection &&
				frame >= self->selectionstart &&
				frame < self->selectionend) {
				if (frame == self->selectionstart || frame == (self->selectionend - 1)) {
					psy_ui_setcolor(g, 0x00333333);
				} else {
					psy_ui_setcolor(g, 0x00262626);
				}
				psy_ui_drawline(g, x, 0, x, size.height);
				psy_ui_setcolor(g, 0x00FF2288);
			} else
			if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT &&
					psy_ui_rectangle_intersect(&cont_loop_rc, x, 0)) {
				psy_ui_setcolor(g, 0x00D1C5B6);
			} else {
				psy_ui_setcolor(g, 0x00B1C8B0);
			}
			psy_ui_drawline(g, x, centery, x, centery + (int)(framevalue * scaley));
		}
		if (self->sample && self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_setcolor(g, 0x00D1C5B6);
			psy_ui_drawline(g, cont_loop_rc.left, 0, cont_loop_rc.left + 1,
				size.height);
			psy_ui_drawline(g, cont_loop_rc.right, 0, cont_loop_rc.right + 1,
				size.height);
		}
		if (self->sample && self->sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_setcolor(g, 0x00B6C5D1);
			psy_ui_drawline(g, sustain_loop_rc.left, 0, sustain_loop_rc.left + 1,
				size.height);
			psy_ui_drawline(g, sustain_loop_rc.right, 0, sustain_loop_rc.right + 1,
				size.height);
		}
	}
}

psy_ui_Rectangle wavebox_framerangetoscreen(WaveBox* self, uintptr_t framebegin,
	uintptr_t frameend)
{
	psy_ui_Rectangle rv;
	int startx;
	int endx;
	psy_ui_Size size;
	
	size = psy_ui_component_size(&self->component);
	startx = wavebox_frametoscreen(self, framebegin);
	if (startx < 0) {
		startx = 0;
	}
	endx = wavebox_frametoscreen(self, frameend);
	if (endx < 0) {
		endx = 0;
	}
	psy_ui_setrectangle(&rv, startx, 0, endx - startx, size.height);
	return rv;
}

void wavebox_onmousedown(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->sample && wavebox_numframes(self) > 0) {
		if (self->hasselection) {
			if (wavebox_hittest(self, self->selectionstart, ev->x, 5)) {
				self->dragmode = WAVEBOX_DRAG_LEFT;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else 
			if (wavebox_hittest(self, self->selectionend, ev->x, 5)) {
				self->dragmode = WAVEBOX_DRAG_RIGHT;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else
			if (wavebox_hittest_range(self, self->selectionstart,
					self->selectionend, ev->x)) {
				self->dragmode = WAVEBOX_DRAG_MOVE;
				self->dragoffset = wavebox_screentoframe(self, ev->x)
					- self->selectionstart;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_MOVE);
			} else {
				self->hasselection = 0;
			}
		} 
		if (!self->hasselection) {
			if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (wavebox_hittest(self, self->sample->loop.start, ev->x, 5)) {
					self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_CONT_LEFT;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
					return;
				} else
				if (wavebox_hittest(self, self->sample->loop.end, ev->x, 5)) {
					self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_CONT_RIGHT;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
					return;
				}
			}
			if (self->sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (wavebox_hittest(self, self->sample->sustainloop.start, ev->x, 5)) {
					self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_SUSTAIN_LEFT;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
					return;
				} else
				if (wavebox_hittest(self, self->sample->sustainloop.end, ev->x, 5)) {
					self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_SUSTAIN_RIGHT;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
					return;
				}
			}			
			self->hasselection = 1;
			self->selectionstart = wavebox_screentoframe(self, ev->x);
			self->selectionend = self->selectionstart;
			self->dragmode = WAVEBOX_DRAG_RIGHT;			
		}
		psy_ui_component_capture(&self->component);
	}
}

void wavebox_onmousemove(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->sample && wavebox_numframes(self) > 0) {
		uintptr_t frame;
		bool changed;

		changed = FALSE;
		frame = wavebox_screentoframe(self, ev->x);
		if (self->dragmode == WAVEBOX_DRAG_LEFT) {		
			self->selectionstart = frame;
			if (self->selectionstart > self->selectionend) {
				wavebox_swapselection(self);
				self->dragmode = WAVEBOX_DRAG_RIGHT;
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_RIGHT) {
			self->selectionend = frame;
			if (self->selectionend < self->selectionstart) {
				wavebox_swapselection(self);
				self->dragmode = WAVEBOX_DRAG_LEFT;
			}
			changed = TRUE;			
		} else 
		if (self->dragmode == WAVEBOX_DRAG_MOVE) {
			uintptr_t length;
			intptr_t start;

			length = self->selectionend - self->selectionstart;
			start = frame - self->dragoffset;		
			if (start < 0) {
				start = 0;
			}
			self->selectionstart = start;
			self->selectionend = self->selectionstart + length;
			if (self->selectionend >= wavebox_numframes(self)) {
				self->selectionend = wavebox_numframes(self) - 1;
				self->selectionstart = wavebox_numframes(self) - 1 - length;
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_CONT_LEFT) {
			if (frame > self->sample->loop.end) {
				self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_CONT_RIGHT;
				sample_setcontloop(self->sample,
					self->sample->loop.type,
					self->sample->loop.end, frame);
			} else {
				sample_setcontloop(self->sample,
					self->sample->loop.type,
					frame, self->sample->loop.end);
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_CONT_RIGHT) {
			if (frame < self->sample->loop.start) {
				sample_setcontloop(self->sample,
					self->sample->loop.type,
					frame, self->sample->loop.start);
				self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_CONT_LEFT;
			} else {
				sample_setcontloop(self->sample,
					self->sample->loop.type,
					self->sample->loop.start, frame);
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_SUSTAIN_LEFT) {												
			if (frame > self->sample->sustainloop.end) {
				self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_SUSTAIN_RIGHT;
				sample_setsustainloop(self->sample,
					self->sample->sustainloop.type,
					self->sample->sustainloop.end, frame);
			} else {
				sample_setsustainloop(self->sample,
					self->sample->sustainloop.type,
					frame, self->sample->sustainloop.end);
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_SUSTAIN_RIGHT) {			
			if (frame < self->sample->sustainloop.start) {
				sample_setsustainloop(self->sample,
					self->sample->sustainloop.type,
					frame, self->sample->sustainloop.start);
				self->dragmode = WAVEBOX_DRAG_psy_audio_SAMPLE_LOOP_SUSTAIN_LEFT;
			} else {
				sample_setsustainloop(self->sample,
					self->sample->sustainloop.type,
					self->sample->sustainloop.start, frame);
			}
			changed = TRUE;
			
		} else
		if (self->hasselection && 
			(wavebox_hittest(self, self->selectionstart, ev->x, 5) ||
				wavebox_hittest(self, self->selectionend, ev->x, 5))) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else
		if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT &&
			(wavebox_hittest(self, self->sample->loop.start, ev->x, 5) ||
				wavebox_hittest(self, self->sample->loop.end, ev->x, 5))) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else
		if (self->sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT &&
			(wavebox_hittest(self, self->sample->sustainloop.start, ev->x, 5) ||
				wavebox_hittest(self, self->sample->sustainloop.end, ev->x, 5))) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else
		if (wavebox_hittest_range(self, self->selectionstart,
				self->selectionend, ev->x)) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_MOVE);
		}
		if (changed) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
			if (!self->preventdrawonselect) {
				psy_ui_component_invalidate(&self->component);
			}
			psy_signal_emit(&self->selectionchanged, self, 0);
		}
	}
}

void wavebox_swapselection(WaveBox* self)
{
	uintptr_t tmp;

	tmp = self->selectionstart;
	self->selectionstart = self->selectionend;
	self->selectionend = self->selectionstart;
}

int wavebox_hittest(WaveBox* self, uintptr_t frame, int x, int epsilon)
{		
	return frame >= wavebox_screentoframe(self, x - epsilon) && 
		   frame <= wavebox_screentoframe(self, x + epsilon);
}

int wavebox_hittest_range(WaveBox* self, uintptr_t framemin,
	uintptr_t framemax, int x)
{	
	uintptr_t frame = wavebox_screentoframe(self, x);
	return frame >= framemin && frame <= framemax;
}

uintptr_t wavebox_screentoframe(WaveBox* self, int x)
{
	uintptr_t rv = 0;
	intptr_t frame;

	if (self->sample) {
		psy_ui_Size size;		
		
		size = psy_ui_component_size(&self->component);			
		frame = (int)(self->offsetstep * x) + 
			(int)(wavebox_numframes(self) * self->zoomleft);
		if (frame < 0) {
			frame = 0;
		} else
		if (frame >= (intptr_t)wavebox_numframes(self)) {
			if (wavebox_numframes(self) > 0) {
				frame = wavebox_numframes(self) - 1;
			} else {
				frame = 0;
			}
		}
		rv = frame;
	}
	return rv;
}

int wavebox_frametoscreen(WaveBox* self, uintptr_t frame)
{
	int rv = 0;

	if (self->sample) {
		psy_ui_Size size;		
		
		size = psy_ui_component_size(&self->component);		
		rv = (int)((frame - (intptr_t)(wavebox_numframes(self) *
				self->zoomleft)) / self->offsetstep);
	}
	return rv;
}

uintptr_t wavebox_numframes(WaveBox* self)
{
	if (self->sample) {
		if (self->doubleloop) {
			return self->sample->numframes + wavebox_numloopframes(self);
		} else {
			return self->sample->numframes;
		}		
	}
	return 0;
}

uintptr_t wavebox_numloopframes(WaveBox* self)
{
	if (self->sample) {
		if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			return self->sample->loop.end - self->sample->loop.start + 1;
		}
	}
	return 0;
}

static psy_dsp_amp_t wavebox_amp(WaveBox* self, uintptr_t frame)
{
	if (self->sample) {
		if (self->doubleloop) {
			if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (frame <= self->sample->loop.end) {
					return self->sample->channels.samples[self->channel][frame];
				} else {
					return self->sample->channels.samples[self->channel]
						[frame - wavebox_numloopframes(self)];
				}
			} 
		}
		return self->sample->channels.samples[self->channel][frame];		
	}
	return 0.f;
}

void wavebox_onmouseup(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->sample && wavebox_numframes(self) > 0) {
		psy_ui_component_releasecapture(&self->component);
		self->dragmode = WAVEBOX_DRAG_NONE;
		if (self->selectionstart == self->selectionend) {
			self->hasselection = 0;
			psy_ui_component_invalidate(&self->component);
			psy_signal_emit(&self->selectionchanged, self, 0);
		}	
	}
}

void wavebox_setzoom(WaveBox* self, psy_dsp_beat_t zoomleft,
	psy_dsp_beat_t zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	if (self->sample) {
		psy_ui_Size size;		

		size = psy_ui_component_size(&self->component);
		self->offsetstep = (float)wavebox_numframes(self) / size.width *
			(self->zoomright - self->zoomleft);
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_update(&self->component);
	}
}

void wavebox_setselection(WaveBox* self, uintptr_t selectionstart,
	uintptr_t selectionend)
{
	self->hasselection = TRUE;
	self->selectionstart = selectionstart;
	self->selectionend = selectionend;	
}

void wavebox_clearselection(WaveBox* self)
{
	self->hasselection = FALSE;	
}

void  wavebox_onsize(WaveBox* self, psy_ui_Size* size)
{	
	if (self->sample) {
		self->offsetstep = (double)wavebox_numframes(self) / size->width *
			(self->zoomright - self->zoomleft);
	} else {
		self->offsetstep = 1.f;
	}
}
