/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicomponentscroll.h"
/* std */
#include <stdlib.h>

void psy_ui_componentscroll_init(psy_ui_ComponentScroll* self)
{
	psy_ui_size_init(&self->step);	
	psy_ui_intpoint_init(&self->hrange);
	psy_ui_intpoint_init(&self->vrange);
	self->wheel = 0;
	self->overflow = psy_ui_OVERFLOW_HIDDEN;
}

psy_ui_ComponentScroll* psy_ui_componentscroll_alloc(void)
{
	return (psy_ui_ComponentScroll*)malloc(sizeof(
		psy_ui_ComponentScroll));
}

psy_ui_ComponentScroll* psy_ui_componentscroll_allocinit(void)
{
	psy_ui_ComponentScroll* rv;

	rv = psy_ui_componentscroll_alloc();
	if (rv) {
		psy_ui_componentscroll_init(rv);
	}
	return rv;
}

void psy_ui_componentscroll_deallocate(psy_ui_ComponentScroll* self)
{
	free(self);
}
