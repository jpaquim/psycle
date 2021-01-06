// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEF_H
#define psy_ui_DEF_H

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "../../detail/stdint.h"
#include <stddef.h>
#include <string.h>
#include "uifont.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined min
#undef min
#endif

#define	psy_ui_NONRECURSIVE 0
#define	psy_ui_RECURSIVE 1

typedef struct psy_ui_PropertyMode {
	bool inherited;
	bool set;
} psy_ui_PropertyMode;

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

void psy_ui_value_init(psy_ui_Value*);

INLINE psy_ui_Value psy_ui_value_makepx(intptr_t px)
{
	psy_ui_Value rv;

	rv.quantity.integer = px;
	rv.unit = psy_ui_UNIT_PX;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_makeew(double em)
{	
	psy_ui_Value rv;

	rv.quantity.real = em;
	rv.unit = psy_ui_UNIT_EW;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_makeeh(double em)
{
	psy_ui_Value rv;

	rv.quantity.real = em;
	rv.unit = psy_ui_UNIT_EH;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_makepe(double pe)
{
	psy_ui_Value rv;

	rv.quantity.real = pe;
	rv.unit = psy_ui_UNIT_PE;
	return rv;
}

intptr_t psy_ui_value_px(const psy_ui_Value*, const psy_ui_TextMetric*);

void psy_ui_value_add(psy_ui_Value*, const psy_ui_Value* other,
	const psy_ui_TextMetric*);
void psy_ui_value_sub(psy_ui_Value*, const psy_ui_Value* other,
	const psy_ui_TextMetric*);
void psy_ui_value_mul_real(psy_ui_Value*, double factor);
int psy_ui_value_comp(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm);

INLINE psy_ui_Value psy_ui_add_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm)
{	
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_add(&rv, &rhs, tm);
	return rv;
}

INLINE psy_ui_Value psy_ui_sub_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm)
{
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_sub(&rv, &rhs, tm);
	return rv;
}


INLINE psy_ui_Value psy_ui_mul_value_real(psy_ui_Value lhs, double factor)
{
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_mul_real(&rv, factor);
	return rv;
}

INLINE psy_ui_Value psy_ui_max_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm)
{	
	if (psy_ui_value_comp(&lhs, &rhs, tm) > 0) {
		return lhs;
	}	
	return rhs;
}

INLINE psy_ui_Value psy_ui_value_zero(void)
{
	return psy_ui_value_makepx(0);
}

INLINE bool psy_ui_value_iszero(const psy_ui_Value* self)
{
	return (self->unit == psy_ui_UNIT_PX)
		? self->quantity.integer == 0
		: self->quantity.real == 0.0;	
}

INLINE bool psy_ui_isvaluezero(psy_ui_Value value)
{
	return psy_ui_value_iszero(&value);
}

typedef struct {
	psy_ui_Value x;
	psy_ui_Value y;
} psy_ui_Point;

INLINE void psy_ui_point_init(psy_ui_Point* self)
{
	self->x = psy_ui_value_zero();
	self->y = psy_ui_value_zero();
}

INLINE void psy_ui_point_init_all(psy_ui_Point* self, psy_ui_Value x, psy_ui_Value y)
{	
	self->x = x;
	self->y = y;	
}

