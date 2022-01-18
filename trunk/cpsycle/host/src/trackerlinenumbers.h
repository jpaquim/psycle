/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERLINENUMBERS)
#define TRACKERLINENUMBERS

/* host */
#include "trackergridstate.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** TrackerLineNumbers
** 
** Draws the tracker linenumbers
*/

struct TrackerView;

typedef struct TrackerLineNumbersLabel {
	/* inherits */
	psy_ui_Component component;	
	/* internal data */
	char_dyn_t* linestr;
	char_dyn_t* defaultstr;
	double headerheight;
	bool showdefaultline;
	bool showbeatoffset;	
	/* references */	
	TrackerState* state;
} TrackerLineNumbersLabel;

void trackerlinenumberslabel_init(TrackerLineNumbersLabel*,
	psy_ui_Component* parent, TrackerState*);

INLINE void trackerlinenumberslabel_setheaderheight(TrackerLineNumbersLabel* self,
	double headerheight)
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

INLINE void trackerlinenumberslabel_showbeat(TrackerLineNumbersLabel* self)
{
	self->showbeatoffset = TRUE;
}

INLINE void trackerlinenumberslabel_hidebeat(TrackerLineNumbersLabel* self)
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

/* TrackerLineNumbers*/
typedef struct TrackerLineNumbers {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	bool showcursor;
	bool shownumbersinhex;
	bool showbeat;
	const char* format;
	const char* format_seqstart;
	/* references */
	TrackerState* state;
	Workspace* workspace;
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, psy_ui_Component* parent,
	TrackerState*, Workspace*);

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const psy_audio_SequenceCursor*);
void trackerlinenumbers_invalidateline(TrackerLineNumbers*, intptr_t line);
void trackerlinenumbers_updateformat(TrackerLineNumbers*);
void trackerlinenumbers_showlinenumbercursor(TrackerLineNumbers*, bool showstate);
void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers*, bool showstate);

INLINE void trackerlinenumbers_showbeat(TrackerLineNumbers* self)
{
	self->showbeat = TRUE;
	trackerlinenumbers_updateformat(self);
}

INLINE void trackerlinenumbers_hidebeat(TrackerLineNumbers* self)
{
	self->showbeat = FALSE;
	trackerlinenumbers_updateformat(self);
}

void trackerlinenumbers_updatecursor(TrackerLineNumbers*);

INLINE psy_ui_Component* trackerlinenumbers_base(TrackerLineNumbers* self)
{
	assert(self);

	return &self->component;
}

/* TrackerLineNumberBar */
typedef struct TrackerLineNumberBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	TrackerLineNumbersLabel linenumberslabel;
	psy_ui_Component linenumberpane;
	TrackerLineNumbers linenumbers;	
	int zoomheightbase;		
	/* references */
	Workspace* workspace;
} TrackerLineNumberBar;

void trackerlinenumberbar_init(TrackerLineNumberBar*, psy_ui_Component* parent,
	TrackerState*, Workspace*);

INLINE psy_ui_Component* trackerlinenumberbar_base(TrackerLineNumberBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERLINENUMBERS */
