/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWCONFIG_H)
#define PATTERNVIEWCONFIG_H

/* ui*/
#include <uifont.h>
/* dsp */
#include <notestab.h>
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PatternViewConfig */

enum {
	PROPERTY_ID_PATTERNDISPLAY = 80000,	
	PROPERTY_ID_TRACKSCOPES,
	PROPERTY_ID_PATTERN_SKIN
};

/* The patternview display modes */
typedef enum {
	PATTERN_DISPLAYMODE_TRACKER,					/* only tracker visible */
	PATTERN_DISPLAYMODE_PIANOROLL,					/* only pianoroll visible */
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,	/* both of them visible */
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL,
	PATTERN_DISPLAYMODE_INVALID,
	PATTERN_DISPLAYMODE_NUM
} PatternDisplayMode;


typedef struct PatternViewConfig {
	/* signals */
	psy_Signal signal_changed;	
	/* internal */
	psy_Property* patternview;
	psy_Property* theme;
	psy_Property* headerskins;
	char* skindir;
	bool has_classic_header;
	double zoom;
	bool singlemode;
	/* references */
	psy_Property* parent;
	struct DirConfig* dirconfig;
} PatternViewConfig;

void patternviewconfig_init(PatternViewConfig*, psy_Property* parent,
	const char* skindir);
void patternviewconfig_dispose(PatternViewConfig*);

void patternviewconfig_set_directories(PatternViewConfig*, struct DirConfig*);
void patternviewconfig_update_header_skins(PatternViewConfig*);
void patternviewconfig_load_bitmap(PatternViewConfig*);
void patternviewconfig_reset_theme(PatternViewConfig* self);
const char* patternviewconfig_headerskinname(PatternViewConfig*);
void patternviewconfig_set_theme(PatternViewConfig*, psy_Property* skin);
bool boolpatternviewconfig_hasthemeproperty(const PatternViewConfig*,
	psy_Property*);
bool patternviewconfig_has_property(const PatternViewConfig*, psy_Property*);

void patternviewconfig_switch_header(PatternViewConfig*);
void patternviewconfig_switch_to_classic(PatternViewConfig*);
void patternviewconfig_switch_to_text(PatternViewConfig*);

bool patternviewconfig_line_numbers(const PatternViewConfig*);
bool patternviewconfig_linenumbersinhex(const PatternViewConfig*);
bool patternviewconfig_showbeatoffset(const PatternViewConfig*);
double patternviewconfig_linenumber_num_digits(const PatternViewConfig*);
bool patternviewconfig_linenumberscursor(const PatternViewConfig*);
bool patternviewconfig_defaultline(const PatternViewConfig*);
bool patternviewconfig_wrap_around(const PatternViewConfig*);
bool patternviewconfig_draw_empty_data(const PatternViewConfig*);
bool patternviewconfig_center_cursor_on_screen(const PatternViewConfig*);
bool patternviewconfig_show_wide_inst_column(const PatternViewConfig*);
psy_dsp_NotesTabMode patternviewconfig_notetabmode(const PatternViewConfig*);
bool patternviewconfig_ismovecursorwhenpaste(const PatternViewConfig*);
void patternviewconfig_setmovecursorwhenpaste(PatternViewConfig*, bool on);
void patternviewconfig_select_pattern_display(PatternViewConfig*,
	PatternDisplayMode);
PatternDisplayMode patternviewconfig_pattern_display(const PatternViewConfig*);
void patternviewconfig_display_single_pattern(PatternViewConfig*);
void patternviewconfig_display_sequence(PatternViewConfig*);
bool patternviewconfig_single_mode(const PatternViewConfig*);
bool patternviewconfig_use_header_bitmap(const PatternViewConfig*);
bool patternviewconfig_show_trackscopes(const PatternViewConfig*);
void patternviewconfig_toggle_pattern_defaultline(PatternViewConfig*);
bool patternviewconfig_is_smooth_scrolling(const PatternViewConfig*);
void patternviewconfig_set_smooth_scrolling(PatternViewConfig*, bool on);
psy_ui_FontInfo patternviewconfig_fontinfo(PatternViewConfig*, double zoom);
void patternviewconfig_write_styles(PatternViewConfig*);
void patternviewconfig_set_zoom(PatternViewConfig*, double zoom);
double patternviewconfig_zoom(const PatternViewConfig*);

/* events */
int patternviewconfig_onchanged(PatternViewConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWCONFIG_H */
