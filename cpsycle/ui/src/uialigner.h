/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_ALIGNER_H
#define psy_ui_ALIGNER_H

/* local */
#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Interface for Aligner
*/

struct psy_ui_Aligner;
struct psy_ui_Component;

typedef void (*psy_ui_fp_aligner_dispose)(struct psy_ui_Aligner*);
typedef void (*psy_ui_fp_aligner_align)(struct psy_ui_Aligner*, struct psy_ui_Component*);
typedef void (*psy_ui_fp_aligner_preferredsize)(struct psy_ui_Aligner*,
	struct psy_ui_Component*, const psy_ui_Size* limit, psy_ui_Size* rv);

typedef struct psy_ui_AlignerVTable {
	psy_ui_fp_aligner_dispose dispose;
	psy_ui_fp_aligner_align align;
	psy_ui_fp_aligner_preferredsize preferredsize;	
} psy_ui_AlignerVTable;

typedef struct psy_ui_Aligner {
	psy_ui_AlignerVTable* vtable;
	int alignexpandmode;
	psy_ui_AlignType insertaligntype;
	psy_ui_Margin insertmargin;
	psy_ui_AlignType alignsorted;
} psy_ui_Aligner;

void psy_ui_aligner_init(psy_ui_Aligner*);

INLINE void psy_ui_aligner_dispose(psy_ui_Aligner* self)
{
	assert(self);

	self->vtable->dispose(self);
}

INLINE void psy_ui_aligner_align(psy_ui_Aligner* self, struct psy_ui_Component* component)
{
	assert(self);

	self->vtable->align(self, component);
}

INLINE void psy_ui_aligner_preferred_size(psy_ui_Aligner* self,
	struct psy_ui_Component* component, const psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(self);

	self->vtable->preferredsize(self, component, limit, rv);
}

/* helper functions */

void psy_ui_aligner_adjust_minmax_size(
	struct psy_ui_Component*, const psy_ui_TextMetric*,
	psy_ui_Size*, psy_ui_Size* parentsize);
void psy_ui_aligner_add_border(struct psy_ui_Component*, psy_ui_Size* rv);
void psy_ui_aligner_add_spacing_and_border(struct psy_ui_Component*,
	psy_ui_Size* rv);
void psy_ui_aligner_adjust_spacing(struct psy_ui_Component*,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright);
void psy_ui_aligner_adjust_border(struct psy_ui_Component*,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright);
void psy_ui_aligner_adjust_margin_size(struct psy_ui_Component* component,
	const psy_ui_Size* parentsize,
	psy_ui_Size* rv);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_ALIGNER_H */
