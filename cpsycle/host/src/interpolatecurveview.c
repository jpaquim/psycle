// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "interpolatecurveview.h"

#include <math.h>
#include <stdlib.h>

#include "../../detail/portable.h"

static void interpolatecurvebox_ondestroy(InterpolateCurveBox*,
	psy_ui_Component* sender);
static void interpolatecurvebox_ondraw(InterpolateCurveBox*, psy_ui_Graphics*);
static void interpolatecurvebox_drawgrid(InterpolateCurveBox*,
	psy_ui_Graphics*);
static void interpolatecurvebox_drawkeyframes(InterpolateCurveBox*,
	psy_ui_Graphics*);
static void interpolatecurvebox_drawselector(InterpolateCurveBox*,
	psy_ui_Graphics*, int x, int y, psy_List* kf);
static void interpolatecurvebox_buildkeyframes(InterpolateCurveBox*);
static void interpolatecurvebox_onmousedown(InterpolateCurveBox*, psy_ui_MouseEvent*);
static void interpolatecurvebox_onmousemove(InterpolateCurveBox*, psy_ui_MouseEvent*);
static void interpolatecurvebox_onmouseup(InterpolateCurveBox*, psy_ui_MouseEvent*);
static psy_List* interpolatecurvebox_hittest(InterpolateCurveBox*, int x, int y);
static void interpolatecurvebox_clear(InterpolateCurveBox*);
static void interpolatecurvebox_insertkeyframe(InterpolateCurveBox* self, int x, int y);

static void interpolatecurveview_ondestroy(InterpolateCurveView*,
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

static double hermitecurveinterpolate(int kf0, int kf1, int kf2,
	int kf3, int curposition, int maxposition, double tangmult, bool interpolation);

KeyFrame* keyframe_alloc(void)
{
	return (KeyFrame*) malloc(sizeof(KeyFrame));
}

KeyFrame* keyframe_allocinit(psy_dsp_big_beat_t offset, int value,
	InterpolateCurveType curve)
{
	KeyFrame* rv;

	rv = keyframe_alloc();
	if (rv) {
		keyframe_init(rv, offset, value, curve);
	}
	return rv;
}

void interpolatecurvebar_init(InterpolateCurveBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_combobox_init(&self->curvetype, &self->component);
	psy_ui_component_setalign(&self->curvetype.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_addtext(&self->curvetype, "Linear");
	psy_ui_combobox_addtext(&self->curvetype, "Hermite Curve");
	psy_ui_combobox_addtext(&self->curvetype, "All to Linear");
	psy_ui_combobox_addtext(&self->curvetype, "All to Hermite");
	psy_ui_combobox_setcursel(&self->curvetype, 0);
	psy_ui_combobox_setcharnumber(&self->curvetype, 15);
	psy_ui_button_init(&self->cancel, &self->component);
	psy_ui_button_settext(&self->cancel, "Cancel");
	psy_ui_component_setalign(&self->cancel.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->ok, &self->component);
	psy_ui_button_settext(&self->ok, "Interpolate");
	psy_ui_component_setalign(&self->ok.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setmargin(
		psy_ui_component_children(&self->component, 0),
		&margin));	
}

static psy_ui_ComponentVtable interpolatecurvebox_vtable;
static int interpolatecurvebox_vtable_initialized = 0;

static void interpolatecurvebox_vtable_init(InterpolateCurveBox* self)
{
	if (!interpolatecurvebox_vtable_initialized) {
		interpolatecurvebox_vtable = *(self->component.vtable);
		interpolatecurvebox_vtable.ondraw =
			(psy_ui_fp_ondraw) interpolatecurvebox_ondraw;
		interpolatecurvebox_vtable.onmousedown = (psy_ui_fp_onmousedown)
			interpolatecurvebox_onmousedown;
		interpolatecurvebox_vtable.onmousemove = (psy_ui_fp_onmousemove)
			interpolatecurvebox_onmousemove;
		interpolatecurvebox_vtable.onmouseup = (psy_ui_fp_onmouseup)
			interpolatecurvebox_onmouseup;		
		interpolatecurvebox_vtable_initialized = 1;
	}
}

void interpolatecurvebox_init(InterpolateCurveBox* self,
	psy_ui_Component* parent, InterpolateCurveView* view,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);	
	interpolatecurvebox_vtable_init(self);
	self->view = view;
	self->component.vtable = &interpolatecurvebox_vtable;
	self->keyframes = 0;
	self->range = (psy_dsp_amp_t)0.f;
	self->valuerange = 0xFF;
	self->minval = 0;
	self->maxval = 0xFF;
	self->pattern = 0;
	self->dragkeyframe = 0;
	self->selected = 0;
	psy_signal_connect(&self->component.signal_destroy, self,
		interpolatecurvebox_ondestroy);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makepx(0),
			psy_ui_value_makeeh(10)));			
}

