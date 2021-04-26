// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavebox.h"

// dsp
#include <operations.h>
// platform
#include "../../detail/portable.h"

static void waveboxselection_swap(WaveBoxSelection*);

void waveboxselection_init(WaveBoxSelection* self)
{
	self->hasselection = 0;
	self->start = 0;
	self->end = 0;
}

void waveboxselection_swap(WaveBoxSelection* self)
{
	uintptr_t tmp;

	tmp = self->start;
	self->start = self->end;
	self->end = self->start;	
}

void waveboxselection_setrange(WaveBoxSelection* self, uintptr_t start,
	uintptr_t end)
{
	self->start = start;
	self->end = end;
}

void waveboxselection_setstart(WaveBoxSelection* self, uintptr_t start,
	bool* swapped)
{
	self->start = start;	
	if (self->start > self->end) {
		waveboxselection_swap(self);
		if (swapped) {
			*swapped = TRUE;
		}
	} else
	if (swapped) {
		*swapped = FALSE;
	}
}

void waveboxselection_setend(WaveBoxSelection* self, uintptr_t end,
	bool* swapped)
{
	self->end = end;
	if (self->start > self->end) {
		waveboxselection_swap(self);
		if (swapped) {
			*swapped = TRUE;
		}
	} else
		if (swapped) {
			*swapped = FALSE;
		}
}

static uintptr_t waveboxcontext_numloopframes(WaveBoxContext*);
static uintptr_t waveboxcontext_numsustainloopframes(WaveBoxContext*);
static void waveboxcontext_updateoffsetstep(WaveBoxContext*);

void waveboxcontext_init(WaveBoxContext* self, psy_ui_Component* component)
{
	self->component = component;
	self->offsetstep = 1.0;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	self->sample = 0;	
	self->channel = 0;
	waveboxselection_init(&self->selection);	
	psy_ui_size_init_em(&self->size, 10.0, 10.0);		
	psy_dsp_multiresampler_init(&self->resampler,
		psy_dsp_RESAMPLERQUALITY_SPLINE);
}

void waveboxcontext_dispose(WaveBoxContext* self)
{
	self->resampler.resampler.vtable->dispose(&self->resampler.resampler);	
}

void waveboxcontext_setsample(WaveBoxContext* self, psy_audio_Sample* sample,
	uintptr_t channel)
{
	self->sample = sample;	
	self->channel = channel;	
	waveboxcontext_updateoffsetstep(self);
}

void waveboxcontext_setsize(WaveBoxContext* self, const psy_ui_Size* size)
{
	self->size = *size;
	waveboxcontext_updateoffsetstep(self);	
}

void waveboxcontext_setzoom(WaveBoxContext* self, double zoomleft,
	double zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	waveboxcontext_updateoffsetstep(self);
}

void waveboxcontext_setselection(WaveBoxContext* self, uintptr_t selectionstart,
	uintptr_t selectionend)
{
	self->selection.hasselection = TRUE;
	self->selection.start = selectionstart;
	self->selection.end = selectionend;
}

void waveboxcontext_clearselection(WaveBoxContext* self)
{
	self->selection.hasselection = FALSE;
}

double waveboxcontext_frametoscreen(WaveBoxContext* self, uintptr_t frame)
{
	return ((double)frame - ((double)waveboxcontext_numframes(self) *
		self->zoomleft)) / self->offsetstep;	
}

uintptr_t waveboxcontext_realframe(WaveBoxContext* self, uintptr_t frame)
{
	uintptr_t rv = 0;

	if (self->sample) {
		if (self->loopviewmode == WAVEBOX_LOOPVIEW_CONT_DOUBLE) {
			if (self->sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (frame <= self->sample->loop.end) {
					return frame;
				} else {
					return frame - waveboxcontext_numloopframes(self);
				}
			}
		} else
			if (self->loopviewmode == WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE) {
				if (self->sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
					if (frame <= self->sample->sustainloop.end) {
						return frame;
					} else {
						return frame - waveboxcontext_numsustainloopframes(self);
					}
				}
			}
	}
	return frame;
}

uintptr_t waveboxcontext_numframes(WaveBoxContext* self)
{
	if (self->sample) {
		if (self->loopviewmode == WAVEBOX_LOOPVIEW_CONT_DOUBLE) {
			return self->sample->numframes + waveboxcontext_numloopframes(self);
		} else
			if (self->loopviewmode == WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE) {
				return self->sample->numframes + waveboxcontext_numsustainloopframes(self);
			} else {
				return self->sample->numframes;
			}
	}
	return 0;
}

