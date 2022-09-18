/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "styles.h"
/* host */
#include "resources/resource.h"
#include "bitmaps.h"
/* file */
#include <dir.h>
/* ui */
#include <uiappstyles.h>
#include <uimaterial.h>
#include <uiwintheme.h>
/* portable */
#include "../../detail/portable.h"

/* prototypes */
static void init_light_theme(psy_ui_Styles*);
static void init_win98_theme(psy_ui_Styles*);

void init_host_styles(psy_ui_Styles* self, psy_ui_ThemeMode theme)
{
	psy_ui_MaterialTheme material;
	psy_ui_Style* style;

	if (theme == psy_ui_WIN98THEME) {
		init_win98_theme(self);
		return;
	}
	if (theme == psy_ui_LIGHTTHEME) {
		init_light_theme(self);
		return;
	}
	self->theme_mode = theme;
	psy_ui_materialtheme_init(&material, theme);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.overlay_12p);
	//psy_ui_style_set_padding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_styles_set_style(self, STYLE_STATUSBAR, "statusbar", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_9p);	
	psy_ui_styles_set_style(self, STYLE_VIEWSTATUSBAR, "viewstatusbar", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTONS, "seqview_buttons",
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_7p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON, "seqview_button",
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_9p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON_HOVER,
		"seqview_button::hover", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,  material.onsecondary,
		material.surface_overlay_9p);	
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON_SELECT,
		"seqview_button::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW, "seqview", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_7p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW, "seqlistview", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_FOCUS, "seqlistview::focus",
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_strong,
		material.surface_overlay_5p);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_SELECT,
		"seqlistview::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_5p);	
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_TRACK, "seqlistview_track",
		style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style, material.onprimary_strong,
		material.surface_overlay_7p);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_TRACK_SELECT,
		"seqlistview_track::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_ITEM,
		"seqlistview_item", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_ITEM_SELECT,
		"seqlistview_item::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 1.0);
	// psy_ui_style_set_colours(style, psy_ui_colour_make(0xFFFFFF),
	//	psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_SEQBAR, "seqbar", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_5p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT, "seqedt", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style, material.onsecondary,
		material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TRACKS, "seqedt_tracks", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onsecondary, material.light),
		psy_ui_colour_weighted(&material.secondary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM, "seqedt_item", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onsecondary, material.accent),
		psy_ui_colour_weighted(&material.secondary, material.pale));
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM_HOVER, "seqedt_item::hover",
		style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onsecondary, material.accent),
		psy_ui_colour_weighted(&material.secondary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM_SELECTED,
		"seqedt_item::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.light),
		psy_ui_colour_make(0x00475E38));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE, "seqedt_sample", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_make(0x00475E38));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE_HOVER,
		"seqedt_sample::hover", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_make(0x00576E48));
	psy_ui_border_init_solid_radius(&style->border,
		psy_ui_colour_make(0x00677E58), 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE_SELECTED,
		"seqedt_sample::select",style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.light),
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER,
		"seqedt_marker", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER_HOVER,
		"seqedt_marker::hover", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_weighted(&material.primary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER_SELECTED,
		"seqedt_marker::select", style);
		
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_24p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TRACK_NUMBER,
		"seqedt_track_number", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_16p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_DESCITEM_SELECTED,
		"seqedt_descitem::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_5p);
	psy_ui_style_set_padding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_6p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TOOLBAR, "seqedt_toolbar",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_8p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SPACER, "style_seqedt_spacer",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_8p);	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_TOP, "seqedt_ruler_top",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.surface_overlay_16p);
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_6p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_BOTTOM,
		"seqedt_ruler_bottom", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_weighted(&material.secondary,
		900));	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_CURSOR,
		"seqedt_ruler_cursor", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_11p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LEFT, "seqedt_left", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP, "seqedt_loop", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP_HOVER,
		"seqedt_loop::hover", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_strong);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP_ACTIVE,
		"loop::active", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 0.5);	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_PROPERTIES,
		"seqedt_properties", style);	

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.secondary,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.15));
	psy_ui_styles_set_style(self, STYLE_SEQ_PROGRESS, "seq_progress", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_overlay(style, 7);
	psy_ui_styles_set_style(self, STYLE_MACHINEBOX, "machinebox", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.primary, material.medium),
		material.overlay_4p);
	psy_ui_styles_set_style(self, STYLE_ZOOMBOX, "zoombox", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_EDIT));
	psy_ui_styles_set_style(self, STYLE_ZOOMBOX_EDIT, "zoombox_edit", style);			
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.primary,
		material.surface_overlay_9p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_9p, 10.0);
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER,
		"stepsequencer", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00292929));	
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE,
		"stepsequencer_tile", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE_ACTIVE,
		"stepsequencer_tile::active", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00CACACA));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE_SELECT,
		"stepsequencer_tile::select", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_5p);
	psy_ui_styles_set_style(self, STYLE_TOP, "top", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 1.0);
	// psy_ui_style_set_background_id(style, IDB_GUI);
	// style->background.repeat = psy_ui_NOREPEAT;
	// style->background.align = psy_ui_ALIGNMENT_RIGHT;
	// psy_ui_bitmap_settransparency(&style->background.bitmap,
	// psy_ui_colour_white());
	psy_ui_styles_set_style(self, STYLE_TOPROWS, "toprows", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_8p);	
	psy_ui_styles_set_style(self, STYLE_TOPROW0, "toprow0", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_TOPROW1, "toprow1", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_TOPROW2, "toprow2", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_padding_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_styles_set_style(self, STYLE_TRACKSCOPES, "trackscopes", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.surface_overlay_24p);
	psy_ui_styles_set_style(self, STYLE_TRACKSCOPE, "trackscope", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 9);
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES, "songproperties",
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES_COMMENTS,
		"songproperties_comments", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES_COMMENTS_EDIT,
		"songproperties_comments_edit", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_5p);	
	psy_ui_style_set_background_id(style, IDB_ABOUT);
	style->background.repeat = psy_ui_NOREPEAT;
	style->background.align = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_set_style(self, STYLE_ABOUT, "style_about", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW, "samplesview", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_set_padding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW_HEADER,
		"samplesview_header", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW_BUTTONS,
		"samplesview_buttons", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW, "instrview", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_set_padding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW_HEADER,
		"instrview_header", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW_BUTTONS,
		"instrview_buttons", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 1.0);
	psy_ui_style_set_background_overlay(style, 6);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 10.0);
	psy_ui_styles_set_style(self, STYLE_PLAYBAR, "playbar", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_set_style(self, STYLE_FILEBAR, "filebar", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_set_style(self, STYLE_UNDOBAR, "undobar", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_5p, 6);
	psy_ui_style_set_padding_em(style, 0.3, 0.3, 0.3, 0.3);
	psy_ui_styles_set_style(self, STYLE_DURATION_TIME, "duration_time", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		material.surface_overlay_16p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY, "key", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_12p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_ACTIVE, "key::active", style);
	
	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_KEY));
	psy_ui_styles_set_style(self, STYLE_KEY_HOVER, "", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.secondary,
		psy_ui_colour_make_rgb(52, 53, 50));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_SELECT, "key::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(128, 172, 131));		
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_SHIFT_SELECT, "key_shift::select",
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(28, 138, 103));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_ALT_SELECT,
		"key_alt::select", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(176, 173, 130));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_CTRL_SELECT, "key_ctrl::select",
		style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_TABLEROW, "tablerow", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_TABLEROW_HOVER, "tablerow::hover",
		style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_16p);
	psy_ui_styles_set_style(self, STYLE_TABLEROW_SELECT, "tablerow::select",
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_style_set_background_overlay(style, 6);
	psy_ui_styles_set_style(self, STYLE_INPUTDEFINER_FOCUS,
		"inputdefiner::focus", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);	
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTIONS_HEADER,
		"newmachine_sections_header", style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION,
		"newmachine_section", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 2);	
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.secondary, 0.30));
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION_SELECTED,
		"newmachine_section::select", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION_HEADER,
		"newmachine_section::header", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.5, 0.0, 1.0, 0.0);	
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SEARCHBAR,
		"newmachine_searchbar", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.20));
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_style_set_padding_em(style, 0.3, 0.3, 0.3, 0.0);	
	psy_ui_styles_set_style(self, STYLE_SEARCHFIELD, "searchfield", style);
	
	style = psy_ui_style_clone(style);	
	psy_ui_border_setcolour(&style->border, material.secondary);
	psy_ui_styles_set_style(self, STYLE_SEARCHFIELD_SELECT,
		"searchfield::select", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 0.3, 0.0, 0.0, 0.0);	
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_RESCANBAR,
		"newmachine_rescanbar", style);	
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTIONBAR,
		"newmachine_sectionbar", psy_ui_style_clone(style));	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_FILTERBAR,
		"newmachine_filterbar", style);
		
	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_12p);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.0, 1.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_FILTERBAR_LABEL,
		"newmachine_filterbar_label", style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 2.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_DETAIL,
		"newmachine_detail", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		 psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.2));
	psy_ui_border_setradius_px(&style->border, 6.0);		 
	psy_ui_style_set_margin_em(style, 0.5, 0.0, 0.0, 0.0);
	psy_ui_style_set_padding_em(style, 0.0, 1.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_DETAIL_GROUP,
		"newmachine_detail_group", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_16p);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);	
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_MAINSECTION,
		"propertyview_mainsection", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);
	psy_ui_style_set_background_colour(style, material.surface_overlay_7p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_MAINSECTIONHEADER,
		"propertyview_mainsectionheader", style);	
		
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_SUBSECTION,
		"propertyview_subsection", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);	
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);	
	psy_ui_style_set_padding_em(style, 1.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_SUBSECTIONHEADER,
		"propertyview_subsectionheader", style);
		
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_SECTION,
		"propertyview_section", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);	
	psy_ui_style_set_padding_em(style, 1.0, 0.0, 0.5, 0.0);	
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_SECTIONHEADER,
		"propertyview_sectionheader", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);	
	psy_ui_styles_set_style(self, STYLE_GEAR, "gear", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);	
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_MAINSECTION,
		"recentview_mainsection", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_MAINSECTIONHEADER,
		"recentview_mainsectionheader", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_LINE,
		"recentview_line", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_colour(style, material.overlay_4p);
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_LINE_HOVER,
		"recentview_line::hover", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.secondary);	
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_LINE_SELECT,
		"recentview_line::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_style_set_padding_em(style, 0.125, 0.5, 0.125, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.4, 0.0);
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_HEADER, "header", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.4, 0.0);
	psy_ui_styles_set_style(self, STYLE_MAINVIEWTOPBAR,
		"mainviewtopbar", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface);
	psy_ui_styles_set_style(self, STYLE_NAVBAR, "navbar", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_12p);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.0, 1.0));
	psy_ui_styles_set_style(self, STYLE_CLIPBOX, "clipbox", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_red());
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_12p);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.0, 1.0));
	psy_ui_styles_set_style(self, STYLE_CLIPBOX_SELECT,
		"clipbox::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_size(style, psy_ui_size_make_em(25.0, 0.8));
	psy_ui_style_set_margin_em(style, 0.0, 0.5, 0.2, 0.0);
	psy_ui_styles_set_style(self, STYLE_MAIN_VU, "main_vu", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_LABELPAIR,
		"labelpair", style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_LABELPAIR_FIRST,
		"labelpair_first", style);	
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_styles_set_style(self, STYLE_LABELPAIR_SECOND,
		"labelpair_second", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON,
		"term_button", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_colour(style, psy_ui_colour_make_argb(0xFFF6B87F));
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON_WARNING,
		"term_button_warning", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x007966CF));
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON_ERROR,
		"term_button_error", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_BGMAIN);	
	psy_ui_style_animate_background(style, 20,
		psy_ui_realsize_make(96, 192), TRUE);
	psy_ui_styles_set_style(self, STYLE_GREET, "greet", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_BGTOP);	
	psy_ui_style_animate_background(style, 40,
		psy_ui_realsize_make(128, 128), TRUE);
	psy_ui_styles_set_style(self, STYLE_GREET_TOP, "greet_top", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW, "pluginview", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW_ITEM, "pluginview_item",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW_ITEM_SELECTED,
		"pluginview_item::select", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_set_style(self, STYLE_LPB_NUMLABEL,
		"lpb_numlabel", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_set_style(self, STYLE_TIMEBAR_NUMLABEL,
		"timebar_numlabel", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_7p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON,
		"dialog::button", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.overlay_12p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON_HOVER,
		"dialog_button::hover", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onsecondary,
		material.overlay_12p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON_SELECT,
		"dialog_button::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_12p);
	psy_ui_styles_set_style(self, STYLE_SCANTASK, "scantask", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0xFF5555));
	psy_ui_styles_set_style(self, STYLE_FILEBOX_DIR_PANE,
		"filebox_dir_pane", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_FILEBOX_FILE_PANE, "", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 0.3, 0.0, 0.0, 0.0);
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x55FF45));
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_FILEVIEW_DIRBAR,
		"fileview_dirbar", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 3.0, 0.5, 0.0);	
	psy_ui_styles_set_style(self, STYLE_FILEVIEW_FILTERS,
		"fileview_filters", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 2.0, 0.5, 2.0);	
	psy_ui_styles_set_style(self, STYLE_FILEVIEW_BUTTONS,
		"fileview_buttons", style);
	
	style = psy_ui_style_allocinit();		
	psy_ui_style_set_padding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_style_set_colour(style, psy_ui_colour_make(0xFF5555));
	psy_ui_styles_set_style(self, STYLE_FILEVIEW_LINKS,
		"fileview_links", style);
	
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.0, 0.0);	
	psy_ui_styles_set_style(self, STYLE_FILEVIEW_OPTIONS,
		"fileview_options", style);
	
	style = psy_ui_style_allocinit();		
	// psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
	//	material.surface_overlay_12p);
	// psy_ui_style_set_margin_em(style, 1.6, 0.0, 1.6, 0.0);
	psy_ui_style_set_size(style, psy_ui_size_make_em(0.0, 0.1));
	psy_ui_styles_set_style(self, STYLE_SEPARATOR, "separator", style);
	
	init_envelope_styles(self);
	init_psycletheme_styles(self);
}

