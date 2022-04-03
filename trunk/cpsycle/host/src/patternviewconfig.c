/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewconfig.h"
/* host */
#include "dirconfig.h"
#include "skinio.h"
#include "styles.h"
#include "resources/resource.h"
/* file */
#include <dir.h>
/* ui */
#include <uiapp.h>
#include <uistyle.h>
/* platform */
#include "../../detail/portable.h"

#define PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN "Psycle Default (internal)"

/* prototypes*/
static void patternviewconfig_makeview(PatternViewConfig*, psy_Property*
	parent);
static void patternviewconfig_maketheme(PatternViewConfig*, psy_Property*
	parent);
static void patternviewconfig_setsource(PatternViewConfig*,
	psy_ui_RealRectangle*, intptr_t vals[4]);
static void patternviewconfig_setdest(PatternViewConfig*, psy_ui_RealPoint*,
	intptr_t vals[4], uintptr_t num);
static void patternviewconfig_setstylecoords(PatternViewConfig*,
	uintptr_t styleid, uintptr_t select_styleid, psy_ui_RealRectangle src,
	psy_ui_RealPoint dst);

/* implementation */
void patternviewconfig_init(PatternViewConfig* self, psy_Property* parent,
	const char* skindir)
{
	assert(self && parent);

	self->parent = parent;
	self->skindir = psy_strdup(skindir);	
	self->dirconfig = NULL;
	self->has_classic_header = TRUE;
	patternviewconfig_makeview(self, parent);	
	psy_signal_init(&self->signal_changed);
}

void patternviewconfig_dispose(PatternViewConfig* self)
{
	assert(self);
		
	psy_signal_dispose(&self->signal_changed);	
	free(self->skindir);
	self->skindir = NULL;
}

void patternviewconfig_setdirectories(PatternViewConfig* self,
	DirConfig* dirconfig)
{
	self->dirconfig = dirconfig;	
	patternviewconfig_update_header_skins(self);
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
	/*
	** useheaderbitmap
	** default set to false, because the bitmap skins dont fit to recent
	** resolutions
	*/
	psy_property_settext(
		psy_property_append_bool(pvc, "useheaderbitmap", FALSE),
		"settingsview.pv.useheaderbitmap");
	/* pattern display choice */
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
	self->headerskins = psy_property_setid(
		psy_property_sethint(psy_property_settext(
			psy_property_append_choice(self->theme, "skins", 0),
			"Skin"), PSY_PROPERTY_HINT_COMBO),
		PROPERTY_ID_PATTERN_SKIN);
	patternviewconfig_update_header_skins(self);
}

void patternviewconfig_update_header_skins(PatternViewConfig* self)
{
	psy_property_clear(self->headerskins);
	psy_property_append_str(self->headerskins,
		PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN,
		PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN);
	if (self->dirconfig) {
		skin_locate_pattern_skins(self->headerskins,
			dirconfig_skins(self->dirconfig));
	}
}

const char* patternviewconfig_header_skin_name(PatternViewConfig* self)
{
	psy_Property* choice;

	assert(self);

	choice = psy_property_at_choice(self->headerskins);
	if (choice && psy_strlen(psy_property_item_str(choice)) > 0) {
		return psy_property_key(choice);
	}
	return "";
}


void patternviewconfig_resettheme(PatternViewConfig* self)
{
	assert(self);

	if (self->theme) {
		psy_property_remove(self->patternview, self->theme);
	}
	patternviewconfig_maketheme(self, self->patternview);
	psy_property_setitem_int(self->headerskins, 0);
	init_patternview_styles(&psy_ui_appdefaults()->styles);	
}

void patternviewconfig_settheme(PatternViewConfig* self, psy_Property* theme)
{
	assert(self);

	if (self->theme) {
		psy_Property* header_skin;

		header_skin = psy_property_at(self->headerskins,
			psy_property_at_str(theme, "pattern_header_skin", ""),
			PSY_PROPERTY_TYPE_STRING);
		if (header_skin) {
			psy_property_setitem_int(self->headerskins,
				psy_property_index(header_skin));
		} else {
			psy_property_setitem_int(self->headerskins, 0);
		}
		psy_property_sync(self->theme, theme);
		patternviewconfig_write_styles(self);
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

/* getter */
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

bool patternviewconfig_center_cursor_on_screen(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "centercursoronscreen", TRUE);
}

bool patternviewconfig_showbeatoffset(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "beatoffset", TRUE);
}

