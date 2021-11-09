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
	bool wraparound;	
	bool ft2home;
	bool ft2delete;
	bool effcursoralwaysdown;
	bool movecursoronestep;
	intptr_t pgupdownstep;
	bool preventscrolltop;
	psy_Table columns;
	bool preventeventdriver;
	PatternCmds cmds;
	/* references */
	TrackerState* state;	
	Workspace* workspace;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, psy_ui_Component* parent,
	psy_ui_Component* view, TrackConfig*, TrackerState*,
	Workspace*);

void trackergrid_build(TrackerGrid*);
void trackergrid_setsharedgridstate(TrackerGrid*, TrackerState*,
	TrackConfig*);
void trackergrid_setpattern(TrackerGrid*, psy_audio_Pattern*);
void trackergrid_showemptydata(TrackerGrid*, int showstate);
void trackergrid_invalidateline(TrackerGrid*, intptr_t line);
bool trackergrid_scrollup(TrackerGrid*, psy_audio_SequenceCursor);
bool trackergrid_scrolldown(TrackerGrid*, psy_audio_SequenceCursor);
bool trackergrid_scrollleft(TrackerGrid*, psy_audio_SequenceCursor);
bool trackergrid_scrollright(TrackerGrid*, psy_audio_SequenceCursor);
void trackergrid_storecursor(TrackerGrid*);
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
	return &self->state->pv.selection;
}

INLINE void trackergrid_enableft2home(TrackerGrid* self)
{
	self->ft2home = TRUE;
}

INLINE void trackergrid_enableithome(TrackerGrid* self)
{
	self->ft2home = FALSE;
}

INLINE void trackergrid_enableft2delete(TrackerGrid* self)
{
	self->ft2delete = TRUE;
}

INLINE void trackergrid_enableitdelete(TrackerGrid* self)
{
	self->ft2delete = FALSE;
}

INLINE void trackergrid_enablemovecursoronestep(TrackerGrid* self)
{
	self->movecursoronestep = TRUE;
}

INLINE void trackergrid_disablemovecursoronestep(TrackerGrid* self)
{
	self->movecursoronestep = FALSE;
}

INLINE void trackergrid_enableeffcursoralwaysdown(TrackerGrid* self)
{
	self->movecursoronestep = TRUE;
}

INLINE void trackergrid_disableffcursoralwaysdown(TrackerGrid* self)
{
	self->movecursoronestep = FALSE;
}

bool trackergrid_handlecommand(TrackerGrid*, intptr_t cmd);
/* block menu */
void trackergrid_changegenerator(TrackerGrid*);
void trackergrid_changeinstrument(TrackerGrid*);
void trackergrid_blockstart(TrackerGrid*);
void trackergrid_blockend(TrackerGrid*);
void trackergrid_blockunmark(TrackerGrid*);
void trackergrid_blockcut(TrackerGrid*);
void trackergrid_blockcopy(TrackerGrid*);
void trackergrid_blockpaste(TrackerGrid*);
void trackergrid_blockmixpaste(TrackerGrid*);
void trackergrid_blockdelete(TrackerGrid*);
void trackergrid_blocktransposeup(TrackerGrid*);
void trackergrid_blocktransposedown(TrackerGrid*);
void trackergrid_blocktransposeup12(TrackerGrid*);
void trackergrid_blocktransposedown12(TrackerGrid*);

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
	if (self->state->pv.pattern) {
		bool rv;
		uintptr_t opcount;

		opcount = self->state->pv.pattern->opcount;
		rv = (opcount != self->component.opcount && self->syncpattern);
		((TrackerGrid*)self)->component.opcount = opcount;
		return rv;
	}
	return FALSE;
}

/* TrackerView */

typedef struct TrackerView {
	psy_ui_Scroller scroller;
	TrackerGrid grid;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent, TrackConfig*,
	TrackerState*, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* TRACKERVIEW */