/* LightTheme */
void init_light_theme(psy_ui_Styles* self)
{
	psy_ui_MaterialTheme material;
	psy_ui_LightTheme light;
	psy_ui_Style* style;
	
	self->theme_mode = psy_ui_LIGHTTHEME;
	psy_ui_lighttheme_init(&light);
	psy_ui_materialtheme_init(&material, self->theme_mode);	

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTONS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onsecondary, material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_16p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium, material.surface_overlay_5p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid_radius(&style->border,
		psy_ui_colour_weighted(&material.secondary, 900), 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_FOCUS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_strong, material.surface_overlay_5p);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_SELECT, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_TRACK, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_TRACK_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_ITEM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_make(0xFFFFFF),
		light.cl_blue_3);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_ITEM_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_SEQEDT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_white_2, light.cl_blue_3);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_white_1, light.cl_blue_3);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_white_1, light.cl_blue_1);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.light),
		psy_ui_colour_make(0x00475E38));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_make(0x00475E38));
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_make(0x00576E48));
	psy_ui_border_init_solid_radius(&style->border,
		psy_ui_colour_make(0x00677E58), 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.light),
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.onprimary, material.accent),
		psy_ui_colour_weighted(&material.primary, material.pale));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 6.0);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_24p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TRACK_NUMBER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_16p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_DESCITEM_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_5p);
	psy_ui_style_set_padding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_6p);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TOOLBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		material.surface_overlay_8p);
	psy_ui_style_set_size(style, psy_ui_size_make_em(0.0, 0.25));
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SPACER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_font_1, light.cl_white_1);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_TOP, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_font_1, light.cl_white_2);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_BOTTOM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_weighted(&material.secondary, 900));
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_CURSOR, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LEFT, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP_HOVER, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP_ACTIVE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.secondary,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.24));
	psy_ui_styles_set_style(self, STYLE_SEQ_PROGRESS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_weighted(&material.primary,
		material.medium));
	psy_ui_style_set_background_overlay(style, 7);
	psy_ui_styles_set_style(self, STYLE_MACHINEBOX, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(&material.primary, material.medium),
		material.overlay_4p);
	psy_ui_styles_set_style(self, STYLE_ZOOMBOX, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_EDIT));
	psy_ui_styles_set_style(self, STYLE_ZOOMBOX_EDIT, "", style);		

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.primary,
		material.surface_overlay_9p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_9p, 10.0);
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00292929));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE_ACTIVE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00CACACA));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_font_1, light.cl_white_1);
	//psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_STATUSBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_5p);
	psy_ui_styles_set_style(self, STYLE_TOP, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_TOPROWS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, light.cl_white_1);
	psy_ui_styles_set_style(self, STYLE_TOPROW0, "", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_set_style(self, STYLE_TOPROW1, "", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_set_style(self, STYLE_TOPROW2, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_green_1, light.cl_black_1);
	psy_ui_style_set_padding_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_styles_set_style(self, STYLE_TRACKSCOPES, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, light.cl_green_1, light.cl_black_1);
	psy_ui_styles_set_style(self, STYLE_TRACKSCOPE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 9);
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES_COMMENTS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES_COMMENTS_EDIT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_5p);
	psy_ui_style_set_background_id(style, IDB_ABOUT);
	style->background.repeat = psy_ui_NOREPEAT;
	style->background.align = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_set_style(self, STYLE_ABOUT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW_BUTTONS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_16p);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, material.surface_overlay_9p);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW_BUTTONS, "", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_style_set_background_colour(style, light.cl_white_2);	
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_11p, 10.0);
	psy_ui_styles_set_style(self, STYLE_PLAYBAR, "", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_set_style(self, STYLE_FILEBAR, "", style);

	style = psy_ui_style_allocinit();
	// psy_ui_style_set_padding_em(style, 0.6, 0.0, 0.6, 1.0);
	psy_ui_styles_set_style(self, STYLE_UNDOBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_5p, 6);
	psy_ui_style_set_padding_em(style, 0.3, 0.3, 0.3, 0.3);
	psy_ui_styles_set_style(self, STYLE_DURATION_TIME, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		material.surface_overlay_16p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_12p);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_ACTIVE, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_KEY));
	psy_ui_styles_set_style(self, STYLE_KEY_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.secondary,
		psy_ui_colour_make_rgb(52, 53, 50));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(128, 172, 131));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_SHIFT_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(28, 138, 103));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_ALT_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		psy_ui_colour_make_rgb(176, 173, 130));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_16p, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_CTRL_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_9p);
	psy_ui_styles_set_style(self, STYLE_TABLEROW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_TABLEROW_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_16p);
	psy_ui_styles_set_style(self, STYLE_TABLEROW_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_style_set_background_overlay(style, 6);
	psy_ui_styles_set_style(self, STYLE_INPUTDEFINER_FOCUS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_style_set_padding_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.3));
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTIONS_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.secondary, 0.30));
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SEARCHBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.20));
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_set_padding_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_set_margin_em(style, 0.0, 8.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_SEARCHFIELD, "", style);

	style = psy_ui_style_clone(style);
	psy_ui_border_setcolour(&style->border, material.secondary);
	psy_ui_styles_set_style(self, STYLE_SEARCHFIELD_SELECT, "",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 0.3, 0.0, 0.0, 0.0);
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_RESCANBAR, "", style);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTIONBAR, "",
		psy_ui_style_clone(style));

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_FILTERBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_FILTERBAR_LABEL, "",
		style);		

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 2.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_DETAIL, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_MAINSECTION, "",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);
	psy_ui_style_set_background_overlay(style, 1);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_MAINSECTIONHEADER, "",
		style);
		
	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_SUBSECTION, "",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_weak);
	psy_ui_style_set_background_overlay(style, 1);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_SUBSECTIONHEADER, "",
		style);		

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid(&style->border, material.surface_overlay_7p);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_MAINSECTION, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_MAINSECTIONHEADER, "",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_blue(),
		psy_ui_colour_blue());
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_LINE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_blue(),
		psy_ui_colour_blue());
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_LINE_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_blue(),
		psy_ui_colour_blue());
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_LINE_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_weak,
		material.overlay_1p);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		material.surface_overlay_7p);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_MAINVIEWTOPBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID,
		material.surface);
	psy_ui_styles_set_style(self, STYLE_NAVBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_12p);
	psy_ui_styles_set_style(self, STYLE_CLIPBOX, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_red());
	psy_ui_border_init_solid(&style->border,
		material.surface_overlay_12p);
	psy_ui_styles_set_style(self, STYLE_CLIPBOX_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_LABELPAIR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_LABELPAIR_FIRST, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_styles_set_style(self, STYLE_LABELPAIR_SECOND, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_colour(style, psy_ui_colour_make_argb(0xFFF6B87F));
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON_WARNING, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x007966CF));
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON_ERROR, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_BGMAIN);	
	psy_ui_style_animate_background(style, 20,
		psy_ui_realsize_make(96, 192), TRUE);
	psy_ui_styles_set_style(self, STYLE_GREET, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_BGTOP);	
	psy_ui_style_animate_background(style, 40,
		psy_ui_realsize_make(128, 128), TRUE);
	psy_ui_styles_set_style(self, STYLE_GREET_TOP, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW_ITEM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW_ITEM_SELECTED, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_set_style(self, STYLE_LPB_NUMLABEL, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_LABEL));
	psy_ui_styles_set_style(self, STYLE_TIMEBAR_NUMLABEL, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_7p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_11p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onsecondary, material.overlay_4p);
	psy_ui_border_init_solid_radius(&style->border, material.surface_overlay_16p, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON_SELECT, "", style);

	init_envelope_styles(self);
	init_psycletheme_styles(self);
}

