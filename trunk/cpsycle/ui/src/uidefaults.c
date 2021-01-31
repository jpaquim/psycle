// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"
#include "uifont.h"
// file
#include <dir.h>
#include <propertiesio.h>

void psy_ui_defaults_init(psy_ui_Defaults* self, bool dark)
{		
	psy_ui_styles_init(&self->styles);
	if (dark) {
		psy_ui_defaults_initdarktheme(self);
	} else {
		psy_ui_defaults_initlighttheme(self);
	}			
	// group
	psy_ui_margin_init_all(&self->hmargin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(1.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_margin_init_all(&self->vmargin,
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5), psy_ui_value_makepx(0));
	psy_ui_margin_init_all(&self->cmargin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(0.5),
		psy_ui_value_makeeh(0.5), psy_ui_value_makeew(0.5));
	// orange 0x002279F1
	// green 0x00B1C8B0
}

void psy_ui_defaults_initdarktheme(psy_ui_Defaults* self)
{
	psy_ui_Style* style;

	self->hasdarktheme = TRUE;
	self->errorcolour = 0xCF6679;
	// common
	style = psy_ui_style_allocinit();	
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init(&style->border);
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00333333));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);
	// font	
	{
		psy_ui_FontInfo fontinfo;

		style->use_font = 1;
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", -16);
		psy_ui_font_init(&style->font, &fontinfo);
	}
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMMON, style);
	// common::select
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMMON_SELECT,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00FFFFFF),
			psy_ui_colour_make(0x00232323)));	
	// button
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00BDBDBD));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON, style);	
	// button::hover
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_HOVER, style);
	// button::select
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00B1C8B0));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_SELECT, style);	
	// tab
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00D1C5B6),
			psy_ui_colour_make(0x00232323)));	
	// tab::hover
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_HOVER,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00EAEAEA),
			psy_ui_colour_make(0x00FFFFFF)));	
	// tab::select
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_SELECT,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00B1C8B0),
			psy_ui_colour_make(0x00B1C8B0)));	
	// slider
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x003E3E3E));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SLIDER, style);	
	// sidemenu
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SIDEMENU,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00D1C5B6),
			psy_ui_colour_make(0x00232323)));	
	// sidemenu::select
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SIDEMENU_SELECT,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00FFFFFF),
			psy_ui_colour_make(0x00232323)));	
	// psy_ui_Style style_containerheader;
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_CONTAINERHEADER,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00D1C5B6),
			psy_ui_colour_make(0x00232323)));	
}

void psy_ui_defaults_initlighttheme(psy_ui_Defaults* self)
{
	psy_ui_Style* style;

	self->hasdarktheme = FALSE;
	self->errorcolour = 0xCF6679;
	// common
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00000000));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make(0x00FBFBFB));
	psy_ui_border_init(&style->border);	
	psy_ui_colour_set(&style->border.colour_top,
		psy_ui_colour_make(0x00B2B2B2));
	psy_ui_colour_set(&style->border.colour_right,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_bottom,
		style->border.colour_top);
	psy_ui_colour_set(&style->border.colour_left,
		style->border.colour_top);	
	// font	
	{
		psy_ui_FontInfo fontinfo;

		style->use_font = 1;
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", -16);
		psy_ui_font_init(&style->font, &fontinfo);
	}
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMMON, style);
	// common::select
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMMON_SELECT,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00FFFFFF),
			psy_ui_colour_make(0x00232323)));
	// button
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00787573));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON, style);	
	// button::hover
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x000000));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_HOVER, style);
	// button::select
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x1b8ff2));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_SELECT, style);
	// tab
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00474849),
			psy_ui_colour_make(0x00F2F2F2)));
	// tab::hover
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_HOVER,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00474849),
			psy_ui_colour_make(0x0041047A)));
	// tab::select
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_SELECT,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00474849),
			psy_ui_colour_make(0x0041047A)));
	// slider
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, psy_ui_colour_make(0x00C9C3C2));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SLIDER, style);
	// sidemenu
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SIDEMENU,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00787573),
			psy_ui_colour_make(0x00E9E9E9)));
	// sidemenu::select
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SIDEMENU_SELECT,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00000000),
			psy_ui_colour_make(0x00FBFBFB)));
	// psy_ui_Style style_containerheader;
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_CONTAINERHEADER,
		psy_ui_style_allocinit_colours(
			psy_ui_colour_make(0x00444444),
			psy_ui_colour_make(0x00DEDEDE)));	
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_styles_dispose(&self->styles);		
}

void psy_ui_defaults_setstyle(psy_ui_Defaults* self, int styletype,
	psy_ui_Style* style)
{
	psy_ui_styles_setstyle(&self->styles, styletype, style);
}

psy_ui_Style* psy_ui_defaults_style(psy_ui_Defaults* self, int type)
{
	return psy_ui_styles_at(&self->styles, type);
}

const psy_ui_Style* psy_ui_defaults_style_const(const psy_ui_Defaults* self, int type)
{
	return psy_ui_styles_at_const(&self->styles, type);
}

void psy_ui_defaults_loadtheme(psy_ui_Defaults* self, const char* configdir, bool isdark)
{
	psy_Property* styleconfig;
	psy_Path path;
	
	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, configdir);
	if (isdark) {
		psy_path_setname(&path, PSYCLE_DARKSTYLES_INI);		
	} else {
		psy_path_setname(&path, PSYCLE_LIGHTSTYLES_INI);
	}	
	// reset to defaults
	if (isdark) {
		psy_ui_defaults_initdarktheme(self);
	}  else {
		psy_ui_defaults_initlighttheme(self);
	}
	styleconfig = psy_property_clone(
		psy_ui_styles_configuration(&self->styles));
	if (propertiesio_load(styleconfig, &path, 0)) {	
		self->hasdarktheme = isdark;
		psy_ui_styles_configure(&self->styles, styleconfig);
		// font
		psy_ui_defaults_style(self, psy_ui_STYLE_COMMON)->use_font = 1;
		{
			psy_ui_FontInfo fontinfo;

			psy_ui_fontinfo_init(&fontinfo, "Tahoma", -16);
			psy_ui_font_init(&psy_ui_defaults_style(self, psy_ui_STYLE_COMMON)->font,
				&fontinfo);
		}
	}	
	psy_property_deallocate(styleconfig);
	psy_path_dispose(&path);	
}
