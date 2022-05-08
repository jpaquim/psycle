/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CONFIGIMPORT_H)
#define CONFIGIMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* host */
#include "config.h"

typedef struct PsycleConfigImport {
	PsycleConfig* config;
	psy_Property* notes;
} PsycleConfigImport;

void psycleconfigimport_init(PsycleConfigImport*, PsycleConfig*);
void psycleconfigimport_dispose(PsycleConfigImport*);

int psycleconfigimport_read(PsycleConfigImport*, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGIMPORT_H */
