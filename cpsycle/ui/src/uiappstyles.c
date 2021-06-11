/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiappstyles.h"
/* local*/
#include "uimaterial.h"

static void psy_ui_appstyles_inittheme_win98(psy_ui_Styles*, bool keepfont);

void psy_ui_appstyles_inittheme(psy_ui_Styles* self, psy_ui_ThemeMode theme,
	bool keepfont)
{	
	psy_ui_Style* style;
	psy_ui_MaterialTheme material;
	psy_ui_Font oldfont;

	if (theme == psy_ui_WIN98THEME) {
		psy_ui_appstyles_inittheme_win98(self, keepfont);
		return;
	}		
	self->theme = theme;
	psy_ui_materialtheme_init(&material, theme);
	/* root */
	if (keepfont) {
		style = psy_ui_styles_at(self, psy_ui_STYLE_ROOT);
		if (style) {
			psy_ui_font_init(&oldfont, NULL);
			psy_ui_font_copy(&oldfont, &style->font);
		} else {
			keepfont = FALSE;
		}
	}
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, 
		psy_ui_colour_weighted(&material.onsurface, material.accent),
			material.surface);
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
	psy_ui_styles_setstyle(self, psy_ui_STYLE_ROOT, style);	
	/* label */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_LABEL, style);
	/* label::disabled */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.weak));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_LABEL_DISABLED, style);
	/* edit */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_EDIT, style);
	/* edit::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make_overlay(6));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_EDIT_FOCUS, style);
	/* button */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, 
		psy_ui_colour_weighted(&material.onsurface, material.medium));	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON, style);	
	/* button::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,		
		psy_ui_colour_weighted(&material.onsurface, material.accent),
		psy_ui_colour_make_overlay(4));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_HOVER, style);
	/* button::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.secondary);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_SELECT, style);
	/* button::active */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onsurface, material.strong),
		psy_ui_colour_make_overlay(4));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_ACTIVE, style);
	/* button::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.medium));
	psy_ui_border_init_solid_radius(&style->border, material.secondary, 6.0);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_FOCUS, style);
	/* combobox */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.medium));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_COMBOBOX, style);
	/* combobox::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.strong));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_COMBOBOX_HOVER, style);
	/* combobox::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.strong));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_COMBOBOX_SELECT, style);	
	/* tabbar */
	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TABBAR, style);	
	/* tab */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.primary, material.medium));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB, style);
	/* tab::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary);
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make_overlay(4));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB_HOVER, style);
	/* tab::select */
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolour(style, material.onprimary);
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_NONE,
		psy_ui_BORDER_NONE, psy_ui_BORDER_SOLID, psy_ui_BORDER_NONE);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_weighted(&material.secondary, material.weak));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB_SELECT, style);
	/* tab_label */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.primary, material.weak));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB_LABEL, style);
	/* header */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.medium),
		psy_ui_colour_make_overlay(9));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_HEADER, style);
	/* scrollpane */
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SCROLLPANE,
		psy_ui_style_allocinit_colours(
			material.onsurface, psy_ui_colour_make_overlay(5)));
	/* scrollthumb */
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SCROLLTHUMB,
		psy_ui_style_allocinit_colours(
			material.onsurface, psy_ui_colour_make_overlay(10)));
	/* scrollthumb::hover */
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SCROLLTHUMB_HOVER,
		psy_ui_style_allocinit_colours(
			material.onsurface, psy_ui_colour_make_overlay(20)));
	/* sliderpane */
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(5));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.08));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SLIDERPANE, style);	
	/* sliderthumb */
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SLIDERTHUMB,
		psy_ui_style_allocinit_colours(
			material.secondary, material.secondary));
	/* sliderthumb::hover */
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SLIDERTHUMB_HOVER,
		psy_ui_style_allocinit_colours(material.onprimary,
			material.onprimary));
	/* splitter */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.05));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SPLITTER, style);
	/* splitter::hover */
	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SPLITTER_HOVER, style);
	/* splitter::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.secondary,
		psy_ui_colour_make_overlay(16));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SPLITTER_SELECT, style);
	/* psy_ui_STYLE_PROGRESSBAR */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.secondary);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_PROGRESSBAR, style);
	/* switch */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.pale));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SWITCH, style);
	/* switch::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.pale));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SWITCH_HOVER, style);
	/* switch::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.secondary, material.weak));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SWITCH_SELECT, style);
}


/* Windows 98 Theme */
void psy_ui_appstyles_inittheme_win98(psy_ui_Styles* self, bool keepfont)
{
	psy_ui_Style* style;
	psy_ui_MaterialTheme material;
	psy_ui_Font oldfont;

	self->theme = psy_ui_LIGHTTHEME;
	psy_ui_materialtheme_init(&material, psy_ui_LIGHTTHEME);
	material.surface = psy_ui_colour_make(0x00C0C0C0);
	/* root */
	if (keepfont) {
		style = psy_ui_styles_at(self, psy_ui_STYLE_ROOT);
		if (style) {
			psy_ui_font_init(&oldfont, NULL);
			psy_ui_font_copy(&oldfont, &style->font);
		} else {
			keepfont = FALSE;
		}
	}
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent),
		material.surface);
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
	psy_ui_styles_setstyle(self, psy_ui_STYLE_ROOT, style);
	/* label */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_LABEL, style);
	/* label::disabled */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.weak));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_LABEL_DISABLED, style);
	/* edit */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_EDIT, style);
	/* edit::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onsurface, material.accent));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make_overlay(6));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_EDIT_FOCUS, style);
	/* button */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));	
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON, style);
	/* button::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_HOVER, style);
	/* button::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0xDFDFDF));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_SELECT, style);
	/* button::active */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0xDFDFDF));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080));
	psy_ui_style_setspacing_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_ACTIVE, style);
	/* button::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_BUTTON_FOCUS, style);
	/* combobox */
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x000000), psy_ui_colour_make(0xFFFFFF));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0x000000), psy_ui_colour_make(0xE0E0E0),
		psy_ui_colour_make(0xE0E0E0), psy_ui_colour_make(0x000000));
	psy_ui_style_setspacing_em(style, 0.0, 0.0, 0.0, 0.5);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_COMBOBOX, style);
	/* combobox::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.strong));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_COMBOBOX_HOVER, style);
	/* combobox::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.onprimary, material.strong));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_COMBOBOX_SELECT, style);
	/* tabbar */
	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TABBAR, style);
	/* tab */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xE0E0E0), psy_ui_colour_make(0x000000),
		psy_ui_colour_make(0xE0E0E0), psy_ui_colour_make(0xE0E0E0));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB, style);
	/* tab::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xE0E0E0), psy_ui_colour_make(0x000000),
		psy_ui_colour_make(0xE0E0E0), psy_ui_colour_make(0xE0E0E0));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB_HOVER, style);
	/* tab::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xE0E0E0), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0xC0C0C0), psy_ui_colour_make(0xE0E0E0));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB_SELECT, style);
	/* tab_label */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.primary, material.weak));
	psy_ui_style_setspacing_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_TAB_LABEL, style);
	/* header */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.medium),
		psy_ui_colour_make_overlay(9));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_HEADER, style);
	/* scrollpane */
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SCROLLPANE,
		psy_ui_style_allocinit_colours(
			material.onsurface, psy_ui_colour_make(0xDFDFDF)));
	/* scrollthumb */	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0xC0C0C0));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SCROLLTHUMB, style);
	/* scrollthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x808080));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SCROLLTHUMB_HOVER, style);	
	/* sliderpane */	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SLIDERPANE,
		psy_ui_style_allocinit_colours(
			material.onsurface, psy_ui_colour_make(0xDFDFDF)));	
	/* sliderthumb */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0xC0C0C0));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SLIDERTHUMB, style);	
	/* sliderthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x808080));
	psy_ui_border_setcolours(&style->border,
		psy_ui_colour_make(0xFFFFFF), psy_ui_colour_make(0x808080),
		psy_ui_colour_make(0x808080), psy_ui_colour_make(0xFFFFFF));
	psy_ui_style_setspacing_em(style, 0.25, 0.25, 0.25, 0.25);
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SLIDERTHUMB_HOVER, style);	
	/* splitter */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.05));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SPLITTER, style);
	/* splitter::hover */
	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SPLITTER_HOVER, style);
	/* splitter::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.secondary,
		psy_ui_colour_make_overlay(16));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SPLITTER_SELECT, style);
	/* psy_ui_STYLE_PROGRESSBAR */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x800000));
	psy_ui_styles_setstyle(self, psy_ui_STYLE_PROGRESSBAR, style);
	/* switch */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SWITCH, style);
	/* switch::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x000000));	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SWITCH_HOVER, style);
	/* switch::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x800000));	
	psy_ui_styles_setstyle(self, psy_ui_STYLE_SWITCH_SELECT, style);
}
