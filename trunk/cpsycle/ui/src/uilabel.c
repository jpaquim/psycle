// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilabel.h"
// std
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void psy_ui_label_ondestroy(psy_ui_Label*);
static void psy_ui_label_ondraw(psy_ui_Label*, psy_ui_Graphics*);
static void psy_ui_label_onpreferredsize(psy_ui_Label*, psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_label_onlanguagechanged(psy_ui_Label*);

static char* strrchrpos(char* str, char c, uintptr_t pos);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(psy_ui_Label* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			psy_ui_label_ondestroy;
		vtable.ondraw = (psy_ui_fp_component_ondraw)
			psy_ui_label_ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_label_onpreferredsize;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_label_onlanguagechanged;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void psy_ui_label_init(psy_ui_Label* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));
	psy_ui_component_doublebuffer(&self->component);
	self->charnumber = 0;
	self->linespacing = 1.0;
	self->textalignment = psy_ui_ALIGNMENT_CENTER_VERTICAL |
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL;
	self->text = NULL;
	self->translation = NULL;
	self->translate = TRUE;		
}

void psy_ui_label_init_text(psy_ui_Label* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);

	psy_ui_label_init(self, parent);
	psy_ui_label_settext(self, text);
}

void psy_ui_label_ondestroy(psy_ui_Label* self)
{
	assert(self);
	
	free(self->text);
	free(self->translation);
}

void psy_ui_label_onlanguagechanged(psy_ui_Label* self)
{
	assert(self);

	psy_strreset(&self->translation, psy_ui_translate(self->text));
	psy_ui_component_invalidate(psy_ui_label_base(self));
}

void psy_ui_label_settext(psy_ui_Label* self, const char* text)
{	
	assert(self);	

	psy_strreset(&self->text, text);
	if (self->translate) {
		psy_strreset(&self->translation, psy_translator_translate(
			psy_ui_translator(), text));
	}
	psy_ui_component_invalidate(psy_ui_label_base(self));
}

void psy_ui_label_text(psy_ui_Label* self, char* text)
{
	assert(self);

	if (self->text) {
		psy_snprintf(self->text, 256, "%s", text);
	} else {
		text[0] = '\0';
	}
}

void psy_ui_label_onpreferredsize(psy_ui_Label* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	char* text;

	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	tm = psy_ui_component_textmetric(psy_ui_label_base(self));	
	if (self->charnumber == 0) {		
		if (psy_strlen(self->text) == 0) {
			rv->width = psy_ui_value_makepx(0);
		} else {
			psy_ui_Size size;

			size = psy_ui_component_textsize(psy_ui_label_base(self),
				text);
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, tm) + 4 +
				psy_ui_margin_width_px(&psy_ui_label_base(self)->spacing, tm));
		}		
	} else {
		rv->width = psy_ui_value_makepx(tm->tmAveCharWidth * self->charnumber);
	}
	rv->height = psy_ui_value_makepx((tm->tmHeight * self->linespacing) +
		psy_ui_margin_height_px(&psy_ui_label_base(self)->spacing, tm));
}

void psy_ui_label_ondraw(psy_ui_Label* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	
	double centerx = 0.0;
	double centery = 0.0;
	uintptr_t count;
	char seps[] = "\n";
	char* token;
	char* string;
	uintptr_t numcolumnavgchars;
	char* text;	
		
	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	if (psy_strlen(text) == 0) {
		return;
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_sizepx(psy_ui_label_base(self));
		
	//psy_ui_textout(g, 0, 0, self->text, strlen(self->text));
	//return;
	if (size.height >= tm->tmHeight * 2) {
		numcolumnavgchars = (int)(size.width / tm->tmAveCharWidth);
	} else {
		numcolumnavgchars = UINTPTR_MAX;
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) == psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		centery = (size.height - tm->tmHeight) / 2;
	}
	string = malloc(strlen(text) + 1);
	psy_snprintf(string, strlen(text) + 1, "%s", text);
	token = strtok(string, seps);
	while (token != NULL) {
		count = strlen(token);
		while (count > 0) {
			uintptr_t numoutput;
			char* wrap;

			numoutput = psy_min(numcolumnavgchars, count);
			if (numoutput < count) {
				wrap = strrchrpos((char*)token, ' ', numoutput);
				if (wrap) {
					++wrap;					
					numoutput = wrap - token;
				}
			}
			if (numoutput == 0) {
				break;
			}
			psy_ui_textout(g, centerx, centery, token, numoutput);
			centery += tm->tmHeight;
			count -= numoutput;
			token += numoutput;
			if (centery + tm->tmHeight >= size.height) {
				numcolumnavgchars = UINTPTR_MAX;
			}
		}
		token = strtok(NULL, seps);
	}
	free(string);
}

void psy_ui_label_setcharnumber(psy_ui_Label* self, double number)
{
	self->charnumber = number;
}

void psy_ui_label_setlinespacing(psy_ui_Label* self, double spacing)
{
	self->linespacing = spacing;
}

void psy_ui_label_settextalignment(psy_ui_Label* self, psy_ui_Alignment alignment)
{
	self->textalignment = alignment;
}

void psy_ui_label_preventtranslation(psy_ui_Label* self)
{
	self->translate = FALSE;
	if (self->translation) {
		free(self->translation);
		self->translation = NULL;
	}
}

char* strrchrpos(char* str, char c, uintptr_t pos)
{
	uintptr_t count;

	if (pos >= strlen(str)) {
		return 0;
	}
	count = pos;
	while (1) {
		if (str[count] == c) {
			return str + count;
		}
		if (count == 0) {
			break;
		}
		--count;
	}
	return 0;
}
