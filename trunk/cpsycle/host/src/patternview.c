/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternview.h"
/* host */
#include "styles.h"
/* audio */
#include <patternio.h>
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static char patternfilter[] = "Pattern (*.psb)" "|*.psb";

/* prototypes */
static void patternview_ondestroy(PatternView*);
static void patternview_rebuild(PatternView*);
static void patternview_inittabbar(PatternView*,
	psy_ui_Component* tabbarparent);
static void patternview_initblockmenu(PatternView*);
static void patternview_ontabbarchange(PatternView*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void patternview_onsongchanged(PatternView*, Workspace* sender,
	int flag, psy_audio_Song*);
static void patternview_connectsong(PatternView*);
static void patternview_onconfigurepatternview(PatternView*,
	PatternViewConfig*, psy_Property*);
static void patternview_onmiscconfigure(PatternView*, KeyboardMiscConfig*,
	psy_Property*);
static void patternview_readfont(PatternView*);
static void patternview_onpatternpropertiesapply(PatternView*,
	psy_ui_Component* sender);
static void patternview_onfocus(PatternView*);
static void patternview_onmousedown(PatternView*, psy_ui_MouseEvent*);
static void  patternview_onmouseup(PatternView*, psy_ui_MouseEvent*);
static void patternview_onkeydown(PatternView*, psy_ui_KeyboardEvent*);
static void patternview_oncontextmenu(PatternView*, psy_ui_Component* sender);
static void patternview_initbasefontsize(PatternView*);
static void patternview_onsequencechanged(PatternView*, psy_audio_Sequence*
	sender);
static void patternview_oncursorchanged(PatternView*,
	psy_audio_Sequence* sender);
static void patternview_beforealign(PatternView*);
static void patternview_ongridscroll(PatternView*, psy_ui_Component* sender);
static void patternview_onzoomboxchanged(PatternView*, ZoomBox* sender);
static void patternview_onappzoom(PatternView*, psy_ui_AppZoom* sender);
static void patternview_updatezoom(PatternView*);
static void patternview_setfont(PatternView*, const psy_ui_Font*);
static void patternview_ontimer(PatternView*, uintptr_t timerid);
static void patternview_updateksin(PatternView*);
static void patternview_numtrackschanged(PatternView*, psy_audio_Pattern*,
	uintptr_t numsongtracks);
static PatternViewTarget patternview_target(PatternView*, psy_EventDriver*
	sender);
static void patternview_onparametertweak(PatternView*, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue);
static void patternview_oncolresize(PatternView*, TrackerGrid* sender);
static void patternview_updatescrollstep(PatternView*);
static const psy_audio_BlockSelection* patternview_blockselection(const
	PatternView*);
static void patternview_swingfill(PatternView*, psy_ui_Component* sender);
static void patternview_onthemechanged(PatternView*, PatternViewConfig*,
	psy_Property* theme);
static uintptr_t patternview_display(const PatternView*);
static void patternview_onshow(PatternView*);
/* vtable */
static psy_ui_ComponentVtable patternview_vtable;
static bool patternview_vtable_initialized = FALSE;

static void patternview_vtable_init(PatternView* self)
{
	if (!patternview_vtable_initialized) {
		patternview_vtable = *(self->component.vtable);
		patternview_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			patternview_ondestroy;
		patternview_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			patternview_ontimer;
		patternview_vtable.beforealign =
			(psy_ui_fp_component_onalign)
			patternview_beforealign;
		patternview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			patternview_onmousedown;
		patternview_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			patternview_onmouseup;
		patternview_vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			patternview_onkeydown;
		patternview_vtable.show =
			(psy_ui_fp_component_show)
			patternview_onshow;
		patternview_vtable.onfocus =
			(psy_ui_fp_component_onfocus)
			patternview_onfocus;
		patternview_vtable_initialized = TRUE;
	}
	self->component.vtable = &patternview_vtable;
}

/* implementation */
void patternview_init(PatternView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,	Workspace* workspace)
{		
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_doublebuffer(&self->component);	
	patternview_vtable_init(self);	
	self->workspace = workspace;
	self->trackmodeswingfill = TRUE;	
	self->aligndisplay = TRUE;
	self->updatealign = 0;	
	patternview_initbasefontsize(self);		
	psy_ui_component_setstyletype(&self->component, STYLE_PATTERNVIEW);
	psy_ui_notebook_init(&self->notebook, &self->component, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(&self->notebook),
		psy_ui_NOBACKGROUND);
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(
		&self->notebook), NULL);
	psy_ui_component_setbackgroundmode(&self->editnotebook.component,
		psy_ui_NOBACKGROUND);
	psy_ui_notebook_select(&self->editnotebook, 0);
	/* Skin */
	patternviewskin_init(&self->skin);
	patternviewskin_settheme(&self->skin,
		workspace->config.patview.theme,
		dirconfig_skins(&workspace->config.directories));
	psy_signal_connect(&psycleconfig_patview(
		workspace_conf(self->workspace))->signal_themechanged,
		self, patternview_onthemechanged);
	psy_signal_connect(
		&psycleconfig_patview(workspace_conf(self->workspace))->signal_changed,
		self, patternview_onconfigurepatternview);
	psy_signal_connect(&psycleconfig_misc(
		workspace_conf(self->workspace))->signal_changed,
		self, patternview_onmiscconfigure);
	/* Pattern Properties */
	patternproperties_init(&self->properties, &self->component, NULL,
		&self->skin, workspace);
	psy_ui_component_setalign(&self->properties.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->properties.component);	
	/* shared states */	
	trackconfig_init(&self->trackconfig,
		patternviewconfig_showwideinstcolumn(
			psycleconfig_patview(workspace_conf(workspace))));
	trackerstate_init(&self->state, &self->trackconfig,
		workspace_song(workspace));	
	self->state.pv.skin = &self->skin;
	/* linenumbers */
	trackerlinenumberbar_init(&self->left, &self->component, &self->component,
		&self->state, self->workspace);
	psy_ui_component_setalign(&self->left.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->left.zoombox.signal_changed, self,
		patternview_onzoomboxchanged);	
	/* header */
	psy_ui_component_init(&self->headerpane, &self->component, &self->component);	
	psy_ui_component_setalign(&self->headerpane, psy_ui_ALIGN_TOP);
	trackerheader_init(&self->header, &self->headerpane,
		&self->trackconfig, &self->state, self->workspace);
	psy_ui_component_setalign(&self->header.component,
		psy_ui_ALIGN_FIXED);
	/* default line */
	psy_ui_component_init(&self->griddefaultspane, &self->component, &self->component);	
	psy_ui_component_setalign(&self->griddefaultspane, psy_ui_ALIGN_TOP);	
	trackergrid_init(&self->griddefaults, &self->griddefaultspane, NULL,
		&self->trackconfig, NULL, workspace);
	psy_ui_component_setwheelscroll(trackergrid_base(&self->griddefaults), 0);
	self->griddefaults.defaultgridstate.pv.skin = &self->skin;
	psy_ui_component_setalign(&self->griddefaults.component,
		psy_ui_ALIGN_FIXED);	
	self->griddefaults.state->drawbeathighlights = FALSE;
	self->griddefaults.preventeventdriver = TRUE;
	self->griddefaults.state->synccursor = FALSE;
	self->griddefaults.state->showresizecursor = TRUE;
	trackergrid_setpattern(&self->griddefaults,
		&workspace_player(self->workspace)->patterndefaults);
	trackergrid_build(&self->griddefaults);
	psy_signal_connect(&self->griddefaults.signal_colresize, self,
		patternview_oncolresize);
	/* tracker view */
	trackerview_init(&self->trackerview, &self->editnotebook.component,		
		&self->trackconfig, &self->state, workspace);		
	psy_signal_connect(&self->trackerview.grid.signal_colresize, self,
		patternview_oncolresize);
	/* PianoRoll */
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, &self->skin,
		workspace);	
	if (workspace->song) {
		patternview_setpattern(self, psy_audio_patterns_at(
			&workspace->song->patterns, 0));		
	} else {
		patternview_setpattern(self, NULL);
	}
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self,
		patternview_onpatternpropertiesapply);
	/* blockmenu */
	patternview_initblockmenu(self);
	/* TransformPatternView */
	transformpatternview_init(&self->transformpattern, &self->component,
		workspace);
	psy_ui_component_setalign(
		transformpatternview_base(&self->transformpattern), psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(transformpatternview_base(&self->transformpattern));
	/* SwingFillView */
	swingfillview_init(&self->swingfillview, &self->component, workspace);
	psy_signal_connect(&self->swingfillview.apply.signal_clicked, self,
		patternview_swingfill);
	psy_ui_component_setalign(swingfillview_base(&self->swingfillview),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(swingfillview_base(&self->swingfillview));
	/* Interpolate View */
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0,
		0, 0, workspace);
	psy_ui_component_setalign(&self->interpolatecurveview.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);
	/* psy_signal_connect(&self->interpolatecurveview.signal_cancel, self,
			trackerview_oninterpolatecurveviewoncancel); */
	/* Tabbar */
	patternview_inittabbar(self, tabbarparent);
	patternview_selectdisplay(self, PATTERN_DISPLAYMODE_TRACKER);
	psy_ui_button_init(&self->contextbutton, &self->sectionbar, NULL);
	psy_ui_button_seticon(&self->contextbutton, psy_ui_ICON_MORE);	
	psy_ui_component_setalign(psy_ui_button_base(&self->contextbutton),
		psy_ui_ALIGN_RIGHT);	
	psy_signal_connect(&self->contextbutton.signal_clicked,
		self, patternview_oncontextmenu);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);		
	psy_signal_connect(&self->trackerview.grid.component.signal_scroll, self,
		patternview_ongridscroll);	
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		patternview_onparametertweak);
	patternview_connectsong(self);
	psy_signal_connect(&psy_ui_app_zoom(psy_ui_app())->signal_zoom, self,
		patternview_onappzoom);	
	patternview_rebuild(self);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void patternview_ondestroy(PatternView* self)
{	
	trackerstate_dispose(&self->state);
	trackconfig_dispose(&self->trackconfig);
	patternviewskin_dispose(&self->skin);
}

