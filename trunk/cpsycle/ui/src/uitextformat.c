/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uitextformat.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static psy_List* psy_ui_textformat_eols(psy_ui_TextFormat*, const char* text);
static psy_List* psy_ui_textformat_wraps(psy_ui_TextFormat*, const char* text,
	uintptr_t num, double width, const psy_ui_TextMetric*);

/* implementation */
void psy_ui_textformat_init(psy_ui_TextFormat* self)
{
	assert(self);
		
	psy_table_init(&self->lines);
	self->wrap = TRUE;
	self->width = 0.0;
	self->avgcharwidth = 0.0;
	self->textheight = 19;
	self->linespacing = 1.0;	
	self->textalignment = psy_ui_textalignment_make(psy_ui_ALIGNMENT_LEFT);
}

void psy_ui_textformat_dispose(psy_ui_TextFormat* self)
{
	psy_table_dispose(&self->lines);	
	self->width = self->avgcharwidth = 0.0;
}

void psy_ui_textformat_clear(psy_ui_TextFormat* self)
{
	psy_table_clear(&self->lines);	
}

void psy_ui_textformat_update(psy_ui_TextFormat* self, const char* text,
	double width, const psy_ui_TextMetric* tm)
{
	if (psy_table_size(&self->lines) > 0 && (self->width != width ||
			self->avgcharwidth != tm->tmAveCharWidth)) {
		psy_ui_textformat_clear(self);
	}
	self->width = width;
	self->textheight = tm->tmHeight;	
	self->avgcharwidth = tm->tmAveCharWidth;	
	if (psy_table_size(&self->lines) == 0) {		
		psy_List* eols;
		
		if (self->wrap) {
			psy_List* p;
			uintptr_t cp;
			uintptr_t line;

			eols = psy_ui_textformat_eols(self, text);
			psy_list_append(&eols, (void*)psy_strlen(text));
			cp = 0;
			for (p = eols, line = 0; p != NULL; p = p->next) {
				uintptr_t eol;
				uintptr_t numout;
				psy_List* wraps;
				psy_List* q;

				eol = (uintptr_t)p->entry;
				numout = eol - cp;
				if (self->wrap) {
					uintptr_t wp;

					wraps = psy_ui_textformat_wraps(self, text + cp, numout, width, tm);
					wp = 0;
					for (q = wraps; q != NULL; q = q->next) {
						uintptr_t wrap;

						wrap = (uintptr_t)q->entry;
						numout = wrap - wp;
						psy_table_insert(&self->lines, line,
							(void*)(cp + wp + numout));
						++line;
						wp = wrap;
					}
					psy_list_free(wraps);
				} else {
					psy_table_insert(&self->lines, line,
						(void*)(cp + numout));
					++line;
				}
				cp = eol;
			}
			psy_list_free(eols);
			eols = NULL;
		}
		if (psy_table_size(&self->lines) == 0) {
			psy_table_insert(&self->lines, 0, (void*)psy_strlen(text));
		}		
	}	
}

psy_List* psy_ui_textformat_eols(psy_ui_TextFormat* self, const char* text)
{
	psy_List* rv;	
	uintptr_t num;
	uintptr_t cp;
		
	rv = NULL;	
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

psy_List* psy_ui_textformat_wraps(psy_ui_TextFormat* self, const char* text,
	uintptr_t num, double width, const psy_ui_TextMetric* tm)
{
	psy_List* rv;	
	uintptr_t numavgchars;
	uintptr_t cp;	

	rv = NULL;	
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
			char c;

			c = text[cp];
			if (c == ' ') {				
				break;
			}
			--cp;
		}
		if (cp < linestart) {
			cp += numavgchars;
			cp = psy_min(num, cp);						
		}
		psy_list_append(&rv, (void*)(cp));
	}
	return rv;
}

uintptr_t psy_ui_textformat_line_at(const psy_ui_TextFormat* self,
	uintptr_t index)
{
	if (psy_table_exists(&self->lines, index)) {
		return (uintptr_t)psy_table_at_const(&self->lines, index);
	}
	return psy_INDEX_INVALID;
}

uintptr_t psy_ui_textformat_numlines(const psy_ui_TextFormat* self)
{
	return psy_table_size(&self->lines);
}

uintptr_t psy_ui_textformat_cursor_position(const psy_ui_TextFormat* self,
	const char* text, psy_ui_RealPoint pt, const psy_ui_TextMetric* tm,
	const psy_ui_Font* font)
{		
	uintptr_t cp;
	uintptr_t line;
	uintptr_t linestart;
	uintptr_t lineend;
	double x;	
	
	line = (uintptr_t)(pt.y / tm->tmHeight);
	if (line > 0) {
		linestart = psy_ui_textformat_line_at(self, line - 1) + 1;
	} else {
		linestart = 0;
	}
	lineend = psy_ui_textformat_line_at(self, line);
	x = 0.0;
	cp = linestart;	
	while (x < pt.x && cp < lineend) {
		psy_ui_Size textsize;
		psy_ui_RealSize textsizepx;

		textsize = psy_ui_font_textsize(font, text + linestart, cp - linestart);
		textsizepx = psy_ui_size_px(&textsize, tm, NULL);
		x = textsizepx.width;
		if (x >= pt.x - (tm->tmAveCharWidth)) {
			break;
		}
		++cp;
	}
	return cp;
}