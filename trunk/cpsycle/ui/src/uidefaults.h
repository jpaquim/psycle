/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_DEFAULTS_H
#define psy_ui_DEFAULTS_H

/* local */
#include "uiappstyles.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Defaults {	
	psy_ui_Styles styles;	
	psy_ui_Margin hmargin;
	psy_ui_Margin vmargin;
	psy_ui_Margin cmargin;	
} psy_ui_Defaults;

void psy_ui_defaults_init(psy_ui_Defaults*, psy_ui_ThemeMode);
void psy_ui_defaults_dispose(psy_ui_Defaults*);

void psy_ui_defaults_setstyle(psy_ui_Defaults*, uintptr_t styletype, psy_ui_Style*);
psy_ui_Style* psy_ui_defaults_style(psy_ui_Defaults* self, uintptr_t styletype);
const psy_ui_Style* psy_ui_defaults_style_const(const psy_ui_Defaults*,
	uintptr_t styletype);
void psy_ui_defaults_inittheme(psy_ui_Defaults*, psy_ui_ThemeMode theme, bool keepfont);
void psy_ui_defaults_load_theme(psy_ui_Defaults*, const char* path, psy_ui_ThemeMode);
void psy_ui_defaults_savetheme(psy_ui_Defaults* self, const char* filename);

INLINE psy_ui_Margin psy_ui_defaults_hmargin(const psy_ui_Defaults* self)
{
	return self->hmargin;
}

INLINE const psy_ui_Margin* psy_ui_defaults_phmargin(const psy_ui_Defaults* self)
{
	return &self->hmargin;
}

INLINE psy_ui_Margin psy_ui_defaults_vmargin(const psy_ui_Defaults* self)
{
	return self->vmargin;
}

INLINE const psy_ui_Margin* psy_ui_defaults_pvmargin(const psy_ui_Defaults* self)
{
	return &self->vmargin;
}

INLINE psy_ui_Margin psy_ui_defaults_cmargin(const psy_ui_Defaults* self)
{
	return self->cmargin;
}

INLINE const psy_ui_Margin* psy_ui_defaults_pcmargin(const psy_ui_Defaults* self)
{
	return &self->cmargin;
}

INLINE psy_ui_Font* psy_ui_defaults_font(psy_ui_Defaults* self)
{
	return &psy_ui_defaults_style(self, psy_ui_STYLE_ROOT)->font;	
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEFAULTS_H */
