/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uimaterial.h"

void psy_ui_materialtheme_init(psy_ui_MaterialTheme* self, psy_ui_ThemeMode mode)
{
	self->mode = mode;
	if (self->mode == psy_ui_DARKTHEME) {		
		self->surface = psy_ui_colour_make_argb(0x00121212);
		self->onsurface = psy_ui_colour_make_argb(0x00FFFFEE);
		self->primary = psy_ui_colour_make_argb(0x00121212);
		self->onprimary = psy_ui_colour_make(0x00EEFFFF);
		self->secondary = psy_ui_colour_make(0x00FAD481);
		self->onsecondary = psy_ui_colour_make_argb(0x00FFFFFF);
		self->overlay = psy_ui_colour_make_argb(0x00FFFFFF);

		self->strong = 50;
		self->accent = 100;
		self->medium = 200;
		self->light = 300;
		self->weak = 400;
		self->pale = 800;
		self->minima = 900;
	} else {
		self->surface = psy_ui_colour_make_argb(0x00FAFAFA);
		self->onsurface = psy_ui_colour_make_argb(0x00000000);
		self->primary = psy_ui_colour_make_argb(0x00FFFFFF);
		self->onprimary = psy_ui_colour_make_argb(0x00000000);
		self->secondary = psy_ui_colour_make(0x00FAD481);
		self->onsecondary = psy_ui_colour_make(0x1b8ff2);
		self->overlay = psy_ui_colour_make_argb(0x00000000);
		self->strong = 900;
		self->accent = 800;
		self->medium = 600;
		self->light = 400;
		self->weak = 200;
		self->pale = 100;
		self->minima = 50;
	}
	/* absolute surface overlays */
	self->surface_overlay_5p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.05);
	self->surface_overlay_7p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.07);
	self->surface_overlay_8p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.08);
	self->surface_overlay_9p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.09);
	self->surface_overlay_11p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.11);
	self->surface_overlay_12p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.12);
	self->surface_overlay_16p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.16);
	self->surface_overlay_24p = psy_ui_colour_overlayed(&self->surface, &self->overlay, 0.24);
	/* relative overlays */
	self->overlay_1p = psy_ui_colour_make_overlay(1);
	self->overlay_4p = psy_ui_colour_make_overlay(4);
	self->overlay_12p = psy_ui_colour_make_overlay(12);
	/* onprimary */
	self->onprimary_weak = psy_ui_colour_weighted(&self->onprimary, self->weak);
	self->onprimary_medium = psy_ui_colour_weighted(&self->onprimary, self->medium);
	self->onprimary_strong = psy_ui_colour_weighted(&self->onprimary, self->strong);
}
