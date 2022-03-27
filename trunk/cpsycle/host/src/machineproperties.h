/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEPROPERTIES_H)
#define MACHINEPROPERTIES_H

/* host */
#include "workspace.h"
/* ui */
#include "uibutton.h"
#include "uilabel.h"
#include "uitextarea.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MachineProperties */
typedef struct MachineProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button issolobypass;
	psy_ui_Button ismute;
	psy_ui_Button isbus;	
	psy_ui_TextArea nameedit;	
	psy_ui_Button remove;
	psy_ui_Button cancel;
	uintptr_t macid;
	bool hasdefaulttext;
	/* references */
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	Workspace* workspace;	
} MachineProperties;

void machineproperties_init(MachineProperties*, psy_ui_Component* parent,
	Workspace*);

void machineproperties_idle(MachineProperties*);
void machineproperties_setmachine(MachineProperties*, psy_audio_Machine*);

INLINE psy_ui_Component* machineproperties_base(MachineProperties* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEPROPERTIES_H */
