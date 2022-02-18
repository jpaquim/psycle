/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineviewconfig.h"
/* host */
#include "dirconfig.h"
#include "resources/resource.h"
#include "styles.h"
#include "skinio.h"
/* ui */
#include <uiapp.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

#define PSYCLE__PATH__DEFAULT_MACHINE_SKIN "Psycle Default (internal)"

static void machineviewconfig_makeview(MachineViewConfig*,
	psy_Property* parent);
static void machineviewconfig_makestackview(MachineViewConfig*,
	psy_Property* parent);
static void machineviewconfig_maketheme(MachineViewConfig*,
	psy_Property* parent);
static void machineviewconfig_setcoords(MachineViewConfig*, psy_Property*);
static void machineviewconfig_setsource(MachineViewConfig*, psy_ui_RealRectangle*,
	intptr_t vals[4]);
static void machineviewconfig_setdest(MachineViewConfig*, psy_ui_RealPoint*,
	intptr_t vals[4], uintptr_t num);

void machineviewconfig_init(MachineViewConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	self->dirconfig = NULL;
	machineviewconfig_makeview(self, parent);
	psy_signal_init(&self->signal_changed);	
}

void machineviewconfig_dispose(MachineViewConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);	
}

void machineviewconfig_setdirectories(MachineViewConfig* self,
	DirConfig* dirconfig)
{
	self->dirconfig = dirconfig;	
	machineviewconfig_update_machine_skins(self);
}

void machineviewconfig_makeview(MachineViewConfig* self, psy_Property* parent)
{	
	assert(self);

	self->machineview = psy_property_settext(
		psy_property_append_section(parent, "machineview"),
		"settingsview.mv.machineview");
	psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawmachineindexes", TRUE),
		"settingsview.mv.draw-machine-indexes");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawvumeters", TRUE),
		"settingsview.mv.draw-vumeters"),
		PROPERTY_ID_DRAWVUMETERS);
	psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawwirehover", FALSE),
		"settingsview.mv.draw-wirehover");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawvirtualgenerators", FALSE),
		"settingsview.mv.draw-virtualgenerators"),
		PROPERTY_ID_DRAWVIRTUALGENERATORS);
	machineviewconfig_makestackview(self, self->machineview);
	machineviewconfig_maketheme(self, self->machineview);
}

void machineviewconfig_makestackview(MachineViewConfig* self, psy_Property* parent)
{
	self->stackview = psy_property_settext(
		psy_property_append_section(parent, "stackview"),
		"settingsview.mv.stackview");
	psy_property_settext(
		psy_property_append_bool(self->stackview,
			"drawsmalleffects", FALSE),
		"settingsview.mv.stackview-draw-smalleffects");
}

void machineviewconfig_maketheme(MachineViewConfig* self, psy_Property* parent)
{
	assert(self);
	
	self->theme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"settingsview.mv.theme.theme");	
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"vu2", 0x00403731, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.vu-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"vu1", 0x0080FF80, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.vu-bar");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"vu3", 0x00262bd7, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.onclip");
	psy_property_settext(
		psy_property_append_str(self->theme,
			"generator_fontface", "Tahoma"),
		"settingsview.mv.theme.generators-font-face");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"generator_font_point", 0x00000050, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.generators-font-point");
	psy_property_settext(psy_property_sethint(
		psy_property_append_int(self->theme,
			"generator_font_flags", 0x00000000, 0, 0),
		PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.generator_font_flags");
	psy_property_settext(psy_property_append_str(self->theme,
		"effect_fontface", "Tahoma"),
		"settingsview.mv.theme.effect-fontface");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"effect_font_point", 0x00000050, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.effect-font-point");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"effect_font_flags", 0x00000000, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.effect-font-flags");	
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_colour", 0x00232323,
			0, 0), PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wirecolour", 0x005F5F5F, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.wirecolour");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wirecolour2", 0x005F5F5F, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.wirecolour2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_polycolour", 0x00B1C8B0, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.polygons");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_generator_fontcolour", 0x00B1C8B0, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.generators-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_effect_fontcolour", 0x00D1C5B6, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.effects-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wirewidth", 0x00000001, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.wire-width");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wireaa", 0x01, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.mv.theme.antialias-halo");
	psy_property_settext(
		psy_property_append_str(self->theme,
			"machine_background", ""),
		"settingsview.mv.theme.machine-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_triangle_size", 0x05, 0, 0),
			PSY_PROPERTY_HINT_EDIT),
		"settingsview.mv.theme.polygon-size");	
	self->machineskins = psy_property_setid(
		psy_property_sethint(psy_property_settext(
		psy_property_append_choice(self->theme, "skins", 0),
		"Skin"), PSY_PROPERTY_HINT_COMBO),
		PROPERTY_ID_MACHINESKIN);
	machineviewconfig_update_machine_skins(self);
}

