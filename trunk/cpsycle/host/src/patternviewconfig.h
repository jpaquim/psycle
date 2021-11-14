/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWCONFIG_H)
#define PATTERNVIEWCONFIG_H

/* host */
#include "patternviewskin.h"
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
	PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER,
	PROPERTY_ID_PATTERN_DISPLAYMODE_PIANOROLL,
	PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,
	PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL,
	PROPERTY_ID_TRACKSCOPES
};

typedef struct PatternViewConfig {
	/* signals */
	psy_Signal signal_changed;
	psy_Signal signal_themechanged;
	/* internal */
	psy_Property* patternview;
	psy_Property* theme;
	PatternViewSkin skin;
	char* skindir;
	/* references */
	psy_Property* parent;
} PatternViewConfig;

void patternviewconfig_init(PatternViewConfig*, psy_Property* parent,
	const char* skindir);
void patternviewconfig_dispose(PatternViewConfig*);

void patternviewconfig_resettheme(PatternViewConfig* self);
void patternviewconfig_settheme(PatternViewConfig*, psy_Property* skin);
bool boolpatternviewconfig_hasthemeproperty(const PatternViewConfig*,
	psy_Property*);
bool patternviewconfig_hasproperty(const PatternViewConfig*, psy_Property*);

bool patternviewconfig_linenumbers(const PatternViewConfig*);
bool patternviewconfig_linenumberscursor(const PatternViewConfig*);
bool patternviewconfig_linenumbersinhex(const PatternViewConfig*);
bool patternviewconfig_defaultline(const PatternViewConfig*);
bool patternviewconfig_wraparound(const PatternViewConfig*);
bool patternviewconfig_drawemptydata(const PatternViewConfig*);
bool patternviewconfig_centercursoronscreen(const PatternViewConfig*);
bool patternviewconfig_showbeatoffset(const PatternViewConfig*);
bool patternviewconfig_showwideinstcolumn(const PatternViewConfig*);
psy_dsp_NotesTabMode patternviewconfig_notetabmode(const PatternViewConfig*);
bool patternviewconfig_ismovecursorwhenpaste(const PatternViewConfig*);
void patternviewconfig_setmovecursorwhenpaste(PatternViewConfig*, bool on);
void patternviewconfig_setdisplaysinglepattern(PatternViewConfig*, bool on);
bool patternviewconfig_issinglepatterndisplay(const PatternViewConfig*);
bool patternviewconfig_useheaderbitmap(const PatternViewConfig*);
bool patternviewconfig_showtrackscopes(const PatternViewConfig*);
void patternviewconfig_togglepatdefaultline(PatternViewConfig*);
bool patternviewconfig_issmoothscrolling(const PatternViewConfig*);
void patternviewconfig_setsmoothscrolling(PatternViewConfig*, bool on);
int patternviewconfig_patterndisplay(const PatternViewConfig*);
psy_ui_FontInfo patternviewconfig_readfont(PatternViewConfig*, double zoom);
/* events */
bool patternviewconfig_onchanged(PatternViewConfig*, psy_Property*);
bool patternviewconfig_onthemechanged(PatternViewConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWCONFIG_H */
