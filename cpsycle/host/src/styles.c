/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "styles.h"
/* host */
#include "resources/resource.h"
/* ui */
#include <uimaterial.h>

void inithoststyles(psy_ui_Styles* self, psy_ui_ThemeMode theme)
{
	psy_ui_MaterialTheme material;
	psy_ui_Style* style;

	self->theme = theme;
	psy_ui_materialtheme_init(&material, theme);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.overlay_12p);
	psy_ui_style_setspacing_em(style, 0.2, 1.0, 0.2, 1.0);
	psy_ui_styles_setstyle(self, STYLE_STATUSBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_medium);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);	
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onsecondary, material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_16p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium, material.surface_overlay_5p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW, style);

	style = psy_ui_style_allocinit();	
	psy_ui_border_init_solid_radius(&style->border,
		psy_ui_colour_weighted(&material.secondary, 900), 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_FOCUS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_strong, material.surface_overlay_5p);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_5p);	
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_strong, material.surface_overlay_7p);	
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onsecondary,
		material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onsecondary, material.light),
		psy_ui_colour_weighted(&material.secondary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onsecondary, material.accent),
		psy_ui_colour_weighted(&material.secondary, material.pale));
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM_HOVER, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onsecondary, material.accent),
		psy_ui_colour_weighted(&material.secondary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.light),
		psy_ui_colour_make(0x00475E38));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SAMPLE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_make(0x00475E38));
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SAMPLE_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_make(0x00576E48));
	psy_ui_border_init_solid_radius(&style->border,
		psy_ui_colour_make(0x00677E58), 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SAMPLE_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.light),
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_MARKER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_MARKER_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_weighted(&material.primary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_MARKER_SELECTED, style);
		
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_24p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_TRACK_NUMBER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_16p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_DESCITEM_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_5p);
	psy_ui_style_setspacing_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_6p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_TOOLBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_8p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SPACER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_8p);	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_TOP, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_6p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_BOTTOM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_11p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LEFT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_weighted(&material.primary, material.medium));
	psy_ui_styles_setstyle(self, STYLE_SEQ_TAB, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.primary, material.accent),
		material.surface);
	psy_ui_styles_setstyle(self, STYLE_SEQ_TAB_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onsecondary, material.surface);
	psy_ui_styles_setstyle(self, STYLE_SEQ_TAB_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&material.primary,
		material.weak));
	psy_ui_styles_setstyle(self, STYLE_SEQ_TAB_NEW, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.secondary,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.24));
	psy_ui_styles_setstyle(self, STYLE_SEQ_PROGRESS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&material.primary,
		material.medium));
	psy_ui_style_setbackgroundoverlay(style, 7);
	psy_ui_styles_setstyle(self, STYLE_MACHINEBOX, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_weighted(&material.primary, material.medium),
		material.overlay_4p);
	psy_ui_styles_setstyle(self, STYLE_ZOOMBOX, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_MACHINEVIEW, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_setspacing_em(style, 0.25, 0.5, 0.5, 0.5);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_styles_setstyle(self, STYLE_MACHINEVIEW_PROPERTIES, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.primary,
		material.surface_overlay_9p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_9p, 10.0);
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_5p);
	psy_ui_styles_setstyle(self, STYLE_TOP, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_TOPROWS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_8p);
	psy_ui_style_setspacing_em(style, 0.0, 2.0, 0.0, 2.0);
	psy_ui_styles_setstyle(self, STYLE_TOPROW0, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_setstyle(self, STYLE_TOPROW1, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setspacing_em(style, 0.5, 2.0, 0.0, 2.0);
	psy_ui_styles_setstyle(self, STYLE_TOPROW2, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface);
	psy_ui_style_setspacing_em(style, 1.0, 0.5, 0.0, 0.5);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.surface_overlay_24p,
		material.surface);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPE, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 9);
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_5p);
	psy_ui_style_setbackgroundid(style, IDB_ABOUT);
	style->backgroundrepeat = psy_ui_NOREPEAT;
	style->backgroundposition = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_setstyle(self, STYLE_ABOUT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_style_setbackgroundoverlay(style, 6);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 10.0);
	psy_ui_styles_setstyle(self, STYLE_PLAYBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_weak);
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_5p, 6);
	psy_ui_style_setspacing_em(style, 0.3, 0.3, 0.3, 0.3);
	psy_ui_styles_setstyle(self, STYLE_DURATION_TIME, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		material.surface_overlay_16p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_12p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_ACTIVE, style);
	
	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_KEY));	
	psy_ui_styles_setstyle(self, STYLE_KEY_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.secondary,
		psy_ui_colour_make_rgb(52, 53, 50));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(128, 172, 131));		
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_SHIFT_SELECT, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(28, 138, 103));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_ALT_SELECT, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(176, 173, 130));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_CTRL_SELECT, style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_TABLEROW, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_TABLEROW_HOVER, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_16p);
	psy_ui_styles_setstyle(self, STYLE_TABLEROW_SELECT, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_medium);
	psy_ui_style_setbackgroundoverlay(style, 6);
	psy_ui_styles_setstyle(self, STYLE_INPUTDEFINER_FOCUS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_style_setspacing_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		 psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.3));
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONS_HEADER, style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 2);	
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.secondary, 0.30));
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION_SELECTED, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.5, 0.0, 0.5, 0.0);	
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SEARCHBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.20));
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_style_setspacing_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 8.0, 0.0, 0.0);	
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD, style);
	
	style = psy_ui_style_clone(style);	
	psy_ui_border_setcolour(&style->border, material.secondary);
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD_SELECT,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.3, 0.0, 0.0, 0.0);	
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_RESCANBAR, style);	
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONBAR,
		psy_ui_style_clone(style));	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.2, 0.0, 0.2, 0.0);	
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SORTBAR, style);	
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_FILTERBAR,
		psy_ui_style_clone(style));		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_CATEGORYBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setspacing_em(style, 0.5, 0.5, 0.5, 2.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_DETAIL, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_setcolour(style, material.onprimary_medium);
	psy_ui_style_setbackgroundoverlay(style, 4);	
	psy_ui_styles_setstyle(self, STYLE_PROPERTYVIEW_MAINSECTION,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_weak);
	psy_ui_style_setbackgroundoverlay(style, 1);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_PROPERTYVIEW_MAINSECTIONHEADER,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_styles_setstyle(self, STYLE_RECENTVIEW_MAINSECTION, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_RECENTVIEW_MAINSECTIONHEADER,
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setspacing_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_HEADER, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_MAINVIEWTOPBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface);
	psy_ui_styles_setstyle(self, STYLE_NAVBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_12p);
	psy_ui_styles_setstyle(self, STYLE_CLIPBOX, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_red());
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_12p);
	psy_ui_styles_setstyle(self, STYLE_CLIPBOX_SELECT, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_LABELPAIR, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_LABELPAIR_FIRST, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_styles_setstyle(self, STYLE_LABELPAIR_SECOND, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make_argb(0x00F6B87F));
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON_WARNING, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x007966CF));
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON_ERROR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_BGMAIN);
	psy_ui_style_animatebackground(style, 20,
		psy_ui_realsize_make(96, 192), TRUE);
	psy_ui_styles_setstyle(self, STYLE_GREET, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_BGTOP);
	psy_ui_style_animatebackground(style, 40,
		psy_ui_realsize_make(128, 128), TRUE);
	psy_ui_styles_setstyle(self, STYLE_GREET_TOP, style);
}
