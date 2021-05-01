// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "styles.h"
// host
#include "resources/resource.h"
// ui
#include <uidefaults.h>

static void initstyles(psy_ui_Defaults*, bool dark);

void initdarkstyles(psy_ui_Defaults* defaults)
{
	initstyles(defaults, TRUE);
}

void initlightstyles(psy_ui_Defaults* defaults)
{
	initstyles(defaults, FALSE);
}

void initstyles(psy_ui_Defaults* defaults, bool dark)
{
	psy_ui_Style* style;
	psy_ui_Colour surface;
	psy_ui_Colour surface_overlay_5p;
	psy_ui_Colour surface_overlay_7p;
	psy_ui_Colour surface_overlay_8p;
	psy_ui_Colour surface_overlay_9p;
	psy_ui_Colour surface_overlay_11p;
	psy_ui_Colour surface_overlay_12p;
	psy_ui_Colour surface_overlay_16p;
	psy_ui_Colour surface_overlay_24p;
	psy_ui_Colour primary;
	psy_ui_Colour secondary;
	psy_ui_Colour overlay;
	psy_ui_Colour overlay_1p;
	psy_ui_Colour overlay_4p;
	psy_ui_Colour overlay_12p;
	psy_ui_Colour onsurface;
	psy_ui_Colour onprimary;
	psy_ui_Colour onprimary_weak;
	psy_ui_Colour onprimary_medium;
	psy_ui_Colour onprimary_strong;
	psy_ui_Colour onsecondary;
	// colour weight
	int strong;
	int accent;
	int light;
	int medium;
	int weak;
	int pale;
	int minima;

	defaults->hasdarktheme = dark;
	if (dark) {
		surface = psy_ui_colour_make_argb(0x00121212);
		onsurface = psy_ui_colour_make_argb(0x00EEFFFF);
		primary = psy_ui_colour_make_argb(0x00121212);
		onprimary = psy_ui_colour_make(0x00EEFFFF);
		secondary = psy_ui_colour_make(0x00FAD481);
		onsecondary = psy_ui_colour_make_argb(0x00FFFFFF);
		overlay = psy_ui_colour_make_argb(0x00FFFFFF);
		strong = 50;
		accent = 100;
		medium = 200;
		light = 300;
		weak = 400;
		pale = 800;
		minima = 900;
	} else {
		surface = psy_ui_colour_make_argb(0x00FAFAFA);
		onsurface = psy_ui_colour_make_argb(0x00000000);
		primary = psy_ui_colour_make_argb(0x00FFFFFF);
		onprimary = psy_ui_colour_make_argb(0x00000000);
		secondary = psy_ui_colour_make(0x00FAD481);
		onsecondary = psy_ui_colour_make(0x1b8ff2);
		overlay = psy_ui_colour_make_argb(0x00000000);
		strong = 900;
		accent = 800;
		medium = 600;
		light = 400;
		weak = 200;
		pale = 100;
		minima = 50;
	}
	// absolute surface overlays
	surface_overlay_5p = psy_ui_colour_overlayed(&surface, &overlay, 0.05);
	surface_overlay_7p = psy_ui_colour_overlayed(&surface, &overlay, 0.07);
	surface_overlay_8p = psy_ui_colour_overlayed(&surface, &overlay, 0.08);
	surface_overlay_9p = psy_ui_colour_overlayed(&surface, &overlay, 0.09);
	surface_overlay_11p = psy_ui_colour_overlayed(&surface, &overlay, 0.11);
	surface_overlay_12p = psy_ui_colour_overlayed(&surface, &overlay, 0.12);
	surface_overlay_16p = psy_ui_colour_overlayed(&surface, &overlay, 0.16);
	surface_overlay_24p = psy_ui_colour_overlayed(&surface, &overlay, 0.24);
	// relative overlays
	overlay_1p = psy_ui_colour_make_overlay(1);
	overlay_4p = psy_ui_colour_make_overlay(4);
	overlay_12p = psy_ui_colour_make_overlay(12);
	// onprimary
	onprimary_weak = psy_ui_colour_weighted(&onprimary, weak);
	onprimary_medium = psy_ui_colour_weighted(&onprimary, medium);
	onprimary_strong = psy_ui_colour_weighted(&onprimary, strong);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style, onprimary_medium, overlay_12p);	
	psy_ui_style_setspacing_em(style, 0.2, 1.0, 0.2, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_STATUSBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary_medium);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_7p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_11p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onsecondary, overlay_4p);	
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_medium, surface_overlay_5p);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_7p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_strong, surface_overlay_5p);
	psy_ui_border_init_solid(&style->border, surface_overlay_7p);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_medium, surface_overlay_5p);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW_TRACK, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_strong, surface_overlay_7p);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW_TRACK_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onsecondary, surface_overlay_9p);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onsecondary, light),
		psy_ui_colour_weighted(&secondary, pale));
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_11p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onsecondary, accent),
		psy_ui_colour_weighted(&secondary, pale));
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_11p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM_HOVER, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onsecondary, accent),	
		psy_ui_colour_weighted(&secondary, pale));
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_11p, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM_SELECTED, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&primary, medium));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, psy_ui_colour_weighted(&primary, accent),
		surface);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onsecondary, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&primary, weak));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_NEW, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, secondary,
		psy_ui_colour_overlayed(&surface, &overlay, 0.24));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_PROGRESS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&primary, medium));
	psy_ui_style_setbackgroundoverlay(style, 7);
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEBOX, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&primary, medium), overlay_4p);
	psy_ui_defaults_setstyle(defaults, STYLE_ZOOMBOX, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundcolour(style, surface_overlay_9p);
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEVIEW, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_setspacing_em(style, 0.25, 0.5, 0.5, 0.5);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_16p);
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEVIEW_PROPERTIES, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, primary, surface_overlay_9p);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_9p, 10.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_STEPSEQUENCER, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_5p);
	psy_ui_defaults_setstyle(defaults, STYLE_TOP, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROWS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_8p);
	psy_ui_style_setspacing_em(style, 0.0, 2.0, 0.0, 2.0);
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW0, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW1, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setspacing_em(style, 0.5, 2.0, 0.0, 2.0);
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW2, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface);
	psy_ui_style_setspacing_em(style, 1.0, 0.5, 0.0, 0.5);
	psy_ui_defaults_setstyle(defaults, STYLE_TRACKSCOPES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, surface_overlay_24p, surface);
	psy_ui_defaults_setstyle(defaults, STYLE_TRACKSCOPE, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 9);
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_defaults_setstyle(defaults, STYLE_SONGPROPERTIES, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_9p);
	psy_ui_defaults_setstyle(defaults, STYLE_SONGPROPERTIES_COMMENTS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_5p);
	psy_ui_style_setbackgroundid(style, IDB_ABOUT);
	style->backgroundrepeat = psy_ui_NOREPEAT;
	style->backgroundposition = psy_ui_ALIGNMENT_CENTER;
	psy_ui_defaults_setstyle(defaults, STYLE_ABOUT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SAMPLESVIEW, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_16p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SAMPLESVIEW_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SAMPLESVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_INSTRVIEW, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_16p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_defaults_setstyle(defaults, STYLE_INSTRVIEW_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_defaults_setstyle(defaults, STYLE_INSTRVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_style_setbackgroundoverlay(style, 6);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_11p, 10.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_PLAYBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary_weak);
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_5p, 6);	
	psy_ui_style_setspacing_em(style, 0.3, 0.3, 0.3, 0.3);
	psy_ui_defaults_setstyle(defaults, STYLE_DURATION_TIME, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_weak, surface_overlay_16p);		
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 4.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_KEY, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_medium, surface_overlay_12p);
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 4.0);
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_ACTIVE, style);
	
	style = psy_ui_style_clone(psy_ui_defaults_style(defaults, STYLE_KEY));	
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, secondary,	
		psy_ui_colour_make_rgb(52, 53, 50));
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 4.0);
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_weak,
		psy_ui_colour_make_rgb(128, 172, 131));		
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 4.0);
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_SHIFT_SELECT, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_weak,
		psy_ui_colour_make_rgb(28, 138, 103));
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 4.0);
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_ALT_SELECT, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_weak,
		psy_ui_colour_make_rgb(176, 173, 130));
	psy_ui_border_init_solid_radius(&style->border, surface_overlay_16p, 4.0);
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_CTRL_SELECT, style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_medium, surface_overlay_9p);
	psy_ui_defaults_setstyle(defaults, STYLE_TABLEROW, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_defaults_setstyle(defaults, STYLE_TABLEROW_HOVER, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_medium, surface_overlay_16p);
	psy_ui_defaults_setstyle(defaults, STYLE_TABLEROW_SELECT, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary_medium);		
	psy_ui_style_setbackgroundoverlay(style, 6);
	psy_ui_defaults_setstyle(defaults, STYLE_INPUTDEFINER_FOCUS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_style_setspacing_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		 psy_ui_colour_overlayed(&surface, &overlay, 0.3));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTIONS_HEADER, style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTION, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 2);	
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&surface, &secondary, 0.30));
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTION_SELECTED, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTION_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.5, 0.0, 0.5, 0.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SEARCHBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary_medium);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.20));
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_style_setspacing_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 8.0, 0.0, 0.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SEARCHFIELD, style);
	
	style = psy_ui_style_clone(style);	
	psy_ui_border_setcolour(&style->border, secondary);	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SEARCHFIELD_SELECT,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.3, 0.0, 0.0, 0.0);	
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_RESCANBAR, style);	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTIONBAR,
		psy_ui_style_clone(style));	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.2, 0.0, 0.2, 0.0);	
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SORTBAR, style);	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_FILTERBAR,
		psy_ui_style_clone(style));		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_CATEGORYBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 0.5, 2.0);
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_DETAIL, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border, surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_setcolour(style, onprimary_medium);
	psy_ui_style_setbackgroundoverlay(style, 4);	
	psy_ui_defaults_setstyle(defaults, STYLE_PROPERTYVIEW_MAINSECTION,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary_weak);
	psy_ui_style_setbackgroundoverlay(style, 1);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_PROPERTYVIEW_MAINSECTIONHEADER,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, onprimary_medium);
	psy_ui_border_init_solid(&style->border, surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_defaults_setstyle(defaults, STYLE_RECENTVIEW_MAINSECTION, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_weak, overlay_1p);	
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_RECENTVIEW_MAINSECTIONHEADER,
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary_weak, overlay_1p);	
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_defaults_setstyle(defaults, STYLE_HEADER, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_defaults_setstyle(defaults, STYLE_MAINVIEWTOPBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID, surface);
	psy_ui_defaults_setstyle(defaults, STYLE_NAVBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border, surface_overlay_12p);
	psy_ui_defaults_setstyle(defaults, STYLE_CLIPBOX, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_red());
	psy_ui_border_init_solid(&style->border, surface_overlay_12p);
	psy_ui_defaults_setstyle(defaults, STYLE_CLIPBOX_SELECT, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_LABELPAIR, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_LABELPAIR_FIRST, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_defaults_setstyle(defaults, STYLE_LABELPAIR_SECOND, style);

	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TERM_BUTTON, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make_argb(0x00F6B87F));
	psy_ui_defaults_setstyle(defaults, STYLE_TERM_BUTTON_WARNING, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x007966CF));
	psy_ui_defaults_setstyle(defaults, STYLE_TERM_BUTTON_ERROR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_BGMAIN);
	psy_ui_style_animatebackground(style, 20,
		psy_ui_realsize_make(96, 192), TRUE);
	psy_ui_defaults_setstyle(defaults, STYLE_GREET, style);
}
