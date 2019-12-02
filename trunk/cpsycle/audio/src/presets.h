// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PRESETS_H)
#define PRESETS_H

#include "preset.h"
#include <list.h>

typedef struct {
  List* container;
} Presets;

void presets_init(Presets*);
void presets_dispose(Presets*);
Presets* presets_alloc(void);
Presets* presets_allocinit(void);
void presets_append(Presets*, Preset*);

#endif