// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEFAULTS_H
#define psy_ui_DEFAULTS_H

// local
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_ui_LIGHTTHEME 0
#define psy_ui_DARKTHEME 1

typedef enum psy_ui_StyleTypes {
	psy_ui_STYLE_ROOT,
	psy_ui_STYLE_LABEL,
	psy_ui_STYLE_LABEL_HOVER,
	psy_ui_STYLE_LABEL_SELECT,
	psy_ui_STYLE_LABEL_DISABLED,
	psy_ui_STYLE_EDIT,
	psy_ui_STYLE_BUTTON,
	psy_ui_STYLE_BUTTON_HOVER,
	psy_ui_STYLE_BUTTON_SELECT,
	psy_ui_STYLE_TABBAR,
	psy_ui_STYLE_TAB,
	psy_ui_STYLE_TAB_HOVER,
	psy_ui_STYLE_TAB_SELECT,	
	psy_ui_STYLE_CONTAINERHEADER,
	psy_ui_STYLE_SCROLLPANE,
	psy_ui_STYLE_SCROLLTHUMB,
	psy_ui_STYLE_SCROLLTHUMB_HOVER,
	psy_ui_STYLE_SLIDERPANE,
	psy_ui_STYLE_SLIDERTHUMB,
	psy_ui_STYLE_SLIDERTHUMB_HOVER,
	psy_ui_STYLE_COMBOBOX,
	psy_ui_STYLE_COMBOBOX_HOVER,
	psy_ui_STYLE_COMBOBOX_SELECT,
	psy_ui_STYLE_SPLITTER,
	psy_ui_STYLE_SPLITTER_HOVER,
	psy_ui_STYLE_SPLITTER_SELECT,
	psy_ui_STYLE_PROGRESSBAR
} psy_ui_StyleTypes;

typedef struct psy_ui_Defaults {
	uint32_t errorcolour;	
	psy_ui_Styles styles;
	psy_ui_Margin hmargin;
	psy_ui_Margin vmargin;
	psy_ui_Margin cmargin;
	bool hasdarktheme;
} psy_ui_Defaults;

void psy_ui_defaults_init(psy_ui_Defaults*, bool dark);
void psy_ui_defaults_dispose(psy_ui_Defaults*);

void psy_ui_defaults_setstyle(psy_ui_Defaults*, uintptr_t styletype, psy_ui_Style*);
psy_ui_Style* psy_ui_defaults_style(psy_ui_Defaults* self, uintptr_t styletype);
const psy_ui_Style* psy_ui_defaults_style_const(const psy_ui_Defaults* self,
	uintptr_t styletype);
void psy_ui_defaults_loadtheme(psy_ui_Defaults*, const char* configdir,
	bool isdark);
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

void psy_ui_defaults_initdarktheme(psy_ui_Defaults*);
void psy_ui_defaults_initlighttheme(psy_ui_Defaults*);
void psy_ui_defaults_loadtheme(psy_ui_Defaults*, const char* path, bool isdark);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEFAULTS_H */
