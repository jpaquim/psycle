/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uitextdraw.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_textdraw_draw_single_line(psy_ui_TextDraw*, psy_ui_Graphics*,
	const psy_ui_TextMetric* tm);
static void psy_ui_textdraw_drawcursor(psy_ui_TextDraw*, psy_ui_Graphics*,
	const psy_ui_TextMetric*, double cpy, uintptr_t linestart, uintptr_t cp);
static double psy_ui_textdraw_center_x(const psy_ui_TextDraw*,
	double width, const char* text, uintptr_t count, const psy_ui_Font*,
	const psy_ui_TextMetric*);
static double psy_ui_textdraw_center_y(const psy_ui_TextDraw*,
	double height, uintptr_t numlines, const psy_ui_Font*,
	const psy_ui_TextMetric*);

/* implementation */
void psy_ui_textdraw_init(psy_ui_TextDraw* self, psy_ui_TextFormat* format, psy_ui_RealSize size,
	const char* text)
{	
	self->format = format;
	self->text = text;
	self->size = size;	
}

void psy_ui_textdraw_dispose(psy_ui_TextDraw* self)
{
}

void psy_ui_textdraw_draw(psy_ui_TextDraw* self, psy_ui_Graphics* g,
	uintptr_t cursorpos)
{		
	psy_ui_RealPoint center;		
	uintptr_t cp;	
	intptr_t line;
	uintptr_t numlines;
	uintptr_t linestart;
	double lineheight;
	psy_ui_RealRectangle clip;	
	const psy_ui_TextMetric* tm;

	tm = psy_ui_font_textmetric(psy_ui_font(g));
	if (!psy_ui_textformat_has_wrap(self->format)) {
		psy_ui_textdraw_draw_single_line(self, g, tm);
		if (cursorpos >= 0 && cursorpos <= psy_strlen(self->text)) {			
			psy_ui_textdraw_drawcursor(self, g, tm,
				psy_ui_textdraw_center_y(self, self->size.height, 1, psy_ui_font(g), tm),
				0, cursorpos);
		}
		return;
	}	
	if (psy_strlen(self->text) == 0) {
		return;
	}		
	psy_ui_textformat_update(self->format, self->text, self->size.width, psy_ui_font(g), tm);
	numlines = psy_ui_textformat_numlines(self->format);
	if (numlines == 0) {
		return;
	}	
	center.y = psy_ui_textdraw_center_y(self, self->size.height, numlines, psy_ui_font(g), tm);
	clip = psy_ui_graphics_cliprect(g);
	lineheight = (self->format->linespacing * tm->tmHeight);
	line = (intptr_t)(clip.top / lineheight);
	if (line > 0 && line < (intptr_t)numlines) {
		linestart = psy_ui_textformat_line_at(self->format, line - 1) + 1;		
	} else {
		linestart = 0;		
	}
	center.y += (line * lineheight);
	numlines = psy_min(numlines, ((uintptr_t)(floor(clip.bottom) / lineheight)) + 1);
	for (; line < (intptr_t)numlines; ++line) {
		if (line >= 0) {
			cp = psy_ui_textformat_line_at(self->format, line);
			center.x = psy_ui_textdraw_center_x(self, self->size.width,
				self->text + linestart, cp - linestart, psy_ui_font(g), tm);
			psy_ui_textout(g, center, self->text + linestart, cp - linestart);
			if (cursorpos >= linestart && cursorpos <= cp) {
				psy_ui_textdraw_drawcursor(self, g, tm, center.y, linestart, cursorpos);				
			}
			linestart = cp + 1;
		}
		center.y += lineheight;		
	}	
}

void psy_ui_textdraw_drawcursor(psy_ui_TextDraw* self, psy_ui_Graphics* g,
	const psy_ui_TextMetric* tm, double cpy, uintptr_t linestart, uintptr_t cp)
{	
		psy_ui_Size textsize;
		double x;		
				
		textsize = psy_ui_textsize(g, self->text + linestart, cp - linestart);
		x = psy_ui_value_px(&textsize.width, tm, NULL);
		psy_ui_drawline(g,
			psy_ui_realpoint_make(x, cpy),
			psy_ui_realpoint_make(x, cpy + tm->tmHeight));	
}


void psy_ui_textdraw_draw_single_line(psy_ui_TextDraw* self, psy_ui_Graphics* g,
	const psy_ui_TextMetric* tm)
{	
	uintptr_t numchars;
	const psy_ui_Font* font;
	
	numchars = psy_strlen(self->text);
	if (numchars == 0) {
		return;
	}	
	font = psy_ui_font(g);
	if (!font) {
		return;
	}
	psy_ui_textout(g, psy_ui_realpoint_make(
		psy_ui_textdraw_center_x(self, self->size.width, self->text, numchars, font, tm),
		psy_ui_textdraw_center_y(self, self->size.height, 1, font, tm)),
		self->text, numchars);	
}

double psy_ui_textdraw_center_x(const psy_ui_TextDraw* self,
	double width, const char* text, uintptr_t count, const psy_ui_Font* font,
	const psy_ui_TextMetric* tm)
{
	if (count > 0 && font) {		
		if (psy_ui_textalignment_horizontal_centered(&self->format->textalignment)) {
			psy_ui_Size textsize;
			psy_ui_RealSize textsizepx;			
			
			textsize = psy_ui_font_textsize(font, text, count);
			textsizepx = psy_ui_size_px(&textsize, tm, NULL);
			return (width - textsizepx.width) / 2.0;
		} else if (psy_ui_textalignment_right(&self->format->textalignment)) {			
			psy_ui_Size textsize;
			psy_ui_RealSize textsizepx;			
			
			textsize = psy_ui_font_textsize(font, text, count);
			textsizepx = psy_ui_size_px(&textsize, tm, NULL);
			return (width - textsizepx.width);			
		}
	}
	return 0.0;
}

double psy_ui_textdraw_center_y(const psy_ui_TextDraw* self, double height,
	uintptr_t numlines, const psy_ui_Font* font,
	const psy_ui_TextMetric* tm)
{	
	if (psy_ui_textalignment_vertical_centered(&self->format->textalignment)) {		
		return (height - self->format->linespacing * tm->tmHeight * numlines) / 2.0;
	}
	return 0.0;
}
