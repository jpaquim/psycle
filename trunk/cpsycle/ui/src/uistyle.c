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
	self->backgroundrepeat = psy_ui_REPEAT;
	self->backgroundposition = psy_ui_ALIGNMENT_NONE;
	self->overlaycolour = psy_ui_colour_make(psy_ui_RGB_WHITE);
	psy_ui_bitmapanimate_init(&self->backgroundanimation);
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->marginset = FALSE;
	psy_ui_margin_init(&self->spacing);
	self->spacingset = FALSE;
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
	self->backgroundrepeat = psy_ui_REPEAT;
	self->backgroundposition = psy_ui_ALIGNMENT_NONE;
	psy_ui_bitmapanimate_init(&self->backgroundanimation);
	self->overlaycolour = psy_ui_colour_make(psy_ui_RGB_WHITE);
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->marginset = FALSE;
	psy_ui_margin_init(&self->spacing);
	self->spacingset = FALSE;
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
}

void psy_ui_style_copy(psy_ui_Style* self, const psy_ui_Style* other)
{
	self->colour = other->colour;
	self->backgroundcolour = other->backgroundcolour;
	self->backgroundid = other->backgroundid;
	self->backgroundrepeat = other->backgroundrepeat;
	self->backgroundposition = other->backgroundposition;
	self->backgroundanimation = other->backgroundanimation;
	self->border = other->border;
	self->margin = other->margin;
	self->marginset = other->marginset;
	self->spacing = other->spacing;
	self->spacingset = other->spacingset;
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

/*
** Styles
** prototypes
*/
static void psy_ui_styles_updateconfig(psy_ui_Styles*);
static void psy_ui_styles_addstyletoconfig(psy_ui_Styles*, psy_ui_Style*,
	uintptr_t styletype);
static void psy_ui_styles_addcolourtoconfig(psy_ui_Styles*,
	psy_Property* parent, const char* key, psy_ui_Colour);
static psy_ui_Style* psy_ui_styles_readstyle(psy_ui_Styles*,
	uintptr_t styletype);
/* implementation */
void psy_ui_styles_init(psy_ui_Styles* self)
{
	psy_table_init(&self->styles);
	psy_property_init(&self->config);
	self->theme = psy_ui_DARKTHEME;
}

void psy_ui_styles_dispose(psy_ui_Styles* self)
{
	psy_table_disposeall(&self->styles,
		(psy_fp_disposefunc)psy_ui_style_dispose);
	psy_property_dispose(&self->config);
}

void psy_ui_styles_setstyle(psy_ui_Styles* self, uintptr_t styletype, psy_ui_Style* style)
{
	psy_ui_Style* currstyle;

	assert(self);
	assert(style);
	
	if (currstyle = (psy_ui_Style*)psy_table_at(&self->styles, styletype)) {
		psy_ui_style_deallocate(currstyle);
	}
	psy_table_insert(&self->styles, styletype, style);
}

psy_ui_Style* psy_ui_styles_at(psy_ui_Styles* self, uintptr_t styletype)
{
	psy_ui_Style* rv;

	assert(self);

	rv = (psy_ui_Style*)psy_table_at(&self->styles, styletype);
	if (!rv) {
		rv = (psy_ui_Style*)psy_table_at(&self->styles, psy_ui_STYLE_ROOT);
		assert(rv);
	}
	return rv;
}

const psy_ui_Style* psy_ui_styles_at_const(const psy_ui_Styles* self,
	uintptr_t styletype)
{
	assert(self);

	return psy_ui_styles_at((psy_ui_Styles*)self, styletype);
}

void psy_ui_styles_configure(psy_ui_Styles* self, psy_Property* config)
{
	uintptr_t i;
	if (psy_property_empty(&self->config)) {
		psy_ui_styles_updateconfig(self);
	}
	psy_property_sync(&self->config, config);
		
	for (i = 0; i < psy_table_size(&self->styles); ++i) {
		psy_ui_Style* style;

		style = psy_ui_styles_readstyle(self, i);
		if (style) {
			psy_ui_styles_setstyle(self, i, style);
		}
	}
}

psy_ui_Style* psy_ui_styles_readstyle(psy_ui_Styles* self, uintptr_t styletype)
{	
	char key[128];
	psy_Property* section;

	psy_snprintf(key, 128, "style-%d", (int)styletype);
	section = psy_property_at(&self->config, key, PSY_PROPERTY_TYPE_SECTION);
	if (section) {
		psy_ui_Style* rv;		

		rv = psy_ui_style_alloc();	
		psy_ui_styles_init_property(rv, section);		
		return rv;
	}
	return NULL;
}

const psy_Property* psy_ui_styles_configuration(const psy_ui_Styles* self)
{	
	psy_ui_styles_updateconfig((psy_ui_Styles*)self);
	return &self->config;
}

void psy_ui_styles_updateconfig(psy_ui_Styles* self)
{
	uintptr_t i;

	psy_property_clear(&self->config);
	if (psy_table_size(&self->styles) > 0) {
		for (i = 0; i <= psy_table_maxkey(&self->styles); ++i) {
			if (psy_table_exists(&self->styles, i)) {
				psy_ui_styles_addstyletoconfig(self, psy_ui_styles_at(self, i), i);
			}
		}
	}
}

void psy_ui_styles_addstyletoconfig(psy_ui_Styles* self, psy_ui_Style* style,
	uintptr_t styletype)
{
	psy_Property* section;	
	char key[128];

	psy_snprintf(key, 128, "style-%d", (int)styletype);
	section = psy_property_append_section(&self->config, key);
	psy_ui_styles_addcolourtoconfig(self, section, "color", style->colour);
	psy_ui_styles_addcolourtoconfig(self, section, "background-color",
		style->backgroundcolour);	
}

void psy_ui_styles_addcolourtoconfig(psy_ui_Styles* self, psy_Property* parent,
	const char* key, psy_ui_Colour colour)
{
	psy_Property* p;
	char_dyn_t* colourset;

	if (colour.mode.set) {
		p = psy_property_append_int(parent, key, psy_ui_colour_colorref(&colour), 0, 0);
	} else {
		psy_property_preventsave(
			psy_property_append_int(parent, key, 0, 0, 0));
	}
	
	colourset = psy_strdup(key);
	colourset = psy_strcat_realloc(colourset, "-set");
	psy_property_preventsave(
		psy_property_append_bool(parent, colourset, colour.mode.set));
	free(colourset);
	colourset = NULL;
}
