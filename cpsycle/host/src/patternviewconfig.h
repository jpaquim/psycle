/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWCONFIG_H)
#define PATTERNVIEWCONFIG_H

#include "patternviewtheme.h"
/* dsp */
#include <notestab.h>
/* container */
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PatternViewConfig */

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
	/* internal */
	psy_Property* patternview;
	PatternViewTheme theme;	
	bool singlemode;
} PatternViewConfig;

void patternviewconfig_init(PatternViewConfig*, psy_Property* parent,
	const char* skindir);
void patternviewconfig_dispose(PatternViewConfig*);

bool patternviewconfig_connect(PatternViewConfig*, const char* key, void* context,
	void* fp);

psy_Property* patternviewconfig_property(PatternViewConfig*, const char* key);
void patternviewconfig_reset_theme(PatternViewConfig* self);
const char* patternviewconfig_headerskinname(PatternViewConfig*);
void patternviewconfig_set_theme(PatternViewConfig*, psy_Property* skin);
bool boolpatternviewconfig_hasthemeproperty(const PatternViewConfig*,
	psy_Property*);

void patternviewconfig_switch_header(PatternViewConfig*);
void patternviewconfig_switch_to_classic(PatternViewConfig*);
void patternviewconfig_switch_to_text(PatternViewConfig*);

bool patternviewconfig_line_numbers(const PatternViewConfig*);
void patternviewconfig_display_line_numbers(PatternViewConfig*);
void patternviewconfig_hide_line_numbers(PatternViewConfig*);
bool patternviewconfig_linenumbersinhex(const PatternViewConfig*);
void patternviewconfig_display_line_numbers_in_hex(PatternViewConfig*);
void patternviewconfig_display_line_numbers_in_dec(PatternViewConfig*);
bool patternviewconfig_showbeatoffset(const PatternViewConfig*);
double patternviewconfig_linenumber_num_digits(const PatternViewConfig*);
bool patternviewconfig_linenumberscursor(const PatternViewConfig*);
void patternviewconfig_display_line_numbers_cursor(PatternViewConfig*);
void patternviewconfig_hide_line_numbers_cursor(PatternViewConfig*);
bool patternviewconfig_defaultline(const PatternViewConfig*);
bool patternviewconfig_wrap_around(const PatternViewConfig*);
void patternviewconfig_enable_wrap_around(PatternViewConfig*);
void patternviewconfig_disable_wrap_around(PatternViewConfig*);
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
void patternviewconfig_toggle_pattern_default_line(PatternViewConfig*);
bool patternviewconfig_is_smooth_scrolling(const PatternViewConfig*);
void patternviewconfig_set_smooth_scrolling(PatternViewConfig*, bool on);
psy_ui_FontInfo patternviewconfig_font_info(PatternViewConfig*, double zoom);
void patternviewconfig_set_zoom(PatternViewConfig*, double zoom);
double patternviewconfig_zoom(const PatternViewConfig*);
/* theme */


#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWCONFIG_H */
