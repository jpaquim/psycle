/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VIEWHISTORY_H)
#define VIEWHISTORY_H

/* host*/
#include "viewindex.h"
/* container */
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_List* container;
	bool prevented;
	psy_List* currnavigation;
	bool navigating;
} ViewHistory;

void viewhistory_init(ViewHistory*);
void viewhistory_dispose(ViewHistory*);
void viewhistory_clear(ViewHistory*);
void viewhistory_add(ViewHistory*, ViewIndex view);
void viewhistory_add_seq_pos(ViewHistory*, uintptr_t seqpos);
bool viewhistory_back(ViewHistory*);
bool viewhistory_forward(ViewHistory*);
ViewIndex viewhistory_current(const ViewHistory*);
bool viewhistory_equal(const ViewHistory*, ViewIndex);

INLINE bool viewhistory_has_curr_view(const ViewHistory* self)
{
	assert(self);

	return self->currnavigation != NULL;
}

INLINE void viewhistory_prevent(ViewHistory* self)
{
	assert(self);

	self->prevented = TRUE;
}

INLINE void viewhistory_enable(ViewHistory* self)
{
	assert(self);

	self->prevented = FALSE;
}

INLINE bool viewhistory_prevented(const ViewHistory* self)
{
	assert(self);

	return self->prevented;
}

#ifdef __cplusplus
}
#endif

#endif /* VIEWHISTORY_H */
