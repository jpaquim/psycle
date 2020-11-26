// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "resources/resource.h"

#include "machineviewskin.h"
#include "skinio.h"

#include <dir.h>

#include <stdlib.h>

#include "../../detail/os.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

// MachineViewSkin
// prototypes
static void machineviewskin_initdefaultmachinecoords(MachineViewSkin*);
static void machineviewskin_setcoords(MachineViewSkin*, psy_Property* p);
// implementation
void machineviewskin_init(MachineViewSkin* self)
{
	psy_ui_FontInfo fontinfo;

	psy_ui_bitmap_init(&self->skinbmp);
	psy_ui_bitmap_loadresource(&self->skinbmp, IDB_MACHINESKIN);
	psy_ui_fontinfo_init(&fontinfo, "Tahoma", 16);
	psy_ui_font_init(&self->font, &fontinfo);
	machineviewskin_initdefaultmachinecoords(self);
}

void machineviewskin_dispose(MachineViewSkin* self)
{
	psy_ui_font_dispose(&self->font);
	psy_ui_bitmap_dispose(&self->skinbmp);
}

void machineviewskin_initdefaultmachinecoords(MachineViewSkin* self)
{
	MachineCoords master = {
		{ 0, 52, 138, 35, 0, 0, 138, 35, 0 },		// background
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 }
	};
	MachineCoords generator = {
		{ 0, 87, 138, 52, 0, 0, 138, 52, 0 },		// background
		{ 0, 156, 0, 7, 4, 20, 129, 7, 129},		// vu0
		{ 108, 156, 1, 7, 4, 20, 1, 7, 82 },		// vupeak
		{ 0, 139, 6, 13, 6, 33, 6, 13, 82 },		// pan
		{ 23, 139, 17, 17, 117, 31, 17, 17, 0 },	// mute
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// bypass
		{ 6, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name
	};
	MachineCoords effect = {
		{ 0, 0, 138, 52, 0, 0, 138, 52, 0 },		// background
		{ 0, 163, 0, 7, 4, 20, 129, 7, 129 },		// vu0
		{ 96, 144, 1, 7, 4, 20, 1, 7, 0 },			// vupeak
		{ 57, 139, 6, 13, 6, 33, 6, 13, 82 },		// pan
		{ 23, 139, 17, 17, 117, 31, 17, 17, 0 },	// mute
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// bypass
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name 
	};
	self->master = master;
	self->generator = generator;
	self->effect = effect;
}

