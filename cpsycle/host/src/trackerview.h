/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERVIEW)
#define TRACKERVIEW

/* host */
#include "trackercolumn.h"
#include "trackergridstate.h"
#include "patternhostcmds.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** TrackerView
**
**  The TrackerView is where you enter notes. It displays a Pattern selected by
**  the SeqView as a tracker grid.
*/

typedef struct TrackerGrid {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_colresize;
	/* internal */
	PatternViewState defaultpvstate;
	TrackerState defaultgridstate;	
	psy_dsp_NotesTabMode notestabmode;   
	psy_audio_SequenceCursor oldcursor;
	psy_audio_SequenceCursor lastdragcursor;	
	int chordmodestarting;
	bool chordmode;
	uintptr_t chordbegin;
	uintptr_t dragtrack;
	uintptr_t dragparamcol;
	bool syncpattern;	
	bool effcursoralwaysdown;	
	bool preventscrolltop;
	psy_Table columns;
	bool preventeventdriver;	
	/* references */
	TrackerState* state;	
	Workspace* workspace;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, psy_ui_Component* parent,
	TrackConfig*, TrackerState*, Workspace*);

void trackergrid_build(TrackerGrid*);
void trackergrid_setsharedgridstate(TrackerGrid*, TrackerState*,
	TrackConfig*);
void trackergrid_setpattern(TrackerGrid*, psy_audio_Pattern*);
void trackergrid_showemptydata(TrackerGrid*, int showstate);
void trackergrid_invalidateline(TrackerGrid*, intptr_t line);
void trackergrid_invalidatelines(TrackerGrid*, intptr_t line1, intptr_t line2);
void trackergrid_invalidatecursor(TrackerGrid*);
void trackergrid_invalidateinternalcursor(TrackerGrid*,
	psy_audio_SequenceCursor);
void trackergrid_centeroncursor(TrackerGrid*);
void trackergrid_setcentermode(TrackerGrid*, int mode);
void trackergrid_tweak(TrackerGrid*, int slot, uintptr_t tweak,
	float normvalue);

INLINE const psy_audio_BlockSelection* trackergrid_selection(
	const TrackerGrid* self)
{
	return &self->state->pv->selection;
}

bool trackergrid_handlecommand(TrackerGrid*, intptr_t cmd);
/* block menu */
void trackergrid_changegenerator(TrackerGrid*);
void trackergrid_changeinstrument(TrackerGrid*);
void trackergrid_blockstart(TrackerGrid*);
void trackergrid_blockend(TrackerGrid*);

INLINE bool trackergrid_midline(TrackerGrid* self)
{
	return self->state->midline;
}

INLINE psy_ui_Component* trackergrid_base(TrackerGrid* self)
{
	assert(self);

	return &self->component;
}

INLINE bool trackergrid_checkupdate(const TrackerGrid* self)
{
	if (self->state->pv->pattern) {
		bool rv;
		uintptr_t opcount;

		opcount = self->state->pv->pattern->opcount;
		rv = (opcount != self->component.opcount);
		((TrackerGrid*)self)->component.opcount = opcount;
		return rv;
	}
	return FALSE;
}

/* TrackerView */

typedef struct TrackerView {
	/* inherits */
	psy_ui_Scroller scroller;
	TrackerGrid grid;
	/* references */
	Workspace* workspace;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent, TrackConfig*,
	TrackerState*, Workspace*);

void trackerview_updatefollowsong(TrackerView*);

#ifdef __cplusplus
}
#endif

#endif /* TRACKERVIEW */
