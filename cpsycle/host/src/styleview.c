/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "styleview.h"
/* host */
#include "styles.h"
/* ui */
#include <uistyleproperty.h>
#include <uidefaults.h>
/* platform */
#include "../../detail/portable.h"


/* StyleViewBar */

/* implementation */
void styleviewbar_init(StyleViewBar* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_text(&self->diskop, &self->component, "DiskOp");	
}


/* StyleView */

/* prototypes */
static void styleview_on_show(StyleView*);
static void styleview_on_destroyed(StyleView*);
static void styleview_on_diskop(StyleView*, psy_ui_Component* sender);
static void styleview_on_load(StyleView*, psy_Property* sender);
static void styleview_on_save(StyleView*, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable styleview_vtable;
static bool styleview_vtable_initialized = FALSE;

static void styleview_vtable_init(StyleView* self)
{
	assert(self);
	
	if (!styleview_vtable_initialized) {
		styleview_vtable = *(self->component.vtable);		
		styleview_vtable.on_destroyed =
			(psy_ui_fp_component)
			styleview_on_destroyed;
		styleview_vtable.show =
			(psy_ui_fp_component)
			styleview_on_show;
		styleview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &styleview_vtable);
}

/* implementation */
void styleview_init(StyleView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	styleview_vtable_init(self);
	self->workspace = workspace;
	psy_ui_component_set_id(&self->component, VIEW_ID_STYLEVIEW);
	psy_ui_component_set_title(&self->component, "main.styles");
	styleviewbar_init(&self->bar, &self->component);
	psy_ui_component_set_align(&self->bar.component, psy_ui_ALIGN_TOP);
	self->styles_property = psy_ui_styles_property_make(
		&psy_ui_appdefaults()->styles);
	propertiesview_init(&self->view, &self->component,
		tabbarparent, self->styles_property, 3, TRUE,
		&self->workspace->inputhandler);
	psy_ui_component_set_align(&self->view.component, psy_ui_ALIGN_CLIENT);
	psy_property_init_type(&self->load, "load", PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->load, self, styleview_on_load);
	psy_property_init_type(&self->save, "save", PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->save, self, styleview_on_save);
	psy_signal_connect(&self->bar.diskop.signal_clicked, self,
		styleview_on_diskop);
}

void styleview_on_destroyed(StyleView* self)
{
	assert(self);
	
	psy_property_deallocate(self->styles_property);
	psy_property_dispose(&self->load);
	psy_property_dispose(&self->save);
}

void styleview_on_show(StyleView* self)
{
	assert(self);
	
	if (!self->view.renderer.state.do_build) {
		self->view.renderer.state.do_build = TRUE;		
		propertiesrenderer_build(&self->view.renderer);
		psy_ui_component_align(&self->view.scroller.pane);
		psy_ui_component_invalidate(&self->view.scroller.pane);
	}	
}

void styleview_on_diskop(StyleView* self, psy_ui_Component* sender)
{
	assert(self);
	
	fileview_set_callbacks(self->workspace->fileview,
		&self->load, &self->save);
	workspace_select_view(self->workspace, viewindex_make(
		VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));			
}

void styleview_on_load(StyleView* self, psy_Property* sender)
{
	assert(self);
	
	psy_ui_styles_load(self->styles_property, psy_property_item_str(sender));
	workspace_select_view(self->workspace, viewindex_make(
		VIEW_ID_STYLEVIEW, 0, 0, psy_INDEX_INVALID));
}

void styleview_on_save(StyleView* self, psy_Property* sender)
{	
	assert(self);

	psy_ui_styles_save(self->styles_property, psy_property_item_str(sender));
	workspace_select_view(self->workspace, viewindex_make(
		VIEW_ID_STYLEVIEW, 0, 0, psy_INDEX_INVALID));
}
