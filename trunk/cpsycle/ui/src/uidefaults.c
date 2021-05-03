// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"
#include "uifont.h"
// file
#include <dir.h>
#include <propertiesio.h>

static void psy_ui_defaults_inittheme(psy_ui_Defaults*, bool dark, bool keepfont);

void psy_ui_defaults_init(psy_ui_Defaults* self, bool dark)
{		
	psy_ui_styles_init(&self->styles);
	if (dark) {
		psy_ui_defaults_initdarktheme(self, FALSE);
	} else {
		psy_ui_defaults_initlighttheme(self, FALSE);
	}			
	// group
	psy_ui_margin_init_em(&self->hmargin, 0.0, 1.0, 0.0, 0.0);		
	psy_ui_margin_init_em(&self->vmargin, 0.0, 0.0, 0.5, 0.0);		
	psy_ui_margin_init_em(&self->cmargin, 0.0, 0.5, 0.5, 0.5);		
}

void psy_ui_defaults_initdarktheme(psy_ui_Defaults* self, bool keepfont)
{
	psy_ui_defaults_inittheme(self, TRUE, keepfont);
}

void psy_ui_defaults_initlighttheme(psy_ui_Defaults* self, bool keepfont)
{
	psy_ui_defaults_inittheme(self, FALSE, keepfont);
}

void psy_ui_defaults_inittheme(psy_ui_Defaults* self, bool dark, bool keepfont)
{
	psy_ui_Style* style;
	psy_ui_Colour surface;
	psy_ui_Colour primary;
	psy_ui_Colour secondary;
	psy_ui_Colour overlay;
	psy_ui_Colour onsurface;
	psy_ui_Colour onprimary;
	psy_ui_Colour onsecondary;
	// colour weight
	int strong;
	int accent;
	int medium;
	int weak;
	psy_ui_Font oldfont;
	
	if (dark) {
		surface = psy_ui_colour_make(0x00121212);
		onsurface = psy_ui_colour_make(0x00EEFFFF);
		primary = psy_ui_colour_make(0x00121212);
		onprimary = psy_ui_colour_make(0x00EEFFFF);
		secondary = psy_ui_colour_make(0x00FAD481);
		onsecondary = psy_ui_colour_make(0x00FAD481);
		overlay = psy_ui_colour_make(0x00FFFFFF);
		strong = 50;
		accent = 100;
		medium = 200;
		weak = 400;		
		self->hasdarktheme = TRUE;		
	} else {
		surface = psy_ui_colour_make_argb(0x00FAFAFA);
		onsurface = psy_ui_colour_make_argb(0x00000000);
		primary = psy_ui_colour_make(0x00121212);
		onprimary = psy_ui_colour_make(0x00000000);
		secondary = psy_ui_colour_make(0x00FAD481);
		onsecondary = psy_ui_colour_make(0x1b8ff2);
		overlay = psy_ui_colour_make(0x00000000);
		strong = 900;
		accent = 800;
		medium = 600;
		weak = 200;
	}
	// root
	if (keepfont) {
		style = psy_ui_defaults_style(self, psy_ui_STYLE_ROOT);
		if (style) {
			psy_ui_font_init(&oldfont, NULL);
			psy_ui_font_copy(&oldfont, &style->font);
		} else {
			keepfont = FALSE;
		}
	}
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, 
		psy_ui_colour_weighted(&onsurface, accent),
		surface);
	if (keepfont) {		
		psy_ui_font_init(&style->font, NULL);
		psy_ui_font_copy(&style->font, &oldfont);
		style->use_font = TRUE;
	} else {
		psy_ui_style_setfont(style, "Tahoma", -16);
	}
	if (keepfont) {
		psy_ui_font_dispose(&oldfont);
	}
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_ROOT, style);	
	// label
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&onsurface, accent));	
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_LABEL, style);
	// label::disabled
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&onsurface, weak));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_LABEL_DISABLED, style);
	// edit
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&onsurface, accent));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_EDIT, style);
	// edit::focus
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&onsurface, accent));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make_overlay(6));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_EDIT_FOCUS, style);
	// button
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, 
		psy_ui_colour_weighted(&onsurface, medium));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON, style);	
	// button::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,		
		psy_ui_colour_weighted(&onsurface, accent),
		psy_ui_colour_make_overlay(4));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_HOVER, style);
	// button::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onsecondary);
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_SELECT, style);
	// button::active
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onsurface, strong),
		psy_ui_colour_make_overlay(4));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_BUTTON_ACTIVE, style);
	// combobox;
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, medium));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMBOBOX, style);
	// combobox::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, strong));	
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMBOBOX_HOVER, style);
	// combobox::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, strong));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_COMBOBOX_SELECT, style);	
	// tabbar
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TABBAR, style);	
	// tab
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&primary, medium));	
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB, style);
	// tab::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary);
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make_overlay(4));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_HOVER, style);
	// tab::select
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolour(style, onprimary);	
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_NONE,
		psy_ui_BORDER_NONE, psy_ui_BORDER_SOLID, psy_ui_BORDER_NONE);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_weighted(&onsecondary, weak));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_SELECT, style);
	// tab_label
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&primary, weak));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_TAB_LABEL, style);
	// header;
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_make_overlay(9));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_HEADER, style);
	// scrollpane
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SCROLLPANE,
		psy_ui_style_allocinit_colours(
			onsurface, psy_ui_colour_make_overlay(5)));
	// scrollthumb
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SCROLLTHUMB,
		psy_ui_style_allocinit_colours(
			onsurface, psy_ui_colour_make_overlay(10)));
	// scrollthumb::hover
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SCROLLTHUMB_HOVER,
		psy_ui_style_allocinit_colours(
			onsurface, psy_ui_colour_make_overlay(20)));
	// sliderpane
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(5));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);	
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.08));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SLIDERPANE, style);	
	// sliderthumb
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SLIDERTHUMB,
		psy_ui_style_allocinit_colours(onsecondary, secondary));
	// sliderthumb::hover
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SLIDERTHUMB_HOVER,
		psy_ui_style_allocinit_colours(onprimary, onprimary));
	// splitter
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));	
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SPLITTER, style);
	// splitter::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SPLITTER_HOVER, style);
	// splitter::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, secondary,
		psy_ui_colour_make_overlay(16));
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_SPLITTER_SELECT, style);
	// psy_ui_STYLE_PROGRESSBAR
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, secondary);
	psy_ui_defaults_setstyle(self, psy_ui_STYLE_PROGRESSBAR, style);
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_styles_dispose(&self->styles);		
}

