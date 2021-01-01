// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(CLIPBOX_H)
#define CLIPBOX_H

// host
#include "workspace.h"
// ui
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

// ClipBox
//
// When the peak amplitude goes above 0 dB, the box on the right will turn
// red to indicate that digital clipping has occurred(and that the volume
// needs to be lowered).
//
// Structure:
//  psy_ui_ComponentImp
//          ^
//          |        <<send>>
//       ClipBox  -------------> psy_audio_Machine (signal_worked)
//                |
//                |  <<send>>
//                -------------> Workspace (signal_songchanged)

typedef struct {
	psy_ui_Colour on;
	psy_ui_Colour off;
	psy_ui_Colour borderon;
	psy_ui_Colour borderoff;
} ClipBoxSkin;

typedef struct ClipBox {
	// inherits
	psy_ui_Component component;
	// internal data
	ClipBoxSkin skin;
	bool isclipon;
	// references
	Workspace* workspace;
} ClipBox;

void clipbox_init(ClipBox*, psy_ui_Component* parent, Workspace* workspace);

INLINE bool clipbox_isclipon(ClipBox* self)
{
	return self->isclipon;
}

INLINE void clipbox_activate(ClipBox* self)
{
	self->isclipon = TRUE;
}

INLINE void clipbox_deactivate(ClipBox* self)
{
	self->isclipon = FALSE;
}

INLINE psy_ui_Component* clipbox_base(ClipBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CLIPBOX_H */