INLINE psy_ui_Point psy_ui_point_make(psy_ui_Value x, psy_ui_Value y)
{
	psy_ui_Point rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

INLINE psy_ui_Point psy_ui_point_makepx(intptr_t x, intptr_t y)
{
	psy_ui_Point rv;

	rv.x = psy_ui_value_makepx(x);
	rv.y = psy_ui_value_makepx(y);
	return rv;
}

INLINE psy_ui_Point psy_ui_point_zero(void)
{
	psy_ui_Point rv;

	rv.x = psy_ui_value_makepx(0);
	rv.y = psy_ui_value_makepx(0);
	return rv;
}

typedef struct { 
	intptr_t x;
	intptr_t y;
} psy_ui_IntPoint;

INLINE void psy_ui_intpoint_init(psy_ui_IntPoint* self)
{
	self->x = 0;
	self->y = 0;
}

INLINE psy_ui_IntPoint psy_ui_intpoint_make(intptr_t x, intptr_t y)
{
	psy_ui_IntPoint rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

typedef struct {
	intptr_t left;
	intptr_t top;
	intptr_t right;
	intptr_t bottom;
} psy_ui_Rectangle;


INLINE psy_ui_Rectangle psy_ui_rectangle_zero(void)
{
	psy_ui_Rectangle rv = { 0, 0, 0, 0 };
	return rv;
}

INLINE psy_ui_Rectangle psy_ui_rectangle_make(intptr_t left, intptr_t top, intptr_t width, intptr_t height)
{
	psy_ui_Rectangle rv;

	rv.left = left;
	rv.top = top;
	rv.right = left + width;
	rv.bottom = top + height;
	return rv;
}

INLINE psy_ui_IntPoint psy_ui_intpoint_zero(void)
{
	psy_ui_IntPoint rv;

	rv.x = 0;
	rv.y = 0;
	return rv;
}

void psy_ui_setrectangle(psy_ui_Rectangle*, intptr_t left, intptr_t top, intptr_t width, intptr_t height);
int psy_ui_rectangle_intersect(const psy_ui_Rectangle*, intptr_t x, intptr_t y);
bool psy_ui_rectangle_intersect_segment(const psy_ui_Rectangle*,
	intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2);
int psy_ui_rectangle_intersect_rectangle(const psy_ui_Rectangle*,
	const psy_ui_Rectangle* other);
void psy_ui_rectangle_union(psy_ui_Rectangle*, const psy_ui_Rectangle* other);
void psy_ui_rectangle_expand(psy_ui_Rectangle*, intptr_t top, intptr_t right, intptr_t bottom, intptr_t left);
void psy_ui_rectangle_move(psy_ui_Rectangle*, intptr_t dx, intptr_t dy);

void psy_ui_error(const char* err, const char* shorterr);

INLINE bool psy_ui_rectangle_equal(psy_ui_Rectangle* self, psy_ui_Rectangle* other)
{
	return memcmp(self, other, sizeof(psy_ui_Rectangle)) == 0;
}

typedef struct {
	intptr_t width;
	intptr_t height;
} psy_ui_IntSize;

INLINE psy_ui_IntSize psy_ui_intsize_make(intptr_t width, intptr_t height)
{
	psy_ui_IntSize rv;

	rv.width = width;
	rv.height = height;
	return rv;
}

typedef struct {
	psy_ui_Value width;
	psy_ui_Value height;
} psy_ui_Size;

INLINE void psy_ui_size_init(psy_ui_Size* self)
{
	psy_ui_value_init(&self->width);
	psy_ui_value_init(&self->height);
}

INLINE void psy_ui_size_init_all(psy_ui_Size* self, psy_ui_Value width, psy_ui_Value height)
{
	self->width = width;
	self->height = height;
}

INLINE psy_ui_Size psy_ui_size_make(psy_ui_Value width, psy_ui_Value height)
{
	psy_ui_Size rv;

	rv.width = width;
	rv.height = height;
	return rv;
}

INLINE psy_ui_Size psy_ui_size_makepx(intptr_t width, intptr_t height)
{
	psy_ui_Size rv;

	rv.width = psy_ui_value_makepx(width);
	rv.height = psy_ui_value_makepx(height);
	return rv;
}

INLINE psy_ui_Size psy_ui_size_makeem(double width, double height)
{
	psy_ui_Size rv;

	rv.width = psy_ui_value_makeew(width);
	rv.height = psy_ui_value_makeeh(height);
	return rv;
}

INLINE psy_ui_Size psy_ui_size_zero(void)
{
	psy_ui_Size rv;

	rv.width = psy_ui_value_makepx(0);
	rv.height = psy_ui_value_makepx(0);
	return rv;
}

INLINE bool psy_ui_size_iszero(const psy_ui_Size* self)
{		
	return psy_ui_value_iszero(&self->width) &&
		psy_ui_value_iszero(&self->height);
}

INLINE bool psy_ui_issizezero(psy_ui_Size size)
{
	return psy_ui_size_iszero(&size);
}

INLINE psy_ui_IntSize psy_ui_intsize_init_size(psy_ui_Size size,
	const psy_ui_TextMetric* tm)
{
	psy_ui_IntSize rv;

	rv.width = psy_ui_value_px(&size.width, tm);
	rv.height = psy_ui_value_px(&size.height, tm);
	return rv;
}

INLINE psy_ui_Size psy_ui_max_size(psy_ui_Size lhs, psy_ui_Size rhs,
	const psy_ui_TextMetric* tm)
{
	psy_ui_Size rv;

	if (psy_ui_value_comp(&lhs.width, &rhs.width, tm) > 0) {
		rv.width = lhs.width;
	} else {
		rv.width = rhs.width;
	}
	if (psy_ui_value_comp(&lhs.height, &rhs.height, tm) > 0) {
		rv.height = lhs.height;
	} else {
		rv.height = rhs.height;
	}
	return rv;
}

typedef struct {
	psy_ui_Value top;
	psy_ui_Value right;
	psy_ui_Value bottom;
	psy_ui_Value left;
} psy_ui_Margin;

void psy_ui_margin_init(psy_ui_Margin*);
void psy_ui_margin_init_all(psy_ui_Margin*, psy_ui_Value top, psy_ui_Value right,
	psy_ui_Value bottom, psy_ui_Value left);
void psy_ui_margin_settop(psy_ui_Margin*, psy_ui_Value value);
void psy_ui_margin_setright(psy_ui_Margin*, psy_ui_Value value);
void psy_ui_margin_setbottom(psy_ui_Margin*, psy_ui_Value value);
void psy_ui_margin_setleft(psy_ui_Margin*, psy_ui_Value value);
psy_ui_Value psy_ui_margin_width(psy_ui_Margin*, const psy_ui_TextMetric*);
intptr_t psy_ui_margin_width_px(psy_ui_Margin*, const psy_ui_TextMetric*);
psy_ui_Value psy_ui_margin_height(psy_ui_Margin*, const psy_ui_TextMetric*);
intptr_t psy_ui_margin_height_px(psy_ui_Margin*, const psy_ui_TextMetric*);

INLINE bool psy_ui_margin_iszero(const psy_ui_Margin* self)
{
	return psy_ui_value_iszero(&self->left) &&
		psy_ui_value_iszero(&self->top) &&
		psy_ui_value_iszero(&self->right) &&
		psy_ui_value_iszero(&self->bottom);
}

INLINE psy_ui_Margin psy_ui_margin_make(psy_ui_Value top, psy_ui_Value right,
	psy_ui_Value bottom, psy_ui_Value left)
{
	psy_ui_Margin rv;

	psy_ui_margin_init_all(&rv, top, right, bottom, left);
	return rv;
}

INLINE psy_ui_Margin psy_ui_margin_zero(void)
{
	psy_ui_Margin rv;

	psy_ui_margin_init(&rv);
	return rv;
}

typedef enum {
	psy_ui_OVERFLOW_HIDDEN = 0,
	psy_ui_OVERFLOW_VSCROLL = 1,
	psy_ui_OVERFLOW_HSCROLL = 2,
	psy_ui_OVERFLOW_SCROLL = psy_ui_OVERFLOW_VSCROLL | psy_ui_OVERFLOW_HSCROLL,
	psy_ui_OVERFLOW_VSCROLLCENTER = 4 | psy_ui_OVERFLOW_VSCROLL,
	psy_ui_OVERFLOW_HSCROLLCENTER = 8 | psy_ui_OVERFLOW_HSCROLL,
	psy_ui_OVERFLOW_SCROLLCENTER = psy_ui_OVERFLOW_VSCROLLCENTER | psy_ui_OVERFLOW_HSCROLLCENTER
} psy_ui_Overflow;

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
	psy_ui_ALIGN_FILL,
	psy_ui_ALIGN_CENTER
} psy_ui_AlignType;

typedef enum {
	psy_ui_ALIGNMENT_NONE = 0,
	psy_ui_ALIGNMENT_LEFT = 2,
	psy_ui_ALIGNMENT_RIGHT = 4,
	psy_ui_ALIGNMENT_CENTER_HORIZONTAL =
		psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_RIGHT,
	psy_ui_ALIGNMENT_TOP = 8,
	psy_ui_ALIGNMENT_BOTTOM = 16,
	psy_ui_ALIGNMENT_CENTER_VERTICAL =
		psy_ui_ALIGNMENT_TOP | psy_ui_ALIGNMENT_BOTTOM
} psy_ui_Alignment;

typedef struct psy_ui_Colour
{
	psy_ui_PropertyMode mode;
	uint32_t value;
} psy_ui_Colour;

INLINE void psy_ui_colour_init(psy_ui_Colour* self)
{
	self->mode.inherited = TRUE;
	self->mode.set = FALSE;
	self->value = 0x00000000;
}

INLINE psy_ui_Colour psy_ui_colour_make(uint32_t value)
{
	psy_ui_Colour rv;

	rv.mode.inherited = TRUE;
	rv.mode.set = TRUE;
	rv.value = value;
	return rv;
}

INLINE void psy_ui_colour_set(psy_ui_Colour* self, psy_ui_Colour colour)
{
	self->mode.inherited = TRUE;
	self->mode.set = TRUE;
	self->value = colour.value;
}

INLINE psy_ui_Colour psy_ui_colour_make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	psy_ui_Colour rv;

	rv.mode.inherited = TRUE;
	rv.mode.set = TRUE;
	rv.value = (uint32_t)(((uint16_t)r) | (((uint16_t)g) << 8) | (((uint16_t)b) << 16));
	return rv;
}