uintptr_t waveboxcontext_numloopframes(WaveBoxContext* self)
{
	if (self->sample && self->sample->loop.type !=
			psy_audio_SAMPLE_LOOP_DO_NOT) {
		return self->sample->loop.end - self->sample->loop.start + 1;
	}
	return 0;
}

uintptr_t waveboxcontext_numsustainloopframes(WaveBoxContext* self)
{
	if (self->sample && self->sample->sustainloop.type !=
			psy_audio_SAMPLE_LOOP_DO_NOT) {
		return self->sample->sustainloop.end - self->sample->sustainloop.start + 1;
	}
	return 0;
}

void waveboxcontext_updateoffsetstep(WaveBoxContext* self)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(self->component);
	if (self->zoomright - self->zoomleft >= 0) {
		self->offsetstep = (double)waveboxcontext_numframes(self) /
			psy_ui_value_px(&self->size.width, tm) *
			(self->zoomright - self->zoomleft);
	} else {
		self->offsetstep = 1.f;
	}
}

psy_dsp_amp_t waveboxcontext_frame_at(WaveBoxContext* self, float frame)
{
	if (self->sample && self->sample->numframes > 0) {				
		return psy_dsp_resampler_work_float(
			&self->resampler.resampler,
			// ptr to sample data absolute start (frame 0)
			self->sample->channels.samples[self->channel], 
			frame, // offset in float
			// bound checks
			self->sample->numframes,
			// ptr to loop begin data
			self->sample->channels.samples[self->channel],
			// ptr to loop end data
			self->sample->channels.samples[self->channel] +
				(self->sample->numframes - 1));
	}
	return 0.f;
}


