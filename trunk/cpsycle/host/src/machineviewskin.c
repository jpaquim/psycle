// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

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

void machinecoords_init(MachineCoords* self)
{
	skincoord_init(&self->background);
	skincoord_init(&self->vu0);
	skincoord_init(&self->vupeak);
	skincoord_init(&self->pan);
	skincoord_init(&self->mute);
	skincoord_init(&self->bypass);
	skincoord_init(&self->solo);
	skincoord_init(&self->name);
}

// MachineViewSkin
// prototypes
static void machineviewskin_initdefaultmachinecoords(MachineViewSkin*);
static void machineviewskin_setcoords(MachineViewSkin*, psy_Property* p);
// implementation
void machineviewskin_init(MachineViewSkin* self, psy_Property* theme,
	const char* skindir)
{
	psy_ui_FontInfo fontinfo;

	psy_ui_bitmap_init(&self->skinbmp);
	psy_ui_bitmap_loadresource(&self->skinbmp, IDB_MACHINESKIN);
	psy_ui_fontinfo_init(&fontinfo, "Tahoma", 16);
	psy_ui_font_init(&self->font, &fontinfo);
	machineviewskin_initdefaultmachinecoords(self);
	self->drawmachineindexes = TRUE;
	self->drawvumeters = TRUE;
	self->hastransparency = FALSE;
	machineviewskin_settheme(self, theme, skindir);
}

void machineviewskin_dispose(MachineViewSkin* self)
{
	psy_ui_font_dispose(&self->font);
	psy_ui_bitmap_dispose(&self->skinbmp);
}

void machineviewskin_initdefaultmachinecoords(MachineViewSkin* self)
{	
	// Master
	machinecoords_init(&self->master);
	skincoord_init_all(&self->master.background, 0, 52, 138, 35, 0, 0, 138, 35, 0);
	// Generator
	skincoord_init_all(&self->generator.background, 0, 87, 138, 52, 0, 0, 138, 52, 0);
	skincoord_init_all(&self->generator.vu0,  0, 156, 0, 7, 4, 20, 129, 7, 129);
	skincoord_init_all(&self->generator.vupeak, 108, 156, 1, 7, 4, 20, 1, 7, 82);
	skincoord_init_all(&self->generator.pan, 0, 139, 6, 13, 6, 33, 6, 13, 82);
	skincoord_init_all(&self->generator.mute, 23, 139, 17, 17, 117, 31, 17, 17, 0);
	skincoord_init_all(&self->generator.bypass, 40, 139, 17, 17, 98, 31, 17, 17, 0);
	skincoord_init_all(&self->generator.solo, 6, 139, 17, 17, 98, 31, 17, 17, 0);
	skincoord_init_all(&self->generator.name, 0, 0, 0, 0, 20, 3, 117, 15, 0);
	// Effect
	skincoord_init_all(&self->effect.background, 0, 0, 138, 52, 0, 0, 138, 52, 0);
	skincoord_init_all(&self->effect.vu0, 0, 163, 0, 7, 4, 20, 129, 7, 129);
	skincoord_init_all(&self->effect.vupeak, 96, 144, 1, 7, 4, 20, 1, 7, 0);
	skincoord_init_all(&self->effect.pan, 57, 139, 6, 13, 6, 33, 6, 13, 82);
	skincoord_init_all(&self->effect.mute, 23, 139, 17, 17, 117, 31, 17, 17, 0);
	skincoord_init_all(&self->effect.bypass, 40, 139, 17, 17, 98, 31, 17, 17, 0);
	skincoord_init_all(&self->effect.solo, 40, 139, 17, 17, 98, 31, 17, 17, 0);
	skincoord_init_all(&self->effect.name, 0, 0, 0, 0, 20, 3, 117, 15, 0);
}

