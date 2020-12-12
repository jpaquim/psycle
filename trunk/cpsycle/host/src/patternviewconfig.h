// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEWCONFIG_H)
#define PATTERNVIEWCONFIG_H

// dsp
#include <notestab.h>
// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// PatternViewConfig

enum {
	PROPERTY_ID_PATTERNDISPLAY = 7000,
	PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER,
	PROPERTY_ID_PATTERN_DISPLAYMODE_PIANOROLL,
	PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,
	PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL,
	PROPERTY_ID_TRACKSCOPES
};

typedef struct PatternViewConfig {
	psy_Property* patternview;
	psy_Property* theme;
	// references
	psy_Property* parent;
	
} PatternViewConfig;

void patternviewconfig_init(PatternViewConfig*, psy_Property* parent);
void patternviewconfig_maketheme(PatternViewConfig* self, psy_Property* parent);

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
bool patternviewconfig_showtrackscopes(const PatternViewConfig*);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWCONFIG_H */
