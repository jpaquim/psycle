// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "interpolatecurveview.h"
#include "../../detail/portable.h"
#include <math.h>

void interpolatecurvebox_init(InterpolateCurveBox* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
}

void interpolatecurveview_init(InterpolateCurveView* self, psy_ui_Component* parent,
	int startsel, int endsel, int lpb, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
}
