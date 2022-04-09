/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uidefaults.h"
/* local */
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

void psy_ui_defaults_setstyle(psy_ui_Defaults* self, uintptr_t styletype,
	psy_ui_Style* style)
{
	if (style) {
		psy_ui_styles_set_style(&self->styles, styletype, style);
	}
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

void psy_ui_defaults_load_theme(psy_ui_Defaults* self, const char* dir, psy_ui_ThemeMode theme)
{
	psy_Property* styleconfig;
	psy_Path path;
	psy_PropertyReader propertyreader;
	
	self->styles.theme = theme;
	psy_path_init(&path, NULL);
	psy_path_set_prefix(&path, dir);
	if (theme == psy_ui_DARKTHEME) {
		psy_path_set_name(&path, PSYCLE_DARKSTYLES_INI);		
	} else {
		psy_path_set_name(&path, PSYCLE_LIGHTSTYLES_INI);
	}	
	styleconfig = psy_property_clone(
		psy_ui_styles_configuration(&self->styles));
	
	psy_propertyreader_init(&propertyreader, styleconfig,
		psy_path_full(&path));
	if (psy_propertyreader_load(&propertyreader) == PSY_OK) {	
		psy_ui_FontInfo fontinfo;

		self->styles.theme = theme;
		psy_ui_styles_configure(&self->styles, styleconfig);				
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", -16);
		psy_ui_font_init(&psy_ui_defaults_style(self, psy_ui_STYLE_ROOT)->font,
			&fontinfo);		
	}	
	psy_propertyreader_dispose(&propertyreader);
	psy_property_deallocate(styleconfig);
	psy_path_dispose(&path);	
}

void psy_ui_defaults_savetheme(psy_ui_Defaults* self, const char* path)
{	
	const psy_Property* styleconfig;	

	assert(self);

	styleconfig = psy_ui_styles_configuration(&self->styles);	
	if (styleconfig && !psy_property_empty(styleconfig)) {	
		psy_PropertyWriter propertywriter;

		psy_propertywriter_init(&propertywriter, styleconfig, path);
		psy_propertywriter_save(&propertywriter);
		psy_propertywriter_dispose(&propertywriter);
	}	
}
