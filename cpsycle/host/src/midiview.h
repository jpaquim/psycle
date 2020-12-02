// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MIDIVIEW_H)
#define MIDIVIEW_H

#include "labelpair.h"
#include "workspace.h"

#include <uibutton.h>
#include <uicheckbox.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_ui_Component component;	
	int* channelmap;
} MidiActiveChannelBox;

void midiactivechannelbox_init(MidiActiveChannelBox*,
	psy_ui_Component* parent, int* channelmap);

typedef struct {
	psy_ui_Component component;
	int* flags;
} MidiActiveClockBox;

void midiactiveclockbox_init(MidiActiveClockBox*,
	psy_ui_Component* parent, int* flags);

typedef struct {
	psy_ui_Component component;
	MidiActiveChannelBox channelmap;
	MidiActiveClockBox clock;
	Workspace* workspace;
} MidiFlagsView;

void midiflagsview_init(MidiFlagsView*, psy_ui_Component* parent, Workspace*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Value colx[4];
	Workspace* workspace;
} MidiChannelMappingView;

void midichannelmappingview_init(MidiChannelMappingView*, psy_ui_Component* parent,
	Workspace*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Component titlebar;
	psy_ui_Label title;
	psy_ui_Button configure;
	psy_ui_Button hide;
	psy_ui_Label coretitle;
	psy_ui_Component top;
	psy_ui_Component resources;
	psy_ui_Label resourcestitle;
	LabelPair resources_win;
	LabelPair resources_mem;
	LabelPair resources_swap;
	LabelPair resources_vmem;		
	psy_ui_Component performance;
	LabelPair audiothreads;
	LabelPair totaltime;
	LabelPair machines;
	LabelPair routing;
	psy_ui_CheckBox cpucheck;
	MidiFlagsView flags;
	psy_ui_Label flagtitle;
	psy_ui_Margin topmargin;
	psy_ui_Component topchannelmapping;
	psy_ui_Label channelmappingtitle;
	psy_ui_Button mapconfigure;
	psy_ui_Scroller scroller;
	MidiChannelMappingView channelmapping;
	Workspace* workspace;
	uintptr_t channelmapupdate;
	uint32_t lastchannelmap;
	uintptr_t lastflags;
	int channelstatcounter;
	int flagstatcounter;
} MidiMonitor;

void midimonitor_init(MidiMonitor*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* midimonitor_base(MidiMonitor* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
