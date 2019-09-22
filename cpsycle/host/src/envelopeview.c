#include "envelopeview.h"

static void OnDraw(EnvelopeView* self, ui_component* sender, ui_graphics* g);
static void OnSize(EnvelopeView* self, ui_component* sender, int width, int height);
static void OnDestroy(EnvelopeView* self, ui_component* component);
static void OnMouseDown(EnvelopeView* self, ui_component* sender, int x, int y, int button);


void InitEnvelopeView(EnvelopeView* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);	
}

void OnDestroy(EnvelopeView* self, ui_component* component)
{		
}

void OnDraw(EnvelopeView* self, ui_component* sender, ui_graphics* g)
{	
//	ui_rectangle r;
//	ui_size size = ui_component_size(&self->component);
//	ui_setrectangle(&r, 0, 0, size.width, size.height);
	
}

void OnSize(EnvelopeView* self, ui_component* sender, int width, int height)
{
}

void OnMouseDown(EnvelopeView* self, ui_component* sender, int x, int y, int button)
{
}

