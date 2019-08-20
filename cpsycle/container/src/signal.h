// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SIGNAL_H)
#define SIGNAL_H

#include "list.h"

typedef struct {
	void* fp;
	void* context;
} Slot;

typedef struct {
	List* slots;
} Signal;

void signal_init(Signal* self);
void signal_connect(Signal* self, void* context, void* fp);
void signal_emit(Signal* self, void* context, int num, ...);
void signal_dispose(Signal* self);

#endif