void machineviewskin_settheme(MachineViewSkin* self, psy_Property* p, const char* skindir)
{
	const char* machine_skin_name;

	if (p) {
		self->colour = psy_ui_colour_make(psy_property_at_int(p, "mv_colour", 0x00232323));
		self->wirecolour = psy_ui_colour_make(psy_property_at_int(p, "mv_wirecolour", 0x005F5F5F));
		self->selwirecolour = psy_ui_colour_make(psy_property_at_int(p, "mv_selwirecolour", 0x007F7F7F));
		self->hoverwirecolour = psy_ui_colour_make(psy_property_at_int(p, "mv_hoverwirecolour", 0x007F7F7F));
		self->polycolour = psy_ui_colour_make(psy_property_at_int(p, "mv_wireaacolour2", 0x005F5F5F));
		self->polycolour = psy_ui_colour_make(psy_property_at_int(p, "mv_polycolour", 0x00B1C8B0));
		self->generator_fontcolour =
			psy_ui_colour_make(psy_property_at_int(p, "mv_generator_fontcolour", 0x00B1C8B0));
		self->effect_fontcolour =
			psy_ui_colour_make(psy_property_at_int(p, "mv_effect_fontcolour", 0x00D1C5B6));
		self->triangle_size = psy_property_at_int(p, "mv_triangle_size", 10);
		machine_skin_name = psy_property_at_str(p, "machine_skin", 0);
	} else {
		self->colour = psy_ui_colour_make(0x00232323);
		self->wirecolour = psy_ui_colour_make(0x005F5F5F);
		self->selwirecolour = psy_ui_colour_make(0x007F7F7F);
		self->hoverwirecolour = psy_ui_colour_make(0x007F7F7F);
		self->polycolour = psy_ui_colour_make(0x005F5F5F);
		self->polycolour = psy_ui_colour_make(0x00B1C8B0);
		self->generator_fontcolour = psy_ui_colour_make(0x00B1C8B0);
		self->effect_fontcolour = psy_ui_colour_make(0x00D1C5B6);
		self->triangle_size = 10;
		machine_skin_name = NULL;
	}
	self->wireaacolour =
		psy_ui_colour_make(((((self->wirecolour.value & 0x00ff0000)
			+ ((self->colour.value & 0x00ff0000) * 4)) / 5) & 0x00ff0000) +
			((((self->wirecolour.value & 0x00ff00)
				+ ((self->colour.value & 0x00ff00) * 4)) / 5) & 0x00ff00) +
			((((self->wirecolour.value & 0x00ff)
				+ ((self->colour.value & 0x00ff) * 4)) / 5) & 0x00ff));
	self->wireaacolour2 =
		psy_ui_colour_make((((((self->wirecolour.value & 0x00ff0000))
			+ ((self->colour.value & 0x00ff0000))) / 2) & 0x00ff0000) +
			(((((self->wirecolour.value & 0x00ff00))
				+ ((self->colour.value & 0x00ff00))) / 2) & 0x00ff00) +
			(((((self->wirecolour.value & 0x00ff))
				+ ((self->colour.value & 0x00ff))) / 2) & 0x00ff));
	
	if (machine_skin_name && strlen(machine_skin_name)) {
		char path[_MAX_PATH];
		char filename[_MAX_PATH];

		strcpy(filename, machine_skin_name);
		strcat(filename, ".bmp");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_ui_Bitmap bmp;

			psy_ui_bitmap_init(&bmp);
			if (psy_ui_bitmap_load(&bmp, path) == 0) {
				psy_ui_bitmap_dispose(&self->skinbmp);
				self->skinbmp = bmp;
			}
		}
		strcpy(filename, machine_skin_name);
		strcat(filename, ".psm");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_Property* coords;

			coords = psy_property_allocinit_key(NULL);
			skin_loadpsh(coords, path);
			machineviewskin_setcoords(self, coords);
			psy_property_deallocate(coords);
		}
	} else {
		psy_ui_bitmap_dispose(&self->skinbmp);
		psy_ui_bitmap_init(&self->skinbmp);
		psy_ui_bitmap_loadresource(&self->skinbmp, IDB_MACHINESKIN);
		machineviewskin_initdefaultmachinecoords(self);
	}
}

void machineviewskin_setcoords(MachineViewSkin* self, psy_Property* p)
{
	const char* s;
	int vals[4];

	// master
	if (s = psy_property_at_str(p, "master_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->master.background, vals);
	}
	// generator
	if (s = psy_property_at_str(p, "generator_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->generator.background, vals);
	}
	if (s = psy_property_at_str(p, "generator_vu0_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->generator.vu0, vals);
	}
	if (s = psy_property_at_str(p, "generator_vu_peak_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->generator.vupeak, vals);
	}
	if (s = psy_property_at_str(p, "generator_pan_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->generator.pan, vals);
	}
	if (s = psy_property_at_str(p, "generator_mute_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->generator.mute, vals);
	}
	if (s = psy_property_at_str(p, "generator_solo_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->generator.solo, vals);
	}
	if (s = psy_property_at_str(p, "generator_vu_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->generator.vu0, vals);
		self->generator.vu0.destwidth = vals[2];
	}
	if (s = psy_property_at_str(p, "generator_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->generator.pan, vals);
	}
	if (s = psy_property_at_str(p, "generator_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->generator.mute, vals);
	}
	if (s = psy_property_at_str(p, "generator_solo_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->generator.solo, vals);
	}
	if (s = psy_property_at_str(p, "generator_name_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->generator.name, vals);
	}
	// effect
	if (s = psy_property_at_str(p, "effect_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->effect.background, vals);
	}
	if (s = psy_property_at_str(p, "effect_vu0_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->effect.vu0, vals);
	}
	if (s = psy_property_at_str(p, "effect_vu_peak_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->effect.vupeak, vals);
	}
	if (s = psy_property_at_str(p, "effect_pan_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->effect.pan, vals);
	}
	if (s = psy_property_at_str(p, "effect_mute_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->effect.mute, vals);
	}
	if (s = psy_property_at_str(p, "effect_bypass_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->effect.bypass, vals);
	}
	if (s = psy_property_at_str(p, "effect_vu_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->effect.vu0, vals);
		self->generator.vu0.destwidth = vals[2];
	}
	if (s = psy_property_at_str(p, "effect_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->effect.pan, vals);
	}
	if (s = psy_property_at_str(p, "effect_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->effect.mute, vals);
	}
	if (s = psy_property_at_str(p, "effect_bypass_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->effect.bypass, vals);
	}
	if (s = psy_property_at_str(p, "effect_name_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->effect.name, vals);
	}
}