void interpolatecurvebox_ondestroy(InterpolateCurveBox* self,
	psy_ui_Component* sender)
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
		psy_ui_Size size;
		double scalex;
		double i;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size(&self->component);
		lines = (uintptr_t)(self->range / 0.25f);
		scalex = psy_ui_value_px(&size.width, &tm) / self->range;
		psy_ui_setcolor(g, 0x00333333);
		for (i = 0; i < lines; i += 0.25) {
			int x;

			x = (int)(i * scalex);
			psy_ui_drawline(g, x, 0, x, psy_ui_value_px(&size.height, &tm));
		}
	}
}

void interpolatecurvebox_drawkeyframes(InterpolateCurveBox* self,
	psy_ui_Graphics* g)
{
	psy_List* kf;
	psy_dsp_big_beat_t lastoffset = 0;
	int step = 100;
	uintptr_t lines;
	psy_ui_Size size;
	double scalex;
	double scaley;
	int val0 = 0;
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	double lastcurveval = 0;
	InterpolateCurveType curve;
	KeyFrame* entry;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	if (!self->keyframes) {
		return;
	}
	entry = (KeyFrame*)self->keyframes->entry;
	val0 = entry->value;
	val1 = entry->value;
	val2 = entry->value;
	val3 = entry->value;
	lastcurveval = (int) entry->value;
	lastoffset = entry->offset;
	size = psy_ui_component_size(&self->component);
	lines = (uintptr_t)(self->range / 0.25f);
	scalex = psy_ui_value_px(&size.width, &tm) / self->range;
	scaley = psy_ui_value_px(&size.height, &tm) / (double) 0xFF;
	psy_ui_setcolor(g, 0x00B1C8B0);
	interpolatecurvebox_drawselector(self, g, (int)(lastoffset * scalex),
		(int)(psy_ui_value_px(&size.height, &tm) - (int)(lastcurveval * scaley)), self->keyframes);
	curve = entry->curve;
	for (kf = self->keyframes->next; kf != 0; kf = kf->next) {
		KeyFrame* entry;
		int distance;
		int x;

		entry = (KeyFrame*)kf->entry;				
		val0 = val1;
		val1 = val2;
		val2 = entry->value;
		val3 = entry->value;
		x = (int)(lastoffset * scalex);
		distance = (int)(entry->offset * scalex) - (int)(lastoffset * scalex);
		if (curve == INTERPOLATECURVETYPE_LINEAR) {
			psy_ui_drawline(g,
				x,
				(int)(psy_ui_value_px(&size.height, &tm) - lastcurveval * scaley),
				x + distance,
				(int)(psy_ui_value_px(&size.height, &tm) - entry->value * scaley));
		} else
		if (curve == INTERPOLATECURVETYPE_HERMITE) {
			int i;

			for (i = 1; i < distance; i++) {
				double curveval = hermitecurveinterpolate(val0, val1, val2, val3, i, distance, 0, TRUE);
				psy_ui_drawline(g, x + i - 1, (int)(psy_ui_value_px(&size.height, &tm) - lastcurveval * scaley),
					x + i, (int)(psy_ui_value_px(&size.height, &tm) - curveval * scaley));
				lastcurveval = curveval;
			}
		}
		lastcurveval = val2;
		lastoffset = entry->offset;
		curve = entry->curve;
		interpolatecurvebox_drawselector(self, g, (int)(lastoffset * scalex),
			(int)(psy_ui_value_px(&size.height, &tm) - (int)(lastcurveval * scaley)), kf);
	 }
}

