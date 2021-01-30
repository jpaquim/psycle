// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEFAULTS_H
#define psy_ui_DEFAULTS_H

#include "../../detail/psyconf.h"
#include "../../detail/stdint.h"
#include "uidef.h"
#include "uistyle.h"
// container
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define psy_ui_LIGHTTHEME 0
#define psy_ui_DARKTHEME 1

typedef enum psy_ui_StyleTypes {
	psy_ui_STYLE_COMMON,
	psy_ui_STYLE_COMMON_SELECT,
	psy_ui_STYLE_BUTTON,
	psy_ui_STYLE_BUTTON_HOVER,
	psy_ui_STYLE_BUTTON_SELECT,
	psy_ui_STYLE_TAB,
	psy_ui_STYLE_TAB_HOVER,
	psy_ui_STYLE_TAB_SELECT,
	psy_ui_STYLE_SLIDER,
	psy_ui_STYLE_SIDEMENU,
	psy_ui_STYLE_SIDEMENU_SELECT,
	psy_ui_STYLE_CONTAINERHEADER
} psy_ui_StyleTypes;

typedef struct psy_ui_Defaults {
	uint32_t errorcolour;	
	psy_Table styles;
	psy_ui_Margin hmargin;
	psy_ui_Margin vmargin;
	psy_ui_Margin cmargin;
	bool hasdarktheme;
} psy_ui_Defaults;

void psy_ui_defaults_init(psy_ui_Defaults*, bool dark);
void psy_ui_defaults_dispose(psy_ui_Defaults*);

void psy_ui_defaults_setstyle(psy_ui_Defaults*, int styletype, psy_ui_Style*);
psy_ui_Style* psy_ui_defaults_style(psy_ui_Defaults* self, int styletype);
const psy_ui_Style* psy_ui_defaults_style_const(const psy_ui_Defaults* self,
	int styletype);

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
	return &psy_ui_defaults_style(self, psy_ui_STYLE_COMMON)->font;	
}

void psy_ui_defaults_initdarktheme(psy_ui_Defaults*);
void psy_ui_defaults_initlighttheme(psy_ui_Defaults*);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEFAULTS_H */
