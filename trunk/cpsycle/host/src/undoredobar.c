// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredobar.h"

static void undoredobar_updatetext(UndoRedoBar*);
static void undoredobar_initalign(UndoRedoBar* self);
static void undoredobar_onundo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onredo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onlanguagechanged(UndoRedoBar*, Translator* sender);
static void undoredobar_ontimer(UndoRedoBar*, psy_ui_Component* sender, uintptr_t timerid);

void undoredobar_init(UndoRedoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->undobutton, &self->component);
	psy_signal_connect(&self->undobutton.signal_clicked, self,
		undoredobar_onundo);
	psy_ui_button_init(&self->redobutton, &self->component);	
	psy_signal_connect(&self->redobutton.signal_clicked, self,
		undoredobar_onredo);
	undoredobar_initalign(self);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		undoredobar_onlanguagechanged);
	undoredobar_updatetext(self);
	psy_signal_connect(&self->component.signal_timer, self,
		undoredobar_ontimer);
	psy_ui_component_starttimer(&self->component, 0, 100);
}

void undoredobar_updatetext(UndoRedoBar* self)
{
	psy_ui_button_settext(&self->undobutton,
		workspace_translate(self->workspace, "undo.undo"));
	psy_ui_button_settext(&self->redobutton,
		workspace_translate(self->workspace, "undo.redo"));
}

void undoredobar_initalign(UndoRedoBar* self)
{			
	psy_list_free(psy_ui_components_setalign(		
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		NULL));
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

void undoredobar_ontimer(UndoRedoBar* self, psy_ui_Component* sender,
	uintptr_t timerid)
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
