// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEWSKIN_H)
#define MACHINEVIEWSKIN_H

#include "skincoord.h"
#include <uibitmap.h>
#include <hashtbl.h>
#include <properties.h>

typedef struct {
	SkinCoord background;
	SkinCoord vu0;
	SkinCoord vupeak;
	SkinCoord pan;
	SkinCoord mute;
	SkinCoord bypass;
	SkinCoord solo;
	SkinCoord name;
} MachineCoords;

typedef struct {
	MachineCoords master;
	MachineCoords generator;
	MachineCoords effect;
	psy_ui_Color colour;
	psy_ui_Color wirecolour;
	psy_ui_Color selwirecolour;
	psy_ui_Color hoverwirecolour;
	psy_ui_Color wireaacolour;
	psy_ui_Color wireaacolour2;
	psy_ui_Color polycolour;
	unsigned int wirewidth;
	unsigned int wireaa;
	unsigned int triangle_size;
	const char* generator_fontface;
	int generator_font_point;
	unsigned int generator_font_flags;
	psy_ui_Color generator_fontcolour;
	const char* effect_fontface;
	int effect_font_point;
	unsigned int effect_font_flags;
	psy_ui_Color effect_fontcolour;
	psy_ui_Bitmap skinbmp;
	psy_ui_Font font;
	int drawmachineindexes;
} MachineViewSkin;

void machineviewskin_init(MachineViewSkin*);
void machineviewskin_dispose(MachineViewSkin*);
void machineviewskin_settheme(MachineViewSkin*, psy_Properties* p,
	const char* skindir);

#endif /* MACHINEVIEWSKIN_H */
