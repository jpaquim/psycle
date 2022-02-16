/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "styles.h"
/* host */
#include "resources/resource.h"
/* file */
#include <dir.h>
/* ui */
#include <uiappstyles.h>
#include <uimaterial.h>
#include <uiwintheme.h>
/* portable */
#include "../../detail/portable.h"

/* prototypes */
static void initlighttheme(psy_ui_Styles*);
static void initwin98theme(psy_ui_Styles*);

void inithoststyles(psy_ui_Styles* self, psy_ui_ThemeMode theme)
{
	psy_ui_MaterialTheme material;
	psy_ui_Style* style;

	if (theme == psy_ui_WIN98THEME) {
		initwin98theme(self);
		return;
	}
	if (theme == psy_ui_LIGHTTHEME) {
		initlighttheme(self);
		return;
	}
	self->theme = theme;
	psy_ui_materialtheme_init(&material, theme);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.overlay_12p);
	psy_ui_style_setpadding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_STATUSBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);
	psy_ui_style_setpadding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_9p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_style_setpadding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_HOVER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,  material.onsecondary,
		material.surface_overlay_9p);	
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_16p, 6.0);
	psy_ui_style_setpadding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW, style);

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
	psy_ui_style_setcolours(style, material.onprimary_strong,
		material.surface_overlay_7p);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_ITEM_SELECT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQBAR, style);

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
	psy_ui_style_setpadding_em(style, 0.25, 0.0, 0.25, 0.0);
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
	psy_ui_style_setcolour(style, material.surface_overlay_16p);
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_6p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_BOTTOM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&material.secondary, 900));	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_CURSOR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_11p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LEFT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_weak);	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_medium);	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_strong);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP_ACTIVE, style);

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

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_EDIT));
	psy_ui_styles_setstyle(self, STYLE_ZOOMBOX_EDIT, style);		

	style = psy_ui_style_allocinit();	
	psy_ui_style_setpadding_em(style, 0.25, 0.5, 0.5, 0.5);
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
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00292929));	
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE_ACTIVE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00CACACA));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE_SELECT, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_5p);
	psy_ui_styles_setstyle(self, STYLE_TOP, style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_TOPROWS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_8p);
	psy_ui_styles_setstyle(self, STYLE_TOPROW0, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 2.0, 0.5, 2.0);	
	psy_ui_styles_setstyle(self, STYLE_TOPROW1, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setpadding_em(style, 0.5, 2.0, 0.5, 2.0);	
	psy_ui_styles_setstyle(self, STYLE_TOPROW2, style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_setpadding_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.surface_overlay_24p);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPE, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 9);
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS_EDIT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_5p);
	if (psy_strlen(PSYCLE_RES_DIR) == 0) {
		psy_ui_style_setbackgroundid(style, IDB_ABOUT);
	} else {
		psy_ui_style_setbackgroundpath(style,
			PSYCLE_RES_DIR psy_SLASHSTR "splash_screen.bmp");
	}
	style->background.repeat = psy_ui_NOREPEAT;
	style->background.align = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_setstyle(self, STYLE_ABOUT, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_HEADER, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_BUTTONS, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_style_setbackgroundoverlay(style, 6);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 10.0);
	psy_ui_styles_setstyle(self, STYLE_PLAYBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_setstyle(self, STYLE_FILEBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_setstyle(self, STYLE_UNDOBAR, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_weak);
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_5p, 6);
	psy_ui_style_setpadding_em(style, 0.3, 0.3, 0.3, 0.3);
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
	psy_ui_style_setpadding_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		 psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.3));
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONS_HEADER, style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 2);	
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 1.0, 1.0);
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
	psy_ui_style_setpadding_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 8.0, 0.0, 0.0);	
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD, style);
	
	style = psy_ui_style_clone(style);	
	psy_ui_border_setcolour(&style->border, material.secondary);
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD_SELECT,
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.3, 0.0, 0.0, 0.0);	
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_RESCANBAR, style);	
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONBAR,
		psy_ui_style_clone(style));	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.2, 0.0, 0.2, 0.0);	
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SORTBAR, style);	
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_FILTERBAR,
		psy_ui_style_clone(style));		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_CATEGORYBAR, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 2.0);
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
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
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
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_RECENTVIEW_MAINSECTIONHEADER,
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_HEADER, style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.4, 0.0);
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

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_setcolour(style, psy_ui_colour_make_argb(0xFFF6B87F));
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON_WARNING, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
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

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW_ITEM_SELECTED, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_setstyle(self, STYLE_LPB_NUMLABEL, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_setstyle(self, STYLE_TIMEBAR_NUMLABEL, style);

	init_psycletheme_styles(self);
}

