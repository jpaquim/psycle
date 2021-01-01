// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEFAULTS_H
#define psy_ui_DEFAULTS_H

#include "../../detail/psyconf.h"

#include "../../detail/stdint.h"
#include "uidef.h"
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Defaults {
	uint32_t errorcolour;
	psy_ui_Style style_common;	
	psy_ui_Margin hmargin;
	psy_ui_Margin vmargin;
	psy_ui_Margin cmargin;
} psy_ui_Defaults;

void psy_ui_defaults_init(psy_ui_Defaults*);
void psy_ui_defaults_dispose(psy_ui_Defaults*);

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

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEFAULTS_H */
