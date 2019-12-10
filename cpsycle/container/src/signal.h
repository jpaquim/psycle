// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SIGNAL_H)
#define SIGNAL_H

#include "list.h"
#include "../../detail/stdint.h"

typedef struct {
	void* fp;
	void* context;
	int prevented;	
} Slot;

typedef struct {
	List* slots;	
} Signal;

void signal_init(Signal*);
void signal_connect(Signal*, void* context, void* fp);
void signal_disconnect(Signal*, void* context, void* fp);
void signal_disconnectall(Signal*);
void signal_prevent(Signal*, void* context, void* fp);
void signal_enable(Signal*, void* context, void* fp);
void signal_emit(Signal*, void* context, int num, ...);
void signal_emit_int(Signal*, void* context, intptr_t param);
void signal_emit_float(Signal*, void* context, float param);
void signal_dispose(Signal*);

#endif
