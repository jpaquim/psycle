/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicomponentsizehints.h"
/* std */
#include <stdlib.h>

void psy_ui_sizehints_init(psy_ui_SizeHints* self)
{
	psy_ui_size_init(&self->minsize);	
	psy_ui_size_init(&self->maxsize);		
	psy_ui_size_init(&self->preferredsize);
	self->preferredsize.width.set = FALSE;
	self->preferredsize.height.set = FALSE;	
}

psy_ui_SizeHints* psy_ui_sizehints_alloc(void)
{
	return (psy_ui_SizeHints*)malloc(sizeof(
		psy_ui_SizeHints));
}

psy_ui_SizeHints* psy_ui_sizehints_allocinit(void)
{
	psy_ui_SizeHints* rv;

	rv = psy_ui_sizehints_alloc();
	if (rv) {
		psy_ui_sizehints_init(rv);
	}
	return rv;
}

void psy_ui_sizehints_deallocate(psy_ui_SizeHints* self)
{
	free(self);
}
