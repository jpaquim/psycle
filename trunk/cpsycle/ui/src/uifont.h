// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_FONT_H
#define psy_ui_FONT_H

#include "../../detail/psydef.h"

// Graphics Font Interface
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Font
// Implementor			psy_ui_FontImp
// Concrete Implementor	psy_ui_win_FontImp
//
// psy_ui_Font <>-------- psy_ui_FontImp
//                                ^
//      ...                       |
//                           psy_ui_win_Font

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {	
    int32_t      lfHeight;
    int32_t      lfWidth;
    int32_t      lfEscapement;
    int32_t      lfOrientation;
    int32_t      lfWeight;
    uint8_t      lfItalic;
    uint8_t      lfUnderline;
    uint8_t      lfStrikeOut;
    uint8_t      lfCharSet;
    uint8_t      lfOutPrecision;
    uint8_t      lfClipPrecision;
    uint8_t      lfQuality;
    uint8_t      lfPitchAndFamily;
    char      lfFaceName[32];
} psy_ui_FontInfo;

void psy_ui_fontinfo_init(psy_ui_FontInfo*, const char* family, int height);
void psy_ui_fontinfo_init_string(psy_ui_FontInfo*, const char* text);
const char* psy_ui_fontinfo_string(psy_ui_FontInfo*);

typedef void (*psy_ui_font_fp_dispose)(struct psy_ui_Font*);
typedef void (*psy_ui_font_fp_copy)(struct psy_ui_Font*, struct psy_ui_Font*);

typedef struct psy_ui_FontVTable {
	psy_ui_font_fp_dispose dispose;	
	psy_ui_font_fp_copy copy;
} psy_ui_FontVTable;

typedef struct psy_ui_Font {
	psy_ui_FontVTable* vtable;
	struct psy_ui_FontImp* imp;	
} psy_ui_Font;

void psy_ui_font_init(psy_ui_Font*, const psy_ui_FontInfo*);

// vtable calls
INLINE void psy_ui_font_dispose(psy_ui_Font* self)
{
	self->vtable->dispose(self);	
}

INLINE void psy_ui_font_copy(psy_ui_Font* self, psy_ui_Font* other)
{
	self->vtable->copy(self, other);
}

// psy_ui_FontImp
typedef void (*psy_ui_font_imp_fp_dispose)(struct psy_ui_FontImp*);
typedef void (*psy_ui_font_imp_fp_copy)(struct psy_ui_FontImp*, struct psy_ui_FontImp*);

typedef struct psy_ui_FontImpVTable {
	psy_ui_font_imp_fp_dispose dev_dispose;	
	psy_ui_font_imp_fp_copy dev_copy;
} psy_ui_FontImpVTable;

typedef struct psy_ui_FontImp {
	psy_ui_FontImpVTable* vtable;
} psy_ui_FontImp;

void psy_ui_font_imp_init(psy_ui_FontImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_FONT_H */
