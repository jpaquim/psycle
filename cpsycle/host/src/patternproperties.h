/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNPROPERTIES)
#define PATTERNPROPERTIES

/* host */
#include "intedit.h"
#include "workspace.h"
/* audio */
#include <pattern.h>
/* ui */
#include <uilabel.h>
#include <uitextarea.h>
#include <uibutton.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PatternProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label namelabel;
	psy_ui_Label lengthlabel;
	psy_ui_TextArea nameedit;
	psy_ui_TextArea lengthedit;
	psy_ui_Button applybutton;
	IntEdit timesig_numerator;
	IntEdit timesig_denominator;
	/* references */
	psy_audio_Patterns* patterns;
	uintptr_t pattern_index;	
} PatternProperties;

void patternproperties_init(PatternProperties*, psy_ui_Component* parent);

void patternproperties_set_patterns(PatternProperties*, psy_audio_Patterns*);
void patternproperties_select(PatternProperties*, uintptr_t pattern_index);


#ifdef __cplusplus
}
#endif

#endif /* PATTERNPROPERTIES */
