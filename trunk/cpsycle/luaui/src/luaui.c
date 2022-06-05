/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "luaui.h"
/* local */
#include "luacomponent.h"
#include "luagraphics.h"
#include "luaopendialog.h"
#include "luapoint.h"
#include "luasavedialog.h"
/* script */
#include <psyclescript.h>

/* platform */
#include "../../detail/portable.h"

static int psy_luaui_exportcmodules(psy_LuaUi*);

void psy_luaui_init(psy_LuaUi* self, psy_PsycleScript* script)
{
	assert(self);	
	assert(script);

	self->script = script;
	psy_luaui_exportcmodules(self);
}

void psy_luaui_dispose(psy_LuaUi* self)
{
	assert(self);	
}

int psy_luaui_exportcmodules(psy_LuaUi* self)
{	
	psyclescript_require(self->script, "psycle.ui.point",
		psy_luaui_point_open);
	psyclescript_require(self->script, "psycle.ui.graphics",
		psy_luaui_graphics_open);
	psyclescript_require(self->script, "psycle.ui.fileopen",
		psy_luaui_opendialog_open);
	psyclescript_require(self->script, "psycle.ui.filesave",
		psy_luaui_savedialog_open);
	psyclescript_require(self->script, "psycle.ui.window",
		psy_luaui_component_open);
	return 1;
}
