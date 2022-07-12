/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineparamconfig.h"
/* host */
#include "dirconfig.h"
#include "paramview.h"
#include "resources/resource.h"
#include "styles.h"
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* audio */
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"

static void machineparamconfig_makeview(MachineParamConfig*, psy_Property*);
static void machineparamconfig_maketheme(MachineParamConfig*, psy_Property*);
static void machineparamconfig_set_colour(MachineParamConfig*,
	const char* key, uintptr_t style_id, psy_ui_Colour);
static void machineparamconfig_set_style_background_colour(MachineParamConfig*,
	const char* key, uintptr_t style_id, psy_ui_Colour);
static void machineparamconfig_on_property_changed(MachineParamConfig*,
	psy_Property* sender);

void machineparamconfig_init(MachineParamConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	self->dirconfig = NULL;
	machineparamconfig_makeview(self, parent);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_themechanged);
}

void machineparamconfig_dispose(MachineParamConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_themechanged);	
}


void machineparamconfig_setdirectories(MachineParamConfig* self,
	DirConfig* dirconfig)
{
	self->dirconfig = dirconfig;
}

void machineparamconfig_makeview(MachineParamConfig* self, psy_Property* parent)
{
	assert(self);

	self->paramview = psy_property_set_text(
		psy_property_append_section(parent, "paramview"),
		"settingsview.paramview.native-machine-parameter-window");
	psy_property_set_text(
		psy_property_append_font(self->paramview, "font", PSYCLE_DEFAULT_FONT),
		"settingsview.paramview.font");
	psy_property_set_id(psy_property_set_text(
		psy_property_append_action(self->paramview, "loadcontrolskin"),
		"settingsview.paramview.load-dial-bitmap"),
		PROPERTY_ID_LOADCONTROLSKIN);
	psy_property_set_id(psy_property_set_text(
		psy_property_append_action(self->paramview, "defaultskin"),
		"settingsview.paramview.default-skin"),
		PROPERTY_ID_DEFAULTCONTROLSKIN);
	machineparamconfig_maketheme(self, self->paramview);
}

void machineparamconfig_maketheme(MachineParamConfig* self, psy_Property* parent)
{
	psy_List* p;

	assert(self);

	self->theme = psy_property_set_text(
		psy_property_append_section(parent, "theme"),
		"settingsview.paramview.theme.theme");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_str(self->theme,
			"machinedial_bmp", ""),
			PSY_PROPERTY_HINT_EDIT),
		"settingsview.paramview.theme.machinedialbmp");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUITitleColor", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.title-background");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUITitleFontColor", 0x00B4B4B4, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.title-font");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUITopColor", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.param-background");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIFontTopColor", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.param-font");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIBottomColor", 0x00444444, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.value-background");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIFontBottomColor", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.value-font");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIHTopColor", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selparam-background");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIHFontTopColor", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selparam-font");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIHBottomColor", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selvalue-background");
	psy_property_set_text(
		psy_property_set_hint(psy_property_append_int(self->theme,
			"machineGUIHFontBottomColor", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selvalue-font");
	for (p = psy_property_begin(self->theme); p != NULL; p = p->next) {
		psy_Property* curr;

		curr = (psy_Property*)p->entry;
		psy_property_connect(curr, self, 
			machineparamconfig_on_property_changed);
	}
}

void machineparamconfig_resettheme(MachineParamConfig* self)
{
	if (self->theme) {
		psy_property_remove(self->paramview, self->theme);
	}
	machineparamconfig_maketheme(self, self->paramview);
	machineparamconfig_update_styles(self);
}

void machineparamconfig_settheme(MachineParamConfig* self, psy_Property* theme)
{
	assert(self);

	if (self->theme) {
		psy_property_sync(self->theme, theme);
		machineparamconfig_update_styles(self);		
	}
}

bool machineparamconfig_hasthemeproperty(const MachineParamConfig* self,
	psy_Property* property)
{
	return (self->theme && psy_property_in_section(property, self->theme));
}

bool machineparamconfig_hasproperty(const MachineParamConfig* self,
	psy_Property* property)
{
	assert(self && self->paramview);

	return psy_property_in_section(property, self->paramview);
}

const char* machineparamconfig_dialbpm(const MachineParamConfig* self)
{
	assert(self);

	return psy_property_at_str(self->theme, "machinedial_bmp", "");
}

void machineparamconfig_set_dial_bpm(MachineParamConfig* self,
	const char* filename)
{
	assert(self);

	psy_property_set_str(self->theme, "machinedial_bmp", filename);	
}

psy_ui_FontInfo machineparamconfig_fontinfo(const MachineParamConfig* self)
{
	psy_ui_FontInfo rv;

	assert(self);
	assert(self->paramview);

	psy_ui_fontinfo_init_string(&rv,
		psy_property_at_str(self->paramview, "font", "tahoma; 16"));
	return rv;
}

void machineparamconfig_on_property_changed(MachineParamConfig* self,
	psy_Property* sender)
{
	assert(self);

	if (machineparamconfig_hasthemeproperty(self, sender)) {
		machineparamconfig_update_styles(self);				
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, sender);
	}
}

void machineparamconfig_update_styles(MachineParamConfig* self)
{
	assert(self);

	if (self->theme) {
		psy_ui_Style* style;

		style = psy_ui_style(STYLE_MACPARAM_TITLE);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUITitleFontColor", 0x00B4B4B4)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUITitleColor", 0x00292929)));
		}
		style = psy_ui_style(STYLE_MACPARAM_TOP);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUIFontTopColor", 0x00CDCDCD)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUITopColor", 0x00555555)));
		}
		style = psy_ui_style(STYLE_MACPARAM_BOTTOM);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguifontbottompcolour", 0x00E7BD18)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUIBottomColor", 0x00444444)));
		}
		style = psy_ui_style(STYLE_MACPARAM_TOP_ACTIVE);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUIHFontTopColor", 0x00CDCDCD)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUIHTopColor", 0x00555555)));
		}
		style = psy_ui_style(STYLE_MACPARAM_BOTTOM_ACTIVE);
		if (style) {
			psy_ui_style_set_colours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUIHFontBottomColor", 0x00E7BD18)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineGUIHBottomColor", 0x00292929)));
		}
		style = psy_ui_style(STYLE_MACPARAM_KNOB);		
		if (psy_strlen(machineparamconfig_dialbpm(self)) == 0 ||
			psy_ui_style_setbackgroundpath(style, 
				machineparamconfig_dialbpm(self)) != PSY_OK) {
			psy_ui_style_set_background_id(style, IDB_PARAMKNOB);
		}				
	}
}