void patternview_initbasefontsize(PatternView* self)
{	
	psy_ui_FontInfo fontinfo;

	fontinfo = psy_ui_font_fontinfo(psy_ui_component_font(&self->component));
	self->baselfheight = fontinfo.lfHeight;	
}

void patternview_inittabbar(PatternView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_Tab* tab;

	psy_ui_component_init_align(&self->sectionbar, tabbarparent, NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_init(&self->tabbar, &self->sectionbar);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_append_tabs(&self->tabbar,
		"patternview.tracker", "patternview.roll", "patternview.split",
		"patternview.vert", "patternview.horz", "patternview.properties",
		NULL);
	tab = psy_ui_tabbar_tab(&self->tabbar, 2);
	if (tab) {
		psy_ui_tab_setmode(tab, psy_ui_TABMODE_LABEL);
	}
	tab = psy_ui_tabbar_tab(&self->tabbar, 0);
	if (tab) {
		psy_ui_Margin margin;
		
		margin = psy_ui_component_margin(&tab->component);
		margin.left = psy_ui_value_make_ew(1.0);
		psy_ui_component_setmargin(&tab->component, margin);
	}
	tab = psy_ui_tabbar_tab(&self->tabbar, 5);
	if (tab) {
		tab->istoggle = TRUE;
	}
	psy_ui_tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);
}

