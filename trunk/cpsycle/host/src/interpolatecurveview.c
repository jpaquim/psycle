/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "interpolatecurveview.h"
/* std */
#include <math.h>
#include <stdlib.h>
/* platform */
#include "../../detail/portable.h"

static void interpolatecurvebox_on_destroyed(InterpolateCurveBox*);
static void interpolatecurvebox_ondraw(InterpolateCurveBox*, psy_ui_Graphics*);
static void interpolatecurvebox_drawgrid(InterpolateCurveBox*,
	psy_ui_Graphics*);
static void interpolatecurvebox_drawkeyframes(InterpolateCurveBox*,
	psy_ui_Graphics*);
static void interpolatecurvebox_drawselector(InterpolateCurveBox*,
	psy_ui_Graphics*, double x, double y, psy_List* kf);
static void interpolatecurvebox_buildkeyframes(InterpolateCurveBox*);
static void interpolatecurvebox_on_mouse_down(InterpolateCurveBox*, psy_ui_MouseEvent*);
static void interpolatecurvebox_onmousemove(InterpolateCurveBox*, psy_ui_MouseEvent*);
static void interpolatecurvebox_on_mouse_up(InterpolateCurveBox*, psy_ui_MouseEvent*);
static psy_List* interpolatecurvebox_hittest(InterpolateCurveBox*, double x, double y);
static void interpolatecurvebox_clear(InterpolateCurveBox*);
static void interpolatecurvebox_insertkeyframe(InterpolateCurveBox* self, double x, double y);

static void interpolatecurveview_on_destroy(InterpolateCurveView*,
	psy_ui_Component* sender);
static void interpolatecurveview_oninterpolate(InterpolateCurveView*,
	psy_ui_Component* sender);
static void interpolatecurveview_oncancel(InterpolateCurveView*,
	psy_ui_Component* sender);
static void interpolatecurveview_updatecurvetype(InterpolateCurveView*);
static void interpolatecurveview_oncurvetypechanged(InterpolateCurveView*, psy_ui_Component* sender,
	int index);
static void interpolatecurveview_setallcurvelinear(InterpolateCurveView*);
static void interpolatecurveview_setallcurvehermite(InterpolateCurveView*);

static double hermitecurveinterpolate(intptr_t kf0, intptr_t kf1, intptr_t kf2,
	intptr_t kf3, intptr_t curposition, intptr_t maxposition, double tangmult, bool interpolation);

KeyFrame* keyframe_alloc(void)
{
	return (KeyFrame*) malloc(sizeof(KeyFrame));
}

KeyFrame* keyframe_allocinit(psy_dsp_big_beat_t offset, intptr_t value,
	InterpolateCurveType curve)
{
	KeyFrame* rv;

	rv = keyframe_alloc();
	if (rv) {
		keyframe_init(rv, offset, value, curve);
	}
	return rv;
}

