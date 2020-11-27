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
	psy_ui_Colour separator;
	psy_ui_Colour separator2;
	psy_ui_Colour background;
	psy_ui_Colour background2;
	psy_ui_Colour row4beat;
	psy_ui_Colour row4beat2;
	psy_ui_Colour rowbeat;
	psy_ui_Colour rowbeat2;
	psy_ui_Colour row;
	psy_ui_Colour row2;
	psy_ui_Colour font;
	psy_ui_Colour font2;
	psy_ui_Colour fontplay;
	psy_ui_Colour fontplay2;
	psy_ui_Colour fontcur;
	psy_ui_Colour fontcur2;
	psy_ui_Colour fontsel;
	psy_ui_Colour fontsel2;
	psy_ui_Colour selection;
	psy_ui_Colour selection2;
	psy_ui_Colour playbar;
	psy_ui_Colour playbar2;
	psy_ui_Colour cursor;
	psy_ui_Colour cursor2;
	psy_ui_Colour midline;
	psy_ui_Colour midline2;
	// additional pianoroll colours {
	psy_ui_Colour keywhite;
	psy_ui_Colour keyblack;
	psy_ui_Colour keyseparator;
	psy_ui_Colour event;
	psy_ui_Colour eventhover;
	psy_ui_Colour eventcurrchannel;
	// }
	TrackerHeaderCoords headercoords;
	psy_ui_Bitmap bitmap;
	// colour/track cache with range of numtracks between two colours
    // (e.g font and font2 define the gradient range)
	psy_Table trackseparatorcolours;
	psy_Table row4beatcolours;
	psy_Table rowbeatcolours;
	psy_Table rowcolours;
	psy_Table fontcolours;
	psy_Table fontplaycolours;
	psy_Table fontcurcolours;
	psy_Table fontselcolours;
	psy_Table selectioncolours;
	psy_Table playbarcolours;
	psy_Table cursorcolours;
	psy_Table midlinecolours;
	psy_Table eventcolours;
	psy_Table eventhovercolours;
	psy_Table eventcurrchannelcolours;
} PatternViewSkin;

void patternviewskin_init(PatternViewSkin*);
void patternviewskin_dispose(PatternViewSkin*);
void patternviewskin_clear(PatternViewSkin*);
void patternviewskin_settheme(PatternViewSkin*, psy_Property*, const char* skindir);
void patternviewskin_setclassicheadercoords(PatternViewSkin*);
void trackerview_setheadercoords(PatternViewSkin*);
void patternviewskin_setheadertextcoords(PatternViewSkin*);
void patternviewskin_setcoords(PatternViewSkin*, psy_Property*);
psy_ui_Colour patternviewskin_separatorcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_row4beatcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_rowbeatcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_rowcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_fontcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_fontplaycolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_fontcurcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_fontselcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_selectioncolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_playbarcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_cursorcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_midlinecolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
// pianoroll
psy_ui_Colour patternviewskin_keywhitecolour(PatternViewSkin*);
psy_ui_Colour patternviewskin_keyblackcolour(PatternViewSkin*);
psy_ui_Colour patternviewskin_keyseparatorcolour(PatternViewSkin*);
psy_ui_Colour patternviewskin_eventcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_eventhovercolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);
psy_ui_Colour patternviewskin_eventcurrchannelcolour(PatternViewSkin*, uintptr_t track, uintptr_t numtracks);

#endif /* PATTERNVIEWSKIN_H */
