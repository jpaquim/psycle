// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEWSKIN_H)
#define PATTERNVIEWSKIN_H

#include "skincoord.h"
#include <uibitmap.h>
#include <hashtbl.h>

typedef struct {
	SkinCoord background;	
	SkinCoord record;
	SkinCoord mute;
	SkinCoord solo;
	SkinCoord digitx0;
	SkinCoord digit0x;	
} TrackerHeaderCoords;

typedef struct {
	unsigned int separator;
	unsigned int separator2;
	unsigned int background;
	unsigned int background2;
	unsigned int row4beat;
	unsigned int row4beat2;
	unsigned int rowbeat;
	unsigned int rowbeat2;
	unsigned int row;
	unsigned int row2;
	unsigned int font;
	unsigned int font2;
	unsigned int fontPlay;
	unsigned int fontPlay2;
	unsigned int fontCur;
	unsigned int fontCur2;
	unsigned int fontSel;
	unsigned int fontSel2;
	unsigned int selection;
	unsigned int selection2;
	unsigned int playbar;
	unsigned int playbar2;
	unsigned int cursor;
	unsigned int cursor2;
	unsigned int midline;
	unsigned int midline2;
	TrackerHeaderCoords headercoords;
	psy_ui_Bitmap bitmap;
	psy_Table trackseparatorcolors;
	psy_Table row4beatcolors;
	psy_Table rowbeatcolors;
	psy_Table rowcolors;
	psy_Table fontcolors;
	psy_Table fontplaycolors;
	psy_Table fontcurcolors;
	psy_Table fontselcolors;
	psy_Table selectioncolors;
	psy_Table playbarcolors;
	psy_Table cursorcolors;
	psy_Table midlinecolors;
} PatternViewSkin;

void patternviewskin_init(PatternViewSkin*);
void patternviewskin_dispose(PatternViewSkin*);
void patternviewskin_clear(PatternViewSkin*);
psy_ui_Color patternviewskin_separatorcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_row4beatcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_rowbeatcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_rowcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_fontcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_fontplaycolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_fontcurcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_fontselcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_selectioncolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_playbarcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_cursorcolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Color patternviewskin_midlinecolor(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);

#endif /* PATTERNVIEWSKIN_H */
