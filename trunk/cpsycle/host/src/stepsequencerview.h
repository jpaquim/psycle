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
** The left StepBarButtons select the current pattern bar
**
** psy_ui_Component
**      ^  
**      |
** StepsequencerView <>-- StepSequencerState ----------------------------------
**                           <>---- Workspace (player, sequence)              |
**                           <>---- Pattern                                   |
**                           <>---- StepSequencerPosition (editposition)      | 
**                           <>---- StepSequencerPosition (playposition)      |
**                                                  psy_ui_Component          |
**                                                         ^                  | 
**                                                         |                  | 
**                   <>-- psy_ui_Component <>- StepsequencerBarSelect <>------|
**                                             <>   psy_ui_Component          |
**                                             |           ^                  |
**                                             |           |                  |
**                                             --- StepsequencerBarButton <>--|
**                     psy_ui_Component               psy_ui_Component        | 
**                            ^                            ^                  |
**                            |                            |                  | 
**                   <>-- StepsequencerBar <>- StepSequencerTile <>-----------|
*/

/* StepSequencerPosition */
typedef struct StepSequencerPosition {
	uintptr_t line;
	uintptr_t steprow;
	uintptr_t seqentryline;
} StepSequencerPosition;

void stepsequencerposition_init(StepSequencerPosition*);
StepSequencerPosition stepsequencerposition_make(uintptr_t line,
	uintptr_t seqentryline, uintptr_t numtiles);

/* StepSequencerState */
typedef struct StepSequencerState {
	StepSequencerPosition editposition;	
	StepSequencerPosition playposition;
	uintptr_t numtiles;
	uintptr_t barbuttonindex;	
	/* references */
	Workspace* workspace;
	psy_audio_Pattern* pattern;
} StepSequencerState;

void stepsequencerstate_init(StepSequencerState*, Workspace*);

bool stepsequencerstate_update_positions(StepSequencerState*);

/* StepSequencerBarButton */
typedef struct StepSequencerBarButton {
	/* inherits */
	psy_ui_Component component;
	uintptr_t index;
	/* references */
	StepSequencerState* state;
} StepSequencerBarButton;

void stepsequencerbarbutton_init(StepSequencerBarButton*, psy_ui_Component* parent,
	psy_ui_Component* view, StepSequencerState*);
StepSequencerBarButton* stepsequencerbarbutton_alloc(void);
StepSequencerBarButton* stepsequencerbarbutton_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	StepSequencerState* state);

void stepsequencerbarbutton_turnoff(StepSequencerBarButton*);
void stepsequencerbarbutton_play(StepSequencerBarButton*);
void stepsequencerbarbutton_resetplay(StepSequencerBarButton*);

INLINE psy_ui_Component* stepsequencerbarbutton_base(StepSequencerBarButton* self)
{
	return &self->component;
}

/* StepsequencerBarSelect */
typedef struct StepsequencerBarSelect {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_Table buttons;
	/* references */
	StepSequencerState* state;	
} StepsequencerBarSelect;

void stepsequencerbarselect_init(StepsequencerBarSelect*,	
	psy_ui_Component* parent, psy_ui_Component* view,
	StepSequencerState*);

/* StepSequencerTile */
typedef struct StepSequencerTile {
	/* inherits */
	psy_ui_Component component;	
} StepSequencerTile;

void stepsequencertile_init(StepSequencerTile*, psy_ui_Component* parent,
	psy_ui_Component* view);
StepSequencerTile* stepsequencertile_alloc(void);
StepSequencerTile* stepsequencertile_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view);

void stepsequencertile_turnoff(StepSequencerTile*);
void stepsequencertile_play(StepSequencerTile*);
void stepsequencertile_resetplay(StepSequencerTile*);

INLINE psy_ui_Component* stepsequencertile_base(StepSequencerTile* self)
{
	return &self->component;
}

/* StepsequencerBar */
typedef struct StepsequencerBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_Table tiles;	
	/* references */
	StepSequencerState* state;	
} StepsequencerBar;

void stepsequencerbar_init(StepsequencerBar*, psy_ui_Component* parent,
	psy_ui_Component* view, StepSequencerState*);

/* StepsequencerView */
typedef struct StepsequencerView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component tilerow;
	StepsequencerBar stepsequencerbar;
	StepsequencerBarSelect stepsequencerbarselect;	
	StepSequencerState state;	
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
