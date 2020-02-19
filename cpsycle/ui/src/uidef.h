// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEF_H
#define psy_ui_DEF_H

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "../../detail/stdint.h"
#include <stddef.h>
#include "uifont.h"

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
	psy_ui_ALIGNMENT_TOP = 8,
	psy_ui_ALIGNMENT_BOTTOM = 16,
	psy_ui_ALIGNMENT_CENTER_VERTICAL = psy_ui_ALIGNMENT_TOP |
	psy_ui_ALIGNMENT_BOTTOM
} psy_ui_Alignment;

void psy_ui_setrectangle(psy_ui_Rectangle*, int left, int top, int width, int height);
int psy_ui_rectangle_intersect(psy_ui_Rectangle*, int x, int y);
int psy_ui_rectangle_intersect_rectangle(const psy_ui_Rectangle*,
	const psy_ui_Rectangle* other);
void psy_ui_rectangle_union(psy_ui_Rectangle*, const psy_ui_Rectangle* other);
void psy_ui_rectangle_expand(psy_ui_Rectangle*, int top, int right, int bottom, int left);
void psy_ui_rectangle_move(psy_ui_Rectangle*, int dx, int dy);
void psy_ui_error(const char* err, const char* shorterr);

typedef enum {
	psy_ui_KEY_LBUTTON         = 0x01,
	psy_ui_KEY_RBUTTON         = 0x02,
	psy_ui_KEY_CANCEL          = 0x03,
	psy_ui_KEY_MBUTTON         = 0x04,
	psy_ui_KEY_BACK            = 0x08,
	psy_ui_KEY_TAB             = 0x09,
	psy_ui_KEY_CLEAR           = 0x0C,
	psy_ui_KEY_RETURN          = 0x0D,
	psy_ui_KEY_SHIFT           = 0x10,
	psy_ui_KEY_CONTROL         = 0x11,
	psy_ui_KEY_MENU            = 0x12,
	psy_ui_KEY_PAUSE           = 0x13,
	psy_ui_KEY_CAPITAL         = 0x14,
	psy_ui_KEY_ESCAPE          = 0x1B,
	psy_ui_KEY_CONVERT         = 0x1C,
	psy_ui_KEY_NONCONVERT      = 0x1D,
	psy_ui_KEY_ACCEPT          = 0x1E,
	psy_ui_KEY_MODECHANGE      = 0x1F,
	psy_ui_KEY_SPACE           = 0x20,
	psy_ui_KEY_PRIOR           = 0x21,
	psy_ui_KEY_NEXT            = 0x22,
	psy_ui_KEY_END             = 0x23,
	psy_ui_KEY_HOME            = 0x24,
	psy_ui_KEY_LEFT            = 0x25,
	psy_ui_KEY_UP              = 0x26,
	psy_ui_KEY_RIGHT           = 0x27,
	psy_ui_KEY_DOWN            = 0x28,
	psy_ui_KEY_SELECT          = 0x29,
	psy_ui_KEY_PRINT           = 0x2A,
	psy_ui_KEY_EXECUTE         = 0x2B,
	psy_ui_KEY_SNAPSHOT        = 0x2C,
	psy_ui_KEY_INSERT          = 0x2D,
	psy_ui_KEY_DELETE          = 0x2E,
	psy_ui_KEY_HELP            = 0x2F,
	psy_ui_KEY_LWIN            = 0x5B,
	psy_ui_KEY_RWIN            = 0x5C,
	psy_ui_KEY_APPS            = 0x5D,
	psy_ui_KEY_SLEEP           = 0x5F,
	psy_ui_KEY_NUMPAD0         = 0x60,
	psy_ui_KEY_NUMPAD1         = 0x61,
	psy_ui_KEY_NUMPAD2         = 0x62,
	psy_ui_KEY_NUMPAD3         = 0x63,
	psy_ui_KEY_NUMPAD4         = 0x64,
	psy_ui_KEY_NUMPAD5         = 0x65,
	psy_ui_KEY_NUMPAD6         = 0x66,
	psy_ui_KEY_NUMPAD7         = 0x67,
	psy_ui_KEY_NUMPAD8         = 0x68,
	psy_ui_KEY_NUMPAD9         = 0x69,
	psy_ui_KEY_MULTIPLY        = 0x6A,
	psy_ui_KEY_ADD             = 0x6B,
	psy_ui_KEY_SEPARATOR       = 0x6C,
	psy_ui_KEY_SUBTRACT        = 0x6D,
	psy_ui_KEY_DECIMAL         = 0x6E,
	psy_ui_KEY_DIVIDE          = 0x6F,
	psy_ui_KEY_F1              = 0x70,
	psy_ui_KEY_F2              = 0x71,
	psy_ui_KEY_F3              = 0x72,
	psy_ui_KEY_F4              = 0x73,
	psy_ui_KEY_F5              = 0x74,
	psy_ui_KEY_F6              = 0x75,
	psy_ui_KEY_F7              = 0x76,
	psy_ui_KEY_F8              = 0x77,
	psy_ui_KEY_F9              = 0x78,
	psy_ui_KEY_F10             = 0x79,
	psy_ui_KEY_F11             = 0x7A,
	psy_ui_KEY_F12             = 0x7B,
	psy_ui_KEY_F13             = 0x7C,
	psy_ui_KEY_F14             = 0x7D,
	psy_ui_KEY_F15             = 0x7E,
	psy_ui_KEY_F16             = 0x7F,
	psy_ui_KEY_F17             = 0x80,
	psy_ui_KEY_F18             = 0x81,
	psy_ui_KEY_F19             = 0x82,
	psy_ui_KEY_F20             = 0x83,
	psy_ui_KEY_F21             = 0x84,
	psy_ui_KEY_F22             = 0x85,
	psy_ui_KEY_F23             = 0x86,
	psy_ui_KEY_F24             = 0x87,
} psy_ui_Key;

typedef uint32_t psy_ui_Color;

#define psy_ui_ETO_OPAQUE	0x0002
#define psy_ui_ETO_CLIPPED	0x0004

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEF_H */