double patternviewconfig_linenumber_width(const PatternViewConfig* self)
{
	double rv;

	assert(self);

	rv = 0.0;
	if (patternviewconfig_linenumbers(self)) {
		rv += 5.0;
		if (patternviewconfig_linenumbersinhex(self)) {
		}
		if (patternviewconfig_showbeatoffset(self)) {
			rv += 5.0;
		}
		if (!patternviewconfig_issinglepatterndisplay(self)) {
			rv += 3.0;
		}
	}
	return rv;
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

/* events */
int patternviewconfig_onchanged(PatternViewConfig* self, psy_Property* property)
{
	int rebuild_level;

	assert(self);

	rebuild_level = 0;
	if (patternviewconfig_hasthemeproperty(self, property)) {
		psy_Property* choice;
		bool worked;

		choice = (psy_property_ischoiceitem(property)) ? psy_property_parent(property) : NULL;
		worked = FALSE;
		if (choice) {
			worked = TRUE;
			switch (psy_property_id(choice)) {
			case PROPERTY_ID_PATTERN_SKIN:
				patternviewconfig_loadbitmap(self);
				break;
			default:
				worked = FALSE;
				break;
			}
		}
		if (!worked) {
			patternviewconfig_write_styles(self);
		}		
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
	return rebuild_level;
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

psy_ui_FontInfo patternviewconfig_readfont(PatternViewConfig* self, double zoom)
{
	psy_ui_FontInfo fontinfo;
	
	psy_ui_fontinfo_init_string(&fontinfo,
		psy_property_at_str(self->patternview, "font", "tahoma;-16"));	
#if PSYCLE_USE_TK == PSYCLE_TK_X11	
	fontinfo.lfHeight = 18;	
#else				
	fontinfo.lfHeight = (int32_t)((double)fontinfo.lfHeight * zoom);
#endif		
	return fontinfo;
}

void patternviewconfig_write_styles(PatternViewConfig* self)
{
	if (self->theme) {
		psy_ui_Style* style;
		psy_ui_Colour bgcolour;
		psy_ui_Colour fgcolour;		

		fgcolour =  psy_ui_colour_make(psy_property_at_colour(self->theme,
			"pvc_font", 0x00CACACA));
		bgcolour = psy_ui_colour_make(psy_property_at_colour(self->theme,
			"pvc_background", 0x00292929));
		style = psy_ui_style(STYLE_PATTERNVIEW);
		if (style) {
			psy_ui_style_set_colours(style, fgcolour, bgcolour);
		}
		style = psy_ui_style(STYLE_PV_ROW);
		if (style) {
			psy_ui_style_set_colours(style,
				fgcolour,
				psy_ui_colour_make(psy_property_at_colour(
					self->theme, "pvc_row", 0x003E3E3E)));
		}
		style = psy_ui_style(STYLE_PV_ROW_SELECT);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_fontSel", 0x00ffffff)),				
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_selection", 0x009B7800)));
		}
		style = psy_ui_style(STYLE_PV_ROWBEAT);
		if (style) {
			psy_ui_style_set_colours(style,
				fgcolour,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_rowbeat", 0x00363636)));
		}		
		style = psy_ui_style(STYLE_PV_ROWBEAT_SELECT);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_fontSel", 0x00ffffff)),			
					psy_ui_diffadd_colours(
						psy_ui_colour_make(psy_property_at_colour(self->theme,
							"pvc_row", 0x003E3E3E)),
						psy_ui_colour_make(psy_property_at_colour(self->theme,
							"pvc_rowbeat", 0x00363636)),
						psy_ui_colour_make(psy_property_at_colour(self->theme,
							"pvc_selection", 0x009B7800))));			
		}
		style = psy_ui_style(STYLE_PV_ROW4BEAT);
		if (style) {
			psy_ui_style_set_colours(style,
				fgcolour,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_row4beat", 0x00595959)));
		}
		style = psy_ui_style(STYLE_PV_ROW4BEAT_SELECT);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_fontSel", 0x00ffffff)),
				psy_ui_diffadd_colours(
					psy_ui_colour_make(psy_property_at_colour(self->theme,
						"pvc_row", 0x003E3E3E)),
					psy_ui_colour_make(psy_property_at_colour(self->theme,
						"pvc_row4beat", 0x00363636)),
					psy_ui_colour_make(psy_property_at_colour(self->theme,
						"pvc_selection", 0x009B7800))));
		}
		style = psy_ui_style(STYLE_PV_CURSOR);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_fontCur", 0x00ffffff)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_cursor", 0x00595959)));
		}
		style = psy_ui_style(STYLE_PV_CURSOR_SELECT);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_fontSel", 0x00ffffff)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"pvc_selection", 0x009B7800)));
		}
	}
	patternviewconfig_loadbitmap(self);
}

