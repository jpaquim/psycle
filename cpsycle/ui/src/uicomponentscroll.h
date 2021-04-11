// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMPONENTSCROLL_H
#define psy_ui_COMPONENTSCROLL_H

// local
#include "uidef.h"

// psy_ui_ComponentScroll

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_ui_ScrollMode {
	psy_ui_SCROLL_GRAPHICS = 1,
	psy_ui_SCROLL_COMPONENTS
} psy_ui_ScrollMode;

// psy_ui_ComponentScroll
typedef struct psy_ui_ComponentScroll {
	psy_ui_ScrollMode mode;
	psy_ui_Size step;
	// psy_ui_SCROLL_GRAPHICS
	psy_ui_Point offset;
	psy_ui_IntPoint vrange;
	psy_ui_IntPoint hrange;
	int wheel;
} psy_ui_ComponentScroll;

void psy_ui_componentscroll_init(psy_ui_ComponentScroll*);

INLINE void psy_ui_componentscroll_setoffset(psy_ui_ComponentScroll* self,
	psy_ui_Point offset)
{
	self->offset = offset;
}

INLINE psy_ui_Point psy_ui_componentscroll_offset(
	const psy_ui_ComponentScroll* self)
{
	return self->offset;
}

INLINE void psy_ui_componentscroll_sethrange(psy_ui_ComponentScroll* self,
	psy_ui_IntPoint range)
{
	self->hrange = range;
}

INLINE psy_ui_IntPoint psy_ui_componentscroll_hrange(
	const psy_ui_ComponentScroll* self)
{
	return self->hrange;
}

INLINE void psy_ui_componentscroll_setvrange(psy_ui_ComponentScroll* self,
	psy_ui_IntPoint range)
{
	self->vrange = range;
}

INLINE psy_ui_IntPoint psy_ui_componentscroll_vrange(
	const psy_ui_ComponentScroll* self)
{
	return self->vrange;
}

INLINE void psy_ui_componentscroll_setwheel(
	psy_ui_ComponentScroll* self, int wheel)
{
	self->wheel = wheel;
}

INLINE int psy_ui_componentscroll_wheel(
	const psy_ui_ComponentScroll* self)
{
	return self->wheel;
}

INLINE void psy_ui_componentscroll_setmode(psy_ui_ComponentScroll* self,
	psy_ui_ScrollMode mode)
{
	self->mode = mode;
}

INLINE psy_ui_ScrollMode psy_ui_componentscroll_mode(
	const psy_ui_ComponentScroll* self)
{
	return self->mode;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSCROLL_H */
