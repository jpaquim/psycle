// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_STYLE_H
#define psy_ui_STYLE_H

#include "uifont.h"
#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Style {
	psy_ui_Font font;
	psy_ui_Colour colour;
	psy_ui_Colour backgroundcolour;
	psy_ui_Border border;
	bool use_font;
} psy_ui_Style;

void psy_ui_style_init(psy_ui_Style*);
void psy_ui_style_init_colours(psy_ui_Style*, psy_ui_Colour,
	psy_ui_Colour background);
void psy_ui_style_dispose(psy_ui_Style*);
void psy_ui_style_copy(psy_ui_Style*, psy_ui_Style* other);

psy_ui_Style* psy_ui_style_alloc(void);
psy_ui_Style* psy_ui_style_allocinit(void);
psy_ui_Style* psy_ui_style_allocinit_colours(psy_ui_Colour,
	psy_ui_Colour background);
void psy_ui_style_deallocate(psy_ui_Style*);



#ifdef __cplusplus
}
#endif

#endif /* psy_ui_STYLE_H */
