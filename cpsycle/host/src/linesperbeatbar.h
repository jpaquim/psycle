/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(LINESPERBEATBAR_H)
#define LINESPERBEATBAR_H

/* host */
#include "workspace.h"
/* ui */
#include "uibutton.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** LinesPerBeatBar
**
** Sets the Lines per beat ranging from 1 to 31.
*/

typedef struct LinesPerBeatBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	/* description label */
	psy_ui_Label desc;
	/* displays the player lpb */
	psy_ui_Label number;
	/* decrement lpb button by -1 */
	psy_ui_Button less;
	/* increment lpb button by +1 */
	psy_ui_Button more;
	/* lpb display cache */
	uintptr_t lpb;
	/* references */
	psy_audio_Player* player;
} LinesPerBeatBar;

void linesperbeatbar_init(LinesPerBeatBar*, psy_ui_Component* parent,
	psy_audio_Player*);

INLINE psy_ui_Component* linesperbeatbar_base(LinesPerBeatBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* LINESPERBEATBAR_H */