void machineviewskin_settheme(MachineViewSkin* self, psy_Property* p, const char* skindir)
{
	const char* machine_skin_name;

	self->hastransparency = FALSE;
	self->transparency = psy_ui_colour_make(0x00000000);
	if (p) {		
		self->wirecolour = psy_ui_colour_make(psy_property_at_colour(p, "mv_wirecolour", 0x005F5F5F));
		self->selwirecolour = psy_ui_colour_make(psy_property_at_colour(p, "mv_selwirecolour", 0x007F7F7F));
		self->hoverwirecolour = psy_ui_colour_make(psy_property_at_colour(p, "mv_hoverwirecolour", 0x007F7F7F));
		self->polycolour = psy_ui_colour_make(psy_property_at_colour(p, "mv_wireaacolour2", 0x005F5F5F));
		self->polycolour = psy_ui_colour_make(psy_property_at_colour(p, "mv_polycolour", 0x00B1C8B0));
		self->generator_fontcolour =
			psy_ui_colour_make(psy_property_at_colour(p, "mv_generator_fontcolour", 0x00B1C8B0));
		self->effect_fontcolour =
			psy_ui_colour_make(psy_property_at_colour(p, "mv_effect_fontcolour", 0x00D1C5B6));
		self->triangle_size = (double)psy_property_at_int(p, "mv_triangle_size", 10);
		machine_skin_name = psy_property_at_str(p, "machine_skin", 0);
		self->transparency = psy_ui_colour_make(psy_property_at_colour(p, "transparency", 0x00000000));
		self->hastransparency = psy_property_at(p, "transparency", PSY_PROPERTY_TYPE_NONE) != NULL;		
	} else {		
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
	self->wireaacolour = self->wirecolour;
	//	psy_ui_colour_make((((self->wirecolour.r + ((self->colour.r * 4)) / 5) +
	//		((self->wirecolour.g) + ((self->colour.g) * 4)) / 5) +
	//		((self->wirecolour.b) + ((self->colour.b) * 4)) / 5));
	self->wireaacolour2 = self->wirecolour;
	//	psy_ui_colour_make(self->wirecolour.r + (self->colour.r / 2) +
	//		(self->wirecolour.g) + (self->colour.g / 2) +
	//		(self->wirecolour.b) + (self->colour.b / 2));	
	if (machine_skin_name && psy_strlen(machine_skin_name)) {
		char path[_MAX_PATH];
		char filename[_MAX_PATH];

		strcpy(filename, machine_skin_name);
		strcat(filename, ".bmp");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_ui_Bitmap bmp;			

			psy_ui_bitmap_init(&bmp);			
			if (psy_ui_bitmap_load(&bmp, path) == PSY_OK) {
				self->skinbmp = bmp;
			}			
		}
		strcpy(filename, machine_skin_name);
		strcat(filename, ".psm");
		psy_dir_findfile(skindir, filename, path);
		if (path[0] != '\0') {
			psy_Property* coords;
			psy_Property* transparency;

			coords = psy_property_allocinit_key(NULL);
			skin_loadpsh(coords, path);
			machineviewskin_setcoords(self, coords);
			transparency = psy_property_at(coords,
				"transparency", PSY_PROPERTY_TYPE_NONE);
			self->hastransparency = transparency != NULL;
			if (transparency) {
				self->transparency = psy_ui_colour_make(
					strtol(psy_property_item_str(transparency), 0, 16));
				psy_ui_bitmap_settransparency(&self->skinbmp,
					self->transparency);
			}
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
	intptr_t vals[4];

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
		skincoord_setdest(&self->generator.vu0, vals, 3);
		psy_ui_realrectangle_setwidth(&self->generator.vu0.dest,
			(double)vals[2]);		
	}
	if (s = psy_property_at_str(p, "generator_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->generator.pan, vals, 3);		
	}
	if (s = psy_property_at_str(p, "generator_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->generator.mute, vals, 2);		
	}
	if (s = psy_property_at_str(p, "generator_solo_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->generator.solo, vals, 2);		
	}
	if (s = psy_property_at_str(p, "generator_name_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->generator.name, vals, 2);		
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
		skincoord_setdest(&self->effect.vu0, vals, 3);
		psy_ui_realrectangle_setwidth(&self->generator.vu0.dest,
			(double)vals[2]);
	}
	if (s = psy_property_at_str(p, "effect_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->effect.pan, vals, 3);
	}
	if (s = psy_property_at_str(p, "effect_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->effect.mute, vals, 2);
	}
	if (s = psy_property_at_str(p, "effect_bypass_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->effect.bypass, vals, 2);
	}
	if (s = psy_property_at_str(p, "effect_name_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->effect.name, vals, 2);
	}
}