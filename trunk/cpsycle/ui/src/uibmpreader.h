/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_BMPREADER_H
#define psy_ui_BMPREADER_H

#include "../../detail/psydef.h"
/* local */
#include "uidef.h"
/* std */
#include <assert.h>

/*
** psy_ui_BmpReader
**
** psy_ui_BmpReader <>-------- psy_ui_Bitmap
*/

#ifdef __cplusplus
extern "C" {
#endif	

struct psy_ui_Bitmap;

typedef struct psy_ui_BmpReader {
	struct psy_ui_Bitmap* bitmap;			
} psy_ui_BmpReader;

void psy_ui_bmpreader_init(psy_ui_BmpReader*,
	struct psy_ui_Bitmap* bitmap);
void psy_ui_bmpreader_dispose(psy_ui_BmpReader*);

int psy_ui_bmpreader_load(psy_ui_BmpReader*, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BMPREADER_H */