void interpolatecurvebar_init(InterpolateCurveBar* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
	psy_ui_combobox_init(&self->curvetype, &self->component);
	psy_ui_component_set_align(&self->curvetype.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_add_text(&self->curvetype, "Linear");
	psy_ui_combobox_add_text(&self->curvetype, "Hermite Curve");
	psy_ui_combobox_add_text(&self->curvetype, "All to Linear");
	psy_ui_combobox_add_text(&self->curvetype, "All to Hermite");
	psy_ui_combobox_select(&self->curvetype, 0);
	psy_ui_combobox_set_char_number(&self->curvetype, 15);
	psy_ui_button_init(&self->cancel, &self->component);
	psy_ui_button_set_text(&self->cancel, "Cancel");
	psy_ui_component_set_align(&self->cancel.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->ok, &self->component);
	psy_ui_button_set_text(&self->ok, "Interpolate");
	psy_ui_component_set_align(&self->ok.component, psy_ui_ALIGN_RIGHT);				
}

static psy_ui_ComponentVtable interpolatecurvebox_vtable;
static bool interpolatecurvebox_vtable_initialized = FALSE;

static void interpolatecurvebox_vtable_init(InterpolateCurveBox* self)
{
	if (!interpolatecurvebox_vtable_initialized) {
		interpolatecurvebox_vtable = *(self->component.vtable);
		interpolatecurvebox_vtable.on_destroyed =
			(psy_ui_fp_component)
			interpolatecurvebox_on_destroyed;
		interpolatecurvebox_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			interpolatecurvebox_ondraw;
		interpolatecurvebox_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			interpolatecurvebox_on_mouse_down;
		interpolatecurvebox_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			interpolatecurvebox_onmousemove;
		interpolatecurvebox_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			interpolatecurvebox_on_mouse_up;		
		interpolatecurvebox_vtable_initialized = TRUE;
	}
}

void interpolatecurvebox_init(InterpolateCurveBox* self,
	psy_ui_Component* parent, InterpolateCurveView* view,
	PatternViewState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	interpolatecurvebox_vtable_init(self);
	self->view = view;
	self->state = state;
	self->component.vtable = &interpolatecurvebox_vtable;
	self->keyframes = 0;
	self->range = (psy_dsp_amp_t)0.f;
	self->valuerange = 0xFF;
	self->minval = 0;
	self->maxval = 0xFF;	
	self->dragkeyframe = 0;
	self->selected = 0;
	self->bpl = 0.25;	
	psy_ui_component_set_aligner(&self->component, NULL);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(0.0, 10.0));
}

void interpolatecurvebox_on_destroyed(InterpolateCurveBox* self)
{
	interpolatecurvebox_clear(self);
}

void interpolatecurvebox_ondraw(InterpolateCurveBox* self,
	psy_ui_Graphics* g)
{
	interpolatecurvebox_drawgrid(self, g);
	interpolatecurvebox_drawkeyframes(self, g);
}

void interpolatecurvebox_drawgrid(InterpolateCurveBox* self,
	psy_ui_Graphics* g)
{	
	if (self->range) {
		uintptr_t lines;
		psy_ui_RealSize size;
		double scalex;
		uintptr_t i;
		
		size = psy_ui_component_scroll_size_px(&self->component);
		lines = (uintptr_t)(self->range / self->bpl);
		scalex = size.width / self->range;
		psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
		for (i = 0; i < lines; ++i) {
			double x;

			x = i * self->bpl * scalex;
			psy_ui_drawline(g, psy_ui_realpoint_make(x, 0),
				psy_ui_realpoint_make(x, size.height));
		}
	}
}

void interpolatecurvebox_drawkeyframes(InterpolateCurveBox* self,
	psy_ui_Graphics* g)
{
	psy_List* kf;
	psy_dsp_big_beat_t lastoffset = 0;
	intptr_t step = 100;
	uintptr_t lines;
	psy_ui_RealSize size;
	double scalex;
	double scaley;
	intptr_t val0 = 0;
	intptr_t val1 = 0;
	intptr_t val2 = 0;
	intptr_t val3 = 0;
	double lastcurveval = 0;
	InterpolateCurveType curve;
	KeyFrame* entry;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	if (!self->keyframes) {
		return;
	}
	entry = (KeyFrame*)self->keyframes->entry;
	val0 = entry->value;
	val1 = entry->value;
	val2 = entry->value;
	val3 = entry->value;
	lastcurveval = (double)entry->value;
	lastoffset = entry->offset;
	size = psy_ui_component_scroll_size_px(&self->component);
	lines = (uintptr_t)(self->range / self->bpl);
	scalex = size.width / self->range;
	scaley = size.height / 0xFF;
	psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
	interpolatecurvebox_drawselector(self, g, lastoffset * scalex,
		size.height - (lastcurveval * scaley), self->keyframes);
	curve = entry->curve;
	for (kf = self->keyframes->next; kf != 0; kf = kf->next) {
		KeyFrame* entry;
		double distance;
		double x;

		entry = (KeyFrame*)kf->entry;				
		val0 = val1;
		val1 = val2;
		val2 = entry->value;
		val3 = entry->value;
		x = lastoffset * scalex;
		distance = (entry->offset * scalex) - (lastoffset * scalex);
		if (curve == INTERPOLATECURVETYPE_LINEAR) {
			psy_ui_drawline(g,
				psy_ui_realpoint_make(x, size.height - lastcurveval * scaley),
				psy_ui_realpoint_make(x + distance, size.height - entry->value * scaley));
		} else
		if (curve == INTERPOLATECURVETYPE_HERMITE) {
			intptr_t i;

			for (i = 1; i < distance; i++) {
				double curveval = hermitecurveinterpolate(val0, val1, val2, val3, i,
					(intptr_t)distance, 0, TRUE);
				psy_ui_drawline(g,
					psy_ui_realpoint_make(x + i - 1, size.height - lastcurveval * scaley),
					psy_ui_realpoint_make(x + i, size.height - curveval * scaley));
				lastcurveval = curveval;
			}
		}
		lastcurveval = (double)val2;
		lastoffset = entry->offset;
		curve = entry->curve;
		interpolatecurvebox_drawselector(self, g, lastoffset * scalex,
			size.height - (lastcurveval * scaley), kf);
	 }
}

