// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternviewconfig.h"

static void patternviewconfig_makeview(PatternViewConfig*, psy_Property*
	parent);
static void patternviewconfig_maketheme(PatternViewConfig*, psy_Property*
	parent);

void patternviewconfig_init(PatternViewConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	patternviewconfig_makeview(self, parent);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_themechanged);
}

void patternviewconfig_dispose(PatternViewConfig* self)
{
	assert(self);
		
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_themechanged);
}

void patternviewconfig_makeview(PatternViewConfig* self, psy_Property* parent)
{
	psy_Property* pvc;
	psy_Property* choice;

	assert(self);

	pvc = psy_property_append_section(parent, "patternview");
	psy_property_settext(pvc,
		"settingsview.pv.patternview");
	self->patternview = pvc;
	psy_property_settext(
		psy_property_append_font(pvc, "font", PSYCLE_DEFAULT_PATTERN_FONT),
		"settingsview.pv.font");
	psy_property_settext(
		psy_property_append_bool(pvc, "smoothscroll", FALSE),
		"settingsview.pv.smoothscroll");
	psy_property_settext(
		psy_property_append_bool(pvc, "drawemptydata", FALSE),
		"settingsview.pv.draw-empty-data");
	psy_property_settext(
		psy_property_append_bool(pvc, "griddefaults", TRUE),
		"settingsview.pv.default-entries");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumbers", TRUE),
		"settingsview.pv.line-numbers");
	psy_property_settext(
		psy_property_append_bool(pvc, "beatoffset", FALSE),
		"settingsview.pv.beat-offset");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumberscursor", TRUE),
		"settingsview.pv.line-numbers-cursor");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumbersinhex", FALSE),
		"settingsview.pv.line-numbers-in-hex");
	psy_property_settext(
		psy_property_append_bool(pvc, "wideinstcolumn", FALSE),
		"settingsview.pv.wide-instrument-column");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(pvc, "trackscopes", TRUE),
		"settingsview.pv.pattern-track-scopes"),
		PROPERTY_ID_TRACKSCOPES);
	psy_property_settext(
		psy_property_append_bool(pvc, "wraparound", TRUE),
		"settingsview.pv.wrap-around");
	psy_property_settext(
		psy_property_append_bool(pvc, "centercursoronscreen", FALSE),
		"settingsview.pv.center-cursor-on-screen");
	psy_property_settext(
		psy_property_append_int(pvc, "beatsperbar", 4, 1, 16),
		"settingsview.pv.bar-highlighting");
	psy_property_settext(
		psy_property_append_bool(pvc, "notetab", TRUE),
		"settingsview.pv.a4-440hz");
	psy_property_settext(
		psy_property_append_bool(pvc, "movecursorwhenpaste", TRUE),
		"settingsview.pv.move-cursor-when-paste");
	psy_property_settext(
		psy_property_append_bool(pvc, "displaysinglepattern", TRUE),
		"settingsview.pv.displaysinglepattern");
	// useheaderbitmap
	// default set to false, because the bitmap skins dont fit to recent
	// resolutions
	psy_property_settext(
		psy_property_append_bool(pvc, "useheaderbitmap", FALSE),
		"settingsview.pv.useheaderbitmap");
	// pattern display choice
	choice = psy_property_setid(psy_property_settext(
		psy_property_append_choice(pvc,
			"patterndisplay", 0),
		"settingsview.pv.patterndisplay"),
		PROPERTY_ID_PATTERNDISPLAY);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "tracker",
			0, 0, 0),
		"settingsview.pv.tracker"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "piano",
			0, 0, 0),
		"settingsview.pv.piano"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_PIANOROLL);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "splitvertical",
			0, 0, 0),
		"settingsview.pv.splitvertical"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "splithorizontal",
			0, 0, 0),
		"settingsview.pv.splithorizontal"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL);
	patternviewconfig_maketheme(self, pvc);
}

void patternviewconfig_maketheme(PatternViewConfig* self, psy_Property* parent)
{
	assert(self);

	self->theme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"settingsview.pv.theme.theme");
	psy_property_settext(
		psy_property_append_str(self->theme,
			"pattern_fontface", "Tahoma"),
		"settingsview.pv.theme.fontface");
	psy_property_settext(
		psy_property_append_int(self->theme,
			"pattern_font_point", 0x00000050, 0, 0),
			"settingsview.pv.theme.fontpoint");
	psy_property_settext(
		psy_property_append_int(self->theme,
			"pattern_font_flags", 0x00000001, 0, 0),
			"settingsview.pv.theme.fontflags");
	psy_property_settext(
		psy_property_append_int(self->theme,
			"pattern_font_x", 0x00000009, 0, 0),
		"settingsview.pv.theme.font_x");
	psy_property_settext(
		psy_property_append_int(self->theme,
			"pattern_font_y", 0x0000000B, 0, 0),
		"settingsview.pv.theme.font_y");
	psy_property_settext(
		psy_property_append_str(self->theme, "pattern_header_skin", ""),
		"settingsview.pv.theme.headerskin");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_separator", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.separator");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_separator2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.separator2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_background", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_background2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.background2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_font", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_font2", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.font2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontCur", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontcur");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontCur2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontcur2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontSel", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontsel");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontSel2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontsel2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontPlay", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontplay");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontPlay2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontplay2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row2", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_rowbeat", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.rowbeat");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_rowbeat2", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.rowbeat2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row4beat", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row4beat");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row4beat2", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row4beat2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_selection", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.selection");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_selection2", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.selection2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_playbar", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.playbar");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_playbar2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.playbar2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_cursor", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.cursor");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_cursor2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.cursor2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_midline", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.midline");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_midline2", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.midline2");
}

