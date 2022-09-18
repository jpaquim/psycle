/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(WIREFRAMES_H)
#define WIREFRAMES_H

/* host */
#include "machineframe.h"
#include "workspace.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MachineWirePane
**
** Displays and edits machines with their wires
*/


struct WireFrame;

typedef struct WireFrames {
	psy_List* wireframes;
	Workspace* workspace;
	psy_ui_Component* view;
} WireFrames;

void wireframes_init(WireFrames*, Workspace*, psy_ui_Component* view);
void wireframes_dispose(WireFrames*);
void wireframes_destroy(WireFrames*);
void wireframes_show(WireFrames*, psy_audio_Wire);
void wireframes_on_frame_destroyed(WireFrames*, psy_ui_Component* sender);
struct WireFrame* wireframes_frame(WireFrames*, psy_audio_Wire);

#ifdef __cplusplus
}
#endif

#endif /* WIREFRAMES_H */
