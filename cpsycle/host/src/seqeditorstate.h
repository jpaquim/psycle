/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITORSTATE_H)
#define SEQEDITORSTATE_H

/* host */
#include "sequencehostcmds.h"
/* ui */
#include <uitextinput.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SeqEditItemType {
	SEQEDITITEM_NONE,
	SEQEDITITEM_PATTERN,
	SEQEDITITEM_SAMPLE,
	SEQEDITITEM_MARKER,
	SEQEDITITEM_TIMESIG,
	SEQEDITITEM_LOOP
} SeqEditItemType;

typedef enum {
	SEQEDIT_DRAGTYPE_UNDEFINED = 0,
	SEQEDIT_DRAGTYPE_MOVE      = 1,
	SEQEDIT_DRAGTYPE_REORDER   = 2
} SeqEditorDragType;

typedef enum {	
	SEQEDIT_DRAG_NONE    = 0,
	SEQEDIT_DRAG_START   = 1,
	SEQEDIT_DRAG_MOVE    = 2,
	SEQEDIT_DRAG_REORDER = 3,
	SEQEDIT_DRAG_LENGTH  = 4,
	SEQEDIT_DRAG_REMOVE  = 5
} SeqEditorDragStatus;

typedef enum {
	SEQEDTCMD_NONE = 0,
	SEQEDTCMD_NEWTRACK = 1,
	SEQEDTCMD_DELTRACK = 2	
} SeqEdtCmd;

typedef struct SeqEditState {
	/* signals */
	psy_Signal signal_cursorchanged;
	psy_Signal signal_itemselected;
	psy_Signal signal_timesigchanged;
	psy_Signal signal_loopchanged;	
	double pxperbeat;
	double defaultpxperbeat;
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;	
	psy_dsp_big_beat_t cursorposition;
	bool updatecursorposition;
	bool drawcursor;
	bool cursoractive;
	bool drawpatternevents;	
	bool showpatternnames;
	/* drag */
	SeqEditorDragType dragtype;
	SeqEditorDragStatus dragstatus;	
	psy_dsp_big_beat_t dragposition;
	psy_audio_OrderIndex dragseqpos;	
	SeqEdtCmd cmd;
	psy_audio_SequenceEntryType inserttype;	
	/* references */
	psy_audio_SequenceEntry* seqentry;
	Workspace* workspace;	
	SequenceCmds* cmds;
	psy_ui_TextInput* edit;
	psy_ui_Component* view;
} SeqEditState;

void seqeditstate_init(SeqEditState*, SequenceCmds*, psy_ui_TextInput*,
	psy_ui_Component* view);
void seqeditstate_dispose(SeqEditState*);

psy_audio_Sequence* seqeditstate_sequence(SeqEditState*);
const psy_audio_Sequence* seqeditstate_sequence_const(const SeqEditState*);
void seqeditstate_outputstatusposition(SeqEditState*);

INLINE psy_audio_OrderIndex seqeditstate_editposition(const SeqEditState* self)
{	
	if (self->workspace->song) {
		return self->workspace->song->sequence.cursor.orderindex;
	}
	return psy_audio_orderindex_make(0, 0);
}

INLINE double seqeditstate_beattopx(const SeqEditState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return floor(position * self->pxperbeat);
}

INLINE psy_ui_Value seqeditstate_preferredwidth(const SeqEditState* self)
{
	assert(self);

	if (seqeditstate_sequence_const(self)) {
		return psy_ui_value_make_px(seqeditstate_beattopx(self,
			psy_audio_sequence_duration(seqeditstate_sequence_const(self)) +
			400.0));
	}
	return psy_ui_value_make_px(seqeditstate_beattopx(self, 400.0));
}

INLINE psy_dsp_big_beat_t seqeditstate_pxtobeat(const
	SeqEditState* self, double px)
{
	assert(self);

	return px / (psy_dsp_big_beat_t)self->pxperbeat;
}

INLINE void seqeditstate_setcursor(SeqEditState* self,
	psy_dsp_big_beat_t cursorposition)
{
	assert(self);

	self->cursorposition = cursorposition;
	seqeditstate_outputstatusposition(self);
	psy_signal_emit(&self->signal_cursorchanged, self, 0);
}

psy_dsp_big_beat_t seqeditstate_quantize(const SeqEditState*,
	psy_dsp_big_beat_t position);

INLINE psy_ui_Value seqeditstate_lineheight(const SeqEditState* self)
{
	return self->lineheight;
}

psy_audio_PatternNode* seqeditstate_node(SeqEditState*, psy_ui_RealPoint,
	psy_dsp_big_beat_t d1, psy_dsp_big_beat_t d2, psy_audio_PatternNode** prev);
psy_audio_Pattern* seqeditstate_globalpattern(SeqEditState*);
void seqeditstate_edit(SeqEditState*, psy_ui_Component* parent,
	psy_ui_RealPoint cp, double width, const char* text);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITORSTATE_H */
