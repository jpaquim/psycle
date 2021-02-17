// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiprogressbar.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void ondestroy(psy_ui_ProgressBar*);
static void ondraw(psy_ui_ProgressBar*, psy_ui_Graphics*);
static void onpreferredsize(psy_ui_ProgressBar*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_ProgressBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)ondestroy;
		vtable.ondraw = (psy_ui_fp_component_ondraw)ondraw;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			onpreferredsize;
		vtable_initialized = TRUE;
	}
}
// implementation
void psy_ui_progressbar_init(psy_ui_ProgressBar* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_PROGRESSBAR,
		psy_ui_STYLE_PROGRESSBAR,
		psy_ui_STYLE_PROGRESSBAR);	
	self->text = NULL;
	self->progress = 0.f;	
}

void ondestroy(psy_ui_ProgressBar* self)
{
	free(self->text);
}

void psy_ui_progressbar_settext(psy_ui_ProgressBar* self, const char* text)
{
	psy_strreset(&self->text, text);
}

void ondraw(psy_ui_ProgressBar* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;	
	psy_ui_RealRectangle r;
			
	tm = psy_ui_component_textmetric(&self->component);	
	size = psy_ui_component_sizepx(&self->component);	
	psy_ui_setrectangle(&r, 0, (size.height - tm->tmHeight) / 2, 
		self->progress * size.width, tm->tmHeight);
	psy_ui_drawsolidrectangle(g, r,
		psy_ui_component_colour(&self->component));	
}

void onpreferredsize(psy_ui_ProgressBar* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	if (rv) {
		rv->width = psy_ui_value_makeew(20);
		rv->height = psy_ui_value_makeeh(1);
	}
}

void psy_ui_progressbar_setprogress(psy_ui_ProgressBar* self, float progress)
{
	self->progress = progress;
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_update(&self->component);	
}

void psy_ui_progressbar_tick(psy_ui_ProgressBar* self)
{
	self->progress += 0.01f;
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_update(&self->component);	
}
