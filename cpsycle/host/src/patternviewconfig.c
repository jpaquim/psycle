/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewconfig.h"
/* host */
#include "resources/resource.h"


/* prototypes*/
static void patternviewconfig_make(PatternViewConfig*, psy_Property*
	parent);

/* implementation */
void patternviewconfig_init(PatternViewConfig* self, psy_Property* parent,
	const char* skindir)
{	
	assert(self);
	assert(parent);
	
	self->singlemode = TRUE;
	patternviewconfig_make(self, parent);
	patternviewtheme_init(&self->theme, self->patternview, skindir);
}

void patternviewconfig_dispose(PatternViewConfig* self)
{
	assert(self);
				
	patternviewtheme_dispose(&self->theme);	
}

void patternviewconfig_make(PatternViewConfig* self, psy_Property* parent)
{
	psy_Property* pvc;
	psy_Property* choice;

	assert(self);

	pvc = psy_property_append_section(parent, "patternview");
	psy_property_set_text(pvc,
		"settings.pv.patternview");
	self->patternview = pvc;
	psy_property_set_text(
		psy_property_append_font(pvc, "font", PSYCLE_DEFAULT_PATTERN_FONT),
		"settings.pv.font");
	psy_property_set_hint(psy_property_set_text(psy_property_append_double(
		self->patternview, "zoom", 1.0, 0.1, 4.0),
		"settings.visual.zoom"),
		PSY_PROPERTY_HINT_ZOOM);
	psy_property_set_text(
		psy_property_append_bool(pvc, "smoothscroll", FALSE),
		"settings.pv.smoothscroll");
	psy_property_set_text(
		psy_property_append_bool(pvc, "drawemptydata", FALSE),
		"settings.pv.draw-empty-data");
	psy_property_set_text(
		psy_property_append_bool(pvc, "griddefaults", TRUE),
		"settings.pv.default-entries");
	psy_property_set_text(
		psy_property_append_bool(pvc, "linenumbers", TRUE),
		"settings.pv.line-numbers");
	psy_property_set_text(
		psy_property_append_bool(pvc, "beatoffset", FALSE),
		"settings.pv.beat-offset");
	psy_property_set_text(
		psy_property_append_bool(pvc, "linenumberscursor", TRUE),
		"settings.pv.line-numbers-cursor");
	psy_property_set_text(
		psy_property_append_bool(pvc, "linenumbersinhex", FALSE),
		"settings.pv.line-numbers-in-hex");
	psy_property_set_text(
		psy_property_append_bool(pvc, "wideinstcolumn", FALSE),
		"settings.pv.wide-instrument-column");
	psy_property_set_text(
		psy_property_append_bool(pvc, "trackscopes", TRUE),
		"settings.pv.pattern-track-scopes");
	psy_property_set_text(
		psy_property_append_bool(pvc, "wraparound", TRUE),
		"settings.pv.wrap-around");
	psy_property_set_text(
		psy_property_append_bool(pvc, "centercursoronscreen", FALSE),
		"settings.pv.center-cursor-on-screen");
	psy_property_set_text(
		psy_property_append_int(pvc, "beatsperbar", 4, 1, 16),
		"settings.pv.bar-highlighting");
	psy_property_set_text(
		psy_property_append_bool(pvc, "notetab", TRUE),
		"settings.pv.a4-440hz");
	psy_property_set_text(
		psy_property_append_bool(pvc, "movecursorwhenpaste", TRUE),
		"settings.pv.move-cursor-when-paste");
	psy_property_set_text(
		psy_property_append_bool(pvc, "displaysinglepattern", TRUE),
		"settings.pv.displaysinglepattern");
	/*
	** useheaderbitmap
	** default set to false, because the bitmap skins dont fit to recent
	** resolutions
	*/
	psy_property_set_text(
		psy_property_append_bool(pvc, "useheaderbitmap", FALSE),
		"settings.pv.useheaderbitmap");
	/* pattern display choice */
	choice = psy_property_set_text(
		psy_property_append_choice(pvc, "patterndisplay", 0),
		"settings.pv.patterndisplay");
	psy_property_set_text(
		psy_property_append_int(choice, "tracker",
			0, 0, 0),
		"settings.pv.tracker");
	psy_property_set_text(
		psy_property_append_int(choice, "piano",
			0, 0, 0),
		"settings.pv.piano");
	psy_property_set_text(
		psy_property_append_int(choice, "splitvertical",
			0, 0, 0),
		"settings.pv.splitvertical");
	psy_property_set_text(
		psy_property_append_int(choice, "splithorizontal",
			0, 0, 0),
		"settings.pv.splithorizontal");	
}

void patternviewconfig_reset_theme(PatternViewConfig* self)
{
	assert(self);

	patternviewtheme_reset(&self->theme);
}

void patternviewconfig_set_theme(PatternViewConfig* self, psy_Property* theme)
{
	assert(self);

	patternviewtheme_set_theme(&self->theme, theme);	
}


bool patternviewconfig_line_numbers(const PatternViewConfig* self)
{
	assert(self);
	
	return psy_property_at_bool(self->patternview, "linenumbers", TRUE);
}

void patternviewconfig_display_line_numbers(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "linenumbers", TRUE);
}

