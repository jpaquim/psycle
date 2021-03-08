// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(PARAMVIEW)
#define PARAMVIEW

#include "workspace.h"
#include <uicomponent.h>
#include <machine.h>
#include <plugin_interface.h>
#include "skincoord.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ParamTweak {
    // internal data
    float tweakbase;
    float tweakval;
    // references
    psy_audio_Machine* machine;
    uintptr_t paramindex;
    ParamSkin skin;
    psy_audio_MachineParam* param;
} ParamTweak;

void paramtweak_init(ParamTweak*);

void paramtweak_begin(ParamTweak*, psy_audio_Machine*,
    uintptr_t paramindex);
void paramtweak_end(ParamTweak*);
void paramtweak_onmousedown(ParamTweak*, psy_ui_MouseEvent*);
void paramtweak_onmousemove(ParamTweak*, psy_ui_MouseEvent*);


typedef struct KnobDraw {
    ParamSkin* skin;
    psy_audio_MachineParam* param;
    psy_ui_RealSize size;
    psy_audio_Machine* machine;
    const psy_ui_TextMetric* tm;
    bool tweaking;
    psy_ui_RealRectangle r_top;
    psy_ui_RealRectangle r_bottom;    
} KnobDraw;

void knobdraw_init(KnobDraw*, ParamSkin*, psy_audio_Machine*,
    psy_audio_MachineParam*, psy_ui_RealSize, const psy_ui_TextMetric*,
    bool tweaking);

typedef struct SliderDraw {
    ParamSkin* skin;
    psy_audio_MachineParam* param;
    psy_ui_RealSize size;
    psy_audio_Machine* machine;
    const psy_ui_TextMetric* tm;
    bool tweaking;
    psy_ui_RealRectangle r_top;
    psy_ui_RealRectangle r_bottom;
    bool drawlabel;
} SliderDraw;

void sliderdraw_init(SliderDraw*, ParamSkin*, psy_audio_Machine*,
    psy_audio_MachineParam*, psy_ui_RealSize, const psy_ui_TextMetric*,
    bool tweaking, bool drawlabel);
void sliderdraw_draw(SliderDraw*, psy_ui_Graphics*);

typedef struct {
    psy_ui_Component component;    
    ParamTweak paramtweak;
    ParamSkin* skin;
    psy_audio_Machine* machine;
    uintptr_t paramindex;    
} ParamKnob;

void paramknob_init(ParamKnob*, psy_ui_Component* parent, psy_audio_Machine*,
    uintptr_t paramindex, Workspace*);

typedef struct ParamView {
   psy_ui_Component component;
   psy_audio_Machine* machine;
   intptr_t cx;
   intptr_t cy;
   uintptr_t tweak;
   uintptr_t lasttweak;
   float tweakbase;
   float tweakval;   
   Workspace* workspace;
   ParamSkin* skin;
   psy_Table positions;
   psy_ui_Size cpmax;
   uintptr_t numparams;
   int sizechanged;   
   psy_ui_FontInfo fontinfo;
   ParamTweak paramtweak;
} ParamView;

void paramview_init(ParamView*, psy_ui_Component* parent, psy_audio_Machine*,
	Workspace*);
ParamView* paramview_alloc(void);
ParamView* paramview_allocinit(psy_ui_Component* parent, psy_audio_Machine*,
	Workspace*);
void paramview_changecontrolskin(const char* path);
void paramview_setzoom(ParamView*, double zoomrate);

#ifdef __cplusplus
}
#endif

#endif
