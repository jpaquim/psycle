// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_SIGNAL_H
#define psy_SIGNAL_H


#include "list.h"
#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void* fp;
	void* context;
	int prevented;
	int abort;
} psy_Slot;

typedef struct {
	psy_List* slots;	
} psy_Signal;

void psy_signal_init(psy_Signal*);
void psy_signal_connect(psy_Signal*, void* context, void* fp);
void psy_signal_disconnect(psy_Signal*, void* context, void* fp);
void psy_signal_disconnectall(psy_Signal*);
void psy_signal_prevent(psy_Signal*, void* context, void* fp);
void psy_signal_enable(psy_Signal*, void* context, void* fp);
void psy_signal_emit(psy_Signal*, void* context, int num, ...);
void psy_signal_emit_int(psy_Signal*, void* context, intptr_t param);
void psy_signal_emit_float(psy_Signal*, void* context, float param);
void psy_signal_dispose(psy_Signal*);
psy_Slot* psy_signal_findslot(psy_Signal*, void* context, void* fp);

#ifdef __cplusplus
}
#endif

#endif /* psy_SIGNAL_H */
