// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternviewskin.h"
#include "skinio.h"
#include "resources/resource.h"

#include <dir.h>

#include <stdlib.h>

#include "../../detail/os.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

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
	psy_ui_bitmap_init(&self->bitmap);
	psy_ui_bitmap_loadresource(&self->bitmap, IDB_HEADERSKIN);
	patternviewskin_setclassicheadercoords(self);
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
	psy_ui_bitmap_dispose(&self->bitmap);
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

	rv = psy_ui_color_make((uintptr_t)psy_table_at(table, track));
	if (rv.value == 0 && !psy_table_exists(table, track)) {
		rv = patternviewskin_calculatetrackcolor(track, numtracks, source1, source2);
		psy_table_insert(table, track, (void*)(uintptr_t)rv.value);
	}
	return rv;
}

psy_ui_Color patternviewskin_calculatetrackcolor(uintptr_t track, uintptr_t numtracks,
	psy_ui_Color source1, psy_ui_Color source2)
{
	psy_ui_Color rv;
	float p0 = (float)((source1.value >> 16) & 0xff);
	float p1 = (float)((source1.value >> 8) & 0xff);
	float p2 = (float)(source1.value & 0xff);

	float d0 = (float)((source2.value >> 16) & 0xff);
	float d1 = (float)((source2.value >> 8) & 0xff);
	float d2 = (float)(source2.value & 0xff);

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
	rv = psy_ui_color_make(((int32_t)(p0 * 0x10000) & 0xff0000)
		| ((int32_t)(p1 * 0x100) & 0xff00)
		| ((int32_t)(p2) & 0xff));
	return rv;
}

void patternviewskin_settheme(PatternViewSkin* self, psy_Properties* p, const char* skindir)
{
	const char* pattern_header_skin_name;

	patternviewskin_clear(self);
	self->separator = psy_ui_color_make(psy_properties_int(p, "pvc_separator", 0x00292929));
	self->separator2 = psy_ui_color_make(psy_properties_int(p, "pvc_separator2", 0x00292929));
	self->background = psy_ui_color_make(psy_properties_int(p, "pvc_background", 0x00292929));
	self->background2 = psy_ui_color_make(psy_properties_int(p, "pvc_background2", 0x00292929));
	self->row4beat = psy_ui_color_make(psy_properties_int(p, "pvc_row4beat", 0x00595959));
	self->row4beat2 = psy_ui_color_make(psy_properties_int(p, "pvc_row4beat2", 0x00595959));
	self->rowbeat = psy_ui_color_make(psy_properties_int(p, "pvc_rowbeat", 0x00363636));
	self->rowbeat2 = psy_ui_color_make(psy_properties_int(p, "pvc_rowbeat2", 0x00363636));
	self->row = psy_ui_color_make(psy_properties_int(p, "pvc_row", 0x003E3E3E));
	self->row2 = psy_ui_color_make(psy_properties_int(p, "pvc_row2", 0x003E3E3E));
	self->font = psy_ui_color_make(psy_properties_int(p, "pvc_font", 0x00CACACA));
	self->font2 = psy_ui_color_make(psy_properties_int(p, "pvc_font2", 0x00CACACA));
	self->fontPlay = psy_ui_color_make(psy_properties_int(p, "pvc_fontplay", 0x00FFFFFF));
	self->fontCur2 = psy_ui_color_make(psy_properties_int(p, "pvc_fontcur2", 0x00FFFFFF));
	self->fontSel = psy_ui_color_make(psy_properties_int(p, "pvc_fontsel", 0x00FFFFFF));
	self->fontSel2 = psy_ui_color_make(psy_properties_int(p, "pvc_fontsel2", 0x00FFFFFF));
	self->selection = psy_ui_color_make(psy_properties_int(p, "pvc_selection", 0x009B7800));
	self->selection2 = psy_ui_color_make(psy_properties_int(p, "pvc_selection2", 0x009B7800));
	self->playbar = psy_ui_color_make(psy_properties_int(p, "pvc_playbar", 0x009F7B00));
	self->playbar2 = psy_ui_color_make(psy_properties_int(p, "pvc_playbar2", 0x009F7B00));
	self->cursor = psy_ui_color_make(psy_properties_int(p, "pvc_cursor", 0x009F7B00));
	self->cursor2 = psy_ui_color_make(psy_properties_int(p, "pvc_cursor2", 0x009F7B00));
	self->midline = psy_ui_color_make(psy_properties_int(p, "pvc_midline", 0x007D6100));
	self->midline2 = psy_ui_color_make(psy_properties_int(p, "pvc_midline2", 0x007D6100));
	pattern_header_skin_name = psy_properties_readstring(p, "pattern_header_skin",
		"");
	if (strcmp(pattern_header_skin_name, "") == 0) {
		psy_ui_bitmap_dispose(&self->bitmap);
		psy_ui_bitmap_init(&self->bitmap);
		psy_ui_bitmap_loadresource(&self->bitmap, IDB_HEADERSKIN);
		patternviewskin_setclassicheadercoords(self);
	} else if (pattern_header_skin_name) {
		char path[_MAX_PATH];
		char filename[_MAX_PATH];

		strcpy(filename, pattern_header_skin_name);
		strcat(filename, ".bmp");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_ui_Bitmap bmp;

			psy_ui_bitmap_init(&bmp);
			if (psy_ui_bitmap_load(&bmp, path) == 0) {
				psy_ui_bitmap_dispose(&self->bitmap);
				self->bitmap = bmp;
			}
		}
		strcpy(filename, pattern_header_skin_name);
		strcat(filename, ".psh");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_Properties* coords;

			coords = psy_properties_create();
			skin_loadpsh(coords, path);
			patternviewskin_setcoords(self, coords);
			properties_free(coords);
		}
	}
}

