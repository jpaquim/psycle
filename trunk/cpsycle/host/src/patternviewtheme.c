/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewtheme.h"
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

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#include <unistd.h>
#endif

/* PatternViewTheme */

/* prototypes */
static void patternviewtheme_make(PatternViewTheme* self, psy_Property* parent);
static void patternviewtheme_update_header_skins(PatternViewTheme*);
static void patternviewtheme_on_pattern_skin(PatternViewTheme*,
	psy_Property* sender);
static void patternviewtheme_load_bitmap(PatternViewTheme*);
static void patternviewtheme_set_source(PatternViewTheme*,
	psy_ui_RealRectangle*, intptr_t vals[4]);
static void patternviewtheme_set_dest(PatternViewTheme*, psy_ui_RealPoint*,
	intptr_t vals[4], uintptr_t num);
static void patternviewtheme_set_style_coords(PatternViewTheme*,
	uintptr_t styleid, uintptr_t select_styleid, psy_ui_RealRectangle src,
	psy_ui_RealPoint dst);
static void patternviewtheme_set_colour(PatternViewTheme*,
	const char* key, uintptr_t style_id, psy_ui_Colour);
static void patternviewtheme_set_background_colour(PatternViewTheme*,
	const char* key, uintptr_t style_id, psy_ui_Colour);
static void patternviewtheme_on_pattern_skin(PatternViewTheme*,
	psy_Property* sender);
static void patternviewtheme_on_colour(PatternViewTheme*,
	psy_Property* sender);
static void patternviewtheme_on_background_colour(PatternViewTheme*,
	psy_Property* sender);

/* implementation */
void patternviewtheme_init(PatternViewTheme* self, psy_Property* parent,
	const char* skindir)
{
	assert(self);

	self->skindir = psy_strdup(skindir);
	self->has_classic_header = TRUE;
	self->dirconfig = NULL;
	patternviewtheme_make(self, parent);
	/* connect configuration */
	patternviewtheme_connect(self, "pvc_font",
		self, patternviewtheme_on_colour);
	patternviewtheme_connect(self, "pvc_background",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_font2",
		self, patternviewtheme_on_colour);
	patternviewtheme_connect(self, "pvc_background2",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_row",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_row2",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_rowbeat",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_rowbeat2",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_row4beat",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_row4beat2",
		self, patternviewtheme_on_background_colour);
	/* playbar */
	patternviewtheme_connect(self, "pvc_playbar",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_playbar2",
		self, patternviewtheme_on_background_colour);
	/* cursor */
	patternviewtheme_connect(self, "pvc_fontcur",
		self, patternviewtheme_on_colour);
	patternviewtheme_connect(self, "pvc_cursor",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_fontcur2",
		self, patternviewtheme_on_colour);
	patternviewtheme_connect(self, "pvc_cursor2",
		self, patternviewtheme_on_background_colour);
	/* midline */
	patternviewtheme_connect(self, "pvc_midline",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_midline2",
		self, patternviewtheme_on_background_colour);	
	/* selection */
	patternviewtheme_connect(self, "pvc_fontsel",
		self, patternviewtheme_on_colour);
	patternviewtheme_connect(self, "pvc_selection",
		self, patternviewtheme_on_background_colour);
	patternviewtheme_connect(self, "pvc_fontsel2",
		self, patternviewtheme_on_colour);
	patternviewtheme_connect(self, "pvc_selection2",
		self, patternviewtheme_on_background_colour);
}

void patternviewtheme_dispose(PatternViewTheme* self)
{
	assert(self);

	free(self->skindir);
	self->skindir = NULL;
}

void patternviewtheme_set_directories(PatternViewTheme* self,
	DirConfig* dirconfig)
{
	self->dirconfig = dirconfig;
	patternviewtheme_update_header_skins(self);
}

