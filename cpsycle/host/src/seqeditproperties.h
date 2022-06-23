/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
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

/* SeqEditPropertiesEntry */

typedef struct SeqEditPropertiesEntry {
	/* inherits */
	psy_ui_Component component;	
	/* references */
	SeqEditState* state;
} SeqEditPropertiesEntry;

void seqeditpropertiesentry_init(SeqEditPropertiesEntry*, psy_ui_Component* parent,
	const char* title, SeqEditState*);

INLINE psy_ui_Component* seqeditpropertiesentry_base(SeqEditPropertiesEntry* self)
{
	return &self->component;
}

/* SeqEditPropertiesSeqEntry */
typedef struct SeqEditPropertiesSeqEntry {
	/* inherits */
	SeqEditPropertiesEntry entry;
	/* internal */	
	LabelPair offset;
	LabelPair length;
	LabelPair end;
	psy_audio_OrderIndex orderindex;		
} SeqEditPropertiesSeqEntry;

void seqeditpropertiesseqentry_init(SeqEditPropertiesSeqEntry*, psy_ui_Component* parent,
	const char* title, SeqEditState*);

void seqeditpropertiesseqentry_set_order_index(SeqEditPropertiesSeqEntry*, psy_audio_OrderIndex);

INLINE psy_ui_Component* seqeditpropertiesseqentry_base(SeqEditPropertiesSeqEntry* self)
{
	return &self->entry.component;
}

/* SeqEditEmptyProperties */
typedef struct SeqEditEmptyProperties {
	/* inherits */
	SeqEditPropertiesEntry entry;
} SeqEditEmptyProperties;

void seqeditemptyproperties_init(SeqEditEmptyProperties*, psy_ui_Component* parent,
	SeqEditState*);


/* SeqEditPatternProperties */
typedef struct SeqEditPatternProperties {
	/* inherits */
	SeqEditPropertiesSeqEntry entry;
} SeqEditPatternProperties;


void seqeditpatternproperties_init(SeqEditPatternProperties*, psy_ui_Component* parent,
	SeqEditState*);

INLINE psy_ui_Component* seqeditpatternproperties_base(SeqEditPatternProperties* self)
{
	return &self->entry.entry.component;
}

/* SeqEditSampleProperties */
typedef struct SeqEditSampleProperties {
	/* inherits */
	SeqEditPropertiesSeqEntry entry;		
} SeqEditSampleProperties;

void seqeditsampleproperties_init(SeqEditSampleProperties*,
	psy_ui_Component* parent, SeqEditState*);

INLINE psy_ui_Component* seqeditsampleproperties_base(SeqEditSampleProperties* self)
{
	return &self->entry.entry.component;
}

/* SeqEditMarkerProperties */
typedef struct SeqEditMarkerProperties {
	/* inherits */
	SeqEditPropertiesSeqEntry entry;
	/* internal */	
	LabelEdit name;	
} SeqEditMarkerProperties;

void seqeditmarkerproperties_init(SeqEditMarkerProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqeditmarkerproperties_set_order_index(SeqEditMarkerProperties*,
	psy_audio_OrderIndex);

/* SeqEditTimesigProperties */
typedef struct SeqEditTimesigProperties {
	/* inherits */
	SeqEditPropertiesEntry entry;
	/* internal */	
	IntEdit nominator;
	IntEdit denominator;
	LabelPair offset;
	uintptr_t timesigindex;	
} SeqEditTimesigProperties;

void seqedittimesigproperties_init(SeqEditTimesigProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqedittimesigproperties_set_timesig_index(SeqEditTimesigProperties*,
	uintptr_t timesigindex);

INLINE psy_ui_Component* seqedittimesigproperties_base(SeqEditTimesigProperties* self)
{
	return &self->entry.component;
}

/* SeqEditLoopProperties */
typedef struct SeqEditLoopProperties {
	/* inherits */
	SeqEditPropertiesEntry entry;
	/* internal */	
	IntEdit numloops;
	LabelPair offset;
	LabelPair length;
	LabelPair end;
	uintptr_t loopindex;		
} SeqEditLoopProperties;

void seqeditloopproperties_init(SeqEditLoopProperties*,
	psy_ui_Component* parent, SeqEditState*);

void seqeditloopproperties_setloopindex(SeqEditLoopProperties*,
	uintptr_t loopindex);

INLINE psy_ui_Component* seqeditloopproperties_base(SeqEditLoopProperties* self)
{
	return &self->entry.component;
}

/* SeqEditProperties */
typedef struct SeqEditProperties {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_ui_Label caption;
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

void seqeditproperties_select(SeqEditProperties*, SeqEditItemType,
	uintptr_t param1, uintptr_t param2);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITTPROPERTIES_H */
