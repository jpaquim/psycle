// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INPUTMAP_H)
#define INPUTMAP_H

#include <hashtbl.h>

typedef struct {
	IntHashTable map;	
} InputMap;

void InitInputMap(InputMap* map);
void DisposeInputMap(InputMap* map);
void DefineInput(InputMap* self, int input, int cmd);
int Cmd(InputMap* self, int input);

#endif
