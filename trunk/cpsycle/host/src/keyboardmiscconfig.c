/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "keyboardmiscconfig.h"
/* host */
#include "resources/resource.h"
/* audio */
#include <pattern.h>


/* prototypes */
static void keyboardmiscconfig_make(KeyboardMiscConfig*, psy_Property*);
static void keyboardmiscconfig_on_follow_song(KeyboardMiscConfig*,
	psy_Property* sender);
static void keyboardmiscconfig_on_pattern_default_lines(KeyboardMiscConfig*,
	psy_Property* sender);

/* implementation */
void keyboardmiscconfig_init(KeyboardMiscConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	self->cursorstep = 1;
	self->follow_song = FALSE;
	keyboardmiscconfig_make(self, parent);
	psy_signal_init(&self->signal_changed);
}

void keyboardmiscconfig_dispose(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void keyboardmiscconfig_make(KeyboardMiscConfig* self, psy_Property* parent)
{
	psy_Property* choice;

	assert(self);

	self->keyboard = psy_property_set_text(
		psy_property_append_section(parent, "keyboard"),
		"settings.kbd.kbd-misc");
	psy_property_set_icon(self->keyboard, IDB_KEYPAD_LIGHT,
		IDB_KEYPAD_DARK);
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"playstartwithrctrl", TRUE),
		"settings.kbd.ctrl-play");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"ft2home", TRUE),
		"settings.kbd.ft2-home");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"ft2delete", TRUE),
		"settings.kbd.ft2-delete");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"effcursoralwayssdown", FALSE),
		"settings.kbd.cursoralwayssdown");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"recordtweaksastws", 0),
		"settings.kbd.record-tws");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"advancelineonrecordtweak", 0),
		"settings.kbd.advance-line-on-record");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard,
			"movecursoronestep", 0),
		"settings.kbd.force-patstep1");
	choice = psy_property_set_text(
		psy_property_append_choice(self->keyboard,
			"pgupdowntype", 0),
		"settings.kbd.pgupdowntype");
	psy_property_set_text(
		psy_property_append_int(choice, "beat",
			0, 0, 0),
		"settings.kbd.pgupdowntype-one-beat");
	psy_property_set_text(
		psy_property_append_int(choice, "bar",
			0, 0, 0),
		"settings.kbd.pgupdowntype-one-bar");
	psy_property_set_text(
		psy_property_append_int(choice, "lines",
			0, 0, 0),
		"settings.kbd.pgupdowntype-lines");
	psy_property_set_text(
		psy_property_append_int(self->keyboard, "pgupdownstep",
			4, 0, 32),
		"settings.kbd.pgupdowntype-step-lines");
	self->keyboard_misc = psy_property_set_text(
		psy_property_append_section(self->keyboard, "misc"),
		"settings.kbd.misc");
	psy_property_set_text(
		psy_property_append_bool(self->keyboard_misc,
			"savereminder", TRUE),
		"settings.kbd.savereminder");
	psy_property_connect(psy_property_set_id(psy_property_set_text(
		psy_property_append_int(self->keyboard_misc,
			"numdefaultlines", 64, 1, 1024),
		"settings.kbd.numdefaultlines"),
		PROPERTY_ID_DEFAULTLINES),
		self, keyboardmiscconfig_on_pattern_default_lines);
	psy_property_set_text(
		psy_property_append_bool(self->keyboard_misc,
			"allowmultiinstances", FALSE),
		"settings.kbd.allowmultiinstances");
	psy_property_connect(psy_property_set_text(
		psy_property_append_bool(self->keyboard_misc,
			"followsong", self->follow_song),
		"settings.kbd.followsong"),
		self, keyboardmiscconfig_on_follow_song);
#if PSYCLE_USE_TK != PSYCLE_TK_X11
	psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(self->keyboard_misc,
			"ft2fileexplorer", FALSE),
		"settings.kbd.ft2-explorer"),
		PROPERTY_ID_FT2FILEEXPLORER);
#endif	
}

bool keyboardmiscconfig_ft2home(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "ft2home", TRUE);
}

void keyboardmiscconfig_enable_ft2home(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, psy_property_set_bool(
		self->keyboard, "ft2home", TRUE));
}

void keyboardmiscconfig_disable_ft2home(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, psy_property_set_bool(
		self->keyboard, "ft2home", FALSE));
}

bool keyboardmiscconfig_ft2delete(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "ft2delete", TRUE);
}

void keyboardmiscconfig_enable_ft2delete(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, psy_property_set_bool(
		self->keyboard, "ft2delete", TRUE));
}

