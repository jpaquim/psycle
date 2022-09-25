/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uilabel.h"
/* local */
#include "uiapp.h"
#include "uitextdraw.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void psy_ui_label_on_destroyed(psy_ui_Label*);
static void psy_ui_label_on_draw(psy_ui_Label*, psy_ui_Graphics*);
static void psy_ui_label_on_preferred_size(psy_ui_Label*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_label_on_language_changed(psy_ui_Label*);
static void psy_ui_label_on_timer(psy_ui_Label*, uintptr_t timerid);
static const char* psy_ui_label_text_internal(const psy_ui_Label*);
static void psy_ui_label_on_property_changed(psy_ui_Label*, psy_Property* sender);
static void psy_ui_label_before_property_destroyed(psy_ui_Label*, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Label* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_label_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_label_on_draw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_label_on_preferred_size;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_label_on_language_changed;		
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			psy_ui_label_on_timer;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_label_init(psy_ui_Label* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_aligner(&self->component, NULL);
	self->charnumber = 0.0;	
	self->text = NULL;
	self->defaulttext = NULL;
	self->translation = NULL;
	self->translate = TRUE;
	self->fadeoutcounter = 0;
	self->fadeout = FALSE;
	self->property = NULL;
	self->property_display_mode = psy_ui_PROPERTY_MODE_VALUE;
	psy_ui_textformat_init(&self->format);
	psy_ui_textformat_set_alignment(&self->format, psy_ui_textalignment_make(
		psy_ui_ALIGNMENT_CENTER_VERTICAL | psy_ui_ALIGNMENT_LEFT));
	psy_ui_textformat_prevent_wrap(&self->format);
	psy_ui_component_set_style_type_disabled(psy_ui_label_base(self),		
		psy_ui_STYLE_LABEL_DISABLED);
	psy_ui_component_set_style_type_select(psy_ui_label_base(self),
		psy_ui_STYLE_LABEL_SELECT);	
}

void psy_ui_label_init_text(psy_ui_Label* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);

	psy_ui_label_init(self, parent);
	psy_ui_label_set_text(self, text);
}

void psy_ui_label_on_destroyed(psy_ui_Label* self)
{
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
	free(self->text);
	free(self->translation);
	free(self->defaulttext);
	psy_ui_textformat_dispose(&self->format);
}

psy_ui_Label* psy_ui_label_alloc(void)
{
	return (psy_ui_Label*)malloc(sizeof(psy_ui_Label));
}

psy_ui_Label* psy_ui_label_allocinit(psy_ui_Component* parent)
{
	psy_ui_Label* rv;

	rv = psy_ui_label_alloc();
	if (rv) {
		psy_ui_label_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_label_on_language_changed(psy_ui_Label* self)
{
	assert(self);

	if (self->translate) {
		if (self->property && (self->property->item.translate ||
				self->property_display_mode == psy_ui_PROPERTY_MODE_RANGE)) {
			psy_ui_label_on_property_changed(self, self->property);
		} else {
			psy_strreset(&self->translation, psy_ui_translate(self->text));				
			psy_ui_textformat_clear(&self->format);
			psy_ui_component_invalidate(psy_ui_label_base(self));
		}		
	}
}

void psy_ui_label_data_exchange(psy_ui_Label* self, psy_Property* property,
	psy_ui_PropertyDisplayMode mode)
{
	self->property = property;
	self->property_display_mode = mode;
	if (self->property) {
		psy_ui_label_on_property_changed(self, self->property);
		psy_property_connect(self->property, self,
			psy_ui_label_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			psy_ui_label_before_property_destroyed);
	}
}

void psy_ui_label_on_property_changed(psy_ui_Label* self, psy_Property* sender)
{
	if (self->property_display_mode == psy_ui_PROPERTY_MODE_VALUE) {
		if (psy_property_is_font(self->property)) {
			char str[128];
			psy_ui_FontInfo fontinfo;
			psy_ui_Font font;
			int pt;
			const psy_ui_TextMetric* tm;

			psy_ui_fontinfo_init_string(&fontinfo,
				psy_property_item_str(self->property));
			psy_ui_font_init(&font, &fontinfo);
			tm = psy_ui_font_textmetric(&font);
			if (fontinfo.lfHeight < 0) {
				pt = ((tm->tmHeight - tm->tmInternalLeading) * 72) /
					psy_ui_logpixelsy();
			}
			else {
				pt = tm->tmHeight;
			}
			psy_ui_font_dispose(&font);
			tm = NULL;
			psy_snprintf(str, 128, "%s %d pt", fontinfo.lfFaceName, (int)pt);
			psy_ui_label_set_text(self, str);
		}
		else if (psy_property_is_int(sender)) {
			char text[64];

			psy_snprintf(text, 40,
				(psy_property_is_hex(self->property)) ? "%X" : "%d",
				(int)psy_property_item_int(self->property));
			psy_ui_label_set_text(self, text);
		}
		else if (psy_property_is_double(sender)) {
			char text[64];

			psy_snprintf(text, 40, "%f", psy_property_item_double(self->property));
			psy_ui_label_set_text(self, text);
		}
		else if (psy_property_is_string(sender)) {
			psy_ui_label_set_text(self, psy_property_item_str(sender));
		}
	} else if (self->property_display_mode == psy_ui_PROPERTY_MODE_RANGE) {		
		char text[256];
			
		psy_snprintf(text, 256, "%s %d %s %d",
			psy_ui_translate("settings.from"),
			self->property->item.min,
			psy_ui_translate("settings.to"),
			self->property->item.max);		
		psy_ui_label_set_text(self, text);		
	}
}

void psy_ui_label_before_property_destroyed(psy_ui_Label* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void psy_ui_label_set_text(psy_ui_Label* self, const char* text)
{	
	assert(self);	

	psy_strreset(&self->text, text);
	if (self->translate) {
		psy_strreset(&self->translation, psy_translator_translate(
			psy_ui_translator(), text));
	}
	psy_ui_textformat_clear(&self->format);
	psy_ui_component_invalidate(psy_ui_label_base(self));	
}

void psy_ui_label_add_text(psy_ui_Label* self, const char* text)
{
	self->text = psy_strcat_realloc(self->text, text);
	if (self->translate) {
		self->translation = psy_strcat_realloc(self->translation,
			psy_translator_translate(psy_ui_translator(), text));
	}
	psy_ui_textformat_clear(&self->format);
}

void psy_ui_label_set_default_text(psy_ui_Label* self, const char* text)
{
	assert(self);

	psy_strreset(&self->defaulttext, text);	
	psy_ui_component_invalidate(psy_ui_label_base(self));
}

const char* psy_ui_label_text(const psy_ui_Label* self)
{
	assert(self);

	return self->text;		
}

void psy_ui_label_on_preferred_size(psy_ui_Label* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	const char* text;
	const psy_ui_Font* font;
	
	text = psy_ui_label_text_internal(self);
	tm = psy_ui_component_textmetric(psy_ui_label_base(self));
	font = psy_ui_component_font(psy_ui_label_base(self));
	if (psy_strlen(text) == 0) {		
		rv->width = psy_ui_value_make_ew(self->charnumber);		
		rv->height = psy_ui_value_make_px((tm->tmHeight * self->format.linespacing));
		return;
	} else {		
		if (psy_ui_textformat_has_word_wrap(&self->format)) {
			if (self->charnumber == 0 && limit) {
				psy_ui_textformat_update(&self->format, text,
					psy_ui_value_px(&limit->width, tm, limit),
					font, tm);
				rv->width = psy_ui_value_make_px(psy_ui_value_px(&limit->width, tm, NULL));
			} else {
				rv->width = psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber);
				psy_ui_textformat_update(&self->format, text,
					self->charnumber * tm->tmAveCharWidth, font, tm);
			}			
			rv->height = psy_ui_value_make_px(
				psy_ui_textformat_numlines(&self->format) *
				(tm->tmHeight * self->format.linespacing));
		} else {
			if (self->charnumber == 0) {
				psy_ui_Size size;
				const psy_ui_Font* font;

				font = psy_ui_component_font(&self->component);
				if (font) {
					size = psy_ui_font_textsize(font, text, psy_strlen(text));
				} else {
					size = psy_ui_size_zero();
				}
				rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width, tm, NULL));				
			} else {
				rv->width = psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber);
			}
			rv->height = psy_ui_value_make_px((tm->tmHeight * self->format.linespacing));
		}
	}	
}

