// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SLIDERGROUPUI_H)
#define SLIDERGROUPUI_H

// host
#include "labelui.h"
#include "levelui.h"
#include "sliderui.h"

#ifdef __cplusplus
extern "C" {
#endif

// SliderUi
typedef struct SliderGroupUi {
	// inherits
	psy_ui_Component component;
	psy_ui_Component controls;
	// internal
	SliderUi slider;
	LevelUi level;
	LabelUi label;		
} SliderGroupUi;

void slidergroupui_init(SliderGroupUi*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* volume,
	psy_audio_MachineParam* level, ParamSkin*);

SliderGroupUi* slidergroupui_alloc(void);
SliderGroupUi* slidergroupui_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* volume,
	psy_audio_MachineParam* level, ParamSkin*);

#ifdef __cplusplus
}
#endif

#endif /* SLIDERGROUPUI_H */
