/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "mainviewbar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void mainviewbar_on_destroy(MainViewBar*);
static void mainviewbar_init_navigation(MainViewBar*, Workspace*);
static void mainviewbar_init_main_tabbar(MainViewBar*);
static void mainviewbar_init_help_settings_tabbar(MainViewBar*s);
static void mainviewbar_init_view_tabbars(MainViewBar*);
static void mainviewbar_on_maxminimize_view(MainViewBar*, psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainViewBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			mainviewbar_on_destroy;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(mainviewbar_base(self), &vtable);
}

/* implementation */
void mainviewbar_init(MainViewBar* self, psy_ui_Component* parent,
	psy_ui_Component* pane, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_style_type(&self->component, STYLE_MAINVIEWTOPBAR);
	psy_ui_component_init_align(&self->tabbars, &self->component, NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_button_init_connect(&self->maximizebtn, &self->component,
		self, mainviewbar_on_maxminimize_view);	
	psy_ui_component_set_align(psy_ui_button_base(&self->maximizebtn),
		psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->view_float, &self->component);
	psy_ui_component_set_align(psy_ui_button_base(&self->view_float),
		psy_ui_ALIGN_RIGHT);
	if (psy_strlen(PSYCLE_RES_DIR) == 0) {
		psy_ui_button_load_resource(&self->maximizebtn,
			IDB_EXPAND_LIGHT, IDB_EXPAND_DARK, psy_ui_colour_white());
		psy_ui_button_load_resource(&self->view_float,
			IDB_FLOAT_LIGHT, IDB_FLOAT_DARK, psy_ui_colour_white());
	} else {
		psy_ui_button_load_bitmap(&self->maximizebtn,
			PSYCLE_RES_DIR"/""expand-light.bmp",
			PSYCLE_RES_DIR"/""expand-dark.bmp",
			psy_ui_colour_white());
		psy_ui_button_load_bitmap(&self->view_float,
			PSYCLE_RES_DIR"/""float-light.bmp",
			PSYCLE_RES_DIR"/""float-dark.bmp",
			psy_ui_colour_white());
	}
	psy_ui_button_init(&self->extract, &self->component);
	psy_ui_button_seticon(&self->extract, psy_ui_ICON_MORE);
	psy_ui_component_set_align(psy_ui_button_base(&self->extract),
		psy_ui_ALIGN_RIGHT);
	mainviewbar_init_navigation(self, workspace);	
	mainviewbar_init_main_tabbar(self);
	mainviewbar_init_help_settings_tabbar(self);
	mainviewbar_init_view_tabbars(self);
	minmaximize_init(&self->minmaximize, pane);
}

void mainviewbar_on_destroy(MainViewBar* self)
{
	minmaximize_dispose(&self->minmaximize);
}

void mainviewbar_init_navigation(MainViewBar* self, Workspace* workspace)
{
	navigation_init(&self->navigation, &self->tabbars, workspace);
	psy_ui_component_set_align(navigation_base(&self->navigation),
		psy_ui_ALIGN_LEFT);
}

void mainviewbar_add_minmaximze(MainViewBar* self, psy_ui_Component* component)
{
	minmaximize_add(&self->minmaximize, component);
}

void mainviewbar_toggle_minmaximze(MainViewBar* self)
{
	minmaximize_toggle(&self->minmaximize);
}

void mainviewbar_on_maxminimize_view(MainViewBar* self, psy_ui_Button* sender)
{
	minmaximize_toggle(&self->minmaximize);
}

void mainviewbar_init_main_tabbar(MainViewBar* self)
{
	psy_ui_tabbar_init(&self->tabbar, &self->tabbars);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_append(&self->tabbar, "main.machines",
		VIEW_ID_MACHINEVIEW,
		IDB_MACHINES_LIGHT, IDB_MACHINES_DARK,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.patterns",
		VIEW_ID_PATTERNVIEW,
		IDB_NOTES_LIGHT, IDB_NOTES_DARK,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.samples",
		VIEW_ID_SAMPLESVIEW,
		psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.instruments",
		VIEW_ID_INSTRUMENTSVIEW,
		psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.properties",
		VIEW_ID_SONGPROPERTIES,
		psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_ui_colour_white());
}

void mainviewbar_init_help_settings_tabbar(MainViewBar* self)
{	
	psy_ui_tabbar_append(&self->tabbar, "main.settings",
		VIEW_ID_SETTINGSVIEW,
		IDB_SETTINGS_LIGHT, IDB_SETTINGS_DARK,
		psy_ui_colour_white());		
	psy_ui_tabbar_append(&self->tabbar, "main.help",		
		VIEW_ID_HELPVIEW,
		psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_ui_colour_white());;
}


void mainviewbar_init_view_tabbars(MainViewBar* self)
{
	psy_ui_notebook_init(&self->viewtabbars, &self->component);
	psy_ui_component_set_margin(&self->viewtabbars.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 4.0));
	psy_ui_component_set_align(&self->viewtabbars.component,
		psy_ui_ALIGN_LEFT);
}
