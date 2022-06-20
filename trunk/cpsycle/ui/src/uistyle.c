/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uistyle.h"
/* local */
#include "uiapp.h"
#include "uicolours.h"
#include "uicomponent.h"
/* platform */
#include "../../detail/portable.h"


/* psy_ui_Background */
void psy_ui_background_init(psy_ui_Background* self)
{
	psy_ui_colour_init(&self->colour);
	self->overlay = psy_ui_colour_make(psy_ui_RGB_WHITE);
	self->image_id = psy_INDEX_INVALID;
	self->image_path = NULL;
	psy_ui_bitmap_init(&self->bitmap);
	self->repeat = psy_ui_REPEAT;
	self->align = psy_ui_ALIGNMENT_NONE;
	psy_ui_bitmapanimate_init(&self->animation);
	self->position = psy_ui_realpoint_zero();
	self->position_set = FALSE;
	self->size = psy_ui_realsize_zero();
	self->size_set = FALSE;
}

void psy_ui_background_dispose(psy_ui_Background* self)
{
	free(self->image_path);
	self->image_path = NULL;
	psy_ui_bitmap_dispose(&self->bitmap);
}

void psy_ui_background_copy(psy_ui_Background* self, const psy_ui_Background* other)
{
	self->colour = other->colour;
	self->image_id = other->image_id;
	psy_strreset(&self->image_path, other->image_path);
	psy_ui_bitmap_copy(&self->bitmap, &other->bitmap);
	self->repeat = other->repeat;
	self->align = other->align;
	self->animation = other->animation;	
	self->position = other->position;
	self->position_set = other->position_set;
	self->size = other->size;
	self->size_set = other->size_set;
	self->overlay = other->overlay;	
}

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
		rv.mode.transparent = TRUE;
	}
	free(colourset);
	colourset = NULL;
	return rv;
}

/* psy_ui_Style */
void psy_ui_style_init(psy_ui_Style* self)
{
	psy_ui_colour_init(&self->colour);
	self->colour.mode.inherit = TRUE;
	psy_ui_font_init(&self->font, NULL);	
	psy_ui_background_init(&self->background);		
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->marginset = FALSE;
	psy_ui_margin_init(&self->padding);
	self->paddingset = FALSE;
	psy_ui_position_init(&self->position);	
	self->dbgflag = 0;	
}

void psy_ui_style_init_default(psy_ui_Style* self, uintptr_t styletype)
{	
	psy_ui_style_init_copy(self, psy_ui_style_const(styletype));	
}

void psy_ui_style_init_copy(psy_ui_Style* self, const psy_ui_Style* other)
{	
	psy_ui_border_init(&self->border);
	psy_ui_font_init(&self->font, NULL);
	psy_ui_font_copy(&self->font, &other->font);
	psy_ui_background_init(&self->background);	
	psy_ui_background_copy(&self->background, &other->background);
	psy_ui_style_copy(self, other);		
}

void psy_ui_style_init_colours(psy_ui_Style* self, psy_ui_Colour colour,
	psy_ui_Colour background)
{
	self->colour = colour;	
	self->colour.mode.inherit = TRUE;
	psy_ui_font_init(&self->font, NULL);
	psy_ui_background_init(&self->background);
	self->background.colour = background;	
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->marginset = FALSE;
	psy_ui_margin_init(&self->padding);
	self->paddingset = FALSE;
	psy_ui_position_init(&self->position);	
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
	if (!colour.mode.transparent) {
		self->colour = colour;
	}
	colour = readcolour(style, "background-color");
	if (!colour.mode.transparent) {
		self->background.colour = colour;
	}	
}

void psy_ui_style_dispose(psy_ui_Style* self)
{		
	psy_ui_font_dispose(&self->font);
	psy_ui_background_dispose(&self->background);
	psy_ui_position_dispose(&self->position);
}

void psy_ui_style_copy(psy_ui_Style* self, const psy_ui_Style* other)
{
	self->colour = other->colour;	
	psy_ui_background_copy(&self->background, &other->background);	
	self->border = other->border;
	self->margin = other->margin;
	self->marginset = other->marginset;
	self->padding = other->padding;
	self->paddingset = other->paddingset;
	psy_ui_position_dispose(&self->position);
	psy_ui_position_init(&self->position);
	if (psy_ui_position_is_active(&other->position)) {
		psy_ui_position_set_rectangle(&self->position,
			*other->position.rectangle);
	}	
	psy_ui_font_copy(&self->font, &other->font);	
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

void psy_ui_style_set_font(psy_ui_Style* self, const char* family, double size)
{	
	psy_ui_FontInfo fontinfo;
	
	psy_ui_font_dispose(&self->font);
	psy_ui_fontinfo_init(&fontinfo, family, size);
	psy_ui_font_init(&self->font, &fontinfo);
}

void psy_ui_style_set_background_id(psy_ui_Style* self,
	uintptr_t id)
{	
	free(self->background.image_path);
	self->background.image_path = NULL;
	self->background.image_id = id;
	psy_ui_bitmap_load_resource(&self->background.bitmap, id);
}

int psy_ui_style_setbackgroundpath(psy_ui_Style* self,
	const char* path)
{		
	psy_strreset(&self->background.image_path, path);
	return psy_ui_bitmap_load(&self->background.bitmap, path);
}
