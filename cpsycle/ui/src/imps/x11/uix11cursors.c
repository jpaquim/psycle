/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11cursors.h"
/* x11 */
#include <X11/cursorfont.h>


#if PSYCLE_USE_TK == PSYCLE_TK_X11

void psy_ui_x11_cursors_init(psy_ui_x11_Cursors* self,
	Display* dpy)
{
	assert(self);
	assert(dpy);
	
	self->dpy = dpy;
	psy_table_init(&self->cursors);
}

void psy_ui_x11_cursors_dispose(psy_ui_x11_Cursors* self)
{
	psy_TableIterator it;
	
	assert(self);

	for (it = psy_table_begin(&self->cursors);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		Cursor cursor;

		cursor = (Cursor)psy_tableiterator_value(&it);
		XFreeCursor(self->dpy, cursor);
	}
	psy_table_dispose(&self->cursors);
}

Cursor psy_ui_x11_cursors_cursor(psy_ui_x11_Cursors* self,
	unsigned int shape)
{
	Cursor cursor;
	
	if (psy_table_exists(&self->cursors, shape)) {
		return (Cursor)(uintptr_t)psy_table_at(&self->cursors, shape);
	}		
	cursor = XCreateFontCursor(self->dpy, shape);
	psy_table_insert(&self->cursors, shape, (void*)cursor);
	return cursor;
}

unsigned int psy_ui_x11_cursors_shape(psy_ui_x11_Cursors* self,
	psy_ui_CursorStyle style)
{
	unsigned int rv;
	
	switch(style) {
	case psy_ui_CURSORSTYLE_AUTO:
		return XC_arrow;
	case psy_ui_CURSORSTYLE_MOVE:
		return XC_fleur;
	case psy_ui_CURSORSTYLE_NODROP:
		return XC_arrow;
	case psy_ui_CURSORSTYLE_COL_RESIZE:
		return XC_sb_h_double_arrow;
	case psy_ui_CURSORSTYLE_ALL_SCROLL:
		return XC_arrow;
	case psy_ui_CURSORSTYLE_POINTER:
		return XC_arrow;
	case psy_ui_CURSORSTYLE_NOT_ALLOWED:
		return XC_arrow;
	case psy_ui_CURSORSTYLE_ROW_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_CROSSHAIR:
		return XC_tcross;	
	case psy_ui_CURSORSTYLE_PROGRESS:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_E_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_NE_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_DEFAULT_TEXT:
		return XC_xterm;	
	case psy_ui_CURSORSTYLE_N_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_NW_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_HELP:
		return XC_question_arrow;	
	case psy_ui_CURSORSTYLE_VERTICAL_TEXT:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_S_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_SE_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_INHERIT:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_WAIT:
		return XC_watch;	
	case psy_ui_CURSORSTYLE_W_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_SW_RESIZE:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_GRAB:
		return XC_arrow;	
	case psy_ui_CURSORSTYLE_RESET:
		return 0;	
	default:
		break;
	}
	return 0;
}

#endif /* PSYCLE_TK_X11 */
