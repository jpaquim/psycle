// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwincompdetail.h"
#include "uicomponent.h"

psy_ui_win_ComponentDetails* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentDetails*) self->platform;
}