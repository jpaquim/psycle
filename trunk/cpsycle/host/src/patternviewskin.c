// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternviewskin.h"

// PatternViewSkin
// prototypes
psy_ui_Color patternviewskin_color(psy_Table* table, uintptr_t track, uintptr_t numtracks,
	psy_ui_Color source1, psy_ui_Color source2);
psy_ui_Color patternviewskin_calculatetrackcolor(uintptr_t track, uintptr_t numtracks,
	psy_ui_Color source1, psy_ui_Color source2);
// implementation
void patternviewskin_init(PatternViewSkin* self)
{
	psy_table_init(&self->trackseparatorcolors);
	psy_table_init(&self->row4beatcolors);
	psy_table_init(&self->rowbeatcolors);
	psy_table_init(&self->rowcolors);
	psy_table_init(&self->fontcolors);
	psy_table_init(&self->fontplaycolors);
	psy_table_init(&self->fontcurcolors);
	psy_table_init(&self->fontselcolors);
	psy_table_init(&self->selectioncolors);
	psy_table_init(&self->playbarcolors);
	psy_table_init(&self->cursorcolors);
	psy_table_init(&self->midlinecolors);
}

void patternviewskin_dispose(PatternViewSkin* self)
{
	psy_table_dispose(&self->trackseparatorcolors);
	psy_table_dispose(&self->row4beatcolors);
	psy_table_dispose(&self->rowbeatcolors);
	psy_table_dispose(&self->rowcolors);
	psy_table_dispose(&self->fontcolors);
	psy_table_dispose(&self->fontplaycolors);
	psy_table_dispose(&self->fontcurcolors);
	psy_table_dispose(&self->fontselcolors);
	psy_table_dispose(&self->selectioncolors);
	psy_table_dispose(&self->playbarcolors);
	psy_table_dispose(&self->cursorcolors);
	psy_table_dispose(&self->midlinecolors);
}

void patternviewskin_clear(PatternViewSkin* self)
{
	psy_table_clear(&self->trackseparatorcolors);
	psy_table_clear(&self->row4beatcolors);
	psy_table_clear(&self->rowbeatcolors);
	psy_table_clear(&self->rowcolors);
	psy_table_clear(&self->fontcolors);
	psy_table_clear(&self->fontplaycolors);
	psy_table_clear(&self->fontcurcolors);
	psy_table_clear(&self->fontselcolors);
	psy_table_clear(&self->selectioncolors);
	psy_table_clear(&self->playbarcolors);
	psy_table_clear(&self->cursorcolors);
	psy_table_clear(&self->midlinecolors);
}

psy_ui_Color patternviewskin_separatorcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->trackseparatorcolors, track, numtracks,
		self->separator, self->separator2);
}

psy_ui_Color patternviewskin_row4beatcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->row4beatcolors, track, numtracks,
		self->row4beat, self->row4beat2);
}

psy_ui_Color patternviewskin_rowbeatcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->rowbeatcolors, track, numtracks,
		self->rowbeat, self->rowbeat2);
}

psy_ui_Color patternviewskin_rowcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->rowcolors, track, numtracks,
		self->row, self->row2);
}

psy_ui_Color patternviewskin_fontcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->fontcolors, track, numtracks,
		self->font, self->font2);
}

psy_ui_Color patternviewskin_fontplaycolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->fontplaycolors, track, numtracks,
		self->fontPlay, self->fontPlay2);
}

psy_ui_Color patternviewskin_fontcurcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->fontcurcolors, track, numtracks,
		self->fontCur, self->fontCur2);
}

psy_ui_Color patternviewskin_fontselcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->fontselcolors, track, numtracks,
		self->fontSel, self->fontSel2);
}

psy_ui_Color patternviewskin_selectioncolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->selectioncolors, track, numtracks,
		self->selection, self->selection2);
}

psy_ui_Color patternviewskin_playbarcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->playbarcolors, track, numtracks,
		self->playbar, self->playbar2);
}

psy_ui_Color patternviewskin_cursorcolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->cursorcolors, track, numtracks,
		self->cursor, self->cursor2);
}

psy_ui_Color patternviewskin_midlinecolor(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_color(&self->midlinecolors, track, numtracks,
		self->midline, self->midline2);
}

psy_ui_Color patternviewskin_color(psy_Table* table, uintptr_t track, uintptr_t numtracks,
	psy_ui_Color source1, psy_ui_Color source2)
{
	psy_ui_Color rv;

	rv = (psy_ui_Color)(uintptr_t)psy_table_at(table, track);
	if (rv == 0 && !psy_table_exists(table, track)) {
		rv = patternviewskin_calculatetrackcolor(track, numtracks, source1, source2);
		psy_table_insert(table, track, (void*)(uintptr_t)rv);
	}
	return rv;
}

psy_ui_Color patternviewskin_calculatetrackcolor(uintptr_t track, uintptr_t numtracks,
	psy_ui_Color source1, psy_ui_Color source2)
{
	psy_ui_Color rv;
	float p0 = (float)((source1 >> 16) & 0xff);
	float p1 = (float)((source1 >> 8) & 0xff);
	float p2 = (float)(source1 & 0xff);

	float d0 = (float)((source2 >> 16) & 0xff);
	float d1 = (float)((source2 >> 8) & 0xff);
	float d2 = (float)(source2 & 0xff);

	int len = numtracks + 1;

	float a0 = (d0 - p0) / (len);
	float a1 = (d1 - p1) / (len);
	float a2 = (d2 - p2) / (len);

	p0 += (a0 * track);
	p1 += (a1 * track);
	p2 += (a2 * track);

	if (p0 < 0)
	{
		p0 = 0;
	} else if (p0 > 255)
	{
		p0 = 255;
	}

	if (p1 < 0)
	{
		p1 = 0;
	} else if (p1 > 255)
	{
		p1 = 255;
	}

	if (p2 < 0)
	{
		p2 = 2;
	} else if (p2 > 255)
	{
		p2 = 255;
	}
	rv = ((int32_t)(p0 * 0x10000) & 0xff0000)
		| ((int32_t)(p1 * 0x100) & 0xff00)
		| ((int32_t)(p2) & 0xff);
	return rv;
}
