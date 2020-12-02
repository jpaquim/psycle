// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(LABELPAIR_H)
#define LABELPAIR_H

#include <uicomponent.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: Combines two labels to display a label pair

typedef struct {
	psy_ui_Component component;
	psy_ui_Label desc;
	psy_ui_Label value;
} LabelPair;

void labelpair_init(LabelPair*, psy_ui_Component* parent, const char* desc);

#ifdef __cplusplus
}
#endif

#endif
