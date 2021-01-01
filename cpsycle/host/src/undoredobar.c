// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredobar.h"
// std
#include <assert.h>

#define UNDOREDOBAR_UPDATERATE 100

// prototypes
static void undoredobar_onundo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onredo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_ontimer(UndoRedoBar*, uintptr_t timerid);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(UndoRedoBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)undoredobar_ontimer;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void undoredobar_init(UndoRedoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);

	psy_ui_component_init(undoredobar_base(self), parent);
	psy_ui_component_setvtable(undoredobar_base(self), vtable_init(self));
	self->workspace = workspace;
	psy_ui_component_setalignexpand(undoredobar_base(self), psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(undoredobar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_connect(&self->undobutton, undoredobar_base(self),
		self, undoredobar_onundo);
	psy_ui_button_settext(&self->undobutton, "undo.undo");
	psy_ui_button_init_connect(&self->redobutton, undoredobar_base(self),
		self, undoredobar_onredo);
	psy_ui_button_settext(&self->redobutton,"undo.redo");	
	psy_ui_component_starttimer(undoredobar_base(self), 0, UNDOREDOBAR_UPDATERATE);
}

void undoredobar_onundo(UndoRedoBar* self, psy_ui_Component* sender)
{	
	workspace_undo(self->workspace);	
}

void undoredobar_onredo(UndoRedoBar* self, psy_ui_Component* sender)
{
	workspace_redo(self->workspace);
}

void undoredobar_ontimer(UndoRedoBar* self, uintptr_t timerid)
{
	if (workspace_currview_hasundo(self->workspace)) {
		psy_ui_component_enableinput(psy_ui_button_base(&self->undobutton),
			psy_ui_NONRECURSIVE);
	} else {
		psy_ui_component_preventinput(psy_ui_button_base(&self->undobutton),
			psy_ui_NONRECURSIVE);
	}
	if (workspace_currview_hasredo(self->workspace)) {
		psy_ui_component_enableinput(psy_ui_button_base(&self->redobutton),
			psy_ui_NONRECURSIVE);
	} else {
		psy_ui_component_preventinput(psy_ui_button_base(&self->redobutton),
			psy_ui_NONRECURSIVE);
	}
}
