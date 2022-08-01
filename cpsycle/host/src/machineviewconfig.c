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
static void machineviewconfig_makevu(MachineViewConfig*,
	psy_Property* parent);	
static void machineviewconfig_setcoords(MachineViewConfig*, psy_Property*);
static void machineviewconfig_setsource(MachineViewConfig*, psy_ui_RealRectangle*,
	intptr_t vals[4]);
static void machineviewconfig_setdest(MachineViewConfig*, psy_ui_RealPoint*,
	intptr_t vals[4], uintptr_t num);
static void machineviewconfig_load_background(MachineViewConfig*);
static void machineviewconfig_load_colours(MachineViewConfig*);
static void machineviewconfig_load_bitmap(MachineViewConfig*);
static void read_colour(psy_Property* config, const char* key, psy_ui_Style*,
	bool fore);
static void machineviewconfig_set_style_default_settings(MachineViewConfig*);
static void machineviewconfig_set_style_default_colours(MachineViewConfig*);
static void machineviewconfig_set_style_default_skin(MachineViewConfig*);
static void machineviewconfig_set_colour(MachineViewConfig*,
	const char* key, uintptr_t style_id, psy_ui_Colour);
static void machineviewconfig_set_style_background_colour(MachineViewConfig*,
	const char* key, uintptr_t style_id, psy_ui_Colour);
static void machineviewconfig_on_load_bitmap(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_vu_colour(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_vu_background_colour(MachineViewConfig*,
	psy_Property* sender);	
static void machineviewconfig_on_vu_clip_colour(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_generator_font(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_generator_font_colour(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_effect_font(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_effect_font_colour(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_background_colour(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_triangle_size(MachineViewConfig*,
	psy_Property* sender);
static void machineviewconfig_on_poly_colour(MachineViewConfig*,
	psy_Property* sender);	

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

	self->machineview = psy_property_set_text(
		psy_property_append_section(parent, "machineview"),
		"settings.mv.machineview");
	psy_property_set_text(
		psy_property_append_bool(self->machineview,
			"drawmachineindexes", TRUE),
		"settings.mv.draw-machine-indexes");
	psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(self->machineview,
			"drawvumeters", TRUE),
		"settings.mv.draw-vumeters"),
		PROPERTY_ID_DRAWVUMETERS);
	psy_property_set_text(
		psy_property_append_bool(self->machineview,
			"drawwirehover", FALSE),
		"settings.mv.draw-wirehover");
	psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(self->machineview,
			"drawvirtualgenerators", FALSE),
		"settings.mv.draw-virtualgenerators"),
		PROPERTY_ID_DRAWVIRTUALGENERATORS);	
	machineviewconfig_makestackview(self, self->machineview);
	machineviewconfig_maketheme(self, self->machineview);
	machineviewconfig_makevu(self, parent);
	machineviewconfig_resettheme(self);
}

void machineviewconfig_makestackview(MachineViewConfig* self, psy_Property* parent)
{
	self->stackview = psy_property_set_text(
		psy_property_append_section(parent, "stackview"),
		"settings.mv.stackview");
	psy_property_set_text(
		psy_property_append_bool(self->stackview,
			"drawsmalleffects", FALSE),
		"settings.mv.stackview-draw-smalleffects");
}

void machineviewconfig_maketheme(MachineViewConfig* self, psy_Property* parent)
{	
	intptr_t style_value = 0;
	const char* style_str = "";	

	assert(self);
	
	/* define theme properties */	
	self->theme = psy_property_set_text(
		psy_property_append_section(parent, "theme"),
		"settings.mv.theme.theme");		
	/* generator */
	psy_property_connect(psy_property_set_text(psy_property_append_font(
		self->theme, "generator_font", PSYCLE_DEFAULT_MACHINE_FONT),
		"settings.mv.theme.generators-font-face"),
		self, machineviewconfig_on_generator_font);
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_generator_fontcolour", style_value, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.generators-font"),
		self, machineviewconfig_on_generator_font_colour);
	/* effect */
	psy_property_connect(psy_property_set_text(psy_property_append_font(
		self->theme, "effect_font", PSYCLE_DEFAULT_MACHINE_FONT),
		"settings.mv.theme.effect-fontface"),
		self, machineviewconfig_on_effect_font);
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_effect_fontcolour", style_value, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.effects-font"),
		self, machineviewconfig_on_effect_font_colour);
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_colour", style_value, 0, 0), PSY_PROPERTY_HINT_EDITCOLOR),			
		"settings.mv.theme.background"),
		self, machineviewconfig_on_background_colour);
	/* wire */
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_wirecolour", style_value, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.wirecolour");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_wirecolour2", style_value, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.wirecolour2");
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_polycolour", style_value, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.polygons"),
		self, machineviewconfig_on_poly_colour);
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_wirewidth", 0x00000001, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.wire-width");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_wireaa", 0x01, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.antialias-halo");
	psy_property_set_text(
		psy_property_append_str(self->theme,
			"machine_background", style_str),
		"settings.mv.theme.machine-background");
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"mv_triangle_size", style_value, 0, 0),
			PSY_PROPERTY_HINT_EDIT),
		"settings.mv.theme.polygon-size"),
		self, machineviewconfig_on_triangle_size);
	self->machineskins =
		psy_property_connect(psy_property_set_id(psy_property_set_hint(
			psy_property_set_text(psy_property_append_choice(
				self->theme, "skins", 0),
				"Skin"), PSY_PROPERTY_HINT_COMBO),
			PROPERTY_ID_MACHINESKIN),
			self, machineviewconfig_on_load_bitmap);
	machineviewconfig_update_machine_skins(self);	
}