void psy_ui_label_on_draw(psy_ui_Label* self, psy_ui_Graphics* g)
{
	psy_ui_TextDraw textdraw;
			
	psy_ui_textdraw_init(&textdraw, &self->format,
		psy_ui_component_size_px(psy_ui_label_base(self)),
		psy_ui_label_text_internal(self));
	psy_ui_textdraw_draw(&textdraw, g, psy_INDEX_INVALID);
	psy_ui_textdraw_dispose(&textdraw);
}

void psy_ui_label_set_char_number(psy_ui_Label* self, double number)
{	
	self->charnumber = psy_max(0.0, number);
}

void psy_ui_label_set_line_spacing(psy_ui_Label* self, double line_spacing)
{
	psy_ui_textformat_set_line_spacing(&self->format, line_spacing);	
}

void psy_ui_label_set_text_alignment(psy_ui_Label* self,
	psy_ui_Alignment alignment)
{
	psy_ui_textformat_set_alignment(&self->format,
		psy_ui_textalignment_make(alignment));
}

void psy_ui_label_prevent_translation(psy_ui_Label* self)
{
	self->translate = FALSE;
	psy_strreset(&self->translation, NULL);	
}

void psy_ui_label_enable_translation(psy_ui_Label* self)
{
	self->translate = TRUE;
}