void machineviewconfig_update_machine_skins(MachineViewConfig* self)
{	
	psy_property_clear(self->machineskins);
	psy_property_append_str(self->machineskins,
		PSYCLE__PATH__DEFAULT_MACHINE_SKIN,
		PSYCLE__PATH__DEFAULT_MACHINE_SKIN);
	if (self->dirconfig) {
		skin_locate_machine_skins(self->machineskins,
			dirconfig_skins(self->dirconfig));
	}
}

void machineviewconfig_resettheme(MachineViewConfig* self)
{	
	assert(self);
	
	init_machineview_styles(&psy_ui_app()->defaults.styles);
	psy_property_setitem_int(self->machineskins, 0);
	machineviewconfig_read_styles(self);
}

void machineviewconfig_settheme(MachineViewConfig* self, psy_Property* theme)
{
	assert(self);

	if (self->theme) {
		psy_Property* machine_skin;		

		machine_skin = psy_property_at(self->machineskins,
			psy_property_at_str(theme, "machine_skin", ""),
			PSY_PROPERTY_TYPE_STRING);
		if (machine_skin) {
			psy_property_setitem_int(self->machineskins,
				psy_property_index(machine_skin));			
		} else {
			psy_property_setitem_int(self->machineskins, 0);
		}
		psy_property_sync(self->theme, theme);
		machineviewconfig_write_styles(self);
	}
}

void machineviewconfig_write_styles(MachineViewConfig* self)
{
	if (self->theme) {
		psy_ui_Styles* styles;
		psy_ui_Style* style;
		psy_ui_Colour bgcolour;		

		styles = &psy_ui_appdefaults()->styles;		
		bgcolour = psy_ui_colour_make(psy_property_at_colour(self->theme,
			"mv_colour", 0x00232323));		
		style = psy_ui_styles_at(styles, STYLE_MACHINEVIEW_WIRES);
		if (style) {
			const char* machine_background;

			machine_background = psy_property_at_str(self->theme,
				"machine_background", "");
			if (psy_strlen(machine_background) > 0) {
				const char* skindir;				

				skindir = dirconfig_skins(self->dirconfig);
				if (skindir) {
					char path[_MAX_PATH];
					char filename[_MAX_PATH];

					strcpy(filename, machine_background);
					psy_dir_findfile(skindir, filename, path);
					if (path[0] != '\0') {
						if (psy_ui_style_setbackgroundpath(style, path) != PSY_OK) {
							psy_ui_style_setbackgroundcolour(style, bgcolour);
						}
					}
				} else {
					psy_ui_style_setbackgroundcolour(style, bgcolour);
				}
			} else {
				psy_ui_style_setbackgroundcolour(style, bgcolour);
			}			
		}
		style = psy_ui_styles_at(styles, STYLE_MACHINEVIEW_STACK);
		if (style) {						
			psy_ui_style_setbackgroundcolour(style, bgcolour);
		}
		style = psy_ui_styles_at(styles, STYLE_MV_WIRE);
		psy_ui_style_setcolour(style, psy_ui_colour_make(
			psy_property_at_colour(self->theme, "mv_wirecolour", 0x005F5F5F)));
		style = psy_ui_styles_at(styles, STYLE_MV_WIRE_SELECT);
		psy_ui_style_setcolour(style, psy_ui_colour_make(
			psy_property_at_colour(self->theme, "mv_selwirecolour", 0x007F7F7F)));
		style = psy_ui_styles_at(styles, STYLE_MV_WIRE_HOVER);
		psy_ui_style_setcolour(style, psy_ui_colour_make(
			psy_property_at_colour(self->theme, "mv_hoverwirecolour", 0x007F7F7F)));
		style = psy_ui_styles_at(styles, STYLE_MV_WIRE_POLY);
		psy_ui_style_setcolour(style, psy_ui_colour_make(
			psy_property_at_colour(self->theme, "mv_polycolour", 0x005F5F5F)));
		psy_ui_style_set_background_size_px(style, (double)psy_property_at_int(
			self->theme, "mv_triangle_size", 10), 0);		
		style = psy_ui_styles_at(styles, STYLE_MV_GENERATOR);
		psy_ui_style_setcolour(style, psy_ui_colour_make(
			psy_property_at_colour(self->theme, "mv_generator_fontcolour", 0x00B1C8B0)));
		psy_ui_style_setfont(style,
			psy_property_at_str(self->theme, "generator_fontface", "Tahoma"),
			psy_property_at_int(self->theme, "generator_font_point", 16) / 7);
		style = psy_ui_styles_at(styles, STYLE_MV_EFFECT);
		psy_ui_style_setcolour(style, psy_ui_colour_make(
			psy_property_at_colour(self->theme, "mv_effect_fontcolour", 0x00D1C5B6)));	
		psy_ui_style_setfont(style,
			psy_property_at_str(self->theme, "effect_fontface", "Tahoma"),
			psy_property_at_int(self->theme, "effect_font_point", 16) / 7);		
		machineviewconfig_loadbitmap(self);
	}
}

