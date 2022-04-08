/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternview.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* PatternView */
/* prototypes */
static void patternview_on_destroy(PatternView*);
static void patternview_rebuild(PatternView*);
static void patternview_on_toggle_properties(PatternView*, PatternViewTabBar*);
static void patternview_on_song_changed(PatternView*, Workspace* sender);
static void patternview_connect_song(PatternView*);
static void patternview_on_configure(PatternView*, PatternViewConfig*,
	psy_Property*);
static void patternview_on_focus(PatternView*);
static void patternview_on_mouse_down(PatternView*, psy_ui_MouseEvent*);
static void patternview_on_mouse_up(PatternView*, psy_ui_MouseEvent*);
static void patternview_on_key_down(PatternView*, psy_ui_KeyboardEvent*);
static void patternview_on_context_menu(PatternView*, psy_ui_Component*);
static void patternview_on_cursor_changed(PatternView*, psy_audio_Sequence*);
static void patternview_update_cursor(PatternView*);
static void patternview_on_grid_scroll(PatternView*, psy_ui_Component*);
static void patternview_on_app_zoom(PatternView*, psy_ui_AppZoom*);
static void patternview_num_tracks_changed(PatternView*, psy_audio_Pattern*,
	uintptr_t numsongtracks);
static void patternview_on_parameter_tweak(PatternView*, Workspace*,
	int slot, uintptr_t tweak, float normvalue);
static void patternview_on_column_resize(PatternView*, TrackerGrid*);
static void patternview_update_scroll_step(PatternView*);
static void patternview_select_display(PatternView*, PatternDisplayMode);
static void patternview_update_font(PatternView*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			patternview_on_destroy;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			patternview_on_mouse_down;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			patternview_on_mouse_up;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			patternview_on_key_down;		
		vtable.on_focus =
			(psy_ui_fp_component_event)
			patternview_on_focus;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);	
}

/* implementation */
void patternview_init(PatternView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,	Workspace* workspace)
{		
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_id(patternview_base(self), VIEW_ID_PATTERNVIEW);
	self->workspace = workspace;	
	self->display = PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER;
	psy_ui_component_set_style_type(&self->component, STYLE_PATTERNVIEW);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);		
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(
		&self->notebook));	
	psy_ui_notebook_select(&self->editnotebook, 0);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_on_song_changed);
	/* Pattern Properties */
	patternproperties_init(&self->properties, &self->component);	
	/* Shared states */	
	patterncmds_init(&self->cmds,		
		&workspace->player, &workspace->undoredo,
		&workspace->config.directories);		
	trackconfig_init(&self->track_config,
		patternviewconfig_show_wide_inst_column(
			psycleconfig_patview(workspace_conf(workspace))));
	patternviewstate_init(&self->pvstate,
		&workspace->config.patview,
		&workspace->config.misc,
		NULL, &self->cmds);
	trackerstate_init(&self->state, &self->track_config, &self->pvstate);
	patternview_connect_song(self);
	/* Header */
	trackerheaderview_init(&self->header, &self->component, 
		&self->track_config, &self->state, self->workspace);	
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);	
	/* Defaultline */
	patterndefaultline_init(&self->defaultline, &self->component,
		&self->track_config, workspace);	
	psy_signal_connect(&self->defaultline.grid.signal_colresize, self,
		patternview_on_column_resize);
	/* Tracker */
	trackerview_init(&self->trackerview, &self->editnotebook.component,		
		&self->state, workspace);
	psy_signal_connect(&self->trackerview.grid.signal_colresize, self,
		patternview_on_column_resize);
	/* Pianoroll */
	pianoroll_init(&self->pianoroll, &self->editnotebook.component,
		&self->pvstate, workspace);	
	/* Blockmenu */
	patternblockmenu_init(&self->blockmenu, patternview_base(self),
		NULL, &self->swingfillview, &self->transformpattern,
		&self->interpolatecurveview, &self->pvstate);
	/* TransformPattern */
	transformpatternview_init(&self->transformpattern, &self->component,
		workspace);
	/* SwingFill */
	swingfillview_init(&self->swingfillview, &self->component,
		&self->pvstate);
	/* Interpolate */
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0,
		0, 0, &self->pvstate, workspace);
	psy_ui_component_set_align(&self->interpolatecurveview.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);	
	/* Tabbar */
	patternviewtabbar_init(&self->tabbar, tabbarparent, workspace);
	psy_signal_connect(&self->tabbar.signal_toggle_properties, self,
		patternview_on_toggle_properties);
	patternview_select_display(self, PATTERN_DISPLAYMODE_TRACKER);		
	psy_signal_connect(&self->tabbar.contextbutton.signal_clicked, self,
		patternview_on_context_menu);
	/* Connect */	
	psy_signal_connect(&self->trackerview.grid.component.signal_scrolled, self,
		patternview_on_grid_scroll);	
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		patternview_on_parameter_tweak);	
	psy_signal_connect(&psy_ui_app_zoom(psy_ui_app())->signal_zoom, self,
		patternview_on_app_zoom);	
	// psy_signal_connect(&self->psy_ui_app_zoom(psy_ui_app())->signal_zoom, self,
	//	patternview_on_pattern_zoom);
	/* Configuration */
	psy_signal_connect(&self->pvstate.patconfig->signal_changed, self,
		patternview_on_configure);
	patternview_rebuild(self);	
	psy_ui_component_start_timer(&self->component, 0, 50);	
}

