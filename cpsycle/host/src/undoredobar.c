// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredobar.h"

static void undoredobar_onundo(UndoRedoBar*, ui_component* sender);
static void undoredobar_onredo(UndoRedoBar*, ui_component* sender);
static void undoredobar_initalign(UndoRedoBar* self);

void undoredobar_init(UndoRedoBar* self, ui_component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_button_init(&self->undobutton, &self->component);
	ui_button_settext(&self->undobutton,
		workspace_translate(workspace, "undo"));
	psy_signal_connect(&self->undobutton.signal_clicked, self,
		undoredobar_onundo);
	ui_button_init(&self->redobutton, &self->component);
	ui_button_settext(&self->redobutton,
		workspace_translate(workspace, "redo"));
	psy_signal_connect(&self->redobutton.signal_clicked, self,
		undoredobar_onredo);
	undoredobar_initalign(self);	
}

void undoredobar_initalign(UndoRedoBar* self)
{		
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));	
	list_free(ui_components_setalign(		
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));
}

void undoredobar_onundo(UndoRedoBar* self, ui_component* sender)
{
	workspace_undo(self->workspace);
}

void undoredobar_onredo(UndoRedoBar* self, ui_component* sender)
{
	workspace_redo(self->workspace);
}