void machineviewconfig_makevu(MachineViewConfig* self, psy_Property* parent)
{
	psy_Property* vu;
	
	vu = psy_property_set_text(
		psy_property_append_section(parent, "vucolors"),
		"VU Colors");	
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(vu,
			"vu2", 0x00403731, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.vu-background"),
		self, machineviewconfig_on_vu_background_colour);
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(vu,
			"vu1", 0x0080FF80, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.vu-bar"),
		self, machineviewconfig_on_vu_colour);
	psy_property_connect(psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(vu,
			"vu3", 0x00262bd7, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settings.mv.theme.onclip"),
		self, machineviewconfig_on_vu_clip_colour);		
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
	
	machineviewconfig_set_style_default_settings(self);
	psy_property_set_item_int(self->machineskins, 0);
	machineviewconfig_save(self);
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
			psy_property_set_item_int(self->machineskins,
				psy_property_index(machine_skin));
		} else {
			psy_property_set_item_int(self->machineskins, 0);
		}
		psy_property_sync(self->theme, theme);
		machineviewconfig_load(self);
	}
}

void machineviewconfig_load(MachineViewConfig* self)
{
	assert(self);
	
	machineviewconfig_load_background(self);	
	machineviewconfig_load_colours(self);
	machineviewconfig_load_bitmap(self);	
}

void machineviewconfig_load_background(MachineViewConfig* self)
{
	char path[_MAX_PATH];
	
	psy_dir_findfile(dirconfig_skins(self->dirconfig), psy_property_at_str(
		self->theme, "machine_background", ""), path);
	psy_ui_style_setbackgroundpath(psy_ui_style(STYLE_MV_WIRES),
		path);
}

