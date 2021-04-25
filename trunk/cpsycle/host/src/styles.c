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
		onsecondary = psy_ui_colour_make(0x00FFFFFF);
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
	// statusbar	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_make_overlay(12));		
	psy_ui_defaults_setstyle(defaults, STYLE_STATUSBAR, style);
	// seqview buttons
	style = psy_ui_style_allocinit();	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTONS, style);
	// seqview button
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, medium));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));
	psy_ui_defaults_setstyle(defaults, STYLE_SEQVIEW_BUTTON, style);
	// seqview button::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, onprimary,
		psy_ui_colour_make_overlay(4));	
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.11));
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
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_border_setcolour(&style->border, 
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
	// seqlistview_track
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_overlayed(&surface, &overlay, 0.05));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW_TRACK, style);
	// seqlistview_track::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, strong),
		psy_ui_colour_overlayed(&surface, &overlay, 0.07));	
	psy_ui_defaults_setstyle(defaults, STYLE_SEQLISTVIEW_TRACK_SELECT, style);
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
			psy_ui_colour_weighted(&onsecondary, 300));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	} else {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&onsecondary, 100));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	}
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.11));
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM, style);
	// sequenceeditor item::hover	
	style = psy_ui_style_allocinit();
	if (dark) {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&onsecondary, 200));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	} else {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&onsecondary, 400));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	}
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.11));
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_defaults_setstyle(defaults, STYLE_SEQEDT_ITEM_HOVER, style);
	// sequenceeditor item::selected	
	style = psy_ui_style_allocinit();
	if (dark) {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&onsecondary, 100));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	} else {
		psy_ui_colour_set(&style->colour,
			psy_ui_colour_weighted(&onsecondary, 400));
		psy_ui_colour_set(&style->backgroundcolour,
			psy_ui_colour_weighted(&secondary, 800));
	}
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.11));
	psy_ui_border_setradius_px(&style->border, 6.0);	
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
		psy_ui_colour_make_overlay(7));
	psy_ui_defaults_setstyle(defaults, STYLE_MACHINEBOX, style);
	// zoombox
	style = psy_ui_style_allocinit();
	psy_ui_colour_set(&style->colour, 
		psy_ui_colour_weighted(&primary, medium));
	psy_ui_colour_set(&style->backgroundcolour,
		psy_ui_colour_make_overlay(4));
	psy_ui_defaults_setstyle(defaults, STYLE_ZOOMBOX, style);
	// machineview
	style = psy_ui_style_allocinit();	
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
		psy_ui_colour_make_overlay(9));
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
	psy_ui_style_setcolour(style,
		psy_ui_colour_make_rgb(18, 18, 18));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(89, 89, 89));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY, style);
	// key::active
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_make_rgb(18, 18, 18));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(69, 69, 69));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_ACTIVE, style);
	// key::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, secondary);
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(79, 79, 79));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_HOVER, style);
	// key::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, secondary);
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(52, 53, 50));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_SELECT, style);
	// shift key
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_make_rgb(18, 18, 18));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(128, 172, 131));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_SHIFT_SELECT, style);
	// alt key
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_make_rgb(18, 18, 18));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(28, 138, 103));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_ALT_SELECT, style);
	// ctrl key
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_make_rgb(18, 18, 18));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_rgb(176, 173, 130));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 4.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_make_rgb(62, 63, 65));
	psy_ui_defaults_setstyle(defaults, STYLE_KEY_CTRL_SELECT, style);	
	// table row
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_overlayed(&surface, &overlay, 0.09));
	psy_ui_defaults_setstyle(defaults, STYLE_TABLEROW, style);
	// table row::hover
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_overlayed(&surface, &overlay, 0.12));
	psy_ui_defaults_setstyle(defaults, STYLE_TABLEROW_HOVER, style);
	// table row::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_overlayed(&surface, &overlay, 0.15));
	psy_ui_defaults_setstyle(defaults, STYLE_TABLEROW_SELECT, style);
	// inputdefiner::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&onprimary, medium),
		psy_ui_colour_make_overlay(6));
	psy_ui_defaults_setstyle(defaults, STYLE_INPUTDEFINER_FOCUS, style);
	// newmachine
	// newmachine_sectionsheader
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_overlayed(&surface, &overlay, 0.3));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTIONS_HEADER, style);		
	// newmachine_section
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.5, 0.5, 1.0, 1.0));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTION, style);
	// newmachine_section::select
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,		
		psy_ui_colour_make_overlay(2));
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.5, 0.5, 1.0, 1.0));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &secondary, 0.30));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTION_SELECTED, style);
	// newmachine_sectionheader
	style = psy_ui_style_allocinit();	
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTION_HEADER, style);
	// newmachine_searchbar
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 0.0));	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SEARCHBAR, style);
	// newmachine_searchfield	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, medium));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.20));	
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.3, 0.3, 0.3, 0.0));
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 8.0, 0.0, 0.0));	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SEARCHFIELD, style);
	// newmachine_rescanbar
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.2, 0.0, 0.2, 0.0));
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.3, 0.0, 0.0, 0.0));	
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(4));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_RESCANBAR, style);
	// newmachine_categorybar
	style = psy_ui_style_allocinit();	
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_CATEGORYBAR, style);
	// newmachine_sectionbar
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.2, 0.0, 0.2, 0.0));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(4));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SECTIONBAR, style);		
	// newmachine_sortbar
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.2, 0.0, 0.2, 0.0));	
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 3.0));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_SORTBAR, style);
	// newmachine_filterbar
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.2, 0.0, 0.2, 0.0));
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 3.0));
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_FILTERBAR, style);
	// newmachine_detail
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.5, 0.5, 0.5, 2.0));	
	psy_ui_defaults_setstyle(defaults, STYLE_NEWMACHINE_DETAIL, style);
	// propertyview
	// propertyview_mainsection
	style = psy_ui_style_allocinit();
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);		
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, medium));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(4));
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.06));
	psy_ui_defaults_setstyle(defaults, STYLE_PROPERTYVIEW_MAINSECTION, style);
	// propertyview_mainsectionheader
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, weak));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(1));
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_overlayed(&surface, &overlay, 0.06));
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));
	psy_ui_defaults_setstyle(defaults,
		STYLE_PROPERTYVIEW_MAINSECTIONHEADER, style);
	// recentview_mainsection
	style = psy_ui_style_allocinit();
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, medium));	
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&surface, &overlay, 0.06));
	psy_ui_defaults_setstyle(defaults, STYLE_RECENTVIEW_MAINSECTION, style);
	// recentview_mainsectionheader
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, weak));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(1));
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_overlayed(&surface, &overlay, 0.06));
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));
	psy_ui_defaults_setstyle(defaults,
		STYLE_RECENTVIEW_MAINSECTIONHEADER, style);
	// header	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&onprimary, weak));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_make_overlay(1));
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_overlayed(&surface, &overlay, 0.06));
	psy_ui_style_setmargin(style,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
	psy_ui_style_setspacing(style,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));
	psy_ui_defaults_setstyle(defaults,
		STYLE_HEADER, style);	
}