void psy_ui_label_fadeout(psy_ui_Label* self)
{	
	/*
	** Keeps new message 5secs active (Timer interval(50ms) * 100). The counter
	** is decremented each timer tick (on_timer). The first 20 ticks the colour 
	** stays full and then starts to fadeout. At zero the label text is reset
	** to the default text
	*/
	self->fadeoutcounter = 100;
	psy_ui_component_set_colour(psy_ui_label_base(self),
		psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void psy_ui_label_on_timer(psy_ui_Label* self, uintptr_t timerid)
{
	super_vtable.on_timer(&self->component, timerid);
	if (self->fadeoutcounter > 0) {
		psy_ui_Colour colour;
		psy_ui_Colour bgcolour;
		float fadeoutstep;

		--self->fadeoutcounter;
		if (self->fadeoutcounter <= 80) {
			colour = psy_ui_style_const(psy_ui_STYLE_ROOT)->colour;
			bgcolour = psy_ui_component_background_colour(
				psy_ui_label_base(self));
			fadeoutstep = self->fadeoutcounter * 1 / 80.f;
			psy_ui_colour_mul_rgb(&colour, fadeoutstep, fadeoutstep, fadeoutstep);
			if (psy_ui_colour_colorref(&colour) > psy_ui_colour_colorref(&bgcolour)) {
				psy_ui_component_set_colour(psy_ui_label_base(self), colour);
			}
			psy_ui_component_invalidate(psy_ui_label_base(self));
		}
		if (self->fadeoutcounter == 0) {
			/* reset to default text */
			psy_ui_label_set_text(self, self->defaulttext);
			psy_ui_component_set_colour(psy_ui_label_base(self),
				psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
			psy_ui_component_stop_timer(&self->component, 0);
		}
	}
}

void psy_ui_label_prevent_wrap(psy_ui_Label* self)
{
	psy_ui_textformat_prevent_wrap(&self->format);
}

void psy_ui_label_enable_wrap(psy_ui_Label* self)
{
	psy_ui_textformat_line_wrap(&self->format);
	psy_ui_textformat_word_wrap(&self->format);
}

const char* psy_ui_label_text_internal(const psy_ui_Label* self)
{
	if (self->translate && self->translation) {
		return self->translation;
	}
	return self->text;
}
