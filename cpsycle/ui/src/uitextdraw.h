/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TEXTDRAW_H
#define psy_ui_TEXTDRAW_H

/* local */
#include "uicomponent.h"
#include "uitextformat.h"

/*
** psy_ui_TextDraw
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_TextPosition {
	uintptr_t caret;
	uintptr_t selection;
} psy_ui_TextPosition;

void psy_ui_textposition_init(psy_ui_TextPosition*);

bool psy_ui_textposition_has_selection(const psy_ui_TextPosition*);
void psy_ui_textposition_selection(const psy_ui_TextPosition*,
	uintptr_t* cp_start, uintptr_t* cp_end);

psy_ui_TextPosition psy_ui_textposition_make(uintptr_t caret,
	uintptr_t selection);

typedef struct psy_ui_TextDraw {    
    psy_ui_TextFormat* format;
    psy_ui_RealSize size;    
    const char* text;
} psy_ui_TextDraw;

void psy_ui_textdraw_init(psy_ui_TextDraw*, psy_ui_TextFormat*,
    psy_ui_RealSize, const char* text);
void psy_ui_textdraw_dispose(psy_ui_TextDraw*);

void psy_ui_textdraw_draw(psy_ui_TextDraw*, psy_ui_Graphics*,
    psy_ui_TextPosition, bool draw_caret);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LABEL_H */
