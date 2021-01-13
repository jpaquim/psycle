// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternview.h"
// audio
#include <patternio.h>
#include <songio.h>
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// PatternViewBar
// prototypes
static void patternviewbar_onconfigure(PatternViewBar*, PatternViewConfig*,
	psy_Property*);
static void patternviewbar_onmovecursorwhenpaste(PatternViewBar*,
	psy_ui_Component* sender);
static void patternviewbar_ondefaultline(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_ondisplaysinglepattern(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_onupdatestatus(PatternViewBar*,
	Workspace* sender);
static void patternviewbar_onsequenceselectionchanged(PatternViewBar*,
	psy_audio_SequenceSelection* sender);
static void patternviewbar_onsongchanged(PatternViewBar*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void patternviewbar_updatestatus(PatternViewBar*);
// implementation
void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_component_setdefaultalign(patternviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	patterncursorstepbox_init(&self->cursorstep, &self->component, workspace);
	// Move cursor when paste
	psy_ui_checkbox_init(&self->movecursorwhenpaste, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->movecursorwhenpaste,
		"settingsview.move-cursor-when-paste");	
	psy_signal_connect(&self->movecursorwhenpaste.signal_clicked, self,
		patternviewbar_onmovecursorwhenpaste);
	// Default line
	psy_ui_checkbox_init(&self->defaultentries, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->defaultentries,
		"settingsview.default-line");
	if (patternviewconfig_defaultline(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->defaultentries);
	}
	psy_signal_connect(&self->defaultentries.signal_clicked, self,
		patternviewbar_ondefaultline);
	// Single pattern display mode
	psy_ui_checkbox_init(&self->displaysinglepattern, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->displaysinglepattern,
		"settingsview.displaysinglepattern");
	if (patternviewconfig_issinglepatterndisplay(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	}
	psy_signal_connect(&self->displaysinglepattern.signal_clicked, self,
		patternviewbar_ondisplaysinglepattern);
	psy_ui_label_init(&self->status, patternviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 40);
	psy_signal_connect(&psycleconfig_patview(
			workspace_conf(workspace))->signal_changed, self,
		patternviewbar_onconfigure);
	psy_signal_connect(&workspace->signal_patterncursorchanged, self,
		patternviewbar_onupdatestatus);
	psy_signal_connect(&workspace->sequenceselection.signal_changed,
		self, patternviewbar_onsequenceselectionchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternviewbar_onsongchanged);
	if (patternviewconfig_ismovecursorwhenpaste(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}	
	patternviewbar_updatestatus(self);	
}

void patternviewbar_onmovecursorwhenpaste(PatternViewBar* self, psy_ui_Component* sender)
{
	patternviewconfig_setmovecursorwhenpaste(
		psycleconfig_patview(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->movecursorwhenpaste));
}

void patternviewbar_ondefaultline(PatternViewBar* self, psy_ui_CheckBox* sender)
{	
	patternviewconfig_togglepatdefaultline(
		psycleconfig_patview(workspace_conf(self->workspace)));
}

void patternviewbar_ondisplaysinglepattern(PatternViewBar* self, psy_ui_CheckBox* sender)
{
	patternviewconfig_setdisplaysinglepattern(
		psycleconfig_patview(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->displaysinglepattern));
}

void patternviewbar_onsongchanged(PatternViewBar* self, Workspace* sender,
	int flag, psy_audio_SongFile* songfile)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_onupdatestatus(PatternViewBar* self, Workspace* sender)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_onsequenceselectionchanged(PatternViewBar* self,
	psy_audio_SequenceSelection* sender)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_updatestatus(PatternViewBar* self)
{
	char text[256];
	psy_audio_PatternCursor cursor;
	uintptr_t patternid;	
	psy_audio_SequenceEntry* entry;
	
	cursor = workspace_patterncursor(self->workspace);
	patternid = psy_INDEX_INVALID;
	if (self->workspace->song) {		
		entry = psy_audio_sequence_entry(
			&self->workspace->song->sequence,
			self->workspace->sequenceselection.editposition);
		if (entry) {
			patternid = entry->patternslot;
		}
	}
	if (patternid == psy_INDEX_INVALID) {
		psy_snprintf(text, 256, "Pat --  Ln --  Trk --  Col --:-- Edit");
	} else {
		psy_snprintf(text, 256, "Pat %d  Ln %d  Trk %d  Col %d:%d Edit",
			patternid, cursor.line, cursor.track, cursor.column, cursor.digit);
	}
	psy_ui_label_settext(&self->status, text);
}

void patternviewbar_onconfigure(PatternViewBar* self, PatternViewConfig* config,
	psy_Property* property)
{
	if (patternviewconfig_ismovecursorwhenpaste(config)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}
	if (patternviewconfig_issinglepatterndisplay(config)) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	} else {
		psy_ui_checkbox_disablecheck(&self->displaysinglepattern);
	}
}

// PatternView
// prototypes
static void patternview_inittabbar(PatternView*, psy_ui_Component* tabbarparent);
static void patternview_initblockmenu(PatternView*);
static void patternview_connectplayer(PatternView*, psy_audio_Player*);
static void patternview_ontabbarchange(PatternView*, psy_ui_Component* sender,
	int tabindex);
static void patternview_onsongchanged(PatternView*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void patternview_onsequenceselectionchanged(PatternView*,
	psy_audio_SequenceSelection* sender);
static void patternview_onpatternviewconfigure(PatternView*,
	PatternViewConfig*, psy_Property*);
static void patternview_onmiscconfigure(PatternView*, KeyboardMiscConfig*,
	psy_Property*);
static void patternview_readfont(PatternView*);
static void patternview_updatepgupdowntype(PatternView*);
static void patternview_onpatternpropertiesapply(PatternView*,
	psy_ui_Component* sender);
static void patternview_onfocus(PatternView*, psy_ui_Component* sender);
static void patternview_onmousedown(PatternView*, psy_ui_MouseEvent*);
static void  patternview_onmouseup(PatternView*, psy_ui_MouseEvent*);
static void patternview_onkeydown(PatternView*, psy_ui_KeyEvent*);
static void patternview_oncontextmenu(PatternView*,
	psy_ui_Component* sender);
static void patternview_initbasefontsize(PatternView*);
static void patternview_ondestroy(PatternView* self, psy_ui_Component* sender);
static void patternview_onpreferredsize(PatternView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void patternview_onsequencechanged(PatternView*, psy_audio_Sequence*
	sender);
static void patternview_onpatterncursorchanged(PatternView* self,
	Workspace* sender);
static void patternview_onalign(PatternView*);
static void patternview_computemetrics(PatternView*);
static void patternview_ongridscroll(PatternView*, psy_ui_Component* sender);
static void patternview_onzoomboxchanged(PatternView*, ZoomBox* sender);
static void patternview_setfont(PatternView*, psy_ui_Font* font);
static void patternview_onlpbchanged(PatternView*, psy_audio_Player* sender,
	uintptr_t lpb);
static void patternview_ontimer(PatternView*, uintptr_t timerid);
static void patternview_updateksin(PatternView*);
static void patternview_numtrackschanged(PatternView*, psy_audio_Player*,
	uintptr_t numsongtracks);
static void patternview_oneventdriverinput(PatternView*, psy_EventDriver*);
static PatternViewTarget patternview_target(PatternView*, psy_EventDriver*
	sender);
static void patternview_ontrackercursorchanged(PatternView*, TrackerGridState* sender);
static void patternview_onparametertweak(PatternView*, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue);
static void patternview_oncolresize(PatternView*, TrackerGrid* sender);
static void patternview_updatescrollstep(PatternView*);
static intptr_t patternview_currpgupdownstep(const PatternView*);
static const psy_audio_PatternSelection* patternview_blockselection(const
	PatternView*);
static void patternview_swingfill(PatternView*, psy_ui_Component* sender);
static void patternview_onthemechanged(PatternView*, PatternViewConfig*,
	psy_Property* theme);
static void patternview_displaysinglepattern(PatternView*);
static void patternview_displaysequencepatterns(PatternView*);
static void patternview_updatestates(PatternView*);

// vtable
static psy_ui_ComponentVtable patternview_vtable;
static bool patternview_vtable_initialized = FALSE;

static void patternview_vtable_init(PatternView* self)
{
	if (!patternview_vtable_initialized) {
		patternview_vtable = *(self->component.vtable);
		patternview_vtable.ontimer = (psy_ui_fp_component_ontimer)
			patternview_ontimer;
		patternview_vtable.onalign = (psy_ui_fp_component_onalign)
			patternview_onalign;
		patternview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			patternview_onpreferredsize;
		patternview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			patternview_onmousedown;
		patternview_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			patternview_onmouseup;
		patternview_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			patternview_onkeydown;
		patternview_vtable_initialized = TRUE;
	}
}
// implementation
void patternview_init(PatternView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,	Workspace* workspace)
{
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent);
	patternview_vtable_init(self);
	self->component.vtable = &patternview_vtable;	
	self->workspace = workspace;
	self->showlinenumbers = TRUE;
	self->showdefaultline = TRUE;
	self->pgupdownstepmode = PATTERNCURSOR_STEP_BEAT;
	self->pgupdownstep = 4;
	self->trackmodeswingfill = TRUE;
	patternview_initbasefontsize(self);	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_focus, self, patternview_onfocus);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(&self->notebook),
		psy_ui_BACKGROUND_NONE);
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(&self->notebook));	
	psy_ui_component_setbackgroundmode(&self->editnotebook.component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_notebook_select(&self->editnotebook, 0);
	patternproperties_init(&self->properties, &self->component, NULL, workspace);
	psy_ui_component_setalign(&self->properties.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->properties.component);
	// Skin
	patternviewskin_init(&self->skin);
	patternviewskin_settheme(&self->skin,
		workspace->config.patview.theme,
		dirconfig_skins(&workspace->config.directories));
	psy_signal_connect(&psycleconfig_patview(workspace_conf(self->workspace))->signal_themechanged,
		self, patternview_onthemechanged);
	psy_signal_connect(
		&psycleconfig_patview(workspace_conf(self->workspace))->signal_changed,
		self, patternview_onpatternviewconfigure);
	psy_signal_connect(&psycleconfig_misc(workspace_conf(self->workspace))->signal_changed,
		self, patternview_onmiscconfigure);
	// shared states
	trackconfig_init(&self->trackconfig,
		patternviewconfig_showwideinstcolumn(
			psycleconfig_patview(workspace_conf(workspace))));
	trackerlinestate_init(&self->linestate);
	trackerlinestate_setlpb(&self->linestate, psy_audio_player_lpb(
		workspace_player(self->workspace)));
	self->linestate.skin = &self->skin;
	trackerpianogridstate_init(&self->gridstate, &self->trackconfig);
	self->gridstate.skin = &self->skin;
	psy_signal_connect(&self->gridstate.signal_cursorchanged, self,
		patternview_ontrackercursorchanged);
	// linenumbers
	trackerlinenumberbar_init(&self->left, &self->component, &self->linestate,
		self->workspace);
	psy_ui_component_setalign(&self->left.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->left.zoombox.signal_changed, self,
		patternview_onzoomboxchanged);	
	// header
	trackerheader_init(&self->header, &self->component,
		&self->trackconfig, &self->gridstate, self->workspace);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// pattern default line
	trackergrid_init(&self->griddefaults, &self->component, &self->trackconfig,
		NULL, NULL, TRACKERGRID_EDITMODE_LOCAL, workspace);
	self->griddefaults.defaultgridstate.skin = &self->skin;
	self->griddefaults.defaultlinestate.skin = &self->skin;
	psy_ui_component_setalign(&self->griddefaults.component, psy_ui_ALIGN_TOP);
	self->griddefaults.columnresize = 1;
	trackergrid_setpattern(&self->griddefaults,
		&workspace_player(self->workspace)->patterndefaults);
	psy_signal_connect(&self->griddefaults.signal_colresize, self,
		patternview_oncolresize);
	// TrackerView	
	trackergrid_init(&self->tracker, &self->editnotebook.component,
		&self->trackconfig, &self->gridstate, &self->linestate,
		TRACKERGRID_EDITMODE_SONG, workspace);	
	psy_ui_component_setoverflow(trackergrid_base(&self->tracker), psy_ui_OVERFLOW_SCROLL);
	psy_ui_scroller_init(&self->trackerscroller, &self->tracker.component,
		&self->editnotebook.component);
	psy_ui_component_setalign(&self->trackerscroller.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->tracker.signal_colresize, self,
		patternview_oncolresize);
	// PianoRoll
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, &self->skin,
		workspace);	
	patternview_setpattern(self, psy_audio_patterns_at(&workspace->song->patterns, 0));	
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self,
		patternview_onpatternpropertiesapply);
	// blockmenu
	patternview_initblockmenu(self);
	// TransformPatternView	
	transformpatternview_init(&self->transformpattern, &self->component,
		workspace);
	psy_ui_component_setalign(transformpatternview_base(&self->transformpattern),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(transformpatternview_base(&self->transformpattern));
	// SwingFillView	
	swingfillview_init(&self->swingfillview, &self->component,
		workspace);
	psy_signal_connect(&self->swingfillview.apply.signal_clicked, self,
		patternview_swingfill);
	psy_ui_component_setalign(swingfillview_base(&self->swingfillview),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(swingfillview_base(&self->swingfillview));
	// Interpolate View
	interpolatecurveview_init(&self->interpolatecurveview, &self->component, 0, 0, 0, workspace);
	psy_ui_component_setalign(&self->interpolatecurveview.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->interpolatecurveview.component);
	//psy_signal_connect(&self->interpolatecurveview.signal_cancel, self,
		//trackerview_oninterpolatecurveviewoncancel);
	// Tabbar
	patternview_inittabbar(self, tabbarparent);
	patternview_selectdisplay(self, PATTERN_DISPLAYMODE_TRACKER);
	psy_ui_button_init(&self->contextbutton, &self->sectionbar);
	psy_ui_button_seticon(&self->contextbutton, psy_ui_ICON_MORE);
	psy_ui_component_setalign(psy_ui_button_base(&self->contextbutton), psy_ui_ALIGN_RIGHT);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(-1.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(1));
	psy_ui_component_setmargin(psy_ui_button_base(&self->contextbutton), &margin);
	psy_signal_connect(&self->component.signal_destroy, self,
		patternview_ondestroy);
	psy_signal_connect(&self->contextbutton.signal_clicked,
		self, patternview_oncontextmenu);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);
	psy_signal_connect(&workspace->sequenceselection.signal_changed,
		self, patternview_onsequenceselectionchanged);
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		patternview_onpatterncursorchanged);
	psy_signal_connect(&self->tracker.component.signal_scroll, self,
		patternview_ongridscroll);
	psy_signal_connect(
		&workspace_player(self->workspace)->eventdrivers.signal_input, self,
		patternview_oneventdriverinput);
	psy_signal_connect(&self->workspace->signal_parametertweak, self,
		patternview_onparametertweak);
	if (workspace->song) {
		psy_signal_connect(&workspace->song->sequence.signal_changed,
			self, patternview_onsequencechanged);
	}
	patternview_connectplayer(self, workspace_player(workspace));
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void patternview_ondestroy(PatternView* self, psy_ui_Component* sender)
{
	trackerlinestate_dispose(&self->linestate);
	trackergridstate_dispose(&self->gridstate);
	trackconfig_dispose(&self->trackconfig);
	patternviewskin_dispose(&self->skin);
}

void patternview_connectplayer(PatternView* self, psy_audio_Player* player)
{
	psy_signal_connect(&player->signal_lpbchanged, self,
		patternview_onlpbchanged);
	psy_signal_connect(&player->signal_numsongtrackschanged, self,
		patternview_numtrackschanged);
}

void patternview_initbasefontsize(PatternView* self)
{	
	psy_ui_FontInfo fontinfo;

	fontinfo = psy_ui_font_fontinfo(psy_ui_component_font(&self->component));
	self->baselfheight = fontinfo.lfHeight;	
}

void patternview_inittabbar(PatternView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_component_init_align(&self->sectionbar, tabbarparent,
		psy_ui_ALIGN_LEFT);
	tabbar_init(&self->tabbar, &self->sectionbar);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "Tracker", "Pianoroll", "Split",
		"Vertical", "Horizontal", "Properties", NULL);
	tabbar_tab(&self->tabbar, 2)->mode = TABMODE_LABEL;
	tabbar_tab(&self->tabbar, 5)->istoggle = TRUE;
	tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);
}

