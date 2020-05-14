// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(ZOOMBOX_H)
#define ZOOMBOX_H

#include <uibutton.h>
#include <uilabel.h>

typedef struct ZoomBox {
	psy_ui_Component component;
	psy_ui_Button zoomin;
	psy_ui_Label label;
	psy_ui_Button zoomout;
	psy_Signal signal_changed;
	double zoomrate;
	double zoomstep;
} ZoomBox;

void zoombox_init(ZoomBox*, psy_ui_Component* parent);
INLINE double zoombox_rate(ZoomBox* self) { return self->zoomrate; }
void zoombox_setrate(ZoomBox*, double);

#endif