void patternviewconfig_hide_line_numbers(PatternViewConfig* self)
{
	assert(self);
	
	psy_property_set_bool(self->patternview, "linenumbers", FALSE);
}

bool patternviewconfig_linenumberscursor(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "linenumberscursor", TRUE);
}

void patternviewconfig_display_line_numbers_cursor(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "linenumberscursor", TRUE);
}

void patternviewconfig_hide_line_numbers_cursor(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "linenumberscursor", FALSE);
}

bool patternviewconfig_linenumbersinhex(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "linenumbersinhex", TRUE);
}

void patternviewconfig_display_line_numbers_in_hex(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "linenumbersinhex", TRUE);
}

void patternviewconfig_display_line_numbers_in_dec(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "linenumbersinhex", FALSE);
}

bool patternviewconfig_defaultline(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "griddefaults", TRUE);
}

bool patternviewconfig_wrap_around(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "wraparound", TRUE);
}

void patternviewconfig_enable_wrap_around(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "wraparound", TRUE);
}

void patternviewconfig_disable_wrap_around(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "wraparound", FALSE);
}

bool patternviewconfig_draw_empty_data(const PatternViewConfig* self)
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

double patternviewconfig_linenumber_num_digits(const PatternViewConfig* self)
{
	double rv;

	assert(self);

	rv = 0.0;
	if (patternviewconfig_line_numbers(self)) {
		rv += 5.0;
		if (patternviewconfig_showbeatoffset(self)) {
			rv += 5.0;
		}
		if (!patternviewconfig_single_mode(self)) {
			rv += 3.0;
		}
	}
	return rv;
}

bool patternviewconfig_show_wide_inst_column(const PatternViewConfig* self)
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

bool patternviewconfig_is_smooth_scrolling(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "smoothscroll",
		TRUE);
}

void patternviewconfig_set_smooth_scrolling(PatternViewConfig* self, bool on)
{
	assert(self);

	psy_property_set_bool(self->patternview, "smoothscroll", on);
}


void patternviewconfig_display_single_pattern(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "displaysinglepattern", TRUE);
	self->singlemode = TRUE;	
}

void patternviewconfig_display_sequence(PatternViewConfig* self)
{
	assert(self);

	psy_property_set_bool(self->patternview, "displaysinglepattern", FALSE);
	self->singlemode = FALSE;	
}

void patternviewconfig_select_pattern_display(PatternViewConfig* self,
	PatternDisplayMode display)
{
	psy_Property* patterndisplay;

	assert(self);
	assert(self->patternview);

	patterndisplay = psy_property_at(self->patternview, "patterndisplay",
		PSY_PROPERTY_TYPE_CHOICE);
	if (patterndisplay) {
		psy_property_set_item_int(patterndisplay, display);
	}	
}

PatternDisplayMode patternviewconfig_pattern_display(const PatternViewConfig* self)
{
	psy_Property* property;

	property = psy_property_at(self->patternview, "patterndisplay",
		PSY_PROPERTY_TYPE_CHOICE);
	if (property) {
		return (PatternDisplayMode)psy_property_item_int(property);
	}
	return PATTERN_DISPLAYMODE_TRACKER;
}

bool patternviewconfig_single_mode(const PatternViewConfig* self)
{
	return self->singlemode;
	/* return psy_property_at_bool(self->patternview, "displaysinglepattern",
		TRUE); */
}

bool patternviewconfig_use_header_bitmap(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "useheaderbitmap", TRUE);
}



bool patternviewconfig_show_trackscopes(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->patternview, "trackscopes", TRUE);
}

void patternviewconfig_toggle_pattern_default_line(PatternViewConfig* self)
{			
	psy_Property* property;

	assert(self);

	property = psy_property_at(self->patternview, "griddefaults",
		PSY_PROPERTY_TYPE_NONE);
	if (property) {
		psy_property_set_item_bool(property, !psy_property_item_bool(property));		
	}
}

void patternviewconfig_set_zoom(PatternViewConfig* self, double zoom)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self->patternview, "zoom",
		PSY_PROPERTY_TYPE_DOUBLE);
	if (property) {
		psy_property_set_item_double(property, zoom);
	}	
}

double patternviewconfig_zoom(const PatternViewConfig* self)
{
	assert(self);

	return psy_property_at_real(self->patternview, "zoom", 1.0);
}

psy_ui_FontInfo patternviewconfig_font_info(PatternViewConfig* self,
	double zoom)
{
	psy_ui_FontInfo fontinfo;

	assert(self);
	
	psy_ui_fontinfo_init_string(&fontinfo,
		psy_property_at_str(self->patternview, "font", "tahoma; 16"));	
	fontinfo.lfHeight = (int32_t)((double)fontinfo.lfHeight * zoom);
	return fontinfo;
}

bool patternviewconfig_connect(PatternViewConfig* self, const char* key, void* context,
	void* fp)
{
	psy_Property* p;

	assert(self);

	p = patternviewconfig_property(self, key);
	if (p) {
		psy_property_connect(p, context, fp);
		return TRUE;
	}
	return FALSE;
}

psy_Property* patternviewconfig_property(PatternViewConfig* self, const char* key)
{
	assert(self);

	return psy_property_at(self->patternview, key, PSY_PROPERTY_TYPE_NONE);
}
