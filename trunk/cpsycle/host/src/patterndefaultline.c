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


/* prototypes */
static void patterndefaultline_on_destroyed(PatternDefaultLine*);
static void patterndefaultline_on_show_line(PatternDefaultLine*,
	psy_Property* sender);
static void patterndefaultline_on_update_size(PatternDefaultLine*,
	psy_Property* sender);
static void patterndefaultline_on_cursor_changed(PatternDefaultLine*,
	psy_audio_Sequence* sender);
static void patterndefaultline_update_preferred_size(PatternDefaultLine*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternDefaultLine* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			patterndefaultline_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void patterndefaultline_init(PatternDefaultLine* self, psy_ui_Component* parent,
	TrackConfig* track_config, PatternViewConfig* config, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->workspace = workspace;
	self->config = config;
	psy_ui_component_set_style_type_select(&self->component,
		STYLE_PV_TRACK_VIEW_SELECT);
	psy_ui_component_set_style_type(&self->component,
		STYLE_PV_TRACK_VIEW);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);	
	/* states */
	patternviewstate_init(&self->pvstate_default_line,
		&workspace->config.visual.patview,
		&workspace->config.misc,
		&workspace->player.patterndefaults.sequence,
		NULL);
	patterndefaultline_update_song_tracks(self);
	trackerstate_init(&self->state, track_config, &self->pvstate_default_line);
	/* label */
	psy_ui_label_init(&self->desc, &self->component);
	psy_ui_label_set_text(&self->desc, "Def");
	psy_ui_label_set_text_alignment(&self->desc,
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);	
	/* scroller */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	/* grid */
	trackergrid_init(&self->grid, &self->pane, &self->state,
		&workspace->inputhandler, workspace);
	self->grid.component.blitscroll = TRUE;
	psy_ui_component_set_wheel_scroll(trackergrid_base(&self->grid), 0);
	psy_ui_component_set_align(&self->grid.component, psy_ui_ALIGN_FIXED);
	self->grid.state->draw_beat_highlights = FALSE;
	self->grid.prevent_event_driver = TRUE;
	self->grid.state->draw_playbar = FALSE;	
	trackergrid_build(&self->grid);
	psy_signal_connect(
		&workspace->player.patterndefaults.sequence.signal_cursorchanged,
		self, patterndefaultline_on_cursor_changed);
	patterndefaultline_update_preferred_size(self);
	/* configuration */	
	patternviewconfig_connect(self->config, "griddefaults",
		self, patterndefaultline_on_show_line);
	patternviewconfig_connect(self->config, "linenumbers",
		self, patterndefaultline_on_update_size);
	patternviewconfig_connect(self->config, "beatoffset", 
		self, patterndefaultline_on_update_size);
	patternviewconfig_connect(self->config, "displaysinglepattern",
		self, patterndefaultline_on_update_size);
}

void patterndefaultline_on_destroyed(PatternDefaultLine* self)
{	
	trackerstate_dispose(&self->state);
	patternviewstate_dispose(&self->pvstate_default_line);
}

void patterndefaultline_on_show_line(PatternDefaultLine* self,
	psy_Property* sender)
{
	patterndefaultline_update_preferred_size(self);
	if (psy_property_item_bool(sender)) {
		psy_ui_component_show_align(&self->component);
	} else {
		psy_ui_component_hide_align(&self->component);
	}	
}

void patterndefaultline_on_update_size(PatternDefaultLine* self,
	psy_Property* sender)
{
	patterndefaultline_update_preferred_size(self);
}

void patterndefaultline_update_preferred_size(PatternDefaultLine* self)
{	
	if (patternviewconfig_linenumber_num_digits(self->config) == 0.0) {
		psy_ui_component_hide(&self->desc.component);
	}else {
		psy_ui_component_set_preferred_size(&self->desc.component,
			psy_ui_size_make(
				psy_ui_mul_values(
					psy_ui_value_make_ew(
						patternviewconfig_linenumber_num_digits(
							self->config)),
					self->state.track_config->flatsize,
					psy_ui_component_textmetric(&self->component),
					NULL),
				psy_ui_value_make_eh(1.0)));
		psy_ui_component_show(&self->desc.component);
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void patterndefaultline_on_cursor_changed(PatternDefaultLine* self,
	psy_audio_Sequence* sender)
{
	patternviewstate_set_cursor(&self->pvstate_default_line,
		psy_audio_sequence_cursor(patternviewstate_sequence(
			&self->pvstate_default_line)));
	trackergrid_invalidate_cursor(&self->grid);
}

void patterndefaultline_update_song_tracks(PatternDefaultLine* self)
{
	if (workspace_song(self->workspace)) {		
		assert(patternviewstate_patterns(&self->pvstate_default_line));

		psy_audio_patterns_set_num_tracks(
			patternviewstate_patterns(&self->pvstate_default_line),
			psy_audio_patterns_num_tracks(&self->workspace->song->patterns));
	}
}