void patternviewskin_setclassicheadercoords(PatternViewSkin* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 79, 40, 17, 17, 66, 3, 17, 17, 0 };
	static SkinCoord solo = { 62, 40, 17, 17, 47, 3, 17, 17, 0 };
	static SkinCoord digitx0 = { 0, 23, 9, 17, 15, 3, 9, 17, 0 };
	static SkinCoord digit0x = { 0, 23, 9, 17, 22, 3, 9, 17, 0 };

	self->headercoords.background = background;
	self->headercoords.record = record;
	self->headercoords.mute = mute;
	self->headercoords.solo = solo;
	self->headercoords.digit0x = digit0x;
	self->headercoords.digitx0 = digitx0;
}

void patternviewskin_setheadercoords(PatternViewSkin* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 79, 40, 17, 17, 75, 66, 3, 17, 17 };
	static SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11, 0 };
	static SkinCoord digitx0 = { 0, 23, 9, 17, 15, 3, 9, 17, 0 };
	static SkinCoord digit0x = { 0, 23, 9, 17, 22, 3, 9, 17, 0 };

	self->headercoords.background = background;
	self->headercoords.record = record;
	self->headercoords.mute = mute;
	self->headercoords.solo = solo;
	self->headercoords.digit0x = digit0x;
	self->headercoords.digitx0 = digitx0;
}

void patternviewskin_setheadertextcoords(PatternViewSkin* self)
{
	SkinCoord background = { 2, 57, 103, 23, 0, 0, 103, 23, 0 };
	SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	SkinCoord mute = { 81, 18, 11, 11, 75, 3, 11, 11, 0 };
	SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11, 0 };
	SkinCoord digitx0 = { 0, 80, 6, 12, 5, 8, 6, 12, 0 };
	SkinCoord digit0x = { 0, 80, 6, 12, 11, 8, 6, 12, 0 };

	self->headercoords.background = background;
	self->headercoords.record = record;
	self->headercoords.mute = mute;
	self->headercoords.solo = solo;
	self->headercoords.digit0x = digit0x;
	self->headercoords.digitx0 = digitx0;
}

void patternviewskin_setcoords(PatternViewSkin* self, psy_Properties* p)
{
	const char* s;
	int vals[4];

	if (s = psy_properties_readstring(p, "background_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.background.srcx = vals[0];
		self->headercoords.background.srcy = vals[1];
		self->headercoords.background.destwidth = vals[2];
		self->headercoords.background.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "number_0_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.digitx0.srcx = vals[0];
		self->headercoords.digitx0.srcy = vals[1];
		self->headercoords.digit0x.srcx = vals[0];
		self->headercoords.digit0x.srcy = vals[1];
		self->headercoords.digitx0.srcwidth = vals[2];
		self->headercoords.digitx0.srcheight = vals[3];
		self->headercoords.digit0x.srcwidth = vals[2];
		self->headercoords.digit0x.srcheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "record_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.record.srcx = vals[0];
		self->headercoords.record.srcy = vals[1];
		self->headercoords.record.destwidth = vals[2];
		self->headercoords.record.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "mute_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.mute.srcx = vals[0];
		self->headercoords.mute.srcy = vals[1];
		self->headercoords.mute.destwidth = vals[2];
		self->headercoords.mute.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "solo_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.solo.srcx = vals[0];
		self->headercoords.solo.srcy = vals[1];
		self->headercoords.solo.destwidth = vals[2];
		self->headercoords.solo.destheight = vals[3];
	}
	if (s = psy_properties_readstring(p, "digit_x0_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.digitx0.destx = vals[0];
		self->headercoords.digitx0.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "digit_0x_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.digit0x.destx = vals[0];
		self->headercoords.digit0x.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "record_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.record.destx = vals[0];
		self->headercoords.record.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "mute_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.mute.destx = vals[0];
		self->headercoords.mute.desty = vals[1];
	}
	if (s = psy_properties_readstring(p, "solo_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.solo.destx = vals[0];
		self->headercoords.solo.desty = vals[1];
	}
}