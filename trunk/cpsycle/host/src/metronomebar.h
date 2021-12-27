/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(METRONOMEBAR_H)
#define METRONOMEBAR_H

/* host */
#include "workspace.h"
/* ui */
#include "uibutton.h"
#include "uicombobox.h"
#include "uitextinput.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MetronomeBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button activated;
	psy_ui_Label desc;
	psy_ui_ComboBox precount;
	psy_ui_Button configure;
	/* references */
	psy_audio_Player* player;
	Workspace* workspace;
} MetronomeBar;

void metronomebar_init(MetronomeBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* metronomebar_base(MetronomeBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* METRONOMEBAR_H */
