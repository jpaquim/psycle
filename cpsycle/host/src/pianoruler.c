/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianoruler.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* prototypes */
static void pianoruler_ondraw(PianoRuler*, psy_ui_Graphics*);
static psy_audio_BlockSelection pianoruler_clipselection(PianoRuler*,
	psy_ui_RealRectangle clip);
static void pianoruler_drawruler(PianoRuler*, psy_ui_Graphics*, psy_audio_BlockSelection);
void pianoruler_drawbeat(PianoRuler*, psy_ui_Graphics*, intptr_t beat, double x,
	double baseline, double tmheight);
static void pianoruler_onpreferredsize(PianoRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
/* vtable */
static psy_ui_ComponentVtable pianoruler_vtable;
static bool pianoruler_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoruler_vtable_init(PianoRuler* self)
{
	assert(self);

	if (!pianoruler_vtable_initialized) {
		pianoruler_vtable = *(self->component.vtable);
		pianoruler_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pianoruler_ondraw;
		pianoruler_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			pianoruler_onpreferredsize;
		pianoruler_vtable_initialized = TRUE;
	}
	return &pianoruler_vtable;
}

void pianoruler_init(PianoRuler* self, psy_ui_Component* parent,
	PianoGridState* gridstate)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(pianoruler_base(self),
		pianoruler_vtable_init(self));
	psy_ui_component_doublebuffer(pianoruler_base(self));
	pianoruler_setsharedgridstate(self, gridstate);	
}

void pianoruler_setsharedgridstate(PianoRuler* self, PianoGridState* gridstate)
{
	assert(self);

	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		pianogridstate_init(&self->defaultgridstate, NULL, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianoruler_onpreferredsize(PianoRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	assert(self);

	rv->width = (patternviewstate_pattern(&self->gridstate->pv))
		? psy_ui_value_make_px(pianogridstate_beattopx(self->gridstate,
		  psy_audio_pattern_length(patternviewstate_pattern(&self->gridstate->pv))))
		: psy_ui_value_make_px(0);
	rv->height = psy_ui_value_make_eh(1.0);
}

void pianoruler_ondraw(PianoRuler* self, psy_ui_Graphics* g)
{	
	assert(self);

	pianoruler_drawruler(self, g, pianoruler_clipselection(self, g->clip));
}

psy_audio_BlockSelection pianoruler_clipselection(PianoRuler* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_BlockSelection rv;

	assert(self);

	rv.topleft.cursor.offset = pianogridstate_quantize(self->gridstate,
		pianogridstate_pxtobeat(self->gridstate, clip.left));
	if (patternviewstate_pattern(&self->gridstate->pv)) {
		rv.bottomright.cursor.offset = psy_min(
			psy_audio_pattern_length(patternviewstate_pattern(&self->gridstate->pv)),
			pianogridstate_pxtobeat(self->gridstate, clip.right));
	} else {
		rv.bottomright.cursor.offset = 0;
	}
	return rv;
}

void pianoruler_drawruler(PianoRuler* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{	
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	double baselinetop;
	double baseline;
	intptr_t currstep;
	psy_dsp_big_beat_t c;
	double scrollleft;	
	
	assert(self);

	if (pianogridstate_step(self->gridstate) == 0) {
		return;
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scrollsize_px(&self->component);	
	baseline = size.height - 1;
	baselinetop = baseline - tm->tmHeight / 3;
	scrollleft = psy_ui_component_scrollleftpx(&self->component);
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->pv.skin,
		0, 0));
	psy_ui_drawline(g, psy_ui_realpoint_make(scrollleft, baseline),
		psy_ui_realpoint_make(size.width + scrollleft, baseline));
	currstep = pianogridstate_beattosteps(self->gridstate, clip.topleft.cursor.offset);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (c = clip.topleft.cursor.offset; c <= clip.bottomright.cursor.offset;
			c += pianogridstate_step(self->gridstate), ++currstep) {
		double cpx;
		
		cpx = pianogridstate_beattopx(self->gridstate, c);
		psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
			psy_ui_realpoint_make(cpx, baselinetop));
		if ((currstep % self->gridstate->pv.cursor.cursor.lpb) == 0) {
			pianoruler_drawbeat(self, g, (intptr_t)(c), cpx, baseline, tm->tmHeight);
		}
	}
}

void pianoruler_drawbeat(PianoRuler* self, psy_ui_Graphics* g, intptr_t beat,
	double x, double baseline, double tmheight)
{	
	char txt[40];

	assert(self);

	if ((beat % 4) == 0) {
		psy_ui_settextcolour(g, patternviewskin_row4beatcolour(self->gridstate->pv.skin,
			0, 0));
	} else {
		psy_ui_settextcolour(g, patternviewskin_rowbeatcolour(self->gridstate->pv.skin,
			0, 0));
	}
	psy_ui_drawline(g, psy_ui_realpoint_make(x, baseline),
		psy_ui_realpoint_make(x, baseline - tmheight / 2));
	psy_snprintf(txt, 40, "%d", beat);
	psy_ui_textout(g, x + 3, baseline - tmheight, txt, psy_strlen(txt));
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->pv.skin,
		0, 0));
}
