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
	uintptr_t num, double width, const psy_ui_Font* font,
	const psy_ui_TextMetric*);
static uintptr_t psy_ui_textformat_numchars(const psy_ui_TextFormat*,
	const char* text, double width, const psy_ui_Font* font,
	const psy_ui_TextMetric* tm);

/* implementation */
void psy_ui_textformat_init(psy_ui_TextFormat* self)
{
	assert(self);
		
	psy_table_init(&self->lines);
	self->word_wrap = TRUE;
	self->line_wrap = TRUE;
	self->width = 0.0;
	self->avgcharwidth = 0.0;
	self->nummaxchars = 0;
	self->textheight = 19;
	self->linespacing = 1.0;	
	self->textalignment = psy_ui_textalignment_make(psy_ui_ALIGNMENT_LEFT);
	self->numavgchars = 0;
}

void psy_ui_textformat_dispose(psy_ui_TextFormat* self)
{
	psy_table_dispose(&self->lines);	
	self->width = self->avgcharwidth = 0.0;
}

void psy_ui_textformat_clear(psy_ui_TextFormat* self)
{
	psy_table_clear(&self->lines);	
	self->numavgchars = 0;
}

void psy_ui_textformat_update(psy_ui_TextFormat* self, const char* text,
	double width, const psy_ui_Font* font, const psy_ui_TextMetric* tm)
{
	if (psy_table_size(&self->lines) > 0 && (self->width != width ||
			self->avgcharwidth != tm->tmAveCharWidth)) {
		psy_ui_textformat_clear(self);
	}
	self->width = width;
	self->textheight = tm->tmHeight;	
	self->avgcharwidth = tm->tmAveCharWidth;	
	if (psy_table_size(&self->lines) == 0) {		
		if (self->line_wrap || self->word_wrap) {
			psy_List* eols;
			psy_List* p;
			uintptr_t cp;
			uintptr_t line;

			eols = psy_ui_textformat_eols(self, text);
			psy_list_append(&eols, (void*)psy_strlen(text));
			cp = 0;
			self->nummaxchars = 0;
			if (self->line_wrap) {
				for (p = eols, line = 0; p != NULL; p = p->next) {
					uintptr_t eol;
					uintptr_t numout;

					eol = (uintptr_t)p->entry;
					numout = eol - cp;
					self->nummaxchars = psy_max(self->nummaxchars, numout);
					if (self->word_wrap) {
						psy_List* wraps;
						psy_List* q;
						uintptr_t wp;

						wraps = psy_ui_textformat_wraps(self, text + cp, numout,
							width, font, tm);
						for (q = wraps, wp = 0; q != NULL; q = q->next) {
							uintptr_t word_wrap;

							word_wrap = (uintptr_t)q->entry;
							numout = word_wrap - wp;
							psy_table_insert(&self->lines, line,
								(void*)(cp + wp + numout));
							++line;
							wp = word_wrap;
						}
						psy_list_free(wraps);
						wraps = NULL;
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
		}
		if (psy_ui_textformat_numlines(self) == 0) {
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
	uintptr_t num, double width, const psy_ui_Font* font, const psy_ui_TextMetric* tm)
{
	psy_List* rv;	
	uintptr_t cp;	

	rv = NULL;	
	cp = 0;		
	while (cp < num) {
		uintptr_t linestart;
		uintptr_t numchars;
		linestart = cp;

		numchars = psy_ui_textformat_numchars(self, text + linestart, width, font, tm);
		if (numchars == 0) {
			return rv;
		}
		cp += numchars;
		if (cp >= num) {
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
			cp += numchars;
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


uintptr_t psy_ui_textformat_numchars(const psy_ui_TextFormat* self,
	const char* text, double width, const psy_ui_Font* font,
	const psy_ui_TextMetric* tm)
{	
	uintptr_t i;
	double curr_width;
	double avg_char_width;	
	
	avg_char_width = (tm->tmAveCharWidth * 1.3);
	for (i = 0, curr_width = 0.0; text[i] != '\0' && curr_width < width; ++i) {
		uintptr_t len;
		
		len = psy_strlen(text);
		((psy_ui_TextFormat*)self)->numavgchars = psy_min(
			(uintptr_t)(width / (tm->tmAveCharWidth * 1.3)), len);
		if (text[i] == '/t') {			
			curr_width += (avg_char_width * 4);
		} else {
			curr_width += avg_char_width;
		}
	}
	return i;
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
		uintptr_t len;
		char_dyn_t* str;

		str = psy_ui_textformat_fill_tabs(text + linestart, cp - linestart, &len);
		textsize = psy_ui_font_textsize(font, str, len);
		free(str);
		textsizepx = psy_ui_size_px(&textsize, tm, NULL);
		x = textsizepx.width;
		if (x >= pt.x - (tm->tmAveCharWidth)) {
			break;
		}
		++cp;
	}
	return cp;
}

double psy_ui_textformat_screen_offset(const psy_ui_TextFormat* self,
	const char* text, uintptr_t count, const psy_ui_Font* font,
	const psy_ui_TextMetric* tm)
{	
	psy_ui_Size textsize;
	psy_ui_RealSize textsizepx;

	textsize = psy_ui_font_textsize(font, text, count);
	textsizepx = psy_ui_size_px(&textsize, tm, NULL);
	return textsizepx.width;
}

char_dyn_t* psy_ui_textformat_fill_tabs(const char* text, uintptr_t len, uintptr_t* rv_len)
{
	char_dyn_t* str;

	str = 0;
	*rv_len = 0;
	if (len) {
		uintptr_t src_cp;
		uintptr_t cp;

		str = (char*)malloc(len * 4);
		src_cp = 0;
		cp = 0;
		while (src_cp < len && text[src_cp] != '\0') {
			if (text[src_cp] == '\t') {
				uintptr_t i;

				for (i = 0; i < 4; ++i, ++cp) {
					str[cp] = ' ';
				}
			} else {
				str[cp] = text[src_cp];
				++cp;
			}
			++src_cp;
		}
		*rv_len = cp;
	}
	return str;
}
