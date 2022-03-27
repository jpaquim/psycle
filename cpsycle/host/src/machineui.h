/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEUI_H)
#define MACHINEUI_H

/* host */
#include "machineeditorview.h" // vst view
#include "machineframe.h"
#include "paramviews.h"
#include "workspace.h"
/* ui */
#include <uitextarea.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Components to draw a machine with its buttonsand vumeterand reacting to
** ui events to solo/mute/bypass or pan the machine
*/

typedef enum MachineViewDragMode {
	MACHINEVIEW_DRAG_NONE,
	MACHINEVIEW_DRAG_MACHINE,
	MACHINEVIEW_DRAG_NEWCONNECTION,
	MACHINEVIEW_DRAG_LEFTCONNECTION,
	MACHINEVIEW_DRAG_RIGHTCONNECTION	
} MachineViewDragMode;

typedef struct EditnameUi {
	/* inherits */
	psy_ui_Label label;
	/* internal */
	/* references */
	psy_audio_Machine* machine;
} EditnameUi;

void editnameui_init(EditnameUi*, psy_ui_Component* parent, psy_audio_Machine*,
	uintptr_t style);

typedef struct VuValues {
	/* output level for display(0 .. 1.f) */
	psy_dsp_amp_t volumedisplay;
	/* output peak level for display (0 .. 1.f) */
	psy_dsp_amp_t volumemaxdisplay;
	/* output peak level display time(refreshrate * 60) */
	int volumemaxcounterlife;
} VuValues;

void vuvalues_init(VuValues*);
void vuvalues_tickcounter(VuValues*);
bool vuvalues_update(VuValues*, psy_audio_Buffer*);

typedef struct VuDisplay {
	VuValues vuvalues;	
} VuDisplay;

void vudisplay_init(VuDisplay*);

bool vudisplay_update(VuDisplay*, psy_audio_Buffer*);
void vudisplay_draw(VuDisplay*, psy_ui_Graphics*,
	psy_ui_Style* vu, psy_ui_Style* vupeak);

typedef struct VuUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	uintptr_t vu0_style;
	uintptr_t vupeak_style;
	VuDisplay vu;
	/* references */
	psy_audio_Machine* machine;
} VuUi;

void vuui_init(VuUi*, psy_ui_Component* parent, psy_audio_Machine*,
	uintptr_t vu_style, uintptr_t vu0_style, uintptr_t vupeak_style);

bool vuui_update(VuUi*);

INLINE psy_ui_Component* vuui_base(VuUi* self)
{
	return &self->component;
}

typedef struct PanUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component slider;
	bool drag;
	double dragoffset;
	/* references */
	psy_audio_Machine* machine;
} PanUi;

void panui_init(PanUi*, psy_ui_Component* parent, psy_audio_Machine*,
	uintptr_t pan_style, uintptr_t pan_slider_style);



psy_ui_Component* machineui_create(psy_audio_Machine* machine,
	psy_ui_Component* parent, ParamViews* paramviews,
	bool machinepos, Workspace* workspace);

void machineui_drawhighlight(psy_ui_Graphics* g, psy_ui_RealRectangle position);

void machineui_enable_vumeter(void);
void machineui_prevent_vumeter(void);
bool machineui_vumeter_prevented(void);

void machineui_enable_macindex(void);
void machineui_prevent_macindex(void);
bool machineui_maxindex_prevented(void);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEUI_H */
