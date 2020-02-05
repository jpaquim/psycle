// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(INTERPOLATECURVEVIEW_H)
#define INTERPOLATECURVEVIEW_H

#include "workspace.h"

#include <uibutton.h>
#include <uicheckbox.h>
#include <uicombobox.h>
#include <uiedit.h>

typedef struct {
	psy_ui_Component component;
} InterpolateCurveBox;

void interpolatecurvebox_init(InterpolateCurveBox*, psy_ui_Component* parent,
	Workspace*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Button checktwk;
	psy_ui_ComboBox combotwk;
	psy_ui_Edit pos;
	psy_ui_Edit value;
	psy_ui_Edit min;
	psy_ui_Edit max;
	psy_ui_ComboBox curvetype;
	InterpolateCurveBox box;	
} InterpolateCurveView;

void interpolatecurveview_init(InterpolateCurveView*, psy_ui_Component* parent,
	int startsel, int endsel, int lpb, Workspace*);


#endif