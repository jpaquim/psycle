/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKBOX_H)
#define TRACKBOX_H

/* ui */
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TrackBox {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_mute;
	psy_Signal signal_solo;
	psy_Signal signal_close;
	psy_Signal signal_resize;
	/* internal */
	psy_ui_Component client;
	psy_ui_Label track;
	psy_ui_Button solo;
	psy_ui_Button mute;
	psy_ui_Label desc;
	psy_ui_Button close;	
	psy_ui_Component resize;
	uintptr_t trackidx;
	bool closeprevented;
	bool doresize;
	double dragoffset;
	double baseheight;
} TrackBox;

void trackbox_init(TrackBox*, psy_ui_Component* parent,
	psy_ui_Component* view);

TrackBox* trackbox_alloc(void);
TrackBox* trackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view);

void trackbox_setdescription(TrackBox*, const char* text);
void trackbox_setindex(TrackBox*, uintptr_t index);
void trackbox_mute(TrackBox*);
void trackbox_unmute(TrackBox*);
void trackbox_solo(TrackBox*);
void trackbox_unsolo(TrackBox*);
void trackbox_preventclose(TrackBox*);

INLINE uintptr_t trackbox_trackindex(const TrackBox* self)
{
	assert(self);

	return self->trackidx;
}

INLINE psy_ui_Component* trackbox_base(TrackBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKBOX_H */