void patternview_ontabbarchange(PatternView* self, psy_ui_Component* sender,
	int tabindex)
{	
	if (tabindex == 5) {
		// Properties
		psy_ui_component_togglevisibility(&self->properties.component);
	} else {
		PatternDisplayMode display;

		switch (tabindex) {
		case 0:
			display = PATTERN_DISPLAYMODE_TRACKER;
			break;
		case 1:
			display = PATTERN_DISPLAYMODE_PIANOROLL;
			break;
		case 3:
			display = PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL;
			break;
		case 4:
			display = PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL;
			break;
		default:
			display = PATTERN_DISPLAYMODE_NUM;
			break;
		}
		if (display != PATTERN_DISPLAYMODE_NUM) {
			workspace_selectpatterndisplay(self->workspace, display);
		}
	}
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{	
	self->linestate.trackidx = workspace_sequenceeditposition(
		self->workspace).track;
	interpolatecurveview_setpattern(&self->interpolatecurveview, pattern);
	trackergrid_setpattern(&self->tracker, pattern);	
	pianoroll_setpattern(&self->pianoroll, pattern);
	patternproperties_setpattern(&self->properties, pattern);	
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_invalidate(&self->griddefaults.component);
}

void patternview_onsongchanged(PatternView* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	psy_audio_Pattern* pattern;

	patternview_updatestates(self);
	if (workspace->song) {		
		pattern = psy_audio_sequence_pattern(
			&workspace->song->sequence,
			workspace->sequenceselection.editposition);
		patternview_setpattern(self, pattern);
		pianoroll_setpattern(&self->pianoroll, pattern);		
		psy_signal_connect(&workspace->song->sequence.signal_changed,
			self, patternview_onsequencechanged);
	} else {		
		patternview_setpattern(self, NULL);
		pianoroll_setpattern(&self->pianoroll, NULL);
	}
	psy_ui_component_invalidate(&self->component);
}

void patternview_onsequenceselectionchanged(PatternView* self,
	psy_audio_SequenceSelection* sender)
{		
	psy_audio_SequenceEntry* entry;	
		
	if (self->workspace->song) {
		entry = psy_audio_sequence_entry(
			&self->workspace->song->sequence,
			self->workspace->sequenceselection.editposition);
	} else {
		entry = NULL;
	}
	if (entry) {
		psy_audio_Pattern* pattern;

		pattern = psy_audio_patterns_at(&self->workspace->song->patterns, 
			entry->patternslot);
		if (pattern != self->gridstate.pattern) {
			patternview_setpattern(self, pattern);
			self->linestate.sequenceentryoffset = entry->offset;
			self->pianoroll.grid.sequenceentryoffset = entry->offset;
		}
	} else {
		patternview_setpattern(self, NULL);		
		self->linestate.sequenceentryoffset = 0.f;
		self->pianoroll.grid.sequenceentryoffset = 0.f;
	}
	psy_ui_component_invalidate(&self->component);
}

void patternview_onsequencechanged(PatternView* self, psy_audio_Sequence* sender)
{
	psy_ui_component_updateoverflow(&self->left.linenumbers.component);
	psy_ui_component_updateoverflow(&self->tracker.component);
	psy_ui_component_updateoverflow(&self->pianoroll.grid.component);
	psy_ui_component_invalidate(&self->tracker.component);
	psy_ui_component_invalidate(&self->pianoroll.grid.component);
	psy_ui_component_invalidate(&self->left.linenumbers.component);
}

void patternview_onpatternpropertiesapply(PatternView* self, psy_ui_Component* sender)
{
	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onfocus(PatternView* self, psy_ui_Component* sender)
{
	if (tabbar_selected(&self->tabbar) == 1) { // Pianoroll
		psy_ui_component_setfocus(&self->pianoroll.grid.component);
		return;
	}
	psy_ui_component_setfocus(&self->tracker.component);
}

void patternview_oncontextmenu(PatternView* self, psy_ui_Component* sender)
{
	patternview_toggleblockmenu(self);	
}

void patternview_onpreferredsize(PatternView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	*rv = psy_ui_component_size(&self->component);
}

void patternview_selectdisplay(PatternView* self, PatternDisplayMode display)
{	
	int tabindex;

	switch (display) {
		case PATTERN_DISPLAYMODE_TRACKER:
			tabindex = 0;
			break;
		case PATTERN_DISPLAYMODE_PIANOROLL:
			tabindex = 1;
			break;
		case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL:
			tabindex = 3;
			break;
		case PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL:
			tabindex = 4;
			break;
		default:
			tabindex = 0;
			break;
	}
	if (tabindex < 2) {
		if (psy_ui_notebook_splitactivated(&self->editnotebook)) {
			psy_ui_notebook_full(&self->editnotebook);
		}		
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_notebook_select(&self->editnotebook, tabindex);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, tabindex);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);		
	} else if (tabindex == 3) {
		psy_ui_notebook_select(&self->notebook, 0);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, 0);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {
			if (patternviewconfig_linenumbers(
					psycleconfig_patview(workspace_conf(self->workspace)))) {
				psy_ui_component_show(&self->left.component);
				psy_ui_component_align(&self->component);
			}
		}
		psy_ui_notebook_split(&self->editnotebook, psy_ui_VERTICAL);
	} else if (tabindex == 4) {
		psy_ui_notebook_select(&self->notebook, 0);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, 0);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		if (!psy_ui_notebook_splitactivated(&self->editnotebook)) {
			if (patternviewconfig_linenumbers(
					psycleconfig_patview(workspace_conf(self->workspace)))) {
				psy_ui_component_show_align(&self->left.component);				
			}
		}
		psy_ui_notebook_split(&self->editnotebook, psy_ui_HORIZONTAL);
	} else {
		psy_ui_notebook_select(&self->notebook, 1);
		psy_signal_prevent(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
		tabbar_select(&self->tabbar, 1);
		psy_signal_enable(&self->tabbar.signal_change, self,
			patternview_ontabbarchange);
	}
}