void machineviewconfig_load_colours(MachineViewConfig* self)
{
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MV);
	read_colour(self->theme, "mv_wirecolour", style, TRUE);
	style = psy_ui_style(STYLE_MV_WIRES);
	read_colour(self->theme, "mv_wirecolour", style, TRUE);
	read_colour(self->theme, "mv_colour", style, FALSE);
	style = psy_ui_style(STYLE_MV_STACK);
	read_colour(self->theme, "mv_wirecolour", style, TRUE);
	read_colour(self->theme, "mv_colour", style, FALSE);
	style = psy_ui_style(STYLE_MV_PROPERTIES);
	read_colour(self->theme, "mv_wirecolour", style, TRUE);
	read_colour(self->theme, "mv_colour", style, FALSE);
	style = psy_ui_style(STYLE_MV_NEWMACHINE);
	read_colour(self->theme, "mv_wirecolour", style, TRUE);
	read_colour(self->theme, "mv_colour", style, FALSE);
	style = psy_ui_style(STYLE_MV_WIRE);
	read_colour(self->theme, "mv_wirecolour", style, TRUE);
	style = psy_ui_style(STYLE_MV_WIRE_SELECT);
	read_colour(self->theme, "mv_selwirecolour", style, TRUE);
	style = psy_ui_style(STYLE_MV_WIRE_HOVER);
	read_colour(self->theme, "mv_hoverwirecolour", style, TRUE);
	style = psy_ui_style(STYLE_MV_WIRE_POLY);
	read_colour(self->theme, "mv_polycolour", style, TRUE);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(
		(double)psy_property_at_int(self->theme, "mv_triangle_size",
			10), 0));
	style = psy_ui_style(STYLE_MV_MASTER);
	style = psy_ui_style(STYLE_MV_GENERATOR);
	read_colour(self->theme, "mv_generator_fontcolour", style, TRUE);	
	psy_ui_style_set_font_string(style,	 psy_property_at_str(self->theme,
		"generator_font", PSYCLE_DEFAULT_MACHINE_FONT));		
	style = psy_ui_style(STYLE_MV_EFFECT);
	read_colour(self->theme, "mv_effect_fontcolour", style, TRUE);
	psy_ui_style_set_font_string(style,	 psy_property_at_str(self->theme,
		"effect_font", PSYCLE_DEFAULT_MACHINE_FONT));
	/* vu */
	style = psy_ui_style(STYLE_MAIN_VU);
	read_colour(self->machineview->parent, "vucolors.vu1", style, TRUE);
	read_colour(self->machineview->parent, "vucolors.vu2", style, FALSE);
	style = psy_ui_style(STYLE_CLIPBOX_SELECT);
	read_colour(self->machineview->parent, "vucolors.vu3", style, FALSE);
}

void read_colour(psy_Property* config, const char* key, psy_ui_Style* style, bool fore)
{
	psy_Property* p;

	if (p = psy_property_at(config, key, PSY_PROPERTY_TYPE_INTEGER)) {
		if (fore) {
			psy_ui_style_set_colour(style, psy_ui_colour_make(
				(uint32_t)psy_property_item_int(p)));
		} else {
			psy_ui_style_set_background_colour(style, psy_ui_colour_make(
				(uint32_t)psy_property_item_int(p)));
		}
	}
}

void machineviewconfig_on_vu_colour(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;
			
	style = psy_ui_style(STYLE_MAIN_VU);
	psy_ui_style_set_colour(style, psy_ui_colour_make(
		psy_property_item_int(sender)));
}

void machineviewconfig_on_vu_background_colour(MachineViewConfig* self,
	psy_Property* sender)	
{
	psy_ui_Style* style;
			
	style = psy_ui_style(STYLE_MAIN_VU);
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(
		psy_property_item_int(sender)));
}

void machineviewconfig_on_vu_clip_colour(MachineViewConfig* self,
	psy_Property* sender)	
{
	psy_ui_Style* style;
			
	style = psy_ui_style(STYLE_CLIPBOX_SELECT);
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(
		psy_property_item_int(sender)));
}

void machineviewconfig_on_generator_font(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;
		
	style = psy_ui_style(STYLE_MV_GENERATOR);	
	psy_ui_style_set_font_string(style, psy_property_item_str(sender));
}

void machineviewconfig_on_generator_font_colour(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;
		
	style = psy_ui_style(STYLE_MV_GENERATOR);	
	psy_ui_style_set_colour(style, psy_ui_colour_make(
		psy_property_item_int(sender)));
}

void machineviewconfig_on_effect_font(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;
		
	style = psy_ui_style(STYLE_MV_EFFECT);	
	psy_ui_style_set_font_string(style, psy_property_item_str(sender));
}

void machineviewconfig_on_effect_font_colour(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;
		
	style = psy_ui_style(STYLE_MV_EFFECT);
	psy_ui_style_set_colour(style, psy_ui_colour_make(
		psy_property_item_int(sender)));
}