/* Win98Theme */
void init_win98_theme(psy_ui_Styles* self)
{
	psy_ui_WinTheme win;
	psy_ui_Style* style;

	self->theme_mode = psy_ui_WIN98THEME;
	psy_ui_wintheme_init(&win);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_style_set_padding_em(style, 0.2, 1.0, 0.2, 1.0);
	psy_ui_styles_set_style(self, STYLE_STATUSBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTONS, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));	
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self,
		psy_ui_STYLE_BUTTON_HOVER));
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON_HOVER, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self,
		psy_ui_STYLE_BUTTON_SELECT));	
	psy_ui_styles_set_style(self, STYLE_SEQVIEW_BUTTON_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border, win.cl_blue);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_FOCUS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_border_init_solid(&style->border, win.cl_blue);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_TRACK, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_ITEM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_navy);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_ITEM_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SEQLISTVIEW_TRACK_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SEQEDT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_silver, win.cl_navy);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_navy);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_blue);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_ITEM_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_silver, win.cl_green);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_green);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_lime);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SAMPLE_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_silver, win.cl_gray);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_gray);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_medgray);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_MARKER_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_right(&style->border, psy_ui_BORDER_SOLID, win.cl_medgray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TRACK_NUMBER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, win.cl_medgray);		
	psy_ui_styles_set_style(self, STYLE_SEQEDT_DESCITEM_SELECTED, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);		
	psy_ui_style_set_padding_em(style, 0.25, 0.0, 0.25, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_TOOLBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_medgray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_SPACER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_TOP, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_BOTTOM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, win.cl_navy);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_RULER_CURSOR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LEFT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP_HOVER, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_SEQEDT_LOOP_ACTIVE, "", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_style_set_background_overlay(style, 7);
	psy_ui_styles_set_style(self, STYLE_MACHINEBOX, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_ZOOMBOX, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_EDIT));
	psy_ui_styles_set_style(self, STYLE_ZOOMBOX_EDIT, "", style);	

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_solid_radius(&style->border,
		win.cl_silver, 10.0);
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00292929));	
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE_ACTIVE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00CACACA));
	psy_ui_styles_set_style(self, STYLE_STEPSEQUENCER_TILE_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID,
		win.cl_silver);
	psy_ui_styles_set_style(self, STYLE_TOP, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_border_setcolours(&style->border,
		win.cl_gray, win.cl_gray, win.cl_gray, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_TOPROWS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.0, 2.0, 0.0, 2.0);
	psy_ui_style_set_border(style, &win.raised);	
	psy_ui_styles_set_style(self, STYLE_TOPROW0, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_TOPROW0));
	psy_ui_style_set_padding_em(style, 0.5, 2.0, 0.5, 2.0);
	psy_ui_styles_set_style(self, STYLE_TOPROW1, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_TOPROW1));
	psy_ui_styles_set_style(self, STYLE_TOPROW2, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_lime, win.cl_black);
	psy_ui_style_set_padding_em(style, 0.25, 0.5, 0.25, 0.5);
	psy_ui_style_set_border(style, &win.lowered);	
	psy_ui_styles_set_style(self, STYLE_TRACKSCOPES, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_lime, win.cl_black);
	psy_ui_styles_set_style(self, STYLE_TRACKSCOPE, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 0.5);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_solid(&style->border, win.cl_medgray);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES_COMMENTS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SONGPROPERTIES_COMMENTS_EDIT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_background_id(style, IDB_ABOUT);
	style->background.repeat = psy_ui_NOREPEAT;
	style->background.align = psy_ui_ALIGNMENT_CENTER;
	psy_ui_styles_set_style(self, STYLE_ABOUT, "", style);

	style = psy_ui_style_allocinit();
	//psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_SAMPLESVIEW_BUTTONS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_INSTRVIEW_BUTTONS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 0.5);	
	psy_ui_border_init_solid(&style->border, win.cl_black);		
	psy_ui_styles_set_style(self, STYLE_PLAYBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_FILEBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_UNDOBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_border(style, &win.lowered);	
	psy_ui_style_set_padding_em(style, 0.3, 0.3, 0.3, 0.3);
	psy_ui_styles_set_style(self, STYLE_DURATION_TIME, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_gray, win.cl_medgray);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_gray, win.cl_medgray);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_ACTIVE, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_KEY));
	psy_ui_styles_set_style(self, STYLE_KEY_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_gray, win.cl_medgray);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_gray, win.cl_green);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_SHIFT_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_gray, win.cl_lime);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_ALT_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_gray, win.cl_blue);
	psy_ui_border_init_solid_radius(&style->border, win.cl_gray, 4.0);
	psy_ui_styles_set_style(self, STYLE_KEY_CTRL_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_TABLEROW, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_medgray);
	psy_ui_styles_set_style(self, STYLE_TABLEROW_HOVER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_TABLEROW_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_INPUTDEFINER_FOCUS, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_style_set_padding_em(style, 1.0, 0.0, 0.0, 0.0);
	psy_ui_border_init_top(&style->border, psy_ui_BORDER_SOLID, win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTIONS_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 1.0, 1.0);
	psy_ui_border_init_solid(&style->border, win.cl_navy);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 1.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTION_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_margin_em(style, 0.5, 0.0, 0.5, 0.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SEARCHBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, win.cl_black);
	psy_ui_border_init_solid(&style->border, win.cl_white);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_set_padding_em(style, 0.3, 0.3, 0.3, 0.0);
	psy_ui_style_set_margin_em(style, 0.0, 8.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_SEARCHFIELD, "", style);

	style = psy_ui_style_clone(style);
	psy_ui_border_setcolour(&style->border, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_SEARCHFIELD_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 0.3, 0.0, 0.0, 0.0);
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_RESCANBAR, "", style);

	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_SECTIONBAR, "",
		psy_ui_style_clone(style));

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.2, 0.0, 0.2, 0.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.0, 3.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_FILTERBAR, "", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_FILTERBAR_LABEL, "",
		style);		

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_em(style, 0.5, 0.5, 0.5, 2.0);
	psy_ui_styles_set_style(self, STYLE_NEWMACHINE_DETAIL, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_style_set_colour(style, win.cl_black);
	psy_ui_style_set_background_overlay(style, 4);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_MAINSECTION, "",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, win.cl_black);
	psy_ui_style_set_background_overlay(style, 1);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_PROPERTYVIEW_MAINSECTIONHEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, win.cl_black);
	psy_ui_border_init_solid(&style->border, win.cl_gray);
	psy_ui_border_setradius_px(&style->border, 6.0);
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_MAINSECTION, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_RECENTVIEW_MAINSECTIONHEADER, "",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_style_set_margin_em(style, 0.0, 0.0, 0.5, 0.0);
	psy_ui_style_set_padding_em(style, 0.5, 0.0, 0.5, 1.0);
	psy_ui_styles_set_style(self, STYLE_HEADER, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_styles_set_style(self, STYLE_MAINVIEWTOPBAR, "", style);	

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_NAVBAR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_CLIPBOX, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_red());
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		win.cl_gray);
	psy_ui_styles_set_style(self, STYLE_CLIPBOX_SELECT, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_LABELPAIR, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_LABELPAIR_FIRST, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_overlay(style, 2);
	psy_ui_styles_set_style(self, STYLE_LABELPAIR_SECOND, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));	
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_colour(style, win.cl_yellow);	
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON_WARNING, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_colour(style, win.cl_red);	
	psy_ui_styles_set_style(self, STYLE_TERM_BUTTON_ERROR, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_BGMAIN);	
	psy_ui_style_animate_background(style, 20,
		psy_ui_realsize_make(96, 192), TRUE);
	psy_ui_styles_set_style(self, STYLE_GREET, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_BGTOP);	
	psy_ui_style_animate_background(style, 40,
		psy_ui_realsize_make(128, 128), TRUE);
	psy_ui_styles_set_style(self, STYLE_GREET_TOP, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW, "", style);


	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_white);
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW_ITEM, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_white, win.cl_navy);
	psy_ui_styles_set_style(self, STYLE_PLUGINVIEW_ITEM_SELECTED, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_styles_set_style(self, STYLE_LPB_NUMLABEL, "", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, win.cl_black, win.cl_silver);
	psy_ui_style_set_border(style, &win.lowered);
	psy_ui_styles_set_style(self, STYLE_TIMEBAR_NUMLABEL, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self,
		psy_ui_STYLE_BUTTON_HOVER));
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON_HOVER, "", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self,
		psy_ui_STYLE_BUTTON_SELECT));
	psy_ui_styles_set_style(self, STYLE_DIALOG_BUTTON_SELECT, "", style);
	
	init_envelope_styles(self);
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
	psy_ui_styles_set_style(self, STYLE_MV, "mv", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_WIRES, "mv_wires", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_STACK, "mv_stack", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_PROPERTIES, "mv_properties", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_NEWMACHINE, "mv_newmachine", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_WIRE, "mv_wire", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_WIRE_SELECT, "mv_wire::select",
		style);
		
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_WIRE_HOVER, "mv_wire::hover", style);
		
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_WIRE_POLY, "mv_wire_poly", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_MASTER, "mv_master", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_MASTER_NAME, "mv_master_name", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR, "mv_generator", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_NAME, "mv_generator_name",
		style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_MUTE, "mv_generator_mute",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_MUTE_SELECT,
		"mv_generator_mute::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_SOLO,
		"mv_generator_solo", style);

	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_SOLO_SELECT,
		"mv_generator_solo::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_VU,
		"mv_generator_vu", style);	
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_VU0,
		"mv_generator_vu0", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_VUPEAK,
		"mv_generator_vupeak", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_PAN,
		"mv_generator_pan", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_GENERATOR_PAN_SLIDER,
		"mv_generator_pan_slider", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT,
		"mv_effect", style);		
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_NAME,
		"mv_effect_name", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_MUTE,
		"mv_effect_mute", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_MUTE_SELECT,
		"mv_effect_mute::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_BYPASS,
		"mv_effect_bypass", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_BYPASS_SELECT,
		"effect_bypass::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_VU,
		"mv_effect_vu", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_VU0,
		"mv_effect_vu0", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_VUPEAK,
		"mv_effect_vupeak", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_PAN,
		"mv_effect_pan", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_EFFECT_PAN_SLIDER,
		"mv_effect_pan_slider", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_ARROW, "mv_arrow", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_CHECK, "mv_check", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_KNOB, "mv_knob", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_LABEL, "mv_label", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_HEADER, "mv_header", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, STYLE_MV_LEVEL, "mv_level", style);
			
	/* configuration in machineviewconfig */
}