void patternview_onpatternviewconfigure(PatternView* self, PatternViewConfig* config,
	psy_Property* property)
{	
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
	if (self->showdefaultline != patternviewconfig_defaultline(config)) {
		self->showdefaultline = patternviewconfig_defaultline(config);
		if (self->showdefaultline) {			
			trackerlinenumberslabel_showdefaultline(&self->left.linenumberslabel);
			psy_ui_component_show_align(&self->griddefaults.component);			
			psy_ui_component_align(trackerlinenumberbar_base(&self->left));
		} else {
			trackerlinenumberslabel_hidedefaultline(&self->left.linenumberslabel);
			psy_ui_component_hide_align(&self->griddefaults.component);
			psy_ui_component_align(trackerlinenumberbar_base(&self->left));
		}
	}
	self->tracker.wraparound = patternviewconfig_wraparound(config);
	trackergrid_showemptydata(&self->tracker,
		patternviewconfig_drawemptydata(config));
	trackergrid_setcentermode(&self->tracker,
		patternviewconfig_centercursoronscreen(config));
	if (patternviewconfig_showbeatoffset(config)) {
		trackerlinenumberslabel_showbeatoffset(&self->left.linenumberslabel);				
		psy_ui_component_align(&self->component);		
	} else {
		trackerlinenumberslabel_hidebeatoffset(&self->left.linenumberslabel);		
		psy_ui_component_align(&self->component);
	}
	trackconfig_initcolumns(&self->trackconfig,
		patternviewconfig_showwideinstcolumn(config));
	self->tracker.notestabmode = self->griddefaults.notestabmode =
		patternviewconfig_notetabmode(config);
	patternview_updateksin(self);
	if (patternviewconfig_issinglepatterndisplay(config)) {
		patternview_displaysinglepattern(self);		
	} else {
		patternview_displaysequencepatterns(self);
	}
	if (self->tracker.midline) {
		trackergrid_centeroncursor(&self->tracker);
	}
}

