// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "patternviewskin.h"
#include "skinio.h"
#include "resources/resource.h"
// file
#include <dir.h>
// platform
#include "../../detail/os.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

// PatternViewSkin
// prototypes
psy_ui_Colour patternviewskin_colour(psy_Table* table, uintptr_t track, uintptr_t numtracks,
	psy_ui_Colour source1, psy_ui_Colour source2);
psy_ui_Colour patternviewskin_calculatetrackcolour(uintptr_t track, uintptr_t numtracks,
	psy_ui_Colour source1, psy_ui_Colour source2);
// implementation
void patternviewskin_init(PatternViewSkin* self)
{
	psy_table_init(&self->trackseparatorcolours);
	psy_table_init(&self->row4beatcolours);
	psy_table_init(&self->rowbeatcolours);
	psy_table_init(&self->rowcolours);
	psy_table_init(&self->fontcolours);
	psy_table_init(&self->fontplaycolours);
	psy_table_init(&self->fontcurcolours);
	psy_table_init(&self->fontselcolours);
	psy_table_init(&self->selectioncolours);
	psy_table_init(&self->selectionbeatcolours);
	psy_table_init(&self->selection4beatcolours);
	psy_table_init(&self->playbarcolours);
	psy_table_init(&self->cursorcolours);
	psy_table_init(&self->midlinecolours);	
	psy_table_init(&self->eventcolours);
	psy_table_init(&self->eventhovercolours);
	psy_table_init(&self->eventcurrchannelcolours);
	psy_ui_bitmap_init(&self->bitmap);
	psy_ui_bitmap_loadresource(&self->bitmap, IDB_HEADERSKIN);
	patternviewskin_setclassicheadercoords(self);
}

void patternviewskin_dispose(PatternViewSkin* self)
{
	psy_table_dispose(&self->trackseparatorcolours);
	psy_table_dispose(&self->row4beatcolours);
	psy_table_dispose(&self->rowbeatcolours);
	psy_table_dispose(&self->rowcolours);
	psy_table_dispose(&self->fontcolours);
	psy_table_dispose(&self->fontplaycolours);
	psy_table_dispose(&self->fontcurcolours);
	psy_table_dispose(&self->fontselcolours);
	psy_table_dispose(&self->selectioncolours);
	psy_table_dispose(&self->playbarcolours);
	psy_table_dispose(&self->cursorcolours);
	psy_table_dispose(&self->midlinecolours);
	psy_table_dispose(&self->eventcolours);
	psy_table_dispose(&self->eventhovercolours);
	psy_table_dispose(&self->eventcurrchannelcolours);
	psy_ui_bitmap_dispose(&self->bitmap);
}

void patternviewskin_clearcache(PatternViewSkin* self)
{
	psy_table_clear(&self->trackseparatorcolours);
	psy_table_clear(&self->row4beatcolours);
	psy_table_clear(&self->rowbeatcolours);
	psy_table_clear(&self->rowcolours);
	psy_table_clear(&self->fontcolours);
	psy_table_clear(&self->fontplaycolours);
	psy_table_clear(&self->fontcurcolours);
	psy_table_clear(&self->fontselcolours);
	psy_table_clear(&self->selectioncolours);
	psy_table_clear(&self->selectionbeatcolours);
	psy_table_clear(&self->selection4beatcolours);
	psy_table_clear(&self->playbarcolours);
	psy_table_clear(&self->cursorcolours);
	psy_table_clear(&self->midlinecolours);
	psy_table_clear(&self->eventcolours);
	psy_table_clear(&self->eventhovercolours);
	psy_table_clear(&self->eventcurrchannelcolours);
}

psy_ui_Colour patternviewskin_backgroundcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{	
	return patternviewskin_colour(&self->trackseparatorcolours, track, numtracks,
		self->background, self->background2);
}

psy_ui_Colour patternviewskin_separatorcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->trackseparatorcolours, track, numtracks,
		self->separator, self->separator2);
}

psy_ui_Colour patternviewskin_row4beatcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->row4beatcolours, track, numtracks,
		self->row4beat, self->row4beat2);
}

psy_ui_Colour patternviewskin_rowbeatcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->rowbeatcolours, track, numtracks,
		self->rowbeat, self->rowbeat2);
}

psy_ui_Colour patternviewskin_rowcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->rowcolours, track, numtracks,
		self->row, self->row2);
}

psy_ui_Colour patternviewskin_fontcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->fontcolours, track, numtracks,
		self->font, self->font2);
}

psy_ui_Colour patternviewskin_fontplaycolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->fontplaycolours, track, numtracks,
		self->fontplay, self->fontplay2);
}