void init_machineparam_styles(psy_ui_Styles* self)
{
	psy_ui_Style* style;
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00B4B4B4),
		psy_ui_colour_make(0x00292929));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_TITLE,
		"macparam_title", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00555555));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_TOP,
		"macparam_top", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00E7BD18),
		psy_ui_colour_make(0x00444444));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_BOTTOM,
		"macparam_bottom", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00555555));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_TOP_ACTIVE,
		"macparam_top::active", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00E7BD18),
		psy_ui_colour_make(0x00292929));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_BOTTOM_ACTIVE,
		"macparam_bottom::active", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_PARAMKNOB);	
	psy_ui_style_animate_background(style, 0,
		psy_ui_realsize_make(28.0, 28.0), TRUE);
	psy_ui_styles_set_style(self, STYLE_MACPARAM_KNOB,
		"macparam_knob", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(30.0, 182.0));
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00E7BD18),
		psy_ui_colour_make(0x00292929));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_SLIDER,
		"macparam_slider", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(22.0, 10.0));
	psy_ui_style_set_background_position_px(style, 0.0, -182.0);
	psy_ui_styles_set_style(self, STYLE_MACPARAM_SLIDERKNOB,
		"macparam_sliderknob", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(28.0, 28.0));
	psy_ui_style_set_background_position_px(style, -30.0, -118.0);
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00444444));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_SWITCHON,
		"macparam_switchon", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(28.0, 28.0));
	psy_ui_style_set_background_position_px(style, -30.0, -90.0);
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00444444));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_SWITCHOFF,
		"macparam_switchoff", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(13.0, 13.0));
	psy_ui_style_set_background_position_px(style, -30.0, -159.0);
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00E7BD18),
		psy_ui_colour_make(0x00444444));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_CHECKON,
		"macparam_checkon", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(13.0, 13.0));
	psy_ui_style_set_background_position_px(style, -30.0, -146.0);
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00CDCDCD),
		psy_ui_colour_make(0x00444444));
	psy_ui_styles_set_style(self, STYLE_MACPARAM_CHECKOFF,
		"macparam_checkoff", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(16.0, 90.0));
	psy_ui_style_set_background_position_px(style, -46.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_MACPARAM_VUON,
		"macparam_vuon", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_MIXERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(16.0, 90.0));
	psy_ui_style_set_background_position_px(style, -30.0, 0.0);
	psy_ui_styles_set_style(self, STYLE_MACPARAM_VUOFF,
		"macparam_vuoff", style);
}

