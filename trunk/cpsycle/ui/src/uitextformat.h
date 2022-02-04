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
    bool wrap;
    const psy_ui_TextMetric* tm;
} psy_ui_TextFormat;

void psy_ui_textformat_init(psy_ui_TextFormat*,
    const psy_ui_TextMetric* tm, bool wrap);

psy_List* psy_ui_textformat_lines(psy_ui_TextFormat*, const char* text, double width);
psy_List* psy_ui_textformat_eols(psy_ui_TextFormat*, const char* text);
psy_List* psy_ui_textformat_wraps(psy_ui_TextFormat*, const char* text,
	uintptr_t num, double width);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TEXTFORMAT_H */
