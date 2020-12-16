// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
		"settingsview.patternview");
	self->patternview = pvc;
	psy_property_settext(
		psy_property_append_font(pvc, "font", PSYCLE_DEFAULT_FONT),
		"settingsview.font");
	psy_property_settext(
		psy_property_append_bool(pvc, "drawemptydata", FALSE),
		"settingsview.draw-empty-data");
	psy_property_settext(
		psy_property_append_bool(pvc, "griddefaults", TRUE),
		"settingsview.default-entries");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumbers", TRUE),
		"settingsview.line-numbers");
	psy_property_settext(
		psy_property_append_bool(pvc, "beatoffset", FALSE),
		"settingsview.beat-offset");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumberscursor", TRUE),
		"settingsview.line-numbers-cursor");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumbersinhex", FALSE),
		"settingsview.line-numbers-in-hex");
	psy_property_settext(
		psy_property_append_bool(pvc, "wideinstcolumn", FALSE),
		"settingsview.wide-instrument-column");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(pvc, "trackscopes", TRUE),
		"settingsview.pattern-track-scopes"),
		PROPERTY_ID_TRACKSCOPES);
	psy_property_settext(
		psy_property_append_bool(pvc, "wraparound", TRUE),
		"settingsview.wrap-around");
	psy_property_settext(
		psy_property_append_bool(pvc, "centercursoronscreen", FALSE),
		"settingsview.center-cursor-on-screen");
	psy_property_settext(
		psy_property_append_int(pvc, "beatsperbar", 4, 1, 16),
		"settingsview.bar-highlighting");
	psy_property_settext(
		psy_property_append_bool(pvc, "notetab", TRUE),
		"settingsview.a4-440hz");
	psy_property_settext(
		psy_property_append_bool(pvc, "movecursorwhenpaste", TRUE),
		"settingsview.move-cursor-when-paste");
	// pattern display choice
	choice = psy_property_setid(psy_property_settext(
		psy_property_append_choice(pvc,
			"patterndisplay", 0),
		"settingsview.patterndisplay"),
		PROPERTY_ID_PATTERNDISPLAY);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "tracker",
			0, 0, 0),
		"settingsview.tracker"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "piano",
			0, 0, 0),
		"settingsview.piano"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_PIANOROLL);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "splitvertical",
			0, 0, 0),
		"settingsview.splitvertical"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL);
	psy_property_setid(psy_property_settext(
		psy_property_append_int(choice, "splithorizontal",
			0, 0, 0),
		"settingsview.splithorizontal"),
		PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL);
	patternviewconfig_maketheme(self, pvc);
}

void patternviewconfig_maketheme(PatternViewConfig* self, psy_Property* parent)
{
	assert(self);

	self->theme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"settingsview.theme");
	psy_property_append_string(self->theme,
		"pattern_fontface", "Tahoma");
	psy_property_append_int(self->theme, "pattern_font_point", 0x00000050, 0, 0),
		psy_property_append_int(self->theme, "pattern_font_flags", 0x00000001, 0, 0);
	psy_property_settext(
		psy_property_append_int(self->theme,
			"pattern_font_x", 0x00000009, 0, 0),
		"Point X");
	psy_property_settext(
		psy_property_append_int(self->theme,
			"pattern_font_y", 0x0000000B, 0, 0),
		"Point Y");
	psy_property_append_string(self->theme, "pattern_header_skin", "");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_separator", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Separator Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_separator2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Separator Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_background", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"BackGnd Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_background2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"BackGnd Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_font", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_font2", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontCur", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Cur Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontCur2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Cur Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontSel", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Sel Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontSel2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Sel Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontPlay", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Play Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_fontPlay2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Play Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Row Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row2", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Row Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_rowbeat", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Beat Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_rowbeat2", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Beat Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row4beat", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Bar Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_row4beat2", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Bar Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_selection", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Selection Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_selection2", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Selection Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_playbar", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Playbar Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_playbar2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Playbar Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_cursor", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Cursor Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_cursor2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Cursor Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_midline", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Midline Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"pvc_midline", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Midline Right");
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
