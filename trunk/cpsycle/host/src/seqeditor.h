/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

/* host */
#include "patternviewskin.h"
#include "sequencehostcmds.h"
#include "sequencetrackbox.h"
#include "workspace.h"
#include "zoombox.h"
/* ui */
#include <uicombobox.h>
#include <uiedit.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SEQEDITORDRAG_NONE = 0,
	SEQEDITORDRAG_MOVE = 1,
	SEQEDITORDRAG_REORDER = 2
} SeqEditorDragMode;

typedef enum {
	SEQEDTCMD_NONE = 0,
	SEQEDTCMD_NEWTRACK = 1,
	SEQEDTCMD_DELTRACK = 2,
	SEQEDTCMD_INSERTPATTERN = 3,
	SEQEDTCMD_REORDER = 4,
	SEQEDTCMD_REMOVEPATTERN = 5,
} SeqEdtCmd;

typedef struct SeqEditorState {
	psy_Signal signal_cursorchanged;
	double pxperbeat;
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;
	psy_ui_Value linemargin;
	psy_dsp_big_beat_t cursorposition;
	bool drawcursor;
	bool cursoractive;
	bool drawpatternevents;
	SeqEdtCmd cmd;
	uintptr_t cmdtrack;
	uintptr_t cmdrow;
	bool showpatternnames;
	/* drag */
	SeqEditorDragMode dragmode;
	bool dragstatus;
	bool dragstart;
	bool draglength;
	bool dragselection;
	psy_dsp_big_beat_t dragstartoffset;
	bool updatecursorposition;
	psy_ui_RealPoint dragpt;
	psy_audio_OrderIndex dragseqpos;
	psy_audio_SequenceEntryType inserttype;
	/* references */
	psy_audio_SequenceEntry* sequenceentry;
	Workspace* workspace;	
	SequenceCmds* cmds;
	psy_ui_Edit* edit;
} SeqEditorState;

void seqeditorstate_init(SeqEditorState*, Workspace*, SequenceCmds*, psy_ui_Edit*);
void seqeditorstate_dispose(SeqEditorState*);

psy_audio_Sequence* seqeditorstate_sequence(SeqEditorState*);

INLINE double seqeditorstate_beattopx(const SeqEditorState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return position * self->pxperbeat;
}

INLINE psy_dsp_big_beat_t seqeditorstate_pxtobeat(const
	SeqEditorState* self, double px)
{
	assert(self);

	return px / (psy_dsp_big_beat_t)self->pxperbeat;
}

INLINE void seqeditorstate_setcursor(SeqEditorState* self,
	psy_dsp_big_beat_t cursorposition)
{
	assert(self);

	self->cursorposition = cursorposition;
	psy_signal_emit(&self->signal_cursorchanged, self, 0);
}

/* SeqEditorRuler */
typedef struct SeqEditorRuler {
	/* inherits */
	psy_ui_Component component;	
	/* references */
	SeqEditorState* state;
	PatternViewSkin* skin;	
	Workspace* workspace;
} SeqEditorRuler;

void seqeditorruler_init(SeqEditorRuler*, psy_ui_Component* parent,
	SeqEditorState*, PatternViewSkin* skin, Workspace*);

INLINE psy_ui_Component* seqeditorruler_base(SeqEditorRuler* self)
{
	return &self->component;
}

/* SeqEditorLine */
typedef struct SeqEditorLine {
	/* inherits */
	psy_ui_Component component;
	/* references */
	SeqEditorState* state;
} SeqEditorLine;

void seqeditorline_init(SeqEditorLine*,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState*);

SeqEditorLine* seqeditorline_alloc(void);
SeqEditorLine* seqeditorline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState*);

void seqeditorline_updateposition(SeqEditorLine*,
	psy_dsp_big_beat_t position);

INLINE psy_ui_Component* seqeditorline_base(SeqEditorLine* self)
{
	return &self->component;
}

/* SeqEditorPlayline */
typedef struct SeqEditorPlayline {
	/* inherits */
	SeqEditorLine seqeditorline;
	/* internal */
	bool drag;
	double dragbase;	
} SeqEditorPlayline;

void seqeditorplayline_init(SeqEditorPlayline*,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState*);

SeqEditorPlayline* seqeditorplayline_alloc(void);
SeqEditorPlayline* seqeditorplayline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState*);
void seqeditorplayline_update(SeqEditorPlayline*);

/* SeqEditorPatternEntry */
typedef struct SeqEditorPatternEntry {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	/* references */
	SeqEditorState* state;
	psy_audio_SequencePatternEntry* sequenceentry;
	psy_audio_OrderIndex seqpos;
} SeqEditorPatternEntry;

void seqeditorpatternentry_init(SeqEditorPatternEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry*, psy_audio_OrderIndex seqpos,
	SeqEditorState*);

SeqEditorPatternEntry* seqeditorpatternentry_alloc(void);
SeqEditorPatternEntry* seqeditorpatternentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditorState*);

/* SeqEditorMarkerEntry */
typedef struct SeqEditorMarkerEntry {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	/* references */
	SeqEditorState* state;
	psy_audio_SequenceMarkerEntry* sequenceentry;
	psy_audio_OrderIndex seqpos;
	bool preventedit;
} SeqEditorMarkerEntry;

