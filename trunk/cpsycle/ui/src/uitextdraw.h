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

typedef struct psy_ui_TextDraw {    
    psy_ui_TextFormat* format;
    psy_ui_RealSize size;    
    const char* text;
} psy_ui_TextDraw;

void psy_ui_textdraw_init(psy_ui_TextDraw*, psy_ui_TextFormat*,
    psy_ui_RealSize, const char* text);
void psy_ui_textdraw_dispose(psy_ui_TextDraw*);

void psy_ui_textdraw_draw(psy_ui_TextDraw*, psy_ui_Graphics*,
    const psy_ui_Font*, const psy_ui_TextMetric*, uintptr_t cursorpos);
double psy_ui_textdraw_center_x(const psy_ui_TextDraw*,
    double width, const char* text, uintptr_t count, const psy_ui_Font*,
    const psy_ui_TextMetric*);
double psy_ui_textdraw_center_y(const psy_ui_TextDraw*,
    double height, uintptr_t numlines, const psy_ui_Font*,
    const psy_ui_TextMetric*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LABEL_H */
