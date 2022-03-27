// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINEDETAIL_H)
#define NEWMACHINEDETAIL_H

// host
#include "newmachinesection.h"
#include "labelpair.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uitextarea.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// NewMachineDetail
typedef struct NewMachineDetail {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_categorychanged;
	// intern
	psy_ui_Label desclabel;
	LabelPair plugname;
	psy_ui_Component details;
	LabelPair dllname;
	LabelPair version;
	LabelPair apiversion;
	psy_ui_Component category;
	psy_ui_Label categorydesc;
	psy_ui_TextArea categoryedit;
	psy_ui_Component bottom;
    psy_ui_Label compatlabel;
    psy_ui_CheckBox compatblitzgamefx;
	// references
	psy_Property* plugin;
    Workspace* workspace;
} NewMachineDetail;

void newmachinedetail_init(NewMachineDetail*, psy_ui_Component* parent,
	Workspace*);

void newmachinedetail_reset(NewMachineDetail*);
void newmachinedetail_update(NewMachineDetail*, psy_Property*);
void newmachinedetail_setdescription(NewMachineDetail*, const char* text);
void newmachinedetail_setplugname(NewMachineDetail*, const char* text);
void newmachinedetail_setdllname(NewMachineDetail*, const char* text);
void newmachinedetail_setcategoryname(NewMachineDetail*, const char* text);
void newmachinedetail_setplugversion(NewMachineDetail* self, int16_t version);
void newmachinedetail_setapiversion(NewMachineDetail* self,
	int16_t apiversion);

#ifdef __cplusplus
}
#endif

#endif /* NEWMACHINEDETAIL_H */