/* LightTheme */
void initlighttheme(psy_ui_Styles* self)
{
	psy_ui_MaterialTheme material;
	psy_ui_LightTheme light;
	psy_ui_Style* style;
	
	self->theme = psy_ui_LIGHTTHEME;
	psy_ui_lighttheme_init(&light);
	psy_ui_materialtheme_init(&material, self->theme);	

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTONS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_medium);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);
	psy_ui_style_setpadding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_style_setpadding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onsecondary, material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_16p, 6.0);
	psy_ui_style_setpadding_em(style, 0.25, 1.0, 0.25, 1.0);
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
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, psy_ui_colour_make(0xFFFFFF),
		light.cl_blue_3);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_ITEM_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_SEQEDT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_white_2, light.cl_blue_3);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_white_1, light.cl_blue_3);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_white_1, light.cl_blue_1);
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
	psy_ui_style_setpadding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_6p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_TOOLBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		material.surface_overlay_8p);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SPACER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_font_1, light.cl_white_1);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_TOP, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_font_1, light.cl_white_2);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_BOTTOM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_weighted(&material.secondary, 900));
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_CURSOR, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LEFT, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP_HOVER, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP_ACTIVE, style);

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

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_EDIT));
	psy_ui_styles_setstyle(self, STYLE_ZOOMBOX_EDIT, style);	

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.25, 0.5, 0.5, 0.5);
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
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00292929));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE_ACTIVE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00CACACA));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_font_1, light.cl_white_1);
	psy_ui_style_setpadding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_STATUSBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_5p);
	psy_ui_styles_setstyle(self, STYLE_TOP, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_TOPROWS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, light.cl_white_1);
	psy_ui_styles_setstyle(self, STYLE_TOPROW0, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_setstyle(self, STYLE_TOPROW1, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_setstyle(self, STYLE_TOPROW2, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_green_1, light.cl_black_1);
	psy_ui_style_setpadding_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, light.cl_green_1, light.cl_black_1);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 9);
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS_EDIT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_5p);
	psy_ui_style_setbackgroundid(style, IDB_ABOUT);
	style->background.repeat = psy_ui_NOREPEAT;
	style->background.align = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_setstyle(self, STYLE_ABOUT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_BUTTONS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, material.surface_overlay_9p);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_BUTTONS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_style_setbackgroundcolour(style, light.cl_white_2);	
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 10.0);
	psy_ui_styles_setstyle(self, STYLE_PLAYBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_setstyle(self, STYLE_FILEBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_setstyle(self, STYLE_UNDOBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, material.onprimary_weak);
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_5p, 6);
	psy_ui_style_setpadding_em(style, 0.3, 0.3, 0.3, 0.3);
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
	psy_ui_style_setpadding_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.3));
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONS_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 1.0, 1.0);
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
	psy_ui_style_setpadding_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 8.0, 0.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD, style);

	style = psy_ui_style_clone(style);
	psy_ui_border_setcolour(&style->border, material.secondary);
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD_SELECT,
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.3, 0.0, 0.0, 0.0);
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_RESCANBAR, style);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONBAR,
		psy_ui_style_clone(style));

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SORTBAR, style);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_FILTERBAR,
		psy_ui_style_clone(style));

	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_CATEGORYBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 2.0);
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
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
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
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_RECENTVIEW_MAINSECTIONHEADER,
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
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

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_setcolour(style, psy_ui_colour_make_argb(0xFFF6B87F));
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON_WARNING, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
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

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW_ITEM_SELECTED, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_setstyle(self, STYLE_LPB_NUMLABEL, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_setstyle(self, STYLE_TIMEBAR_NUMLABEL, style);

	init_psycletheme_styles(self);
}