psy_ui_Colour patternviewskin_fontcurcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->fontcurcolours, track, numtracks,
		self->fontcur, self->fontcur2);
}

psy_ui_Colour patternviewskin_fontselcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->fontselcolours, track, numtracks,
		self->fontsel, self->fontsel2);
}

psy_ui_Colour patternviewskin_selectioncolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{	
	return patternviewskin_colour(&self->selectioncolours, track, numtracks,
		self->selection, self->selection2);
}

psy_ui_Colour patternviewskin_selectionbeatcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{		
	return patternviewskin_colour(&self->selectionbeatcolours, track, numtracks,
		self->selectionbeat, self->selectionbeat2);
}

psy_ui_Colour patternviewskin_selection4beatcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->selection4beatcolours, track, numtracks,
		self->selection4beat, self->selection4beat2);
}

psy_ui_Colour patternviewskin_playbarcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->playbarcolours, track, numtracks,
		self->playbar, self->playbar2);
}

psy_ui_Colour patternviewskin_cursorcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->cursorcolours, track, numtracks,
		self->cursor, self->cursor2);
}

psy_ui_Colour patternviewskin_midlinecolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return patternviewskin_colour(&self->midlinecolours, track, numtracks,
		self->midline, self->midline2);
}

psy_ui_Colour patternviewskin_keywhitecolour(PatternViewSkin* self)
{	
	return self->keywhite;
}

psy_ui_Colour patternviewskin_keyblackcolour(PatternViewSkin* self)
{
	return self->keyblack;
}

psy_ui_Colour patternviewskin_keyseparatorcolour(PatternViewSkin* self)
{
	return self->keyseparator;
}

psy_ui_Colour patternviewskin_eventcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return self->event;
}

psy_ui_Colour patternviewskin_eventhovercolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return self->eventhover;
}

psy_ui_Colour patternviewskin_eventcurrchannelcolour(PatternViewSkin* self, uintptr_t track, uintptr_t numtracks)
{
	return self->eventcurrchannel;
}

psy_ui_Colour patternviewskin_colour(psy_Table* table, uintptr_t track, uintptr_t numtracks,
	psy_ui_Colour source1, psy_ui_Colour source2)
{
	psy_ui_Colour rv;

	if (psy_ui_equal_colours(source1, source2)) {
		return source1;
	}
	if (numtracks < 2) {
		return source1;
	} else if (numtracks == 2) {
		if (track == 0) {
			return source1;
		}
		return source2;
	}	
	if (!psy_table_exists(table, track) || (psy_table_size(table) !=
			numtracks)) {
		rv = patternviewskin_calculatetrackcolour(track, numtracks, source1, source2);
		psy_table_insert(table, track, (void*)(uintptr_t)rv.value);
	} else {
		rv = psy_ui_colour_make((uint32_t)(uintptr_t)psy_table_at(table, track));
	}
	return rv;
}

psy_ui_Colour patternviewskin_calculatetrackcolour(uintptr_t track, uintptr_t numtracks,
	psy_ui_Colour source1, psy_ui_Colour source2)
{
	psy_ui_Colour rv;
	float p0 = (float)((source1.value >> 16) & 0xff);
	float p1 = (float)((source1.value >> 8) & 0xff);
	float p2 = (float)(source1.value & 0xff);

	float d0 = (float)((source2.value >> 16) & 0xff);
	float d1 = (float)((source2.value >> 8) & 0xff);
	float d2 = (float)(source2.value & 0xff);

	uintptr_t len = numtracks + 1;

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
	rv = psy_ui_colour_make(((int32_t)(p0 * 0x10000) & 0xff0000)
		| ((int32_t)(p1 * 0x100) & 0xff00)
		| ((int32_t)(p2) & 0xff));
	return rv;
}