void patternviewconfig_loadbitmap(PatternViewConfig* self)
{
	const char* pattern_header_skin_name;
	static int styles[] = {
		STYLE_PV_TRACK_HEADER,
		STYLE_PV_TRACK_HEADER_DIGITX0,
		STYLE_PV_TRACK_HEADER_DIGIT0X,		
		STYLE_PV_TRACK_HEADER_MUTE_SELECT,		
		STYLE_PV_TRACK_HEADER_SOLO_SELECT,		
		STYLE_PV_TRACK_HEADER_RECORD_SELECT,		
		STYLE_PV_TRACK_HEADER_PLAY_SELECT
	};
	
	pattern_header_skin_name = patternviewconfig_header_skin_name(self);
	if (psy_strlen(pattern_header_skin_name) == 0) {
		int i;
		psy_ui_Style* style;

		for (i = 0; styles[i] != 0; ++i) {
			style = psy_ui_style(styles[i]);			
			psy_ui_style_set_background_id(style, IDB_HEADERSKIN);			
		}
	} else {
		psy_Path filename;
		char path[_MAX_PATH];		
		
		psy_path_init_all(&filename, "", pattern_header_skin_name, "bmp");		
		psy_dir_findfile(self->skindir, psy_path_full(&filename), path);
		if (path[0] != '\0') {
			int i;			

			for (i = 0; styles[i] != 0; ++i) {
				psy_ui_Style* style;

				style = psy_ui_style(styles[i]);
				if (psy_ui_style_setbackgroundpath(style, path) != PSY_OK) {
					psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
				}
			}			
		}
		psy_path_setext(&filename, "psh");		
		psy_dir_findfile(self->skindir, psy_path_full(&filename), path);
		psy_path_dispose(&filename);
		if (psy_strlen(path) > 0) {		
			psy_Property* coords;

			coords = psy_property_allocinit_key(NULL);
			if (skin_load_pattern_header(coords, path) == PSY_OK) {
				const char* s;
				intptr_t vals[4];
				psy_ui_RealRectangle src;
				psy_ui_RealPoint dst;
				psy_ui_Style* style;
				psy_Property* transparency;
				
				src = psy_ui_realrectangle_zero();
				dst = psy_ui_realpoint_zero();
				if (s = psy_property_at_str(coords, "background_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewconfig_setsource(self, &src, vals);
					style = psy_ui_style(STYLE_PV_TRACK_HEADER);
					psy_ui_style_set_background_size_px(style, src.right - src.left, src.bottom - src.top);
					psy_ui_style_set_background_position_px(style, -src.left, -src.top);
				}
				if (s = psy_property_at_str(coords, "mute_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewconfig_setsource(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "mute_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewconfig_setdest(self, &dst, vals, 2);
					patternviewconfig_setstylecoords(self,
						STYLE_PV_TRACK_HEADER_MUTE,
						STYLE_PV_TRACK_HEADER_MUTE_SELECT,
						src, dst);
				}
				if (s = psy_property_at_str(coords, "solo_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewconfig_setsource(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "solo_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewconfig_setdest(self, &dst, vals, 2);
					patternviewconfig_setstylecoords(self,
						STYLE_PV_TRACK_HEADER_SOLO,
						STYLE_PV_TRACK_HEADER_SOLO_SELECT,
						src, dst);
				}
				if (s = psy_property_at_str(coords, "record_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewconfig_setsource(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "record_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewconfig_setdest(self, &dst, vals, 2);
					patternviewconfig_setstylecoords(self,
						STYLE_PV_TRACK_HEADER_RECORD,
						STYLE_PV_TRACK_HEADER_RECORD_SELECT,
						src, dst);
				}
				if (s = psy_property_at_str(coords, "number_0_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewconfig_setsource(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "digit_x0_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewconfig_setdest(self, &dst, vals, 2);
					style = psy_ui_style(STYLE_PV_TRACK_HEADER_DIGITX0);
					psy_ui_style_set_background_size_px(style, src.right - src.left,
						src.bottom - src.top);
					psy_ui_style_set_background_position_px(style, -src.left, -src.top);
					psy_ui_style_set_padding_px(style, dst.y, 0.0, 0.0, dst.x);
				}
				if (s = psy_property_at_str(coords, "digit_0x_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewconfig_setdest(self, &dst, vals, 2);
					style = psy_ui_style(STYLE_PV_TRACK_HEADER_DIGIT0X);
					psy_ui_style_set_background_size_px(style, src.right - src.left,
						src.bottom - src.top);
					psy_ui_style_set_background_position_px(style, -src.left, -src.top);
					psy_ui_style_set_padding_px(style, dst.y, 0.0, 0.0, dst.x);
				}
				if (s = psy_property_at_str(coords, "playing_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewconfig_setsource(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "playing_on_dest", 0)) {
						skin_psh_values(s, 2, vals);
					patternviewconfig_setdest(self, &dst, vals, 2);
					patternviewconfig_setstylecoords(self,
						STYLE_PV_TRACK_HEADER_PLAY,
						STYLE_PV_TRACK_HEADER_PLAY_SELECT,
						src, dst);
				}
				if (transparency = psy_property_at(coords, "transparency",
						PSY_PROPERTY_TYPE_NONE)) {
					if (transparency->item.marked) {
						psy_ui_Colour cltransparency;
						int i;

						cltransparency = psy_ui_colour_make(
							strtol(psy_property_item_str(transparency), 0, 16));
						for (i = 0; styles[i] != 0; ++i) {
							style = psy_ui_style(styles[i]);
							psy_ui_bitmap_settransparency(&style->background.bitmap,
								cltransparency);
						}
					}										
				}
			}
			psy_property_deallocate(coords);
		}
	}
}

void patternviewconfig_switch_header(PatternViewConfig* self)
{
	if (self->has_classic_header) {
		patternviewconfig_switch_to_text(self);
	} else {
		patternviewconfig_switch_to_classic(self);
	}
}

void patternviewconfig_switch_to_text(PatternViewConfig* self)
{
	psy_ui_Styles* styles;
	psy_ui_Style* style;	

	self->has_classic_header = FALSE;
	styles = &psy_ui_appdefaults()->styles;

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_DIGITX0));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_DIGITX0, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_DIGIT0X));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_DIGIT0X, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_MUTE));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_MUTE, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_MUTE_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_MUTE_SELECT, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_SOLO));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_SOLO, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_SOLO_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_SOLO_SELECT, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_RECORD));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_RECORD, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_RECORD_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_RECORD_SELECT, style);

	/* style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_PLAY));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_PLAY, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_PLAY_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_PLAY_SELECT, style); */

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_TEXT_HEADER_TEXT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_TEXT, style);
}

