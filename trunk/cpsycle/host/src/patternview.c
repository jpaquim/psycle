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
static void patternview_ondestroy(PatternView*);
static void patternview_rebuild(PatternView*);
static void patternview_setpattern(PatternView*, psy_audio_Pattern*);
static void patternview_ontoggleproperties(PatternView*, PatternViewTabBar*);
static void patternview_onsongchanged(PatternView*, Workspace* sender);
static void patternview_connectsong(PatternView*);
static void patternview_onconfigure(PatternView*, PatternViewConfig*,
	psy_Property*);
static void patternview_onmiscconfigure(PatternView*, KeyboardMiscConfig*,
	psy_Property*);
static void patternview_onpatternpropertiesapply(PatternView*,
	psy_ui_Component*);
static void patternview_onfocus(PatternView*);
static void patternview_onmousedown(PatternView*, psy_ui_MouseEvent*);
static void  patternview_onmouseup(PatternView*, psy_ui_MouseEvent*);
static void patternview_onkeydown(PatternView*, psy_ui_KeyboardEvent*);
static void patternview_oncontextmenu(PatternView*, psy_ui_Component*);
static void patternview_oncursorchanged(PatternView*, psy_audio_Sequence*);
static void patternview_beforealign(PatternView*);
static void patternview_ongridscroll(PatternView*, psy_ui_Component*);
static void patternview_onappzoom(PatternView*, psy_ui_AppZoom*);
static void patternview_ontimer(PatternView*, uintptr_t timerid);
static void patternview_numtrackschanged(PatternView*, psy_audio_Pattern*,
	uintptr_t numsongtracks);
static void patternview_onparametertweak(PatternView*, Workspace*,
	int slot, uintptr_t tweak, float normvalue);
static void patternview_oncolresize(PatternView*, TrackerGrid*);
static void patternview_updatescrollstep(PatternView*);
static void patternview_updatedefaultline(PatternView*);
static void patternview_onshow(PatternView*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			patternview_ondestroy;
		vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			patternview_ontimer;
		vtable.beforealign =
			(psy_ui_fp_component_event)
			patternview_beforealign;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			patternview_onmousedown;
		vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			patternview_onmouseup;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			patternview_onkeydown;
		vtable.show =
			(psy_ui_fp_component_show)
			patternview_onshow;
		vtable.onfocus =
			(psy_ui_fp_component_event)
			patternview_onfocus;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void patternview_init(PatternView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,	Workspace* workspace)
{		
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	self->workspace = workspace;	
	self->aligndisplay = TRUE;
	self->updatealign = 0;
	self->zoom = 1.0;	
	psy_ui_component_setstyletype(&self->component, STYLE_PATTERNVIEW);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);		
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(
		&self->notebook));	
	psy_ui_notebook_select(&self->editnotebook, 0);	
	/* Pattern Properties */
	patternproperties_init(&self->properties, &self->component, NULL, workspace);	
	/* Shared states */	
	patterncmds_init(&self->cmds, workspace->song, &workspace->undoredo, NULL,
		&workspace->config.directories);		
	trackconfig_init(&self->trackconfig,
		patternviewconfig_showwideinstcolumn(
			psycleconfig_patview(workspace_conf(workspace))));
	patternviewstate_init(&self->pvstate, &workspace->config.patview,
		workspace_song(workspace), &self->cmds);
	trackerstate_init(&self->state, &self->trackconfig, &self->pvstate);
	/* Configuration */
	psy_signal_connect(&self->pvstate.patconfig->signal_changed, self,
		patternview_onconfigure);
	psy_signal_connect(
		&psycleconfig_misc(workspace_conf(self->workspace))->signal_changed,
		self, patternview_onmiscconfigure);
	/* Linenumbers */
	trackerlinenumberbar_init(&self->left, &self->component, &self->state,
		self->workspace);
	psy_ui_component_setalign(&self->left.component, psy_ui_ALIGN_LEFT);	
	/* Header */
	psy_ui_component_init(&self->headerpane, &self->component, NULL);	
	psy_ui_component_setalign(&self->headerpane, psy_ui_ALIGN_TOP);
	trackerheader_init(&self->header, &self->headerpane,
		&self->trackconfig, &self->state, self->workspace);
	psy_ui_component_setalign(&self->header.component,
		psy_ui_ALIGN_FIXED);
	/* Defaultline */
	patterndefaultline_init(&self->defaultline, &self->component,
		&self->trackconfig, workspace);	
	psy_signal_connect(&self->defaultline.grid.signal_colresize, self,
		patternview_oncolresize);
	/* Tracker */
	trackerview_init(&self->trackerview, &self->editnotebook.component,		
		&self->state, workspace);
	psy_signal_connect(&self->trackerview.grid.signal_colresize, self,
		patternview_oncolresize);
	/* Pianoroll */
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, &self->pvstate,
		workspace);	
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self,
		patternview_onpatternpropertiesapply);
	/* Blockmenu */
	patternblockmenu_init(&self->blockmenu, patternview_base(self),
		NULL, &self->swingfillview, &self->transformpattern,
		&self->interpolatecurveview, &self->pvstate);
	/* TransformPattern */
	transformpatternview_init(&self->transformpattern, &self->component,
		NULL, workspace);
	/* SwingFill */
	swingfillview_init(&self->swingfillview, &self->component,
		NULL, &self->pvstate);
	/* Interpolate */
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0,
		0, 0, workspace);
	psy_ui_component_setalign(&self->interpolatecurveview.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);	
	/* Tabbar */
	patternviewtabbar_init(&self->tabbar, tabbarparent, workspace);
	psy_signal_connect(&self->tabbar.signal_toggle_properties, self,
		patternview_ontoggleproperties);
	patternview_selectdisplay(self, PATTERN_DISPLAYMODE_TRACKER);		
	psy_signal_connect(&self->tabbar.contextbutton.signal_clicked, self,
		patternview_oncontextmenu);
	/* Connect */
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);		
	psy_signal_connect(&self->trackerview.grid.component.signal_scroll, self,
		patternview_ongridscroll);	
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		patternview_onparametertweak);
	patternview_connectsong(self);
	psy_signal_connect(&psy_ui_app_zoom(psy_ui_app())->signal_zoom, self,
		patternview_onappzoom);	
	patternview_setpattern(self, NULL);
	patternview_rebuild(self);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void patternview_ondestroy(PatternView* self)
{	
	assert(self);

	trackerstate_dispose(&self->state);
	trackconfig_dispose(&self->trackconfig);
	patternviewstate_dispose(&self->pvstate);
}