/* Win98Theme */
void initwin98theme(psy_ui_Styles* self)
{
	psy_ui_WinTheme win;
	psy_ui_Style* style;

	self->theme = psy_ui_WIN98THEME;
	psy_ui_wintheme_init(&win);

	style = psy_ui_style_allocinit();	
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_style_setpadding_em(style, 0.2, 1.0, 0.2, 1.0);
	psy_ui_styles_setstyle(self, STYLE_STATUSBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTONS, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));	
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self,
		psy_ui_STYLE_BUTTON_HOVER));
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_HOVER, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self,
		psy_ui_STYLE_BUTTON_SELECT));	
	psy_ui_styles_setstyle(self, STYLE_SEQVIEW_BUTTON_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border, win.cl_blue);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_FOCUS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_border_init_solid(&style->border, win.cl_blue);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_navy);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_ITEM_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SEQLISTVIEW_TRACK_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_silver, win.cl_navy);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_navy);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_blue);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_ITEM_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_silver, win.cl_green);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SAMPLE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_green);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SAMPLE_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_lime);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SAMPLE_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_silver, win.cl_gray);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_MARKER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_gray);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_MARKER_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_medgray);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_MARKER_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID, win.cl_medgray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_TRACK_NUMBER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, win.cl_medgray);		
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_DESCITEM_SELECTED, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);		
	psy_ui_style_setpadding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_TOOLBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_medgray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_SPACER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_TOP, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_setcolours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_BOTTOM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, win.cl_navy);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_RULER_CURSOR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LEFT, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP_HOVER, style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_setstyle(self, STYLE_SEQEDT_LOOP_ACTIVE, style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_style_setbackgroundoverlay(style, 7);
	psy_ui_styles_setstyle(self, STYLE_MACHINEBOX, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_ZOOMBOX, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_EDIT));
	psy_ui_styles_setstyle(self, STYLE_ZOOMBOX_EDIT, style);		

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.25, 0.5, 0.5, 0.5);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_medgray);
	psy_ui_styles_setstyle(self, STYLE_MACHINEVIEW_PROPERTIES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_solid_radius(&style->border,
		win.cl_silver, 10.0);
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00292929));	
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE_ACTIVE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00CACACA));
	psy_ui_styles_setstyle(self, STYLE_STEPSEQUENCER_TILE_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		win.cl_silver);
	psy_ui_styles_setstyle(self, STYLE_TOP, style);

	style = psy_ui_style_allocinit();	
	psy_ui_border_setcolours(&style->border,
		win.cl_gray, win.cl_gray, win.cl_gray, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_TOPROWS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.0, 2.0, 0.0, 2.0);
	psy_ui_style_setborder(style, &win.raised);	
	psy_ui_styles_setstyle(self, STYLE_TOPROW0, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_TOPROW0));
	psy_ui_style_setpadding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_setstyle(self, STYLE_TOPROW1, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_TOPROW1));
	psy_ui_styles_setstyle(self, STYLE_TOPROW2, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_lime, win.cl_black);
	psy_ui_style_setpadding_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_style_setborder(style, &win.lowered);	
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_lime, win.cl_black);
	psy_ui_styles_setstyle(self, STYLE_TRACKSCOPE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_solid(&style->border, win.cl_medgray);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SONGPROPERTIES_COMMENTS_EDIT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setbackgroundid(style, IDB_ABOUT);
	style->background.repeat = psy_ui_NOREPEAT;
	style->background.align = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_setstyle(self, STYLE_ABOUT, style);

	style = psy_ui_style_allocinit();
	//psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SAMPLESVIEW_BUTTONS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_style_setpadding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_INSTRVIEW_BUTTONS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 0.5);	
	psy_ui_border_init_solid(&style->border, win.cl_black);		
	psy_ui_styles_setstyle(self, STYLE_PLAYBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_FILEBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_UNDOBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setborder(style, &win.lowered);	
	psy_ui_style_setpadding_em(style, 0.3, 0.3, 0.3, 0.3);
	psy_ui_styles_setstyle(self, STYLE_DURATION_TIME, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_gray, win.cl_medgray);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_gray, win.cl_medgray);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_ACTIVE, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_KEY));
	psy_ui_styles_setstyle(self, STYLE_KEY_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_gray, win.cl_medgray);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_gray, win.cl_green);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_SHIFT_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_gray, win.cl_lime);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_ALT_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_gray, win.cl_blue);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_setstyle(self, STYLE_KEY_CTRL_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_TABLEROW, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_medgray);
	psy_ui_styles_setstyle(self, STYLE_TABLEROW_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_TABLEROW_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_INPUTDEFINER_FOCUS, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_style_setpadding_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID, win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONS_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_border_init_solid(&style->border, win.cl_navy);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTION_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SEARCHBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, win.cl_black);
	psy_ui_border_init_solid(&style->border, win.cl_white);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_setpadding_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 8.0, 0.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD, style);

	style = psy_ui_style_clone(style);
	psy_ui_border_setcolour(&style->border, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_SEARCHFIELD_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.3, 0.0, 0.0, 0.0);
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_RESCANBAR, style);

	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SECTIONBAR,
		psy_ui_style_clone(style));

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_SORTBAR, style);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_FILTERBAR,
		psy_ui_style_clone(style));

	style = psy_ui_style_allocinit();
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_CATEGORYBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setpadding_em(style, 0.5, 0.5, 0.5, 2.0);
	psy_ui_styles_setstyle(self, STYLE_NEWMACHINE_DETAIL, style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_setcolour(style, win.cl_black);
	psy_ui_style_setbackgroundoverlay(style, 4);
	psy_ui_styles_setstyle(self, STYLE_PROPERTYVIEW_MAINSECTION,
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, win.cl_black);
	psy_ui_style_setbackgroundoverlay(style, 1);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_PROPERTYVIEW_MAINSECTIONHEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, win.cl_black);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_setstyle(self, STYLE_RECENTVIEW_MAINSECTION, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_RECENTVIEW_MAINSECTIONHEADER,
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_setmargin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_setpadding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_setstyle(self, STYLE_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_styles_setstyle(self, STYLE_MAINVIEWTOPBAR, style);	

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_NAVBAR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_CLIPBOX, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_red());
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_styles_setstyle(self, STYLE_CLIPBOX_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_LABELPAIR, style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_setstyle(self, STYLE_LABELPAIR_FIRST, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundoverlay(style, 2);
	psy_ui_styles_setstyle(self, STYLE_LABELPAIR_SECOND, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));	
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_setcolour(style, win.cl_yellow);	
	psy_ui_styles_setstyle(self, STYLE_TERM_BUTTON_WARNING, style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_setcolour(style, win.cl_red);	
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

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW, style);


	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_white);
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW_ITEM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_white, win.cl_navy);
	psy_ui_styles_setstyle(self, STYLE_PLUGINVIEW_ITEM_SELECTED, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_styles_setstyle(self, STYLE_LPB_NUMLABEL, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style, win.cl_black, win.cl_silver);
	psy_ui_style_setborder(style, &win.lowered);
	psy_ui_styles_setstyle(self, STYLE_TIMEBAR_NUMLABEL, style);
	
	init_psycletheme_styles(self);
}

