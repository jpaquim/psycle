/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianokeyboard.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"


void pianokeycolours_init(PianoKeyColours* self)
{
	assert(self);
	
	self->keyblack = psy_ui_colour_make(0x00595959);
	self->keywhite = psy_ui_colour_make(0x00C0C0C0);
	self->keyseparator = psy_ui_colour_make(0x999999);
	self->keyactive = psy_ui_colour_make(0x00808080);
	self->keywhiteselect = psy_ui_colour_make(0x00FF2288);
	self->keyblackselect = psy_ui_colour_weighted(&self->keywhiteselect, 800);
}

/* PianoKeyboard */

/* prototypes */
static void pianokeyboard_on_draw(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_draw_vertical(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_draw_horizontal(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_on_mouse_down(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_on_mouse_move(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_on_mouse_up(PianoKeyboard*, psy_ui_MouseEvent*);
static void pianokeyboard_on_preferred_size(PianoKeyboard*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void pianokeyboard_on_timer(PianoKeyboard*, uintptr_t timer_id);
static void pianokeyboard_play(PianoKeyboard*, uint8_t key);
static uint8_t pianokeyboard_top_key(const PianoKeyboard*);

static void pianokeyboard_ondraw_horizontal(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_drawwhitekeys_horizontal(PianoKeyboard*,
	psy_ui_Graphics*);
static void pianokeyboard_drawblackkeys_horizontal(PianoKeyboard*,
	psy_ui_Graphics*);
static psy_ui_Colour pianokeyboard_key_colour(PianoKeyboard*, uint8_t key);
static void pianokeyboard_on_align(PianoKeyboard*);

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
		pianokeyboard_vtable.onalign =
			(psy_ui_fp_component)
			pianokeyboard_on_align;
		pianokeyboard_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			pianokeyboard_on_preferred_size;
		pianokeyboard_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			pianokeyboard_on_timer;
		pianokeyboard_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pianokeyboard_base(self),
		&pianokeyboard_vtable);	
}

/* implementation */
void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, psy_audio_Player* player,
	psy_ui_Component* grid)
{
	assert(self);
	assert(keyboardstate);	

	psy_ui_component_init(pianokeyboard_base(self), parent, NULL);	
	pianokeyboard_vtable_init(self);	
	self->keyboardstate =  keyboardstate;
	self->player = player;
	self->active_note = psy_audio_NOTECOMMANDS_EMPTY;
	self->grid = grid;
	self->scroll = 0;
	self->scrollspeed = 1;
	self->scrollcount = 0;
	pianokeycolours_init(&self->colours);
	if (self->keyboardstate->orientation == psy_ui_VERTICAL) {
		psy_ui_component_set_preferred_width(pianokeyboard_base(self),
			psy_ui_value_make_ew(10.0));
	} else {
		psy_ui_component_set_preferred_height(pianokeyboard_base(self),
			psy_ui_value_make_ew(8.0));
	}
}

void pianokeyboard_on_draw(PianoKeyboard* self, psy_ui_Graphics* g)
{
	if (self->keyboardstate->orientation == psy_ui_VERTICAL) {
		pianokeyboard_draw_vertical(self, g);
	} else {
		pianokeyboard_ondraw_horizontal(self, g);
	}
}

void pianokeyboard_draw_vertical(PianoKeyboard* self, psy_ui_Graphics* g)
{	
	uint8_t key;	
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	

	assert(self);
	
	size = psy_ui_component_scroll_size_px(pianokeyboard_base(self));
	tm = psy_ui_component_textmetric(pianokeyboard_base(self));
	self->keyboardstate->keyboard_extent_px = keyboardstate_extent(self->keyboardstate, tm);
	self->keyboardstate->key_extent_px = psy_ui_value_px(&self->keyboardstate->key_extent, tm, NULL);
	psy_ui_setcolour(g, self->colours.keyseparator);
	if (self->keyboardstate->drawpianokeys) {
		psy_ui_set_text_colour(g, self->colours.keyseparator);
	} else {
		psy_ui_set_text_colour(g, self->colours.keywhite);
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		double cp;

		cp = keyboardstate_key_to_px(self->keyboardstate, key);
		psy_ui_drawline(g, psy_ui_realpoint_make(0, cp),
			psy_ui_realpoint_make(size.width, cp));
		if (self->keyboardstate->drawpianokeys) {
			if (psy_dsp_isblack(key)) {
				psy_ui_Colour keycolour;				
								
				psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
					psy_ui_realpoint_make(size.width * 0.60, cp),
					psy_ui_realsize_make(size.width * 0.40,
						self->keyboardstate->key_extent_px)),
					self->colours.keywhite);
				if (key > 0 && self->player && psy_audio_player_is_active_key(self->player, key - 1)) {
					psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
						psy_ui_realpoint_make(size.width * 0.60,
							cp + self->keyboardstate->key_extent_px / 2),
						psy_ui_realsize_make(size.width * 0.40,
							self->keyboardstate->key_extent_px / 2)),
						self->colours.keyactive);
				} else if (self->player && psy_audio_player_is_active_key(self->player, key + 1)) {
					psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
						psy_ui_realpoint_make(size.width * 0.60, cp),
						psy_ui_realsize_make(size.width * 0.40,
							self->keyboardstate->key_extent_px / 2)),
						self->colours.keyactive);
				}
				psy_ui_drawline(g,
					psy_ui_realpoint_make(0.0, cp +
						self->keyboardstate->key_extent_px / 2),
					psy_ui_realpoint_make(size.width, cp +
						self->keyboardstate->key_extent_px / 2));
				if (self->player && psy_audio_player_is_active_key(self->player, key)) {
					keycolour = self->colours.keyactive;
				} else {
					keycolour = self->colours.keyblack;
				}
			psy_ui_drawsolidrectangle(g,
				psy_ui_realrectangle_make(psy_ui_realpoint_make(0, cp),
					psy_ui_realsize_make(size.width * 0.60,
						self->keyboardstate->key_extent_px)),
				keycolour);
			} else {
			psy_ui_RealRectangle r;
			psy_ui_Colour keycolour;

			r = psy_ui_realrectangle_make(psy_ui_realpoint_make(0.0, cp),
				psy_ui_realsize_make(size.width,
					self->keyboardstate->key_extent_px));
			if (self->player && psy_audio_player_is_active_key(self->player, key)) {
				keycolour = self->colours.keyactive;
			} else {
				keycolour = self->colours.keywhite;
			}
			psy_ui_drawsolidrectangle(g, r, keycolour);
			if (psy_dsp_iskey_c(key) || psy_dsp_iskey_e(key)) {
				psy_ui_drawline(g,
					psy_ui_realpoint_make(0, cp + self->keyboardstate->key_extent_px),
					psy_ui_realpoint_make(size.width, cp + self->keyboardstate->key_extent_px));
				if (psy_dsp_iskey_c(key)) {
					psy_ui_set_text_colour(g, self->colours.keyblack);
					psy_ui_textoutrectangle(g,
						psy_ui_realpoint_make(
							size.width - tm->tmAveCharWidth * 4,
							cp),
						psy_ui_ETO_CLIPPED, r,
						psy_dsp_notetostr(key, self->keyboardstate->notemode),
						psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
					psy_ui_set_text_colour(g, self->colours.keyseparator);
				}
			}
			}
		} else {
		psy_ui_RealRectangle r;
		psy_ui_Colour restorebg;
		psy_ui_Colour bg;
		psy_ui_Component* curr;

		restorebg = psy_ui_component_background_colour(&self->component);
		bg = restorebg;
		curr = &self->component;
		while (curr && bg.mode.transparent) {
			curr = psy_ui_component_parent(curr);
			if (curr) {
				bg = psy_ui_component_background_colour(curr);
			}
		}
		r = psy_ui_realrectangle_make(psy_ui_realpoint_make(0.0, cp),
			psy_ui_realsize_make(size.width,
				self->keyboardstate->key_extent_px));
		if (psy_dsp_isblack(key) && psy_strlen(psy_dsp_notetostr(key,
			self->keyboardstate->notemode)) > 0) {
			psy_ui_colour_add_rgb(&bg, 5, 5, 5);
			psy_ui_set_background_colour(g, bg);
		} else {
			psy_ui_set_background_colour(g, bg);
		}
		psy_ui_textoutrectangle(g,
			psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_CLIPPED | psy_ui_ETO_OPAQUE, r,
			psy_dsp_notetostr(key, self->keyboardstate->notemode),
			psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
		psy_ui_set_background_colour(g, restorebg);
		}
	}
}

