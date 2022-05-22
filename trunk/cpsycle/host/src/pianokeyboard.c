/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianokeyboard.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* PianoKeyboard */

/* prototypes */
static void pianokeyboard_on_draw(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_on_mouse_down(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_on_mouse_move(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_on_mouse_up(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_on_preferred_size(PianoKeyboard*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianokeyboard_play(PianoKeyboard*, uint8_t key);

/* vtable */
static psy_ui_ComponentVtable pianokeyboard_vtable;
static bool pianokeyboard_vtable_initialized = FALSE;

static void pianokeyboard_vtable_init(PianoKeyboard* self)
{
	assert(self);

	if (!pianokeyboard_vtable_initialized) {
		pianokeyboard_vtable = *(self->component.vtable);
		pianokeyboard_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pianokeyboard_on_draw;
		pianokeyboard_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			pianokeyboard_on_mouse_down;
		pianokeyboard_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			pianokeyboard_on_mouse_move;
		pianokeyboard_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			pianokeyboard_on_mouse_up;
		pianokeyboard_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			pianokeyboard_on_preferred_size;
		pianokeyboard_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pianokeyboard_base(self),
		&pianokeyboard_vtable);	
}

/* implementation */
void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, psy_audio_Player* player)
{
	assert(self);
	assert(keyboardstate);
	assert(player);

	psy_ui_component_init(pianokeyboard_base(self), parent, NULL);	
	pianokeyboard_vtable_init(self);
	self->keyboardstate =  keyboardstate;
	self->player = player;
	self->active_note = psy_audio_NOTECOMMANDS_EMPTY;
	psy_ui_component_set_preferred_width(pianokeyboard_base(self),
		psy_ui_value_make_ew(10.0));
}

void pianokeyboard_on_draw(PianoKeyboard* self, psy_ui_Graphics* g)
{	
	uint8_t key;	
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	psy_ui_Colour keyseparator;
	psy_ui_Colour keywhite;
	psy_ui_Colour keyblack;
	psy_ui_Colour keyactive;

	assert(self);

	keyblack = psy_ui_colour_make(0x00595959);
	keywhite = psy_ui_colour_make(0x00C0C0C0);
	keyseparator = psy_ui_colour_make(0x999999);
	keyactive = psy_ui_colour_make(0x00808080);
	size = psy_ui_component_scroll_size_px(pianokeyboard_base(self));
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
				psy_ui_Colour keycolour;				
								
				psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
					psy_ui_realpoint_make(size.width * 0.60, cpy),
					psy_ui_realsize_make(size.width * 0.40,
						self->keyboardstate->keyheightpx)),
					keywhite);
				if (key > 0 && psy_audio_player_is_active_key(self->player, key - 1)) {
					psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
						psy_ui_realpoint_make(size.width * 0.60,
							cpy + self->keyboardstate->keyheightpx / 2),
						psy_ui_realsize_make(size.width * 0.40,
							self->keyboardstate->keyheightpx / 2)),
						keyactive);
				} else if (psy_audio_player_is_active_key(self->player, key + 1)) {
					psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
						psy_ui_realpoint_make(size.width * 0.60, cpy),
						psy_ui_realsize_make(size.width * 0.40,
							self->keyboardstate->keyheightpx / 2)),
						keyactive);
				}
				psy_ui_drawline(g,
					psy_ui_realpoint_make(0.0, cpy +
						self->keyboardstate->keyheightpx / 2),
					psy_ui_realpoint_make(size.width, cpy +
						self->keyboardstate->keyheightpx / 2));
				if (psy_audio_player_is_active_key(self->player, key)) {
					keycolour = keyactive;
				} else {
					keycolour = keyblack;
				}
				psy_ui_drawsolidrectangle(g,
					psy_ui_realrectangle_make(psy_ui_realpoint_make(0, cpy),
						psy_ui_realsize_make(size.width * 0.60,
							self->keyboardstate->keyheightpx)),
					keycolour);
			} else {
				psy_ui_RealRectangle r;
				psy_ui_Colour keycolour;

				r = psy_ui_realrectangle_make(psy_ui_realpoint_make(0.0, cpy),
						psy_ui_realsize_make(size.width,
							self->keyboardstate->keyheightpx));
				if (psy_audio_player_is_active_key(self->player, key)) {
					keycolour = keyactive;					
				} else {
					keycolour = keywhite;
				}
				psy_ui_drawsolidrectangle(g, r, keycolour);
				if (psy_dsp_iskey_c(key) || psy_dsp_iskey_e(key)) {
					psy_ui_drawline(g,
						psy_ui_realpoint_make(0, cpy + self->keyboardstate->keyheightpx),
						psy_ui_realpoint_make(size.width, cpy + self->keyboardstate->keyheightpx));
					if (psy_dsp_iskey_c(key)) {
						psy_ui_settextcolour(g, keyblack);
						psy_ui_textoutrectangle(g,
							psy_ui_realpoint_make(
								size.width - tm->tmAveCharWidth * 4,
								cpy),							
							psy_ui_ETO_CLIPPED, r,
							psy_dsp_notetostr(key, self->keyboardstate->notemode),
							psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
						psy_ui_settextcolour(g, keyseparator);
					}
				}
			}
		} else {
			psy_ui_RealRectangle r;
			psy_ui_Colour restorebg;
			psy_ui_Colour bg;
			psy_ui_Component* curr;

			restorebg = psy_ui_component_backgroundcolour(&self->component);
			bg = restorebg;
			curr = &self->component;
			while (curr && bg.mode.transparent) {
				curr = psy_ui_component_parent(curr);
				if (curr) {
					bg = psy_ui_component_backgroundcolour(curr);
				}
			}
			r = psy_ui_realrectangle_make(psy_ui_realpoint_make(0.0, cpy),
					psy_ui_realsize_make(size.width,
						self->keyboardstate->keyheightpx));			
			if (psy_dsp_isblack(key) && psy_strlen(psy_dsp_notetostr(key,
					self->keyboardstate->notemode)) > 0) {
				psy_ui_colour_add_rgb(&bg, 5, 5, 5);
				psy_ui_setbackgroundcolour(g, bg);
			} else {
				psy_ui_setbackgroundcolour(g, bg);
			}
			psy_ui_textoutrectangle(g,
				psy_ui_realrectangle_topleft(&r),
				psy_ui_ETO_CLIPPED | psy_ui_ETO_OPAQUE, r,
				psy_dsp_notetostr(key, self->keyboardstate->notemode),
				psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
			psy_ui_setbackgroundcolour(g, restorebg);			
		}
	}	
}

