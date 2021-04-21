// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponentsizehints.h"

void psy_ui_componentsizehints_init(psy_ui_ComponentSizeHints* self)
{
	psy_ui_margin_init(&self->margin);
	psy_ui_size_init(&self->minsize);
	psy_ui_size_setroundmode(&self->minsize, psy_ui_ROUND_FLOOR);
	psy_ui_size_init(&self->maxsize);
	psy_ui_size_setroundmode(&self->maxsize, psy_ui_ROUND_FLOOR);
	psy_ui_margin_init(&self->spacing);	
	psy_ui_size_init(&self->preferredsize);
	self->preferredwidthset = FALSE;
	self->preferredheightset = FALSE;
}