void pianokeyboard_draw_horizontal(PianoKeyboard* self, psy_ui_Graphics* g)
{	
	uint8_t key;	
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	

	assert(self);
	
	size = psy_ui_component_scroll_size_px(pianokeyboard_base(self));
	tm = psy_ui_component_textmetric(pianokeyboard_base(self));
	self->keyboardstate->keyboard_extent_px = keyboardstate_extent(self->keyboardstate, tm);
	self->keyboardstate->key_extent_px = psy_ui_value_px(&self->keyboardstate->key_extent, tm, NULL);
	psy_ui_setcolour(g, self->colours.keyseparator);
	if (self->keyboardstate->drawpianokeys) {
		psy_ui_set_text_colour(g, self->colours.keyseparator);
	} else {
		psy_ui_set_text_colour(g, self->colours.keywhite);
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		double cp;

		cp = keyboardstate_key_to_px(self->keyboardstate, key);
		psy_ui_drawline(g, psy_ui_realpoint_make(cp, 0),
			psy_ui_realpoint_make(cp, size.height));
		if (self->keyboardstate->drawpianokeys) {
			if (psy_dsp_isblack(key)) {
				psy_ui_Colour keycolour;				
								
				psy_ui_drawsolidrectangle(g,
					psy_ui_realrectangle_make(
						psy_ui_realpoint_make(cp, size.height * 0.60),
						psy_ui_realsize_make(
							self->keyboardstate->key_extent_px,
							size.height * 0.40)),
					self->colours.keywhite);
				if (key > 0 && self->player && psy_audio_player_is_active_key(self->player, key - 1)) {
					psy_ui_drawsolidrectangle(g,
						psy_ui_realrectangle_make(
							psy_ui_realpoint_make(
								cp + self->keyboardstate->key_extent_px / 2,
								size.height * 0.60),
						psy_ui_realsize_make(
							self->keyboardstate->key_extent_px / 2,
							size.height * 0.40)),
						self->colours.keyactive);
				} else if (self->player && psy_audio_player_is_active_key(self->player, key + 1)) {
					psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
						psy_ui_realpoint_make(cp, size.width * 0.60),
						psy_ui_realsize_make(
							self->keyboardstate->key_extent_px / 2,
							size.height * 0.40)),
						self->colours.keyactive);
				}
				psy_ui_drawline(g,
					psy_ui_realpoint_make(cp +
						self->keyboardstate->key_extent_px / 2, 0.0),
					psy_ui_realpoint_make(cp +
						self->keyboardstate->key_extent_px / 2,
						size.height));
				if (self->player && psy_audio_player_is_active_key(self->player, key)) {
					keycolour = self->colours.keyactive;
				} else {
					keycolour = self->colours.keyblack;
				}
				psy_ui_drawsolidrectangle(g,
					psy_ui_realrectangle_make(psy_ui_realpoint_make(cp, 0),
					psy_ui_realsize_make(
						self->keyboardstate->key_extent_px,
						size.height * 0.60)),
				keycolour);
			} else {
				psy_ui_RealRectangle r;
				psy_ui_Colour keycolour;

				r = psy_ui_realrectangle_make(
						psy_ui_realpoint_make(cp, 0.0),
						psy_ui_realsize_make(
							self->keyboardstate->key_extent_px,
							size.height));
				if (self->player && psy_audio_player_is_active_key(self->player, key)) {
					keycolour = self->colours.keyactive;
				} else {
					keycolour = self->colours.keywhite;
				}
				psy_ui_drawsolidrectangle(g, r, keycolour);
				if (psy_dsp_iskey_c(key) || psy_dsp_iskey_e(key)) {
					psy_ui_drawline(g,
						psy_ui_realpoint_make(cp + self->keyboardstate->key_extent_px, 0),
						psy_ui_realpoint_make(cp + self->keyboardstate->key_extent_px, size.height));
					if (psy_dsp_iskey_c(key)) {
						psy_ui_set_text_colour(g, self->colours.keyblack);
						psy_ui_textoutrectangle(g,
							psy_ui_realpoint_make(							
								cp,
								size.height - tm->tmHeight * 1.0),
							psy_ui_ETO_CLIPPED, r,
							psy_dsp_notetostr(key, self->keyboardstate->notemode),
							psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
						psy_ui_set_text_colour(g, self->colours.keyseparator);
					}
				}
			}
		} else {
			psy_ui_RealRectangle r;
			psy_ui_Colour restorebg;
			psy_ui_Colour bg;
			psy_ui_Component* curr;

			restorebg = psy_ui_component_background_colour(&self->component);
			bg = restorebg;
			curr = &self->component;
			while (curr && bg.mode.transparent) {
				curr = psy_ui_component_parent(curr);
				if (curr) {
					bg = psy_ui_component_background_colour(curr);
				}
			}
			r = psy_ui_realrectangle_make(psy_ui_realpoint_make(cp, 0.0),
				psy_ui_realsize_make(
					self->keyboardstate->key_extent_px,
					size.height));
			if (psy_dsp_isblack(key) && psy_strlen(psy_dsp_notetostr(key,
				self->keyboardstate->notemode)) > 0) {
				psy_ui_colour_add_rgb(&bg, 5, 5, 5);
				psy_ui_set_background_colour(g, bg);
			} else {
				psy_ui_set_background_colour(g, bg);
			}
			psy_ui_textoutrectangle(g,
				psy_ui_realrectangle_topleft(&r),
				psy_ui_ETO_CLIPPED | psy_ui_ETO_OPAQUE, r,
				psy_dsp_notetostr(key, self->keyboardstate->notemode),
				psy_strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
			psy_ui_set_background_colour(g, restorebg);
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
			psy_ui_mouseevent_offset(ev),
			(self->keyboardstate->orientation == psy_ui_VERTICAL)
			? size.width
			: size.height));
	psy_ui_mouseevent_stop_propagation(ev);
}

