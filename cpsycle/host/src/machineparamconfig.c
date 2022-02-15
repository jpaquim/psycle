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
/* audio */
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"

static void machineparamconfig_makeview(MachineParamConfig*, psy_Property*);
static void machineparamconfig_maketheme(MachineParamConfig*, psy_Property*);

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

	self->paramview = psy_property_settext(
		psy_property_append_section(parent, "paramview"),
		"settingsview.paramview.native-machine-parameter-window");
	psy_property_settext(
		psy_property_append_font(self->paramview, "font", PSYCLE_DEFAULT_FONT),
		"settingsview.paramview.font");
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->paramview, "loadcontrolskin"),
		"settingsview.paramview.load-dial-bitmap"),
		PROPERTY_ID_LOADCONTROLSKIN);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->paramview, "defaultskin"),
		"settingsview.paramview.default-skin"),
		PROPERTY_ID_DEFAULTCONTROLSKIN);
	machineparamconfig_maketheme(self, self->paramview);
}

void machineparamconfig_maketheme(MachineParamConfig* self, psy_Property* parent)
{
	assert(self);

	self->theme = psy_property_settext(
		psy_property_append_section(parent, "theme"),
		"settingsview.paramview.theme.theme");
	psy_property_settext(
		psy_property_sethint(psy_property_append_str(self->theme,
			"machinedialbmp", ""),
			PSY_PROPERTY_HINT_EDIT),
		"settingsview.paramview.theme.machinedialbmp");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguititlecolour", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.title-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguititlefontcolour", 0x00B4B4B4, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.title-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguitopcolour", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.param-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguifonttopcolour", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.param-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguibottomcolour", 0x00444444, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.value-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguifontbottomcolour", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.value-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihtopcolour", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selparam-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihfonttopcolour", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selparam-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihbottomcolour", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selvalue-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->theme,
			"machineguihfontbottomcolour", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"settingsview.paramview.theme.selvalue-font");
}

void machineparamconfig_resettheme(MachineParamConfig* self)
{
	if (self->theme) {
		psy_property_remove(self->paramview, self->theme);
	}
	machineparamconfig_maketheme(self, self->paramview);
	machineparamconfig_updatestyles(self);
}

void machineparamconfig_settheme(MachineParamConfig* self, psy_Property* theme)
{
	assert(self);

	if (self->theme) {
		psy_property_sync(self->theme, theme);
		machineparamconfig_updatestyles(self);		
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
	// machineparamconfig_releaseskin();
	/* forces a reload of the dialbitmap */
	// machineparamconfig_skin(self);
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

/* events */
bool machineparamconfig_onchanged(MachineParamConfig* self, psy_Property*
	property)
{
	assert(self);

	if (machineparamconfig_hasthemeproperty(self, property)) {
		machineparamconfig_updatestyles(self);
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
	return TRUE;
}

void machineparamconfig_updatestyles(MachineParamConfig* self)
{
	assert(self);

	if (self->theme) {
		psy_ui_Style* style;

		style = psy_ui_style(STYLE_MACPARAM_TITLE);
		if (style) {
			psy_ui_style_setcolours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguititlefontcolour", 0x00B4B4B4)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguititlecolour", 0x00292929)));
		}
		style = psy_ui_style(STYLE_MACPARAM_TOP);
		if (style) {
			psy_ui_style_setcolours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguifonttopcolour", 0x00CDCDCD)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguitopcolour", 0x00555555)));
		}
		style = psy_ui_style(STYLE_MACPARAM_BOTTOM);
		if (style) {
			psy_ui_style_setcolours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguifontbottompcolour", 0x00E7BD18)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguibottomcolour", 0x00444444)));
		}
		style = psy_ui_style(STYLE_MACPARAM_TOP_ACTIVE);
		if (style) {
			psy_ui_style_setcolours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguihfonttopcolour", 0x00CDCDCD)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguihtopcolour", 0x00555555)));
		}
		style = psy_ui_style(STYLE_MACPARAM_BOTTOM_ACTIVE);
		if (style) {
			psy_ui_style_setcolours(style,
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguihfontbottomcolour", 0x00E7BD18)),
				psy_ui_colour_make(psy_property_at_colour(self->theme,
					"machineguihbottomcolour", 0x00292929)));
		}
		style = psy_ui_style(STYLE_MACPARAM_KNOB);		
		if (psy_strlen(machineparamconfig_dialbpm(self)) == 0 ||
			psy_ui_style_setbackgroundpath(style, 
				machineparamconfig_dialbpm(self)) != PSY_OK) {
			psy_ui_style_setbackgroundid(style, IDB_PARAMKNOB);
		}				
	}
}

psy_ui_RealSize mpfsize(const psy_ui_TextMetric* tm, uintptr_t paramtype,
	bool issmall)
{
	static float SMALLDIV = 1.6f;
	psy_ui_RealSize rv;
	psy_ui_Style* style;

	assert(tm);

	switch (paramtype) {
	case MPF_IGNORE:
		rv.height = 0;
		rv.width = 0;
		break;
	case MPF_CHECK: {
		psy_ui_Style* checkoff_style;

		checkoff_style = psy_ui_style(STYLE_MACPARAM_CHECKOFF);		
		rv.height = psy_max(
			checkoff_style->background.size.height,
			tm->tmHeight);
		rv.width = checkoff_style->background.size.width +
			tm->tmAveCharWidth * 5;
		break; }
	case MPF_SLIDER: {
		psy_ui_Style* checkoff_style;
		psy_ui_Style* vuon_style;

		checkoff_style = psy_ui_style(STYLE_MACPARAM_CHECKOFF);		
		vuon_style = psy_ui_style(STYLE_MACPARAM_VUON);
		rv = checkoff_style->background.size;
		if (rv.width < tm->tmAveCharWidth * 30) {
			rv.width = tm->tmAveCharWidth * 30;
		}
		if (issmall) {
			rv.width = rv.width / SMALLDIV;
		}
		if (rv.width < vuon_style->background.size.width +
			checkoff_style->background.size.width + 50 +
			tm->tmAveCharWidth * 5) {
			rv.width = vuon_style->background.size.width +
				checkoff_style->background.size.width + 50 +
				tm->tmAveCharWidth * 5;
		}
		break; }
	case MPF_LEVEL: {
		psy_ui_Style* vuon_style;

		vuon_style = psy_ui_style(STYLE_MACPARAM_VUON);
		rv.height = vuon_style->background.size.height;
		style = psy_ui_style(STYLE_MACPARAM_SLIDER);
		rv.width = style->background.size.width;
		if (issmall) {
			rv.width = rv.width / SMALLDIV;
		}
		break; }
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