void patternview_ontabbarchange(PatternView* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{		
	if (tabindex <= PATTERN_DISPLAYMODE_NUM) {
		static PatternDisplayMode display[] = {
			PATTERN_DISPLAYMODE_TRACKER,
			PATTERN_DISPLAYMODE_PIANOROLL,
			PATTERN_DISPLAYMODE_NUM,
			PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,
			PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL
		};
		workspace_selectpatterndisplay(self->workspace, display[tabindex]);
	} else if (tabindex == 5) {
		psy_ui_component_togglevisibility(&self->properties.component);
	}	
	psy_ui_component_align_full(psy_ui_notebook_activepage(&self->notebook));
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{	
	interpolatecurveview_setpattern(&self->interpolatecurveview, pattern);
	trackergrid_setpattern(&self->trackerview.grid, pattern);	
	trackergrid_build(&self->griddefaults);	
	pianoroll_setpattern(&self->pianoroll, pattern);
	patternproperties_setpattern(&self->properties, pattern);
	interpolatecurveview_setpattern(&self->interpolatecurveview,
		patternviewstate_pattern(&self->state.pv));
	psy_ui_component_align(&self->left.linenumberpane);
	psy_ui_component_invalidate(&self->component);
}

void patternview_onsongchanged(PatternView* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{		
	patternview_connectsong(self);
	self->state.pv.pattern = NULL;
	self->state.pv.cursor.orderindex = psy_audio_orderindex_zero();
	self->state.pv.cursor.patternid = psy_INDEX_INVALID;
}

void patternview_connectsong(PatternView* self)
{	
	patternviewstate_setsong(&self->state.pv, workspace_song(self->workspace));
	patternviewstate_setsong(&self->griddefaults.state->pv, workspace_song(
		self->workspace));
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&self->workspace->song->sequence.signal_changed,
			self, patternview_onsequencechanged);
		psy_signal_connect(
			&self->workspace->song->patterns.signal_numsongtrackschanged, self,
			patternview_numtrackschanged);
		psy_signal_connect(
			&self->workspace->song->sequence.signal_cursorchanged, self,
			patternview_oncursorchanged);
	}
}

void patternview_onsequencechanged(PatternView* self,
	psy_audio_Sequence* sender)
{
	self->updatealign = 1;
}

void patternview_onpatternpropertiesapply(PatternView* self,
	psy_ui_Component* sender)
{	
	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onfocus(PatternView* self)
{
	if (psy_ui_tabbar_selected(&self->tabbar) == 1) { /* Pianoroll */
		psy_ui_component_setfocus(&self->pianoroll.grid.component);
		return;
	}
	psy_ui_component_setfocus(&self->trackerview.grid.component);
}

void patternview_oncontextmenu(PatternView* self, psy_ui_Component* sender)
{
	patternview_toggleblockmenu(self);	
}

void patternview_selectdisplay(PatternView* self, PatternDisplayMode display)
{
	if (self->state.pv.display == display) {
		return;
	}
	self->state.pv.display = display;
	self->pianoroll.gridstate.pv.display = display;
	psy_signal_prevent(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);	
	switch (self->state.pv.display) {
	case PATTERN_DISPLAYMODE_TRACKER:
	case PATTERN_DISPLAYMODE_PIANOROLL:
		if (psy_ui_notebook_splitactivated(&self->editnotebook)) {
			psy_ui_notebook_full(&self->editnotebook);
		}
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_notebook_select(&self->editnotebook, self->state.pv.display);
		psy_ui_tabbar_select(&self->tabbar, self->state.pv.display);
		break;
	case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL:
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_tabbar_select(&self->tabbar, 0);
		psy_ui_notebook_split(&self->editnotebook, psy_ui_VERTICAL);
		break;
	case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL:
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_tabbar_select(&self->tabbar, 0);
		psy_ui_notebook_split(&self->editnotebook, psy_ui_HORIZONTAL);
		break;
	default:
		psy_ui_notebook_select(&self->notebook, 1);
		psy_ui_tabbar_select(&self->tabbar, 1);
		break;
	}
	psy_signal_enable(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);
}

uintptr_t patternview_display(const PatternView* self)
{
	return self->state.pv.display;
}

void patternview_onconfigurepatternview(PatternView* self,
	PatternViewConfig* config, psy_Property* property)
{	
	/* self->header.usebitmapskin = patternviewconfig_useheaderbitmap(config); */
	self->left.linenumberslabel.useheaderbitmap = TRUE;
	/* patternviewconfig_useheaderbitmap(config);
	   self->header.usebitmapskin; */
	if (patternviewconfig_linenumbers(config)) {
		patternview_showlinenumbers(self);
	} else {
		patternview_hidelinenumbers(self);
	}
	if (patternviewconfig_showbeatoffset(config)) {
		patternview_showbeatnumbers(self);
	} else {
		patternview_hidebeatnumbers(self);
	}
	trackerlinenumbers_showlinenumbercursor(&self->left.linenumbers,
		patternviewconfig_linenumberscursor(config));
	trackerlinenumbers_showlinenumbersinhex(&self->left.linenumbers,
		patternviewconfig_linenumbersinhex(config));
	if (psy_ui_component_visible(&self->griddefaultspane) !=
			patternviewconfig_defaultline(config)) {		
		if (patternviewconfig_defaultline(config)) {
			trackerlinenumberslabel_showdefaultline(
				&self->left.linenumberslabel);
			psy_ui_component_show_align(&self->griddefaultspane);			
			psy_ui_component_align(trackerlinenumberbar_base(&self->left));
		} else {
			trackerlinenumberslabel_hidedefaultline(
				&self->left.linenumberslabel);
			psy_ui_component_hide_align(&self->griddefaultspane);
			psy_ui_component_align(trackerlinenumberbar_base(&self->left));
		}
	}
	self->trackerview.grid.wraparound = patternviewconfig_wraparound(config);
	trackergrid_showemptydata(&self->trackerview.grid,
		patternviewconfig_drawemptydata(config));
	trackergrid_setcentermode(&self->trackerview.grid,
		patternviewconfig_centercursoronscreen(config));
	if (patternviewconfig_showbeatoffset(config)) {
		trackerlinenumberslabel_showbeatoffset(&self->left.linenumberslabel);		
	} else {
		trackerlinenumberslabel_hidebeatoffset(&self->left.linenumberslabel);		
	}
	trackconfig_initcolumns(&self->trackconfig,
		patternviewconfig_showwideinstcolumn(config));
	self->trackerview.grid.notestabmode = self->griddefaults.notestabmode =
		patternviewconfig_notetabmode(config);
	patternview_updateksin(self);
	if (patternviewconfig_issinglepatterndisplay(config)) {
		patternviewstate_displaypattern(&self->state.pv);		
	} else {
		patternviewstate_displaysequence(&self->state.pv);
	}
	if (patternviewconfig_centercursoronscreen(config)) {
		trackergrid_centeroncursor(&self->trackerview.grid);
	}	
	patternview_selectdisplay(self,
		(PatternDisplayMode)patternviewconfig_patterndisplay(config));	
	if (patternviewconfig_issmoothscrolling(config)) {
		psy_ui_scroller_scrollsmooth(&self->trackerview.scroller);
		psy_ui_scroller_scrollsmooth(&self->pianoroll.scroller);
	} else {
		psy_ui_scroller_scrollfast(&self->trackerview.scroller);
		psy_ui_scroller_scrollfast(&self->pianoroll.scroller);
	}
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align(&self->component);
	}
}

void patternview_onmiscconfigure(PatternView* self, KeyboardMiscConfig* config,
	psy_Property* property)
{	
	patternview_readfont(self);
	if (keyboardmiscconfig_ft2home(config)) {
		trackergrid_enableft2home(&self->trackerview.grid);
	} else {
		trackergrid_enableithome(&self->trackerview.grid);
	}
	if (keyboardmiscconfig_ft2delete(config)) {
		trackergrid_enableft2delete(&self->trackerview.grid);
	} else {
		trackergrid_enableitdelete(&self->trackerview.grid);
	}
	if (keyboardmiscconfig_movecursoronestep(config)) {
		trackergrid_enablemovecursoronestep(&self->trackerview.grid);
	} else {
		trackergrid_disablemovecursoronestep(&self->trackerview.grid);
	}
	if (keyboardmiscconfig_effcursoralwaysdown(config)) {
		trackergrid_enableeffcursoralwaysdown(&self->trackerview.grid);
	} else {
		trackergrid_disableffcursoralwaysdown(&self->trackerview.grid);
	}	
	patternviewstate_setpgupdownstep(&self->state.pv, (PatternCursorStepMode)
		keyboardmiscconfig_pgupdowntype(config));
	self->state.pv.pgupdownstepmode = (PatternCursorStepMode)
		keyboardmiscconfig_pgupdowntype(config);
	patternviewstate_setpgupdownstep(&self->pianoroll.gridstate.pv, (PatternCursorStepMode)
		keyboardmiscconfig_pgupdowntype(config));
	self->pianoroll.gridstate.pv.pgupdownstepmode = (PatternCursorStepMode)
		keyboardmiscconfig_pgupdowntype(config);	
}

void patternview_readfont(PatternView* self)
{
	psy_Property* pv;

	pv = psy_property_findsection(&self->workspace->config.config,
		"visual.patternview");
	if (pv) {
		psy_ui_FontInfo fontinfo;	
		psy_ui_Font font;
		double factor;

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));		
		factor = (double)fontinfo.lfHeight / self->baselfheight *
			psy_ui_app_zoomrate(psy_ui_app());
#if PSYCLE_USE_TK == PSYCLE_TK_X11
		/* todo */
		self->state.lineheight = psy_ui_value_make_eh(1.0);		
		fontinfo.lfHeight = 18;
		self->baselfheight = 18;
#else			
		self->state.lineheight = psy_ui_value_make_eh(factor);
		fontinfo.lfHeight = (int)(-16 * factor);
#endif		
		psy_ui_font_init(&font, &fontinfo);
		patternview_setfont(self, &font);
		psy_ui_font_dispose(&font);		
	}
}

