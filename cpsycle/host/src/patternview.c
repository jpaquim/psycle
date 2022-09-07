/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternview.h"
/* host */
#include "styles.h"
#include "viewindex.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void patternview_on_destroyed(PatternView*);
static void patternview_rebuild(PatternView*);
static void patternview_on_toggle_properties(PatternView*, PatternViewTabBar*);
static void patternview_on_song_changed(PatternView*, psy_audio_Player* sender);
static void patternview_connect_song(PatternView*);
static void patternview_on_configure(PatternView*, PatternViewConfig*,
	psy_Property*);
static void patternview_on_zoom(PatternView*, psy_Property*);
static void patternview_on_font(PatternView*, psy_Property*);
static void patternview_on_select_display(PatternView*, psy_Property*);
static void patternview_on_follow_song(PatternView*, psy_Property* sender);
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
// static void patternview_on_parameter_tweak(PatternView*, Workspace*,
//	int slot, uintptr_t tweak, float normvalue);
static void patternview_on_column_resize(PatternView*, TrackerGrid*);
static void patternview_update_scroll_step(PatternView*);
static void patternview_select_display(PatternView*, PatternDisplayMode);
static void patternview_update_font(PatternView*);
static void patternview_on_track_reposition(PatternView*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void patternview_on_pattern_length_changed(PatternView*,
	psy_audio_Patterns*, uintptr_t pattern_idx);
static void patternview_select_section(PatternView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);	
static uintptr_t patternview_section(const PatternView*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			patternview_on_destroyed;
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
			(psy_ui_fp_component)
			patternview_on_focus;
		vtable.section =
			(psy_ui_fp_component_section)
			patternview_section;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(patternview_base(self), &vtable);
}

/* implementation */
void patternview_init(PatternView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,	Workspace* workspace)
{	
	PatternViewConfig* pvconfig;

	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_tab_index(patternview_base(self), 0);
	psy_ui_component_set_title(patternview_base(self), "main.patterns");
	psy_ui_component_set_id(patternview_base(self), VIEW_ID_PATTERNVIEW);
	self->workspace = workspace;	
	self->display = PATTERN_DISPLAYMODE_TRACKER;
	psy_ui_component_set_style_type(&self->component, STYLE_PATTERNVIEW);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);		
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(
		&self->notebook));	
	psy_ui_notebook_select(&self->editnotebook, 0);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		patternview_on_song_changed);
	/* pattern properties */
	patternproperties_init(&self->properties, &self->component);	
	/* shared states */	
	patterncmds_init(&self->cmds,		
		&workspace->player, &workspace->undoredo,
		&workspace->config.directories);		
	trackconfig_init(&self->track_config,
		patternviewconfig_show_wide_inst_column(
			psycleconfig_patview(workspace_conf(workspace))));
	patternviewstate_init(&self->pvstate,
		&workspace->config.visual.patview,
		&workspace->config.misc,
		NULL, &self->cmds);
	trackerstate_init(&self->state, &self->track_config, &self->pvstate);
	patternview_connect_song(self);
	/* header */
	trackerheaderview_init(&self->header, &self->component, 
		&self->track_config, &self->state, self->workspace);	
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);	
	/* defaultline */
	patterndefaultline_init(&self->defaultline, &self->component,
		&self->track_config, &workspace->config.visual.patview, workspace);	
	psy_signal_connect(&self->defaultline.grid.signal_colresize, self,
		patternview_on_column_resize);
	/* tracker */
	trackerview_init(&self->trackerview, &self->editnotebook.component,		
		&self->state, workspace);
	psy_signal_connect(&self->trackerview.grid.signal_colresize, self,
		patternview_on_column_resize);
	/* pianoroll */
	pianoroll_init(&self->pianoroll, &self->editnotebook.component,
		&self->pvstate, workspace);	
	/* blockmenu */
	patternblockmenu_init(&self->blockmenu, patternview_base(self),
		NULL, &self->swingfillview, &self->transformpattern,
		&self->interpolatecurveview, &self->pvstate);
	/* transformpattern */
	transformpatternview_init(&self->transformpattern, &self->component,
		workspace);
	/* swingfill */
	swingfillview_init(&self->swingfillview, &self->component,
		&self->pvstate);
	/* interpolate */
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0,
		0, 0, &self->pvstate, workspace);
	psy_ui_component_set_align(&self->interpolatecurveview.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);	
	/* tabbar */
	patternviewtabbar_init(&self->tabbar, tabbarparent,
		&self->workspace->config.visual.patview);
	psy_signal_connect(&self->tabbar.signal_toggle_properties, self,
		patternview_on_toggle_properties);
	self->display = PATTERN_DISPLAYMODE_INVALID;
	patternview_select_display(self, PATTERN_DISPLAYMODE_TRACKER);
	psy_signal_connect(&self->tabbar.context_button.signal_clicked, self,
		patternview_on_context_menu);
	/* connect */	
	psy_signal_connect(&self->trackerview.grid.component.signal_scrolled, self,
		patternview_on_grid_scroll);	
// 	psy_signal_connect(&self->workspace->signal_parametertweak, self,
//		patternview_on_parameter_tweak);	
	psy_signal_connect(&psy_ui_app_zoom(psy_ui_app())->signal_zoom, self,
		patternview_on_app_zoom);
	psy_signal_connect(&self->component.signal_selectsection, self,
		patternview_select_section);	
	/* configuration */
	pvconfig = &self->workspace->config.visual.patview;
	patternviewconfig_connect(pvconfig,
		"patterndisplay", self, patternview_on_select_display);
	patternviewconfig_connect(pvconfig,
		"zoom", self, patternview_on_zoom);
	patternviewconfig_connect(pvconfig,
		"font", self, patternview_on_font);
	keyboardmiscconfig_connect(self->pvstate.keymiscconfig, "followsong",
		self, patternview_on_follow_song);
	patternview_on_configure(self, self->pvstate.patconfig, NULL);
	patternview_rebuild(self);	
}

