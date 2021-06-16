/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITORENTRY_H)
#define SEQEDITORENTRY_H

/* host */
#include "intedit.h"
#include "wavebox.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SeqEditState;

/* SeqEditEntry */
typedef struct SeqEditEntry {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_audio_OrderIndex seqpos;
	bool preventresize;
	/* references */
	struct SeqEditState* state;
	psy_audio_SequenceEntry* seqentry;
} SeqEditEntry;

void seqeditentry_init(SeqEditEntry*,
	psy_ui_Component* parent, psy_ui_Component* view, psy_audio_SequenceEntry*,
	psy_audio_OrderIndex seqpos, struct SeqEditState*);

void seqeditentry_startdrag(SeqEditEntry*, psy_ui_MouseEvent*);

INLINE psy_audio_OrderIndex seqeditentry_seqpos(const SeqEditEntry* self)
{
	return self->seqpos;
}

/* SeqEditPatternEntry */
typedef struct SeqEditPatternEntry {
	/* inherits */
	SeqEditEntry seqeditorentry;
	/* internal */	
	/* references */	
	psy_audio_SequencePatternEntry* sequenceentry;	
} SeqEditPatternEntry;

void seqeditpatternentry_init(SeqEditPatternEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry*, psy_audio_OrderIndex seqpos,
	struct SeqEditState*);

SeqEditPatternEntry* seqeditpatternentry_alloc(void);
SeqEditPatternEntry* seqeditpatternentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry, psy_audio_OrderIndex seqpos,
	struct SeqEditState*);

INLINE psy_ui_Component* seqeditpatternentry_base(SeqEditPatternEntry* self)
{
	assert(self);

	return &self->seqeditorentry.component;
}

/* SeqEditMarkerEntry */
typedef struct SeqEditMarkerEntry {
	/* inherits */
	SeqEditEntry seqeditorentry;
	/* internal */
	/* references */	
	psy_audio_SequenceMarkerEntry* sequenceentry;	
} SeqEditMarkerEntry;

void seqeditmarkerentry_init(SeqEditMarkerEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry*, psy_audio_OrderIndex seqpos,
	struct SeqEditState*);

SeqEditMarkerEntry* seqeditmarkerentry_alloc(void);
SeqEditMarkerEntry* seqeditmarkerentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry, psy_audio_OrderIndex seqpos,
	struct SeqEditState*);

INLINE psy_ui_Component* seqeditmarkerentry_base(SeqEditMarkerEntry* self)
{
	assert(self);

	return &self->seqeditorentry.component;
}

/* SeqEditSampleEntry */
typedef struct SeqEditSampleEntry {
	/* inherits */
	SeqEditEntry seqeditorentry;
	/* internal */
	WaveBox wavebox;
	psy_ui_Label label;
	/* references */		
	psy_audio_SequenceSampleEntry* sequenceentry;	
	bool preventedit;
} SeqEditSampleEntry;

void seqeditsampleentry_init(SeqEditSampleEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry*, psy_audio_OrderIndex seqpos,
	struct SeqEditState*);

SeqEditSampleEntry* seqeditsampleentry_alloc(void);
SeqEditSampleEntry* seqeditsampleentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry, psy_audio_OrderIndex seqpos,
	struct SeqEditState*);

void seqeditsampleentry_updatesample(SeqEditSampleEntry*);

INLINE psy_ui_Component* seqeditsampleentry_base(SeqEditSampleEntry* self)
{
	assert(self);

	return &self->seqeditorentry.component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITORENTRY_H */
