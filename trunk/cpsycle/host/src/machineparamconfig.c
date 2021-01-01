// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineparamconfig.h"

static void machineparamconfig_makeview(MachineParamConfig*, psy_Property*);
static void machineparamconfig_maketheme(MachineParamConfig*, psy_Property*);

void machineparamconfig_init(MachineParamConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
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

void machineparamconfig_makeview(MachineParamConfig* self, psy_Property* parent)
{
	assert(self);

	self->paramview = psy_property_settext(
		psy_property_append_section(parent, "paramview"),
		"settingsview.native-machine-parameter-window");
	psy_property_settext(
		psy_property_append_font(self->paramview, "font", PSYCLE_DEFAULT_FONT),
		"settingsview.font");
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->paramview, "loadcontrolskin"),
		"settingsview.load-dial-bitmap"),
		PROPERTY_ID_LOADCONTROLSKIN);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->paramview, "defaultskin"),
		"settingsview.default-skin"),
		PROPERTY_ID_DEFAULTCONTROLSKIN);
	psy_property_settext(
		psy_property_append_bool(self->paramview, "showaswindow", 1),
		"settingsview.show-as-window");
	machineparamconfig_maketheme(self, self->paramview);
}

void machineparamconfig_maketheme(MachineParamConfig* self, psy_Property* parent)
{
	assert(self);

	self->theme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"theme");
	psy_property_settext(
		psy_property_sethint(psy_property_append_string(self->theme,
			"machinedialbmp", ""),
			PSY_PROPERTY_HINT_EDIT),
		"settingsview.machinedialbmp");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguititlecolour", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.title-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguititlefontcolour", 0x00B4B4B4, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.title-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguitopcolour", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.param-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguifonttopcolour", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.param-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguibottomcolour", 0x00444444, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.value-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguifontbottomcolour", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.value-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihtopcolour", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selparam-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihfonttopcolour", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selparam-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihbottomcolour", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selvalue-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihfontbottomcolour", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selvalue-font");
}

void machineparamconfig_resettheme(MachineParamConfig* self)
{	
	if (self->theme) {
		psy_property_remove(self->paramview, self->theme);
	}
	machineparamconfig_maketheme(self, self->paramview);
}

void machineparamconfig_settheme(MachineParamConfig* self, psy_Property* skin)
{
	assert(self);

	if (self->theme) {
		psy_property_sync(self->theme, skin);
	}
}

bool machineparamconfig_hasthemeproperty(const MachineParamConfig* self,
	psy_Property* property)
{
	return (self->theme && psy_property_insection(property, self->theme));
}

bool machineparamconfig_hasproperty(const MachineParamConfig* self,
	psy_Property* property)
{
	assert(self && self->paramview);

	return psy_property_insection(property, self->paramview);
}

bool machineparamconfig_showfloated(const MachineParamConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->paramview, "showaswindow", TRUE);
}

const char* machineparamconfig_dialbpm(const MachineParamConfig* self)
{
	assert(self);

	return psy_property_at_str(self->theme, "machinedialbmp", "");
}

void machineparamconfig_setdialbpm(MachineParamConfig* self, const char* path)
{
	assert(self);

	psy_property_set_str(self->theme, "machinedialbmp", path);
}

// events
bool machineparamconfig_onchanged(MachineParamConfig* self, psy_Property*
	property)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, property);
	return TRUE;
}

bool machineparamconfig_onthemechanged(MachineParamConfig* self, psy_Property* property)
{
	assert(self);

	psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
	return TRUE;
}