void patternviewskin_settheme(PatternViewSkin* self, psy_Property* p, const char* skindir)
{
	const char* pattern_header_skin_name;	

	patternviewskin_clearcache(self);
	self->separator = psy_ui_colour_make(psy_property_at_colour(p, "pvc_separator", 0x00292929));
	self->separator2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_separator2", 0x00292929));
	self->background = psy_ui_colour_make(psy_property_at_colour(p, "pvc_background", 0x00292929));
	self->background2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_background2", 0x00292929));
	self->row4beat = psy_ui_colour_make(psy_property_at_colour(p, "pvc_row4beat", 0x00595959));
	self->row4beat2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_row4beat2", 0x00595959));
	self->rowbeat = psy_ui_colour_make(psy_property_at_colour(p, "pvc_rowbeat", 0x00363636));
	self->rowbeat2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_rowbeat2", 0x00363636));
	self->row = psy_ui_colour_make(psy_property_at_colour(p, "pvc_row", 0x003E3E3E));
	self->row2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_row2", 0x003E3E3E));
	self->font = psy_ui_colour_make(psy_property_at_colour(p, "pvc_font", 0x00CACACA));
	self->font2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_font2", 0x00CACACA));
	self->fontplay = psy_ui_colour_make(psy_property_at_colour(p, "pvc_fontplay", 0x00ffffff));
	self->fontcur = psy_ui_colour_make(psy_property_at_colour(p, "pvc_fontcur", 0x00ffffff));
	self->fontcur2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_fontcur2", 0x00ffffff));
	self->fontsel = psy_ui_colour_make(psy_property_at_colour(p, "pvc_fontsel", 0x00ffffff));
	self->fontsel2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_fontsel2", 0x00ffffff));
	self->selection = psy_ui_colour_make(psy_property_at_colour(p, "pvc_selection", 0x009B7800));
	self->selection2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_selection2", 0x009B7800));
	self->playbar = psy_ui_colour_make(psy_property_at_colour(p, "pvc_playbar", 0x009F7B00));
	self->playbar2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_playbar2", 0x009F7B00));
	self->cursor = psy_ui_colour_make(psy_property_at_colour(p, "pvc_cursor", 0x009F7B00));
	self->cursor2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_cursor2", 0x009F7B00));
	self->midline = psy_ui_colour_make(psy_property_at_colour(p, "pvc_midline", 0x007D6100));
	self->midline2 = psy_ui_colour_make(psy_property_at_colour(p, "pvc_midline2", 0x007D6100));
	// selection
	self->selectionbeat = psy_ui_diffadd_colours(self->row, self->rowbeat, self->selection);
	self->selectionbeat2 = psy_ui_diffadd_colours(self->row2, self->rowbeat2, self->selection2);
	self->selection4beat = psy_ui_diffadd_colours(self->row, self->row4beat, self->selection);
	self->selection4beat2 = psy_ui_diffadd_colours(self->row2, self->row4beat2, self->selection2);	
	// colours not part of current skin format
	self->keyblack = psy_ui_colour_make(0x00595959);
	self->keywhite = psy_ui_colour_make(0x00C0C0C0);	
	self->keyseparator = psy_ui_colour_make(0x999999);	
	self->event = psy_ui_colour_make(0x00999999);
	self->eventhover = psy_ui_colour_make(0x00DADADA);
	self->eventcurrchannel = psy_ui_colour_make(0x00CACACA);	
	pattern_header_skin_name = psy_property_at_str(p, "pattern_header_skin",
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
				self->bitmap = bmp;
			}			
		}
		strcpy(filename, pattern_header_skin_name);
		strcat(filename, ".psh");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_Property* coords;

			coords = psy_property_allocinit_key(NULL);
			skin_loadpsh(coords, path);
			patternviewskin_setcoords(self, coords);
			psy_property_deallocate(coords);
		}
	}
}

void patternviewskin_setclassicheadercoords(PatternViewSkin* self)
{
	skincoord_init_all(&self->headercoords.background, 2, 0, 102, 23, 0, 0, 102, 23, 0);
	skincoord_init_all(&self->headercoords.record, 96, 40, 17, 17, 85, 3, 17, 17, 0);
	skincoord_init_all(&self->headercoords.mute, 79, 40, 17, 17, 66, 3, 17, 17, 0);
	skincoord_init_all(&self->headercoords.solo, 62, 40, 17, 17, 47, 3, 17, 17, 0);
	skincoord_init_all(&self->headercoords.digitx0, 0, 23, 9, 17, 15, 3, 9, 17, 0);
	skincoord_init_all(&self->headercoords.digit0x, 0, 23, 9, 17, 22, 3, 9, 17, 0);
	skincoord_init_all(&self->headercoords.dplayon, 0, 40, 7, 17, 5, 3, 7, 17, 0);	
}

void patternviewskin_setheadercoords(PatternViewSkin* self)
{
	skincoord_init_all(&self->headercoords.background, 2, 0, 102, 23, 0, 0, 102, 23, 0);
	skincoord_init_all(&self->headercoords.record, 0, 18, 7, 12, 52, 3, 7, 12, 0);
	skincoord_init_all(&self->headercoords.mute, 79, 40, 17, 17, 75, 66, 3, 17, 17);
	skincoord_init_all(&self->headercoords.solo, 92, 18, 11, 11, 97, 3, 11, 11, 0);
	skincoord_init_all(&self->headercoords.digitx0, 0, 23, 9, 17, 15, 3, 9, 17, 0);
	skincoord_init_all(&self->headercoords.digit0x, 0, 23, 9, 17, 22, 3, 9, 17, 0);
	skincoord_init_all(&self->headercoords.dplayon, 0, 40, 7, 17, 5, 3, 7, 17, 0);	
}