void patternview_on_destroy(PatternView* self)
{	
	assert(self);

	trackerstate_dispose(&self->state);
	trackconfig_dispose(&self->track_config);
	patternviewstate_dispose(&self->pvstate);
}

void patternview_on_toggle_properties(PatternView* self,
	PatternViewTabBar*sender)
{		
	assert(self);
	
	psy_ui_component_toggle_visibility(&self->properties.component);
	psy_ui_component_invalidate(&self->component);	
}

void patternview_on_song_changed(PatternView* self, Workspace* sender)
{	
	assert(self);

	patternview_connect_song(self);	
	self->state.pv->cursor.orderindex = psy_audio_orderindex_make_invalid();
	self->state.pv->cursor.patternid = psy_INDEX_INVALID;
	patternview_update_cursor(self);
}

void patternview_connect_song(PatternView* self)
{	
	psy_audio_Song* song;

	assert(self);

	song = workspace_song(self->workspace);
	if (song) {
		patternviewstate_set_sequence(self->state.pv, &song->sequence);
		patternproperties_set_patterns(&self->properties, &song->patterns);
		psy_signal_connect(&song->sequence.signal_cursorchanged, self,
			patternview_on_cursor_changed);
		psy_signal_connect(&song->patterns.signal_numsongtrackschanged, self,
			patternview_num_tracks_changed);		
	} else {
		patternviewstate_set_sequence(self->state.pv, NULL);
		patternproperties_set_patterns(&self->properties, NULL);
	}
}

void patternview_on_focus(PatternView* self)
{
	assert(self);

	if (psy_ui_tabbar_selected(&self->tabbar.tabbar) == 1) { /* Pianoroll */
		psy_ui_component_set_focus(&self->pianoroll.grid.component);		
	} else {
		psy_ui_component_set_focus(&self->trackerview.grid.component);
	}
}

void patternview_on_context_menu(PatternView* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_toggle_visibility(patternblockmenu_base(&self->blockmenu));
	psy_ui_component_invalidate(&self->component);
}

void patternview_select_display(PatternView* self, PatternDisplayMode display)
{
	assert(self);

	if (self->display != display) {
		self->display = display;
		switch (display) {
		case PATTERN_DISPLAYMODE_TRACKER:
		case PATTERN_DISPLAYMODE_PIANOROLL:
			psy_ui_tabbar_mark(&self->tabbar.tabbar, display);
			psy_ui_notebook_full(&self->editnotebook);
			psy_ui_notebook_select(&self->editnotebook, display);
			break;
		case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL:
			psy_ui_tabbar_mark(&self->tabbar.tabbar, 0);
			psy_ui_notebook_split(&self->editnotebook, psy_ui_VERTICAL);
			break;
		case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL:
			psy_ui_tabbar_mark(&self->tabbar.tabbar, 0);
			psy_ui_notebook_split(&self->editnotebook, psy_ui_HORIZONTAL);
			break;
		default:
			break;
		}
		psy_ui_notebook_select(&self->notebook, 0);
	}
}

void patternview_on_configure(PatternView* self, PatternViewConfig* config,
	psy_Property* property)
{	
	assert(self);

	patternview_update_font(self);
	patternviewstate_configure(&self->pvstate);		
	trackconfig_init_columns(&self->track_config,
		patternviewconfig_show_wide_inst_column(config));
	patternview_select_display(self, (PatternDisplayMode)
		patternviewconfig_pattern_display(config));
	psy_ui_component_align(&self->component);	
	patternview_update_font(self);
}

void patternview_on_grid_scroll(PatternView* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scroll_left_px(&self->trackerview.grid.component) !=
		psy_ui_component_scroll_left_px(&self->header.header.component)) {
		psy_ui_component_set_scroll_left(&self->header.header.component,
			psy_ui_component_scroll_left(&self->trackerview.grid.component));
		psy_ui_component_invalidate(&self->header.pane);
		psy_ui_component_set_scroll_left(&self->defaultline.grid.component,
			psy_ui_component_scroll_left(&self->trackerview.grid.component));
	}	
}

