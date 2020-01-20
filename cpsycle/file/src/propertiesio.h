// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_PROPERTIESIO_H
#define psy_PROPERTIESIO_H

#include "properties.h"

#ifdef __cplusplus
extern "C" {
#endif

int propertiesio_load(psy_Properties*, const char* path, int allowappend);
int propertiesio_loadsection(psy_Properties*, const char* path,
	const char* section, int allowappend);
void propertiesio_save(psy_Properties*, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_PROPERTIESIO_H */
