// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(PARAMVIEWS_H)
#define PARAMVIEWS_H

// host
#include "machineframe.h"
#include "workspace.h"
// container
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ParamViews is the container of all Machine Frames(Native / Vst / Internal) */

typedef struct ParamViews {
	psy_Table frames;	
	/* references */
	psy_ui_Component* view;
	Workspace* workspace;
} ParamViews;

void paramviews_init(ParamViews*, psy_ui_Component* view, Workspace*);
void paramviews_dispose(ParamViews*);

void paramviews_show(ParamViews*, uintptr_t macid);
MachineFrame* paramviews_frame(ParamViews*, uintptr_t macid);
void paramviews_erase(ParamViews*, uintptr_t macid);
void paramviews_remove(ParamViews*, uintptr_t macid);
void paramviews_removeall(ParamViews*);

#ifdef __cplusplus
}
#endif

#endif /* PARAMVIEWS_H */
