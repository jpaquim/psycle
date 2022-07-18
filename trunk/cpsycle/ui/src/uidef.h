/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_DEF_H
#define psy_ui_DEF_H

/* local */
#include "uiborder.h"
#include "uicolour.h"
#include "uifont.h"
#include "uigeometry.h"
#include "uikeys.h"

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
	psy_ui_CURSOR_COLRESIZE,
	psy_ui_CURSOR_RESET
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
	psy_ui_CURSORSTYLE_RESET
} psy_ui_CursorStyle;

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEF_H */