void init_psycletheme_styles(psy_ui_Styles* self)
{
	init_machineview_styles(self);
	init_machineparam_styles(self);
	init_patternview_styles(self);
}

void init_machineview_styles(psy_ui_Styles* self)
{
	psy_ui_Style* style;	

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style,
		psy_ui_colour_make(0x005F5F5F));
	psy_ui_styles_setstyle(self, STYLE_MACHINEVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x005F5F5F),
		psy_ui_colour_make(0x00232323));
	psy_ui_style_setfont(style, "Tahoma", 12);
	psy_ui_styles_setstyle(self, STYLE_MACHINEVIEW_WIRES, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x005F5F5F),
		psy_ui_colour_make(0x00232323));
	psy_ui_style_setfont(style, "Tahoma", 16);
	psy_ui_styles_setstyle(self, STYLE_MACHINEVIEW_STACK, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x005F5F5F));	
	psy_ui_styles_setstyle(self, STYLE_MV_WIRE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x007F7F7F));
	psy_ui_styles_setstyle(self, STYLE_MV_WIRE_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x007F7F7F));
	psy_ui_styles_setstyle(self, STYLE_MV_WIRE_HOVER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolour(style, psy_ui_colour_make(0x005F5F5F));
	style->background.size = psy_ui_realsize_make(5.0, 5.0);
	style->background.size_set = TRUE;
	psy_ui_styles_setstyle(self, STYLE_MV_WIRE_POLY, style);
	
	/* master */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 138.0, 35.0);	
	psy_ui_style_set_background_position_px(style, 0.0, -52.0);
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00333333));
	psy_ui_styles_setstyle(self, STYLE_MV_MASTER, style);

	/* master:name */
	style = psy_ui_style_allocinit();
	psy_ui_style_setsize_px(style, 117.0, 15.0);
	psy_ui_style_setpadding_px(style, 3.0, 0, 0.0, 20.0);
	psy_ui_styles_setstyle(self, STYLE_MV_MASTER_NAME, style);

	/* generator */
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00B1C8B0),
		psy_ui_colour_make(0x002f3E25));
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 138.0, 52.0);
	psy_ui_style_set_background_position_px(style, 0.0, -87.0);	
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR, style);	

	/* generator:name */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(20.0, 3.0),
			psy_ui_size_make_px(117.0, 15.0)));	
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_NAME, style);

	/* generator:mute */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_MUTE, style);

	/* generator:mute:select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_set_background_position_px(style, -23.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_MUTE_SELECT, style);

	/* generator:solo */
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_SOLO, style);

	/* generator:solo:select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 17, 17);	
	psy_ui_style_set_background_position_px(style, -6.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_SOLO_SELECT, style);

	/* generator:vu */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(4.0, 20.0), psy_ui_size_make_px(129.0, 7.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_VU, style);

	/* generator:vu0 */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 129.0, 7);	
	psy_ui_style_set_background_position_px(style, 0.0, -156.0);
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_VU0, style);

	/* generator:vupeak */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 1, 7);	
	psy_ui_style_set_background_position_px(style, -108.0, -156.0);
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_VUPEAK, style);

	/* generator:pan */
	style = psy_ui_style_allocinit();		
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(6.0, 33.0), psy_ui_size_make_px(82.0 + 6.0, 13.0)));
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_PAN, style);

	/* generator:pan:slider */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 6.0, 13.0);
	psy_ui_style_set_background_position_px(style, -0.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(6.0, 13.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_GENERATOR_PAN_SLIDER, style);

	/* effect*/
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 138.0, 52.0);
	psy_ui_style_setcolours(style, psy_ui_colour_make(0x00D1C5B6),
		psy_ui_colour_make(0x003E2f25));
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT, style);

	/* effect:name */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(20.0, 3.0), psy_ui_size_make_px(117.0, 15.0)));
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_NAME, style);

	/* effect:mute */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_MUTE, style);

	/* effect:mute:select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_set_background_position_px(style, -23.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_MUTE_SELECT, style);

	/* effect:bypass */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_BYPASS, style);

	/* effect:bypass:select */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 17, 17);
	psy_ui_style_set_background_position_px(style, -6.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_BYPASS_SELECT, style);

	/* effect:vu */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(4.0, 20.0), psy_ui_size_make_px(129.0, 7.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_VU, style);

	/* effect:vu0 */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 129.0, 7.0);	
	psy_ui_style_set_background_position_px(style, 0.0, -163.0);	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_VU0, style);

	/* effect:vupeak */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 1.0, 7.0);	
	psy_ui_style_set_background_position_px(style, -96.0, -144.0);	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_VUPEAK, style);

	/* effect:pan */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(6.0, 33.0), psy_ui_size_make_px(82.0 + 3.0, 13.0)));	
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_PAN, style);

	/* effect:pan:slider */
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
	psy_ui_style_setsize_px(style, 6.0, 13.0);
	psy_ui_style_set_background_position_px(style, -0.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(6.0, 13.0)));
	psy_ui_styles_setstyle(self, STYLE_MV_EFFECT_PAN_SLIDER, style);	

	/* arrow */
	style = psy_ui_style_allocinit();	
	psy_ui_style_setbackgroundcolour(style,		
		psy_ui_colour_transparent());
	psy_ui_style_setsize_px(style, 138.0, 52.0);
	psy_ui_styles_setstyle(self, STYLE_MV_ARROW, style);

	/* check */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_CHECK, style);

	/* knob */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_KNOB, style);

	/* label */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_LABEL, style);

	/* header */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_HEADER, style);

	/* level */
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style,
		psy_ui_colour_transparent());
	psy_ui_styles_setstyle(self, STYLE_MV_LEVEL, style);
}

