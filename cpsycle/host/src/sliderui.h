/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SLIDERUI_H)
#define SLIDERUI_H

/* host */
#include "paramtweak.h"
/* ui */
#include <uicomponent.h>

/*
** SliderUi
**
** Slider to display a MachineParameter (MPF_SLIDER)
** (see also slidergroupui)
*/

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

/* SliderUi */
typedef struct SliderUi {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	ParamTweak paramtweak;
	uintptr_t paramidx;
	/* references */
	struct ParamSkin* skin;	
	struct psy_audio_Machine* machine;	
	struct psy_audio_MachineParam* param;
} SliderUi;

void sliderui_init(SliderUi*, psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*, struct ParamSkin*);

SliderUi* sliderui_alloc(void);
SliderUi* sliderui_allocinit(psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*, struct ParamSkin*);

INLINE psy_ui_Component* sliderui_base(SliderUi* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SLIDERUI_H */