double hermitecurveinterpolate(intptr_t kf0, intptr_t kf1, intptr_t kf2,
	intptr_t kf3, intptr_t curposition, intptr_t maxposition, double tangmult, bool interpolation)
{
	if (interpolation == TRUE)
	{
		double s = (double)curposition / (double)maxposition;
		double pws3 = pow(s, 3);
		double pws2 = pow(s, 2);
		double pws23 = 3 * pws2;
		double h1 = (2 * pws3) - pws23 + 1;
		double h2 = (-2 * pws3) + pws23;
		double h3 = pws3 - (2 * pws2) + s;
		double h4 = pws3 - pws2;

		double t1 = tangmult * (kf2 - kf0);
		double t2 = tangmult * (kf3 - kf1);

		return (h1 * kf1 + h2 * kf2 + h3 * t1 + h4 * t2);
	}
	else
	{
		return (double)kf1;
	}
}

void interpolatecurvebox_drawselector(InterpolateCurveBox* self,
	psy_ui_Graphics* g, double x, double y, psy_List* keyframe)
{
	psy_ui_RealRectangle r;
	double half = 2;
	psy_ui_Colour colour;

	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(x - half, y - half),
		psy_ui_realsize_make(half * 2, half * 2));
	if (self->selected == keyframe) {
		colour = psy_ui_colour_make(0x000000FF);
	} else {
		colour = psy_ui_colour_make(0x00FFFFFF);
	}
	psy_ui_drawsolidrectangle(g, r, colour);
}