void patternview_onmiscconfigure(PatternView* self, KeyboardMiscConfig* config,
	psy_Property* property)
{	
	patternview_readfont(self);		
	patternview_computemetrics(self);	
	if (keyboardmiscconfig_ft2home(config)) {
		trackergrid_enableft2home(&self->tracker);
	} else {
		trackergrid_enableithome(&self->tracker);
	}
	if (keyboardmiscconfig_ft2delete(config)) {
		trackergrid_enableft2delete(&self->tracker);
	} else {
		trackergrid_enableitdelete(&self->tracker);
	}
	if (keyboardmiscconfig_movecursoronestep(config)) {
		trackergrid_enablemovecursoronestep(&self->tracker);
	} else {
		trackergrid_disablemovecursoronestep(&self->tracker);
	}
	if (keyboardmiscconfig_effcursoralwaysdown(config)) {
		trackergrid_enableeffcursoralwaysdown(&self->tracker);
	} else {
		trackergrid_disableffcursoralwaysdown(&self->tracker);
	}	
	self->pgupdownstepmode = (PatternCursorStepMode)
		keyboardmiscconfig_pgupdowntype(config);
	self->pgupdownstep = keyboardmiscconfig_pgupdownstep(config);
	patternview_updatepgupdowntype(self);
}