void patternviewskin_setheadertextcoords(PatternViewSkin* self)
{
	skincoord_init_all(&self->headercoords.background, 2, 57, 103, 23, 0, 0, 103, 23, 0);
	skincoord_init_all(&self->headercoords.record, 0, 18, 7, 12, 52, 3, 7, 12, 0);
	skincoord_init_all(&self->headercoords.mute, 81, 18, 11, 11, 75, 3, 11, 11, 0);
	skincoord_init_all(&self->headercoords.solo, 92, 18, 11, 11, 97, 3, 11, 11, 0);
	skincoord_init_all(&self->headercoords.digitx0, 0, 80, 6, 12, 5, 8, 6, 12, 0);
	skincoord_init_all(&self->headercoords.digit0x, 0, 80, 6, 12, 11, 8, 6, 12, 0);
}

void patternviewskin_setcoords(PatternViewSkin* self, psy_Property* p)
{
	const char* s;
	intptr_t vals[4];

	if (s = psy_property_at_str(p, "background_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.background.src.left = (double)vals[0];
		self->headercoords.background.src.top = (double)vals[1];
		psy_ui_realrectangle_setwidth(&self->headercoords.background.dest,
			(double)vals[2]);
		psy_ui_realrectangle_setheight(&self->headercoords.background.dest,
			(double)vals[3]);
	}
	if (s = psy_property_at_str(p, "number_0_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.digitx0.src.left = (double)vals[0];
		self->headercoords.digitx0.src.top = (double)vals[1];
		self->headercoords.digit0x.src.left = (double)vals[0];
		self->headercoords.digit0x.src.top = (double)vals[1];
		psy_ui_realrectangle_setwidth(&self->headercoords.digitx0.src, (double)vals[2]);
		psy_ui_realrectangle_setheight(&self->headercoords.digitx0.src, (double)vals[3]);
		psy_ui_realrectangle_setwidth(&self->headercoords.digit0x.src, (double)vals[2]);
		psy_ui_realrectangle_setheight(&self->headercoords.digit0x.src, (double)vals[3]);
	}
	if (s = psy_property_at_str(p, "record_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.record.src.left = (double)vals[0];
		self->headercoords.record.src.top = (double)vals[1];
		psy_ui_realrectangle_setwidth(&self->headercoords.record.dest, (double)vals[2]);
		psy_ui_realrectangle_setheight(&self->headercoords.record.dest, (double)vals[3]);
	}
	if (s = psy_property_at_str(p, "mute_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.mute.src.left = (double)vals[0];
		self->headercoords.mute.src.top = (double)vals[1];
		psy_ui_realrectangle_setwidth(&self->headercoords.mute.dest, (double)vals[2]);
		psy_ui_realrectangle_setheight(&self->headercoords.mute.dest, (double)vals[3]);
	}
	if (s = psy_property_at_str(p, "solo_on_source", 0)) {
		skin_psh_values(s, 4, vals);
		self->headercoords.solo.src.left = (double)vals[0];
		self->headercoords.solo.src.top = (double)vals[1];
		psy_ui_realrectangle_setwidth(&self->headercoords.solo.dest, (double)vals[2]);
		psy_ui_realrectangle_setheight(&self->headercoords.solo.dest, (double)vals[3]);
	}
	if (s = psy_property_at_str(p, "digit_x0_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.digitx0.dest.left = (double)vals[0];
		self->headercoords.digitx0.dest.top = (double)vals[1];
	}
	if (s = psy_property_at_str(p, "digit_0x_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.digit0x.dest.left = (double)vals[0];
		self->headercoords.digit0x.dest.top = (double)vals[1];
	}
	if (s = psy_property_at_str(p, "record_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.record.dest.left = (double)vals[0];
		self->headercoords.record.dest.top = (double)vals[1];
	}
	if (s = psy_property_at_str(p, "mute_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.mute.dest.left = (double)vals[0];
		self->headercoords.mute.dest.top = (double)vals[1];
	}
	if (s = psy_property_at_str(p, "solo_on_dest", 0)) {
		skin_psh_values(s, 2, vals);
		self->headercoords.solo.dest.left = (double)vals[0];
		self->headercoords.solo.dest.top = (double)vals[1];
	}
}