INLINE void psy_ui_colour_rgb(psy_ui_Colour* self,
	uint8_t* r, uint8_t* g, uint8_t* b)
{
	uint32_t temp;
		
	temp = (self->value & 0xFF);
	*r = (uint8_t) temp;	
	temp = ((self->value >> 8) & 0xFF);
	*g = (uint8_t) temp;	
	temp = ((self->value >> 16) & 0xFF);
	*b = (uint8_t) temp;
}

psy_ui_Colour* psy_ui_colour_add_rgb(psy_ui_Colour* self, float r, float g, float b);
psy_ui_Colour* psy_ui_colour_mul_rgb(psy_ui_Colour* self, float r, float g, float b);
psy_ui_Colour psy_ui_diffadd_colours(psy_ui_Colour base, psy_ui_Colour adjust,
	psy_ui_Colour add);

INLINE bool psy_ui_equal_colours(psy_ui_Colour lhs, psy_ui_Colour rhs)
{
	return lhs.value == rhs.value;
}

typedef enum {
	psy_ui_BORDER_NONE,
	psy_ui_BORDER_SOLID
} psy_ui_BorderStyle;

typedef struct {
	psy_ui_PropertyMode mode;
	psy_ui_BorderStyle top;
	psy_ui_BorderStyle right;
	psy_ui_BorderStyle bottom;
	psy_ui_BorderStyle left;
	psy_ui_Colour colour_top;
	psy_ui_Colour colour_right;
	psy_ui_Colour colour_bottom;
	psy_ui_Colour colour_left;
} psy_ui_Border;

