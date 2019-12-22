// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PROPERTIESIO_H)
#define PROPERTIESIO_H

#include "properties.h"

int propertiesio_load(psy_Properties*, const char* path, int allowappend);
int propertiesio_loadsection(psy_Properties*, const char* path,
	const char* section, int allowappend);
void propertiesio_save(psy_Properties*, const char* path);

#endif