void init_patternview_styles(psy_ui_Styles* self)
{
	psy_ui_Style* style;
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00CACACA),
		psy_ui_colour_make(0x00292929));
	psy_ui_styles_set_style(self, STYLE_PATTERNVIEW,
		"style_patternview", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_padding_px(style, 0.0, 0.0, 0.0, 1.0);	
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_VIEW,
		"pv_track_view", style);

	style = psy_ui_style_allocinit();	
	psy_ui_border_init_left(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_make(0x00808080));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_VIEW_SELECT,
		"pv_track_view::select", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	style->background.repeat = psy_ui_NOREPEAT;	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(102.0, 23.0));
	psy_ui_style_set_background_position_px(style, -2.0, -0.0);
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER,
		"pv_track_header", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);	
	style->background.repeat = psy_ui_NOREPEAT;
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(9, 17));
	psy_ui_style_set_background_position_px(style, 0.0, -23.0);
	psy_ui_style_set_padding_px(style, 3.0, 0.0, 0.0, 15.0);
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_DIGITX0,
		"pv_track_header_digitx0", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	style->background.repeat = psy_ui_NOREPEAT;
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(9, 17));
	psy_ui_style_set_background_position_px(style, 0.0, -23.0);
	psy_ui_style_set_padding_px(style, 3.0, 0.0, 0.0, 22.0);
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_DIGIT0X,
		"pv_track_header_digit0x", style);

	style = psy_ui_style_allocinit();		
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(66.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_MUTE,
		"pv_track_header_mute", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);	
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(17.0, 17.0));
	psy_ui_style_set_background_position_px(style, -79.0, -40.0);	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(66.0, 3.0), psy_ui_size_make_px(17.0, 17.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_MUTE_SELECT,
		"pv_track_header_mute::select", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(47.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_SOLO,
		"pv_track_header::solo", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	style->background.repeat = psy_ui_NOREPEAT;
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(17.0, 17.0));
	psy_ui_style_set_background_position_px(style, -62, -40.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(47.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_SOLO_SELECT,
		"pv_track_header_solo::select", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(85.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_RECORD, "", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);	
	style->background.repeat = psy_ui_NOREPEAT;
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(17.0, 17.0));
	psy_ui_style_set_background_position_px(style, -96.0, -40.0);	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(85.0, 3.0), psy_ui_size_make_px(17.0, 17.)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_RECORD_SELECT,
		"pv_track_header_record::select", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(5.0, 3.0), psy_ui_size_make_px(8.0, 17.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_PLAY,
		"pv_track_header_play", style);

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	style->background.repeat = psy_ui_NOREPEAT;
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(8.0, 17.0));
	psy_ui_style_set_background_position_px(style, 0.0, -40.0);	
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(5.0, 3.0), psy_ui_size_make_px(8.0, 17.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_PLAY_SELECT,
		"pv_track_header_play::select", style);

	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_HEADER_TEXT,
		"pv_track_header_text", style);

	/* copy to classic */
	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER,
		"pv_track_classic_header", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_DIGITX0));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_DIGITX0,
		"pv_track_classic_header_digitx0", style);	

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_DIGIT0X));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_DIGIT0X,
		"pv_track_classic_header_digit0x", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_MUTE));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_MUTE,
		"pv_track_classic_header_mute", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_MUTE_SELECT));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_MUTE_SELECT,
		"pv_track_classic_header_mute::select", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_SOLO));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_SOLO,
		"pv_track_classic_header_solo", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_SOLO_SELECT));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_SOLO_SELECT,
		"pv_track_classic_header_solo::select", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_RECORD));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_RECORD,
		"pv_track_classic_header_record", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_RECORD_SELECT));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_RECORD_SELECT,
		"pv_track_classic_header_record::select", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_PLAY));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_PLAY,
		"pv_track_classic_header_play", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_PLAY_SELECT));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_PLAY_SELECT,
		"pv_track_classic_header_play::select", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_TEXT));	
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_CLASSIC_HEADER_TEXT,
		"pv_track_classic_header_text", style);

	/* text */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(102.0, 23.0));
	psy_ui_style_set_background_position_px(style, -2.0, -57.0);
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER,
		"pv_track_text_header", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(6, 12));
	psy_ui_style_set_background_position_px(style, -0.0, -80.0);
	psy_ui_style_set_padding_px(style, 8.0, 0.0, 0.0, 5.0);
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_DIGITX0,
		"pv_track_text_header_digitx0", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(6, 12));
	psy_ui_style_set_background_position_px(style, 0.0, -80.0);
	psy_ui_style_set_padding_px(style, 8.0, 0.0, 0.0, 11.0);
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_DIGIT0X,
		"pv_track_text_header_digit0x", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(66.0, 3.0), psy_ui_size_make_px(17.0, 4.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_MUTE,
		"pv_track_text_header_mute", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(17.0, 4.0));
	psy_ui_style_set_background_position_px(style, -29.0 - 2, -92.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(66.0, 3.0), psy_ui_size_make_px(17.0, 4.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_MUTE_SELECT,
		"pv_track_text_header_mute::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(47.0, 3.0), psy_ui_size_make_px(17.0, 4.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_SOLO,
		"pv_track_text_header_solo", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(17.0 - 2, 4.0));
	psy_ui_style_set_background_position_px(style, -12.0, -92.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(47.0, 3.0), psy_ui_size_make_px(17.0, 4.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_SOLO_SELECT,
		"pv_track_text_header_solo::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(85.0, 3.0), psy_ui_size_make_px(17.0, 4.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_RECORD,
		"pv_track_text_header_record", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_id(style, IDB_HEADERSKIN);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(17.0, 4.0));
	psy_ui_style_set_background_position_px(style, -46.0, -92.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(85.0, 3.0), psy_ui_size_make_px(17.0, 4.0)));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_RECORD_SELECT,
		"pv_track_text_header_record::select", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_PLAY));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_PLAY,
		"pv_track_text_header_play", style);

	style = psy_ui_style_clone(psy_ui_styles_at(self, STYLE_PV_TRACK_HEADER_PLAY_SELECT));
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_PLAY_SELECT,
		"pv_track_text_header_play::select", style);
	
	style = psy_ui_style_allocinit();
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(18.0, 8.0), psy_ui_size_make_px(84.0, 13.0)));
#if defined(DIVERSALIS__OS__UNIX)
		psy_ui_style_set_font(style, "FreeSans", 10);