static void wavebox_updatetext(WaveBox*);
static void wavebox_onlanguagechanged(WaveBox*, psy_ui_Component* sender);
static void wavebox_ondraw(WaveBox*, psy_ui_Graphics*);
static void wavebox_ondestroy(WaveBox*, psy_ui_Component* sender);
static void wavebox_onmousedown(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmousemove(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmouseup(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmousedoubleclick(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static int wavebox_hittest(WaveBox*, uintptr_t frame, double x, int epsilon);
static psy_ui_RealRectangle wavebox_framerangetoscreen(WaveBox*, uintptr_t framebegin,
	uintptr_t frameend);
static int wavebox_hittest_range(WaveBox*, uintptr_t framemin, uintptr_t framemax, 
	double x);
static uintptr_t wavebox_screentoframe(WaveBox*, double x);
static double wavebox_frametoscreen(WaveBox*, uintptr_t frame);
static void  wavebox_onsize(WaveBox*, psy_ui_Size*);
static psy_dsp_amp_t wavebox_amp(WaveBox*, float frame);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(WaveBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondraw = (psy_ui_fp_component_ondraw) wavebox_ondraw;
		vtable.onsize = (psy_ui_fp_component_onsize) wavebox_onsize;
		vtable_initialized = 1;
	}
}

void wavebox_init(WaveBox* self, psy_ui_Component* parent, Workspace* workspace)
{			
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_preventalign(&self->component);
	psy_ui_component_doublebuffer(&self->component);	
	self->nowavetext = strdup("");
	self->context.loopviewmode = WAVEBOX_LOOPVIEW_CONT_SINGLE;
	self->preventdrawonselect = FALSE;
	self->dragstarted = FALSE;
	self->drawline = FALSE;
	waveboxcontext_init(&self->context, &self->component);
	psy_signal_init(&self->selectionchanged);
	psy_signal_connect(&self->component.signal_destroy, self, wavebox_ondestroy);
	psy_signal_connect(&self->component.signal_mousedown, self,
		wavebox_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		wavebox_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		wavebox_onmouseup);
	psy_signal_connect(&self->component.signal_mousedoubleclick, self,
		wavebox_onmousedoubleclick);
	psy_signal_connect(&self->component.signal_languagechanged, self,
		wavebox_onlanguagechanged);
	wavebox_updatetext(self);
}

void wavebox_ondestroy(WaveBox* self, psy_ui_Component* sender)
{	
	waveboxcontext_dispose(&self->context);
	psy_signal_dispose(&self->selectionchanged);
	free(self->nowavetext);
}

void wavebox_updatetext(WaveBox* self)
{
	wavebox_setnowavetext(self, psy_ui_translate(
		"samplesview.no-wave-loaded"));
}

void wavebox_onlanguagechanged(WaveBox* self, psy_ui_Component* sender)
{
	wavebox_updatetext(self);
}

WaveBox* wavebox_alloc(void)
{
	return (WaveBox*)malloc(sizeof(WaveBox));
}

WaveBox* wavebox_allocinit(psy_ui_Component* parent, Workspace* workspace)
{
	WaveBox* rv;

	rv = wavebox_alloc();
	if (rv) {
		wavebox_init(rv, parent, workspace);
	}
	return rv;
}

void wavebox_setnowavetext(WaveBox* self, const char* text)
{
	if (text != self->nowavetext) {
		free(self->nowavetext);
		self->nowavetext = strdup(text);
	}
}

void wavebox_setsample(WaveBox* self, psy_audio_Sample* sample,
	uintptr_t channel)
{		
	waveboxcontext_setsample(&self->context, sample, channel);
	psy_ui_component_invalidate(&self->component);
}

void wavebox_ondraw(WaveBox* self, psy_ui_Graphics* g)
{	
	psy_ui_RealRectangle r;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	uintptr_t prevframe = 0;
	psy_dsp_amp_t lastframevalue = 0;
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_innersize_px(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
	if (!self->context.sample) {		
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
		psy_ui_textout(g,
			(size.width - tm->tmAveCharWidth * psy_strlen(self->nowavetext)) / 2,
			(size.height - tm->tmHeight) / 2,
			self->nowavetext, psy_strlen(self->nowavetext));
	} else {
		double scaley;		
		double x;
		double centery = size.height / 2;
		psy_ui_RealRectangle cont_loop_rc;
		psy_ui_RealRectangle cont_doubleloop_rc;
		psy_ui_RealRectangle sustain_loop_rc;		
		psy_ui_RealRectangle sustain_doubleloop_rc;
		bool firstselstart = TRUE;

		if (self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			cont_loop_rc = wavebox_framerangetoscreen(self,
				self->context.sample->loop.start,
				self->context.sample->loop.end);
			if (self->context.loopviewmode == WAVEBOX_LOOPVIEW_CONT_DOUBLE) {
				cont_doubleloop_rc =
					wavebox_framerangetoscreen(self,
						self->context.sample->loop.end + 1,
						self->context.sample->loop.end + 1 +
							waveboxcontext_numloopframes(&self->context));
			}			
		}
		if (self->context.sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			sustain_loop_rc = wavebox_framerangetoscreen(self,
				self->context.sample->sustainloop.start,
				self->context.sample->sustainloop.end);
			if (self->context.loopviewmode == WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE) {
				sustain_doubleloop_rc =
					wavebox_framerangetoscreen(self,
						self->context.sample->sustainloop.end + 1,
						self->context.sample->sustainloop.end + 1 +
						waveboxcontext_numsustainloopframes(&self->context));
			}
		}
		scaley = (size.height / 2) / (psy_dsp_amp_t)32768;
		if (self->context.sample && self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_drawsolidrectangle(g, cont_loop_rc, psy_ui_colour_make(0x00303030));
			if (self->context.loopviewmode == WAVEBOX_LOOPVIEW_CONT_DOUBLE) {
				psy_ui_drawsolidrectangle(g, cont_doubleloop_rc, psy_ui_colour_make(0x00292929));
			}
		}
		if (self->context.sample && self->context.sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_drawsolidrectangle(g, sustain_loop_rc, psy_ui_colour_make(0x00444444));
			if (self->context.loopviewmode == WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE) {
				psy_ui_drawsolidrectangle(g, sustain_doubleloop_rc, psy_ui_colour_make(0x00292929));
			}
		}
		if (self->context.selection.hasselection) {
			psy_ui_drawsolidrectangle(g, wavebox_framerangetoscreen(self,
				self->context.selection.end, self->context.selection.end), psy_ui_colour_make(0x00B1C8B0));
		}	
		if (self->drawline) {
			if (g->clip.left > 0) {
				prevframe = (int)(self->context.offsetstep * (g->clip.left - 1) +
					(int)((float)(waveboxcontext_numframes(&self->context)) * self->context.zoomleft));
			} else {
				prevframe = 0;
			}
			lastframevalue = wavebox_amp(self, (float)prevframe);
		}
		for (x = g->clip.left; x < g->clip.right; ++x) {
			float frame;
			uintptr_t realframe;
			psy_dsp_amp_t framevalue;
			psy_ui_RealRectangle r;

			frame = (float)((self->context.offsetstep * x +
				((double)waveboxcontext_numframes(&self->context) * self->context.zoomleft)));
			r.left = x;
			r.right = x+1;
			realframe = waveboxcontext_realframe(&self->context, (uintptr_t)frame);
			if (frame >= waveboxcontext_numframes(&self->context)) {
				break;
			}
			framevalue = wavebox_amp(self, frame);			
			if (self->context.selection.hasselection &&
				realframe >= self->context.selection.start &&
				realframe <= self->context.selection.end) {
				if (realframe == self->context.selection.start ||
					realframe == (self->context.selection.end)) {
					psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
				} else {
					psy_ui_setcolour(g, psy_ui_colour_make(0x00262626));
				}
				r.top = 0;
				r.bottom = size.height;
				if (firstselstart || self->dragstarted == FALSE) {
					psy_ui_drawrectangle(g, r);
					firstselstart = FALSE;
					psy_ui_setcolour(g, psy_ui_colour_make(0x00FF2288));
				} else {
					psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
				}
			} else
			if (self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT &&
					psy_ui_realrectangle_intersect(&cont_loop_rc,
						psy_ui_realpoint_make(x, 0))) {
				psy_ui_setcolour(g, psy_ui_colour_make(0x00D1C5B6));
			} else {
				psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
			}
			r.top = centery - (int)(framevalue * scaley);
			r.bottom = centery;
			if (self->drawline) {
				psy_ui_drawline(g,
					psy_ui_realpoint_make(r.left - 1, centery - (int)(lastframevalue * scaley)),
					psy_ui_realpoint_make(r.left, r.top));
				lastframevalue = framevalue;
			} else {
				if (r.top > r.bottom) {
					double temp;
					temp = r.top;
					r.top = r.bottom;
					r.bottom = temp;
				}
				psy_ui_drawrectangle(g, r); // x, centery, x, centery - (int)(framevalue * scaley));
			}
			
			if (frame >= waveboxcontext_numframes(&self->context)) {
				break;
			}
		}
		if (self->context.sample && self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_setcolour(g, psy_ui_colour_make(0x00D1C5B6));
			psy_ui_drawline(g, psy_ui_realpoint_make(cont_loop_rc.left, 0),
				psy_ui_realpoint_make(cont_loop_rc.left + 1, size.height));
			psy_ui_drawline(g, psy_ui_realpoint_make(cont_loop_rc.right, 0),
				psy_ui_realpoint_make(cont_loop_rc.right + 1, size.height));
		}
		if (self->context.sample && self->context.sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_setcolour(g, psy_ui_colour_make(0x00B6C5D1));
			psy_ui_drawline(g, psy_ui_realpoint_make(sustain_loop_rc.left, 0),
				psy_ui_realpoint_make(sustain_loop_rc.left + 1, size.height));
			psy_ui_drawline(g, psy_ui_realpoint_make(sustain_loop_rc.right, 0),
				psy_ui_realpoint_make(sustain_loop_rc.right + 1, size.height));
		}
	}
}

psy_ui_RealRectangle wavebox_framerangetoscreen(WaveBox* self, uintptr_t framebegin,
	uintptr_t frameend)
{	
	double startx;
	double endx;
	psy_ui_RealSize size;	
		
	startx = wavebox_frametoscreen(self, framebegin);	
	endx = wavebox_frametoscreen(self, frameend);	
	size = psy_ui_component_innersize_px(&self->component);
	return psy_ui_realrectangle_make(psy_ui_realpoint_make(startx, 0.0),
		psy_ui_realsize_make(endx - startx, size.height));
}

void wavebox_onmousedown(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	psy_ui_component_capture(&self->component);
	self->dragstarted = TRUE;	
	if (self->context.sample && waveboxcontext_numframes(&self->context) > 0) {
		if (self->context.selection.hasselection) {
			if (wavebox_hittest(self, self->context.selection.start, ev->pt.x, 5)) {
				self->dragstarted = FALSE;
				self->dragmode = WAVEBOX_DRAG_LEFT;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else if (wavebox_hittest(self, self->context.selection.end, ev->pt.x, 5)) {
				self->dragstarted = FALSE;
				self->dragmode = WAVEBOX_DRAG_RIGHT;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else if (wavebox_hittest_range(self, self->context.selection.start,
				self->context.selection.end, ev->pt.x)) {
				self->dragstarted = FALSE;
				self->dragmode = WAVEBOX_DRAG_MOVE;
				self->dragoffset = wavebox_screentoframe(self, ev->pt.x)
					- self->context.selection.start;					
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_MOVE);
			} else {
				self->context.selection.hasselection = FALSE;
				psy_ui_component_invalidate(&self->component);
			}
		}		
		if (!self->context.selection.hasselection) {
			if (self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (wavebox_hittest(self, self->context.sample->loop.start, ev->pt.x, 5)) {
					self->dragmode = WAVEBOX_DRAG_LOOP_CONT_LEFT;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
					return;
				} else
					if (wavebox_hittest(self, self->context.sample->loop.end, ev->pt.x, 5)) {
						self->dragmode = WAVEBOX_DRAG_LOOP_CONT_RIGHT;
						psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
						return;
					}
			}
			if (self->context.sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (wavebox_hittest(self, self->context.sample->sustainloop.start, ev->pt.x, 5)) {
					self->dragmode = WAVEBOX_DRAG_LOOP_SUSTAIN_LEFT;
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
					return;
				} else
					if (wavebox_hittest(self, self->context.sample->sustainloop.end, ev->pt.x, 5)) {
						self->dragmode = WAVEBOX_DRAG_LOOP_SUSTAIN_RIGHT;
						psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
						return;
					}
			}
			self->context.selection.hasselection = TRUE;
			self->context.selection.start = wavebox_screentoframe(self, ev->pt.x);
			self->context.selection.end = self->context.selection.start;
			self->dragmode = WAVEBOX_DRAG_RIGHT;
			psy_ui_component_invalidate(&self->component);
		}		
	}
}

void wavebox_onmousemove(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->context.sample && waveboxcontext_numframes(&self->context) > 0) {
		uintptr_t frame;
		uintptr_t realframe;
		bool changed;
		bool swapped;
		int olddragmode;

		self->dragstarted = FALSE;
		changed = FALSE;
		frame = wavebox_screentoframe(self, ev->pt.x);
		realframe = waveboxcontext_realframe(&self->context, frame);
		self->context.oldselection = self->context.selection;		
		olddragmode = self->dragmode;
		if (self->dragmode == WAVEBOX_DRAG_LEFT) {	
			waveboxselection_setstart(&self->context.selection, realframe, &swapped);
			if (swapped) {
				self->dragmode = WAVEBOX_DRAG_RIGHT;
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_RIGHT) {			
			waveboxselection_setend(&self->context.selection, realframe, &swapped);
			if (swapped) {
				self->dragmode = WAVEBOX_DRAG_LEFT;
			}
			changed = TRUE;			
		} else 
		if (self->dragmode == WAVEBOX_DRAG_MOVE) {
			uintptr_t length;
			intptr_t start;

			length = self->context.selection.end - self->context.selection.start;
			start = realframe - self->dragoffset;
			if (start < 0) {
				start = 0;
			}
			waveboxselection_setrange(&self->context.selection, start, start + length);			
			if (self->context.selection.end >= waveboxcontext_numframes(&self->context)) {
				self->context.selection.end = waveboxcontext_numframes(&self->context) - 1;
				self->context.selection.start = waveboxcontext_numframes(&self->context) - 1 - length;
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_LOOP_CONT_LEFT) {
			if (frame > self->context.sample->loop.end) {
				self->dragmode = WAVEBOX_DRAG_LOOP_CONT_RIGHT;
				psy_audio_sample_setcontloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->loop.type,
						self->context.sample->loop.end, frame));
			} else {
				psy_audio_sample_setcontloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->loop.type,
						frame, self->context.sample->loop.end));
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_LOOP_CONT_RIGHT) {
			if (frame < self->context.sample->loop.start) {
				psy_audio_sample_setcontloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->loop.type,
						frame, self->context.sample->loop.start));
				self->dragmode = WAVEBOX_DRAG_LOOP_CONT_LEFT;
			} else {
				psy_audio_sample_setcontloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->loop.type,
						self->context.sample->loop.start, frame));
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_LOOP_SUSTAIN_LEFT) {												
			if (frame > self->context.sample->sustainloop.end) {
				self->dragmode = WAVEBOX_DRAG_LOOP_SUSTAIN_RIGHT;
				psy_audio_sample_setsustainloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->sustainloop.type,
						self->context.sample->sustainloop.end,
						frame));
			} else {
				psy_audio_sample_setsustainloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->sustainloop.type,
						frame,
						self->context.sample->sustainloop.end));
			}
			changed = TRUE;			
		} else
		if (self->dragmode == WAVEBOX_DRAG_LOOP_SUSTAIN_RIGHT) {			
			if (frame < self->context.sample->sustainloop.start) {
				psy_audio_sample_setsustainloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->sustainloop.type,
						frame, self->context.sample->sustainloop.start));
				self->dragmode = WAVEBOX_DRAG_LOOP_SUSTAIN_LEFT;
			} else {
				psy_audio_sample_setsustainloop(self->context.sample,
					psy_audio_sampleloop_make(
						self->context.sample->sustainloop.type,
						self->context.sample->sustainloop.start,
						frame));
			}
			changed = TRUE;			
		} else
		if (self->context.selection.hasselection &&
			(wavebox_hittest(self, self->context.selection.start, ev->pt.x, 5) ||
				wavebox_hittest(self, self->context.selection.end, ev->pt.x, 5))) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else
		if (self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT &&
			(wavebox_hittest(self, self->context.sample->loop.start, ev->pt.x, 5) ||
				wavebox_hittest(self, self->context.sample->loop.end, ev->pt.x, 5))) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else
		if (self->context.sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT &&
			(wavebox_hittest(self, self->context.sample->sustainloop.start, ev->pt.x, 5) ||
				wavebox_hittest(self, self->context.sample->sustainloop.end, ev->pt.x, 5))) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else
		if (self->context.selection.hasselection &&
				wavebox_hittest_range(self, self->context.selection.start,
					self->context.selection.end, ev->pt.x)) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_MOVE);
		}
		if (changed) {
			if (self->dragmode == WAVEBOX_DRAG_MOVE) {
				psy_ui_component_setcursor(&self->component,
					psy_ui_CURSORSTYLE_MOVE);
			} else {
				psy_ui_component_setcursor(&self->component,
					psy_ui_CURSORSTYLE_COL_RESIZE);
			}
			if (!self->preventdrawonselect) {
				psy_ui_RealRectangle rold;
				psy_ui_RealRectangle rnew;
				
				if (olddragmode == WAVEBOX_DRAG_RIGHT &&
						self->dragmode == WAVEBOX_DRAG_RIGHT) {
					if (self->context.oldselection.end < self->context.selection.end) {
						rnew = wavebox_framerangetoscreen(self,
							self->context.oldselection.end,
							self->context.selection.end);
						psy_ui_component_invalidaterect(&self->component, rnew);
					} else {
						rnew = wavebox_framerangetoscreen(self,
							self->context.selection.end,
							self->context.oldselection.end);
						psy_ui_realrectangle_expand(&rnew, 0, 2, 0, 2);
						psy_ui_component_invalidaterect(&self->component, rnew);
					}												
				} else
				if (olddragmode == WAVEBOX_DRAG_LEFT &&
					self->dragmode == WAVEBOX_DRAG_LEFT) {
					if (self->context.oldselection.start < self->context.selection.start) {
						rnew = wavebox_framerangetoscreen(self,
							self->context.oldselection.start,
							self->context.selection.start);
						psy_ui_component_invalidaterect(&self->component, rnew);
					} else {
						rnew = wavebox_framerangetoscreen(self,
							self->context.selection.start,
							self->context.oldselection.start);
						psy_ui_realrectangle_expand(&rnew, 0, 2, 0, 2);
						psy_ui_component_invalidaterect(&self->component, rnew);
					}
				} else {
					rold = wavebox_framerangetoscreen(self,
						self->context.oldselection.start,
						self->context.oldselection.end);
					rnew = wavebox_framerangetoscreen(self,
						self->context.selection.start,
						self->context.selection.end);
					psy_ui_realrectangle_expand(&rold, 0, 2, 0, 2);					
					psy_ui_realrectangle_union(&rnew, &rold);
					psy_ui_component_invalidaterect(&self->component, rnew);
				}
			}
			psy_signal_emit(&self->selectionchanged, self, 0);
		}
	}
}

