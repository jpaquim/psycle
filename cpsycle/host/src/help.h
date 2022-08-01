/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(HELP_H)
#define HELP_H

/* host */
#include "dirconfig.h"
/* ui */
#include <uilabel.h>
#include <uiscroller.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Help
**
** Shows the psycle help files inside the psycle doc directory.
** Configure the used files with PSYCLE_HELPFILES in psyconf.h.
*/

typedef struct Help {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Scroller scroller;
	psy_ui_Label text;
	psy_ui_TabBar tabbar;	
	psy_Table filenames;
	psy_ui_AlignType lastalign;
	/* references */
	DirConfig* dir_config;
	bool dos_to_utf8;
} Help;

void help_init(Help*, psy_ui_Component* parent, DirConfig*);

INLINE psy_ui_Component* help_base(Help* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* HELP_H */
