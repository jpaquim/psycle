/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

/* host */
#include "sequencehostcmds.h"
#include "sequencetrackbox.h"
#include "wavebox.h"
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
	psy_Signal signal_cursorchanged;
	double pxperbeat;
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
	psy_ui_Edit* edit;	
} SeqEditState;

void seqeditstate_init(SeqEditState*, SequenceCmds*, psy_ui_Edit*);
void seqeditstate_dispose(SeqEditState*);

psy_audio_Sequence* seqeditstate_sequence(SeqEditState*);

INLINE psy_audio_OrderIndex seqeditstate_editposition(const SeqEditState* self)
{	
	return workspace_sequenceeditposition(self->workspace);
}

INLINE double seqeditstate_beattopx(const SeqEditState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return floor(position * self->pxperbeat);
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
	psy_signal_emit(&self->signal_cursorchanged, self, 0);
}

psy_dsp_big_beat_t seqeditstate_quantize(const SeqEditState*,
	psy_dsp_big_beat_t position);

INLINE psy_ui_Value seqeditstate_lineheight(const SeqEditState* self)
{
	return self->lineheight;
}

/* SeqEditRuler */
typedef struct SeqEditRuler {
	/* inherits */
	psy_ui_Component component;	
	/* references */
	SeqEditState* state;		
} SeqEditRuler;

void seqeditruler_init(SeqEditRuler*, psy_ui_Component* parent,
	SeqEditState*);

INLINE psy_ui_Component* seqeditruler_base(SeqEditRuler* self)
{
	return &self->component;
}

/* SeqEditorLine */
typedef struct SeqEditorLine {
	/* inherits */
	psy_ui_Component component;
	/* references */
	SeqEditState* state;
} SeqEditorLine;

void seqeditorline_init(SeqEditorLine*,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState*);

SeqEditorLine* seqeditorline_alloc(void);
SeqEditorLine* seqeditorline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState*);

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
	SeqEditState*);

SeqEditorPlayline* seqeditorplayline_alloc(void);
SeqEditorPlayline* seqeditorplayline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState*);
void seqeditorplayline_update(SeqEditorPlayline*);


/* SeqEditEntry */
typedef struct SeqEditEntry {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_audio_OrderIndex seqpos;
	bool preventresize;
	/* references */
	SeqEditState* state;
	psy_audio_SequenceEntry* seqentry;	
} SeqEditEntry;

void seqeditentry_init(SeqEditEntry*,
	psy_ui_Component* parent, psy_ui_Component* view, psy_audio_SequenceEntry*,
	psy_audio_OrderIndex seqpos, SeqEditState*);

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
	SeqEditState*);

SeqEditPatternEntry* seqeditpatternentry_alloc(void);
SeqEditPatternEntry* seqeditpatternentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditState*);

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
	bool preventedit;
} SeqEditMarkerEntry;

void seqeditmarkerentry_init(SeqEditMarkerEntry*,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry*, psy_audio_OrderIndex seqpos,
	SeqEditState*);

SeqEditMarkerEntry* seqeditmarkerentry_alloc(void);
SeqEditMarkerEntry* seqeditmarkerentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditState*);

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
	SeqEditState*);

SeqEditSampleEntry* seqeditsampleentry_alloc(void);
SeqEditSampleEntry* seqeditsampleentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditState*);

void seqeditsampleentry_updatesample(SeqEditSampleEntry*);

INLINE psy_ui_Component* seqeditsampleentry_base(SeqEditSampleEntry* self)
{
	assert(self);

	return &self->seqeditorentry.component;
}

/* SeqEditTrack */
struct SeqEditTrack;
struct SeqEditorTracks;

typedef struct SeqEditTrack {
	psy_ui_Component component;	
	psy_audio_SequenceTrack* currtrack;	
	uintptr_t trackindex;
	SeqEditState* state;	
	Workspace* workspace;	
	psy_ui_Component* view;
	psy_List* entries;
} SeqEditTrack;

void seqedittrack_init(SeqEditTrack*,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState*, Workspace*);
void seqedittrack_dispose(SeqEditTrack*);

SeqEditTrack* seqedittrack_alloc(void);
SeqEditTrack* seqedittrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState*, Workspace*);

void seqedittrack_updatetrack(SeqEditTrack*,
	psy_audio_SequenceTrackNode*,
	psy_audio_SequenceTrack*,
	uintptr_t trackindex);

INLINE psy_ui_Component* seqedittrack_base(SeqEditTrack* self)
{
	assert(self);

	return &self->component;
}

/* SeqEditTrackDesc */
typedef struct SeqEditTrackDesc {
	/* inherits */
	psy_ui_Component component;	
	/* signals */
	psy_Signal signal_resize;
	/* references */
	SeqEditState* state;
	Workspace* workspace;	
} SeqEditTrackDesc;

void seqedittrackdesc_init(SeqEditTrackDesc*, psy_ui_Component* parent,
	SeqEditState*, Workspace*);

typedef struct SeqEditorTracks {
	psy_ui_Component component;
	SeqEditState* state;
	Workspace* workspace;	
	SeqEditorPlayline* playline;
	SeqEditorLine* cursorline;
	SeqEditorLine* seqeditposline;
} SeqEditorTracks;

void seqeditortracks_init(SeqEditorTracks*, psy_ui_Component* parent,
	SeqEditState*, Workspace*);

void seqeditortracks_checkcursorline(SeqEditorTracks*);

INLINE psy_ui_Component* seqeditortracks_base(SeqEditorTracks* self)
{
	return &self->component;
}

typedef struct SeqEditorHeaderBar {
	psy_ui_Component component;
	ZoomBox hzoom;	
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
	psy_ui_Button assignsample;
	psy_ui_Button configure;
	psy_ui_Button expand;
	/* references */	
	SeqEditState* state;
} SeqEditToolBar;

void seqedittoolbar_init(SeqEditToolBar*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditortoolbar_setdragtype(SeqEditToolBar*, SeqEditorDragType);

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
	SeqEditRuler ruler;
	psy_ui_Scroller scroller;	
	psy_ui_Component left;
	SeqEditorHeaderBar headerbar;
	ZoomBox vzoom;
	psy_ui_Component trackdescpane;
	SeqEditTrackDesc trackdesc;
	SeqEditorTracks tracks;	
	SeqEditState state;
	SequenceCmds cmds;	
	bool expanded;
	psy_ui_Edit edit;
} SeqEditor;

void seqeditor_init(SeqEditor*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* seqeditor_base(SeqEditor* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITOR_H */
