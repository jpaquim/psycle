// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredobar.h"

// prototypes
static void undoredobar_updatetext(UndoRedoBar*);
static void undoredobar_onundo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onredo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onlanguagechanged(UndoRedoBar*, Translator* sender);
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
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_connect(&self->undobutton, undoredobar_base(self),
		self, undoredobar_onundo);	
	psy_ui_button_init_connect(&self->redobutton, undoredobar_base(self),
		self, undoredobar_onredo);
	self->workspace = workspace;
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		undoredobar_onlanguagechanged);
	undoredobar_updatetext(self);	
	psy_ui_component_starttimer(&self->component, 0, 100);
}

void undoredobar_updatetext(UndoRedoBar* self)
{
	psy_ui_button_settext(&self->undobutton,
		workspace_translate(self->workspace, "undo.undo"));
	psy_ui_button_settext(&self->redobutton,
		workspace_translate(self->workspace, "undo.redo"));
}

void undoredobar_onundo(UndoRedoBar* self, psy_ui_Component* sender)
{	
	workspace_undo(self->workspace);	
}

void undoredobar_onredo(UndoRedoBar* self, psy_ui_Component* sender)
{
	workspace_redo(self->workspace);
}

void undoredobar_onlanguagechanged(UndoRedoBar* self, Translator* sender)
{
	undoredobar_updatetext(self);
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
