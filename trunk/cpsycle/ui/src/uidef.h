/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_DEF_H
#define psy_ui_DEF_H

/* local */
#include "uiborder.h"
#include "uicolour.h"
#include "uifont.h"
#include "uigeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	psy_ui_NONE_RECURSIVE 0
#define	psy_ui_RECURSIVE 1

#define psy_ui_ETO_OPAQUE	0x0002
#define psy_ui_ETO_CLIPPED	0x0004

typedef enum psy_ui_Overflow {
	psy_ui_OVERFLOW_HIDDEN = 0,
	psy_ui_OVERFLOW_VSCROLL = 1,
	psy_ui_OVERFLOW_HSCROLL = 2,
	psy_ui_OVERFLOW_SCROLL = psy_ui_OVERFLOW_VSCROLL | psy_ui_OVERFLOW_HSCROLL,
	psy_ui_OVERFLOW_VSCROLLCENTER = 4 | psy_ui_OVERFLOW_VSCROLL,
	psy_ui_OVERFLOW_HSCROLLCENTER = 8 | psy_ui_OVERFLOW_HSCROLL,
	psy_ui_OVERFLOW_SCROLLCENTER = psy_ui_OVERFLOW_VSCROLLCENTER | psy_ui_OVERFLOW_HSCROLLCENTER
} psy_ui_Overflow;

typedef enum psy_ui_Cursor {
	psy_ui_CURSOR_DEFAULT,
	psy_ui_CURSOR_COLRESIZE
} psy_ui_Cursor;

typedef enum psy_ui_ExpandMode {
	psy_ui_NOEXPAND = 1,	
	psy_ui_HEXPAND = 2,
	psy_ui_VERTICALEXPAND = 4	
} psy_ui_ExpandMode;

typedef enum psy_ui_Orientation {
	psy_ui_HORIZONTAL,
	psy_ui_VERTICAL
} psy_ui_Orientation;

typedef enum psy_ui_ContainerAlignType {
	psy_ui_CONTAINER_ALIGN_NONE,
	psy_ui_CONTAINER_ALIGN_LCL,
	psy_ui_CONTAINER_ALIGN_GRID
} psy_ui_ContainerAlignType;

typedef enum psy_ui_AlignType {
	psy_ui_ALIGN_NONE,
	psy_ui_ALIGN_CLIENT,
	psy_ui_ALIGN_TOP,
	psy_ui_ALIGN_LEFT,
	psy_ui_ALIGN_BOTTOM,
	psy_ui_ALIGN_RIGHT,
	psy_ui_ALIGN_CENTER,
	psy_ui_ALIGN_VCLIENT,
	psy_ui_ALIGN_HCLIENT,
	psy_ui_ALIGN_FIXED,	
} psy_ui_AlignType;

typedef enum psy_ui_Alignment {
	psy_ui_ALIGNMENT_NONE = 0,
	psy_ui_ALIGNMENT_LEFT = 2,
	psy_ui_ALIGNMENT_RIGHT = 4,	
	psy_ui_ALIGNMENT_TOP = 8,
	psy_ui_ALIGNMENT_BOTTOM = 16,
	psy_ui_ALIGNMENT_CENTER_VERTICAL =
	psy_ui_ALIGNMENT_TOP | psy_ui_ALIGNMENT_BOTTOM,
	psy_ui_ALIGNMENT_CENTER_HORIZONTAL =
	psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_RIGHT,
	psy_ui_ALIGNMENT_CENTER =
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL |
		psy_ui_ALIGNMENT_CENTER_VERTICAL
} psy_ui_Alignment;

typedef struct psy_ui_TextAlignment {
	psy_ui_Alignment alignment;
} psy_ui_TextAlignment;

INLINE psy_ui_TextAlignment psy_ui_textalignment_make(psy_ui_Alignment alignment)
{
	psy_ui_TextAlignment rv;

	rv.alignment = alignment;
	return rv;
}

INLINE bool psy_ui_textalignment_vertical_centered(const psy_ui_TextAlignment* self)
{
	return  ((self->alignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
}

INLINE bool psy_ui_textalignment_horizontal_centered(const psy_ui_TextAlignment* self)
{
	return  ((self->alignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
}

INLINE bool psy_ui_textalignment_right(const psy_ui_TextAlignment* self)
{
	return ((self->alignment & psy_ui_ALIGNMENT_RIGHT) ==
		psy_ui_ALIGNMENT_RIGHT);
}

typedef enum psy_ui_CursorStyle {
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
	psy_ui_CURSORSTYLE_SW_RESIZE,
	psy_ui_CURSORSTYLE_GRAB,
} psy_ui_CursorStyle;

typedef enum psy_ui_Key {
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
