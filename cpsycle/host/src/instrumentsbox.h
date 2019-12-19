// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(INSTRUMENTSBOX_H)
#define INSTRUMENTSBOX_H

#include <uilistbox.h>
#include "instruments.h"

typedef struct {
	ui_listbox instrumentlist;	
	Instruments* instruments;
} InstrumentsBox;

void instrumentsbox_init(InstrumentsBox*, ui_component* parent, Instruments*);
void instrumentsbox_setinstruments(InstrumentsBox*, Instruments* instruments);
int instrumentsbox_selected(InstrumentsBox*);

#endif
