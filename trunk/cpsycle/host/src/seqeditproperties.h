/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTPROPERTIES_H)
#define SEQEDITTPROPERTIES_H

/* host */
#include "intedit.h"
#include "labeledit.h"
#include "labelpair.h"
#include "seqeditorstate.h"
/* ui */
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqEditEntryProperties */
typedef struct SeqEditEntryProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	LabelPair offset;
	LabelPair length;
	LabelPair end;
	psy_audio_OrderIndex orderindex;
	/* references */
	SeqEditState* state;
} SeqEditEntryProperties;

void seqeditentryproperties_init(SeqEditEntryProperties*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditentryproperties_setorderindex(SeqEditEntryProperties*, psy_audio_OrderIndex);

/* SeqEditEmptyProperties */
typedef struct SeqEditEmptyProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label caption;	
	/* references */
	SeqEditState* state;	
} SeqEditEmptyProperties;


void seqeditemptyproperties_init(SeqEditEmptyProperties*, psy_ui_Component* parent,
	SeqEditState*);


/* SeqEditPatternProperties */
typedef struct SeqEditPatternProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label caption;
	SeqEditEntryProperties entry;
	/* references */
	SeqEditState* state;	
} SeqEditPatternProperties;


void seqeditpatternproperties_init(SeqEditPatternProperties*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditpatternproperties_setorderindex(SeqEditPatternProperties* self,
	psy_audio_OrderIndex orderindex);

/* SeqEditSampleProperties */
typedef struct SeqEditSampleProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label caption;
	SeqEditEntryProperties entry;
	/* references */
	SeqEditState* state;	
} SeqEditSampleProperties;

void seqeditsampleproperties_init(SeqEditSampleProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqeditsampleproperties_setorderindex(SeqEditSampleProperties* self,
	psy_audio_OrderIndex orderindex);

/* SeqEditMarkerProperties */
typedef struct SeqEditMarkerProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label caption;
	LabelEdit name;
	SeqEditEntryProperties entry;
	/* references */
	SeqEditState* state;	
} SeqEditMarkerProperties;

void seqeditmarkerproperties_init(SeqEditMarkerProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqeditmarkerproperties_setorderindex(SeqEditMarkerProperties*,
	psy_audio_OrderIndex);

/* SeqEditTimesigProperties */
typedef struct SeqEditTimesigProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label caption;
	IntEdit nominator;
	IntEdit denominator;
	LabelPair offset;
	uintptr_t timesigindex;
	/* references */
	SeqEditState* state;	
} SeqEditTimesigProperties;

void seqedittimesigproperties_init(SeqEditTimesigProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqedittimesigproperties_settimesigindex(SeqEditTimesigProperties*,
	uintptr_t timesigindex);

/* SeqEditLoopProperties */
typedef struct SeqEditLoopProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label caption;
	IntEdit numloops;
	LabelPair offset;
	LabelPair length;
	LabelPair end;
	uintptr_t loopindex;	
	/* references */
	SeqEditState* state;	
} SeqEditLoopProperties;

void seqeditloopproperties_init(SeqEditLoopProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqeditloopproperties_setloopindex(SeqEditLoopProperties*,
	uintptr_t loopindex);

/* SeqEditProperties */
typedef struct SeqEditProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_ui_Notebook notebook;
	SeqEditEmptyProperties empty;
	SeqEditPatternProperties pattern;
	SeqEditSampleProperties sample;
	SeqEditMarkerProperties marker;
	SeqEditTimesigProperties timesig;
	SeqEditLoopProperties loop;
	SeqEditItemType itemtype;
	uintptr_t param1;
	uintptr_t param2;
	/* references */
	SeqEditState* state;	
} SeqEditProperties;

void seqeditproperties_init(SeqEditProperties*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditproperties_selectitem(SeqEditProperties*, SeqEditItemType,
	uintptr_t param1, uintptr_t param2);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTPROPERTIES_H */