int wavebox_hittest(WaveBox* self, uintptr_t frame, double x, int epsilon)
{		
	return frame >= wavebox_screentoframe(self, x - epsilon) &&
		   frame <= wavebox_screentoframe(self, x + epsilon);
}

int wavebox_hittest_range(WaveBox* self, uintptr_t framemin,
	uintptr_t framemax, double x)
{	
	uintptr_t frame = wavebox_screentoframe(self, x);
	return frame >= framemin && frame <= framemax;
}

uintptr_t wavebox_screentoframe(WaveBox* self, double x)
{
	uintptr_t rv = 0;
	intptr_t frame;

	if (self->context.sample) {
		psy_ui_Size size;		
		
		size = psy_ui_component_offsetsize(&self->component);			
		frame = (int)(self->context.offsetstep * x) +
			(int)(waveboxcontext_numframes(&self->context) * self->context.zoomleft);
		if (frame < 0) {
			frame = 0;
		} else
		if (frame >= (intptr_t)waveboxcontext_numframes(&self->context)) {
			if (waveboxcontext_numframes(&self->context) > 0) {
				frame = waveboxcontext_numframes(&self->context) - 1;
			} else {
				frame = 0;
			}
		}
		rv = frame;
	}
	return rv;
}

double wavebox_frametoscreen(WaveBox* self, uintptr_t frame)
{
	return waveboxcontext_frametoscreen(&self->context, frame);
}

