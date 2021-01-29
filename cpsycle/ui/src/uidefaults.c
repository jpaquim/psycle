// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"
#include "uifont.h"

void psy_ui_defaults_init(psy_ui_Defaults* self, bool dark)
{		
	if (dark) {
		psy_ui_defaults_initdarktheme(self);
	} else {
		psy_ui_defaults_initlighttheme(self);
	}
	self->style_common.use_font = 1;
	{
		psy_ui_FontInfo fontinfo;
	
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", -16);	
		psy_ui_font_init(&self->style_common.font, &fontinfo);
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
	self->hasdarktheme = TRUE;
	self->errorcolour = 0xCF6679;	
	// common
	psy_ui_colour_set(&self->style_common.colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&self->style_common.backgroundcolour,
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init(&self->style_common.border);
	psy_ui_colour_set(&self->style_common.border.colour_top,
		psy_ui_colour_make(0x00333333));
	psy_ui_colour_set(&self->style_common.border.colour_right,
		self->style_common.border.colour_top);
	psy_ui_colour_set(&self->style_common.border.colour_bottom,
		self->style_common.border.colour_top);
	psy_ui_colour_set(&self->style_common.border.colour_left,
		self->style_common.border.colour_top);
	// button
	psy_ui_style_init(&self->style_button);
	psy_ui_colour_set(&self->style_button.colour, psy_ui_colour_make(0x00BDBDBD));
	// button::hover
	psy_ui_style_init(&self->style_button_hover);
	psy_ui_colour_set(&self->style_button_hover.colour,
		psy_ui_colour_make(0x00FFFFFF));
	// button::select
	psy_ui_style_init(&self->style_button_select);
	psy_ui_colour_set(&self->style_button_select.colour,
		psy_ui_colour_make(0x00B1C8B0));	
	// tab
	psy_ui_style_init(&self->style_tab);
	psy_ui_colour_set(&self->style_tab.colour, psy_ui_colour_make(0x00D1C5B6));
	// tab::hover
	psy_ui_style_init(&self->style_tab_hover);
	psy_ui_colour_set(&self->style_tab_hover.colour,
		psy_ui_colour_make(0x00EAEAEA));
	psy_ui_colour_set(&self->style_tab_hover.backgroundcolour,
		psy_ui_colour_make(0x00FFFFFF));
	// tab::select
	psy_ui_style_init(&self->style_tab_select);
	psy_ui_colour_set(&self->style_tab_select.colour,
		psy_ui_colour_make(0x00B1C8B0));
	psy_ui_colour_set(&self->style_tab_select.backgroundcolour,
		psy_ui_colour_make(0x00B1C8B0));
}

void psy_ui_defaults_initlighttheme(psy_ui_Defaults* self)
{
	self->hasdarktheme = FALSE;
	self->errorcolour = 0xCF6679;
	// common
	psy_ui_colour_set(&self->style_common.colour, psy_ui_colour_make(0x00000000));
	psy_ui_colour_set(&self->style_common.backgroundcolour,
		psy_ui_colour_make(0x00FBFBFB));
	psy_ui_border_init(&self->style_common.border);
	psy_ui_colour_set(&self->style_common.border.colour_top,
		psy_ui_colour_make(0x00333333));
	psy_ui_colour_set(&self->style_common.border.colour_right,
		self->style_common.border.colour_top);
	psy_ui_colour_set(&self->style_common.border.colour_bottom,
		self->style_common.border.colour_top);
	psy_ui_colour_set(&self->style_common.border.colour_left,
		self->style_common.border.colour_top);
	// button
	psy_ui_style_init(&self->style_button);
	psy_ui_colour_set(&self->style_button.colour,
		psy_ui_colour_make(0x00787573));
	// button::hover
	psy_ui_style_init(&self->style_button_hover);
	psy_ui_colour_set(&self->style_button_hover.colour,
		psy_ui_colour_make(0x000000));
	// button::select
	psy_ui_style_init(&self->style_button_select);
	psy_ui_colour_set(&self->style_button_select.colour,
		psy_ui_colour_make(0x1b8ff2));
	// tab
	psy_ui_style_init(&self->style_tab);
	psy_ui_colour_set(&self->style_tab.colour,
		psy_ui_colour_make(0x00FFFFFF));
	psy_ui_colour_set(&self->style_tab.backgroundcolour,
		psy_ui_colour_make(0x00474849));
	// tab::hover
	psy_ui_style_init(&self->style_tab_hover);
	psy_ui_colour_set(&self->style_tab_hover.colour,
		psy_ui_colour_make(0x00FFFFFF));
	psy_ui_colour_set(&self->style_tab_hover.backgroundcolour,
		psy_ui_colour_make(0x1b8ff2));
	// tab::select
	psy_ui_style_init(&self->style_tab_select);
	psy_ui_colour_set(&self->style_tab_select.colour,
		psy_ui_colour_make(0x00FFFFFF));
	psy_ui_colour_set(&self->style_tab_select.backgroundcolour,
		psy_ui_colour_make(0x1b8ff2));
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_style_dispose(&self->style_common);
}

