// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEF_H
#define psy_ui_DEF_H

#include "../../detail/prefix.h"
#include "../../detail/stdint.h"
#include <stddef.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined min
#undef min
#endif

typedef struct psy_ui_TextMetric
{
    int32_t tmHeight;
    int32_t tmAscent;
    int32_t tmDescent;
    int32_t tmInternalLeading;
    int32_t tmExternalLeading;
    int32_t tmAveCharWidth;
    int32_t tmMaxCharWidth;
    int32_t tmWeight;
    int32_t tmOverhang;
    int32_t tmDigitizedAspectX;
    int32_t tmDigitizedAspectY;
    uint8_t tmFirstChar;
    uint8_t tmLastChar;
    uint8_t tmDefaultChar;
    uint8_t tmBreakChar;
    uint8_t tmItalic;
    uint8_t tmUnderlined;
    uint8_t tmStruckOut;
    uint8_t tmPitchAndFamily;
    uint8_t tmCharSet;
} psy_ui_TextMetric;

typedef enum {
	psy_ui_UNIT_EH,
	psy_ui_UNIT_EW,
	psy_ui_UNIT_PX,
	psy_ui_UNIT_PE
} psy_ui_Unit;

typedef struct {
	union {
		intptr_t integer;
		double real;
	} quantity;
	psy_ui_Unit unit;
} psy_ui_Value;

psy_ui_Value psy_ui_value_makepx(intptr_t px);
psy_ui_Value psy_ui_value_makeew(double em);
psy_ui_Value psy_ui_value_makeeh(double em);
intptr_t psy_ui_value_px(psy_ui_Value*, const psy_ui_TextMetric*);

typedef struct { 
	int x;
	int y;
} psy_ui_Point;

psy_ui_Point psy_ui_point_make(int x, int y);

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
} psy_ui_Rectangle;

typedef struct {
	int width;
	int height;
} psy_ui_Size;

typedef struct {
	psy_ui_Value top;
	psy_ui_Value right;
	psy_ui_Value bottom;
	psy_ui_Value left;
} psy_ui_Margin;

void psy_ui_margin_init(psy_ui_Margin*, psy_ui_Value top, psy_ui_Value right,
	psy_ui_Value bottom, psy_ui_Value left);
intptr_t psy_ui_margin_width_px(psy_ui_Margin*, const psy_ui_TextMetric*);
intptr_t psy_ui_margin_height_px(psy_ui_Margin*, const psy_ui_TextMetric*);

typedef struct {
	LOGFONT lf; 
} psy_ui_FontInfo;

void psy_ui_fontinfo_init(psy_ui_FontInfo*, const char* family, int height);

typedef struct {
	HFONT hfont;
	int stock;
} psy_ui_Font;

typedef enum {
	psy_ui_CURSOR_DEFAULT,
	psy_ui_CURSOR_COLRESIZE
} psy_ui_Cursor;

typedef enum {
	psy_ui_JUSTIFY_NONE,	
	psy_ui_JUSTIFY_EXPAND	
} psy_ui_JustifyType;


typedef enum {
	psy_ui_NOEXPAND = 1,	
	psy_ui_HORIZONTALEXPAND = 2,
	psy_ui_VERTICALEXPAND = 4	
} psy_ui_ExpandMode;

typedef enum {
	psy_ui_HORIZONTAL,
	psy_ui_VERTICAL
} psy_ui_Orientation;

typedef enum {
	psy_ui_ALIGN_NONE,
	psy_ui_ALIGN_CLIENT,
	psy_ui_ALIGN_TOP,
	psy_ui_ALIGN_LEFT,
	psy_ui_ALIGN_BOTTOM,
	psy_ui_ALIGN_RIGHT,
	psy_ui_ALIGN_FILL
} psy_ui_AlignType;

typedef enum {
	psy_ui_ALIGNMENT_NONE = 0,	
	psy_ui_ALIGNMENT_LEFT = 2,
	psy_ui_ALIGNMENT_RIGHT = 4,
	psy_ui_ALIGNMENT_CENTER_HORIZONTAL = psy_ui_ALIGNMENT_LEFT |
		psy_ui_ALIGNMENT_RIGHT,
	psy_ui_ALIGNMENT_TOP	= 8,
	psy_ui_ALIGNMENT_BOTTOM = 16,
	psy_ui_ALIGNMENT_CENTER_VERTICAL = psy_ui_ALIGNMENT_TOP |
		psy_ui_ALIGNMENT_BOTTOM
} psy_ui_Alignment;

void psy_ui_setrectangle(psy_ui_Rectangle*, int left, int top, int width, int height);
int psy_ui_rectangle_intersect(psy_ui_Rectangle*, int x, int y);
int psy_ui_rectangle_intersect_rectangle(const psy_ui_Rectangle*,
	const psy_ui_Rectangle* other);
void psy_ui_rectangle_union(psy_ui_Rectangle*, const psy_ui_Rectangle* other);
void psy_ui_error(const char* err, const char* shorterr);

void psy_ui_font_init(psy_ui_Font*, const psy_ui_FontInfo* info);
void psy_ui_font_copy(psy_ui_Font*, const psy_ui_Font* other);
void psy_ui_font_dispose(psy_ui_Font*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEF_H */

