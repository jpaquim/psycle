/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianokeyboard.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* PianoKeyboard */
/* prototypes */
static void pianokeyboard_ondraw(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_drawuncoveredbottombackground(PianoKeyboard*,
	psy_ui_Graphics*, psy_ui_RealSize);
static void pianokeyboard_onmousedown(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_onpreferredsize(PianoKeyboard*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
/* vtable */
static psy_ui_ComponentVtable pianokeyboard_vtable;
static bool pianokeyboard_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianokeyboard_vtable_init(PianoKeyboard* self)
{
	assert(self);

	if (!pianokeyboard_vtable_initialized) {
		pianokeyboard_vtable = *(self->component.vtable);
		pianokeyboard_vtable.ondraw = (psy_ui_fp_component_ondraw)
			pianokeyboard_ondraw;
		pianokeyboard_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			pianokeyboard_onmousedown;
		pianokeyboard_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pianokeyboard_onpreferredsize;
		pianokeyboard_vtable_initialized = TRUE;
	}
	return &pianokeyboard_vtable;
}
/* implementation */
void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate)
{
	psy_ui_component_init(pianokeyboard_base(self), parent, NULL);
	psy_ui_component_setvtable(pianokeyboard_base(self),
		pianokeyboard_vtable_init(self));
	pianokeyboard_setsharedkeyboardstate(self, keyboardstate);	
	psy_ui_component_setbackgroundmode(pianokeyboard_base(self),
		psy_ui_NOBACKGROUND);
	psy_ui_component_setpreferredsize(pianokeyboard_base(self),
		psy_ui_size_make_em(10.0, 0.0));
	pianokeyboard_base(self)->sizehints->preferredheightset = FALSE;	
}

void pianokeyboard_setsharedkeyboardstate(PianoKeyboard* self,
	KeyboardState* keyboardstate)
{
	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianokeyboard_ondraw(PianoKeyboard* self, psy_ui_Graphics* g)
{	
	uint8_t key;	
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	psy_ui_Colour keyseparator;
	psy_ui_Colour keywhite;
	psy_ui_Colour keyblack;

	assert(self);

	keyblack = psy_ui_colour_make(0x00595959);
	keywhite = psy_ui_colour_make(0x00C0C0C0);
	keyseparator = psy_ui_colour_make(0x999999);
	size = psy_ui_component_scrollsize_px(pianokeyboard_base(self));
	tm = psy_ui_component_textmetric(pianokeyboard_base(self));
	self->keyboardstate->keyboardheightpx = keyboardstate_height(self->keyboardstate, tm);
	self->keyboardstate->keyheightpx = psy_ui_value_px(&self->keyboardstate->keyheight, tm, NULL);
	psy_ui_setcolour(g, keyseparator);
	if (self->keyboardstate->drawpianokeys) {
		psy_ui_settextcolour(g, keyseparator);
	} else {
		psy_ui_settextcolour(g, keywhite);
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		double cpy;

		cpy = keyboardstate_keytopx(self->keyboardstate, key);
		psy_ui_drawline(g, psy_ui_realpoint_make(0, cpy),
			psy_ui_realpoint_make(size.width, cpy));
		if (self->keyboardstate->drawpianokeys) {
			if (psy_dsp_isblack(key)) {
				psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
					psy_ui_realpoint_make(size.width * 0.75, cpy),
					psy_ui_realsize_make(size.width * 0.25,
						self->keyboardstate->keyheightpx)),
					keywhite);
				psy_ui_drawline(g,
					psy_ui_realpoint_make(0, cpy + self->keyboardstate->keyheightpx / 2),
					psy_ui_realpoint_make(size.width, cpy + self->keyboardstate->keyheightpx / 2));
				psy_ui_drawsolidrectangle(g,
					psy_ui_realrectangle_make(psy_ui_realpoint_make(0, cpy),
						psy_ui_realsize_make(size.width * 0.75,
							self->keyboardstate->keyheightpx)),
					keyblack);
			} else {
				psy_ui_RealRectangle r;

				r = psy_ui_realrectangle_make(psy_ui_realpoint_make(0.0, cpy),
						psy_ui_realsize_make(size.width,
							self->keyboardstate->keyheightpx));
				psy_ui_drawsolidrectangle(g, r, keywhite);
				if (psy_dsp_iskey_c(key) || psy_dsp_iskey_e(key)) {
					psy_ui_drawline(g,
						psy_ui_realpoint_make(0, cpy + self->keyboardstate->keyheightpx),
						psy_ui_realpoint_make(size.width, cpy + self->keyboardstate->keyheightpx));
					if (psy_dsp_iskey_c(key)) {
						psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
							psy_ui_ETO_CLIPPED, r,
							psy_dsp_notetostr(key, self->keyboardstate->notemode),
							psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
					}
				}
			}
		} else {
			psy_ui_RealRectangle r;

			r = psy_ui_realrectangle_make(psy_ui_realpoint_make(0.0, cpy),
					psy_ui_realsize_make(size.width,
						self->keyboardstate->keyheightpx));
			psy_ui_textoutrectangle(g,
				psy_ui_realrectangle_topleft(&r), psy_ui_ETO_CLIPPED, r,
				psy_dsp_notetostr(key, self->keyboardstate->notemode),
				psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
			psy_ui_drawline(g,
				psy_ui_realpoint_make(0, cpy + self->keyboardstate->keyheightpx),
				psy_ui_realpoint_make(size.width, cpy + self->keyboardstate->keyheightpx));
		}
	}
	pianokeyboard_drawuncoveredbottombackground(self, g, size);
}

void pianokeyboard_drawuncoveredbottombackground(PianoKeyboard* self, psy_ui_Graphics*
	g, psy_ui_RealSize size)
{
	double blankstart;
	psy_ui_Value scrolltop; 
	assert(self);

	blankstart = self->keyboardstate->keyboardheightpx;
	scrolltop = psy_ui_component_scrolltop(&self->component);
	if (blankstart - psy_ui_component_scrolltop_px(&self->component) <
			size.height) {
		psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0, blankstart),
			psy_ui_realsize_make(size.width,
				size.height - (blankstart - psy_ui_component_scrolltop_px(
					pianokeyboard_base(self))))),
			psy_ui_component_backgroundcolour(&self->component));
			//patternviewskin_separatorcolour(self->keyboardstate->skin, 1, 2));
	}
}

void pianokeyboard_onmousedown(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{	
	assert(self);

	psy_ui_mouseevent_stop_propagation(ev);
}

void pianokeyboard_setkeyboardtype(PianoKeyboard* self, KeyboardType
	keyboardtype)
{
	double width_em;

	assert(self);

	width_em = 10.0;
	switch (keyboardtype) {
		case KEYBOARDTYPE_KEYS:
			self->keyboardstate->drawpianokeys = TRUE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_A440;
			break;
		case KEYBOARDTYPE_NOTES:
			self->keyboardstate->drawpianokeys = FALSE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_A440;
			break;
		case KEYBOARDTYPE_DRUMS:
			self->keyboardstate->drawpianokeys = FALSE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_GMPERCUSSION;
			width_em = 21.0;			
			break;
		default:
			break;
	}
	psy_ui_component_setpreferredsize(pianokeyboard_base(self),
		psy_ui_size_make_em(width_em, 0.0));
	self->component.sizehints->preferredheightset = FALSE;
	psy_ui_component_invalidate(pianokeyboard_base(self));	
}

void pianokeyboard_onpreferredsize(PianoKeyboard* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	*rv = psy_ui_size_make(psy_ui_value_make_ew(10.0),
		psy_ui_value_make_px(self->keyboardstate->keyboardheightpx));
}
