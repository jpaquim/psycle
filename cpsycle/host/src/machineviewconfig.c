// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineviewconfig.h"

static void machineviewconfig_makeview(MachineViewConfig*,
	psy_Property* parent);
static void machineviewconfig_makestackview(MachineViewConfig*,
	psy_Property* parent);
static void machineviewconfig_maketheme(MachineViewConfig*,
	psy_Property* parent);

void machineviewconfig_init(MachineViewConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	machineviewconfig_makeview(self, parent);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_themechanged);
}

void machineviewconfig_dispose(MachineViewConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_themechanged);
}

void machineviewconfig_makeview(MachineViewConfig* self, psy_Property* parent)
{	
	assert(self);

	self->machineview = psy_property_settext(
		psy_property_append_section(parent, "machineview"),
		"settingsview.machineview");
	psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawmachineindexes", TRUE),
		"settingsview.draw-machine-indexes");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawvumeters", TRUE),
		"settingsview.draw-vumeters"),
		PROPERTY_ID_DRAWVUMETERS);
	psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawwirehover", FALSE),
		"settingsview.draw-wirehover");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->machineview,
			"drawvirtualgenerators", FALSE),
		"settingsview.draw-virtualgenerators"),
		PROPERTY_ID_DRAWVIRTUALGENERATORS);
	machineviewconfig_makestackview(self, self->machineview);
	machineviewconfig_maketheme(self, self->machineview);
}

void machineviewconfig_makestackview(MachineViewConfig* self, psy_Property* parent)
{
	self->stackview = psy_property_settext(
		psy_property_append_section(parent, "stackview"),
		"settingsview.stackview");
	psy_property_settext(
		psy_property_append_bool(self->stackview,
			"drawsmalleffects", FALSE),
		"settingsview.stackview-draw-smalleffects");
}

void machineviewconfig_maketheme(MachineViewConfig* self, psy_Property* parent)
{
	assert(self);

	self->theme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"settingsview.theme");	
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"vu2", 0x00403731, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.vu-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"vu1", 0x0080FF80, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.vu-bar");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"vu3", 0x00262bd7, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.onclip");
	psy_property_settext(
		psy_property_append_string(self->theme,
			"generator_fontface", "Tahoma"),
		"settingsview.generators-font-face");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"generator_font_point", 0x00000050, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.generators-font-point");
	psy_property_settext(psy_property_sethint(
		psy_property_append_int(self->theme,
			"generator_font_flags", 0x00000000, 0, 0),
		PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.generator_font_flags");
	psy_property_settext(psy_property_append_string(self->theme,
		"effect_fontface", "Tahoma"),
		"settingsview.effect_fontface");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"effect_font_point", 0x00000050, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.effect_font_point");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"effect_font_flags", 0x00000000, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.effect_font_flags");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_colour", 0x00232323, 0, 0), //
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wirecolour", 0x005F5F5F, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.wirecolour");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wirecolour2", 0x005F5F5F, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.wirecolour2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_polycolour", 0x00B1C8B0, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.polygons");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_generator_fontcolour", 0x00B1C8B0, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.generators-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_effect_fontcolour", 0x00D1C5B6, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.effects-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wirewidth", 0x00000001, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.wire-width");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_wireaa", 0x01, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.antialias-halo");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machine_background", 0, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.machine-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"mv_triangle_size", 0x0A, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.polygon-size");
	psy_property_append_string(self->theme, "machine_skin", "");//	
}

void machineviewconfig_resettheme(MachineViewConfig* self)
{	
	assert(self);

	if (self->theme) {
		psy_property_remove(self->machineview, self->theme);
	}
	machineviewconfig_maketheme(self, self->machineview);
	psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
}

void machineviewconfig_settheme(MachineViewConfig* self, psy_Property* skin)
{
	assert(self);

	if (self->theme) {
		psy_property_sync(self->theme, skin);
		psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
	}
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


// events
bool machineviewconfig_onchanged(MachineViewConfig* self, psy_Property*
	property)
{
	assert(self);

	if (machineviewconfig_hasthemeproperty(self, property)) {
		machineviewconfig_onthemechanged(self, property);		
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
	return TRUE;
}

bool machineviewconfig_onthemechanged(MachineViewConfig* self, psy_Property* property)
{
	assert(self);

	psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
	return TRUE;
}
