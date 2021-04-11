// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponentscroll.h"

static bool componentscroll_initialized = FALSE;
static psy_ui_ComponentScroll componentscroll;

void psy_ui_componentscroll_init(psy_ui_ComponentScroll* self)
{
	psy_ui_size_init(&self->step);
	self->mode = psy_ui_SCROLL_GRAPHICS;
	psy_ui_point_init(&self->offset);
	psy_ui_intpoint_init(&self->hrange);
	psy_ui_intpoint_init(&self->vrange);
	self->wheel = 0;
	self->overflow = psy_ui_OVERFLOW_HIDDEN;
}
