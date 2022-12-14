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
static void pianoruler_drawruler(PianoRuler*, psy_ui_Graphics*,
	psy_audio_BlockSelection);
static void pianoruler_drawbeat(PianoRuler*, psy_ui_Graphics*, intptr_t beat,
	double x, double baseline, double tmheight);
static void pianoruler_on_preferred_size(PianoRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

/* vtable */
static psy_ui_ComponentVtable pianoruler_vtable;
static bool pianoruler_vtable_initialized = FALSE;

static void pianoruler_vtable_init(PianoRuler* self)
{
	assert(self);

	if (!pianoruler_vtable_initialized) {
		pianoruler_vtable = *(self->component.vtable);
		pianoruler_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pianoruler_ondraw;
		pianoruler_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			pianoruler_on_preferred_size;
		pianoruler_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pianoruler_base(self), &pianoruler_vtable);
}

void pianoruler_init(PianoRuler* self, psy_ui_Component* parent,
	PianoGridState* gridstate)
{
	assert(self);
	assert(gridstate);

	psy_ui_component_init(&self->component, parent, NULL);	
	pianoruler_vtable_init(self);
	self->gridstate = gridstate;
}

void pianoruler_on_preferred_size(PianoRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	assert(self);

	rv->width = (patternviewstate_pattern(self->gridstate->pv))
		? psy_ui_value_make_px(pianogridstate_beat_to_px(self->gridstate,
			patternviewstate_length(self->gridstate->pv)))
		: psy_ui_value_make_px(0.0);
	rv->height = psy_ui_value_make_eh(2.0);
}

void pianoruler_ondraw(PianoRuler* self, psy_ui_Graphics* g)
{	
	assert(self);

	pianoruler_drawruler(self, g, pianoruler_clipselection(self,
		psy_ui_graphics_cliprect(g)));
}

psy_audio_BlockSelection pianoruler_clipselection(PianoRuler* self,
	psy_ui_RealRectangle clip)
{
	psy_audio_BlockSelection rv;

	assert(self);

	pianogridstate_clip(self->gridstate, clip.left, clip.right,
		&rv.topleft.offset, &rv.bottomright.offset);
	return rv;
}

void pianoruler_drawruler(PianoRuler* self, psy_ui_Graphics* g,
	psy_audio_BlockSelection clip)
{	
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	double baselinetop;
	double baseline;
	intptr_t line;
	psy_dsp_big_beat_t c;
	double scrollleft;	
	psy_ui_Style* style;
	psy_ui_Colour overlaycolour;
	psy_ui_Colour linecolour;
	psy_ui_Colour linebeatcolour;
	psy_ui_Colour linebeat4colour;
	
	assert(self);

	if (pianogridstate_step(self->gridstate) == 0) {
		return;
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scroll_size_px(&self->component);	
	baseline = size.height - tm->tmHeight + 2;
	
	//size.height - 1;
	baselinetop = baseline + tm->tmHeight / 3;
	scrollleft = psy_ui_component_scroll_left_px(&self->component);
	style = psy_ui_style(STYLE_PV_ROWBEAT);
	psy_ui_setcolour(g, style->background.colour);
	// patternviewskin_rowbeatcolour(patternviewstate_skin(self->gridstate->pv),
	//	0, 0));
	linecolour = psy_ui_style(STYLE_SEQEDT_RULER_BOTTOM)->colour;
	if (psy_ui_app_hasdarktheme(psy_ui_app())) {
		overlaycolour = psy_ui_colour_white();
	} else {
		overlaycolour = psy_ui_colour_black();
	}
	linebeatcolour = psy_ui_colour_overlayed(&linecolour, &overlaycolour, 0.10);
	linebeat4colour = psy_ui_colour_overlayed(&linecolour, &overlaycolour, 0.20);
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(), psy_ui_realsize_make(size.width, baseline)),
		psy_ui_style(STYLE_SEQEDT_RULER_TOP)->background.colour);	
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, baseline),
		psy_ui_realsize_make(size.width, size.height - baseline)),
		psy_ui_style(STYLE_SEQEDT_RULER_BOTTOM)->background.colour);
	psy_ui_setcolour(g, linecolour);	
	psy_ui_drawline(g, psy_ui_realpoint_make(scrollleft, baseline),
		psy_ui_realpoint_make(size.width + scrollleft, baseline));
	c = patternviewstate_draw_offset(self->gridstate->pv,
		clip.topleft.offset);
	line = pianogridstate_beat_to_line(self->gridstate, c);		
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (; c <= clip.bottomright.offset;
			c += pianogridstate_step(self->gridstate), ++line) {
		double cpx;
		
		cpx = pianogridstate_beat_to_px(self->gridstate, c);
		if (c > clip.bottomright.offset) {
			break;
		}
		psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
			psy_ui_realpoint_make(cpx, baselinetop));
		if ((line % self->gridstate->pv->cursor.lpb) == 0) {
			pianoruler_drawbeat(self, g, (intptr_t)(c), cpx, baseline,
				tm->tmHeight);
		}
	}
}

void pianoruler_drawbeat(PianoRuler* self, psy_ui_Graphics* g, intptr_t beat,
	double x, double baseline, double tmheight)
{	
	char txt[40];
	psy_ui_Style* style;

	assert(self);
	
	if ((beat % 4) == 0) {
		style = psy_ui_style(STYLE_PV_ROW4BEAT);
		psy_ui_set_text_colour(g, style->background.colour);
		// patternviewskin_row4beatcolour(patternviewstate_skin(self->gridstate->pv),
		//	0, 0));
	} else {
		style = psy_ui_style(STYLE_PV_ROWBEAT);
		psy_ui_set_text_colour(g, style->background.colour);
		// psy_ui_set_text_colour(g, patternviewskin_rowbeatcolour(patternviewstate_skin(self->gridstate->pv),
		//	0, 0));
	}
	psy_ui_drawline(g, psy_ui_realpoint_make(x, baseline),
		psy_ui_realpoint_make(x, baseline + tmheight / 2));
	psy_snprintf(txt, 40, "%d", beat);
	psy_ui_textout(g, psy_ui_realpoint_make(x + 3, baseline - tmheight), txt, psy_strlen(txt));
	style = psy_ui_style(STYLE_PV_ROWBEAT);
	psy_ui_set_text_colour(g, style->background.colour);
	// psy_ui_setcolour(g, patternviewskin_rowbeatcolour(patternviewstate_skin(self->gridstate->pv),
	//	0, 0));
}