void machineparamconfig_set_param_top_colour(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_colour(self, "machineGUIFontTopColor",
		STYLE_MACPARAM_TOP, colour);
}

void machineparamconfig_set_param_top_background_colour(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_style_background_colour(self, "machineGUITopColor",
		STYLE_MACPARAM_TOP, colour);
}

void machineparamconfig_set_param_top_colour_active(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_colour(self, "machineGUIHFontTopColor",
		STYLE_MACPARAM_TOP_ACTIVE, colour);
}

void machineparamconfig_set_param_top_background_colour_active(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_style_background_colour(self, "machineGUIHTopColor",
		STYLE_MACPARAM_TOP_ACTIVE, colour);
}

void machineparamconfig_set_param_bottom_colour(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_colour(self, "machineGUIFontBottomColor",
		STYLE_MACPARAM_BOTTOM, colour);
}

void machineparamconfig_set_param_bottom_background_colour(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_style_background_colour(self, "machineGUIBottomColor",
		STYLE_MACPARAM_BOTTOM, colour);
}

void machineparamconfig_set_param_bottom_colour_active(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_colour(self, "machineGUIHFontBottomColor",
		STYLE_MACPARAM_BOTTOM_ACTIVE, colour);
}

void machineparamconfig_set_param_bottom_background_colour_active(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_style_background_colour(self, "machineGUIHBottomColor",
		STYLE_MACPARAM_BOTTOM, colour);
}

void machineparamconfig_set_param_title_colour(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_colour(self, "machineGUITitleFontColor",
		STYLE_MACPARAM_TITLE, colour);
}

void machineparamconfig_set_param_title_background_colour(MachineParamConfig* self,
	psy_ui_Colour colour)
{
	machineparamconfig_set_style_background_colour(self, "machineGUITitleColor",
		STYLE_MACPARAM_TITLE, colour);
}

void machineparamconfig_set_param_machinedial_bmp(MachineParamConfig* self,
	const char* str)
{
	psy_Property* property;
	
	property = psy_property_set_str(self->theme, "machinedial_bmp", str);
	if (property) {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}

void machineparamconfig_set_colour(MachineParamConfig* self,
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

void machineparamconfig_set_style_background_colour(MachineParamConfig* self,
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