void machineviewconfig_read_styles(MachineViewConfig* self)
{
	psy_ui_Style* style;

	assert(self);
	
	psy_property_set_int(self->theme, "vu2", 0x00403731);
	psy_property_set_int(self->theme, "vu1", 0x0080FF80);
	psy_property_set_int(self->theme, "vu3", 0x00262bd7);				
	psy_property_set_str(self->theme, "generator_fontface", "Tahoma");		
	psy_property_set_int(self->theme, "generator_font_point", 0x00000050);
	psy_property_set_int(self->theme, "generator_font_flags", 0x00000000);
	psy_property_set_str(self->theme, "effect_fontface", "Tahoma");
	psy_property_set_int(self->theme, "effect_font_point", 0x00000050);
	psy_property_set_int(self->theme, "effect_font_flags", 0x00000000);
	style = psy_ui_style(STYLE_MACHINEVIEW_WIRES);
	psy_property_set_int(self->theme, "mv_colour",
		psy_ui_colour_colorref(&style->background.colour));
	style = psy_ui_style(STYLE_MV_WIRE);
	psy_property_set_int(self->theme, "mv_wirecolour",
		psy_ui_colour_colorref(&style->colour));
	psy_property_set_int(self->theme, "mv_wirecolour2",
		psy_ui_colour_colorref(&style->colour));
	style = psy_ui_style(STYLE_MV_WIRE_POLY);
	psy_property_set_int(self->theme, "mv_polycolour",
		psy_ui_colour_colorref(&style->colour));
	psy_property_set_int(self->theme, "mv_triangle_size", 
		(int)style->background.size.width);
	style = psy_ui_style(STYLE_MV_GENERATOR);
	psy_property_set_int(self->theme, "mv_generator_fontcolour",
		psy_ui_colour_colorref(&style->colour));
	style = psy_ui_style(STYLE_MV_EFFECT);
	psy_property_set_int(self->theme, "mv_effect_fontcolour",
		psy_ui_colour_colorref(&style->colour));
	psy_property_set_int(self->theme, "mv_wirewidth", 0x00000001);
	psy_property_set_int(self->theme, "mv_wireaa", 0x01);
	psy_property_set_str(self->theme, "machine_background", "");
}