void psy_ui_defaults_setstyle(psy_ui_Defaults* self, uintptr_t styletype,
	psy_ui_Style* style)
{
	if (style) {
		psy_ui_styles_setstyle(&self->styles, styletype, style);
	}
}

psy_ui_Style* psy_ui_defaults_style(psy_ui_Defaults* self, uintptr_t type)
{
	return psy_ui_styles_at(&self->styles, type);
}

const psy_ui_Style* psy_ui_defaults_style_const(const psy_ui_Defaults* self,
	uintptr_t type)
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
		psy_ui_defaults_initdarktheme(self, TRUE);
	}  else {
		psy_ui_defaults_initlighttheme(self, TRUE);
	}
	styleconfig = psy_property_clone(
		psy_ui_styles_configuration(&self->styles));
	if (propertiesio_load(styleconfig, &path, 0) == PSY_OK) {	
		self->hasdarktheme = isdark;
		psy_ui_styles_configure(&self->styles, styleconfig);
		// font
		psy_ui_defaults_style(self, psy_ui_STYLE_ROOT)->use_font = 1;
		{
			psy_ui_FontInfo fontinfo;

			psy_ui_fontinfo_init(&fontinfo, "Tahoma", -16);
			psy_ui_font_init(&psy_ui_defaults_style(self, psy_ui_STYLE_ROOT)->font,
				&fontinfo);
		}
	}	
	psy_property_deallocate(styleconfig);
	psy_path_dispose(&path);	
}

void psy_ui_defaults_savetheme(psy_ui_Defaults* self, const char* filename)
{	
	const psy_Property* styleconfig;

	assert(self);

	styleconfig = psy_ui_styles_configuration(&self->styles);
	if (styleconfig && !psy_property_empty(styleconfig)) {	
		propertiesio_save(styleconfig, filename);		
	}	
}
