// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

#include "workspace.h"
#include "zoombox.h"

#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SeqEditorTrackState {
	int pxperbeat;
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;
	psy_ui_Value linemargin;
} SeqEditorTrackState;

void seqeditortrackstate_init(SeqEditorTrackState*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Colour rulerbaselinecolour;
	psy_ui_Colour rulermarkcolour;
	SeqEditorTrackState* trackstate;
	Workspace* workspace;
} SeqEditorRuler;

void seqeditorruler_init(SeqEditorRuler*, psy_ui_Component* parent,
	SeqEditorTrackState*, Workspace*);

INLINE psy_ui_Component* seqeditorruler_base(SeqEditorRuler* self)
{
	return &self->component;
}

struct SeqEditorTrack;
struct SeqEditorTracks;

typedef void (*seqeditortrack_fp_ondraw)(struct SeqEditorTrack*, psy_ui_Graphics*, int x, int y);
typedef void (*seqeditortrack_fp_onpreferredsize)(struct SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
typedef bool (*seqeditortrack_fp_onmousedown)(struct SeqEditorTrack*,
	psy_ui_MouseEvent*);
typedef bool (*seqeditortrack_fp_onmousemove)(struct SeqEditorTrack*,
	psy_ui_MouseEvent*);
typedef bool (*seqeditortrack_fp_onmouseup)(struct SeqEditorTrack*,
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

INLINE bool seqeditortrack_onmousedown(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	return self->vtable->onmousedown(self, ev);
}

INLINE bool seqeditortrack_onmousemove(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	return self->vtable->onmousemove(self, ev);
}

INLINE bool seqeditortrack_onmouseup(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	return self->vtable->onmouseup(self, ev);
}

typedef struct SeqEditorTrackHeader {
	SeqEditorTrack base;	
} SeqEditorTrackHeader;

void seqeditortrackheader_init(SeqEditorTrackHeader*,
	struct SeqEditorTracks* parent,
	SeqEditorTrackState*, Workspace*);

SeqEditorTrackHeader* seqeditortrackheader_alloc(void);
SeqEditorTrackHeader* seqeditortrackheader_allocinit(struct SeqEditorTracks* parent,
	SeqEditorTrackState*, Workspace*);

INLINE SeqEditorTrack* seqeditortrackheader_base(SeqEditorTrackHeader* self)
{
	return &self->base;
}

enum {
	SEQEDITOR_TRACKMODE_ENTRY,
	SEQEDITOR_TRACKMODE_HEADER,
};

typedef struct SeqEditorTracks {
	psy_ui_Component component;
	SeqEditorTrackState* trackstate;
	Workspace* workspace;
	psy_List* tracks;
	int lastplaylinepx;
	SeqEditorTrack* capture;
	int mode;
} SeqEditorTracks;

void seqeditortracks_init(SeqEditorTracks*, psy_ui_Component* parent,
	SeqEditorTrackState*, int mode, Workspace*);
bool seqeditortracks_playlinechanged(SeqEditorTracks*);

INLINE psy_ui_Component* seqeditortracks_base(SeqEditorTracks* self)
{
	return &self->component;
}

typedef struct {
	psy_ui_Component component;
	SeqEditorRuler ruler;
	psy_ui_Scroller scroller;
	psy_ui_Component left;
	ZoomBox zoombox_beat;
	ZoomBox zoombox_height;
	SeqEditorTracks trackheaders;
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

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITOR_H */