void patternviewconfig_switch_to_classic(PatternViewConfig* self)
{
	psy_ui_Styles* styles;
	psy_ui_Style* style;

	self->has_classic_header = TRUE;
	styles = &psy_ui_appdefaults()->styles;

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_DIGITX0));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_DIGITX0, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_DIGIT0X));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_DIGIT0X, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_MUTE));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_MUTE, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_MUTE_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_MUTE_SELECT, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_SOLO));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_SOLO, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_SOLO_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_SOLO_SELECT, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_RECORD));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_RECORD, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_RECORD_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_RECORD_SELECT, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_PLAY));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_PLAY, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_PLAY_SELECT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_PLAY_SELECT, style);

	style = psy_ui_style_clone(psy_ui_style(STYLE_PV_TRACK_CLASSIC_HEADER_TEXT));
	psy_ui_styles_set_style(styles, STYLE_PV_TRACK_HEADER_TEXT, style);
}

void patternviewconfig_setsource(PatternViewConfig* self, psy_ui_RealRectangle* r,
	intptr_t vals[4])
{
	r->left = (double)vals[0];
	r->top = (double)vals[1];
	r->right = (double)vals[0] + (double)vals[2];
	r->bottom = (double)vals[1] + (double)vals[3];
}

void patternviewconfig_setdest(PatternViewConfig* self, psy_ui_RealPoint* pt,
	intptr_t vals[4], uintptr_t num)
{
	pt->x = (double)vals[0];
	pt->y = (double)vals[1];
}

void patternviewconfig_setstylecoords(PatternViewConfig* self,
	uintptr_t styleid, uintptr_t select_styleid, psy_ui_RealRectangle src,
	psy_ui_RealPoint dst)
{	
	psy_ui_Style* style;
	psy_ui_Point pt;
	psy_ui_Size size;
	
	size = psy_ui_size_make_real(psy_ui_realrectangle_size(&src));		
	pt = psy_ui_point_make_real(dst);
	style = psy_ui_style(styleid);
	if (style) {
		psy_ui_style_set_position(style, psy_ui_rectangle_make(pt, size));
	}
	style = psy_ui_style(select_styleid);
	if (style) {
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_background_size_px(style, 
			psy_ui_realrectangle_width(&src),
			psy_ui_realrectangle_height(&src));
		psy_ui_style_set_position(style, psy_ui_rectangle_make(pt, size));
	}
}
