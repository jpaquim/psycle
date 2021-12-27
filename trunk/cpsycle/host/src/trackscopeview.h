/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKSCOPEVIEW_H)
#define TRACKSCOPEVIEW_H

/* host */
#include "workspace.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** TrackScopes
**
** Displays scopes for each pattern track with the current machine buffer
*/

/* TrackScopes */
typedef struct TrackScopes {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	uintptr_t maxcolumns;
	double trackwidth;
	double trackheight;
	double textheight;	
	bool running;
	/* references */
	Workspace* workspace;	
} TrackScopes;

void trackscopes_init(TrackScopes*, psy_ui_Component* parent,
	Workspace*);
void trackscopes_start(TrackScopes*);
void trackscopes_stop(TrackScopes*);
void trackscopes_idle(TrackScopes*);

INLINE psy_ui_Component* trackscopes_base(TrackScopes* self)
{
	assert(self);

	return &self->component;
}

/* TrackScopeView */
typedef struct TrackScopeView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	TrackScopes scopes;
} TrackScopeView;

void trackscopeview_init(TrackScopeView*, psy_ui_Component* parent,
	Workspace*);

void trackscopeview_idle(TrackScopeView*);

INLINE psy_ui_Component* trackscopeview_base(TrackScopeView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKSCOPEVIEW_H */
