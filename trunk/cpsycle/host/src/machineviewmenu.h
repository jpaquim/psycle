/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEMENU_H)
#define MACHINEMENU_H

/* host */
#include "workspace.h"
/* ui */
#include <uibutton.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MachineMenu
** Context Menu for Machines (Machineview)
*/

struct WireFrames;

typedef struct MachineMenuState {
	psy_audio_Machines* machines;
	struct WireFrames* wireframes;
	uintptr_t mac_id;
	psy_ui_Component* menu;
} MachineMenuState;

void machinemenustate_init(MachineMenuState*, struct WireFrames*,
	psy_ui_Component* menu);
	
void machinemenustate_hide_menu(MachineMenuState*);

typedef struct MachineConnectionsMenu {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_Table wires;	
	/* references */
	MachineMenuState* state;	
} MachineConnectionsMenu;

void machineconnectionsmenu_init(MachineConnectionsMenu*,
	psy_ui_Component* parent, MachineMenuState*);

typedef struct MachineConnectToMenu {
	/* inherits */
	psy_ui_Component component;
	/* references */
	MachineMenuState* state;
} MachineConnectToMenu;

void machineconnecttomenu_init(MachineConnectToMenu*,
	psy_ui_Component* parent, MachineMenuState*);

typedef struct MachineMenu {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component pane;
	psy_ui_Label machine;
	psy_ui_Button parameters;
	psy_ui_Button properties;
	psy_ui_Button bank;
	psy_ui_Component separator1;
	psy_ui_Button connect;
	psy_ui_Button connections;
	psy_ui_Component separator2;
	psy_ui_Button replace;
	psy_ui_Button clone;
	psy_ui_Button insertbefore;
	psy_ui_Button insertafter;
	psy_ui_Button del;
	psy_ui_Component separator3;
	psy_ui_Button mute;
	psy_ui_Button solo;
	psy_ui_Button bypass;	
	MachineConnectToMenu connect_to_menu;
	MachineConnectionsMenu connections_menu;	
	MachineMenuState state;
} MachineMenu;

void machinemenu_init(MachineMenu*, psy_ui_Component* parent,
	struct WireFrames*);

void machinemenu_set_machines(MachineMenu*, psy_audio_Machines*);
void machinemenu_select(MachineMenu*, uintptr_t mac_id);
void machinemenu_hide(MachineMenu*);

INLINE psy_ui_Component* machinemenu_base(MachineMenu* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEMENU_H */
