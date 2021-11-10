/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uilabel.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_label_ondestroy(psy_ui_Label*);
static void psy_ui_label_ondraw(psy_ui_Label*, psy_ui_Graphics*);
static void psy_ui_label_onpreferredsize(psy_ui_Label*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_label_onlanguagechanged(psy_ui_Label*);
static void psy_ui_label_ontimer(psy_ui_Label*, uintptr_t timerid);
static psy_List* psy_ui_label_lines(psy_ui_Label*, const char* text, double width);
static psy_List* psy_ui_label_eols(psy_ui_Label*);
static psy_List* psy_ui_label_wraps(psy_ui_Label*, const char* text, uintptr_t num,
	double width);
static void psy_ui_label_onupdatestyles(psy_ui_Label*);
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
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			psy_ui_label_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_label_ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_label_onpreferredsize;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_label_onlanguagechanged;
		vtable.onupdatestyles =
			(psy_ui_fp_component_event)
			psy_ui_label_onupdatestyles;
		vtable.ontimer = (psy_ui_fp_component_ontimer)
			psy_ui_label_ontimer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &vtable);
}
/* implementation */
void psy_ui_label_init(psy_ui_Label* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);	
	vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);
	self->charnumber = 0;
	self->linespacing = 1.0;
	self->textalignment = psy_ui_ALIGNMENT_CENTER_VERTICAL |
		psy_ui_ALIGNMENT_LEFT;
	self->text = NULL;
	self->defaulttext = NULL;
	self->translation = NULL;
	self->translate = TRUE;
	self->fadeoutcounter = 0;
	self->fadeout = FALSE;
	self->preventwrap = TRUE;
	psy_ui_component_setstyletypes(psy_ui_label_base(self),
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_ui_STYLE_LABEL_DISABLED);
}

void psy_ui_label_init_text(psy_ui_Label* self, psy_ui_Component* parent,
	psy_ui_Component* view, const char* text)
{
	assert(self);

	psy_ui_label_init(self, parent, view);
	psy_ui_label_settext(self, text);
}

psy_ui_Label* psy_ui_label_alloc(void)
{
	return (psy_ui_Label*)malloc(sizeof(psy_ui_Label));
}

psy_ui_Label* psy_ui_label_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_Label* rv;

	rv = psy_ui_label_alloc();
	if (rv) {
		psy_ui_label_init(rv, parent, view);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void psy_ui_label_ondestroy(psy_ui_Label* self)
{
	assert(self);
	
	free(self->text);
	free(self->translation);
	free(self->defaulttext);
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

void psy_ui_label_setdefaulttext(psy_ui_Label* self, const char* text)
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

void psy_ui_label_onpreferredsize(psy_ui_Label* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	char* text;
	psy_ui_Margin spacing;

	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	tm = psy_ui_component_textmetric(psy_ui_label_base(self));
	spacing = psy_ui_component_spacing(psy_ui_label_base(self));
	if (self->charnumber == 0) {		
		if (psy_strlen(text) == 0) {
			rv->width = psy_ui_value_make_ew(0.0);
		} else {
			psy_ui_Size size;
			
			size = psy_ui_component_textsize(psy_ui_label_base(self), text);						
			rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width, tm, NULL));
		}		
	} else {
		rv->width = psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber);
	}
	rv->height = psy_ui_value_make_px((tm->tmHeight * self->linespacing));
	rv->height = psy_ui_add_values(rv->height, psy_ui_margin_height(&spacing, tm, NULL), tm, NULL);
	rv->width = psy_ui_add_values(rv->width, psy_ui_margin_width(&spacing, tm, NULL), tm, NULL);
}

void psy_ui_label_ondraw(psy_ui_Label* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	double centerx;
	double centery;	
	double cpy;
	char* text;
	uintptr_t cp;
	psy_List* p;
	psy_List* lines;
	uintptr_t linestart;

	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	if (psy_strlen(text) == 0) {
		return;
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size_px(psy_ui_label_base(self));
	centerx = 0.0;
	if ((self->textalignment & psy_ui_ALIGNMENT_RIGHT) ==
			psy_ui_ALIGNMENT_RIGHT) {
		psy_ui_Size textsize;
		psy_ui_RealSize textsizepx;

		textsize = psy_ui_textsize(g, text, psy_strlen(text));
		textsizepx = psy_ui_size_px(&textsize, tm, NULL);
		centerx = (size.width - textsizepx.width);
	}
	lines = psy_ui_label_lines(self, text, size.width);
	if (!lines) {
		return;
	}
	centery = 0.0;
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
			psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		centery = (size.height - tm->tmHeight * psy_list_size(lines)) / 2;
	}
	cpy = centery;
	linestart = 0;
	for (p = lines; p != NULL; p = p->next) {
		cp = (uintptr_t)p->entry;		
		if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
				psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
			if (self->preventwrap) {
				psy_ui_Size textsize;
				psy_ui_RealSize textsizepx;

				textsize = psy_ui_textsize(g, text, psy_strlen(text));
				textsizepx = psy_ui_size_px(&textsize, tm, NULL);
				centerx = (size.width - textsizepx.width) / 2.0;
			} else {
				centerx = (size.width - (cp - linestart) * tm->tmAveCharWidth) / 2;
			}
		}		
		psy_ui_textout(g, centerx, cpy, text + linestart, cp - linestart);
		linestart = cp + 1;
		cpy += tm->tmHeight;
	}
	psy_list_free(lines);
}

