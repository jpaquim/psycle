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


struct PluginScanThread;

typedef struct PsycleConfig {	
	/* internal */
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
	struct PluginScanThread*, psy_audio_MachineFactory*);
void psycleconfig_dispose(PsycleConfig*);

void psycleconfig_notify_all(PsycleConfig*);
void psycleconfig_load_driver_configurations(PsycleConfig*);
void psycleconfig_save_configuration(PsycleConfig*);

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

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
