// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


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

typedef struct {
    psy_ui_Colour fonttopcolour;
    psy_ui_Colour fontbottomcolour;
    psy_ui_Colour fonthtopcolour;
    psy_ui_Colour fonthbottomcolour;
    psy_ui_Colour fonttitlecolour;
    psy_ui_Colour topcolour;
    psy_ui_Colour bottomcolour;
    psy_ui_Colour htopcolour;
    psy_ui_Colour hbottomcolour;
    psy_ui_Colour titlecolour;
    psy_ui_Bitmap knobbitmap;
    psy_ui_Bitmap mixerbitmap;
    char* bitmappath;
    SkinCoord slider;
    SkinCoord knob;
    SkinCoord sliderknob;
    SkinCoord vuoff;
    SkinCoord vuon;
    SkinCoord switchon;
    SkinCoord switchoff;
    SkinCoord checkon;
    SkinCoord checkoff;    
} ParamSkin;

typedef struct ParamView {
   psy_ui_Component component;
   psy_audio_Machine* machine;
   int cx;
   int cy;      
   uintptr_t tweak;
   int tweakbase;   
   float tweakval;   
   Workspace* workspace;
   ParamSkin* skin;
   psy_Table positions;
   psy_ui_Size cpmax;
   uintptr_t numparams;
   int sizechanged;
   psy_ui_FontInfo fontinfo;
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