static psy_dsp_amp_t wavebox_amp(WaveBox* self, float frame)
{
	if (self->context.sample) {
		if (self->context.loopviewmode == WAVEBOX_LOOPVIEW_CONT_DOUBLE) {
			if (self->context.sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (frame <= self->context.sample->loop.end) {
					return waveboxcontext_frame_at(&self->context, frame);
				} else {
					return waveboxcontext_frame_at(&self->context,
						frame - waveboxcontext_numloopframes(&self->context));
				}
			} 
		} else
		if (self->context.loopviewmode == WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE) {
			if (self->context.sample->sustainloop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				if (frame <= self->context.sample->sustainloop.end) {
					return waveboxcontext_frame_at(&self->context, frame);
				} else {
					return waveboxcontext_frame_at(&self->context,						
						frame - waveboxcontext_numsustainloopframes(&self->context));
				}
			}
		}
		return waveboxcontext_frame_at(&self->context, frame);
	}
	return 0.f;
}

void wavebox_onmouseup(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if (waveboxcontext_sample(&self->context) && waveboxcontext_numframes(&self->context) > 0) {		
		self->dragmode = WAVEBOX_DRAG_NONE;
		if (self->dragstarted != FALSE) {
			self->context.selection.hasselection = FALSE;
			psy_ui_component_invalidate(&self->component);
			psy_signal_emit(&self->selectionchanged, self, 0);
		}	
	}
	self->dragstarted = FALSE;
}