void keyboardmiscconfig_disable_ft2delete(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, psy_property_set_bool(
		self->keyboard, "ft2delete", FALSE));
}

bool keyboardmiscconfig_effcursoralwaysdown(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "effcursoralwayssdown", FALSE);
}

bool keyboardmiscconfig_playstartwithrctrl(KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "playstartwithrctrl", TRUE);
}

void keyboardmiscconfig_enable_playstartwithrctrl(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, psy_property_set_bool(
		self->keyboard, "playstartwithrctrl", TRUE));
}

void keyboardmiscconfig_disable_playstartwithrctrl(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, psy_property_set_bool(
		self->keyboard, "playstartwithrctrl", FALSE));
}

bool keyboardmiscconfig_movecursoronestep(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "movecursoronestep", TRUE);
}

bool keyboardmiscconfig_savereminder(const KeyboardMiscConfig* self)
{
	assert(self);

	return  psy_property_at_bool(self->keyboard_misc, "savereminder", TRUE);
}

intptr_t keyboardmiscconfig_patdefaultlines(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_int(self->keyboard_misc, "numdefaultlines", 64);
}

bool keyboardmiscconfig_allowmultipleinstances(const KeyboardMiscConfig* self)
{
	return psy_property_at_bool(self->keyboard_misc, "allowmultiinstances",
		FALSE);
}

bool keyboardmiscconfig_recordtweaksastws(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard,
		"recordtweaksastws", 0);
}

bool keyboardmiscconfig_advancelineonrecordtweak(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard,
		"advancelineonrecordtweak", 0);
}

uintptr_t keyboardmiscconfig_pgupdowntype(const KeyboardMiscConfig* self)
{
	psy_Property* p;

	assert(self);

	p = psy_property_at(self->keyboard, "pgupdowntype", PSY_PROPERTY_TYPE_CHOICE);
	if (p) {
		return (uintptr_t)psy_property_item_int(p);
	}
	return 0;
}

intptr_t keyboardmiscconfig_pgupdownstep(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_int(self->keyboard, "pgupdownstep", 4);
}

void keyboardmiscconfig_setcursorstep(KeyboardMiscConfig* self, int step)
{
	assert(self);

	self->cursorstep = step;
}

int keyboardmiscconfig_cursor_step(const KeyboardMiscConfig* self)
{
	assert(self);

	return self->cursorstep;
}


bool keyboardmiscconfig_ft2fileexplorer(const KeyboardMiscConfig* self)
{
	assert(self);
#if PSYCLE_USE_TK == PSYCLE_TK_X11
	return TRUE;
#else
	return psy_property_at_bool(self->keyboard_misc, "ft2fileexplorer", FALSE);
#endif
}

bool keyboardmiscconfig_following_song(const KeyboardMiscConfig* self)
{
	assert(self);

	return self->follow_song;
}

void keyboardmiscconfig_on_follow_song(KeyboardMiscConfig* self,
	psy_Property* sender)
{
	keyboardmiscconfig_follow_song(self);
}

void keyboardmiscconfig_follow_song(KeyboardMiscConfig* self)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self->keyboard_misc, "followsong",
		PSY_PROPERTY_TYPE_NONE);
	if (property) {
		psy_property_set_item_bool(property, TRUE);
		self->follow_song = TRUE;
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}

void keyboardmiscconfig_stop_follow_song(KeyboardMiscConfig* self)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self->keyboard_misc, "followsong",
		PSY_PROPERTY_TYPE_NONE);
	if (property) {
		psy_property_set_item_bool(property, FALSE);
		self->follow_song = FALSE;
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
}

void keyboardmiscconfig_on_pattern_default_lines(KeyboardMiscConfig* self,
	psy_Property* sender)
{
	if (psy_property_item_int(sender) > 0) {
		psy_audio_pattern_setdefaultlines((uintptr_t)
			psy_property_item_int(sender));
	}	
	psy_signal_emit(&self->signal_changed, self, 1, sender);	
}

bool keyboardmiscconfig_connect(KeyboardMiscConfig* self, const char* key, void* context,
	void* fp)
{
	psy_Property* p;

	assert(self);

	p = keyboardmiscconfig_property(self, key);
	if (p) {
		psy_property_connect(p, context, fp);
		return TRUE;
	}
	return FALSE;
}

psy_Property* keyboardmiscconfig_property(KeyboardMiscConfig* self, const char* key)
{
	assert(self);

	return psy_property_at(self->keyboard_misc, key, PSY_PROPERTY_TYPE_NONE);
}