void machineviewconfig_on_background_colour(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;
			
	style = psy_ui_style(STYLE_MV_WIRES);
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(
		psy_property_item_int(sender)));
}

void machineviewconfig_on_poly_colour(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;	
			
	style = psy_ui_style(STYLE_MV_WIRE_POLY);
	psy_ui_style_set_colour(style, psy_ui_colour_make(
		(uint32_t)psy_property_item_int(sender)));	
}

void machineviewconfig_on_triangle_size(MachineViewConfig* self,
	psy_Property* sender)
{
	psy_ui_Style* style;	
			
	style = psy_ui_style(STYLE_MV_WIRE_POLY);
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(
		psy_property_item_int(sender), psy_property_item_int(sender)));
}

void machineviewconfig_on_load_bitmap(MachineViewConfig* self, psy_Property* sender)
{

}

void machineviewconfig_load_bitmap(MachineViewConfig* self)
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

	style = psy_ui_style(STYLE_MV);
	psy_ui_style_set_background_id(style, psy_INDEX_INVALID);
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
						psy_ui_style_set_background_id(style, IDB_MACHINESKIN);
					}
					psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
				}				
			} else {
				machineviewconfig_set_style_default_skin(self);
			}
			psy_path_setext(&filename, "psm");
			psy_dir_findfile(skindir, psy_path_full(&filename), path);
			psy_path_dispose(&filename);
			if (psy_strlen(path) > 0) {
				psy_Property coords;
				psy_Property* transparency;

				psy_property_init(&coords);
				if (skin_load_machine(&coords, path) == PSY_OK) {
					machineviewconfig_setcoords(self, &coords);
					
					transparency = psy_property_at(&coords,
						"transparency", PSY_PROPERTY_TYPE_NONE);
					if (transparency && transparency->item.marked) {
						psy_ui_Colour cltransparency;
						int i;

						cltransparency = psy_ui_colour_make(
							strtol(psy_property_item_str(transparency), 0, 16));
						for (i = 0; styles[i] != 0; ++i) {
							style = psy_ui_style(styles[i]);
							psy_ui_bitmap_settransparency(&style->background.bitmap,
								cltransparency);
						}
					}
				}
				psy_property_dispose(&coords);
			}
		}
	}
}

