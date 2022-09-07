/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uistyles.h"
/* local */
#include "uiappstyles.h"
/* platform */
#include "../../detail/portable.h"


/* implementation */
void psy_ui_styles_init(psy_ui_Styles* self)
{
	assert(self);
	
	psy_table_init(&self->styles);	
	self->theme_mode = psy_ui_DARKTHEME;
}

void psy_ui_styles_dispose(psy_ui_Styles* self)
{
	assert(self);
	
	psy_table_dispose_all(&self->styles, (psy_fp_disposefunc)
		psy_ui_style_dispose);	
}

void psy_ui_styles_set_style(psy_ui_Styles* self, uintptr_t styletype,
	const char* name, psy_ui_Style* style)
{
	psy_ui_Style* currstyle;

	assert(self);
	assert(style);	
	
	if ((currstyle = (psy_ui_Style*)psy_table_at(&self->styles, styletype))) {
		psy_ui_style_deallocate(currstyle);
	}
	psy_ui_style_set_name(style, name);
	psy_table_insert(&self->styles, styletype, style);
}

psy_ui_Style* psy_ui_styles_at(psy_ui_Styles* self, uintptr_t styletype)
{
	psy_ui_Style* rv;

	assert(self);

	rv = (psy_ui_Style*)psy_table_at(&self->styles, styletype);
	if (!rv) {
		rv = (psy_ui_Style*)psy_table_at(&self->styles, psy_ui_STYLE_ROOT);
		assert(rv);
	}
	return rv;
}

const psy_ui_Style* psy_ui_styles_at_const(const psy_ui_Styles* self,
	uintptr_t styletype)
{
	assert(self);

	return psy_ui_styles_at((psy_ui_Styles*)self, styletype);
}
