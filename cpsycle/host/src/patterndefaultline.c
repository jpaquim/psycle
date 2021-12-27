/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patterndefaultline.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* PatternDefaultLine */
/* prototypes */
static void patterndefaultline_ondestroy(PatternDefaultLine*);
static void patterndefaultline_onconfigure(PatternDefaultLine*,
	PatternViewConfig*, psy_Property*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternDefaultLine* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			patterndefaultline_ondestroy;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void patterndefaultline_init(PatternDefaultLine* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_TOP);
	/* states */
	patternviewstate_init(&self->pvstate, &workspace->config.patview,
		workspace_song(workspace), NULL);
	trackerstate_init(&self->state, trackconfig, &self->pvstate);
	/* grid */
	trackergrid_init(&self->grid, &self->component, &self->state, workspace);
	psy_ui_component_setwheelscroll(trackergrid_base(&self->grid), 0);
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_FIXED);
	self->grid.state->drawbeathighlights = FALSE;
	self->grid.preventeventdriver = TRUE;
	self->grid.state->synccursor = FALSE;	
	trackergrid_setpattern(&self->grid,
		workspace_player(workspace)->patterndefaults);
	trackergrid_build(&self->grid);
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		patterndefaultline_onconfigure);
}

void patterndefaultline_ondestroy(PatternDefaultLine* self)
{	
	trackerstate_dispose(&self->state);
	patternviewstate_dispose(&self->pvstate);
}

void patterndefaultline_onconfigure(PatternDefaultLine* self,
	PatternViewConfig* config, psy_Property* property)
{
	if (psy_ui_component_visible(&self->component) !=
		patternviewconfig_defaultline(config)) {
		if (patternviewconfig_defaultline(config)) {
			psy_ui_component_show_align(&self->component);
		} else {
			psy_ui_component_hide_align(&self->component);
		}
	}
	self->grid.notestabmode = patternviewconfig_notetabmode(config);
}