double hermitecurveinterpolate(int kf0, int kf1, int kf2,
	int kf3, int curposition, int maxposition, double tangmult, bool interpolation)
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
		return kf1;
	}
}

void interpolatecurvebox_drawselector(InterpolateCurveBox* self,
	psy_ui_Graphics* g, int x, int y, psy_List* keyframe)
{
	psy_ui_Rectangle r;
	int half = 2;
	psy_ui_Color color;

	psy_ui_setrectangle(&r, x - half, y - half, half * 2, half * 2);
	if (self->selected == keyframe) {
		color = 0x000000FF;
	} else {
		color = 0x00FFFFFF;
	}
	psy_ui_drawsolidrectangle(g, r, color);
}

void interpolatecurvebox_onmousedown(InterpolateCurveBox* self, psy_ui_MouseEvent* ev)
{
	psy_List* selected;
	
	self->dragkeyframe = 0;
	selected = interpolatecurvebox_hittest(self, ev->x, ev->y);
	if (ev->button == 1) {
		if (selected) {
			self->dragkeyframe = selected;
			self->selected = selected;
			interpolatecurveview_updatecurvetype(self->view);
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_capture(&self->component);
		} else {
			interpolatecurvebox_insertkeyframe(self, ev->x, ev->y);
		}
	} else
	if (ev->button == 2) {
		if (selected) {
			if (self->selected == selected) {
				self->selected = 0;
			}
			psy_list_remove(&self->keyframes, selected);
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void interpolatecurvebox_insertkeyframe(InterpolateCurveBox* self, int x, int y)
{
	psy_ui_Size size;
	double scalex;
	double scaley;
	KeyFrame* keyframe;
	psy_List* kf;
	psy_dsp_big_amp_t value;
	psy_dsp_big_beat_t offset;
	psy_ui_TextMetric tm;
	
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	scalex = psy_ui_value_px(&size.width, &tm) / self->range;
	scaley = psy_ui_value_px(&size.height, &tm) / (double)0xFF;

	offset = x / scalex;
	offset = (int)(offset / 0.25) * 0.25;
	value = (psy_dsp_big_amp_t)(psy_ui_value_px(&size.height, &tm) - y) / scaley;
	
	kf = self->keyframes;
	while (kf != 0) {
		KeyFrame* entry;

		entry = (KeyFrame*) kf->entry;
		if (entry->offset > offset) {
			break;
		}
		if (entry->offset == offset) {
			entry->value = (int) value;
			return;
		}
		kf = kf->next;
	}
	keyframe = keyframe_allocinit(
		offset,
		(int) value,
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
			psy_ui_combobox_setcursel(&self->bar.curvetype, 0);
		} else
		if (keyframe->curve == INTERPOLATECURVETYPE_HERMITE) {
			psy_ui_combobox_setcursel(&self->bar.curvetype, 1);
		}
	}
}

void interpolatecurvebox_onmousemove(InterpolateCurveBox* self, psy_ui_MouseEvent* ev)
{
	if (self->dragkeyframe) {
		psy_ui_Size size;
		KeyFrame* entry;
		double scaley;
		psy_ui_TextMetric tm;

		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		scaley = psy_ui_value_px(&size.height, &tm) / (double)0xFF;
		entry = (KeyFrame*)self->dragkeyframe->entry;
		entry->value = (intptr_t)((psy_ui_value_px(&size.height, &tm) - ev->y) * 1 / scaley);
		if (entry->value < self->minval) {
			entry->value = self->minval;
		} else 
		if (entry->value > self->maxval) {
			entry->value = self->maxval;
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void interpolatecurvebox_onmouseup(InterpolateCurveBox* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	self->dragkeyframe = 0;
}

psy_List* interpolatecurvebox_hittest(InterpolateCurveBox* self, int x, int y)
{
	psy_List* kf;
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	double scalex;
	double scaley;
	int half = 4;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	scalex = psy_ui_value_px(&size.width, &tm) / self->range;
	scaley = psy_ui_value_px(&size.height, &tm) / (double)0xFF;
	kf = self->keyframes;
	while (kf != 0) {
		KeyFrame* entry;

		entry = (KeyFrame*)kf->entry;
		if ((int)(entry->offset * scalex) >= x - half && (int)(entry->offset * scalex) < x + half) {
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

void interpolatecurveview_init(InterpolateCurveView* self, psy_ui_Component* parent,
	int startsel, int endsel, int lpb, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);	
	interpolatecurvebar_init(&self->bar, &self->component, workspace);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	interpolatecurvebox_init(&self->box, &self->component, self, workspace);
	psy_ui_component_setalign(&self->box.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_init(&self->signal_cancel);
	psy_signal_connect(&self->component.signal_destroy, self,
		interpolatecurveview_ondestroy);
	psy_signal_connect(&self->bar.cancel.signal_clicked, self,
		interpolatecurveview_oncancel);
	psy_signal_connect(&self->bar.ok.signal_clicked, self,
		interpolatecurveview_oninterpolate);
	psy_signal_connect(&self->bar.curvetype.signal_selchanged, self,
		interpolatecurveview_oncurvetypechanged);	
}

void interpolatecurveview_ondestroy(InterpolateCurveView* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_cancel);
}

void interpolatecurveview_setselection(InterpolateCurveView* self,
	PatternSelection selection)
{
	self->box.range = selection.bottomright.offset - selection.topleft.offset;
	self->box.selection = selection;
	interpolatecurvebox_buildkeyframes(&self->box);
	if (self->box.keyframes) {
		KeyFrame* keyframe;

		keyframe = (KeyFrame*)self->box.keyframes;
		self->box.selected = self->box.keyframes;
		if (keyframe->curve == INTERPOLATECURVETYPE_LINEAR) {
			psy_ui_combobox_setcursel(&self->bar.curvetype, 0);
		} else
		if (keyframe->curve == INTERPOLATECURVETYPE_HERMITE) {
			psy_ui_combobox_setcursel(&self->bar.curvetype, 1);
		}
	} else {
		self->box.selected = 0;
		psy_ui_combobox_setcursel(&self->bar.curvetype, 0);
	}
	psy_ui_component_invalidate(&self->box.component);
}

void interpolatecurvebox_buildkeyframes(InterpolateCurveBox* self)
{
	interpolatecurvebox_clear(self);
	if (self->pattern) {
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		int insertlast = 1;

		p = psy_audio_pattern_greaterequal(self->pattern,
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
						patternentry_front(entry)->parameter,
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

void interpolatecurveview_setpattern(InterpolateCurveView* self,
	psy_audio_Pattern* pattern)
{
	self->box.pattern = pattern;
}

void interpolatecurveview_oninterpolate(InterpolateCurveView* self,
	psy_ui_Component* sender)
{
	if (self->box.pattern && self->box.keyframes && self->box.keyframes->next) {
		psy_List* kf;
		psy_dsp_big_beat_t lastoffset;
		intptr_t lastvalue;
		KeyFrame* entry;
		psy_dsp_beat_t bpl = 0.25f;
		KeyFrame* lastentry;

		entry = (KeyFrame*) self->box.keyframes->entry;
		lastentry = entry;
		lastoffset = entry->offset + self->box.selection.topleft.offset;
		lastvalue = entry->value;
		for (kf = self->box.keyframes->next; kf != 0; kf = kf->next) {
			KeyFrame* entry;
			psy_audio_PatternEditPosition start;
			psy_audio_PatternEditPosition end;

			psy_audio_patterneditposition_init(&start);
			psy_audio_patterneditposition_init(&end);
			start.offset = lastoffset;
			start.line = (uintptr_t)(lastoffset / 0.25f);
			entry = (KeyFrame*)kf->entry;
			end.offset = (entry->offset + self->box.selection.topleft.offset);
			end.line = (uintptr_t)(end.offset / bpl) + 1;
			if (lastentry->curve == INTERPOLATECURVETYPE_LINEAR) {
				psy_audio_pattern_blockinterpolaterange(self->box.pattern,
					start, end,
					bpl, lastvalue, entry->value);
			} else
			if (lastentry->curve == INTERPOLATECURVETYPE_HERMITE) {
				psy_audio_pattern_blockinterpolaterangehermite(self->box.pattern,
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
