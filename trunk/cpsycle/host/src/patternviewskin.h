// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEWSKIN_H)
#define PATTERNVIEWSKIN_H

#include "skincoord.h"
#include <uibitmap.h>
#include <hashtbl.h>
#include <properties.h>

typedef struct {
	SkinCoord background;	
	SkinCoord record;
	SkinCoord mute;
	SkinCoord solo;
	SkinCoord digitx0;
	SkinCoord digit0x;	
} TrackerHeaderCoords;

typedef struct {
	psy_ui_Color separator;
	psy_ui_Color separator2;
	psy_ui_Color background;
	psy_ui_Color background2;
	psy_ui_Color row4beat;
	psy_ui_Color row4beat2;
	psy_ui_Color rowbeat;
	psy_ui_Color rowbeat2;
	psy_ui_Color row;
	psy_ui_Color row2;
	psy_ui_Color font;
	psy_ui_Color font2;
	psy_ui_Color fontPlay;
	psy_ui_Color fontPlay2;
	psy_ui_Color fontCur;
	psy_ui_Color fontCur2;
	psy_ui_Color fontSel;
	psy_ui_Color fontSel2;
	psy_ui_Color selection;
	psy_ui_Color selection2;
	psy_ui_Color playbar;
	psy_ui_Color playbar2;
	psy_ui_Color cursor;
	psy_ui_Color cursor2;
	psy_ui_Color midline;
	psy_ui_Color midline2;
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
void patternviewskin_settheme(PatternViewSkin*, psy_Properties*, const char* skindir);
void patternviewskin_setclassicheadercoords(PatternViewSkin*);
void trackerview_setheadercoords(PatternViewSkin*);
void patternviewskin_setheadertextcoords(PatternViewSkin*);
void patternviewskin_setcoords(PatternViewSkin*, psy_Properties*);
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