void machineviewconfig_save(MachineViewConfig* self)
{
	psy_ui_Style* style;
	psy_ui_FontInfo fontinfo;
	char font_str[256];

	assert(self);	
	
	style = psy_ui_style(STYLE_MV_WIRES);
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
	psy_property_set_int(self->theme, "mv_triangle_size",
		(int)style->background.size.height);	
	psy_property_set_int(self->theme, "mv_wirewidth", 0x00000001);
	psy_property_set_int(self->theme, "mv_wireaa", 0x01);	
	/* generator */
	style = psy_ui_style(STYLE_MV_GENERATOR);	
	psy_property_set_int(self->theme, "mv_generator_fontcolour",
		psy_ui_colour_colorref(&style->colour));
	fontinfo = psy_ui_font_fontinfo(&style->font);	
	psy_ui_fontinfo_string(&fontinfo, font_str, 256);
	psy_property_set_font(self->theme, "generator_font", font_str);
	psy_property_set_str(self->theme, "machine_background",
		style->background.image_path);
	/* effect */		
	style = psy_ui_style(STYLE_MV_EFFECT);
	psy_property_set_int(self->theme, "mv_effect_fontcolour",
		psy_ui_colour_colorref(&style->colour));
	fontinfo = psy_ui_font_fontinfo(&style->font);
	psy_ui_fontinfo_string(&fontinfo, font_str, 256);
	psy_property_set_font(self->theme, "effect_font", font_str);
	psy_property_set_str(self->theme, "machine_background",
		style->background.image_path);
	/* vu */
	style = psy_ui_style(STYLE_MAIN_VU);
	psy_property_set_int(self->machineview->parent, "vucolors.vu1",
		psy_ui_colour_colorref(&style->colour));
	psy_property_set_int(self->machineview->parent, "vucolors.vu2",
		psy_ui_colour_colorref(&style->background.colour));
	style = psy_ui_style(STYLE_CLIPBOX_SELECT);
	psy_property_set_int(self->machineview->parent, "vucolors.vu3",
		psy_ui_colour_colorref(&style->background.colour));
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
		psy_ui_style_set_size(style, psy_ui_size_make_px(src.right - src.left,
			src.bottom - src.top));
		psy_ui_style_set_background_size_px(style, 
			psy_ui_realrectangle_size(&src));
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}
	/* generator */
	if (s = psy_property_at_str(p, "generator_source", 0)) {		
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &srcmac, vals);
		style = psy_ui_style(STYLE_MV_GENERATOR);
		psy_ui_style_set_size(style, psy_ui_size_make_px(srcmac.right - srcmac.left,
			srcmac.bottom - srcmac.top));
		psy_ui_style_set_background_size_px(style,
			psy_ui_realrectangle_size(&srcmac));
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
		psy_ui_style_set_background_size_px(style,
			psy_ui_realrectangle_size(&src));			
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
		psy_ui_style_set_background_size_px(style, 
			psy_ui_realsize_make((double)vals[2], src.bottom - src.top));
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);		
	}
	if (s = psy_property_at_str(p, "generator_vu_peak_source", 0)) {
		style = psy_ui_style(STYLE_MV_GENERATOR_VUPEAK);
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &src, vals);
		psy_ui_style_set_background_size_px(style,
			psy_ui_realrectangle_size(&src));			
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}		
	/* effect */	
	if (s = psy_property_at_str(p, "effect_source", 0)) {
		skin_psh_values(s, 4, vals);
		machineviewconfig_setsource(self, &srcmac, vals);
		style = psy_ui_style(STYLE_MV_EFFECT);
		psy_ui_style_set_size(style, psy_ui_size_make_px(srcmac.right - srcmac.left,
			srcmac.bottom - srcmac.top));
		psy_ui_style_set_background_size_px(style,
			psy_ui_realrectangle_size(&srcmac));			
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
		psy_ui_style_set_background_size_px(style,
			psy_ui_realrectangle_size(&src));			
		psy_ui_style_set_position(style, psy_ui_rectangle_make(
			psy_ui_point_make_px(0.0, 0.0),
			psy_ui_size_make_px(src.right - src.left, src.bottom - src.top)));
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
		psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(
			(double)vals[2], src.bottom - src.top));
		psy_ui_style_set_background_position_px(style, -src.left, -src.top);
	}
	if (s = psy_property_at_str(p, "effect_vu_peak_source", 0)) {		
		skin_psh_values(s, 4, vals);
		style = psy_ui_style(STYLE_MV_EFFECT_VUPEAK);
		machineviewconfig_setsource(self, &src, vals);
		psy_ui_style_set_background_size_px(style,
			psy_ui_realrectangle_size(&src));			
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
	return (self->theme && psy_property_in_section(property, self->theme));
}

