// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERLINENUMBERS)
#define TRACKERLINENUMBERS

// host
#include "trackergridstate.h"
#include "trackerlinestate.h"
#include "workspace.h"
#include "zoombox.h"

#ifdef __cplusplus
extern "C" {
#endif

// TrackerLineNumbers
// 
// Draws the tracker linenumbers

struct TrackerView;

typedef struct TrackerLineNumbersLabel {
	// inherits
	psy_ui_Component component;	
	// internal data
	char_dyn_t* linestr;
	char_dyn_t* defaultstr;
	int headerheight;
	bool showdefaultline;
	bool showbeatoffset;
	// references
	struct TrackerView* view;
	TrackerLineState* linestate;
	TrackerLineState defaultlinestate;
	Workspace* workspace;
} TrackerLineNumbersLabel;

void trackerlinenumberslabel_init(TrackerLineNumbersLabel*,
	psy_ui_Component* parent, TrackerLineState*, Workspace*);

INLINE void trackerlinenumberslabel_setheaderheight(TrackerLineNumbersLabel* self,
	int headerheight)
{
	self->headerheight = headerheight;
}

INLINE void trackerlinenumberslabel_showdefaultline(TrackerLineNumbersLabel* self)
{
	self->showdefaultline = TRUE;
}

INLINE void trackerlinenumberslabel_hidedefaultline(TrackerLineNumbersLabel* self)
{
	self->showdefaultline = FALSE;
}

INLINE void trackerlinenumberslabel_showbeatoffset(TrackerLineNumbersLabel* self)
{
	self->showbeatoffset = TRUE;
}

INLINE void trackerlinenumberslabel_hidebeatoffset(TrackerLineNumbersLabel* self)
{
	self->showbeatoffset = FALSE;
}

INLINE psy_ui_Component* TrackerLineNumbersLabel_base(TrackerLineNumbersLabel* self)
{
	assert(self);

	return &self->component;
}

INLINE psy_ui_Component* trackerlinenumberslabel_base(TrackerLineNumbersLabel* self)
{
	assert(self);

	return &self->component;
}


typedef struct TrackerLineNumbers {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_audio_PatternCursor lastcursor;	
	bool showcursor;
	bool shownumbersinhex;
	// references
	TrackerLineState* linestate;
	TrackerLineState defaultlinestate;
	Workspace* workspace;
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, psy_ui_Component* parent,
	TrackerLineState*, Workspace*);
void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers*, TrackerLineState*);
void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const psy_audio_PatternCursor*);
void trackerlinenumbers_invalidateline(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset);
void trackerlinenumbers_showlinenumbercursor(TrackerLineNumbers*, bool showstate);
void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers*, bool showstate);

INLINE psy_ui_Component* trackerlinenumbers_base(TrackerLineNumbers* self)
{
	assert(self);

	return &self->component;
}

// TrackerLineNumberBar
typedef struct TrackerLineNumberBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	TrackerLineNumbersLabel linenumberslabel;
	TrackerLineNumbers linenumbers;
	ZoomBox zoombox;
	// internal data
	int zoomheightbase;
} TrackerLineNumberBar;

void trackerlinenumberbar_init(TrackerLineNumberBar*, psy_ui_Component*
	parent, TrackerLineState*, Workspace*);

INLINE psy_ui_Component* trackerlinenumberbar_base(TrackerLineNumberBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERLINENUMBERS */
