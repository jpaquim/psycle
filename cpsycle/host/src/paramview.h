/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PARAMVIEW_H)
#define PARAMVIEW_H

/* host */
#include "skincoord.h"
#include "workspace.h"
/* ui */
#include <uicomponent.h>
/* audio */
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

struct MachineFrame;

typedef struct ParamView {
   psy_ui_Component component;
   psy_audio_Machine* machine;   
   MachineParamConfig* config;
   int sizechanged;
   uintptr_t paramstrobe;
   psy_ui_FontInfo fontinfo; 
   ParamSkin* skin;
   psy_ui_Component** frameview;
} ParamView;

void paramview_init(ParamView*, psy_ui_Component* parent, psy_audio_Machine*,
	MachineParamConfig*, psy_ui_Component** frameview);

ParamView* paramview_alloc(void);
ParamView* paramview_allocinit(psy_ui_Component* parent, psy_audio_Machine*,
	MachineParamConfig*, psy_ui_Component** frameview);
void paramview_setzoom(ParamView*, double zoomrate);

#ifdef __cplusplus
}
#endif

#endif /* PARAMVIEW_H */