psy_List* psy_ui_label_lines(psy_ui_Label* self, const char* text, double width)
{
	psy_List* rv;
	psy_List* eols;	

	rv = NULL;
	if (!self->preventwrap) {
		psy_List* p;
		uintptr_t cp;		

		eols = psy_ui_label_eols(self);
		psy_list_append(&eols, (void*)psy_strlen(text));
		cp = 0;
		for (p = eols; p != NULL; p = p->next) {
			uintptr_t eol;
			uintptr_t numout;
			psy_List* wraps;
			psy_List* q;

			eol = (uintptr_t)p->entry;
			numout = eol - cp;
			if (!self->preventwrap) {
				uintptr_t wp;

				wraps = psy_ui_label_wraps(self, text + cp, numout, width);
				wp = 0;
				for (q = wraps; q != NULL; q = q->next) {
					uintptr_t wrap;

					wrap = (uintptr_t)q->entry;
					numout = wrap - wp;
					psy_list_append(&rv, (void*)(cp + wp + numout));
					wp = wrap + 1;
				}
				psy_list_free(wraps);
			} else {
				psy_list_append(&rv, (void*)(cp + numout));
			}
			cp = eol;
		}
		psy_list_free(eols);
		eols = NULL;		
	}
	if (!rv) {
		psy_list_append(&rv, (void*)psy_strlen(text));
	}
	return rv;
}

psy_List* psy_ui_label_eols(psy_ui_Label* self)
{
	psy_List* rv;
	char* text;	
	uintptr_t num;
	uintptr_t cp;
		
	rv = NULL;
	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	num = psy_strlen(text);
	if (num == 0) {
		return NULL;
	}
	for (cp = 0; cp < num; ++cp) {
		if (text[cp] == '\n') {
			psy_list_append(&rv, (void*)cp);
		}
	}		
	return rv;
}

psy_List* psy_ui_label_wraps(psy_ui_Label* self, const char* text,
	uintptr_t num, double width)
{
	psy_List* rv;
	const psy_ui_TextMetric* tm;
	uintptr_t numavgchars;
	uintptr_t cp;	

	rv = NULL;
	tm = psy_ui_component_textmetric(&self->component);
	numavgchars = (uintptr_t)(width / (tm->tmAveCharWidth * 1.4));
	if (numavgchars == 0) {
		return rv;
	}
	cp = 0;		
	while (cp < num) {
		uintptr_t linestart;

		linestart = cp;
		cp += numavgchars;
		if (cp > num - linestart) {
			psy_list_append(&rv, (void*)num);
			return rv;
		}		
		while (cp > linestart + 1) {
			if (text[cp] == ' ') {				
				break;
			}
			--cp;
		}
		if (cp > linestart + 1) {
			psy_list_append(&rv, (void*)cp);
		} else {
			cp += numavgchars;
			cp = psy_min(num, cp);
			psy_list_append(&rv, (void*)cp);
		}				
	}
	return rv;
}

void psy_ui_label_setcharnumber(psy_ui_Label* self, double number)
{	
	self->charnumber = psy_max(0.0, number);
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
	psy_strreset(&self->translation, NULL);	
}

void psy_ui_label_enabletranslation(psy_ui_Label* self)
{
	self->translate = TRUE;
}

void psy_ui_label_fadeout(psy_ui_Label* self)
{
	self->fadeout = TRUE;
	/*
	** Keeps new message 5secs active (Timer interval(50ms) * 100). The counter
	** is decremented each timer tick (ontimer). The first 20 ticks the colour 
	** stays full and then starts to fadeout. At zero the label text is reset
	** to the default text
	*/
	self->fadeoutcounter = 100;
	psy_ui_component_setcolour(psy_ui_label_base(self),
		psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void psy_ui_label_ontimer(psy_ui_Label* self, uintptr_t timerid)
{
	super_vtable.ontimer(&self->component, timerid);
	if (self->fadeoutcounter > 0) {
		psy_ui_Colour colour;
		psy_ui_Colour bgcolour;
		float fadeoutstep;

		--self->fadeoutcounter;
		if (self->fadeoutcounter <= 80) {
			colour = psy_ui_style_const(psy_ui_STYLE_ROOT)->colour;
			bgcolour = psy_ui_component_backgroundcolour(
				psy_ui_label_base(self));
			fadeoutstep = self->fadeoutcounter * 1 / 80.f;
			psy_ui_colour_mul_rgb(&colour, fadeoutstep, fadeoutstep, fadeoutstep);
			if (psy_ui_colour_colorref(&colour) > psy_ui_colour_colorref(&bgcolour)) {
				psy_ui_component_setcolour(psy_ui_label_base(self), colour);
			}
			psy_ui_component_invalidate(psy_ui_label_base(self));
		}
		if (self->fadeoutcounter == 0) {
			/* reset to default text */
			psy_ui_label_settext(self, self->defaulttext);
			psy_ui_component_setcolour(psy_ui_label_base(self),
				psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
			psy_ui_component_stoptimer(&self->component, 0);
		}
	}
}

void psy_ui_label_preventwrap(psy_ui_Label* self)
{
	self->preventwrap = TRUE;
}

void psy_ui_label_enablewrap(psy_ui_Label* self)
{
	self->preventwrap = FALSE;
}

void psy_ui_label_onupdatestyles(psy_ui_Label* self)
{	
	self->component.style.overridestyle.colour.mode.set = FALSE;		
}