void patternview_on_cursor_changed(PatternView* self,
	psy_audio_Sequence* sender)
{
	patternview_update_cursor(self);	
}

void patternview_update_cursor(PatternView* self)
{
	patternviewstate_sync_cursor_to_sequence(self->state.pv);
	patternproperties_select(&self->properties,
		self->state.pv->cursor.patternid);
}

void patternview_num_tracks_changed(PatternView* self,
	psy_audio_Pattern* sender, uintptr_t numsongtracks)
{
	assert(self);
		
	patternview_rebuild(self);	
}

void patternview_rebuild(PatternView* self)
{
	assert(self);

	patterndefaultline_update_song_tracks(&self->defaultline);
	trackerheader_build(&self->header.header);	
	trackergrid_build(&self->trackerview.grid);	
	trackergrid_build(&self->defaultline.grid);
	trackerheader_build(&self->header.header);		
	psy_ui_component_align_full(&self->trackerview.scroller.component);
	psy_ui_component_invalidate(&self->component);	
}

void patternview_on_mouse_down(PatternView* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 2) {
		if (psy_ui_component_visible(interpolatecurveview_base(
				&self->interpolatecurveview))) {
			psy_ui_component_hide_align(interpolatecurveview_base(
				&self->interpolatecurveview));			
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void  patternview_on_mouse_up(PatternView* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 1) {
		if (psy_audio_blockselection_valid(&self->trackerview.grid.state->pv->selection)) {			
			interpolatecurveview_set_selection(&self->interpolatecurveview,
				trackergrid_selection(&self->trackerview.grid));
			transformpatternview_set_pattern_selection(&self->transformpattern,
				trackergrid_selection(&self->trackerview.grid));
		}
	} else if (psy_ui_mouseevent_button(ev) == 2) {
		 psy_ui_component_toggle_visibility(patternblockmenu_base(
			 &self->blockmenu));		 
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void patternview_on_key_down(PatternView* self, psy_ui_KeyboardEvent* ev)
{
	assert(self);

	if (psy_ui_keyboardevent_keycode(ev) != psy_ui_KEY_ESCAPE) {
		return;
	}
	if (psy_ui_component_visible(patternblockmenu_base(&self->blockmenu))) {
		psy_ui_component_hide_align(patternblockmenu_base(&self->blockmenu));
	}
	if (psy_ui_component_visible(interpolatecurveview_base(
			&self->interpolatecurveview))) {
		psy_ui_component_hide_align(interpolatecurveview_base(
			&self->interpolatecurveview));
	}	
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);	
}

void patternview_on_column_resize(PatternView* self, TrackerGrid* sender)
{
	assert(self);

	psy_ui_component_align(&self->trackerview.scroller.pane);
	psy_ui_component_align(&self->defaultline.pane);
	psy_ui_component_align(&self->header.pane);
	psy_ui_component_invalidate(&self->component);
}

void patternview_on_parameter_tweak(PatternView* self, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue)
{
	assert(self);

	trackergrid_tweak(&self->trackerview.grid, slot, tweak, normvalue);	
}

void patternview_on_app_zoom(PatternView* self, psy_ui_AppZoom* sender)
{
	assert(self);

	patternview_update_font(self);
}

void patternview_update_font(PatternView* self)
{	
	psy_ui_FontInfo fontinfo;	
	psy_ui_Font font;
	double zoomrate;
	
	assert(self);

	zoomrate = psy_ui_app_zoomrate(psy_ui_app()) * 
		((self->state.pv->patconfig)
		? patternviewconfig_zoom(self->state.pv->patconfig)
		: 1.0);
	fontinfo = patternviewconfig_fontinfo(self->pvstate.patconfig, zoomrate);	
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(&self->component, &font);	
	psy_ui_font_dispose(&font);		
	keyboardstate_update_metrics(&self->pianoroll.keyboardstate,
		psy_ui_component_textmetric(&self->component));
	patternview_update_scroll_step(self);
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align_full(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}

void patternview_update_scroll_step(PatternView* self)
{
	psy_ui_Value step;

	assert(self);

	step = psy_ui_value_make_px(trackerstate_trackwidth(&self->state,
		psy_INDEX_INVALID, psy_ui_component_textmetric(&self->component)));
	psy_ui_component_set_scroll_step_width(trackergrid_base(
		&self->trackerview.grid), step);
	psy_ui_component_set_scroll_step_width(trackergrid_base(
		&self->defaultline.grid), step);
	psy_ui_component_set_scroll_step_width(trackerheader_base(
		&self->header.header), step);
	psy_ui_component_set_scroll_step_height(trackergrid_base(
		&self->trackerview.grid), self->state.line_height);	
}