INLINE void psy_ui_border_init(psy_ui_Border* self)
{
	self->mode.inherited = FALSE;
	self->mode.set = FALSE;
	self->top = psy_ui_BORDER_NONE;
	self->right = psy_ui_BORDER_NONE;
	self->bottom = psy_ui_BORDER_NONE;
	self->left = psy_ui_BORDER_NONE;
	psy_ui_colour_init(&self->colour_top);
	psy_ui_colour_init(&self->colour_right);
	psy_ui_colour_init(&self->colour_bottom);
	psy_ui_colour_init(&self->colour_left);
}

INLINE void psy_ui_border_init_all(psy_ui_Border* self, psy_ui_BorderStyle top,
	psy_ui_BorderStyle right, psy_ui_BorderStyle bottom, psy_ui_BorderStyle left)
{
	self->mode.inherited = FALSE;
	self->mode.set = TRUE;
	self->top = top;
	self->right = right;
	self->bottom = bottom;
	self->left = left;
	psy_ui_colour_init(&self->colour_top);
	psy_ui_colour_init(&self->colour_right);
	psy_ui_colour_init(&self->colour_bottom);
	psy_ui_colour_init(&self->colour_left);
}

typedef enum {
	psy_ui_CURSORSTYLE_AUTO,
	psy_ui_CURSORSTYLE_MOVE,
	psy_ui_CURSORSTYLE_NODROP,
	psy_ui_CURSORSTYLE_COL_RESIZE,
	psy_ui_CURSORSTYLE_ALL_SCROLL,
	psy_ui_CURSORSTYLE_POINTER,
	psy_ui_CURSORSTYLE_NOT_ALLOWED,
	psy_ui_CURSORSTYLE_ROW_RESIZE,
	psy_ui_CURSORSTYLE_CROSSHAIR,
	psy_ui_CURSORSTYLE_PROGRESS,
	psy_ui_CURSORSTYLE_E_RESIZE,
	psy_ui_CURSORSTYLE_NE_RESIZE,
	psy_ui_CURSORSTYLE_DEFAULT_TEXT,
	psy_ui_CURSORSTYLE_N_RESIZE,
	psy_ui_CURSORSTYLE_NW_RESIZE,
	psy_ui_CURSORSTYLE_HELP,
	psy_ui_CURSORSTYLE_VERTICAL_TEXT,
	psy_ui_CURSORSTYLE_S_RESIZE,
	psy_ui_CURSORSTYLE_SE_RESIZE,
	psy_ui_CURSORSTYLE_INHERIT,
	psy_ui_CURSORSTYLE_WAIT,
	psy_ui_CURSORSTYLE_W_RESIZE,
	psy_ui_CURSORSTYLE_SW_RESIZE
} psy_ui_CursorStyle;

