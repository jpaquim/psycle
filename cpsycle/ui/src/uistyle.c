/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uistyle.h"
#include "uiapp.h"
#include "uicolours.h"
#include "uicomponent.h"
/* platform */
#include "../../detail/portable.h"

/* Helper */
static psy_ui_Colour readcolour(psy_Property* parent, const char* key)
{
	psy_ui_Colour rv;
	char_dyn_t* colourset;
	bool use;

	colourset = psy_strdup(key);
	colourset = psy_strcat_realloc(colourset, "-set");
	use = psy_property_at_bool(parent, colourset, FALSE);
	if (use) {
		uint32_t value;

		value = psy_property_at_colour(parent, key, 0x0);
		rv = psy_ui_colour_make(value);
	} else {
		rv = psy_ui_colour_make(0);
		rv.mode.set = FALSE;
	}
	free(colourset);
	colourset = NULL;
	return rv;
}

/* psy_ui_Style */
void psy_ui_style_init(psy_ui_Style* self)
{
	psy_ui_colour_init(&self->colour);
	psy_ui_colour_init(&self->backgroundcolour);
	self->backgroundid = psy_INDEX_INVALID;
	self->backgroundpath = NULL;
	self->backgroundrepeat = psy_ui_REPEAT;
	self->backgroundposition = psy_ui_ALIGNMENT_NONE;
	self->overlaycolour = psy_ui_colour_make(psy_ui_RGB_WHITE);
	psy_ui_bitmapanimate_init(&self->backgroundanimation);
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->marginset = FALSE;
	psy_ui_margin_init(&self->padding);
	self->paddingset = FALSE;
	self->use_font = 0;
	self->dbgflag = 0;	
}

void psy_ui_style_init_default(psy_ui_Style* self, uintptr_t styletype)
{	
	psy_ui_style_init_copy(self, psy_ui_style_const(styletype));	
}

void psy_ui_style_init_copy(psy_ui_Style* self, const psy_ui_Style* other)
{	
	psy_ui_border_init(&self->border);
	self->use_font = 0;
	psy_ui_style_copy(self, other);	
}

void psy_ui_style_init_colours(psy_ui_Style* self, psy_ui_Colour colour,
	psy_ui_Colour background)
{
	self->colour = colour;
	self->backgroundcolour = background;
	self->backgroundid = psy_INDEX_INVALID;
	self->backgroundpath = NULL;
	self->backgroundrepeat = psy_ui_REPEAT;
	self->backgroundposition = psy_ui_ALIGNMENT_NONE;
	psy_ui_bitmapanimate_init(&self->backgroundanimation);
	self->overlaycolour = psy_ui_colour_make(psy_ui_RGB_WHITE);
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->marginset = FALSE;
	psy_ui_margin_init(&self->padding);
	self->paddingset = FALSE;
	self->use_font = 0;
	self->dbgflag = 0;
}

void psy_ui_styles_init_property(psy_ui_Style* self, psy_Property* style)
{	
	psy_ui_Colour colour;

	psy_ui_style_init(self);
	if (!style) {
		return;
	}
	colour = readcolour(style, "color");
	if (colour.mode.set) {
		self->colour = colour;
	}
	colour = readcolour(style, "background-color");
	if (colour.mode.set) {
		self->backgroundcolour = colour;
	}	
}


void psy_ui_style_dispose(psy_ui_Style* self)
{	
	if (self->use_font) {
		psy_ui_font_dispose(&self->font);
	}	
	free(self->backgroundpath);
}

void psy_ui_style_copy(psy_ui_Style* self, const psy_ui_Style* other)
{
	self->colour = other->colour;
	self->backgroundcolour = other->backgroundcolour;
	self->backgroundid = other->backgroundid;
	psy_strreset(&self->backgroundpath, other->backgroundpath);
	self->backgroundrepeat = other->backgroundrepeat;
	self->backgroundposition = other->backgroundposition;
	self->backgroundanimation = other->backgroundanimation;
	self->border = other->border;
	self->margin = other->margin;
	self->marginset = other->marginset;
	self->padding = other->padding;
	self->paddingset = other->paddingset;
	if (other->use_font) {
		if (self->use_font) {
			psy_ui_font_dispose(&self->font);
		}
		psy_ui_font_init(&self->font, NULL);
		psy_ui_font_copy(&self->font, &other->font);
		self->use_font = TRUE;
	}	
}

psy_ui_Style* psy_ui_style_alloc(void)
{
	return (psy_ui_Style*)malloc(sizeof(psy_ui_Style));
}

psy_ui_Style* psy_ui_style_allocinit(void)
{
	psy_ui_Style* rv;

	rv = psy_ui_style_alloc();
	if (rv) {
		psy_ui_style_init(rv);
	}
	return rv;
}

psy_ui_Style* psy_ui_style_allocinit_colours(psy_ui_Colour colour,
	psy_ui_Colour background)
{
	psy_ui_Style* rv;

	rv = psy_ui_style_alloc();
	if (rv) {
		psy_ui_style_init_colours(rv, colour, background);
	}
	return rv;
}

psy_ui_Style* psy_ui_style_clone(const psy_ui_Style* other)
{
	if (other) {
		psy_ui_Style* rv;

		rv = psy_ui_style_allocinit();
		if (rv) {
			psy_ui_style_copy(rv, other);
		}
		return rv;
	}
	return NULL;
}

void psy_ui_style_deallocate(psy_ui_Style* self)
{
	psy_ui_style_dispose(self);
	free(self);
}

void psy_ui_style_setfont(psy_ui_Style* self, const char* family, int size)
{	
	psy_ui_FontInfo fontinfo;

	self->use_font = 1;
	psy_ui_fontinfo_init(&fontinfo, family, size);
	psy_ui_font_init(&self->font, &fontinfo);
}

void psy_ui_style_setbackgroundpath(psy_ui_Style* self,
	const char* path)
{
		psy_strreset(&self->backgroundpath, path);
}
