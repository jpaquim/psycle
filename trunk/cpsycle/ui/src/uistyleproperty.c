/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uiapp.h"
#include "uicomponent.h"
#include "uistyleproperty.h"
/* file */
#include <propertiesio.h>
/* platform */
#include "../../detail/portable.h"

static psy_Property* psy_ui_style_property_make_colour(psy_Property* parent,
	const char* key, psy_ui_Colour*);
static psy_Property* psy_ui_style_property_make_margin(psy_Property* parent,
	const char* key, psy_ui_Margin*);
static psy_Property* psy_ui_style_property_make_border(psy_Property* parent,
	const char* key, psy_ui_Border*);
static psy_Property* psy_ui_style_property_make_border_side(
	psy_Property* parent, const char* key, psy_ui_BorderSide*);
static psy_Property* psy_ui_style_property_make_border_style(
	psy_Property* parent, const char* key, psy_ui_BorderStyle*);
static psy_Property* psy_ui_style_property_make_value(psy_Property* parent,
	const char* key, psy_ui_Value*);
static void psy_ui_style_property_on_value(psy_ui_Value*, psy_Property* sender);
static void psy_ui_style_property_on_int_choice(intptr_t*,
	psy_Property* sender);
static void psy_ui_style_property_on_colour(psy_ui_Colour* value,
	psy_Property* sender);
	
static bool prevent_align = FALSE;
	
static void align(void);

/* styles */
psy_Property* psy_ui_styles_property_make(psy_ui_Styles* styles)
{
	psy_Property* rv;
	
	assert(styles);	
	rv = psy_property_allocinit_key(NULL);
	psy_ui_styles_property_append(rv, styles);
	return rv;
}

void psy_ui_styles_load(psy_Property* styles, const char* path)
{
	psy_PropertyReader reader;
		
	assert(styles);
	
	if (!path) {
		return;
	}
	psy_propertyreader_init(&reader, styles, path);
	prevent_align = TRUE;
	psy_propertyreader_load(&reader);
	prevent_align = FALSE;
	psy_propertyreader_dispose(&reader);
	align();
}

void psy_ui_styles_save(psy_Property* styles, const char* path)
{	
	psy_PropertyWriter writer;

	assert(styles);

	if (!path) {
		return;
	}
	psy_propertywriter_init(&writer, styles, path);
	psy_propertywriter_save(&writer);
	psy_propertywriter_dispose(&writer);		
}

void psy_ui_styles_property_append(psy_Property* parent, psy_ui_Styles* styles)
{
	psy_TableIterator it;

	assert(parent);
	assert(styles);
	
	for (it = psy_table_begin(&styles->styles);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_Style* style;
		
		style = (psy_ui_Style*)psy_tableiterator_value(&it);
		if (psy_strlen(psy_ui_style_name(style)) > 0) {
			psy_ui_style_property_append(parent, style);
		}
	}	
}

/* style */
void psy_ui_style_property_append(psy_Property* parent, psy_ui_Style* style)
{
	psy_Property* p;	
	
	assert(parent);
	assert(style);	
	
	p = psy_property_append_section(parent, psy_ui_style_name(style));	
	psy_ui_style_property_make_colour(p, "colour", &style->colour);
	psy_ui_style_property_make_colour(p, "background-colour",
		&style->background.colour);	
	psy_ui_style_property_make_margin(p, "padding", &style->padding);
	psy_ui_style_property_make_margin(p, "margin", &style->margin);
	psy_ui_style_property_make_border(p, "border", &style->border);
}

psy_Property* psy_ui_style_property_make_colour(psy_Property* parent,
	const char* key, psy_ui_Colour* colour)
{	
	psy_Property* p;
		
	assert(parent);
	assert(colour);
	
	p = psy_property_set_hint(psy_property_append_int(
			parent, key, psy_ui_colour_colorref(colour), 0, 0),
		PSY_PROPERTY_HINT_EDITCOLOR);
	psy_property_connect(p, colour, psy_ui_style_property_on_colour);
}

psy_Property* psy_ui_style_property_make_margin(psy_Property* parent,
	const char* key, psy_ui_Margin* margin)
{
	psy_Property* p;
	
	assert(parent);
	assert(margin);
	
	p = psy_property_append_section(parent, key);
	psy_ui_style_property_make_value(p, "top", &margin->top);
	psy_ui_style_property_make_value(p, "right", &margin->right);
	psy_ui_style_property_make_value(p, "bottom", &margin->bottom);
	psy_ui_style_property_make_value(p, "left", &margin->left);
	return p;
}

psy_Property* psy_ui_style_property_make_border(psy_Property* parent,
	const char* key, psy_ui_Border* border)
{
	psy_Property* p;
	
	assert(parent);
	assert(border);
	
	p = psy_property_append_section(parent, key);
	psy_ui_style_property_make_border_side(p, "top", &border->top);
	psy_ui_style_property_make_border_side(p, "right", &border->right);
	psy_ui_style_property_make_border_side(p, "bottom", &border->bottom);
	psy_ui_style_property_make_border_side(p, "left", &border->left);
	return p;
}

psy_Property* psy_ui_style_property_make_border_side(psy_Property* parent,
	const char* key, psy_ui_BorderSide* side)
{
	psy_Property* p;
	
	assert(parent);
	assert(side);
	
	p = psy_property_append_section(parent, key);	
	psy_ui_style_property_make_border_style(p, "style", &side->style);
	psy_ui_style_property_make_colour(p, "colour", &side->colour);
	psy_ui_style_property_make_value(p, "radius", &side->radius);
	psy_ui_style_property_make_value(p, "width", &side->width);
	return p;
}

psy_Property* psy_ui_style_property_make_border_style(psy_Property* parent,
	const char* key, psy_ui_BorderStyle* style)
{
	psy_Property* p;
	
	assert(parent);
	assert(style);
	
	p = psy_property_append_choice(parent, key, (intptr_t)(*style));
	psy_property_set_hint(p, PSY_PROPERTY_HINT_COMBO);	
	psy_property_append_int(p, "none", 0, 0, 0);	
	psy_property_append_int(p, "solid", 1, 0, 0);
	psy_property_connect(p, style, psy_ui_style_property_on_int_choice);
	return p;
}

psy_Property* psy_ui_style_property_make_value(psy_Property* parent,
	const char* key, psy_ui_Value* value)
{
	char text[64];
	
	assert(parent);
	assert(value);
	
	return psy_property_connect(psy_property_append_str(parent, key,
		psy_ui_value_to_string(value, text)), value,
		psy_ui_style_property_on_value);
}

void psy_ui_style_property_on_value(psy_ui_Value* value, psy_Property* sender)
{
	assert(value);
	assert(sender);
	
	*value = psy_ui_value_make_string(psy_property_item_str(sender));
	align();
}

void psy_ui_style_property_on_int_choice(intptr_t* value, psy_Property* sender)
{
	psy_Property* item;
	
	assert(value);
	assert(sender);

	item = psy_property_at_choice(sender);
	if (item && psy_property_is_int(item)) {
		*value = psy_property_item_int(item);
	}
	align();
}

void psy_ui_style_property_on_colour(psy_ui_Colour* value, psy_Property* sender)
{	
	assert(value);
	assert(sender);
	
	*value = psy_ui_colour_make(psy_property_item_int(sender));	
	psy_ui_component_invalidate(psy_ui_app_main(psy_ui_app()));
}

void align(void)
{
	if (!prevent_align) {
		psy_ui_component_align_full(psy_ui_app_main(psy_ui_app()));
		psy_ui_component_invalidate(psy_ui_app_main(psy_ui_app()));
	}
}
