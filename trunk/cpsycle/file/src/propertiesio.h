/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_PROPERTIESIO_H
#define psy_PROPERTIESIO_H

/* local */
#include "dir.h"
/* container */
#include "properties.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROPERTIESIO_DEFAULT_COMMENT 0
#define PROPERTIESIO_CPP_COMMENT 1

int propertiesio_load(psy_Property*, const char* path, int allowappend,
	bool cpp_comment);
int propertiesio_save(const psy_Property*, const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* psy_PROPERTIESIO_H */
