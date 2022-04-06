/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiprogressbar.h"

/* prototypes */
static void ondraw(psy_ui_ProgressBar*, psy_ui_Graphics*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_ProgressBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			ondraw;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_progressbar_init(psy_ui_ProgressBar* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	vtable_init(self);
	psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_PROGRESSBAR);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(10.0, 0.0));
	self->progress = 0.0;
}

void ondraw(psy_ui_ProgressBar* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;	
	psy_ui_RealRectangle bar;
				
	size = psy_ui_component_size_px(&self->component);	
	psy_ui_realrectangle_init_all(&bar,
		psy_ui_realpoint_zero(),
		psy_ui_realsize_make(self->progress * size.width, size.height));
	psy_ui_drawsolidrectangle(g, bar,
		psy_ui_component_colour(progressbar_base(self)));
}

void psy_ui_progressbar_setprogress(psy_ui_ProgressBar* self, double progress)
{
	self->progress = progress;
	psy_ui_component_invalidate(progressbar_base(self));
	psy_ui_component_update(progressbar_base(self));
}

void psy_ui_progressbar_tick(psy_ui_ProgressBar* self)
{
	psy_ui_progressbar_setprogress(self, self->progress + 0.01);	
}