void patternview_updatepgupdowntype(PatternView* self)
{
	trackergrid_setpgupdownstep(&self->tracker,
		patternview_currpgupdownstep(self));
	pianoroll_setpgupdownstep(&self->pianoroll,
		patternview_currpgupdownstep(self));
}

void patternview_readfont(PatternView* self)
{
	psy_Property* pv;

	pv = psy_property_findsection(&self->workspace->config.config, "visual.patternview");
	if (pv) {
		psy_ui_FontInfo fontinfo;		
		psy_ui_Font font;
		double factor;

		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));		
		factor = (double)fontinfo.lfHeight / self->baselfheight;
		self->linestate.lineheight = psy_ui_value_makeeh(factor);		
		fontinfo.lfHeight = (int)(-16 * factor);
		psy_ui_font_init(&font, &fontinfo);
		patternview_setfont(self, &font);
		psy_ui_font_dispose(&font);		
	}
}

void patternview_onalign(PatternView* self)
{
	psy_ui_RealSize headersize;	

	headersize = psy_ui_component_sizepx(&self->header.component);	
	trackerlinenumberslabel_setheaderheight(&self->left.linenumberslabel,
		headersize.height);
	patternview_computemetrics(self);
}

void patternview_computemetrics(PatternView* self)
{
	psy_ui_Size gridsize;
	psy_ui_TextMetric tm;
	psy_ui_TextMetric gridtm;
	double trackwidth;

	gridsize = psy_ui_component_size(trackergrid_base(&self->tracker));
	tm = psy_ui_component_textmetric(patternview_base(self));
	gridtm = psy_ui_component_textmetric(trackergrid_base(&self->tracker));
	self->gridstate.trackconfig->textwidth = (int)(gridtm.tmAveCharWidth * 1.5) + 2;
	self->linestate.lineheightpx =
		floor(psy_ui_value_px(&self->linestate.lineheight, &tm));
	self->griddefaults.linestate->lineheightpx = self->linestate.lineheightpx;
	trackwidth = psy_max(
		trackergridstate_preferredtrackwidth(&self->gridstate),
		trackergridstate_preferredtrackwidth(&self->gridstate));
	self->trackconfig.patterntrackident = (trackwidth -
		trackergridstate_preferredtrackwidth(&self->gridstate)) / 2;
	if (self->trackconfig.patterntrackident < 0) {
		self->trackconfig.patterntrackident = 0;
	}
	self->trackconfig.headertrackident = 0;
	self->linestate.visilines = (intptr_t)(psy_ui_value_px(&gridsize.height, &tm) /
		self->linestate.lineheightpx);
}

void patternview_ongridscroll(PatternView* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleftpx(&self->tracker.component) !=
		psy_ui_component_scrollleftpx(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->tracker.component));
		psy_ui_component_setscrollleft(&self->griddefaults.component,
			psy_ui_component_scrollleft(&self->tracker.component));
	}
	if (psy_ui_component_scrolltoppx(&self->tracker.component) !=
			psy_ui_component_scrolltoppx(&self->left.linenumbers.component)) {
		psy_ui_component_setscrolltop(&self->left.linenumbers.component,
			psy_ui_component_scrolltop(&self->tracker.component));
	}
}

void patternview_showlinenumbers(PatternView* self)
{		
	psy_ui_component_show_align(&self->left.component);
	self->showlinenumbers = TRUE;	
}

void patternview_hidelinenumbers(PatternView* self)
{	
	psy_ui_component_hide_align(&self->left.component);
	self->showlinenumbers = FALSE;	
}