static void wavebox_onmousedoubleclick(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (waveboxcontext_sample(&self->context) && waveboxcontext_numframes(&self->context) > 0) {
		if (self->context.selection.hasselection) {
			self->context.selection.hasselection = FALSE;
		} else {
			self->context.selection.hasselection = TRUE;
			self->context.selection.start = 0;
			self->context.selection.end = waveboxcontext_numframes(&self->context) - 1;
		}
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->selectionchanged, self, 0);
	}
}

void wavebox_setzoom(WaveBox* self, double zoomleft, double zoomright)
{
	waveboxcontext_setzoom(&self->context, zoomleft, zoomright);		
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_update(&self->component);	
}

void wavebox_setselection(WaveBox* self, uintptr_t selectionstart,
	uintptr_t selectionend)
{
	waveboxcontext_setselection(&self->context, selectionstart, selectionend);
}

void wavebox_clearselection(WaveBox* self)
{
	waveboxcontext_clearselection(&self->context);
}

void wavebox_refresh(WaveBox* self)
{
	waveboxcontext_updateoffsetstep(&self->context);
}

void  wavebox_onsize(WaveBox* self, psy_ui_Size* size)
{	
	waveboxcontext_setsize(&self->context, size);	
}

void wavebox_setquality(WaveBox* self, psy_dsp_ResamplerQuality quality)
{
	psy_dsp_multiresampler_setquality(&self->context.resampler, quality);
	psy_ui_component_invalidate(&self->component);
}
