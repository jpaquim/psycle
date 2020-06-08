// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEPSEQUENCERVIEW_H)
#define STEPSEQUENCERVIW_H

#include <uislider.h>
#include "workspace.h"

#include <pattern.h>

// aim: A classical stepsequencer view for entering notes in the patterneditor
//      The Left StepBar component selects the current pattern bar.

typedef struct {
	int line;
	int steprow;	
} StepSequencerPosition;

typedef struct {
	StepSequencerPosition position;
	psy_audio_Player* player;	
	psy_dsp_beat_t sequenceentryoffset;
	psy_Signal signal_linetick;
	int doseqtick;
}  StepTimer;

void steptimer_init(StepTimer*, psy_audio_Player*);
void steptimer_dispose(StepTimer*, psy_audio_Player*);
void steptimer_reset(StepTimer*, psy_dsp_beat_t entryoffset);
void steptimer_tick(StepTimer*);
StepSequencerPosition steptimer_position(StepTimer*);

void stepsequencerposition_init(StepSequencerPosition*);

typedef struct {
	psy_ui_Component component;
	Workspace* workspace;	
	int lineheight;
	int colwidth;
	psy_audio_Pattern* pattern;
	StepSequencerPosition position;
	StepTimer* steptimer;
	psy_Signal signal_selected;
} StepsequencerBarSelect;

void stepsequencerbarselect_init(StepsequencerBarSelect*,
	psy_ui_Component* parent,
	StepTimer* steptimer,
	Workspace* workspace);
void stepsequencerbarselect_setpattern(StepsequencerBarSelect*, psy_audio_Pattern*);

typedef struct {
	psy_ui_Component component;
	Workspace* workspace;
	psy_audio_Pattern* pattern;		
	int stepwidth;	
	int stepheight;
	StepTimer* steptimer;
	StepSequencerPosition position;	
} StepsequencerBar;

void stepsequencerbar_init(StepsequencerBar*,
	psy_ui_Component* parent,
	StepTimer* steptimer,
	Workspace*);
void stepsequencerbar_setpattern(StepsequencerBar*, psy_audio_Pattern*);


typedef struct StepsequencerView {
	psy_ui_Component component;
	StepsequencerBar stepsequencerbar;
	StepsequencerBarSelect stepsequencerbarselect;			
	StepTimer steptimer;
	Workspace* workspace;
} StepsequencerView;

void stepsequencerview_init(StepsequencerView*, psy_ui_Component* parent,
	Workspace*);

#endif
