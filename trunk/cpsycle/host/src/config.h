// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CONFIG)
#define CONFIG

#ifdef __cplusplus
extern "C" {
#endif

#include "audioconfig.h"
#include "eventdriverconfig.h"
#include "generalconfig.h"
#include "keyboardmiscconfig.h"
#include "languageconfig.h"
#include "dirconfig.h"
#include "machineviewconfig.h"
#include "machineparamconfig.h"
#include "midiviewconfig.h"
#include "patternviewconfig.h"
#include "compatconfig.h"

enum {
	PROPERTY_ID_REGENERATEPLUGINCACHE = 1,
	PROPERTY_ID_LOADSKIN,
	PROPERTY_ID_DEFAULTSKIN,	
	PROPERTY_ID_DEFAULTFONT	
};

typedef struct PsycleConfig {
	psy_Property config;
	psy_Property* global;
	GeneralConfig general;
	AudioConfig audio;
	EventDriverConfig input;
	LanguageConfig language;
	DirConfig directories;
	psy_Property* visual;
	PatternViewConfig patview;
	MachineViewConfig macview;
	MachineParamConfig macparam;
	KeyboardMiscConfig misc;
	MidiViewConfig midi;
	CompatConfig compat;
	char* dialbitmappath;
} PsycleConfig;

void psycleconfig_init(PsycleConfig*, psy_audio_Player*,
	psy_audio_MachineFactory*);
void psycleconfig_dispose(PsycleConfig*);

void psycleconfig_loadskin(PsycleConfig*, const char* path);
void psycleconfig_resetskin(PsycleConfig*);
void psycleconfig_setcontrolskinpath(PsycleConfig*, const char* path);

INLINE GeneralConfig* psycleconfig_general(PsycleConfig* self)
{
	return &self->general;
}

INLINE AudioConfig* psycleconfig_audio(PsycleConfig* self)
{	
	return &self->audio;
}

INLINE EventDriverConfig* psycleconfig_input(PsycleConfig* self)
{
	return &self->input;
}

INLINE LanguageConfig* psycleconfig_language(PsycleConfig* self)
{
	return &self->language;
}

INLINE DirConfig* psycleconfig_directories(PsycleConfig* self)
{
	return &self->directories;
}

INLINE PatternViewConfig* psycleconfig_patview(PsycleConfig* self)
{
	return &self->patview;
}

INLINE MachineViewConfig* psycleconfig_macview(PsycleConfig* self)
{
	return &self->macview;
}

INLINE MachineParamConfig* psycleconfig_macparam(PsycleConfig* self)
{
	return &self->macparam;
}

INLINE KeyboardMiscConfig* psycleconfig_misc(PsycleConfig* self)
{
	return &self->misc;
}

INLINE MidiViewConfig* psycleconfig_midi(PsycleConfig* self)
{
	return &self->midi;
}

INLINE CompatConfig* psycleconfig_compat(PsycleConfig* self)
{
	return &self->compat;
}

const char* psycleconfig_defaultfontstr(const PsycleConfig* self);
bool psycleconfig_enableaudio(const PsycleConfig* self);

#ifdef __cplusplus
}
#endif

#endif
