/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEVIEWBAR_H)
#define MACHINEVIEWBAR_H

/* host */
#include "workspace.h"
/* ui */
#include <uicheckbox.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MachineViewBar
**
** Statusbar that displays actions of the machineview and adds a selector for
** the mixer, if new wires are added to the mixer inputs or sent/returns.
*/

/* MachineViewBar */
typedef struct MachineViewBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_ui_CheckBox mixersend;	
	/* references */	
	psy_audio_Player* player;
} MachineViewBar;

void machineviewbar_init(MachineViewBar*, psy_ui_Component* parent,
	psy_audio_Player*);

INLINE psy_ui_Component* machineviewbar_base(MachineViewBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEWBAR_H */