void seqeditormarkerentry_init(SeqEditorMarkerEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry*, psy_audio_OrderIndex seqpos,
	SeqEditorState*);

SeqEditorMarkerEntry* seqeditormarkerentry_alloc(void);
SeqEditorMarkerEntry* seqeditormarkerentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditorState*);


/* SeqEditorSampleEntry */
typedef struct SeqEditorSampleEntry {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	/* references */
	SeqEditorState* state;
	psy_audio_SequenceSampleEntry* sequenceentry;
	psy_audio_OrderIndex seqpos;
	bool preventedit;
} SeqEditorSampleEntry;

void seqeditorsampleentry_init(SeqEditorSampleEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry*, psy_audio_OrderIndex seqpos,
	SeqEditorState*);

SeqEditorSampleEntry* seqeditorsampleentry_alloc(void);
SeqEditorSampleEntry* seqeditorsampleentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditorState*);


/* SeqEditorTrack */
struct SeqEditorTrack;
struct SeqEditorTracks;

typedef struct SeqEditorTrack {
	psy_ui_Component component;	
	psy_audio_SequenceTrack* currtrack;
	psy_audio_SequenceTrackNode* currtracknode;
	uintptr_t trackindex;
	SeqEditorState* state;
	bool dragstarting;	
	double dragstartpx;	
	psy_audio_SequenceEntryNode* drag_sequenceitem_node;	
	Workspace* workspace;
	PatternViewSkin* skin;
	psy_ui_Component* view;
	SeqEditorLine* dragline;
	psy_List* entries;
} SeqEditorTrack;

void seqeditortrack_init(SeqEditorTrack*,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState*, PatternViewSkin*, Workspace*);
void seqeditortrack_dispose(SeqEditorTrack*);

SeqEditorTrack* seqeditortrack_alloc(void);
SeqEditorTrack* seqeditortrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState*, PatternViewSkin*, Workspace*);

void seqeditortrack_updatetrack(SeqEditorTrack*,
	psy_audio_SequenceTrackNode*,
	psy_audio_SequenceTrack*,
	uintptr_t trackindex);

INLINE psy_ui_Component* seqeditortrack_base(SeqEditorTrack* self)
{
	assert(self);

	return &self->component;
}

/* SeqEditorTrackDesc */
typedef struct SeqEditorTrackDesc {
	/* inherits */
	psy_ui_Component component;	
	/* references */
	PatternViewSkin* skin;
	SeqEditorState* state;
	Workspace* workspace;	
} SeqEditorTrackDesc;

void seqeditortrackdesc_init(SeqEditorTrackDesc*, psy_ui_Component* parent,
	SeqEditorState*, Workspace*);

typedef struct SeqEditorTracks {
	psy_ui_Component component;
	SeqEditorState* state;
	Workspace* workspace;	
	PatternViewSkin* skin;
	SeqEditorPlayline* playline;
	SeqEditorLine* cursorline;
	SeqEditorLine* seqeditposline;
	psy_audio_PatternSelection selection;
	psy_audio_PatternCursor selectionbase;
} SeqEditorTracks;

void seqeditortracks_init(SeqEditorTracks*, psy_ui_Component* parent,
	SeqEditorState*, PatternViewSkin*, Workspace*);

void seqeditortracks_checkcursorline(SeqEditorTracks*);

INLINE psy_ui_Component* seqeditortracks_base(SeqEditorTracks* self)
{
	return &self->component;
}

typedef struct SeqEditorHeaderBar {
	psy_ui_Component component;
	ZoomBox zoombox_beat;	
} SeqEditorHeaderBar;

void seqeditorheaderbar_init(SeqEditorHeaderBar*, psy_ui_Component* parent);

/* SeqEditToolBar */
typedef struct SeqEditToolBar {
	/* inherits */
	psy_ui_Component component;	
	psy_ui_Button move;
	psy_ui_Button reorder;
	psy_ui_Label desctype;
	psy_ui_ComboBox inserttype;
	psy_ui_Button expand;
	/* references */
	Workspace* workspace;
	SeqEditorState* state;
} SeqEditToolBar;

void seqedittoolbar_init(SeqEditToolBar*, psy_ui_Component* parent,
	SeqEditorState*, Workspace*);

void seqeditortoolbar_setdragmode(SeqEditToolBar*, SeqEditorDragMode);

INLINE psy_ui_Component* seqedittoolbar_base(SeqEditToolBar* self)
{
	assert(self);

	return &self->component;
}

typedef struct SeqEditor {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	SeqEditToolBar toolbar;
	psy_ui_Component spacer;
	psy_ui_Component header;
	psy_ui_Component rulerpane;
	SeqEditorRuler ruler;
	psy_ui_Scroller scroller;	
	psy_ui_Component left;
	SeqEditorHeaderBar headerbar;
	ZoomBox zoombox_height;
	psy_ui_Component trackdescpane;
	SeqEditorTrackDesc trackdescriptions;
	SeqEditorTracks tracks;	
	SeqEditorState state;
	SequenceCmds cmds;	
	bool expanded;
	psy_ui_Edit edit;
	/* references */
	Workspace* workspace;
} SeqEditor;

void seqeditor_init(SeqEditor*, psy_ui_Component* parent,
	PatternViewSkin* skin, Workspace*);

INLINE psy_ui_Component* seqeditor_base(SeqEditor* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITOR_H */