void machineviewconfig_loadbitmap(MachineViewConfig* self)
{
	const char* machine_skin_name;	
	static int styles[] = {
		STYLE_MV_MASTER,		
		STYLE_MV_GENERATOR,		
		STYLE_MV_GENERATOR_MUTE_SELECT,		
		STYLE_MV_GENERATOR_SOLO_SELECT,		
		STYLE_MV_GENERATOR_VU0,
		STYLE_MV_GENERATOR_VUPEAK,		
		STYLE_MV_GENERATOR_PAN_SLIDER,
		STYLE_MV_EFFECT,		
		STYLE_MV_EFFECT_MUTE_SELECT,		
		STYLE_MV_EFFECT_BYPASS_SELECT,		
		STYLE_MV_EFFECT_VU0,
		STYLE_MV_EFFECT_VUPEAK,		
		STYLE_MV_EFFECT_PAN_SLIDER,
		0 };
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MACHINEVIEW);
	psy_ui_style_setbackgroundid(style, psy_INDEX_INVALID);
	psy_ui_style_setbackgroundpath(style, "");
	machine_skin_name = machineviewconfig_machine_skin_name(self);
	if (self->dirconfig && psy_strlen(machine_skin_name) > 0) {
		const char* skindir;
		psy_ui_Style* style;

		skindir = dirconfig_skins(self->dirconfig);
		if (skindir) {
			psy_Path filename;
			char path[_MAX_PATH];			
			
			psy_path_init_all(&filename, "", machine_skin_name, "bmp");
			psy_dir_findfile(skindir, psy_path_full(&filename), path);
			if (path[0] != '\0') {				
				int i;
				
				for (i = 0; styles[i] != 0; ++i) {
					style = psy_ui_style(styles[i]);
					if (psy_ui_style_setbackgroundpath(style, path) != PSY_OK) {
						psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
					}
				}				
			} else { /* reset default machine bitmaps and coords */
				int i;

				for (i = 0; styles[i] != 0; ++i) {
					style = psy_ui_style(styles[i]);					
					psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);					
				}
				/* reset machine coordinates */
				style = psy_ui_style(STYLE_MV_MASTER);
				psy_ui_style_set_background_size_px(style, 138.0, 35.0);
				psy_ui_style_set_background_position_px(style, 0.0, -52.0);								
				style = psy_ui_style(STYLE_MV_MASTER_NAME);
				psy_ui_style_set_background_size_px(style, 117.0, 15.0);
				psy_ui_style_setpadding_px(style, 3.0, 0, 0.0, 20.0);			
				style = psy_ui_style(STYLE_MV_GENERATOR);				
				psy_ui_style_set_background_size_px(style, 138.0, 52.0);
				psy_ui_style_set_background_position_px(style, 0.0, -87.0);				
				style = psy_ui_style(STYLE_MV_GENERATOR_NAME);
				psy_ui_style_set_position(style,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(20.0, 3.0),
						psy_ui_size_make_px(117.0, 15.0)));				
				style = psy_ui_style(STYLE_MV_GENERATOR_MUTE);				
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));
				style = psy_ui_style(STYLE_MV_GENERATOR_MUTE_SELECT);				
				psy_ui_style_set_background_position_px(style, -23.0, -139.0);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));				
				style = psy_ui_style(STYLE_MV_GENERATOR_SOLO);		
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));				
				style = psy_ui_style(STYLE_MV_GENERATOR_SOLO_SELECT);
				psy_ui_style_set_background_size_px(style, 17, 17);
				psy_ui_style_set_background_position_px(style, -6.0, -139.0);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));				
				style = psy_ui_style(STYLE_MV_GENERATOR_VU);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(4.0, 20.0), psy_ui_size_make_px(129.0, 7.0)));				
				style = psy_ui_style(STYLE_MV_GENERATOR_VU0);
				psy_ui_style_setbackgroundid(style, IDB_MACHINESKIN);
				psy_ui_style_set_background_size_px(style, 129.0, 7);
				psy_ui_style_set_background_position_px(style, 0.0, -156.0);								
				style = psy_ui_style(STYLE_MV_GENERATOR_VUPEAK);				
				psy_ui_style_set_background_size_px(style, 1, 7);
				psy_ui_style_set_background_position_px(style, -108.0, -156.0);								
				style = psy_ui_style(STYLE_MV_GENERATOR_PAN);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(6.0, 33.0), psy_ui_size_make_px(82.0 + 6.0, 13.0)));				
				style = psy_ui_style(STYLE_MV_GENERATOR_PAN_SLIDER);				
				psy_ui_style_set_background_size_px(style, 6.0, 13.0);
				psy_ui_style_set_background_position_px(style, -0.0, -139.0);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(6.0, 13.0)));					
				style = psy_ui_style(STYLE_MV_EFFECT);				
				psy_ui_style_set_background_size_px(style, 138.0, 52.0);				
				style = psy_ui_style(STYLE_MV_EFFECT_NAME);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(20.0, 3.0), psy_ui_size_make_px(117.0, 15.0)));								
				style = psy_ui_style(STYLE_MV_EFFECT_MUTE);				
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));								
				style = psy_ui_style(STYLE_MV_EFFECT_MUTE_SELECT);				
				psy_ui_style_set_background_position_px(style, -23.0, -139.0);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));								
				style = psy_ui_style(STYLE_MV_EFFECT_BYPASS);				
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));								
				style = psy_ui_style(STYLE_MV_EFFECT_BYPASS_SELECT);				
				psy_ui_style_set_background_size_px(style, 17, 17);
				psy_ui_style_set_background_position_px(style, -6.0, -139.0);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));								
				style = psy_ui_style(STYLE_MV_EFFECT_VU);				
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(4.0, 20.0), psy_ui_size_make_px(129.0, 7.0)));								
				style = psy_ui_style(STYLE_MV_EFFECT_VU0);				
				psy_ui_style_set_background_size_px(style, 129.0, 7.0);
				psy_ui_style_set_background_position_px(style, 0.0, -163.0);								
				style = psy_ui_style(STYLE_MV_EFFECT_VUPEAK);				
				psy_ui_style_set_background_size_px(style, 1.0, 7.0);
				psy_ui_style_set_background_position_px(style, -96.0, -144.0);				
				style = psy_ui_style(STYLE_MV_EFFECT_PAN);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(6.0, 33.0), psy_ui_size_make_px(82.0 + 3.0, 13.0)));				
				style = psy_ui_style(STYLE_MV_EFFECT_PAN_SLIDER);				
				psy_ui_style_set_background_size_px(style, 6.0, 13.0);
				psy_ui_style_set_background_position_px(style, -0.0, -139.0);
				psy_ui_style_set_position(style, psy_ui_rectangle_make(
					psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(6.0, 13.0)));				
			}
			psy_path_setext(&filename, "psm");
			psy_dir_findfile(skindir, psy_path_full(&filename), path);
			psy_path_dispose(&filename);
			if (psy_strlen(path) > 0) {
				psy_Property* coords;
				psy_Property* transparency;

				coords = psy_property_allocinit_key(NULL);
				if (skin_load_machine_skin(coords, path) == PSY_OK) {
					machineviewconfig_setcoords(self, coords);
					transparency = psy_property_at(coords,
						"transparency", PSY_PROPERTY_TYPE_NONE);
					if (transparency) {
						psy_ui_Colour cltransparency;

						cltransparency = psy_ui_colour_make(
							strtol(psy_property_item_str(transparency), 0, 16));
						int i;

						for (i = 0; styles[i] != 0; ++i) {
							style = psy_ui_style(styles[i]);
							psy_ui_bitmap_settransparency(&style->background.bitmap,
								cltransparency);
						}
					}
				}
				psy_property_deallocate(coords);
			}
		}
	}
}