void patternviewconfig_resettheme(PatternViewConfig* self)
{
	assert(self);

	if (self->theme) {
		psy_property_remove(self->patternview, self->theme);
	}
	patternviewconfig_maketheme(self, self->patternview);
	psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
}

void patternviewconfig_settheme(PatternViewConfig* self, psy_Property* skin)
{
	assert(self);

	if (self->theme) {
		psy_property_sync(self->theme, skin);
		psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
	}
}

bool patternviewconfig_hasthemeproperty(const PatternViewConfig* self,
	psy_Property* property)
{
	return (self->theme && psy_property_insection(property, self->theme));
}

bool patternviewconfig_hasproperty(const PatternViewConfig* self,
	psy_Property* property)
{
	assert(self &&  self->patternview);

	return (psy_property_insection(property, self->patternview));
}

// getter
bool patternviewconfig_linenumbers(const PatternViewConfig* self)
{
	assert(self);
	
	return psy_property_at_bool(self->patternview, "linenumbers", TRUE);
}

bool patternviewconfig_linenumberscursor(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "linenumberscursor", TRUE);
}

bool patternviewconfig_linenumbersinhex(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "linenumbersinhex", TRUE);
}

bool patternviewconfig_defaultline(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "griddefaults", TRUE);
}

bool patternviewconfig_wraparound(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "wraparound", TRUE);
}

bool patternviewconfig_drawemptydata(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "drawemptydata", TRUE);
}

bool patternviewconfig_centercursoronscreen(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "centercursoronscreen", TRUE);
}

bool patternviewconfig_showbeatoffset(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "beatoffset", TRUE);
}

bool patternviewconfig_showwideinstcolumn(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "wideinstcolumn", TRUE);
}

psy_dsp_NotesTabMode patternviewconfig_notetabmode(const PatternViewConfig* self)
{
	assert(self);

	return (psy_property_at_bool(self->patternview, "notetab",
			psy_dsp_NOTESTAB_A440))
		? psy_dsp_NOTESTAB_A440
		: psy_dsp_NOTESTAB_A220;
}

bool patternviewconfig_ismovecursorwhenpaste(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "movecursorwhenpaste",
		TRUE);
}

void patternviewconfig_setmovecursorwhenpaste(PatternViewConfig* self, bool on)
{
	assert(self);

	psy_property_set_bool(self->patternview, "movecursorwhenpaste", on);
}

bool patternviewconfig_issmoothscrolling(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "smoothscroll",
		TRUE);
}

void patternviewconfig_setsmoothscrolling(PatternViewConfig* self, bool on)
{
	assert(self);

	psy_property_set_bool(self->patternview, "smoothscroll", on);
}


void patternviewconfig_setdisplaysinglepattern(PatternViewConfig* self, bool on)
{
	psy_Property* property;

	assert(self);

	property = psy_property_set_bool(self->patternview, "displaysinglepattern",
		on);
	if (property) {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}

bool patternviewconfig_issinglepatterndisplay(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "displaysinglepattern", TRUE);
}

bool patternviewconfig_useheaderbitmap(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "useheaderbitmap", TRUE);
}



bool patternviewconfig_showtrackscopes(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "trackscopes", TRUE);
}
// events
bool patternviewconfig_onchanged(PatternViewConfig* self, psy_Property* property)
{
	assert(self);

	if (patternviewconfig_hasthemeproperty(self, property)) {
		patternviewconfig_onthemechanged(self, property);		
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
	return TRUE;
}

bool patternviewconfig_onthemechanged(PatternViewConfig* self, psy_Property* property)
{
	assert(self);

	psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
	return TRUE;
}

void patternviewconfig_togglepatdefaultline(PatternViewConfig* self)
{			
	psy_Property* property;

	assert(self);

	property = psy_property_at(self->patternview, "griddefaults",
		PSY_PROPERTY_TYPE_NONE);
	if (property) {
		psy_property_setitem_bool(property, !psy_property_item_bool(property));
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}

int patternviewconfig_patterndisplay(const PatternViewConfig* self)
{
	psy_Property* property;

	property = psy_property_at(self->patternview, "patterndisplay",
		PSY_PROPERTY_TYPE_CHOICE);
	if (property) {
		return (int)psy_property_item_int(property);
	}
	return PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER;		
}
