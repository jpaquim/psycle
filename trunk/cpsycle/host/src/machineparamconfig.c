// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineparamconfig.h"
// host
#include "paramview.h"
#include "resources/resource.h"
// audio
#include <plugin_interface.h>
// platform
#include "../../detail/portable.h"

static bool paramskin_initialized = FALSE;
static ParamSkin paramskin;

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
	machineparamconfig_releaseskin();
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
	if (paramskin_initialized) {
		machineparamconfig_releaseskin();
		// forces a reload of the dialbitmap
		machineparamconfig_skin(self);
	}
}

void machineparamconfig_settheme(MachineParamConfig* self, psy_Property* theme)
{
	assert(self);

	if (self->theme) {
		psy_property_sync(self->theme, theme);
		if (paramskin_initialized) {
			machineparamconfig_releaseskin();
			// forces a reload of the dialbitmap
			machineparamconfig_skin(self);
		}
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

void machineparamconfig_setdialbpm(MachineParamConfig* self,
	const char* filename)
{
	assert(self);
	
	psy_property_set_str(self->theme, "machinedialbmp", filename);
	machineparamconfig_releaseskin();
	// forces a reload of the dialbitmap
	machineparamconfig_skin(self);
}

psy_ui_FontInfo machineparamconfig_fontinfo(const MachineParamConfig* self)
{
	psy_ui_FontInfo rv;

	assert(self);
	assert(self->paramview);
	
	psy_ui_fontinfo_init_string(&rv,
		psy_property_at_str(self->paramview, "font", "tahoma;-16"));
	return rv;
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

	machineparamconfig_releaseskin();
	// forces a reload of the dialbitmap
	machineparamconfig_skin(self);
	psy_signal_emit(&self->signal_themechanged, self, 1, self->theme);
	return TRUE;
}

ParamSkin* machineparamconfig_skin(MachineParamConfig* self)
{
	if (!paramskin_initialized) {
		psy_Property* theme;
		SkinCoord knob;
		SkinCoord slider;
		SkinCoord sliderknob;
		SkinCoord vuoff;
		SkinCoord vuon;
		SkinCoord switchon;
		SkinCoord switchoff;
		SkinCoord checkon;
		SkinCoord checkoff;

		theme = self->theme;
		skincoord_init_all(&knob, 0.0, 0.0, 28.0, 28.0, 0.0, 0.0, 28.0, 28.0, 0.0);
		skincoord_init_all(&slider, 0.0, 0.0, 30.0, 182.0, 0.0, 0.0, 30.0, 182.0, 0.0);
		skincoord_init_all(&sliderknob, 0.0, 182.0, 22.0, 10.0, 0.0, 0.0, 22.0, 10.0, 0.0);
		skincoord_init_all(&vuoff, 30.0, 0.0, 16.0, 90.0, 0.0, 0.0, 16.0, 90.0, 0.0);
		skincoord_init_all(&vuon, 46.0, 0.0, 16.0, 90.0, 0.0, 0.0, 16.0, 90.0, 0.0);
		skincoord_init_all(&switchon, 30.0, 118.0, 28.0, 28.0, 0.0, 0.0, 28.0, 28.0, 0.0);
		skincoord_init_all(&switchoff, 30.0, 90.0, 28.0, 28.0, 0.0, 0.0, 28.0, 28.0, 0.0);
		skincoord_init_all(&checkon, 30.0, 159.0, 13.0, 13.0, 0.0, 0.0, 13.0, 13.0, 0.0);
		skincoord_init_all(&checkoff, 30.0, 146.0, 13.0, 13.0, 0.0, 0.0, 13.0, 13.0, 0.0);

		paramskin.slider = slider;
		paramskin.sliderknob = sliderknob;
		paramskin.knob = knob;
		paramskin.vuoff = vuoff;
		paramskin.vuon = vuon;
		paramskin.switchon = switchon;
		paramskin.switchoff = switchoff;
		paramskin.checkon = checkon;
		paramskin.checkoff = checkoff;

		paramskin.topcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguitopcolour", 0x00555555));
		paramskin.fonttopcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguifonttopcolour", 0x00CDCDCD));
		paramskin.bottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguibottomcolour", 0x00444444));
		paramskin.fontbottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguifontbottomcolour", 0x00E7BD18));
		//highlighted param colours
		paramskin.htopcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihtopcolour", 0x00555555));
		paramskin.fonthtopcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihfonttopcolour", 0x00CDCDCD));
		paramskin.hbottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihbottomcolour", 0x00292929));
		paramskin.fonthbottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihfontbottomcolour", 0x00E7BD18));

		paramskin.titlecolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguititlecolour", 0x00292929));
		paramskin.fonttitlecolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguititlefontcolour", 0x00B4B4B4));
		psy_ui_bitmap_init(&paramskin.knobbitmap);
		if (psy_strlen(machineparamconfig_dialbpm(self)) == 0) {
			psy_ui_bitmap_loadresource(&paramskin.knobbitmap, IDB_PARAMKNOB);
		} else if (psy_ui_bitmap_load(&paramskin.knobbitmap,
			machineparamconfig_dialbpm(self)) != PSY_OK) {
			psy_ui_bitmap_loadresource(&paramskin.knobbitmap, IDB_PARAMKNOB);
		}
		psy_ui_bitmap_init(&paramskin.mixerbitmap);
		psy_ui_bitmap_loadresource(&paramskin.mixerbitmap, IDB_MIXERSKIN);
		paramskin.paramwidth = 26.0;
		paramskin.paramwidth_small = 18.0;
		paramskin_initialized = TRUE;
	}
	return &paramskin;
}

void machineparamconfig_releaseskin(void)
{
	if (paramskin_initialized) {		
		psy_ui_bitmap_dispose(&paramskin.knobbitmap);
		psy_ui_bitmap_dispose(&paramskin.mixerbitmap);
		paramskin_initialized = 0;
	}
}

psy_ui_RealSize mpfsize(ParamSkin* skin, const psy_ui_TextMetric* tm, uintptr_t paramtype,
	bool issmall)
{
	static float SMALLDIV = 1.6f;
	psy_ui_RealSize rv;

	assert(skin);
	assert(tm);

	switch (paramtype) {
	case MPF_IGNORE:
		rv.height = 0;
		rv.width = 0;
		break;
	case MPF_CHECK:
		rv.height = psy_max(psy_ui_realrectangle_height(&skin->checkoff.dest),
			tm->tmHeight);
		rv.width = psy_ui_realrectangle_height(&skin->checkoff.dest) +
			tm->tmAveCharWidth * 5;
		break;
	case MPF_SLIDER:
		rv.height = psy_ui_realrectangle_height(&skin->slider.dest);
		rv.width = psy_ui_realrectangle_width(&skin->slider.dest);
		if (rv.width < tm->tmAveCharWidth * 30) {
			rv.width = tm->tmAveCharWidth * 30;
		}
		if (issmall) {
			rv.width = rv.width / SMALLDIV;
		}
		if (rv.width < psy_ui_realrectangle_width(&skin->vuon.dest) +
			psy_ui_realrectangle_width(&skin->checkoff.dest) + 50 +
			tm->tmAveCharWidth * 5) {
			rv.width = psy_ui_realrectangle_width(&skin->vuon.dest) +
				psy_ui_realrectangle_width(&skin->checkoff.dest) + 50 +
				tm->tmAveCharWidth * 5;
		}
		break;
	case MPF_LEVEL:
		rv.height = psy_ui_realrectangle_height(&skin->vuon.dest);
		rv.width = psy_ui_realrectangle_width(&skin->slider.dest);
		if (issmall) {
			rv.width = rv.width / SMALLDIV;
		}
		break;
	default:
		rv.width = tm->tmAveCharWidth * 30;
		rv.height = tm->tmHeight * 2;
		if (issmall) {
			rv.width = rv.width / SMALLDIV;
		}
		break;
	}
	return rv;
}

