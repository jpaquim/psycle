/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patterndefaultline.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* PatternDefaultLine */
/* prototypes */
static void patterndefaultline_on_destroy(PatternDefaultLine*);
static void patterndefaultline_on_configure(PatternDefaultLine*,
	PatternViewConfig*, psy_Property*);
static void patterndefaultline_oncursorchanged(PatternDefaultLine*,
	psy_audio_Sequence* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternDefaultLine* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			patterndefaultline_on_destroy;
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
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	/* states */
	patternviewstate_init(&self->pvstate, &workspace->config.patview,
		&workspace->player.patterndefaults.sequence,
		&workspace->player.patterndefaults.patterns, NULL);	
	trackerstate_init(&self->state, trackconfig, &self->pvstate);
	/* label */
	psy_ui_label_init(&self->desc, &self->component);
	psy_ui_label_set_text(&self->desc, "Def");
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_size(&self->desc.component,
		psy_ui_size_make_em(8.0, 1.0));
	/* scroller */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	/* grid */
	trackergrid_init(&self->grid, &self->pane, &self->state, workspace);
	psy_ui_component_set_wheel_scroll(trackergrid_base(&self->grid), 0);
	psy_ui_component_set_align(&self->grid.component, psy_ui_ALIGN_FIXED);
	self->grid.state->drawbeathighlights = FALSE;
	self->grid.preventeventdriver = TRUE;
	self->grid.state->draw_playbar = FALSE;	
	trackergrid_scroll_to_order(&self->grid);
	trackergrid_build(&self->grid);
	psy_signal_connect(&workspace->player.patterndefaults.sequence.signal_cursorchanged,
		self, patterndefaultline_oncursorchanged);
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		patterndefaultline_on_configure);	
}

void patterndefaultline_on_destroy(PatternDefaultLine* self)
{	
	trackerstate_dispose(&self->state);
	patternviewstate_dispose(&self->pvstate);
}

void patterndefaultline_on_configure(PatternDefaultLine* self,
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
	if (patternviewconfig_linenumber_width(config) == 0.0) {
		psy_ui_component_hide(&self->desc.component);
	} else {
		psy_ui_component_set_preferred_size(&self->desc.component,
			psy_ui_size_make_em(
				patternviewconfig_linenumber_width(config) * self->state.flatsize,
				1.0));
		psy_ui_component_show(&self->desc.component);
	}
	self->grid.notestabmode = patternviewconfig_notetabmode(config);
}

void patterndefaultline_oncursorchanged(PatternDefaultLine* self,
	psy_audio_Sequence* sender)
{
	self->state.pv->cursor = self->grid.workspace->player.patterndefaults.sequence.cursor;
	trackergrid_invalidate_cursor(&self->grid);
}