void patternview_beforealign(PatternView* self)
{
	psy_ui_Size preferredheadersize;	

	preferredheadersize = psy_ui_component_preferredsize(&self->header.component, NULL);
	trackerlinenumberslabel_setheaderheight(&self->left.linenumberslabel,
		psy_ui_value_px(&preferredheadersize.height,
		psy_ui_component_textmetric(&self->header.component), NULL));				
}

void patternview_ongridscroll(PatternView* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleftpx(&self->trackerview.grid.component) !=
		psy_ui_component_scrollleftpx(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->trackerview.grid.component));
		psy_ui_component_invalidate(&self->headerpane);
		psy_ui_component_setscrollleft(&self->griddefaults.component,
			psy_ui_component_scrollleft(&self->trackerview.grid.component));
	}
	if (psy_ui_component_scrolltop_px(&self->trackerview.grid.component) !=
			psy_ui_component_scrolltop_px(&self->left.linenumbers.component)) {
		psy_ui_component_setscrolltop(&self->left.linenumbers.component,
			psy_ui_component_scrolltop(&self->trackerview.grid.component));
		psy_ui_component_invalidate(&self->left.linenumberpane);
	}
}

void patternview_showlinenumbers(PatternView* self)
{		
	psy_ui_component_show_align(&self->left.component);	
}

