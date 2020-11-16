// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

#include "workspace.h"

#include <uiscroller.h>

typedef struct SeqEditorTrackState {
	int pxperbeat;
} SeqEditorTrackState;

void seqeditortrackstate_init(SeqEditorTrackState*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Color rulerbaselinecolour;
	psy_ui_Color rulermarkcolour;
	SeqEditorTrackState* trackstate;
	Workspace* workspace;
} SeqEditorHeader;

void seqeditorheader_init(SeqEditorHeader*, psy_ui_Component* parent,
	SeqEditorTrackState*, Workspace*);

INLINE psy_ui_Component* seqeditorheader_base(SeqEditorHeader* self)
{
	return &self->component;
}

struct SeqEditorTrack;
struct SeqEditorTracks;

typedef void (*seqeditortrack_fp_ondraw)(struct SeqEditorTrack*, psy_ui_Graphics*, int x, int y);
typedef void (*seqeditortrack_fp_onpreferredsize)(struct SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
typedef void (*seqeditortrack_fp_onmousedown)(struct SeqEditorTrack*,
	psy_ui_MouseEvent*);
typedef void (*seqeditortrack_fp_onmousemove)(struct SeqEditorTrack*,
	psy_ui_MouseEvent*);
typedef void (*seqeditortrack_fp_onmouseup)(struct SeqEditorTrack*,
	psy_ui_MouseEvent*);

typedef struct SeqEditorTrackVTable {
	seqeditortrack_fp_ondraw ondraw;
	seqeditortrack_fp_onpreferredsize onpreferredsize;
	seqeditortrack_fp_onmousedown onmousedown;
	seqeditortrack_fp_onmousemove onmousemove;
	seqeditortrack_fp_onmouseup onmouseup;
} SeqEditorTrackVTable;

typedef struct SeqEditorTrack {
	SeqEditorTrackVTable* vtable;
	struct SeqEditorTracks* parent;
	psy_audio_SequenceTrack* currtrack;
	uintptr_t trackindex;
	SeqEditorTrackState* trackstate;
	bool dragstarting;
	psy_audio_SequenceEntryNode* drag_sequenceitem_node;
	psy_dsp_big_beat_t itemdragposition;
	Workspace* workspace;
} SeqEditorTrack;

void seqeditortrack_init(SeqEditorTrack*, struct SeqEditorTracks* parent,
	SeqEditorTrackState*, Workspace*);
void seqeditortrack_dispose(SeqEditorTrack*);

SeqEditorTrack* seqeditortrack_alloc(void);
SeqEditorTrack* seqeditortrack_allocinit(struct SeqEditorTracks* parent,
	SeqEditorTrackState*, Workspace*);

void seqeditortrack_updatetrack(SeqEditorTrack*, psy_audio_SequenceTrack*,
	uintptr_t trackindex);

INLINE SeqEditorTrack* seqeditortrack_base(SeqEditorTrack* self)
{
	assert(self);

	return self;
}

INLINE void seqeditortrack_ondraw(SeqEditorTrack* self, psy_ui_Graphics* g, int x, int y)
{
	assert(self);

	self->vtable->ondraw(self, g, x, y);
}

INLINE void seqeditortrack_onpreferredsize(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(self);

	self->vtable->onpreferredsize(self, limit, rv);
}

INLINE void seqeditortrack_onmousedown(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	self->vtable->onmousedown(self, ev);
}

INLINE void seqeditortrack_onmousemove(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	self->vtable->onmousemove(self, ev);
}

INLINE void seqeditortrack_onmouseup(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	self->vtable->onmouseup(self, ev);
}

typedef struct SeqEditorTracks {
	psy_ui_Component component;
	SeqEditorTrackState* trackstate;
	Workspace* workspace;
	psy_List* tracks;
	int lastplaylinepx;
	SeqEditorTrack* capture;
} SeqEditorTracks;

void seqeditortracks_init(SeqEditorTracks*, psy_ui_Component* parent,
	SeqEditorTrackState*, Workspace*);
bool seqeditortracks_playlinechanged(SeqEditorTracks*);

INLINE psy_ui_Component* seqeditortracks_base(SeqEditorTracks* self)
{
	return &self->component;
}

typedef struct {
	psy_ui_Component component;
	SeqEditorHeader header;
	psy_ui_Scroller scroller;
	SeqEditorTracks tracks;
	SeqEditorTrackState trackstate;
	Workspace* workspace;
} SeqEditor;

void seqeditor_init(SeqEditor*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* seqeditor_base(SeqEditor* self)
{
	return &self->component;
}

#endif /* SEQEDITOR_H */