void patternview_showbeatnumbers(PatternView* self)
{
	self->left.linenumbers.showbeat = TRUE;
	psy_ui_component_align(&self->component);	
}

void patternview_hidebeatnumbers(PatternView* self)
{
	self->left.linenumbers.showbeat = FALSE;
	psy_ui_component_align(&self->component);	
}

void patternview_onzoomboxchanged(PatternView* self, ZoomBox* sender)
{
	psy_ui_Font* font;

	font = psy_ui_component_font(&self->tracker.component);
	if (font) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font newfont;				

		fontinfo = psy_ui_font_fontinfo(font);
		self->linestate.lineheight = psy_ui_mul_value_real(
			self->linestate.defaultlineheight, zoombox_rate(sender));		
		fontinfo.lfHeight = (int)(self->linestate.lineheight.quantity.real * self->baselfheight);
		psy_ui_font_init(&newfont, &fontinfo);
		patternview_setfont(self, &newfont);
		psy_ui_font_dispose(&newfont);
		self->linestate.gridfont = psy_ui_component_font(&self->tracker.component);		
		patternview_computemetrics(self);		
		psy_ui_component_align(&self->left.component);		
		psy_ui_component_align(&self->left.linenumbers.component);
		psy_ui_component_align(&self->component);
		psy_ui_component_updateoverflow(&self->tracker.component);
		psy_ui_component_invalidate(&self->tracker.component);
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_invalidate(&self->left.linenumbers.component);
		psy_ui_component_invalidate(&self->left.linenumberslabel.component);
	}
}

void patternview_setfont(PatternView* self, psy_ui_Font* font)
{	
	psy_ui_TextMetric tm;
	tm = psy_ui_component_textmetric(&self->tracker.component);

	psy_ui_component_setfont(trackergrid_base(&self->griddefaults), font);
	psy_ui_component_setfont(trackergrid_base(&self->tracker), font);
	psy_ui_component_setfont(trackerlinenumbers_base(&self->left.linenumbers), font);
	self->linestate.gridfont = psy_ui_component_font(trackergrid_base(&self->tracker));	
	self->linestate.lineheightpx = psy_ui_value_px(
		&self->linestate.lineheight, &tm);
	patternview_computemetrics(self);
	patternview_updatescrollstep(self);
	psy_ui_component_align(&self->component);
}