#define psy_ui_ETO_OPAQUE	0x0002
#define psy_ui_ETO_CLIPPED	0x0004

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
	psy_ui_KEY_DIGIT0          = 0x30,
	psy_ui_KEY_DIGIT1          = 0x31,
	psy_ui_KEY_DIGIT2          = 0x32,
	psy_ui_KEY_DIGIT3          = 0x33,
	psy_ui_KEY_DIGIT4          = 0x34,
	psy_ui_KEY_DIGIT5          = 0x35,
	psy_ui_KEY_DIGIT6          = 0x36,
	psy_ui_KEY_DIGIT7          = 0x37,
	psy_ui_KEY_DIGIT8          = 0x38,
	psy_ui_KEY_DIGIT9          = 0x39,
	psy_ui_KEY_A               = 0x41,
	psy_ui_KEY_B               = 0x42,
	psy_ui_KEY_C               = 0x43,
	psy_ui_KEY_D               = 0x44,
	psy_ui_KEY_E               = 0x45,
	psy_ui_KEY_F               = 0x46,
	psy_ui_KEY_G               = 0x47,
	psy_ui_KEY_H               = 0x48,
	psy_ui_KEY_I               = 0x49,
	psy_ui_KEY_J               = 0x4A,
	psy_ui_KEY_K               = 0x4B,
	psy_ui_KEY_L               = 0x4C,
	psy_ui_KEY_M               = 0x4D,
	psy_ui_KEY_N               = 0x4E,
	psy_ui_KEY_O               = 0x4F,
	psy_ui_KEY_P               = 0x50,
	psy_ui_KEY_Q               = 0x51,
	psy_ui_KEY_R               = 0x52,
	psy_ui_KEY_S               = 0x53,
	psy_ui_KEY_T               = 0x54,
	psy_ui_KEY_U               = 0x55,
	psy_ui_KEY_V               = 0x56,
	psy_ui_KEY_W               = 0x57,
	psy_ui_KEY_X               = 0x58,
	psy_ui_KEY_Y               = 0x59,
	psy_ui_KEY_Z               = 0x5A,
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
	psy_ui_KEY_SEMICOLON       = 0xBA,
	psy_ui_KEY_EQUAL           = 0xBB,
	psy_ui_KEY_COMMA           = 0xBC,
	psy_ui_KEY_MINUS           = 0xBD,
	psy_ui_KEY_PERIOD          = 0xBE,
	psy_ui_KEY_SLASH           = 0xBF,
	psy_ui_KEY_BACKQUOTE       = 0xC0,
	psy_ui_KEY_BRACKETLEFT     = 0xDB,
	psy_ui_KEY_BACKSLASH       = 0xDC,
	psy_ui_KEY_QUOTE           = 0xDE,
	psy_ui_KEY_BRACKETRIGHT    = 0xDD
} psy_ui_Key;


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEF_H */
