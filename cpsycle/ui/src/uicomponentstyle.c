// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponentstyle.h"
// local
#include "uiapp.h"
// platform
#include "../../detail/portable.h"

void psy_ui_componentstyle_init(psy_ui_ComponentStyle* self)
{
	psy_ui_style_init(&self->style);
	psy_ui_style_init(&self->hover);
	psy_ui_style_init(&self->select);
	psy_ui_style_init(&self->disabled);
	self->currstyle = &self->style;
	self->style_id = psy_INDEX_INVALID;
	self->hover_id = psy_INDEX_INVALID;
	self->select_id = psy_INDEX_INVALID;
	self->disabled_id = psy_INDEX_INVALID;
	self->state = psy_ui_STYLESTATE_NONE;
}

void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle* self)
{
	psy_ui_style_dispose(&self->style);
	psy_ui_style_dispose(&self->hover);
	psy_ui_style_dispose(&self->select);
}