void machineviewconfig_setcoords(MachineViewConfig* self, psy_Property* p)
{
	const char* s;
	intptr_t vals[4];
	psy_ui_Style* style;
	psy_ui_RealRectangle srcmac;
	psy_ui_RealRectangle src;
	psy_ui_RealPoint dst;

	src = srcmac = psy_ui_realrectangle_zero();
	dst = psy_ui_realpoint_zero();
	/* master */	
	if (s = psy_property_at_str(p, "master_source", 0)) {
		style = psy_ui_style(STYLE_MV_MASTER);
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
		psy_ui_style_set_background_size_px(style, src.right - src.left, src.bottom - src.top);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}
	/* generator */
	if (s = psy_property_at_str(p, "generator_source", 0)) {		
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &srcmac, vals);
		style = psy_ui_style(STYLE_MV_GENERATOR);
		psy_ui_style_set_background_size_px(style, srcmac.right - srcmac.left, srcmac.bottom - srcmac.top);
		psy_ui_style_set_background_position_px(style, -srcmac.left, -srcmac.top);
	}
	if (s = psy_property_at_str(p, "generator_mute_source", 0)) {		
		skin_psh_values(s, 4, vals);		
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "generator_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		machineviewconfig_setdest(self, &dst, vals, 2);
		style = psy_ui_style(STYLE_MV_GENERATOR_MUTE);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_GENERATOR_MUTE_SELECT);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
	}
	if (s = psy_property_at_str(p, "generator_solo_source", 0)) {
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "generator_solo_dest", 0)) {
		skin_psh_values(s, 2, vals);
		machineviewconfig_setdest(self, &dst, vals, 2);
		style = psy_ui_style(STYLE_MV_GENERATOR_SOLO);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_GENERATOR_SOLO_SELECT);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
	}
	if (s = psy_property_at_str(p, "generator_name_dest", 0)) {
		style = psy_ui_style(STYLE_MV_GENERATOR_NAME);
		skin_psh_values(s, 2, vals);
		machineviewconfig_setdest(self, &dst, vals, 2);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(srcmac.right - dst.x, 15.0)));
	}
	if (s = psy_property_at_str(p, "generator_pan_source", 0)) {
		skin_psh_values(s, 4, vals);
		style = psy_ui_style(STYLE_MV_GENERATOR_PAN);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "generator_pan_dest", 0)) {		
		skin_psh_values(s, 3, vals);		
		machineviewconfig_setdest(self, &dst, vals, 3);
		style = psy_ui_style(STYLE_MV_GENERATOR_PAN);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px((double)vals[2] + src.right - src.left,
				src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_GENERATOR_PAN_SLIDER);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_background_size_px(style, src.right - src.left, src.bottom - src.top);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
	}
	if (s = psy_property_at_str(p, "generator_vu0_source", 0)) {		
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "generator_vu_dest", 0)) {
		skin_psh_values(s, 3, vals);
		machineviewconfig_setdest(self, &dst, vals, 3);
		style = psy_ui_style(STYLE_MV_GENERATOR_VU);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px((double)vals[2], src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_GENERATOR_VU0);				
		psy_ui_style_set_background_size_px(style, (double)vals[2], src.bottom - src.top);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);		
	}
	if (s = psy_property_at_str(p, "generator_vu_peak_source", 0)) {
		style = psy_ui_style(STYLE_MV_GENERATOR_VUPEAK);
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
		psy_ui_style_set_background_size_px(style, src.right - src.left, src.bottom - src.top);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}		
	/* effect */	
	if (s = psy_property_at_str(p, "effect_source", 0)) {
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &srcmac, vals);
		style = psy_ui_style(STYLE_MV_EFFECT);
		psy_ui_style_set_background_size_px(style, srcmac.right - srcmac.left,
			srcmac.bottom - srcmac.top);
		psy_ui_style_set_background_position_px(style, -srcmac.left, -srcmac.top);
	}
	if (s = psy_property_at_str(p, "effect_mute_source", 0)) {
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "effect_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		machineviewconfig_setdest(self, &dst, vals, 2);
		style = psy_ui_style(STYLE_MV_EFFECT_MUTE);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_EFFECT_MUTE_SELECT);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
	}
	if (s = psy_property_at_str(p, "effect_bypass_source", 0)) {
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "effect_bypass_dest", 0)) {
		skin_psh_values(s, 2, vals);
		machineviewconfig_setdest(self, &dst, vals, 2);
		style = psy_ui_style(STYLE_MV_EFFECT_BYPASS);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_EFFECT_BYPASS_SELECT);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
	}
	if (s = psy_property_at_str(p, "effect_name_dest", 0)) {		
		skin_psh_values(s, 2, vals);
		machineviewconfig_setdest(self, &dst, vals, 2);
		style = psy_ui_style(STYLE_MV_EFFECT_NAME);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px(srcmac.right - dst.x, 15.0)));
	}
	if (s = psy_property_at_str(p, "effect_pan_source", 0)) {
		skin_psh_values(s, 4, vals);
		style = psy_ui_style(STYLE_MV_EFFECT_PAN);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "effect_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		machineviewconfig_setdest(self, &dst, vals, 3);
		style = psy_ui_style(STYLE_MV_EFFECT_PAN);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px((double)vals[2] + src.right - src.left,
				src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_EFFECT_PAN_SLIDER);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
		psy_ui_style_set_background_size_px(style, src.right - src.left, src.bottom - src.top);
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(src.right - src.left,
				src.bottom - src.top)));
	}
	if (s = psy_property_at_str(p, "effect_vu0_source", 0)) {
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
	}
	if (s = psy_property_at_str(p, "effect_vu_dest", 0)) {
		skin_psh_values(s, 3, vals);
		machineviewconfig_setdest(self, &dst, vals, 3);
		style = psy_ui_style(STYLE_MV_EFFECT_VU);		
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(dst.x, dst.y),
			psy_ui_size_make_px((double)vals[2], src.bottom - src.top)));
		style = psy_ui_style(STYLE_MV_EFFECT_VU0);
		psy_ui_style_set_background_size_px(style, (double)vals[2], src.bottom - src.top);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}
	if (s = psy_property_at_str(p, "effect_vu_peak_source", 0)) {		
		skin_psh_values(s, 4, vals);
		style = psy_ui_style(STYLE_MV_EFFECT_VUPEAK);
		machineviewconfig_setsource(self, &src, vals);
		psy_ui_style_set_background_size_px(style, src.right - src.left, src.bottom - src.top);
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}			
}

