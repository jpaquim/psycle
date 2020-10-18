// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(INSTRUMENTSBOX_H)
#define INSTRUMENTSBOX_H

#include "workspace.h"

#include <uilabel.h>
#include <uilistbox.h>

#include "instruments.h"

// aim: displays the instruments of the current song and  instrument slot used
//      by the pattern editor in a listbox. The view is synchronized with 
//      'psy_audio_Instruments', which stores the instrument and
//      the selected instrument slot of the song.

typedef struct {
	psy_ui_Component component;
	psy_ui_ListBox grouplist;
	psy_ui_Label header;
	psy_ui_Label group;
	psy_ui_ListBox instrumentlist;	
	psy_audio_Instruments* instruments;
} InstrumentsBox;

void instrumentsbox_init(InstrumentsBox*, psy_ui_Component* parent, psy_audio_Instruments*,
	Workspace*);
void instrumentsbox_setinstruments(InstrumentsBox*, psy_audio_Instruments* instruments);
int instrumentsbox_selected(InstrumentsBox*);
void instrumentsbox_rebuild(InstrumentsBox*);

#endif
