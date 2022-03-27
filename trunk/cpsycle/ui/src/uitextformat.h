/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TEXTFORMAT_H
#define psy_ui_TEXTFORMAT_H

#include "uicomponent.h"

/*
** psy_ui_TextFormat
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_TextFormat {
    psy_Table lines;
    bool wrap;
    double width;
    double avgcharwidth;
    double textheight;
    double linespacing;
    psy_ui_TextAlignment textalignment;
} psy_ui_TextFormat;

void psy_ui_textformat_init(psy_ui_TextFormat*);
void psy_ui_textformat_dispose(psy_ui_TextFormat*);

void psy_ui_textformat_clear(psy_ui_TextFormat*);
void psy_ui_textformat_update(psy_ui_TextFormat*, const char* text,
    double width, const psy_ui_TextMetric*);
uintptr_t psy_ui_textformat_line_at(const psy_ui_TextFormat*,
    uintptr_t index);
uintptr_t psy_ui_textformat_numlines(const psy_ui_TextFormat*);

INLINE void psy_ui_textformat_set_alignment(psy_ui_TextFormat* self,
    psy_ui_TextAlignment text_alignment)
{
    self->textalignment = text_alignment;
}

INLINE void psy_ui_textformat_set_line_spacing(psy_ui_TextFormat* self,
    double line_spacing)
{
    self->linespacing = line_spacing;
}

INLINE void psy_ui_textformat_wrap(psy_ui_TextFormat* self)
{
    self->wrap = TRUE;
    psy_ui_textformat_clear(self);
}

INLINE void psy_ui_textformat_prevent_wrap(psy_ui_TextFormat* self)
{
    self->wrap = FALSE;
    psy_ui_textformat_clear(self);
}

uintptr_t psy_ui_textformat_cursor_position(const psy_ui_TextFormat*,
    const char* text, psy_ui_RealPoint, const psy_ui_TextMetric*,
    const psy_ui_Font*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TEXTFORMAT_H */
