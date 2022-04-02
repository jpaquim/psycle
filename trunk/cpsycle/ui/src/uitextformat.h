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
    bool word_wrap;
    bool line_wrap;
    double width;
    double avgcharwidth;
    double textheight;
    double linespacing;
    uintptr_t numavgchars;
    uintptr_t nummaxchars;
    psy_ui_TextAlignment textalignment;
} psy_ui_TextFormat;

void psy_ui_textformat_init(psy_ui_TextFormat*);
void psy_ui_textformat_dispose(psy_ui_TextFormat*);

void psy_ui_textformat_clear(psy_ui_TextFormat*);
void psy_ui_textformat_update(psy_ui_TextFormat*, const char* text,
    double width, const psy_ui_Font*, const psy_ui_TextMetric*);
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

INLINE void psy_ui_textformat_word_wrap(psy_ui_TextFormat* self)
{
    self->word_wrap = TRUE;
    psy_ui_textformat_clear(self);
}

INLINE void psy_ui_textformat_prevent_word_wrap(psy_ui_TextFormat* self)
{
    self->word_wrap = FALSE;
    psy_ui_textformat_clear(self);
}

INLINE bool psy_ui_textformat_has_word_wrap(const psy_ui_TextFormat* self)
{
    return self->word_wrap;
}

INLINE void psy_ui_textformat_prevent_wrap(psy_ui_TextFormat* self)
{
    self->line_wrap = FALSE;
    self->word_wrap = FALSE;
    psy_ui_textformat_clear(self);
}

INLINE bool psy_ui_textformat_has_wrap(const psy_ui_TextFormat* self)
{
    return self->word_wrap || self->line_wrap;
}

INLINE void psy_ui_textformat_line_wrap(psy_ui_TextFormat* self)
{
    self->line_wrap = TRUE;
    psy_ui_textformat_clear(self);
}

INLINE void psy_ui_textformat_prevent_line_wrap(psy_ui_TextFormat* self)
{
    self->line_wrap = FALSE;
    psy_ui_textformat_clear(self);
}

INLINE bool psy_ui_textformat_has_line_wrap(const psy_ui_TextFormat* self)
{
    return self->line_wrap;
}

uintptr_t psy_ui_textformat_cursor_position(const psy_ui_TextFormat*,
    const char* text, psy_ui_RealPoint, const psy_ui_TextMetric*,
    const psy_ui_Font*);
double psy_ui_textformat_screen_offset(const psy_ui_TextFormat*,
    const char* text, uintptr_t count, const psy_ui_Font*,
    const psy_ui_TextMetric*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TEXTFORMAT_H */
