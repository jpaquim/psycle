/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_STYLE_H
#define psy_ui_STYLE_H

/* local */
#include "uicolour.h"
#include "uibitmap.h"
#include "uifont.h"
#include "uidef.h"
/* container */
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_ui_ThemeMode {
	psy_ui_LIGHTTHEME = 0,
	psy_ui_DARKTHEME  = 1,
	psy_ui_WIN98THEME = 2
} psy_ui_ThemeMode;

typedef enum {
	psy_ui_REPEAT,
	psy_ui_NOREPEAT	
} psy_ui_BackgroundRepeat;

/* psy_ui_Style */
typedef struct psy_ui_Style {
	psy_ui_Font font;
	psy_ui_Colour colour;
	psy_ui_Colour backgroundcolour;
	uintptr_t backgroundid;
	char* backgroundpath;
	psy_ui_BackgroundRepeat backgroundrepeat;
	psy_ui_Alignment backgroundposition;
	psy_ui_BitmapAnimate backgroundanimation;
	psy_ui_Colour overlaycolour;
	psy_ui_Border border;
	psy_ui_Margin margin;
	bool marginset;
	psy_ui_Margin padding;
	bool paddingset;	
	int dbgflag;	
} psy_ui_Style;

void psy_ui_style_init(psy_ui_Style*);
void psy_ui_style_init_default(psy_ui_Style*, uintptr_t styletype);
void psy_ui_style_init_copy(psy_ui_Style*, const psy_ui_Style* other);
void psy_ui_style_init_colours(psy_ui_Style*, psy_ui_Colour,
	psy_ui_Colour background);
void psy_ui_styles_init_property(psy_ui_Style*, psy_Property* style);
void psy_ui_style_dispose(psy_ui_Style*);
void psy_ui_style_copy(psy_ui_Style*, const psy_ui_Style* other);

psy_ui_Style* psy_ui_style_alloc(void);
psy_ui_Style* psy_ui_style_allocinit(void);
psy_ui_Style* psy_ui_style_allocinit_colours(psy_ui_Colour,
	psy_ui_Colour background);
psy_ui_Style* psy_ui_style_clone(const psy_ui_Style* other);
void psy_ui_style_deallocate(psy_ui_Style*);

/* Properties */
INLINE void psy_ui_style_setcolour(psy_ui_Style* self, psy_ui_Colour colour)
{
	self->colour = colour;
}

INLINE void psy_ui_style_setbackgroundcolour(psy_ui_Style* self,
	psy_ui_Colour colour)
{
	self->backgroundcolour = colour;
}

INLINE void psy_ui_style_setbackgroundoverlay(psy_ui_Style* self,
	int overlay)
{
	self->backgroundcolour = psy_ui_colour_make_overlay(overlay);
}

INLINE void psy_ui_style_setbackgroundid(psy_ui_Style* self,
	uintptr_t id)
{
	self->backgroundid = id;
}

void psy_ui_style_setbackgroundpath(psy_ui_Style* self,
	const char* path);

INLINE void psy_ui_style_animatebackground(psy_ui_Style* self,
	uintptr_t interval, psy_ui_RealSize framesize, bool horizontal)
{
	self->backgroundanimation.enabled = TRUE;
	self->backgroundanimation.interval = interval;
	self->backgroundanimation.framesize = framesize;
	self->backgroundanimation.horizontal = horizontal;
}

INLINE void psy_ui_style_setcolours(psy_ui_Style* self,
	psy_ui_Colour colour, psy_ui_Colour background)
{
	psy_ui_style_setcolour(self, colour);
	psy_ui_style_setbackgroundcolour(self, background);	
}

void psy_ui_style_setfont(psy_ui_Style*, const char* family, int size);

INLINE void psy_ui_style_setmargin(psy_ui_Style* self, psy_ui_Margin margin)
{
	self->margin = margin;
	self->marginset = TRUE;
}

INLINE psy_ui_Margin psy_ui_style_margin(const psy_ui_Style* self)
{
	return self->margin;	
}

INLINE void psy_ui_style_setmargin_em(psy_ui_Style* self,
	double top, double right, double bottom, double left)
{
	psy_ui_style_setmargin(self,
		psy_ui_margin_make_em(top, right, bottom, left));
}

INLINE void psy_ui_style_setpadding(psy_ui_Style* self,
	psy_ui_Margin padding)
{
	self->padding = padding;
	self->paddingset = TRUE;
}

INLINE void psy_ui_style_setpadding_em(psy_ui_Style* self,
	double top, double right, double bottom, double left)
{
	psy_ui_style_setpadding(self,
		psy_ui_margin_make_em(top, right, bottom, left));	
}

INLINE psy_ui_Margin psy_ui_style_padding(const psy_ui_Style* self)
{
	return self->padding;
}

INLINE void psy_ui_style_setborder(psy_ui_Style* self, const psy_ui_Border* border)
{
	assert(border);

	self->border = *border;
}


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_STYLE_H */