void patternviewtheme_make(PatternViewTheme* self, psy_Property* parent)
{
	assert(self);

	self->theme = psy_property_set_text(
		psy_property_append_section(parent, "theme"),
		"settingsview.pv.theme.theme");
	psy_property_set_text(
		psy_property_append_str(self->theme,
			"pattern_fontface", "Tahoma"),
		"settingsview.pv.theme.fontface");
	psy_property_set_text(
		psy_property_append_int(self->theme,
			"pattern_font_point", 0x00000050, 0, 0),
		"settingsview.pv.theme.fontpoint");
	psy_property_set_text(
		psy_property_append_int(self->theme,
			"pattern_font_flags", 0x00000001, 0, 0),
		"settingsview.pv.theme.fontflags");
	psy_property_set_text(
		psy_property_append_int(self->theme,
			"pattern_font_x", 0x00000009, 0, 0),
		"settingsview.pv.theme.font_x");
	psy_property_set_text(
		psy_property_append_int(self->theme,
			"pattern_font_y", 0x0000000B, 0, 0),
		"settingsview.pv.theme.font_y");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_separator", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.separator");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_separator2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.separator2");
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_background", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.background"),
		STYLE_PATTERNVIEW);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_background2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.background2"),
		STYLE_PATTERNVIEW);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_font", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.font"),
		STYLE_PATTERNVIEW);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_font2", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.font2"),
		STYLE_PATTERNVIEW);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_fontCur", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontcur"),
		STYLE_PV_CURSOR);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_fontCur2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontcur2"),
		STYLE_PV_CURSOR);
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_fontSel", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontsel");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_fontSel2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontsel2");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_fontPlay", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontplay");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_fontPlay2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.fontplay2");
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_row", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row"),
		STYLE_PV_ROW);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_row2", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row2"),
		STYLE_PV_ROW);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_rowbeat", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.rowbeat"),
		STYLE_PV_ROWBEAT);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_rowbeat2", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.rowbeat2"),
		STYLE_PV_ROWBEAT);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_row4beat", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row4beat"),
		STYLE_PV_ROW4BEAT);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_row4beat2", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.row4beat2"),
		STYLE_PV_ROW4BEAT);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_selection", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.selection"),
		STYLE_PV_ROW_SELECT);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_selection2", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.selection2"),
		STYLE_PV_ROW_SELECT);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_playbar", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.playbar"),
		STYLE_PV_PLAYBAR);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_playbar2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.playbar2"),
		STYLE_PV_PLAYBAR);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_cursor", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.cursor"),
		STYLE_PV_CURSOR);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_cursor2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.cursor2"),
		STYLE_PV_CURSOR);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_midline", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.midline"),
		STYLE_PV_PLAYBAR);
	psy_property_set_id(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"pvc_midline2", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.pv.theme.midline2"),
		STYLE_PV_PLAYBAR);
	self->headerskins = psy_property_connect(
		psy_property_set_hint(psy_property_set_text(
			psy_property_append_choice(self->theme, "skins", 0),
			"Skin"), PSY_PROPERTY_HINT_COMBO),		
		self, patternviewtheme_on_pattern_skin);
	patternviewtheme_update_header_skins(self);
}

void patternviewtheme_update_header_skins(PatternViewTheme* self)
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

void patternviewtheme_reset(PatternViewTheme* self)
{
	psy_Property* parent;

	assert(self);
		
	if (self->theme) {
		parent = self->theme->parent;				
	} else {
		parent = NULL;
	}
	if (parent) {
		psy_property_remove(self->theme->parent, self->theme);
	}
	patternviewtheme_make(self, parent);
	psy_property_set_item_int(self->headerskins, 0);
	init_patternview_styles(&psy_ui_appdefaults()->styles);
}

void patternviewtheme_set_theme(PatternViewTheme* self, psy_Property* theme)
{
	assert(self);

	if (self->theme) {
		psy_Property* header_skin;

		header_skin = psy_property_at(self->headerskins,
			psy_property_at_str(theme, "pattern_header_skin", ""),
			PSY_PROPERTY_TYPE_STRING);
		if (header_skin) {
			psy_property_set_item_int(self->headerskins,
				psy_property_index(header_skin));
		}
		else {
			psy_property_set_item_int(self->headerskins, 0);
		}
		psy_property_sync(self->theme, theme);
		patternviewtheme_write_styles(self);
	}
}

const char* patternviewtheme_header_skin_name(const PatternViewTheme* self)
{
	psy_Property* choice;

	assert(self);

	choice = psy_property_at_choice(((PatternViewTheme*)self)->headerskins);
	if (choice && psy_strlen(psy_property_item_str(choice)) > 0) {
		return psy_property_key(choice);
	}
	return "";
}

