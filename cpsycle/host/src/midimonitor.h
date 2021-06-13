// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MIDIMONITOR_H)
#define MIDIMONITOR_H

// host
#include "labelpair.h"
#include "titlebar.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MidiActiveChannelBox {
	// inherits
	psy_ui_Component component;
	// references
	uint32_t* channelmap;
} MidiActiveChannelBox;

void midiactivechannelbox_init(MidiActiveChannelBox*,
	psy_ui_Component* parent, uint32_t* channelmap);

INLINE psy_ui_Component* midiactivechannelbox_base(MidiActiveChannelBox* self)
{
	return &self->component;
}

typedef struct MidiActiveClockBox {
	// inherits
	psy_ui_Component component;
	// references
	uint32_t* flags;
} MidiActiveClockBox;

void midiactiveclockbox_init(MidiActiveClockBox*,
	psy_ui_Component* parent, uint32_t* flags);

INLINE psy_ui_Component* midiactiveclockbox_base(MidiActiveClockBox* self)
{
	return &self->component;
}

typedef struct MidiFlagsView {
	// inherits
	psy_ui_Component component;
	// ui elements
	MidiActiveChannelBox channelmap;
	MidiActiveClockBox clock;
	// references
	Workspace* workspace;
} MidiFlagsView;

void midiflagsview_init(MidiFlagsView*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* midiflagsview_base(MidiFlagsView* self)
{
	return &self->component;
}

typedef struct MidiChannelMappingView {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_ui_Value colx[4];
	// references
	Workspace* workspace;
} MidiChannelMappingView;

void midichannelmappingview_init(MidiChannelMappingView*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* midichannelmappingview_base(MidiChannelMappingView* self)
{
	return &self->component;
}

typedef struct MidiMonitor {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component client;
	TitleBar titlebar;	
	psy_ui_Button configure;	
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
	psy_ui_Component topchannelmapping;
	psy_ui_Label channelmappingtitle;
	psy_ui_Button mapconfigure;
	psy_ui_Scroller scroller;
	MidiChannelMappingView channelmapping;	
	// internal data
	psy_ui_Margin topmargin;
	uintptr_t channelmapupdate;
	uint32_t lastchannelmap;
	uintptr_t lastflags;
	int channelstatcounter;
	int flagstatcounter;
	// references
	Workspace* workspace;
} MidiMonitor;

void midimonitor_init(MidiMonitor*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* midimonitor_base(MidiMonitor* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MIDIMONITOR_H */