void patternview_on_destroyed(PatternView* self)
{	
	assert(self);

	trackerstate_dispose(&self->state);
	trackconfig_dispose(&self->track_config);
	patternviewstate_dispose(&self->pvstate);
}

void patternview_on_toggle_properties(PatternView* self,
	PatternViewTabBar* sender)
{		
	assert(self);
	
	psy_ui_component_toggle_visibility(&self->properties.component);
	psy_ui_component_invalidate(&self->component);	
}

void patternview_on_song_changed(PatternView* self, psy_audio_Player* sender)
{	
	assert(self);

	patternview_connect_song(self);	
	psy_audio_sequencecursor_set_order_index(&self->state.pv->cursor,	
		psy_audio_orderindex_make_invalid());	
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
		psy_signal_connect(&song->sequence.signal_trackreposition,
			self, patternview_on_track_reposition);
		psy_signal_connect(&song->patterns.signal_lengthchanged,
			self, patternview_on_pattern_length_changed);
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
	patternview_select_display(self,
		patternviewconfig_pattern_display(config));
	trackconfig_init_columns(&self->track_config,
		patternviewconfig_show_wide_inst_column(config));
	patternview_select_display(self, (PatternDisplayMode)
		patternviewconfig_pattern_display(config));
	psy_ui_component_align(&self->component);	
}

void patternview_on_font(PatternView* self, psy_Property* sender)
{
	patternview_update_font(self);
}

void patternview_on_zoom(PatternView* self, psy_Property* sender)
{
	assert(self);

	patternview_update_font(self);
}

void patternview_on_select_display(PatternView* self, psy_Property* sender)
{
	assert(self);
	
	patternview_select_display(self, (PatternDisplayMode)
		psy_property_item_int(sender));
}

void patternview_on_follow_song(PatternView* self, psy_Property* sender)
{
	assert(self);
	
	psy_ui_component_invalidate(&self->trackerview.component);
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
//	patternproperties_select(&self->properties,
//		self->state.pv->cursor.patternid);
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
		if (psy_audio_blockselection_valid(
				&self->trackerview.grid.state->pv->selection)) {			
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

/*void patternview_on_parameter_tweak(PatternView* self, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue)
{
	assert(self);

	trackergrid_tweak(&self->trackerview.grid, slot, tweak, normvalue);	
}*/

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
	fontinfo = patternviewconfig_font_info(self->pvstate.patconfig, zoomrate);	
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_set_font(&self->component, &font);	
	psy_ui_font_dispose(&font);		
	trackerstate_update_textmetric(&self->state, 
		psy_ui_component_textmetric(&self->component));
	trackerstate_update_textmetric(&self->defaultline.state, 
		psy_ui_component_textmetric(&self->component));
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

void patternview_on_track_reposition(PatternView* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	assert(self);
	
	if (!patternviewstate_single_mode(self->state.pv)) {	
		psy_ui_component_align(&self->trackerview.scroller.pane);
		psy_ui_component_align(&self->trackerview.lines.pane);
		psy_ui_component_invalidate(&self->trackerview.component);
		psy_ui_component_invalidate(&self->trackerview.lines.pane);
	}
}

void patternview_on_pattern_length_changed(PatternView* self,
	psy_audio_Patterns* sender, uintptr_t pattern_idx)
{
	assert(self);
	
	if (patternviewstate_single_mode(self->state.pv)) {
		psy_ui_component_align(&self->trackerview.scroller.pane);
		psy_ui_component_align(&self->trackerview.lines.pane);
		psy_ui_component_invalidate(&self->trackerview.component);
		psy_ui_component_invalidate(&self->trackerview.lines.pane);
	} /* else handled in patternview_on_track_reposition */
}

uintptr_t patternview_section(const PatternView* self)
{
	assert(self);
	
	switch (self->display) {
	case PATTERN_DISPLAYMODE_TRACKER:
		return SECTION_ID_PATTERNVIEW_TRACKER;		
	case PATTERN_DISPLAYMODE_PIANOROLL:
		return SECTION_ID_PATTERNVIEW_PIANO;
	case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL:
		return SECTION_ID_PATTERNVIEW_VSPLIT;
	case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL:
		return SECTION_ID_PATTERNVIEW_HSPLIT;
	default:
		return SECTION_ID_PATTERNVIEW_TRACKER;
	}	
}

void patternview_select_section(PatternView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{
	PatternViewConfig* pvconfig;
	
	assert(self);
	
	if (section == psy_INDEX_INVALID) {
		return;
	}
	pvconfig = &self->workspace->config.visual.patview;
	switch (section) {
	case SECTION_ID_PATTERNVIEW_TRACKER:		
		patternviewconfig_select_pattern_display(pvconfig,
			PATTERN_DISPLAYMODE_TRACKER);
		break;
	case SECTION_ID_PATTERNVIEW_PIANO:		
		patternviewconfig_select_pattern_display(pvconfig,
			PATTERN_DISPLAYMODE_PIANOROLL);
		break;
	case SECTION_ID_PATTERNVIEW_VSPLIT:
		patternviewconfig_select_pattern_display(pvconfig,
			PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL);
		break;
	case SECTION_ID_PATTERNVIEW_HSPLIT:
		patternviewconfig_select_pattern_display(pvconfig,
			PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL);
		break;
	default:
		break;
	}	
}
