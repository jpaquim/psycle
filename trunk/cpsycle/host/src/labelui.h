// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(LABELUI_H)
#define LABELUI_H

// host
#include "paramview.h"
// ui
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

// LabelUi
typedef struct LabelUi {
	// inherits
	psy_ui_Component component;
	// internal	
	// references
	ParamSkin* skin;	
	psy_ui_Component* view;
	psy_audio_MachineParam* param;
} LabelUi;

void labelui_init(LabelUi*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam*, ParamSkin*);

#ifdef __cplusplus
}
#endif

#endif /* LABELUI_H */
