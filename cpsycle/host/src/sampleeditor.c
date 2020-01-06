// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampleeditor.h"

#include <operations.h>
#include <alignedalloc.h>
#include <exclusivelock.h>

#include <portable.h>

static void sampleeditorplaybar_initalign(SampleEditorPlayBar*);
static void sampleeditorheader_init(SampleEditorHeader*, psy_ui_Component* parent,
	SampleEditor*);
static void sampleeditorheader_ondraw(SampleEditorHeader*, psy_ui_Component* sender, psy_ui_Graphics*);
static void sampleeditorheader_drawruler(SampleEditorHeader*, psy_ui_Graphics*);
static void samplezoom_ondraw(SampleZoom*, psy_ui_Component* sender, psy_ui_Graphics*);
static void samplezoom_drawsamples(SampleZoom*, psy_ui_Graphics*);

static void sampleeditor_initsampler(SampleEditor*);
static void sampleeditor_ondestroy(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onsize(SampleEditor*, psy_ui_Component* sender, ui_size*);
static void sampleeditor_onzoom(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_computemetrics(SampleEditor*, SampleEditorMetrics* rv);
static void sampleeditor_onsongchanged(SampleEditor*, Workspace* workspace);
static void sampleeditor_connectmachinessignals(SampleEditor*, Workspace*);
static void sampleeditor_onplay(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onstop(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onmasterworked(SampleEditor*, psy_audio_Machine*,
	unsigned int slot, psy_audio_BufferContext*);

static void samplezoom_ondestroy(SampleZoom*, psy_ui_Component* sender);
static void samplezoom_onmousedown(SampleZoom*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void samplezoom_onmouseup(SampleZoom*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void samplezoom_onmousemove(SampleZoom*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);

enum {
	SAMPLEEDITOR_DRAG_NONE,
	SAMPLEEDITOR_DRAG_LEFT,
	SAMPLEEDITOR_DRAG_RIGHT,
	SAMPLEEDITOR_DRAG_MOVE
};

void sampleeditorplaybar_init(SampleEditorPlayBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	// ui_button_init(&self->loop, &self->component);
	// ui_button_settext(&self->loop, "Loop");	
	// psy_signal_connect(&self->loop.signal_clicked, self, onloopclicked);	
	ui_button_init(&self->play, &self->component);
	ui_button_settext(&self->play, workspace_translate(workspace, "play"));	
	ui_button_init(&self->stop, &self->component);
	ui_button_settext(&self->stop, workspace_translate(workspace, "stop"));
	// psy_signal_connect(&self->stop.signal_clicked, self, onstopclicked);	
	sampleeditorplaybar_initalign(self);	
}

void sampleeditorplaybar_initalign(SampleEditorPlayBar* self)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
}

void sampleeditorheader_init(SampleEditorHeader* self, psy_ui_Component* parent,
	SampleEditor* view)
{
	self->view = view;
	self->scrollpos = 0;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_resize(&self->component, 100, 50);
	psy_signal_connect(&self->component.signal_draw, self,
		sampleeditorheader_ondraw);
}

void sampleeditorheader_ondraw(SampleEditorHeader* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	sampleeditorheader_drawruler(self, g);	
}

void sampleeditorheader_drawruler(SampleEditorHeader* self, psy_ui_Graphics* g)
{
	ui_size size;	
	int baseline;		
	psy_dsp_beat_t cpx;	
	int c;	

	size = ui_component_size(&self->component);	
	baseline = size.height - 1;
	ui_setcolor(g, 0x00CACACA); 
	ui_drawline(g, 0, baseline, size.width, baseline);	
	ui_setbackgroundmode(g, TRANSPARENT);
	ui_settextcolor(g, 0x00CACACA);
	for (c = 0, cpx = 0; c <= self->view->metrics.visisteps; 
			cpx += self->view->metrics.stepwidth, ++c) {
		char txt[40];
		int frame;
		
		ui_drawline(g, (int) cpx, baseline, (int) cpx, baseline - 4);
		frame = (int)((c - self->scrollpos) * 
			(self->view->sample
				? (self->view->sample->numframes / self->view->metrics.visisteps)
				: 0));
		psy_snprintf(txt, 40, "%d", frame);
		ui_textout(g, (int) cpx + 3, baseline - 14, txt, strlen(txt));
	}
}

void samplezoom_init(SampleZoom* self, psy_ui_Component* parent)
{
	self->sample = 0;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;	
	self->dragmode = SAMPLEEDITOR_DRAG_NONE;
	self->dragoffset = 00;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	psy_signal_init(&self->signal_zoom);
	psy_signal_connect(&self->component.signal_destroy, self,
		samplezoom_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self,
		samplezoom_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		samplezoom_onmousedown);
	psy_signal_connect(&self->component.signal_mouseup, self,
		samplezoom_onmouseup);
	psy_signal_connect(&self->component.signal_mousemove, self,
		samplezoom_onmousemove);
	ui_component_resize(&self->component, 100, 50);
}

void samplezoom_ondestroy(SampleZoom* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_zoom);
}

void samplezoom_ondraw(SampleZoom* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	ui_rectangle r;
	ui_size size;
	int zoomleftx;
	int zoomrightx;

	size = ui_component_size(&self->component);
	zoomleftx = (int)(size.width * self->zoomleft);
	zoomrightx = (int)(size.width * self->zoomright);
	if (zoomleftx == zoomrightx) {
		++zoomrightx;
	}
	ui_setcolor(g, 0x00B1C8B0);
	ui_setrectangle(&r, zoomleftx, 0, zoomrightx - zoomleftx, size.height);
	ui_drawrectangle(g, r);
	if (self->sample) {
		samplezoom_drawsamples(self, g);
	}
}

void samplezoom_drawsamples(SampleZoom* self, psy_ui_Graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setcolor(g, 0x00B1C8B0);
	if (!self->sample) {
		ui_textmetric tm;
		static const char* txt = "No wave loaded";

		tm = ui_component_textmetric(&self->component);
		ui_setbackgroundmode(g, TRANSPARENT);
		ui_settextcolor(g, 0x00D1C5B6);
		ui_textout(g, (size.width - tm.tmAveCharWidth * strlen(txt)) / 2,
			(size.height - tm.tmHeight) / 2, txt, strlen(txt));
	} else {
		int x;
		int centery = size.height / 2;
		float offsetstep;
		psy_dsp_amp_t scaley;

		scaley = (size.height / 2) / (psy_dsp_amp_t)32768;		
		offsetstep = (float) self->sample->numframes / size.width;
		ui_setcolor(g, 0x00B1C8B0);
		for (x = 0; x < size.width; ++x) {			
			uintptr_t frame = (int)(offsetstep * x);
			float framevalue;
			
			if (frame >= self->sample->numframes) {
				break;
			}
			framevalue = self->sample->channels.samples[0][frame];							
			ui_drawline(g, x, centery, x, centery + (int)(framevalue * scaley));
		}
	}
}

void samplezoom_onmousedown(SampleZoom* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	ui_size size;
	int zoomleftx;
	int zoomrightx;

	size = ui_component_size(&self->component);
	zoomleftx = (int)(size.width * self->zoomleft);
	if (ev->x >= zoomleftx - 5 && ev->x < zoomleftx + 5) {
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		self->dragmode = SAMPLEEDITOR_DRAG_LEFT;
		self->dragoffset = ev->x - zoomleftx;		
	} else {
		zoomrightx = (int)(size.width * self->zoomright);
		if (ev->x >= zoomrightx - 5 && ev->x < zoomrightx + 5) {
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			self->dragmode = SAMPLEEDITOR_DRAG_RIGHT;
			self->dragoffset = ev->x - zoomrightx;
		} else
		if (ev->x > zoomleftx && ev->x < zoomrightx) {
			self->dragmode = SAMPLEEDITOR_DRAG_MOVE;
			self->dragoffset = ev->x - zoomleftx;
		}
	}
	ui_component_capture(&self->component);
}

void samplezoom_onmousemove(SampleZoom* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	ui_size size;	

	size = ui_component_size(&self->component);
	if (self->dragmode == SAMPLEEDITOR_DRAG_NONE) {
		int zoomleftx;
		int zoomrightx;

		zoomleftx = (int)(size.width * self->zoomleft);
		if (ev->x >= zoomleftx - 5 && ev->x < zoomleftx + 5) {
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		} else {
			zoomrightx = (int)(size.width * self->zoomright);
			if (ev->x >= zoomrightx - 5 && ev->x < zoomrightx + 5) {
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			}
		}
	} else 
	if (self->dragmode == SAMPLEEDITOR_DRAG_LEFT) {
		float zoomold;

		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		zoomold = self->zoomleft;
		self->zoomleft = (ev->x - self->dragoffset) / (float) size.width;
		if (self->zoomleft > self->zoomright) {
			self->zoomleft = self->zoomright;
		} else
		if (self->zoomleft < 0.f) {
			self->zoomleft = 0.f;
		} else
		if (self->zoomleft > 1.f) {
			self->zoomleft = 1.f;
		}
		if (zoomold != self->zoomright) {
			ui_component_invalidate(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else
	if (self->dragmode == SAMPLEEDITOR_DRAG_RIGHT) {
		float zoomold;

		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		zoomold = self->zoomright;
		self->zoomright = (ev->x - self->dragoffset) / (float) size.width;
		if (self->zoomright < self->zoomleft) {
			self->zoomright = self->zoomleft;
		} else
		if (self->zoomright < 0.f) {
			self->zoomright = 0.f;
		} else
		if (self->zoomright > 1.f) {
			self->zoomright = 1.f;
		}
		if (zoomold != self->zoomright) {
			ui_component_invalidate(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else
	if (self->dragmode == SAMPLEEDITOR_DRAG_MOVE) {
		float zoomold;
		float length;
		
		zoomold = self->zoomleft;
		length = self->zoomright - self->zoomleft;
		self->zoomleft = (ev->x - self->dragoffset) / (float) size.width;
		if (self->zoomleft < 0.f) {
			self->zoomleft = 0.f;			
		}
		if (self->zoomleft + length > 1.f) {
			self->zoomleft = 1.f - length;
			if (self->zoomleft < 0.f) {
				self->zoomleft = 0.f;			
			}
		}
		if (self->zoomleft != zoomold) {			
			self->zoomright = self->zoomleft + length;
			ui_component_invalidate(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	}
}

void samplezoom_onmouseup(SampleZoom* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	self->dragmode = SAMPLEEDITOR_DRAG_NONE;
	ui_component_releasecapture(&self->component);
}

void sampleeditor_init(SampleEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{						
	self->sample = 0;
	self->samplerevents = 0;
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self, sampleeditor_ondestroy);
	ui_component_enablealign(&self->component);	
	sampleeditorplaybar_init(&self->playbar, &self->component, workspace);
	psy_signal_connect(&self->playbar.play.signal_clicked, self, sampleeditor_onplay);
	psy_signal_connect(&self->playbar.stop.signal_clicked, self, sampleeditor_onstop);
	ui_component_setalign(&self->playbar.component, UI_ALIGN_TOP);
	sampleeditorheader_init(&self->header, &self->component, self);
	ui_component_setalign(&self->header.component, UI_ALIGN_TOP);
	wavebox_init(&self->samplebox, &self->component);
	ui_component_setalign(&self->samplebox.component, UI_ALIGN_CLIENT);	
	samplezoom_init(&self->zoom, &self->component);
	ui_component_setalign(&self->zoom.component, UI_ALIGN_BOTTOM);
	sampleeditor_computemetrics(self, &self->metrics);
	psy_signal_connect(&self->zoom.signal_zoom, self, sampleeditor_onzoom);
	psy_signal_connect(&workspace->signal_songchanged, self,
		sampleeditor_onsongchanged);
	sampleeditor_initsampler(self);
	sampleeditor_connectmachinessignals(self, workspace);	
}

void sampleeditor_initsampler(SampleEditor* self)
{
	uintptr_t c;

	sampler_init(&self->sampler,
		self->workspace->machinefactory.machinecallback);
	buffer_init(&self->samplerbuffer, 2);
	for (c = 0; c < self->samplerbuffer.numchannels; ++c) {
		self->samplerbuffer.samples[c] = dsp.memory_alloc(MAX_STREAM_SIZE,
			sizeof(float));
	}	
}

void sampleeditor_ondestroy(SampleEditor* self, psy_ui_Component* sender)
{
	uintptr_t c;

	self->sampler.custommachine.machine.vtable->dispose(
		&self->sampler.custommachine.machine);
	for (c = 0; c < self->samplerbuffer.numchannels; ++c) {
		dsp.memory_dealloc(self->samplerbuffer.samples[c]);
	}
	buffer_dispose(&self->samplerbuffer);	
}

void sampleeditor_setsample(SampleEditor* self, psy_audio_Sample* sample)
{
	self->sample = sample;
	wavebox_setsample(&self->samplebox, sample);
	self->zoom.sample = sample;
	sampleeditor_computemetrics(self, &self->metrics);
	ui_component_invalidate(&self->component);
}

void sampleeditor_onsize(SampleEditor* self, psy_ui_Component* sender, ui_size* size)
{
	sampleeditor_computemetrics(self, &self->metrics);
}

void sampleeditor_onzoom(SampleEditor* self, psy_ui_Component* sender)
{
	wavebox_setzoom(&self->samplebox, self->zoom.zoomleft,
		self->zoom.zoomright);
}

void sampleeditor_computemetrics(SampleEditor* self, SampleEditorMetrics* rv)
{	
	ui_size sampleboxsize;

	sampleboxsize = ui_component_size(&self->samplebox.component);
	rv->samplewidth = self->sample
		? (float) sampleboxsize.width / self->sample->numframes
		: 0;	
	rv->visisteps = 10;	
	rv->stepwidth = self->sample
		? rv->samplewidth * (self->sample->numframes / 10)
		: sampleboxsize.width;	
}

void sampleeditor_onsongchanged(SampleEditor* self, Workspace* workspace)
{
	sampleeditor_connectmachinessignals(self, workspace);
}

void sampleeditor_connectmachinessignals(SampleEditor* self,
	Workspace* workspace)
{
	if (workspace && workspace->song &&
			machines_master(&workspace->song->machines)) {
		psy_signal_connect(
			&machines_master(&workspace->song->machines)->signal_worked, self,
			sampleeditor_onmasterworked);
	}
}

void sampleeditor_onplay(SampleEditor* self, psy_ui_Component* sender)
{	
	if (self->workspace->song && self->sample) {
		psy_audio_PatternEvent event;
		lock_enter();
		psy_list_free(self->samplerevents);
		patternevent_init_all(&event,
			(unsigned char) 48,
			(unsigned char) instruments_slot(&self->workspace->song->instruments),
			NOTECOMMANDS_MACH_EMPTY,
			NOTECOMMANDS_VOL_EMPTY,
			0, 0);	
		patternentry_init_all(&self->samplerentry, &event, 0, 0, 120.f, 0);
		self->samplerevents = psy_list_create(&self->samplerentry);
		lock_leave();
	}
}

void sampleeditor_onstop(SampleEditor* self, psy_ui_Component* sender)
{	
	psy_audio_PatternEvent event;

	lock_enter();
	psy_list_free(self->samplerevents);
	patternevent_init_all(&event,
		NOTECOMMANDS_RELEASE,
		0,		
		NOTECOMMANDS_MACH_EMPTY,
		NOTECOMMANDS_VOL_EMPTY,
		0, 0);	
	self->samplerevents = psy_list_create(&self->samplerentry);
	patternentry_init_all(&self->samplerentry, &event, 0, 0, 120.f, 0);
	lock_leave();
}

void sampleeditor_onmasterworked(SampleEditor* self, psy_audio_Machine* machine,
	unsigned int slot, psy_audio_BufferContext* bc)
{
	psy_audio_BufferContext samplerbc;
		
	buffercontext_init(&samplerbc, self->samplerevents, 0,
		&self->samplerbuffer, bc->numsamples, 16, 0);
	buffer_clearsamples(&self->samplerbuffer, bc->numsamples);
	machine_work(&self->sampler.custommachine.machine, &samplerbc);
	buffer_addsamples(bc->output, &self->samplerbuffer, bc->numsamples, 
		(psy_dsp_amp_t) 1.f);
	if (self->samplerevents) {
		patternentry_dispose(&self->samplerentry);
	}
	psy_list_free(self->samplerevents);
	self->samplerevents = 0;
	
}
