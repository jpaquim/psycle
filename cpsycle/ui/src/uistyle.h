// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_STYLE_H
#define psy_ui_STYLE_H

// local
#include "uifont.h"
#include "uidef.h"
// container
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_ui_Style
typedef struct psy_ui_Style {
	psy_ui_Font font;
	psy_ui_Colour colour;
	psy_ui_Colour backgroundcolour;	
	psy_ui_Border border;	
	bool use_font;
	int dbgflag;
} psy_ui_Style;

void psy_ui_style_init(psy_ui_Style*);
void psy_ui_style_init_default(psy_ui_Style*, int styletype);
void psy_ui_style_init_copy(psy_ui_Style*, const psy_ui_Style* other);
void psy_ui_style_init_colours(psy_ui_Style*, psy_ui_Colour,
	psy_ui_Colour background);
void psy_ui_style_dispose(psy_ui_Style*);
void psy_ui_style_copy(psy_ui_Style*, const psy_ui_Style* other);

psy_ui_Style* psy_ui_style_alloc(void);
psy_ui_Style* psy_ui_style_allocinit(void);
psy_ui_Style* psy_ui_style_allocinit_colours(psy_ui_Colour,
	psy_ui_Colour background);
void psy_ui_style_deallocate(psy_ui_Style*);

// Properties
INLINE void psy_ui_style_setcolour(psy_ui_Style* self, psy_ui_Colour colour)
{
	self->colour = colour;
}

INLINE void psy_ui_style_setbackgroundcolour(psy_ui_Style* self,
	psy_ui_Colour colour)
{
	self->backgroundcolour = colour;
}

INLINE void psy_ui_style_setcolours(psy_ui_Style* self,
	psy_ui_Colour colour, psy_ui_Colour background)
{
	psy_ui_style_setcolour(self, colour);
	psy_ui_style_setbackgroundcolour(self, background);	
}

void psy_ui_style_setfont(psy_ui_Style*, const char* family, int size);


// psy_ui_Styles
typedef struct psy_ui_Styles {
	psy_Table styles;
	psy_Property config;
} psy_ui_Styles;

void psy_ui_styles_init(psy_ui_Styles*);
void psy_ui_styles_dispose(psy_ui_Styles*);

void psy_ui_styles_setstyle(psy_ui_Styles*, int styletype, psy_ui_Style*);
void psy_ui_styles_mixstyle(psy_ui_Styles*, int styletype, psy_ui_Style*);
psy_ui_Style* psy_ui_styles_at(psy_ui_Styles* self, int styletype);
const psy_ui_Style* psy_ui_styles_at_const(const psy_ui_Styles* self,
	int styletype);
void psy_ui_styles_configure(psy_ui_Styles*, psy_Property*);
const psy_Property* psy_ui_styles_configuration(const psy_ui_Styles*);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_STYLE_H */
