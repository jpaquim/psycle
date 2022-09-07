/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uidefaults.h"
/* local */
#include "uilclaligner.h"
#include "uifont.h"
/* file */
#include <dir.h>
#include <propertiesio.h>

void psy_ui_defaults_init(psy_ui_Defaults* self, psy_ui_ThemeMode theme)
{		
	psy_ui_styles_init(&self->styles);	
	psy_ui_defaults_inittheme(self, theme, FALSE);	
	psy_ui_margin_init_em(&self->hmargin, 0.0, 1.0, 0.0, 0.0);		
	psy_ui_margin_init_em(&self->vmargin, 0.0, 0.0, 0.5, 0.0);		
	psy_ui_margin_init_em(&self->cmargin, 0.0, 0.5, 0.5, 0.5);	
}

void psy_ui_defaults_inittheme(psy_ui_Defaults* self, psy_ui_ThemeMode theme,
	bool keepfont)
{		
	psy_ui_appstyles_inittheme(&self->styles, theme, keepfont);	
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_styles_dispose(&self->styles);		
}

psy_ui_Style* psy_ui_defaults_style(psy_ui_Defaults* self, uintptr_t type)
{
	return psy_ui_styles_at(&self->styles, type);
}

const psy_ui_Style* psy_ui_defaults_style_const(const psy_ui_Defaults* self,
	uintptr_t type)
{
	return psy_ui_styles_at_const(&self->styles, type);
}
