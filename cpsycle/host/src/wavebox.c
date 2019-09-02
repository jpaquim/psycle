#include "wavebox.h"

static void OnDraw(WaveBox* self, ui_component* sender, ui_graphics* g);
static void OnSize(WaveBox* self, ui_component* sender, int width, int height);
static void OnDestroy(WaveBox* self, ui_component* component);
static void OnMouseDown(WaveBox* self, ui_component* sender, int x, int y, int button);


void InitWaveBox(WaveBox* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	self->sample = 0;
}

void OnDestroy(WaveBox* self, ui_component* component)
{		
}

void OnDraw(WaveBox* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, 0xFFAAAAA);
	if (self->sample) {
		unsigned int frame;
		for (frame = 0; frame < self->sample->numframes; ++frame) {
			float framevalue = self->sample->channels.samples[0][frame]; 
			ui_drawline(g, (int)(frame * 0.1f), 100, (int)(frame * 0.1f), (int) (100 + framevalue / 1000));	
		}
	}
}

void OnSize(WaveBox* self, ui_component* sender, int width, int height)
{
}

void OnMouseDown(WaveBox* self, ui_component* sender, int x, int y, int button)
{
}

