/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditheaderdesc.h"
/* host */
#include "styles.h"

/* SeqEditorHeaderBar */
/* implementation */
void seqeditorheaderdescbar_init(SeqEditorHeaderDescBar* self,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 2.0));
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	zoombox_init(&self->hzoom, &self->top);	
	psy_ui_component_setalign(&self->hzoom.component, psy_ui_ALIGN_LEFT);
}
