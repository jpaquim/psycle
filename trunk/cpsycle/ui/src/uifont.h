/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_FONT_H
#define psy_ui_FONT_H

#include "../../detail/psydef.h"
/* local */
#include "uigeometry.h"

/*
** Graphics Font Interface
** Bridge
** Aim                    avoid coupling to one platform (win32, x11, ...)
** Abstraction/Refined    psy_ui_Font
** Implementor			  psy_ui_FontImp
** Concrete Implementor	  psy_ui_win_FontImp/psy_ui_x11_FontImp
**
** psy_ui_Font <>-------- psy_ui_FontImp
**                                ^
**      ...                       |
**                     psy_ui_platform_FontImp
*/

#ifdef __cplusplus
extern "C" {
#endif

/* psy_ui_FontInfo */
typedef struct psy_ui_FontInfo {
    int32_t lfHeight;
    int32_t lfWidth;
    int32_t lfEscapement;
    int32_t lfOrientation;
    int32_t lfWeight;
    uint8_t lfItalic;
    uint8_t lfUnderline;
    uint8_t lfStrikeOut;
    uint8_t lfCharSet;
    uint8_t lfOutPrecision;
    uint8_t lfClipPrecision;
    uint8_t lfQuality;
    uint8_t lfPitchAndFamily;
    char    lfFaceName[32];
} psy_ui_FontInfo;

void psy_ui_fontinfo_init(psy_ui_FontInfo*, const char* family,
    int32_t height);
void psy_ui_fontinfo_init_string(psy_ui_FontInfo*, const char* text);
void psy_ui_fontinfo_string(const psy_ui_FontInfo*, char* rv,
    uintptr_t max_len);

/* Forward Handler to FontImp */
struct psy_ui_FontImp;

/* psy_ui_Font */
typedef struct psy_ui_Font {    
	struct psy_ui_FontImp* imp;
    int dbg;
} psy_ui_Font;

void psy_ui_font_init(psy_ui_Font*, const psy_ui_FontInfo*);
void psy_ui_font_dispose(psy_ui_Font*);
void psy_ui_font_copy(psy_ui_Font*, const psy_ui_Font* other);
psy_ui_FontInfo psy_ui_font_fontinfo(const psy_ui_Font*);
const psy_ui_TextMetric* psy_ui_font_textmetric(const psy_ui_Font*);
psy_ui_Size psy_ui_font_textsize(const psy_ui_Font*, const char* text, uintptr_t count);
bool psy_ui_font_equal(const psy_ui_Font*, const psy_ui_Font* other);

/* psy_ui_FontImpVTable */
typedef void (*psy_ui_font_imp_fp_dispose)(struct psy_ui_FontImp*);
typedef void (*psy_ui_font_imp_fp_copy)(struct psy_ui_FontImp*,
    const struct psy_ui_FontImp*);
typedef const psy_ui_FontInfo (*psy_ui_font_imp_fp_dev_fontinfo)
    (const struct psy_ui_FontImp*);
typedef const psy_ui_TextMetric* (*psy_ui_font_imp_fp_dev_textmetric)(const struct psy_ui_FontImp*);
typedef bool (*psy_ui_font_imp_fp_dev_equal)(const struct psy_ui_FontImp*,
    const struct psy_ui_FontImp*);
typedef psy_ui_Size(*psy_ui_font_imp_fp_dev_textsize)(const struct psy_ui_FontImp*,
    const char* text, uintptr_t count);

typedef struct psy_ui_FontImpVTable {
	psy_ui_font_imp_fp_dispose dev_dispose;	
	psy_ui_font_imp_fp_copy dev_copy;
    psy_ui_font_imp_fp_dev_fontinfo dev_fontinfo;
    psy_ui_font_imp_fp_dev_textmetric dev_textmetric;
    psy_ui_font_imp_fp_dev_textsize dev_textsize;
    psy_ui_font_imp_fp_dev_equal dev_equal;
} psy_ui_FontImpVTable;

/* psy_ui_FontImp */
typedef struct psy_ui_FontImp {
	psy_ui_FontImpVTable* vtable;
} psy_ui_FontImp;

void psy_ui_font_imp_init(psy_ui_FontImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_FONT_H */