void pianokeyboard_on_mouse_move(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{
	psy_ui_Size size;
	psy_ui_RealPoint pt;

	assert(self);

	if (!self->player) {
		return;
	}
	pt = psy_ui_mouseevent_offset(ev);
	if (!psy_audio_player_is_active_key(self->player, psy_audio_NOTECOMMANDS_EMPTY)) {
		psy_ui_RealSize size;

		size = psy_ui_component_scroll_size_px(pianokeyboard_base(self));
		pianokeyboard_play(self,
			keyboardstate_screen_to_key(self->keyboardstate,
				psy_ui_mouseevent_offset(ev),
				(self->keyboardstate->orientation == psy_ui_VERTICAL)
				? size.width
				: size.height));
	}
	size = psy_ui_component_client_size(&self->component);	
	if (self->keyboardstate->orientation == psy_ui_VERTICAL) {
		if (pt.y - psy_ui_component_scroll_top_px(&self->component) > psy_ui_value_px(&size.height,
				psy_ui_component_textmetric(&self->component), NULL)) {
			if (self->scroll == 0) {
				psy_ui_component_start_timer(&self->component, 0, 10);
			}
			self->scroll = 1;
		} else if (pt.y < psy_ui_component_scroll_top_px(&self->component)) {
			if (self->scroll == 0) {
				psy_ui_component_start_timer(&self->component, 0, 10);
			}
			self->scroll = -1;
		} else {
			if (self->scroll != 0) {
				psy_ui_component_stop_timer(&self->component, 0);
				self->scroll = 0;
				self->scrollcount = 0;
			}				
		}
	} else {
		/*if (pt.x - psy_ui_component_scroll_left_px(&self->component) > psy_ui_value_px(&size.width,
				psy_ui_component_textmetric(&self->component), NULL)) {
			if (self->scroll == 0) {
				psy_ui_component_start_timer(&self->component, 0, 10);
			}
			self->scroll = 1;
		} else if (pt.x < psy_ui_component_scroll_left_px(&self->component)) {
			if (self->scroll == 0) {
				psy_ui_component_start_timer(&self->component, 0, 10);
			}
			self->scroll = -1;
		} else {
			if (self->scroll != 0) {
				psy_ui_component_stop_timer(&self->component, 0);
				self->scroll = 0;
				self->scrollcount = 0;
			}				
		}*/		
	}
	psy_ui_mouseevent_stop_propagation(ev);	
}

uint8_t pianokeyboard_top_key(const PianoKeyboard* self)
{
	return keyboardstate_screen_to_key(self->keyboardstate,
		self->keyboardstate->orientation == psy_ui_VERTICAL
		? psy_ui_realpoint_make(0.0, psy_ui_component_scroll_top_px(&self->component))
		: psy_ui_realpoint_make(psy_ui_component_scroll_left_px(&self->component), 0.0),
		0.0);
}

void pianokeyboard_on_mouse_up(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_release_capture(&self->component);	
	pianokeyboard_play(self, psy_audio_NOTECOMMANDS_RELEASE);
	psy_ui_component_stop_timer(&self->component, 0);
	if (self->scroll != 0) {
		psy_ui_component_stop_timer(&self->component, 0);
		self->scroll = 0;
		self->scrollcount = 0;
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void pianokeyboard_play(PianoKeyboard* self, uint8_t key)
{
	if (!self->player) {
		return;
	}
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
	if (!self->player) {
		return;
	}
	if (self->active_note != self->player->active_note) {
		self->active_note = self->player->active_note;
		psy_ui_component_invalidate(&self->component);		
	}
}

void pianokeyboard_on_preferred_size(PianoKeyboard* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	*rv = psy_ui_size_make(psy_ui_value_make_ew(10.0),
		psy_ui_value_make_px(self->keyboardstate->keyboard_extent_px));
}

void pianokeyboard_on_timer(PianoKeyboard* self, uintptr_t timer_id)
{
	if (!self->player) {
		return;
	}
	if (self->scroll != 0) {
		if (self->scrollcount == 0) {
			uint8_t dest_key;
			uint8_t play_key;

			self->scrollcount = self->scrollspeed;
			dest_key = self->keyboardstate->keymax - pianokeyboard_top_key(self);
			play_key = self->player->active_note;
			if (self->scroll < 0) {
				if (dest_key > 0) {
					--dest_key;
					++play_key;					
				}
			} else {
				++dest_key;
				--play_key;
			}
			if (self->grid) {
				if (self->keyboardstate->orientation == psy_ui_VERTICAL) {
					psy_ui_component_set_scroll_top_px(self->grid,
						dest_key * self->keyboardstate->key_extent_px);
				} else {
					psy_ui_component_set_scroll_left_px(self->grid,
						dest_key * self->keyboardstate->key_extent_px);
				}
			}
			pianokeyboard_play(self, play_key);
		} else {
			--self->scrollcount;
		}
	}
}

void pianokeyboard_ondraw_horizontal(PianoKeyboard* self, psy_ui_Graphics* g)
{				
	pianokeyboard_drawwhitekeys_horizontal(self, g);	
	pianokeyboard_drawblackkeys_horizontal(self, g);	
}

void pianokeyboard_drawwhitekeys_horizontal(PianoKeyboard* self,
	psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	int key;
	double cp = 0;
	double top = 0.60;
	double bottom = 1 - top;	

	assert(self);
	
	size = psy_ui_component_size_px(&self->component);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_set_text_colour(g, psy_ui_colour_make(0x00333333));	
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		if (!psy_dsp_isblack(key)) {
			psy_ui_RealRectangle r;			

			r = psy_ui_realrectangle_make(
					psy_ui_realpoint_make(cp, 0),
					psy_ui_realsize_make(
						(self->keyboardstate->key_extent_px + 1),
						size.height));
			//if (psy_audio_parameterrange_intersect(&self->entry.keyrange, key)) {				
			psy_ui_drawsolidrectangle(g, r,
				pianokeyboard_key_colour(self, key));
			psy_ui_drawline(g, psy_ui_realpoint_make(cp, 0),
				psy_ui_realpoint_make(cp, size.height));
			cp += self->keyboardstate->key_extent_px;			
		}
	}
}

void pianokeyboard_drawblackkeys_horizontal(PianoKeyboard* self,
	psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	int key;
	double cp;
	double top = 0.60;
	double bottom = 1 - top;	

	assert(self);
	
	size = psy_ui_component_size_px(&self->component);
	psy_ui_set_text_colour(g, psy_ui_colour_make(0x00CACACA));
	cp = 0;
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		if (!psy_dsp_isblack(key)) {
			cp += self->keyboardstate->key_extent_px;
		} else {
			psy_ui_RealRectangle r;
			int x;
			int width;			

			x = (int)cp - (int)(self->keyboardstate->key_extent_px * 0.68 / 2);
			width = (int)(self->keyboardstate->key_extent_px * 0.68);
			if (self->keyboardstate->key_align == psy_ui_ALIGN_BOTTOM) {
				r = psy_ui_realrectangle_make(psy_ui_realpoint_make(x, 0),
					psy_ui_realsize_make(width, (int)(size.height * top)));
			} else {
				r = psy_ui_realrectangle_make(psy_ui_realpoint_make(x, 
						size.height * (1 - top)),
					psy_ui_realsize_make(width, (int)(size.height * top)));
			}
			psy_ui_drawsolidrectangle(g, r,
				pianokeyboard_key_colour(self, key));
		}
	}
}

psy_ui_Colour pianokeyboard_key_colour(PianoKeyboard* self, uint8_t key)
{
	if (psy_audio_parameterrange_intersect(
			&self->keyboardstate->entry.keyrange, key)) {
		if (psy_dsp_isblack(key)) {
			return self->colours.keyblackselect;
		} else {
			return self->colours.keywhiteselect;
		}
	} else if (self->player && psy_audio_player_is_active_key(self->player,
			key)) {
		return self->colours.keyactive;
	} else if (psy_dsp_isblack(key)) {
		return self->colours.keyblack;
	}
	return self->colours.keywhite;
}

static int numwhitekey(int key)
{
	int octave = key / 12;
	int offset = key % 12;
	int c = 0;
	int i;

	for (i = 1; i <= offset; ++i) {
		if (!psy_dsp_isblack(i)) ++c;
	}
	return octave * 7 + c;
}

void pianokeyboard_on_align(PianoKeyboard* self)
{
	if (self->keyboardstate->align_keys) {
		int keymin = 0;
		int keymax = psy_audio_NOTECOMMANDS_RELEASE;
		int key;
		int numwhitekeys;
		const psy_ui_TextMetric* tm;
		psy_ui_IntSize size;

		numwhitekeys = 0;
		for (key = keymin; key < keymax; ++key) {
			if (!psy_dsp_isblack(key)) {
				++numwhitekeys;
			}
		}
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_scroll_size(&self->component), tm, NULL);
		self->keyboardstate->key_extent_px = size.width / (double)numwhitekeys;		
	}
}