void init_machineparam_styles(psy_ui_Styles* self)
{
	psy_ui_Style* style;
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00B4B4B4),
		psy_ui_colour_make(0x00292929));
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_TITLE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00555555));
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_TOP, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00E7BD18),
		psy_ui_colour_make(0x00444444));
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_BOTTOM, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00555555));
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_TOP_ACTIVE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00E7BD18),
		psy_ui_colour_make(0x00292929));
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_BOTTOM_ACTIVE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_PARAMKNOB);
	psy_ui_style_animatebackground(style, 0,
		psy_ui_realsize_make(28.0, 28.0), TRUE);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_KNOB, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 30.0, 182.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_SLIDER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 22.0, 10.0);
	psy_ui_style_set_background_position_px(style, 0.0, -182.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_SLIDERKNOB, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 28.0, 28.0);
	psy_ui_style_set_background_position_px(style, -30.0, -118.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_SWITCHON, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 28.0, 28.0);
	psy_ui_style_set_background_position_px(style, -30.0, -90.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_SWITCHOFF, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 13.0, 13.0);
	psy_ui_style_set_background_position_px(style, -30.0, -159.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_CHECKON, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 13.0, 13.0);
	psy_ui_style_set_background_position_px(style, -30.0, -146.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_CHECKOFF, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 16.0, 90.0);
	psy_ui_style_set_background_position_px(style, -46.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_VUON, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_MIXERSKIN);
	psy_ui_style_setsize_px(style, 16.0, 90.0);
	psy_ui_style_set_background_position_px(style, -30.0, 0.0);
	psy_ui_styles_setstyle(self, STYLE_MACPARAM_VUOFF, style);	
}

