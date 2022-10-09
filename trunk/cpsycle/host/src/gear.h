/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(GEAR_H)
#define GEAR_H

/* host */
#include "instrumentsbox.h"
#include "machinesbox.h"
#include "samplesbox.h"
#include "titlebar.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uilistbox.h>
#include <uinotebook.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Gear
** The Gear Rack is a convenient way to organise your machines. Use the
** buttons "Generators", "Effects" and "Instruments" to switch between the
** different tabs
*/

struct ParamViews;

typedef struct GearButtons {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button createreplace;
	psy_ui_Button del;
	psy_ui_Button parameters;
	psy_ui_Button properties;
	psy_ui_Button exchange;
	psy_ui_Button clone;
	psy_ui_Button showmaster;	
} GearButtons;

void gearbuttons_init(GearButtons*, psy_ui_Component* parent,
	struct ParamViews*);

INLINE psy_ui_Component* gearbuttons_base(GearButtons* self)
{
	return &self->component;
}

typedef struct Gear {
	/* inherits */
	psy_ui_Component component;
	/* ui elements */
	psy_ui_Component client;
	psy_ui_TabBar tabbar;	
	TitleBar titlebar;
	psy_ui_Label label;
	psy_ui_Notebook notebook;
	psy_ui_ListBox listbox;	
	MachinesBox machinesboxgen;
	MachinesBox machinesboxfx;
	InstrumentsBox instrumentsbox;
	SamplesBox samplesbox;	
	GearButtons buttons;	
	/* references */
	psy_audio_Machines* machines;
	struct ParamViews* param_views;
	Workspace* workspace;	
} Gear;

void gear_init(Gear*, psy_ui_Component* parent, struct ParamViews*, Workspace*);

void gear_select(Gear*, psy_List* list);

INLINE psy_ui_Component* gear_base(Gear* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* GEAR_H */