void patternview_hidelinenumbers(PatternView* self)
{		
	psy_ui_component_hide_align(&self->left.component);
}

void patternview_showbeatnumbers(PatternView* self)
{
	self->left.linenumbers.showbeat = TRUE;
	psy_ui_component_align(&self->component);	
}

void patternview_hidebeatnumbers(PatternView* self)
{	
	self->left.linenumbers.showbeat = FALSE;
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align(&self->component);
	}
}

void patternview_setfont(PatternView* self, const psy_ui_Font* font)
{	
	const psy_ui_TextMetric* tm;	
	
	psy_ui_component_setfont(&self->component, font);	
	tm = psy_ui_component_textmetric(&self->trackerview.grid.component);
	trackerstate_setfont(&self->state, psy_ui_component_font(&self->component),
		tm);
	trackerstate_setfont(self->griddefaults.state,
		psy_ui_component_font(&self->component), tm);
	patternview_updatescrollstep(self);
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align_full(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}

void patternview_oncursorchanged(PatternView* self, psy_audio_Sequence* sender)
{		
	psy_audio_SequenceCursor oldcursor;
	psy_audio_SequenceCursor cursor;

	if (!workspace_song(self->workspace)) {
		patternview_setpattern(self, NULL);
		return;
	}	
	oldcursor = self->state.pv.cursor;
	cursor = sender->cursor;
	if (!self->state.pv.pattern ||
			cursor.patternid != oldcursor.patternid) {
		patternview_setpattern(self, psy_audio_sequence_pattern(
			psy_audio_song_sequence(workspace_song(self->workspace)),
				cursor.orderindex));		
	}
	self->state.pv.cursor = cursor;
	/* lpb changed? */
	if (self->state.pv.cursor.lpb != oldcursor.lpb) {		
		psy_ui_component_align(&self->trackerview.scroller.pane);
		psy_ui_component_align(&self->left.linenumberpane);		
		psy_ui_component_invalidate(trackergrid_base(&self->trackerview.grid));
		psy_ui_component_invalidate(trackerlinenumbers_base(
			&self->left.linenumbers));
		return;
	}	
	if (!psy_audio_sequencecursor_equal(&sender->cursor, &oldcursor)) {
		if (psy_audio_player_playing(&self->workspace->player) &&
				workspace_followingsong(self->workspace)) {			
			psy_dsp_big_beat_t lastposition;
			psy_dsp_big_beat_t currposition;
			intptr_t lastline;
			intptr_t currline;

			lastposition = psy_audio_sequencecursor_offset_abs(&sender->lastcursor);			
			currposition = psy_audio_sequencecursor_offset_abs(&sender->cursor);
			lastline = trackerstate_beattoline(&self->state, lastposition);
			currline = trackerstate_beattoline(&self->state, currposition);
			trackergrid_invalidateline(&self->trackerview.grid, lastline);
			trackergrid_invalidateline(&self->trackerview.grid, currline);			
			trackerlinenumbers_invalidateline(&self->left.linenumbers, lastline);
			trackerlinenumbers_invalidateline(&self->left.linenumbers, currline);
			if (currposition >= cursor.seqoffset &&
					currposition < cursor.seqoffset +
					patternviewstate_pattern(&self->state.pv)->length) {				
				if (lastposition < currposition) {
					trackergrid_scrolldown(&self->trackerview.grid, cursor);						
				} else {
					trackergrid_scrollup(&self->trackerview.grid, cursor);						
				}
			}			
			trackergrid_storecursor(&self->trackerview.grid);			
		} else if (self->state.midline) {
			trackergrid_centeroncursor(&self->trackerview.grid);
		} else {
			trackergrid_invalidateinternalcursor(&self->trackerview.grid,
				oldcursor);
			trackergrid_invalidatecursor(&self->trackerview.grid);
		}
	}		
}

void patternview_onshow(PatternView* self)
{	
	/* center splitview when first displayed */
	if (self->aligndisplay && self->state.pv.display > PATTERN_DISPLAYMODE_PIANOROLL) {
		PatternDisplayMode display;

		self->aligndisplay = FALSE;
		display = self->state.pv.display;
		self->state.pv.display = PATTERN_DISPLAYMODE_TRACKER;
		patternview_selectdisplay(self, display);
	}
}

void patternview_ontimer(PatternView* self, uintptr_t timerid)
{		
	if (trackergrid_checkupdate(&self->trackerview.grid)) {
		psy_ui_component_invalidate(&self->trackerview.grid.component);
		psy_ui_component_invalidate(&self->left.linenumbers.component);		
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

void patternview_updateksin(PatternView* self)
{
	psy_ui_Style* style;
	
	style = psy_ui_app_style(psy_ui_app(), STYLE_PATTERNVIEW);
	if (style) {
		psy_ui_style_setcolours(style,
			patternviewskin_fontcolour(self->state.pv.skin, 0, 0),
			patternviewskin_backgroundcolour(self->state.pv.skin, 0, 0));
		psy_ui_component_invalidate(&self->component);
	}		
}

void patternview_numtrackschanged(PatternView* self, psy_audio_Pattern* sender,
	uintptr_t numsongtracks)
{			
	patternview_rebuild(self);
	psy_ui_component_align_full(&self->trackerview.scroller.component);
	psy_ui_component_invalidate(&self->component);
}

void patternview_rebuild(PatternView* self)
{
	trackerheader_build(&self->header);	
	trackergrid_build(&self->trackerview.grid);	
	trackergrid_build(&self->griddefaults);
	trackerheader_build(&self->header);	
	psy_ui_component_invalidate(&self->component);	
}

void patternview_initblockmenu(PatternView* self)
{
	patternblockmenu_init(&self->blockmenu, patternview_base(self),
		self, self->workspace);
	psy_ui_component_setalign(patternblockmenu_base(&self->blockmenu),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(patternblockmenu_base(&self->blockmenu));
}

void patternview_toggleblockmenu(PatternView* self)
{	
	psy_ui_component_togglevisibility(patternblockmenu_base(&self->blockmenu));
	psy_ui_component_togglevisibility(
		psy_ui_button_base(&self->pianoroll.bar.blockmenu));
}

void patternview_toggleinterpolatecurve(PatternView* self,
	psy_ui_Component* sender)
{
	interpolatecurveview_setpattern(&self->interpolatecurveview,
		patternviewstate_pattern(&self->state.pv));
	interpolatecurveview_setselection(&self->interpolatecurveview,
		trackergrid_selection(&self->trackerview.grid));
	psy_ui_component_togglevisibility(interpolatecurveview_base(
		&self->interpolatecurveview));
}

void patternview_toggletransformpattern(PatternView* self, psy_ui_Component*
	sender)
{	
	psy_ui_component_togglevisibility(transformpatternview_base(
		&self->transformpattern));
}

void patternview_toggleswingfill(PatternView* self, psy_ui_Component* sender)
{	
	if (!psy_ui_component_visible(swingfillview_base(&self->swingfillview))) {
		swingfillview_reset(&self->swingfillview, (int)psy_audio_player_bpm(
			workspace_player(self->workspace)));
	}
	psy_ui_component_togglevisibility(swingfillview_base(&self->swingfillview));
}

void patternview_onmousedown(PatternView* self, psy_ui_MouseEvent* ev)
{
	bool blockmenu;

	blockmenu =		
		((ev->button == 2) && ev->event.target == trackergrid_base(&self->trackerview.grid)) ||
		((ev->button == 2) && ev->event.target == patternblockmenu_base(
			&self->blockmenu)) ||
		(ev->event.target == psy_ui_button_base(&self->pianoroll.bar.blockmenu));
	if  (blockmenu) {
		psy_ui_component_setalign(&self->blockmenu.component,
			psy_ui_ALIGN_RIGHT);
		if (ev->event.target == trackergrid_base(&self->trackerview.grid)) {
			self->blockmenu.target = PATTERNVIEWTARGET_TRACKER;
		} else {
			self->blockmenu.target = PATTERNVIEWTARGET_PIANOROLL;
		}
		if (psy_ui_notebook_splitactivated(&self->editnotebook)) {
			if (self->blockmenu.target == PATTERNVIEWTARGET_TRACKER) {
				psy_ui_component_setalign(patternblockmenu_base(
					&self->blockmenu), psy_ui_ALIGN_LEFT);
			}
		}
		patternview_toggleblockmenu(self);
	} else if (ev->button == 2) {
		if (psy_ui_component_visible(interpolatecurveview_base(
				&self->interpolatecurveview))) {
			patternview_toggleinterpolatecurve(self, patternview_base(self));
		}
	}
}

void  patternview_onmouseup(PatternView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1 && self->trackerview.grid.state->pv.selection.valid) {
		interpolatecurveview_setpattern(&self->interpolatecurveview,
			patternviewstate_pattern(&self->state.pv));
		interpolatecurveview_setselection(&self->interpolatecurveview,
			trackergrid_selection(&self->trackerview.grid));
	}
	transformpatternview_setpatternselection(&self->transformpattern,
		trackergrid_selection(&self->trackerview.grid));
}

void patternview_onkeydown(PatternView* self, psy_ui_KeyboardEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (psy_ui_component_visible(patternblockmenu_base(
				&self->blockmenu))) {
			patternview_toggleblockmenu(self);			
		}
		if (psy_ui_component_visible(interpolatecurveview_base(
				&self->interpolatecurveview))) {
			patternview_toggleinterpolatecurve(self, patternview_base(self));			
		}		
		psy_ui_keyboardevent_stop_propagation(ev);
	}
}

void patternview_onpatternimport(PatternView* self)
{
	if (patternviewstate_pattern(&self->state.pv)) {
		psy_ui_OpenDialog dialog;		

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", patternfilter,
			"PSB", dirconfig_songs(psycleconfig_directories(workspace_conf(
				self->workspace))));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(patternviewstate_pattern(&self->state.pv),
				psy_ui_opendialog_path(&dialog), trackerstate_bpl(&self->state));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void patternview_onpatternexport(PatternView* self)
{
	if (patternviewstate_pattern(&self->state.pv)) {
		psy_ui_SaveDialog dialog;		

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", patternfilter,
			"PSB", dirconfig_songs(psycleconfig_directories(workspace_conf(
				self->workspace))));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(patternviewstate_pattern(&self->state.pv),
				psy_ui_savedialog_path(&dialog), trackerstate_bpl(&self->state),
				psy_audio_song_numsongtracks(workspace_song(self->workspace)));
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void patternview_oninterpolatelinear(PatternView* self)
{	
	psy_audio_pattern_blockinterpolatelinear(
		patternviewstate_pattern(&self->state.pv),
		patternview_blockselection(self));			
}

const psy_audio_BlockSelection* patternview_blockselection(const PatternView*
	self)
{
	if (patternblockmenu_target(&self->blockmenu) ==
			PATTERNVIEWTARGET_TRACKER) {
		return trackergrid_selection(&self->trackerview.grid);
	}
	return pianogrid_selection(&self->pianoroll.grid);
}

PatternViewTarget patternview_target(PatternView* self, psy_EventDriver* sender)
{	
	if (psy_ui_component_hasfocus(&self->griddefaults.component)) {
		return PATTERNVIEWTARGET_DEFAULTLINE;
	} else if (psy_ui_component_hasfocus(&self->trackerview.grid.component)) {
		return PATTERNVIEWTARGET_TRACKER;
	} else if (psy_ui_component_hasfocus(&self->pianoroll.grid.component)) {
		return PATTERNVIEWTARGET_PIANOROLL;
	}
	return self->blockmenu.target;	
}

void patternview_oncolresize(PatternView* self, TrackerGrid* sender)
{
	psy_ui_component_align(&self->trackerview.scroller.pane);
	psy_ui_component_align(&self->griddefaultspane);
	psy_ui_component_align(&self->headerpane);
	psy_ui_component_invalidate(&self->component);
}

void patternview_onparametertweak(PatternView* self, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue)
{
	trackergrid_tweak(&self->trackerview.grid, slot, tweak, normvalue);
}

void patternview_updatescrollstep(PatternView* self)
{
	psy_ui_Value step;	
		
	step = psy_ui_value_make_px(trackerstate_defaulttrackwidth(&self->state));
	psy_ui_component_setscrollstep_width(trackergrid_base(&self->trackerview.grid),
		step);			
	psy_ui_component_setscrollstep_width(trackergrid_base(&self->griddefaults),
		step);
	psy_ui_component_setscrollstep_width(trackerheader_base(&self->header),
		step);
}

void patternview_swingfill(PatternView* self, psy_ui_Component* sender)
{
	if (self->trackmodeswingfill || psy_audio_blockselection_valid(
			trackergrid_selection(&self->trackerview.grid))) {		
		psy_audio_pattern_swingfill(patternviewstate_pattern(&self->state.pv),
			trackergrid_selection(&self->trackerview.grid),
			self->trackmodeswingfill, trackerstate_bpl(&self->state),
			swingfillview_values(&self->swingfillview));
	}
}

void patternview_onthemechanged(PatternView* self, PatternViewConfig* sender,
	psy_Property* theme)
{	
	patternviewskin_settheme(&self->skin, theme, dirconfig_skins(
		psycleconfig_directories(workspace_conf(self->workspace))));
	patternview_updateksin(self);
}

void patternview_onzoomboxchanged(PatternView* self, ZoomBox* sender)
{
	patternview_updatezoom(self);
}

void patternview_onappzoom(PatternView* self, psy_ui_AppZoom* sender)
{
	patternview_updatezoom(self);
}

void patternview_updatezoom(PatternView* self)
{
	const psy_ui_Font* font;

	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font newfont;		

		fontinfo = psy_ui_font_fontinfo(font);
		fontinfo.lfHeight = (int)(self->state.defaultlineheight.quantity *
			self->baselfheight * psy_ui_app_zoomrate(psy_ui_app()) *
			zoombox_rate(&self->left.zoombox));
		psy_ui_font_init(&newfont, &fontinfo);
		patternview_setfont(self, &newfont);
		psy_ui_font_dispose(&newfont);
	}
}
