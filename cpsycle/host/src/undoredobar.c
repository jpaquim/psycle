/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "undoredobar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* std */
#include <assert.h>

#define UNDOREDOBAR_UPDATERATE 100

/* prototypes */
static void undoredobar_onundo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onredo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_on_timer(UndoRedoBar*, uintptr_t timerid);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(UndoRedoBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			undoredobar_on_timer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(undoredobar_base(self), &vtable);
}

/* implementation */
void undoredobar_init(UndoRedoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);

	psy_ui_component_init(undoredobar_base(self), parent, NULL);
	vtable_init(self);
	self->workspace = workspace;
	psy_ui_component_set_style_type(undoredobar_base(self), STYLE_UNDOBAR);		
	psy_ui_component_set_default_align(undoredobar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_connect(&self->undobutton, undoredobar_base(self),
		self, undoredobar_onundo);
	psy_ui_button_set_text(&self->undobutton, "undo.undo");
	psy_ui_button_load_resource(&self->undobutton, IDB_UNDO_LIGHT,
		IDB_UNDO_DARK, psy_ui_colour_white());
	psy_ui_button_init_connect(&self->redobutton, undoredobar_base(self),
		self, undoredobar_onredo);
	psy_ui_button_set_text(&self->redobutton,"undo.redo");
	psy_ui_button_load_resource(&self->redobutton, IDB_REDO_LIGHT,
		IDB_REDO_DARK, psy_ui_colour_white());	
	psy_ui_component_start_timer(undoredobar_base(self), 0,
		UNDOREDOBAR_UPDATERATE);
}

void undoredobar_onundo(UndoRedoBar* self, psy_ui_Component* sender)
{
	assert(self);

	workspace_undo(self->workspace);	
}

void undoredobar_onredo(UndoRedoBar* self, psy_ui_Component* sender)
{
	assert(self);

	workspace_redo(self->workspace);
}

void undoredobar_on_timer(UndoRedoBar* self, uintptr_t timerid)
{
	assert(self);

#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
	if (workspace_currview_has_undo(self->workspace)) {
		psy_ui_component_enableinput(psy_ui_button_base(&self->undobutton),
			psy_ui_NONE_RECURSIVE);
	} else {
		psy_ui_component_preventinput(psy_ui_button_base(&self->undobutton),
			psy_ui_NONE_RECURSIVE);
	}
	if (workspace_currview_has_redo(self->workspace)) {
		psy_ui_component_enableinput(psy_ui_button_base(&self->redobutton),
			psy_ui_NONE_RECURSIVE);
	} else {
		psy_ui_component_preventinput(psy_ui_button_base(&self->redobutton),
			psy_ui_NONE_RECURSIVE);
	}
#endif
}