bool machineviewconfig_hasproperty(const MachineViewConfig* self,
	psy_Property* property)
{
	assert(self && self->machineview);

	return  psy_property_in_section(property, self->machineview);
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

void machineviewconfig_set_style_default_settings(MachineViewConfig* self)
{
	machineviewconfig_set_style_default_colours(self);
	machineviewconfig_set_style_default_skin(self);
}

void machineviewconfig_set_style_default_colours(MachineViewConfig* self)
{
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MV);
	psy_ui_style_set_colour(style,
		psy_ui_colour_make(0x005F5F5F));
	style = psy_ui_style(STYLE_MV_WIRES);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x005F5F5F),
		psy_ui_colour_make(0x00232323));
	style = psy_ui_style(STYLE_MV_STACK);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x005F5F5F),
		psy_ui_colour_make(0x00232323));
	style = psy_ui_style(STYLE_MV_PROPERTIES);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x005F5F5F),
		psy_ui_colour_make(0x00232323));
	style = psy_ui_style(STYLE_MV_NEWMACHINE);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x005F5F5F),
		psy_ui_colour_make(0x00232323));
	style = psy_ui_style(STYLE_MV_PROPERTIES);
	psy_ui_style_set_padding_em(style, 0.25, 0.5, 0.5, 0.5);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID,
		psy_ui_colour_make(0x00333333));
	style = psy_ui_style(STYLE_MV_WIRE);
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x005F5F5F));
	style = psy_ui_style(STYLE_MV_WIRE_SELECT);
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x007F7F7F));
	style = psy_ui_style(STYLE_MV_WIRE_HOVER);
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x007F7F7F));
	style = psy_ui_style(STYLE_MV_WIRE_POLY);
	psy_ui_style_set_colour(style, psy_ui_colour_make(0x005F5F5F));
	psy_ui_style_set_background_size_px(style, psy_ui_realsize_make(5.0, 5.0));

	style = psy_ui_style(STYLE_MV_MASTER);
	psy_ui_style_set_background_colour(style, psy_ui_colour_make(0x00333333));

	style = psy_ui_style(STYLE_MV_GENERATOR);
	psy_ui_style_set_font(style, "Tahoma", 12);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00B1C8B0),
		psy_ui_colour_make(0x002f3E25));

	style = psy_ui_style(STYLE_MV_EFFECT);
	psy_ui_style_set_font(style, "Tahoma", 12);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x00D1C5B6),
		psy_ui_colour_make(0x003E2f25));		
	/* vu */
	style = psy_ui_style(STYLE_MAIN_VU);
	psy_ui_style_set_colours(style,
		psy_ui_colour_make(0x0080FF80),
		psy_ui_colour_make(0x00403731));	
	style = psy_ui_style(STYLE_CLIPBOX_SELECT);
	psy_ui_style_set_background_colour(style,	
		psy_ui_colour_make(0x00262bd7));
}

void machineviewconfig_set_style_default_skin(MachineViewConfig* self)
{
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MV_WIRES);
	psy_ui_style_set_background_id(style, psy_INDEX_INVALID);

	style = psy_ui_style(STYLE_MV_MASTER);
	psy_ui_style_set_size(style, psy_ui_size_make_px(138.0, 35.0));	
	psy_ui_style_set_font_string(style, PSYCLE_DEFAULT_MACHINE_FONT);
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_position_px(style, 0.0, -52.0);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(138.0, 35.0));

	style = psy_ui_style(STYLE_MV_MASTER_NAME);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(117.0, 15.0));
	psy_ui_style_set_padding_px(style, 3.0, 0, 0.0, 20.0);

	style = psy_ui_style(STYLE_MV_GENERATOR);
	psy_ui_style_set_font_string(style, PSYCLE_DEFAULT_MACHINE_FONT);
	psy_ui_style_set_size(style, psy_ui_size_make_px(138.0, 52.0));	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_position_px(style, 0.0, -87.0);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(138.0, 52.0));

	style = psy_ui_style(STYLE_MV_GENERATOR_NAME);
	psy_ui_style_set_position(style,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(20.0, 3.0),
			psy_ui_size_make_px(117.0, 15.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_MUTE);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_MUTE_SELECT);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_position_px(style, -23.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_SOLO);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_SOLO_SELECT);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(17, 17));
	psy_ui_style_set_background_position_px(style, -6.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_VU);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(4.0, 20.0), psy_ui_size_make_px(129.0, 7.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_VU0);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(129.0, 7));
	psy_ui_style_set_background_position_px(style, 0.0, -156.0);

	style = psy_ui_style(STYLE_MV_GENERATOR_VUPEAK);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(1, 7));
	psy_ui_style_set_background_position_px(style, -108.0, -156.0);

	style = psy_ui_style(STYLE_MV_GENERATOR_PAN);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(6.0, 33.0), psy_ui_size_make_px(82.0 + 6.0, 13.0)));

	style = psy_ui_style(STYLE_MV_GENERATOR_PAN_SLIDER);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style, 
		psy_ui_realsize_make(6.0, 13.0));
	psy_ui_style_set_background_position_px(style, -0.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(6.0, 13.0)));

	style = psy_ui_style(STYLE_MV_EFFECT);
	psy_ui_style_set_font_string(style, PSYCLE_DEFAULT_MACHINE_FONT);
	psy_ui_style_set_size(style, psy_ui_size_make_px(138.0, 52.0));		
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(138.0, 52.0));

	style = psy_ui_style(STYLE_MV_EFFECT_NAME);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(20.0, 3.0), psy_ui_size_make_px(117.0, 15.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_MUTE);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_MUTE_SELECT);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_position_px(style, -23.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(117.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_BYPASS);
	psy_ui_style_set_background_colour(style,
		psy_ui_colour_transparent());
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_BYPASS_SELECT);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(17, 17));
	psy_ui_style_set_background_position_px(style, -6.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(98.0, 31.0), psy_ui_size_make_px(17.0, 17.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_VU);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(4.0, 20.0), psy_ui_size_make_px(129.0, 7.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_VU0);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(129.0, 7.0));
	psy_ui_style_set_background_position_px(style, 0.0, -163.0);

	style = psy_ui_style(STYLE_MV_EFFECT_VUPEAK);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(1.0, 7.0));
	psy_ui_style_set_background_position_px(style, -96.0, -144.0);

	style = psy_ui_style(STYLE_MV_EFFECT_PAN);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(6.0, 33.0), psy_ui_size_make_px(82.0 + 3.0, 13.0)));

	style = psy_ui_style(STYLE_MV_EFFECT_PAN_SLIDER);	
	psy_ui_style_set_background_id(style, IDB_MACHINESKIN);	
	psy_ui_style_set_background_repeat(style, psy_ui_NOREPEAT);
	psy_ui_style_set_background_size_px(style,
		psy_ui_realsize_make(6.0, 13.0));
	psy_ui_style_set_background_position_px(style, -0.0, -139.0);
	psy_ui_style_set_position(style, psy_ui_rectangle_make(
		psy_ui_point_make_px(0.0, 0.0), psy_ui_size_make_px(6.0, 13.0)));

	style = psy_ui_style(STYLE_MV_ARROW);
	psy_ui_style_set_size(style, psy_ui_size_make_px(138.0, 52.0));
	style = psy_ui_style(STYLE_MV_LEVEL);
	psy_ui_style_set_size(style, psy_ui_size_make_px(16.0, 90.0));
}