void patternview_onpatterncursorchanged(PatternView* self, Workspace* sender)
{
	psy_audio_PatternCursor oldcursor;

	oldcursor = self->gridstate.cursor;	
	self->gridstate.cursor = workspace_patterncursor(sender);
	if (!psy_audio_patterncursor_equal(&self->gridstate.cursor, &oldcursor)) {
		if (self->gridstate.pattern && psy_audio_player_playing(&sender->player) && workspace_followingsong(sender)) {
			bool scrolldown;

			scrolldown = self->linestate.lastplayposition <
				psy_audio_player_position(workspace_player(self->workspace));
			trackergrid_invalidateline(&self->tracker, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->left.linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = psy_audio_player_position(workspace_player(self->workspace));
			trackergrid_invalidateline(&self->tracker, self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->left.linenumbers,
				self->linestate.lastplayposition);
			if (self->linestate.lastplayposition >= self->linestate.sequenceentryoffset &&
				self->linestate.lastplayposition < self->linestate.sequenceentryoffset +
				trackergridstate_pattern(&self->gridstate)->length) {
				if (scrolldown != FALSE) {
					trackergrid_scrolldown(&self->tracker, self->gridstate.cursor);
				} else {
					trackergrid_scrollup(&self->tracker, self->gridstate.cursor);
				}
			}
			trackergrid_storecursor(&self->tracker);
		} else if (self->tracker.midline) {
			trackergrid_centeroncursor(&self->tracker);
		} else {
			trackergrid_invalidatecursor(&self->tracker);
		}
	}	
	trackergrid_invalidateinternalcursor(&self->tracker,
		oldcursor);
}

void patternview_onlpbchanged(PatternView* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	trackerlinestate_setlpb(&self->linestate, lpb);	
	patternview_updatepgupdowntype(self);
	psy_ui_component_updateoverflow(trackergrid_base(&self->tracker));
	psy_ui_component_invalidate(trackergrid_base(&self->tracker));
	psy_ui_component_invalidate(trackerlinenumbers_base(
		&self->left.linenumbers));
}

void patternview_ontimer(PatternView* self, uintptr_t timerid)
{
	if (trackergridstate_pattern(&self->gridstate)) {
		if (psy_audio_player_playing(workspace_player(self->workspace))) {
			if (!workspace_followingsong(self->workspace)) {
				trackergrid_invalidateline(&self->tracker, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->left.linenumbers,
					self->linestate.lastplayposition);
				self->linestate.lastplayposition =
					psy_audio_player_position(workspace_player(self->workspace));
				trackergrid_invalidateline(&self->tracker, self->linestate.lastplayposition);
				trackerlinenumbers_invalidateline(&self->left.linenumbers,
					self->linestate.lastplayposition);
			}
			psy_ui_component_invalidate(&self->header.component);
		} else if (self->linestate.lastplayposition != -1) {
			trackergrid_invalidateline(&self->tracker,
				self->linestate.lastplayposition);
			trackerlinenumbers_invalidateline(&self->left.linenumbers,
				self->linestate.lastplayposition);
			self->linestate.lastplayposition = -1;
		}
		if (trackergridstate_pattern(&self->gridstate) &&
				trackergridstate_pattern(&self->gridstate)->opcount != self->tracker.opcount &&
			self->tracker.syncpattern) {
			psy_ui_component_invalidate(&self->tracker.component);
			psy_ui_component_invalidate(&self->left.linenumbers.component);
		}
		self->tracker.opcount = (trackergridstate_pattern(&self->gridstate))
			? trackergridstate_pattern(&self->gridstate)->opcount
			: 0;
	}
}

void patternview_updateksin(PatternView* self)
{
	psy_ui_component_setbackgroundcolour(&self->left.linenumbers.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->blockmenu.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->transformpattern.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->interpolatecurveview.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setcolour(&self->interpolatecurveview.component,
		patternviewskin_fontcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerscroller.hscroll.sliderpane.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setcolour(&self->trackerscroller.hscroll.sliderpane.component,
		patternviewskin_rowcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setbackgroundcolour(&self->trackerscroller.vscroll.sliderpane.component,
		patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_setcolour(&self->trackerscroller.vscroll.sliderpane.component,
		patternviewskin_rowcolour(self->gridstate.skin, 0, 0));
	psy_ui_component_invalidate(&self->component);
}

void patternview_numtrackschanged(PatternView* self, psy_audio_Player* player,
	uintptr_t numsongtracks)
{	
	trackergridstate_setnumsongtracks(self->griddefaults.gridstate,
		numsongtracks);
	psy_ui_component_updateoverflow(trackergrid_base(&self->tracker));
	psy_ui_component_invalidate(trackergrid_base(&self->tracker));
	psy_ui_component_invalidate(trackergrid_base(&self->griddefaults));
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


void patternview_toggleinterpolatecurve(PatternView* self, psy_ui_Component* sender)
{
	interpolatecurveview_setpattern(&self->interpolatecurveview,
		trackergridstate_pattern(&self->gridstate));
	interpolatecurveview_setselection(&self->interpolatecurveview,
		trackergrid_selection(&self->tracker));
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
		((ev->button == 2) && ev->target == trackergrid_base(&self->tracker)) ||
		((ev->button == 2) && ev->target == patternblockmenu_base(&self->blockmenu)) ||
		(ev->target == psy_ui_button_base(&self->pianoroll.bar.blockmenu));
	if  (blockmenu) {
		psy_ui_component_setalign(&self->blockmenu.component, psy_ui_ALIGN_RIGHT);
		if (ev->target == trackergrid_base(&self->tracker)) {
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
	if (ev->button == 1 && self->tracker.selection.valid) {
		interpolatecurveview_setpattern(&self->interpolatecurveview,
			trackergridstate_pattern(&self->gridstate));
		interpolatecurveview_setselection(&self->interpolatecurveview,
			trackergrid_selection(&self->tracker));
	}
	transformpatternview_setpatternselection(&self->transformpattern,
		trackergrid_selection(&self->tracker));
}

void patternview_onkeydown(PatternView* self, psy_ui_KeyEvent* ev)
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
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void patternview_onpatternimport(PatternView* self)
{
	if (trackergridstate_pattern(&self->gridstate)) {
		psy_ui_OpenDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.psb";

		psy_ui_opendialog_init_all(&dialog, 0, "Import Pattern", filter, "PSB",
			dirconfig_songs(&self->workspace->config.directories));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_patternio_load(trackergridstate_pattern(&self->gridstate),
				psy_ui_opendialog_path(&dialog),
				psy_audio_player_bpl(workspace_player(self->workspace)));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void patternview_onpatternexport(PatternView* self)
{
	if (trackergridstate_pattern(&self->gridstate)) {
		psy_ui_SaveDialog dialog;
		static char filter[] = "Pattern (*.psb)" "|*.PSB";

		psy_ui_savedialog_init_all(&dialog, 0, "Export Pattern", filter, "PSB",
			dirconfig_songs(psycleconfig_directories(workspace_conf(
				self->workspace))));
		if (psy_ui_savedialog_execute(&dialog)) {
			psy_audio_patternio_save(trackergridstate_pattern(&self->gridstate),
				psy_ui_savedialog_path(&dialog),
				psy_audio_player_bpl(workspace_player(self->workspace)),
				psy_audio_player_numsongtracks(workspace_player(self->workspace)));
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void patternview_oninterpolatelinear(PatternView* self)
{	
	psy_audio_pattern_blockinterpolatelinear(
		trackergridstate_pattern(&self->gridstate),
		patternview_blockselection(self),
		psy_audio_player_bpl(workspace_player(self->workspace)));
}

const psy_audio_PatternSelection* patternview_blockselection(const PatternView*
	self)
{
	if (patternblockmenu_target(&self->blockmenu) ==
			PATTERNVIEWTARGET_TRACKER) {
		return trackergrid_selection(&self->tracker);
	}
	return pianogrid_selection(&self->pianoroll.grid);
}

void patternview_oneventdriverinput(PatternView* self, psy_EventDriver* sender)
{
	if (workspace_currview(self->workspace) == VIEW_ID_PATTERNVIEW) {
		psy_EventDriverCmd cmd;
		PatternViewTarget target;

		target = patternview_target(self, sender);
		if (target == PATTERNVIEWTARGET_TRACKER) {
			cmd = psy_eventdriver_getcmd(sender, "tracker");
			if (cmd.id != -1) {
				if (psy_ui_component_hasfocus(trackergrid_base(
						&self->tracker))) {
					trackergrid_handlecommand(&self->tracker, cmd.id);
				}				
			}
		} else if (target == PATTERNVIEWTARGET_DEFAULTLINE) {
			cmd = psy_eventdriver_getcmd(sender, "tracker");
			if (cmd.id != -1 && (psy_ui_component_hasfocus(trackergrid_base(
					&self->griddefaults)))) {
				trackergrid_handlecommand(&self->griddefaults, cmd.id);
			}
		} else if (target == PATTERNVIEWTARGET_PIANOROLL) {
			cmd = psy_eventdriver_getcmd(sender, "pianoroll");
			if (cmd.id == CMD_COLUMNNEXT || cmd.id == CMD_COLUMNPREV) {
				trackergrid_handlecommand(&self->tracker, cmd.id);
			} else {
				pianoroll_handlecommand(&self->pianoroll, cmd.id);
			}
		}
	}
}

PatternViewTarget patternview_target(PatternView* self, psy_EventDriver* sender)
{
	PatternViewTarget rv;

	if (psy_eventdriver_target(sender)) {
		if (strcmp(psy_eventdriver_target(sender), "tracker") == 0) {
			rv = PATTERNVIEWTARGET_TRACKER;
		} else {
			rv = PATTERNVIEWTARGET_PIANOROLL;
		}
	} else if (psy_ui_component_hasfocus(&self->griddefaults.component)) {
		rv = PATTERNVIEWTARGET_DEFAULTLINE;
	} else if (psy_ui_component_hasfocus(&self->tracker.component)) {
		rv = PATTERNVIEWTARGET_TRACKER;
	} else if (psy_ui_component_hasfocus(&self->pianoroll.grid.component)) {
		rv = PATTERNVIEWTARGET_PIANOROLL;
	} else {
		rv = self->blockmenu.target;
	}
	return rv;
}

void patternview_oncolresize(PatternView* self, TrackerGrid* sender)
{
	if (sender == &self->griddefaults) {
		psy_ui_component_invalidate(trackergrid_base(&self->tracker));
	} else {
		psy_ui_component_invalidate(trackergrid_base(&self->griddefaults));
	}
	psy_ui_component_invalidate(trackerheader_base(&self->header));
}

void patternview_ontrackercursorchanged(PatternView* self,
	TrackerGridState* sender)
{
	workspace_setpatterncursor(self->workspace, sender->cursor);		
}

void patternview_onparametertweak(PatternView* self, Workspace* sender,
	int slot, uintptr_t tweak, float normvalue)
{
	trackergrid_tweak(&self->tracker, slot, tweak, normvalue);
}

void patternview_updatescrollstep(PatternView* self)
{
	double scrollstepx;
	psy_ui_Value scrollleft;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(trackergrid_base(&self->tracker));
	scrollleft = psy_ui_component_scrollleft(trackergrid_base(&self->tracker));
	scrollstepx = trackergridstate_trackwidth(&self->gridstate,
			trackergridstate_pxtotrack(&self->gridstate,
				psy_ui_value_px(&scrollleft, &tm),
				trackergridstate_numsongtracks(&self->gridstate)));
	psy_ui_component_setscrollstep(trackergrid_base(&self->tracker),
		psy_ui_value_makepx(scrollstepx),
		psy_ui_value_makepx(trackerlinestate_lineheight(&self->linestate)));
	psy_ui_component_setscrollstepx(trackergrid_base(&self->griddefaults),
		psy_ui_value_makepx(scrollstepx));
	psy_ui_component_setscrollstepx(trackerheader_base(&self->header),
		psy_ui_value_makepx(scrollstepx));
	psy_ui_component_setscrollstepy(&self->left.linenumbers.component,
		psy_ui_value_makepx(trackerlinestate_lineheight(&self->linestate)));
}

intptr_t patternview_currpgupdownstep(const PatternView* self)
{
	if (self->pgupdownstepmode == PATTERNCURSOR_STEP_BEAT) {
		return psy_audio_player_lpb(workspace_player(self->workspace));
	} else if (self->pgupdownstepmode == PATTERNCURSOR_STEP_4BEAT) {
		return psy_audio_player_lpb(workspace_player(self->workspace)) * 4;
	}
	return self->pgupdownstep;
}

void patternview_swingfill(PatternView* self, psy_ui_Component* sender)
{
	if (self->trackmodeswingfill || psy_audio_patternselection_valid(
			trackergrid_selection(&self->tracker))) {
		int tempo;
		int width;
		float variance;
		float phase;
		bool offset;

		swingfillview_values(&self->swingfillview, &tempo, &width, &variance,
			&phase, &offset);
		psy_audio_pattern_swingfill(trackergridstate_pattern(&self->gridstate),
			trackergrid_selection(&self->tracker), self->trackmodeswingfill,
			trackerlinestate_bpl(&self->linestate),
			tempo, width, variance, phase, offset);
	}
}

void patternview_onthemechanged(PatternView* self, PatternViewConfig* sender,
	psy_Property* theme)
{	
		patternviewskin_settheme(&self->skin, theme,
			dirconfig_skins(psycleconfig_directories(
				workspace_conf(self->workspace))));
		patternview_updateksin(self);
}

void patternview_displaysinglepattern(PatternView* self)
{
	self->linestate.singlemode = TRUE;
	self->gridstate.singlemode = TRUE;
	patternview_updatestates(self);
	psy_ui_component_align(&self->component);
	psy_ui_component_updateoverflow(&self->tracker.component);
}

void patternview_displaysequencepatterns(PatternView* self)
{
	self->linestate.singlemode = FALSE;
	self->gridstate.singlemode = FALSE;
	patternview_updatestates(self);
	psy_ui_component_align(&self->component);
	psy_ui_component_updateoverflow(&self->tracker.component);
}

void patternview_updatestates(PatternView* self)
{
	if (self->workspace->song) {
		psy_audio_Pattern* pattern;

		trackergridstate_setsequence(&self->gridstate,
			&self->workspace->song->sequence);
		self->pianoroll.grid.sequenceentryoffset = 0.f;
		trackerlinestate_setsequence(&self->linestate,
			&self->workspace->song->sequence);
		self->linestate.sequenceentryoffset = 0.f;
		pattern = psy_audio_sequence_pattern(
			&self->workspace->song->sequence,
			self->workspace->sequenceselection.editposition);
		trackergridstate_setpattern(&self->gridstate,
			pattern);
		trackerlinestate_setpattern(&self->linestate,
			pattern);		
	} else {
		trackergridstate_setsequence(&self->gridstate, NULL);
		self->pianoroll.grid.sequenceentryoffset = 0.f;
		trackerlinestate_setsequence(&self->linestate, NULL);
		self->linestate.sequenceentryoffset = 0.f;
		trackergridstate_setpattern(&self->gridstate, NULL);
		trackerlinestate_setpattern(&self->linestate, NULL);
	}
}