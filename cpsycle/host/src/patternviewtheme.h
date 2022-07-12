/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWTHEME_H)
#define PATTERNVIEWTHEME_H

/* ui*/
#include <uicolour.h>
#include <uifont.h>
/* dsp */
#include <notestab.h>
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PatternViewTheme */

struct DirConfig;

typedef struct PatternViewTheme {
	psy_Property* theme;
	psy_Property* headerskins;	
	char* skindir;
	bool has_classic_header;
	/* references */
	struct DirConfig* dirconfig;
} PatternViewTheme;

void patternviewtheme_init(PatternViewTheme*, psy_Property* parent,
	const char* skindir);
void patternviewtheme_dispose(PatternViewTheme*);

bool patternviewtheme_connect(PatternViewTheme*, const char* key, void* context,
	void* fp);
psy_Property* patternviewtheme_property(PatternViewTheme*, const char* key);

void patternviewtheme_set_directories(PatternViewTheme* self,
	struct DirConfig*);
void patternviewtheme_reset(PatternViewTheme* self);
void patternviewtheme_set_theme(PatternViewTheme*, psy_Property* theme);
void patternviewtheme_write_styles(PatternViewTheme*);

void patternviewtheme_switch_header(PatternViewTheme*);
void patternviewtheme_switch_to_classic(PatternViewTheme*);
void patternviewtheme_switch_to_text(PatternViewTheme*);

const char* patternviewtheme_header_skin_name(const PatternViewTheme*);
void patternviewtheme_set_background_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_background_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_row4beat_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_row4beat_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_rowbeat_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_rowbeat_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_row_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_row_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_font_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_font_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_font_play_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_font_play_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_font_sel_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_font_sel_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_selection_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_selection_colour_right(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_playbar_colour_left(PatternViewTheme*,
	psy_ui_Colour);
void patternviewtheme_set_playbar_colour_right(PatternViewTheme*,
	psy_ui_Colour);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWTHEME_H */
