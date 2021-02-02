// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_PROPERTIESIO_H
#define psy_PROPERTIESIO_H

#include "properties.h"
#include "dir.h"

#ifdef __cplusplus
extern "C" {
#endif

int propertiesio_load(psy_Property*, const psy_Path*, int allowappend);
int propertiesio_save(const psy_Property*, const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* psy_PROPERTIESIO_H */
