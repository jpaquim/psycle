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
static void mainviewbar_ondestroy(MainViewBar*);
static void mainviewbar_initnavigation(MainViewBar*, Workspace* workspace);
static void mainviewbar_initmaintabbar(MainViewBar*);
static void mainviewbar_inithelpsettingstabbar(MainViewBar*s);
static void mainviewbar_initviewtabbars(MainViewBar*);
static void mainviewbar_onmaxminimizeview(MainViewBar*, psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MainViewBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			mainviewbar_ondestroy;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(mainviewbar_base(self), &vtable);
}

/* implementation */
void mainviewbar_init(MainViewBar* self, psy_ui_Component* parent,
	psy_ui_Component* pane, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setstyletype(&self->component, STYLE_MAINVIEWTOPBAR);
	psy_ui_component_init_align(&self->tabbars, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_init_connect(&self->maximizebtn, &self->component,
		self, mainviewbar_onmaxminimizeview);
	psy_ui_component_setalign(psy_ui_button_base(&self->maximizebtn),
		psy_ui_ALIGN_RIGHT);
	if (psy_strlen(PSYCLE_RES_DIR) == 0) {
		psy_ui_button_loadresource(&self->maximizebtn,
			IDB_EXPAND_LIGHT, IDB_EXPAND_DARK, psy_ui_colour_white());
	} else {
		psy_ui_bitmap_load(&self->maximizebtn.bitmapicon,
			PSYCLE_RES_DIR"/""expand-dark.bmp");
	}
	mainviewbar_initnavigation(self, workspace);
	mainviewbar_initmaintabbar(self);
	mainviewbar_inithelpsettingstabbar(self);
	mainviewbar_initviewtabbars(self);
	minmaximize_init(&self->minmaximize, pane);
}

void mainviewbar_ondestroy(MainViewBar* self)
{
	minmaximize_dispose(&self->minmaximize);
}

void mainviewbar_initnavigation(MainViewBar* self, Workspace* workspace)
{
	navigation_init(&self->navigation, &self->tabbars, workspace);
	psy_ui_component_setalign(navigation_base(&self->navigation),
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

void mainviewbar_onmaxminimizeview(MainViewBar* self, psy_ui_Button* sender)
{
	minmaximize_toggle(&self->minmaximize);
}

void mainviewbar_initmaintabbar(MainViewBar* self)
{
	psy_ui_tabbar_init(&self->tabbar, &self->tabbars);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_append(&self->tabbar, "main.machines",
		IDB_MACHINES_LIGHT, IDB_MACHINES_DARK, psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.patterns",
		IDB_NOTES_LIGHT, IDB_NOTES_DARK, psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.samples",
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.instruments",
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "main.properties",
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());	
}

void mainviewbar_inithelpsettingstabbar(MainViewBar* self)
{
	psy_ui_tabbar_init(&self->helpsettingstabbar, &self->tabbars);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->helpsettingstabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(psy_ui_tabbar_base(&self->helpsettingstabbar),
		psy_ui_margin_make_em(0.0, 4.0, 0.0, 4.0));
	psy_ui_tabbar_append(&self->helpsettingstabbar, "main.settings",
		IDB_SETTINGS_LIGHT, IDB_SETTINGS_DARK, psy_ui_colour_white());
	psy_ui_tabbar_append(&self->helpsettingstabbar, "main.help",
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());;
}


void mainviewbar_initviewtabbars(MainViewBar* self)
{
	psy_ui_notebook_init(&self->viewtabbars, &self->tabbars);
	psy_ui_component_setalign(&self->viewtabbars.component, psy_ui_ALIGN_LEFT);
}