void patternviewtheme_write_styles(PatternViewTheme* self)
{
	if (self->theme) {
		psy_ui_Style* style;
		psy_ui_Colour bgcolour;
		psy_ui_Colour fgcolour;

		fgcolour = psy_ui_colour_make(psy_property_at_colour(self->theme,
			"pvc_font", 0x00CACACA));
		bgcolour = psy_ui_colour_make(psy_property_at_colour(self->theme,
			"pvc_background", 0x00292929));
		style = psy_ui_style(STYLE_PATTERNVIEW);
		if (style) {
			psy_ui_style_set_colours(style, fgcolour, bgcolour);
		}
		style = psy_ui_style(STYLE_PV_ROW);
		if (style) {
			psy_ui_style_set_background_colour(style,
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
			psy_ui_style_set_background_colour(style,				
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
			psy_ui_style_set_background_colour(style,				
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
	patternviewtheme_load_bitmap(self);
}

void patternviewtheme_load_bitmap(PatternViewTheme* self)
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

	pattern_header_skin_name = patternviewtheme_header_skin_name(self);
	if (psy_strlen(pattern_header_skin_name) == 0) {
		int i;
		psy_ui_Style* style;

		for (i = 0; styles[i] != 0; ++i) {
			style = psy_ui_style(styles[i]);
			psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
		}
	}
	else {
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
					patternviewtheme_set_source(self, &src, vals);
					style = psy_ui_style(STYLE_PV_TRACK_HEADER);
					psy_ui_style_set_background_size_px(style,
						psy_ui_realsize_make(
							src.right - src.left,
							src.bottom - src.top));
					psy_ui_style_set_background_position_px(style, -src.left, -src.top);
				}
				if (s = psy_property_at_str(coords, "mute_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewtheme_set_source(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "mute_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewtheme_set_dest(self, &dst, vals, 2);
					patternviewtheme_set_style_coords(self,
						STYLE_PV_TRACK_HEADER_MUTE,
						STYLE_PV_TRACK_HEADER_MUTE_SELECT,
						src, dst);
				}
				if (s = psy_property_at_str(coords, "solo_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewtheme_set_source(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "solo_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewtheme_set_dest(self, &dst, vals, 2);
					patternviewtheme_set_style_coords(self,
						STYLE_PV_TRACK_HEADER_SOLO,
						STYLE_PV_TRACK_HEADER_SOLO_SELECT,
						src, dst);
				}
				if (s = psy_property_at_str(coords, "record_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewtheme_set_source(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "record_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewtheme_set_dest(self, &dst, vals, 2);
					patternviewtheme_set_style_coords(self,
						STYLE_PV_TRACK_HEADER_RECORD,
						STYLE_PV_TRACK_HEADER_RECORD_SELECT,
						src, dst);
				}
				if (s = psy_property_at_str(coords, "number_0_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewtheme_set_source(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "digit_x0_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewtheme_set_dest(self, &dst, vals, 2);
					style = psy_ui_style(STYLE_PV_TRACK_HEADER_DIGITX0);
					psy_ui_style_set_background_size_px(style,
						psy_ui_realrectangle_size(&src));
					psy_ui_style_set_background_position_px(style, -src.left, -src.top);
					psy_ui_style_set_padding_px(style, dst.y, 0.0, 0.0, dst.x);
				}
				if (s = psy_property_at_str(coords, "digit_0x_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewtheme_set_dest(self, &dst, vals, 2);
					style = psy_ui_style(STYLE_PV_TRACK_HEADER_DIGIT0X);
					psy_ui_style_set_background_size_px(style,
						psy_ui_realrectangle_size(&src));
					psy_ui_style_set_background_position_px(style, -src.left, -src.top);
					psy_ui_style_set_padding_px(style, dst.y, 0.0, 0.0, dst.x);
				}
				if (s = psy_property_at_str(coords, "playing_on_source", 0)) {
					skin_psh_values(s, 4, vals);
					patternviewtheme_set_source(self, &src, vals);
				}
				if (s = psy_property_at_str(coords, "playing_on_dest", 0)) {
					skin_psh_values(s, 2, vals);
					patternviewtheme_set_dest(self, &dst, vals, 2);
					patternviewtheme_set_style_coords(self,
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

void patternviewtheme_switch_header(PatternViewTheme* self)
{
	if (self->has_classic_header) {
		patternviewtheme_switch_to_text(self);
	}
	else {
		patternviewtheme_switch_to_classic(self);
	}
}

void patternviewtheme_switch_to_text(PatternViewTheme* self)
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

void patternviewtheme_switch_to_classic(PatternViewTheme* self)
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

void patternviewtheme_set_source(PatternViewTheme* self, psy_ui_RealRectangle* r,
	intptr_t vals[4])
{
	r->left = (double)vals[0];
	r->top = (double)vals[1];
	r->right = (double)vals[0] + (double)vals[2];
	r->bottom = (double)vals[1] + (double)vals[3];
}

void patternviewtheme_set_dest(PatternViewTheme* self, psy_ui_RealPoint* pt,
	intptr_t vals[4], uintptr_t num)
{
	pt->x = (double)vals[0];
	pt->y = (double)vals[1];
}

void patternviewtheme_set_style_coords(PatternViewTheme* self,
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
			psy_ui_realrectangle_size(&src));
		psy_ui_style_set_position(style, psy_ui_rectangle_make(pt, size));
	}
}

void patternviewtheme_on_colour(PatternViewTheme* self,
	psy_Property* sender)
{
	psy_ui_Style* style;

	style = psy_ui_style(psy_property_id(sender));
	if (style) {
		psy_ui_style_set_colour(style, psy_ui_colour_make(
			(uint32_t)psy_property_item_int(sender)));
	}
}

void patternviewtheme_on_background_colour(PatternViewTheme* self,
	psy_Property* sender)
{
	psy_ui_Style* style;

	style = psy_ui_style(psy_property_id(sender));
	if (style) {
		psy_ui_style_set_background_colour(style, psy_ui_colour_make(
			(uint32_t)psy_property_item_int(sender)));
	}
}

void patternviewtheme_set_background_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour_left)
{
	patternviewtheme_set_background_colour(self, "pvc_background",
		STYLE_PATTERNVIEW, colour_left);
}

void patternviewtheme_set_background_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour_right)
{
	patternviewtheme_set_background_colour(self, "pvc_background2",
		STYLE_PATTERNVIEW, colour_right);
}

void patternviewtheme_set_row4beat_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_row4beat",
		STYLE_PV_ROW4BEAT, colour);
}

void patternviewtheme_set_row4beat_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_row4beat2",
		STYLE_PV_ROW4BEAT, colour);
}

void patternviewtheme_set_rowbeat_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_rowbeat",
		STYLE_PV_ROWBEAT, colour);
}

void patternviewtheme_set_rowbeat_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_rowbeat2",
		STYLE_PV_ROWBEAT, colour);
}

