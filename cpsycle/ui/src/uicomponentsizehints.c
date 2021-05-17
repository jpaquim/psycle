/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicomponentsizehints.h"
/* std */
#include <stdlib.h>

void psy_ui_componentsizehints_init(psy_ui_ComponentSizeHints* self)
{
	psy_ui_size_init(&self->minsize);	
	psy_ui_size_init(&self->maxsize);		
	psy_ui_size_init(&self->preferredsize);
	self->preferredwidthset = FALSE;
	self->preferredheightset = FALSE;
}

psy_ui_ComponentSizeHints* psy_ui_componentsizehints_alloc(void)
{
	return (psy_ui_ComponentSizeHints*)malloc(sizeof(
		psy_ui_ComponentSizeHints));
}

psy_ui_ComponentSizeHints* psy_ui_componentsizehints_allocinit(void)
{
	psy_ui_ComponentSizeHints* rv;

	rv = psy_ui_componentsizehints_alloc();
	if (rv) {
		psy_ui_componentsizehints_init(rv);
	}
	return rv;
}

void psy_ui_componentsizehints_deallocate(psy_ui_ComponentSizeHints* self)
{
	free(self);
}