void patternview_ontoggleproperties(PatternView* self,
	PatternViewTabBar*sender)
{		
	assert(self);
	
	psy_ui_component_togglevisibility(&self->properties.component);
	psy_ui_component_invalidate(&self->component);	
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{	
	assert(self);

	patternviewstate_setpattern(&self->pvstate, pattern);
	interpolatecurveview_setpattern(&self->interpolatecurveview, pattern);
	trackergrid_setpattern(&self->trackerview.grid, pattern);	
	trackergrid_build(&self->defaultline.grid);
	pianoroll_setpattern(&self->pianoroll, pattern);
	patternproperties_setpattern(&self->properties, pattern);
	interpolatecurveview_setpattern(&self->interpolatecurveview,
		patternviewstate_pattern(self->state.pv));
	psy_ui_component_align(&self->left.linenumberpane);
	psy_ui_component_invalidate(&self->component);
}

void patternview_onsongchanged(PatternView* self, Workspace* sender)
{	
	assert(self);

	patternview_connectsong(self);	
	self->state.pv->pattern = NULL;
	self->state.pv->cursor.orderindex = psy_audio_orderindex_zero();
	self->state.pv->cursor.patternid = psy_INDEX_INVALID;
}

void patternview_connectsong(PatternView* self)
{	
	psy_audio_Song* song;

	assert(self);

	song = workspace_song(self->workspace);	
	patternviewstate_setsong(self->state.pv, song);
	patternviewstate_setsong(self->defaultline.grid.state->pv, song);
	if (song) {		
		psy_signal_connect(&song->patterns.signal_numsongtrackschanged, self,
			patternview_numtrackschanged);
		psy_signal_connect(&song->sequence.signal_cursorchanged, self,
			patternview_oncursorchanged);
	}
}

void patternview_onpatternpropertiesapply(PatternView* self,
	psy_ui_Component* sender)
{	
	assert(self);

	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onfocus(PatternView* self)
{
	assert(self);

	if (psy_ui_tabbar_selected(&self->tabbar.tabbar) == 1) { /* Pianoroll */
		psy_ui_component_setfocus(&self->pianoroll.grid.component);		
	} else {
		psy_ui_component_setfocus(&self->trackerview.grid.component);
	}
}

void patternview_oncontextmenu(PatternView* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_togglevisibility(patternblockmenu_base(&self->blockmenu));
	psy_ui_component_invalidate(&self->component);
}

void patternview_selectdisplay(PatternView* self, PatternDisplayMode display)
{
	assert(self);

	if (self->state.pv->display != display) {
		self->state.pv->display = display;
		switch (self->state.pv->display) {
		case PATTERN_DISPLAYMODE_TRACKER:
		case PATTERN_DISPLAYMODE_PIANOROLL:
			psy_ui_tabbar_mark(&self->tabbar.tabbar, self->pvstate.display);
			psy_ui_notebook_full(&self->editnotebook);
			psy_ui_notebook_select(&self->editnotebook, self->pvstate.display);
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

void patternview_onconfigure(PatternView* self, PatternViewConfig* config,
	psy_Property* property)
{	
	assert(self);

	patternview_updatefont(self);
	patternviewstate_configure(&self->pvstate);		
	trackconfig_initcolumns(&self->trackconfig,
		patternviewconfig_showwideinstcolumn(config));				
	patternview_selectdisplay(self, (PatternDisplayMode)
		patternviewconfig_patterndisplay(config));	
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align(&self->component);
	}
}

void patternview_onmiscconfigure(PatternView* self, KeyboardMiscConfig* config,
	psy_Property* property)
{	
	assert(self);

	patternviewstate_configure_keyboard(&self->pvstate, config);
}

void patternview_beforealign(PatternView* self)
{
	psy_ui_Size preferredheadersize;	

	assert(self);

	preferredheadersize = psy_ui_component_preferredsize(&self->header.component, NULL);
	trackerlinenumberslabel_setheaderheight(&self->left.linenumberslabel,
		psy_ui_value_px(&preferredheadersize.height,
		psy_ui_component_textmetric(&self->header.component), NULL));				
}

void patternview_ongridscroll(PatternView* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scrollleft_px(&self->trackerview.grid.component) !=
		psy_ui_component_scrollleft_px(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->trackerview.grid.component));
		psy_ui_component_invalidate(&self->headerpane);
		psy_ui_component_setscrollleft(&self->defaultline.grid.component,
			psy_ui_component_scrollleft(&self->trackerview.grid.component));
	}
	if (psy_ui_component_scrolltop_px(&self->trackerview.grid.component) !=
			psy_ui_component_scrolltop_px(&self->left.linenumbers.component)) {
		psy_ui_component_setscrolltop(&self->left.linenumbers.component,
			psy_ui_component_scrolltop(&self->trackerview.grid.component));
		psy_ui_component_invalidate(&self->left.linenumberpane);
	}
}

void patternview_oncursorchanged(PatternView* self, psy_audio_Sequence* sender)
{
	assert(self);

	if (!workspace_song(self->workspace)) {
		self->updatealign = 1;
		patternview_setpattern(self, NULL);
	} else if (!self->state.pv->pattern || sender->cursor.patternid !=
			sender->lastcursor.patternid) {
		self->updatealign = 1;
		patternview_setpattern(self, psy_audio_sequence_pattern(
			psy_audio_song_sequence(workspace_song(self->workspace)),
				sender->cursor.orderindex));		
	}
	self->state.pv->cursor = sender->cursor;	
}

void patternview_onshow(PatternView* self)
{	
	assert(self);

	/* center splitview when first displayed */
	if (self->aligndisplay && self->state.pv->display > PATTERN_DISPLAYMODE_PIANOROLL) {
		PatternDisplayMode display;

		self->aligndisplay = FALSE;
		display = self->state.pv->display;
		self->state.pv->display = PATTERN_DISPLAYMODE_TRACKER;
		patternview_selectdisplay(self, display);
	}
}

void patternview_ontimer(PatternView* self, uintptr_t timerid)
{
	assert(self);

	if (trackergrid_checkupdate(&self->trackerview.grid)) {
		psy_ui_component_invalidate(&self->component);		
	}
	if (self->updatealign == 1) {
		psy_ui_component_align(&self->left.linenumberpane);
		psy_ui_component_invalidate(&self->left.linenumberpane);
		if (psy_ui_component_visible(&self->trackerview.scroller.component)) {
			psy_ui_component_align(&self->trackerview.scroller.pane);
		}
		if (psy_ui_component_visible(&self->pianoroll.scroller.pane)) {
			psy_ui_component_align(&self->pianoroll.scroller.pane);
			psy_ui_component_align(&self->pianoroll.top);
		}		
	}
	if (self->updatealign > 0) {
		--self->updatealign;
	}
}

void patternview_numtrackschanged(PatternView* self, psy_audio_Pattern* sender,
	uintptr_t numsongtracks)
{
	assert(self);

	patternview_rebuild(self);	
}

void patternview_rebuild(PatternView* self)
{
	assert(self);

	trackerheader_build(&self->header);	
	trackergrid_build(&self->trackerview.grid);	
	trackergrid_build(&self->defaultline.grid);
	trackerheader_build(&self->header);		
	psy_ui_component_align_full(&self->trackerview.scroller.component);
	psy_ui_component_invalidate(&self->component);	
}

void patternview_onmousedown(PatternView* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 2) {
		if (psy_ui_component_visible(interpolatecurveview_base(
				&self->interpolatecurveview))) {
			psy_ui_component_hide_align(interpolatecurveview_base(
				&self->interpolatecurveview));			
		} else {
			psy_ui_component_togglevisibility(patternblockmenu_base(
				&self->blockmenu));						
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void  patternview_onmouseup(PatternView* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 1 && self->trackerview.grid.state->pv->selection.valid) {
		interpolatecurveview_setpattern(&self->interpolatecurveview,
			patternviewstate_pattern(self->state.pv));
		interpolatecurveview_setselection(&self->interpolatecurveview,
			trackergrid_selection(&self->trackerview.grid));
		transformpatternview_setpatternselection(&self->transformpattern,
			trackergrid_selection(&self->trackerview.grid));
	}
}

void patternview_onkeydown(PatternView* self, psy_ui_KeyboardEvent* ev)
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

void patternview_oncolresize(PatternView* self, TrackerGrid* sender)
{
	assert(self);

	psy_ui_component_align(&self->trackerview.scroller.pane);
	psy_ui_component_align(patterndefaultline_base(&self->defaultline));
	psy_ui_component_align(&self->headerpane);
	psy_ui_component_invalidate(&self->component);
}

void patternview_onparametertweak(PatternView* self, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue)
{
	assert(self);

	trackergrid_tweak(&self->trackerview.grid, slot, tweak, normvalue);	
}

void patternview_onappzoom(PatternView* self, psy_ui_AppZoom* sender)
{
	assert(self);

	patternview_updatefont(self);
}

void patternview_updatefont(PatternView* self)
{	
	psy_ui_FontInfo fontinfo;	
	psy_ui_Font font;
	double zoomrate;
	
	assert(self);

	zoomrate = psy_ui_app_zoomrate(psy_ui_app()) * self->zoom;
	fontinfo = patternviewconfig_readfont(self->pvstate.patconfig, zoomrate);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(&self->component, &font);	
	psy_ui_font_dispose(&font);	
	trackerstate_updatemetric(&self->state,
		psy_ui_component_textmetric(&self->component), zoomrate);
	trackerstate_updatemetric(self->defaultline.grid.state,
		psy_ui_component_textmetric(&self->component), zoomrate);
	patternview_updatescrollstep(self);
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align_full(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}

void patternview_updatescrollstep(PatternView* self)
{
	psy_ui_Value step;

	assert(self);

	step = psy_ui_value_make_px(trackerstate_trackwidth(&self->state, psy_INDEX_INVALID));
	psy_ui_component_setscrollstep_width(trackergrid_base(
		&self->trackerview.grid), step);
	psy_ui_component_setscrollstep_width(trackergrid_base(&self->defaultline.grid),
		step);
	psy_ui_component_setscrollstep_width(trackerheader_base(&self->header),
		step);
	psy_ui_component_setscrollstep_height(trackergrid_base(
		&self->trackerview.grid), self->state.lineheight);	
}
