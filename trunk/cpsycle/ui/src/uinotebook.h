// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UINOTEBOOK_H)
#define UINOTEBOOK_H

#include "uicomponent.h"

typedef struct {
   ui_component component;
   int pageindex;
} ui_notebook;

void ui_notebook_init(ui_notebook*, ui_component* parent);
void ui_notebook_setpage(ui_notebook*, int page);
int ui_notebook_page(ui_notebook*);
void ui_notebook_connectcontroller(ui_notebook*, Signal* controllersignal);

#endif