void init_patternview_styles(psy_ui_Styles* self)
{
	psy_ui_Style* style;
	
	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundcolour(style, psy_ui_colour_make(0x00232323));
	psy_ui_styles_setstyle(self, STYLE_PATTERNVIEW, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 102.0, 23.0);
	psy_ui_style_set_background_position_px(style, -2.0, -0.0);
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 9, 17);
	psy_ui_style_set_background_position_px(style, 0.0, -23.0);
	psy_ui_style_setpadding_px(style, 3.0, 0.0, 0.0, 15.0);
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_DIGITX0, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 9, 17);
	psy_ui_style_set_background_position_px(style, 0.0, -23.0);
	psy_ui_style_setpadding_px(style, 3.0, 0.0, 0.0, 22.0);
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_DIGIT0X, style);

	style = psy_ui_style_allocinit();		
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(66.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_MUTE, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 17.0, 17.0);
	psy_ui_style_set_background_position_px(style, -79.0, -40.0);	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(66.0, 3.0), psy_ui_size_make_px(17.0, 17.0)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_MUTE_SELECT, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(47.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_SOLO, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 17.0, 17.0);
	psy_ui_style_set_background_position_px(style, -62, -40.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(47.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_SOLO_SELECT, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(85.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_RECORD, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 17.0, 17.0);
	psy_ui_style_set_background_position_px(style, -96.0, -40.0);	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(85.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_RECORD_SELECT, style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(5.0, 3.0), psy_ui_size_make_px(7, 17)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_PLAY, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setbackgroundid(style, IDB_HEADERSKIN);
	psy_ui_style_setsize_px(style, 8, 17);
	psy_ui_style_set_background_position_px(style, 0.0, -40.0);	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(3.0, 5.0), psy_ui_size_make_px(8, 17)));
	psy_ui_styles_setstyle(self, STYLE_PV_TRACK_HEADER_PLAY_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00CACACA),
		psy_ui_colour_make(0x003E3E3E));
	psy_ui_styles_setstyle(self, STYLE_PV_ROW, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_PV_ROW_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00CACACA),
		psy_ui_colour_make(0x00363636));
	psy_ui_styles_setstyle(self, STYLE_PV_ROWBEAT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_PV_ROWBEAT_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00CACACA),
		psy_ui_colour_make(0x00595959));
	psy_ui_styles_setstyle(self, STYLE_PV_ROW4BEAT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_PV_ROW4BEAT_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_setstyle(self, STYLE_PV_CURSOR, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_setstyle(self, STYLE_PV_CURSOR_SELECT, style);

	style = psy_ui_style_allocinit();
	psy_ui_style_setcolours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_setstyle(self, STYLE_PV_PLAYBAR, style);
}
