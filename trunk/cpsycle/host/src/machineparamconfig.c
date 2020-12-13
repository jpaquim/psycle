// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineparamconfig.h"

static void machineparamconfig_makeparamview(MachineParamConfig*, psy_Property*);
// static void machineparamconfig_makeparamtheme(MachineParamConfig*, psy_Property*);

void machineparamconfig_init(MachineParamConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	machineparamconfig_makeparamview(self, parent);
}

void machineparamconfig_makeparamview(MachineParamConfig* self, psy_Property* parent)
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
	psy_property_settext(
		psy_property_append_bool(self->paramview, "showaswindow", 1),
		"settingsview.show-as-window");
	machineparamconfig_makeparamtheme(self, self->paramview);
}

void machineparamconfig_makeparamtheme(MachineParamConfig* self, psy_Property* parent)
{
	assert(self);

	self->paramtheme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"theme");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguititlecolour", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.title-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguititlefontcolour", 0x00B4B4B4, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.title-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguitopcolour", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.param-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguifonttopcolour", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.param-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguibottomcolour", 0x00444444, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.value-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguifontbottomcolour", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.value-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihtopcolour", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selparam-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihfonttopcolour", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selparam-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihbottomcolour", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selvalue-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihfontbottomcolour", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.selvalue-font");
}

bool machineparamconfig_showfloated(const MachineParamConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->paramview, "showaswindow", TRUE);
}

