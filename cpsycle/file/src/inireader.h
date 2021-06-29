/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_INIREADER_H
#define psy_INIREADER_H

/* local */
#include "dir.h"
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_IniReader {
	psy_Signal signal_read;
	psy_Signal signal_comment;
	psy_Signal signal_section;
	char* section;
} psy_IniReader;

void psy_inireader_init(psy_IniReader*);
void psy_inireader_dispose(psy_IniReader*);

int inireader_load(psy_IniReader*, const psy_Path*);

#ifdef __cplusplus
}
#endif

#endif /* psy_INIREADER_H */
