/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CONFIG_H)
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* host */
#include "compatconfig.h"
#include "dirconfig.h"
#include "eventdriverconfig.h"
#include "generalconfig.h"
#include "globalconfig.h"
#include "keyboardmiscconfig.h"
#include "metronomeconfig.h"
#include "midiviewconfig.h"
#include "predefsconfig.h"
#include "seqeditconfig.h"
#include "visualconfig.h"
/* audio */
#include <audioconfig.h>


typedef struct PsycleConfig {
	psy_Property config;
	GlobalConfig global;	
	GeneralConfig general;
	VisualConfig visual;
	AudioConfig audio;
	EventDriverConfig input;	
	DirConfig directories;	
	MetronomeConfig metronome;
	KeyboardMiscConfig misc;
	MidiViewConfig midi;
	CompatConfig compat;
	PredefsConfig predefs;
	SeqEditConfig seqedit;
} PsycleConfig;

void psycleconfig_init(PsycleConfig*, psy_audio_Player*,
	psy_audio_MachineFactory*);
void psycleconfig_dispose(PsycleConfig*);

void psycleconfig_loadskin(PsycleConfig*, const char* path);
void psycleconfig_reset_skin(PsycleConfig*);
void psycleconfig_reset_control_skin(PsycleConfig*);

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
	return &self->global.language;
}

INLINE DirConfig* psycleconfig_directories(PsycleConfig* self)
{
	return &self->directories;
}

INLINE PatternViewConfig* psycleconfig_patview(PsycleConfig* self)
{
	return &self->visual.patview;
}

INLINE MachineViewConfig* psycleconfig_macview(PsycleConfig* self)
{
	return &self->visual.macview;
}

INLINE MachineParamConfig* psycleconfig_macparam(PsycleConfig* self)
{
	return &self->visual.macparam;
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

INLINE PredefsConfig* psycleconfig_predefs(PsycleConfig* self)
{
	return &self->predefs;
}

const char* psycleconfig_defaultfontstr(const PsycleConfig* self);
bool psycleconfig_audio_enabled(const PsycleConfig*);
void psycleconfig_enable_audio(PsycleConfig*, bool on);

void psycleconfig_notifyall_changed(PsycleConfig*);
uintptr_t psycleconfig_notify_changed(PsycleConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