void interpolatecurvebox_on_mouse_down(InterpolateCurveBox* self, psy_ui_MouseEvent* ev)
{
	psy_List* selected;
	
	self->dragkeyframe = 0;
	selected = interpolatecurvebox_hittest(self, psy_ui_mouseevent_pt(ev).x,
		psy_ui_mouseevent_pt(ev).y);
	if (psy_ui_mouseevent_button(ev) == 1) {
		if (selected) {
			self->dragkeyframe = selected;
			self->selected = selected;
			interpolatecurveview_updatecurvetype(self->view);
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_capture(&self->component);
		} else {
			interpolatecurvebox_insertkeyframe(self, psy_ui_mouseevent_pt(ev).x,
				psy_ui_mouseevent_pt(ev).y);
		}
	} else if (psy_ui_mouseevent_button(ev) == 2) {
		if (selected) {
			if (self->selected == selected) {
				self->selected = 0;
			}
			psy_list_remove(&self->keyframes, selected);
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void interpolatecurvebox_insertkeyframe(InterpolateCurveBox* self, double x, double y)
{
	psy_ui_Size size;
	double scalex;
	double scaley;
	KeyFrame* keyframe;
	psy_List* kf;
	psy_dsp_big_amp_t value;
	psy_dsp_big_beat_t offset;
	const psy_ui_TextMetric* tm;
	
	size = psy_ui_component_scroll_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	scalex = psy_ui_value_px(&size.width, tm, NULL) / self->range;
	scaley = psy_ui_value_px(&size.height, tm, NULL) / (double)0xFF;

	offset = x / scalex;
	offset = (intptr_t)(offset / 0.25) * 0.25;
	value = (psy_dsp_big_amp_t)(psy_ui_value_px(&size.height, tm, NULL) - y) / scaley;
	
	kf = self->keyframes;
	while (kf != 0) {
		KeyFrame* entry;

		entry = (KeyFrame*) kf->entry;
		if (entry->offset > offset) {
			break;
		}
		if (entry->offset == offset) {
			entry->value = (intptr_t) value;
			return;
		}
		kf = kf->next;
	}
	keyframe = keyframe_allocinit(
		offset,
		(intptr_t) value,
		INTERPOLATECURVETYPE_LINEAR);
	if (kf) {
		psy_list_insert(&self->keyframes, kf->prev, keyframe);
	} else {
		psy_list_append(&self->keyframes, keyframe);
	}
	psy_ui_component_invalidate(&self->component);
}

void interpolatecurveview_updatecurvetype(InterpolateCurveView* self)
{
	if (self->box.selected) {
		KeyFrame* keyframe;

		keyframe = (KeyFrame*)self->box.selected->entry;
		if (keyframe->curve == INTERPOLATECURVETYPE_LINEAR) {
			psy_ui_combobox_select(&self->bar.curvetype, 0);
		} else
		if (keyframe->curve == INTERPOLATECURVETYPE_HERMITE) {
			psy_ui_combobox_select(&self->bar.curvetype, 1);
		}
	}
}

void interpolatecurvebox_onmousemove(InterpolateCurveBox* self, psy_ui_MouseEvent* ev)
{
	if (self->dragkeyframe) {
		psy_ui_Size size;
		KeyFrame* entry;
		double scaley;
		const psy_ui_TextMetric* tm;

		size = psy_ui_component_scroll_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		scaley = psy_ui_value_px(&size.height, tm, NULL) / (double)0xFF;
		entry = (KeyFrame*)self->dragkeyframe->entry;
		entry->value = (intptr_t)((psy_ui_value_px(&size.height, tm, NULL) -
			psy_ui_mouseevent_pt(ev).y) * 1 / scaley);
		if (entry->value < self->minval) {
			entry->value = self->minval;
		} else 
		if (entry->value > self->maxval) {
			entry->value = self->maxval;
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void interpolatecurvebox_on_mouse_up(InterpolateCurveBox* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_release_capture(&self->component);
	self->dragkeyframe = 0;
}

psy_List* interpolatecurvebox_hittest(InterpolateCurveBox* self, double x, double y)
{
	psy_List* kf;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	double scalex;
	double scaley;
	intptr_t half = 4;

	size = psy_ui_component_scroll_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	scalex = psy_ui_value_px(&size.width, tm, NULL) / self->range;
	scaley = psy_ui_value_px(&size.height, tm, NULL) / (double)0xFF;
	kf = self->keyframes;
	while (kf != 0) {
		KeyFrame* entry;

		entry = (KeyFrame*)kf->entry;
		if ((intptr_t)(entry->offset * scalex) >= x - half && (intptr_t)(entry->offset * scalex) < x + half) {
			break;
		}
		kf = kf->next;
	}
	return kf;
}

void interpolatecurvebox_clear(InterpolateCurveBox* self)
{
	psy_List* kf;

	for (kf = self->keyframes; kf != 0; kf = kf->next) {
		KeyFrame* entry;

		entry = (KeyFrame*)kf->entry;
		free(entry);
	}
	psy_list_free(self->keyframes);
	self->keyframes = 0;
}

/* InterpolateCurveView */

/* prototypes */
static void interpolatecurveview_on_destroyed(InterpolateCurveView*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(InterpolateCurveView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			interpolatecurveview_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void interpolatecurveview_init(InterpolateCurveView* self, psy_ui_Component*
	parent, intptr_t startsel, intptr_t endsel, uintptr_t lpb, PatternViewState* state,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->state = state;
	interpolatecurvebar_init(&self->bar, &self->component);
	psy_ui_component_set_align(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	interpolatecurvebox_init(&self->box, &self->component, self, state,
		workspace);
	psy_ui_component_set_align(&self->box.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_init(&self->signal_cancel);	
	psy_signal_connect(&self->bar.cancel.signal_clicked, self,
		interpolatecurveview_oncancel);
	psy_signal_connect(&self->bar.ok.signal_clicked, self,
		interpolatecurveview_oninterpolate);
	psy_signal_connect(&self->bar.curvetype.signal_selchanged, self,
		interpolatecurveview_oncurvetypechanged);	
}

void interpolatecurveview_on_destroyed(InterpolateCurveView* self)
{
	psy_signal_dispose(&self->signal_cancel);
}

void interpolatecurveview_set_selection(InterpolateCurveView* self,
	const psy_audio_BlockSelection* selection)
{
	assert(selection);

	/* todo abs */
	self->box.range = selection->bottomright.offset -
		selection->topleft.offset;
	self->box.selection = *selection;
	interpolatecurvebox_buildkeyframes(&self->box);
	if (self->box.keyframes) {
		KeyFrame* keyframe;

		keyframe = (KeyFrame*)self->box.keyframes;
		self->box.selected = self->box.keyframes;
		if (keyframe->curve == INTERPOLATECURVETYPE_LINEAR) {
			psy_ui_combobox_select(&self->bar.curvetype, 0);
		} else
		if (keyframe->curve == INTERPOLATECURVETYPE_HERMITE) {
			psy_ui_combobox_select(&self->bar.curvetype, 1);
		}
	} else {
		self->box.selected = 0;
		psy_ui_combobox_select(&self->bar.curvetype, 0);
	}
	psy_ui_component_invalidate(&self->box.component);
}

void interpolatecurvebox_buildkeyframes(InterpolateCurveBox* self)
{	
	psy_audio_Pattern* pattern;

	interpolatecurvebox_clear(self);
	pattern = patternviewstate_pattern(self->state);
	if (pattern) {
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		intptr_t insertlast = 1;

		/* todo abs */
		p = psy_audio_pattern_greaterequal(pattern,
			(psy_dsp_beat_t)self->selection.topleft.offset);
		if (p) {
			psy_audio_PatternEntry* entry;

			entry = psy_audio_patternnode_entry(p);
			if (entry->offset > self->selection.topleft.offset) {
				KeyFrame* keyframe;

				keyframe = keyframe_allocinit(0.f, 0,
					INTERPOLATECURVETYPE_LINEAR);
				if (keyframe) {
					psy_list_append(&self->keyframes, keyframe);
				}
			}
		}
		while (p != NULL) {
			psy_audio_PatternEntry* entry;
			q = p->next;

			entry = psy_audio_patternnode_entry(p);
			if (entry->offset < self->selection.bottomright.offset) {
				if (entry->track == self->selection.topleft.track) {
					KeyFrame* keyframe;

					keyframe = keyframe_allocinit(
						entry->offset - self->selection.topleft.offset,
						psy_audio_patternentry_front(entry)->parameter,
						INTERPOLATECURVETYPE_LINEAR);
					if (keyframe) {
						psy_list_append(&self->keyframes, keyframe);
						if (entry->offset == self->selection.bottomright.offset - 0.25f) {
							insertlast = 0;
							break;
						}
					}
				}
			} else {
				break;
			}
			p = q;
		}
		if (insertlast) {
			KeyFrame* keyframe;

			/* todo abs */
			keyframe = keyframe_allocinit(
				self->selection.bottomright.offset - self->selection.topleft.offset - 0.25f,
				0,
				INTERPOLATECURVETYPE_LINEAR);
			if (keyframe) {
				psy_list_append(&self->keyframes, keyframe);
			}
		}
	}	
}

void interpolatecurveview_oninterpolate(InterpolateCurveView* self,
	psy_ui_Component* sender)
{
	psy_audio_Pattern* pattern;

	interpolatecurvebox_clear(&self->box);
	pattern = patternviewstate_pattern(self->state);
	if (!pattern) {
		return;
	}
	if (self->box.keyframes && self->box.keyframes->next) {
		psy_List* kf;
		psy_dsp_big_beat_t lastoffset;
		intptr_t lastvalue;
		KeyFrame* entry;
		psy_dsp_beat_t bpl = 0.25f;
		KeyFrame* lastentry;

		entry = (KeyFrame*) self->box.keyframes->entry;
		lastentry = entry;
		/* todo abs */
		lastoffset = entry->offset + self->box.selection.topleft.offset;
		lastvalue = entry->value;
		for (kf = self->box.keyframes->next; kf != 0; kf = kf->next) {
			KeyFrame* entry;
			psy_audio_SequenceCursor start;
			psy_audio_SequenceCursor end;

			/* todo abs */
			psy_audio_sequencecursor_init(&start);
			psy_audio_sequencecursor_init(&end);
			/* todo abs */
			start.offset = lastoffset;
			entry = (KeyFrame*)kf->entry;
			/* todo abs */
			end.offset = (entry->offset + self->box.selection.topleft.offset);
			if (lastentry->curve == INTERPOLATECURVETYPE_LINEAR) {
				psy_audio_pattern_blockinterpolaterange(pattern,
					start, end,
					bpl, lastvalue, entry->value);
			} else if (lastentry->curve == INTERPOLATECURVETYPE_HERMITE) {
				psy_audio_pattern_blockinterpolaterangehermite(pattern,
					start, end,
					bpl, lastvalue, entry->value);
			}
			lastoffset = entry->offset + self->box.selection.topleft.offset;
			lastvalue = entry->value;
			lastentry = entry;
		}
	}
}

void interpolatecurveview_oncancel(InterpolateCurveView* self,
	psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_cancel, self, 0);
}

void interpolatecurveview_oncurvetypechanged(InterpolateCurveView* self, psy_ui_Component* sender,
	int index)
{
	if (self->box.selected) {
		KeyFrame* entry;

		entry = (KeyFrame*)self->box.selected->entry;
		if (index == 0) {
			entry->curve = INTERPOLATECURVETYPE_LINEAR;
		} else
		if (index == 1) {
			entry->curve = INTERPOLATECURVETYPE_HERMITE;
		}		
		psy_ui_component_invalidate(&self->box.component);
	}
	if (index == 2) {
		interpolatecurveview_setallcurvelinear(self);
	} else
	if (index == 3) {
		interpolatecurveview_setallcurvehermite(self);
	}
}

void interpolatecurveview_setallcurvelinear(InterpolateCurveView * self)
{
	psy_List* kf;

	for (kf = self->box.keyframes; kf != 0; kf = kf->next) {
		KeyFrame* entry;

		entry = (KeyFrame*)kf->entry;
		entry->curve = INTERPOLATECURVETYPE_LINEAR;
	}
	psy_ui_component_invalidate(&self->box.component);
}

void interpolatecurveview_setallcurvehermite(InterpolateCurveView* self)
{
	psy_List* kf;

	for (kf = self->box.keyframes; kf != 0; kf = kf->next) {
		KeyFrame* entry;

		entry = (KeyFrame*)kf->entry;
		entry->curve = INTERPOLATECURVETYPE_HERMITE;
	}
	psy_ui_component_invalidate(&self->box.component);
}
