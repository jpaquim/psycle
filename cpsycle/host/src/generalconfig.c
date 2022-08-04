/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "generalconfig.h"
/* host */
#include "resources/resource.h"

static void generalconfig_make(GeneralConfig*, psy_Property*);
static void generalconfig_make_workbench(GeneralConfig*, psy_Property* parent);

void generalconfig_init(GeneralConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	generalconfig_make(self, parent);
	psy_signal_init(&self->signal_changed);
}

void generalconfig_dispose(GeneralConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void generalconfig_make(GeneralConfig* self, psy_Property* parent)
{
	assert(self);

	self->general = psy_property_set_text(
		psy_property_append_section(parent, "general"),
		"settings.general.general");
	psy_property_set_icon(self->general, IDB_TOGGLE_LIGHT,
		IDB_TOGGLE_DARK);
	psy_property_hide(psy_property_set_text(
		psy_property_append_str(self->general, "version", "alpha"),
		"settings.general.version"));	
	psy_property_set_text(
		psy_property_append_bool(self->general, "showsonginfoonload", TRUE),
		"settings.general.show-song-info-on-load");		
	psy_property_set_text(
		psy_property_append_bool(self->general, "playsongafterload", TRUE),
		"settings.general.play-song-after-load");
	psy_property_set_text(
		psy_property_append_bool(self->general, "showpatternnames", FALSE),
		"settings.general.show-pattern-names");
	psy_property_set_text(
		psy_property_append_bool(self->general, "saverecentsongs", TRUE),
		"settings.general.save-recent-songs");
	generalconfig_make_workbench(self, self->general);
}

void generalconfig_make_workbench(GeneralConfig* self, psy_Property* parent)
{
	psy_Property* bench;
	
	bench = psy_property_set_text(psy_property_append_section(parent,
		"bench"), "settings.general.bench");
	psy_property_set_text(
		psy_property_append_bool(bench, "showaboutatstart", TRUE),
		"settings.general.show-about-at-startup");		
	psy_property_set_text(
		psy_property_append_bool(bench, "showmaximizedatstart", TRUE),
		"settings.general.show-maximized-at-startup");
	psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(bench, "showsequenceedit", FALSE),
		"settings.general.show-sequenceedit"),
		PROPERTY_ID_SHOWSEQUENCEEDIT);
	psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(bench, "showstepsequencer", FALSE),
		"settings.general.show-sequencestepbar"),
		PROPERTY_ID_SHOWSTEPSEQUENCER);
	psy_property_set_text(
		psy_property_append_bool(bench, "showpianokbd", FALSE),
		"settings.general.show-pianokbd");
	psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(bench, "showplaylist", FALSE),
		"settings.general.show-playlist"),
		PROPERTY_ID_SHOWPLAYLIST);
	psy_property_set_text(
		psy_property_append_bool(bench, "showplugineditor", FALSE),
		"settings.general.show-plugineditor");		
	psy_property_set_text(
			psy_property_append_bool(bench, "showparamrack", FALSE),
		"settings.general.show-paramrack");		
	psy_property_set_text(
		psy_property_append_bool(bench, "showgear", FALSE),
		"settings.general.show-gear");
	psy_property_set_text(
		psy_property_append_bool(bench, "showmidi", FALSE),
		"settings.general.show-midi");
	psy_property_set_text(
		psy_property_append_bool(bench, "showcpu", FALSE),
		"settings.general.show-cpu");
}

bool generalconfig_showing_song_info_on_load(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showsonginfoonload", TRUE);
}

void generalconfig_show_song_info_on_load(GeneralConfig* self)
{
	assert(self);

	psy_property_set_bool(self->general, "showsonginfoonload", TRUE);
}

void generalconfig_prevent_song_info_on_load(GeneralConfig* self)
{
	assert(self);

	psy_property_set_bool(self->general, "showsonginfoonload", FALSE);
}

void generalconfig_show_about_at_start(GeneralConfig* self)
{
	assert(self);

	psy_property_set_bool(self->general, "bench.showaboutatstart", TRUE);
}

void generalconfig_hide_about_at_start(GeneralConfig* self)
{
	assert(self);

	psy_property_set_bool(self->general, "bench.showaboutatstart", FALSE);
}

bool generalconfig_showing_about_at_start(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "bench.showaboutatstart", TRUE);
}

ViewIndex generalconfig_start_view(const GeneralConfig* self)
{
	if (generalconfig_showing_about_at_start(self)) {
		return viewindex_make(VIEW_ID_HELPVIEW,
			SECTION_ID_HELPVIEW_ABOUT, 0, psy_INDEX_INVALID);
	}
	return viewindex_make(VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_WIRES, 0, psy_INDEX_INVALID);
}

bool generalconfig_show_maximized_at_start(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "bench.showmaximizedatstart", TRUE);
}

bool generalconfig_saving_recent_songs(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "saverecentsongs", TRUE);
}


void generalconfig_show_saving_recent_songs(GeneralConfig* self)
{
	assert(self);

	psy_property_set_bool(self->general, "saverecentsongs", TRUE);
}

void generalconfig_prevent_saving_recent_songs(GeneralConfig* self)
{
	assert(self);

	psy_property_set_bool(self->general, "saverecentsongs", FALSE);
}

bool generalconfig_playsongafterload(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "playsongafterload", TRUE);
}

bool generalconfig_showing_pattern_names(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showpatternnames", TRUE);
}

bool generalconfig_showsequenceedit(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "bench.showsequenceedit", FALSE);
}

void generalconfig_setsequenceeditshowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "bench.showsequenceedit", state);
}

bool generalconfig_showstepsequencer(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "bench.showstepsequencer", FALSE);
}

bool generalconfig_showpianokbd(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "bench.showpianokbd", FALSE);
}

bool generalconfig_showplaylist(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "bench.showplaylist", TRUE);
}

void generalconfig_setplaylistshowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "bench.showplaylist", state);
}

void generalconfig_setstepsequencershowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "showstepsequencer", state);
}

void generalconfig_setpianokbdshowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "bench.showpianokbd", state);
}

void generalconfig_show_pattern_names(GeneralConfig* self)
{
	assert(self);
	
	psy_property_set_bool(self->general, "showpatternnames", TRUE);
}

void generalconfig_show_pattern_ids(GeneralConfig* self)
{
	assert(self);
	
	psy_property_set_bool(self->general, "showpatternnames", FALSE);
}

bool generalconfig_hasproperty(const GeneralConfig* self,
	psy_Property* property)
{
	assert(self && self->general);

	return psy_property_in_section(property, self->general);
}

bool generalconfig_connect(GeneralConfig* self, const char* key, void* context,
	void* fp)
{
	psy_Property* p;
	
	assert(self);

	p = generalconfig_property(self, key);
	if (p) {
		psy_property_connect(p, context, fp);
		psy_property_notify(p);
		return TRUE;
	}
	return FALSE;
}

psy_Property* generalconfig_property(GeneralConfig* self, const char* key)
{
	assert(self);

	return psy_property_at(self->general, key, PSY_PROPERTY_TYPE_NONE);	
}