void patternviewtheme_set_row_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_row",
		STYLE_PV_ROW, colour);
}

void patternviewtheme_set_row_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_row2",
		STYLE_PV_ROW, colour);
}

void patternviewtheme_set_font_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_font",
		STYLE_PATTERNVIEW, colour);
}

void patternviewtheme_set_font_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_font2",
		STYLE_PATTERNVIEW, colour);
}

void patternviewtheme_set_font_play_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_fontPlay",
		STYLE_PV_PLAYBAR, colour);
}

void patternviewtheme_set_font_play_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_fontPlay2",
		STYLE_PV_PLAYBAR, colour);
}

void patternviewtheme_set_font_sel_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_fontSel",
		STYLE_PV_ROW_SELECT, colour);
}

void patternviewtheme_set_font_sel_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_fontSel2",
		STYLE_PV_ROW_SELECT, colour);
}

void patternviewtheme_set_selection_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_selection",
		STYLE_PV_ROW_SELECT, colour);
}

void patternviewtheme_set_selection_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_background_colour(self, "pvc_selection2",
		STYLE_PV_ROW_SELECT, colour);
}

void patternviewtheme_set_playbar_colour_left(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_playbar",
		STYLE_PV_PLAYBAR, colour);
}

void patternviewtheme_set_playbar_colour_right(PatternViewTheme* self,
	psy_ui_Colour colour)
{
	patternviewtheme_set_colour(self, "pvc_playbar2",
		STYLE_PV_PLAYBAR, colour);
}

void patternviewtheme_set_colour(PatternViewTheme* self,
	const char* key, uintptr_t style_id, psy_ui_Colour colour)
{
	psy_ui_Style* style;

	style = psy_ui_style(style_id);
	if (style) {
		psy_ui_style_set_colour(style, colour);
	}
	psy_property_set_int(self->theme, key,
		psy_ui_colour_colorref(&colour));
}

void patternviewtheme_set_background_colour(PatternViewTheme* self,
	const char* key, uintptr_t style_id, psy_ui_Colour colour)
{
	psy_ui_Style* style;

	style = psy_ui_style(style_id);
	if (style) {
		psy_ui_style_set_background_colour(style, colour);
	}
	psy_property_set_int(self->theme, key,
		psy_ui_colour_colorref(&colour));
}

void patternviewtheme_on_pattern_skin(PatternViewTheme* self, psy_Property* sender)
{
	patternviewtheme_load_bitmap(self);
}

bool patternviewtheme_connect(PatternViewTheme* self, const char* key, void* context,
	void* fp)
{
	psy_Property* p;

	assert(self);

	p = patternviewtheme_property(self, key);
	if (p) {
		psy_property_connect(p, context, fp);
		return TRUE;
	}
	return FALSE;
}

psy_Property* patternviewtheme_property(PatternViewTheme* self, const char* key)
{
	assert(self);

	return psy_property_at(self->theme, key, PSY_PROPERTY_TYPE_NONE);
}