void pianokeyboard_on_mouse_down(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;

	assert(self);

	psy_ui_component_capture(&self->component);
	size = psy_ui_component_scroll_size_px(pianokeyboard_base(self));
	pianokeyboard_play(self,
		keyboardstate_screen_to_key(self->keyboardstate, 
		psy_ui_mouseevent_offset(ev), size.width));	
	psy_ui_mouseevent_stop_propagation(ev);
}

void pianokeyboard_on_mouse_move(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (!psy_audio_player_is_active_key(self->player, psy_audio_NOTECOMMANDS_EMPTY)) {
		psy_ui_RealSize size;

		size = psy_ui_component_scroll_size_px(pianokeyboard_base(self));		
		pianokeyboard_play(self,
			keyboardstate_screen_to_key(self->keyboardstate,
				psy_ui_mouseevent_offset(ev), size.width));
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void pianokeyboard_on_mouse_up(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_release_capture(&self->component);	
	pianokeyboard_play(self, psy_audio_NOTECOMMANDS_RELEASE);
	psy_ui_mouseevent_stop_propagation(ev);
}

void pianokeyboard_play(PianoKeyboard* self, uint8_t key)
{
	if (!psy_audio_player_is_active_key(self->player, key)) {
		psy_audio_PatternEvent ev;		
				
		ev = psy_audio_player_pattern_event(self->player, key);
		ev.note = key;
		psy_audio_player_playevent(self->player, &ev);		
		psy_ui_component_invalidate(&self->component);
	}
	self->active_note = self->player->active_note;
}

void pianokeyboard_set_keyboard_type(PianoKeyboard* self, KeyboardType
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
	psy_ui_component_set_preferred_width(pianokeyboard_base(self),
		psy_ui_value_make_ew(width_em));	
	psy_ui_component_invalidate(pianokeyboard_base(self));	
}

void pianokeyboard_idle(PianoKeyboard* self)
{
	if (self->active_note != self->player->active_note) {
		self->active_note = self->player->active_note;
		psy_ui_component_invalidate(&self->component);		
	}
}

void pianokeyboard_on_preferred_size(PianoKeyboard* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	*rv = psy_ui_size_make(psy_ui_value_make_ew(10.0),
		psy_ui_value_make_px(self->keyboardstate->keyboardheightpx));
}
