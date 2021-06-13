/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

/* host */
#include "intedit.h"
#include "sequencehostcmds.h"
#include "sequencetrackbox.h"
#include "seqeditorentry.h"
#include "seqeditorstate.h"
#include "wavebox.h"
#include "workspace.h"
#include "zoombox.h"
/* ui */
#include <uicombobox.h>
#include <uicheckbox.h>
#include <uiedit.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif


/* SeqEditRuler */
typedef struct SeqEditRuler {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	bool showtimesig;
	bool preventedit;
	bool editnominator;
	/* references */
	SeqEditState* state;		
	psy_audio_PatternEvent* currtimesig;
} SeqEditRuler;

void seqeditruler_init(SeqEditRuler*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditruler_hidetimesig(SeqEditRuler*);
void seqeditruler_showtimesig(SeqEditRuler*);

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

void seqedittrack_setsequencetrack(SeqEditTrack*,
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
	psy_ui_Component top;
	psy_ui_Button timesig;
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
	psy_ui_CheckBox usesamplerindex;	
	IntEdit samplerindex;
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
