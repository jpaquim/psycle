/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PRESETSVIEW_H)
#define PRESETSVIEW_H

/* host */
#include "titlebar.h"
#include "workspace.h"
/* ui */
#include <uicombobox.h>

#ifdef __cplusplus
extern "C" {
#endif

struct MachineFrame;

typedef struct PresetsView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	TitleBar title_bar;
	psy_ui_Component client;
	psy_ui_ComboBox presets;	
	psy_ui_Component buttons;
	psy_ui_Button importpresets;
	psy_ui_Button exportpresets;
	psy_ui_Button savepresets;
} PresetsView;

void presetsview_init(PresetsView*, psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* PRESETSVIEW_H */