#else
		psy_ui_style_set_font(style, "Tahoma", 12);
#endif
	psy_ui_styles_set_style(self, STYLE_PV_TRACK_TEXT_HEADER_TEXT,
		"pv_track_text_header_text", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,		
		psy_ui_colour_make(0x003E3E3E));
	psy_ui_styles_set_style(self, STYLE_PV_ROW, "pv_row", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_PV_ROW_SELECT, "pv_row::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		psy_ui_colour_make(0x00363636));
	psy_ui_styles_set_style(self, STYLE_PV_ROWBEAT, "pv_rowbeat", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_PV_ROWBEAT_SELECT,
		"pv_rowbeat::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		psy_ui_colour_make(0x00595959));
	psy_ui_styles_set_style(self, STYLE_PV_ROW4BEAT, "pv_row4beat", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_PV_ROW4BEAT_SELECT,
		"pv_row4beat::select", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_set_style(self, STYLE_PV_CURSOR, "pv_cursor", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, STYLE_PV_CURSOR_SELECT, "pv_cursor::select",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00ffffff),
		psy_ui_colour_make(0x009F7B00));
	psy_ui_styles_set_style(self, STYLE_PV_PLAYBAR, "pv_playbar", style);	
}

void init_envelope_styles(psy_ui_Styles* self)
{
	psy_ui_Style* style;

	style = psy_ui_style_allocinit();	
	psy_ui_style_set_size(style, psy_ui_size_make_em(20.0, 15.0));
	psy_ui_styles_set_style(self, STYLE_ENVELOPE, "envelope", style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x00B1C8B0));
	psy_ui_styles_set_style(self, STYLE_ENVELOPE_POINT, "envelope_point",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x00B1C8B0));
	psy_ui_styles_set_style(self, STYLE_ENVELOPE_CURVE, "envelope_curve",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x00333333));
	psy_ui_styles_set_style(self, STYLE_ENVELOPE_GRID, "envelope_grid",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x00516850));
	psy_ui_styles_set_style(self, STYLE_ENVELOPE_SUSTAIN, "envelope_sustain",
		style);

	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x00434343));
	psy_ui_styles_set_style(self, STYLE_ENVELOPE_RULER, "envelope_ruler",
		style);
}