void machineviewconfig_set_background_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_style_background_colour(self, "mv_colour",
		STYLE_MV, colour);
}

void machineviewconfig_set_poly_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_colour(self, "mv_polycolour",
		STYLE_MV_WIRE_POLY, colour);
}

void machineviewconfig_set_wire_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_colour(self, "mv_wirecolour",
		STYLE_MV_WIRE, colour);
}

void machineviewconfig_set_generator_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_colour(self, "mv_generator_fontcolour",
		STYLE_MV_GENERATOR, colour);
}

void machineviewconfig_set_effect_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_colour(self, "mv_generator_fontcolour",
		STYLE_MV_EFFECT, colour);
}

void machineviewconfig_set_param_top_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_colour(self, "machineGUIFontTopColor",
		STYLE_MACPARAM_TOP, colour);
}

void machineviewconfig_set_param_bottom_colour(MachineViewConfig* self,
	psy_ui_Colour colour)
{
	machineviewconfig_set_colour(self, "machineGUIFontBottomColor",
		STYLE_MACPARAM_BOTTOM, colour);
}

void machineviewconfig_set_colour(MachineViewConfig* self,
	const char* key, uintptr_t style_id, psy_ui_Colour colour)
{
	psy_ui_Style* style;
	psy_Property* property;

	style = psy_ui_style(style_id);
	if (style) {
		psy_ui_style_set_colour(style, colour);
	}
	property = psy_property_set_int(self->theme, key,
		psy_ui_colour_colorref(&colour));
	if (property) {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}

void machineviewconfig_set_style_background_colour(MachineViewConfig* self,
	const char* key, uintptr_t style_id, psy_ui_Colour colour)
{
	psy_ui_Style* style;
	psy_Property* property;

	style = psy_ui_style(style_id);
	if (style) {
		psy_ui_style_set_background_colour(style, colour);
	}
	property = psy_property_set_int(self->theme, key,
		psy_ui_colour_colorref(&colour));
	if (property) {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}
