/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PARAMROLL_H)
#define PARAMROLL_H

/* host */
#include "patternhostcmds.h"
#include "pianogridstate.h"
#include "workspace.h"
/* ui */
#include <uiscrollbar.h>


#ifdef __cplusplus
extern "C" {
#endif

/*
** ParamRoll
*/

/* ParamRuler */

typedef struct ParamRuler {
	/* inherits */
	psy_ui_Component component;	
} ParamRuler;

void paramruler_init(ParamRuler*, psy_ui_Component* parent);


/* ParamDraw */
typedef struct ParamDraw {
	/* inherits */
	psy_ui_Component component;
	bool tweaking;
	psy_audio_SequenceCursor tweak_cursor;
	/* references */
	PianoGridState* state;
	Workspace* workspace;
	psy_audio_PatternNode* tweak_node;
	psy_audio_Pattern* tweak_pattern;
} ParamDraw;

void paramdraw_init(ParamDraw*, psy_ui_Component* parent,
	PianoGridState*, Workspace*);


/* ParamRoll */
typedef struct ParamRoll {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_ui_Component left;
	ParamRuler ruler;	
	psy_ui_Component pane;
	ParamDraw draw;
	psy_ui_ScrollBar hscroll;	
} ParamRoll;

void paramroll_init(ParamRoll*, psy_ui_Component* parent,
	PianoGridState*, Workspace*);

INLINE psy_ui_Component* paramroll_base(ParamRoll* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PARAMROLL_H */
