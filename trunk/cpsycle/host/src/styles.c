// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "styles.h"
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
	int minima;

	if (dark) {
		surface = psy_ui_colour_make(0x00121212);
		onsurface = psy_ui_colour_make(0x00FFFFFF);
		primary = psy_ui_colour_make(0x00121212);
		onprimary = psy_ui_colour_make(0x00EEFFFF);
		secondary = psy_ui_colour_make(0x00FAD481);
		onsecondary = psy_ui_colour_make(0x00FAD481);
		overlay = psy_ui_colour_make(0x00FFFFFF);
		strong = 50;
		accent = 100;
		medium = 200;
		weak = 400;	
		minima = 900;
	} else {
		surface = psy_ui_colour_make_argb(0x00FFFFFF);
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
		minima = 50;
	}
	// Statusbar	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_overlayed(&surface, &overlay, 0.12));	
	psy_ui_defaults_setstyle(defaults, STYLE_STATUSBAR, style);
	// seqview buttons
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTONS, style);
	// seqview button
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		surface);
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.3));
	psy_ui_border_setradius_px(&style->border, 5.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON, style);
	// seqview button::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary, surface);
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, onprimary);
	psy_ui_border_setradius_px(&style->border, 5.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON_HOVER, style);
	// seqview button::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onsecondary, surface);
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.3));
	psy_ui_border_setradius_px(&style->border, 5.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON_SELECT, style);
	// seqlistview
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, 
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_colour_set(&style->border.colour_right,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_colour_set(&style->border.colour_bottom,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW, style);
	// seqlistview::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, strong),
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_colour_set(&style->border.colour_right,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_colour_set(&style->border.colour_bottom,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW_SELECT, style);
	// seqeditor
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, onsecondary);
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT, style);
	// sequenceeditor item
	style = psy_ui_style_allocinit();
	if (dark) {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&secondary, 800));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	} else {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&secondary, 100));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 100));
	}
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, onprimary);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM, style);
	// sequenceeditor item::selected	
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour,
		psy_ui_colour_weighted(&secondary, 500));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_weighted(&secondary, 500));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border, onprimary);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM_SELECTED, style);
	// sequencetrackbox tab
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour,
		psy_ui_colour_weighted(&primary, medium));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB, style);
	// sequencetrackbox tab::hover
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour,
		psy_ui_colour_weighted(&primary, accent));
	psy_ui_colour_set(&style->backgroundcolour, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_HOVER, style);
	// sequencetrackbox::select
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, onsecondary);
	psy_ui_colour_set(&style->backgroundcolour, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_SELECT, style);
	// sequencetrackbox newtrack
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour,
		psy_ui_colour_weighted(&primary, weak));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_TAB_NEW, style);
	// sequenceview progress
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, secondary);
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.24));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQ_PROGRESS, style);
	// machinebox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour,
		psy_ui_colour_weighted(&primary, medium));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));	
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEBOX, style);
	// zoombox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, 
		psy_ui_colour_weighted(&primary, medium));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.13));
	psy_ui_defaults_setstyle(defaults, STYLE_ZOOMBOX, style);
	// machineview
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, primary);
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEVIEW, style);
	// stepsequencer
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, primary);
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_border_setradius_px(&style->border, 10.0);
	psy_ui_defaults_setstyle(defaults, STYLE_STEPSEQUENCER, style);
	// top
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_NONE, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE);
	psy_ui_border_setcolour(&style->border, 
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_defaults_setstyle(defaults, STYLE_TOP, style);	
	// toprows
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROWS, style);
	// toprow0
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW0, style);
	// toprow1
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW1, style);
	// toprow2
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_TOPROW2, style);
	// trackscopes
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_TRACKSCOPES, style);
	// songproperties
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_SONGPROPERTIES, style);
	// songproperties-comments
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_SONGPROPERTIES_COMMENTS, style);
	// samplesview
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_SAMPLESVIEW, style);
	// about
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_defaults_setstyle(defaults, STYLE_ABOUT, style);
	// instrumentview
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_INSTRUMENTVIEW, style);
	// playbar
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.10));
	psy_ui_border_setradius_px(&style->border, 10.0);
	psy_ui_defaults_setstyle(defaults, STYLE_PLAYBAR, style);
	// duration
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		surface);
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));
	psy_ui_colour_set(&style->border.colour_right,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_colour_set(&style->border.colour_bottom,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_defaults_setstyle(defaults, STYLE_DURATION_TIME, style);
	// key
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_KEY, style);
	// key::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_HOVER, style);
	// key::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onsecondary, surface);	
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_SELECT, style);
}