void machineviewconfig_setsource(MachineViewConfig* self, psy_ui_RealRectangle* r,
	intptr_t vals[4])
{	
	r->left = (double)vals[0];
	r->top = (double)vals[1];
	r->right = (double)vals[0] + (double)vals[2];
	r->bottom = (double)vals[1] + (double)vals[3];	
}

void machineviewconfig_setdest(MachineViewConfig* self, psy_ui_RealPoint* pt,
	intptr_t vals[4], uintptr_t num)
{
	pt->x = (double)vals[0];
	pt->y = (double)vals[1];	
}


bool machineviewconfig_hasthemeproperty(const MachineViewConfig* self,
	psy_Property* property)
{
	return (self->theme && psy_property_insection(property, self->theme));
}

bool machineviewconfig_hasproperty(const MachineViewConfig* self,
	psy_Property* property)
{
	assert(self && self->machineview);

	return  psy_property_insection(property, self->machineview);
}

bool machineviewconfig_machineindexes(const MachineViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->machineview, "drawmachineindexes",
		TRUE);
}

bool machineviewconfig_wirehover(const MachineViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->machineview, "drawwirehover", TRUE);
}

bool machineviewconfig_vumeters(const MachineViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->machineview, "drawvumeters", TRUE);
}

const char* machineviewconfig_machine_skin_name(MachineViewConfig* self)
{
	psy_Property* choice;

	assert(self);

	choice = psy_property_at_choice(self->machineskins);
	if (choice && psy_strlen(psy_property_item_str(choice)) > 0) {
		return psy_property_key(choice);		
	}
	return "";
}

bool machineviewconfig_virtualgenerators(const MachineViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->machineview, "drawvirtualgenerators", FALSE);
}

bool machineviewconfig_stackview_drawsmalleffects(const MachineViewConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->stackview, "drawsmalleffects", FALSE);
}

/* events */
int machineviewconfig_onchanged(MachineViewConfig* self, psy_Property*
	property)
{
	int rebuild_level;

	assert(self);

	rebuild_level = 0;
	if (machineviewconfig_hasthemeproperty(self, property)) {
		psy_Property* choice;
		bool worked;

		choice = (psy_property_ischoiceitem(property)) ? psy_property_parent(property) : NULL;
		worked = FALSE;
		if (choice) {
			worked = TRUE;
			switch (psy_property_id(choice)) {
			case PROPERTY_ID_MACHINESKIN:
				machineviewconfig_loadbitmap(self);
				break;
			default:
				worked = FALSE;
				break;
			}
		}
		if (!worked) {
			machineviewconfig_write_styles(self);
		}
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
	return rebuild_level;
}
