// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PROPERTIESIO_H)
#define PROPERTIESIO_H

#include "properties.h"

int propertiesio_load(Properties*, const char* path, int allowappend);
void propertiesio_save(Properties*, const char* path);

#endif
