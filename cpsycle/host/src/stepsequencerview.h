/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(STEPSEQUENCERVIEW_H)
#define STEPSEQUENCERVIW_H

/* host */
#include "workspace.h"
/* audio */
#include <pattern.h>
/* ui */
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** StepsequencerView
**
** A classical stepsequencer view for entering notes in the patterneditor
** The Left StepBar component selects the current pattern bar.
*/

/* StepSequencerPosition */
typedef struct StepSequencerPosition {
	intptr_t line;
	intptr_t steprow;
} StepSequencerPosition;

/* StepTimer */
typedef struct StepTimer {
	StepSequencerPosition position;
	psy_audio_Player* player;	
	psy_dsp_big_beat_t sequenceentryoffset;
	psy_Signal signal_linetick;
	int doseqtick;
} StepTimer;

void steptimer_init(StepTimer*, psy_audio_Player*);
void steptimer_dispose(StepTimer*);
void steptimer_reset(StepTimer*, psy_dsp_big_beat_t entryoffset);
void steptimer_tick(StepTimer*);
StepSequencerPosition steptimer_position(StepTimer*);

void stepsequencerposition_init(StepSequencerPosition*);

/* StepsequencerBarSelect */
typedef struct StepsequencerBarSelect {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;
	/* internal */	
	int lineheight;
	int colwidth;	
	StepSequencerPosition position;
	StepTimer* steptimer;	
	/* references */
	psy_audio_Pattern* pattern;
	Workspace* workspace;
} StepsequencerBarSelect;

void stepsequencerbarselect_init(StepsequencerBarSelect*,	
	psy_ui_Component* parent,
	StepTimer* steptimer,
	Workspace* workspace);
void stepsequencerbarselect_setpattern(StepsequencerBarSelect*,
	psy_audio_Pattern*);

/* StepSequencerTile */
typedef struct StepSequencerTile {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	bool on;
	bool play;
} StepSequencerTile;

void stepsequencertile_init(StepSequencerTile*, psy_ui_Component* parent,
	psy_ui_Component* view);
StepSequencerTile* stepsequencertile_alloc(void);
StepSequencerTile* stepsequencertile_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view);

void stepsequencertile_turnon(StepSequencerTile*);
void stepsequencertile_turnoff(StepSequencerTile*);
void stepsequencertile_play(StepSequencerTile*);
void stepsequencertile_resetplay(StepSequencerTile*);

bool stepsequencertile_ison(StepSequencerTile*);
bool stepsequencertile_isplaying(StepSequencerTile*);

/* StepsequencerBar */
typedef struct StepsequencerBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_Table tiles;
	StepTimer* steptimer;
	StepSequencerPosition position;
	uintptr_t currplaystep;
	uintptr_t laststate;
	/* references */
	psy_audio_Pattern* pattern;
	Workspace* workspace;
} StepsequencerBar;

void stepsequencerbar_init(StepsequencerBar*,
	psy_ui_Component* parent,
	StepTimer* steptimer,
	Workspace*);
void stepsequencerbar_setpattern(StepsequencerBar*, psy_audio_Pattern*);

/* StepsequencerView */
typedef struct StepsequencerView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	StepsequencerBar stepsequencerbar;
	StepsequencerBarSelect stepsequencerbarselect;			
	StepTimer steptimer;
	/* references */
	Workspace* workspace;
} StepsequencerView;

void stepsequencerview_init(StepsequencerView*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* stepsequencerview_base(StepsequencerView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* STEPSEQUENCERVIEW_H */
