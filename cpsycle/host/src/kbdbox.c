// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "kbdbox.h"
// host
#include "styles.h"
// audio
#include <player.h>
// platform
#include "../../detail/portable.h"

static void kbdbox_makekeyproperties(KbdBox*);
static void kbdbox_addkeyproperties(KbdBox*, psy_Property* section,
	uintptr_t keycode, const char* label, int size, int cr);
static psy_ui_RealRectangle kbdbox_bounds(KbdBox*);

// prototypes
static void kbdboxkey_ondestroy(KbdBoxKey*);
static void kbdboxkey_ondraw(KbdBoxKey*, psy_ui_Graphics*);
// vtable
static psy_ui_ComponentVtable kbdboxkey_vtable;
static bool kbdboxkey_vtable_initialized = FALSE;

static void kbdboxkey_vtable_init(KbdBoxKey* self)
{
	if (!kbdboxkey_vtable_initialized) {
		kbdboxkey_vtable = *(self->component.vtable);
		kbdboxkey_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			kbdboxkey_ondestroy;
		kbdboxkey_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			kbdboxkey_ondraw;
		kbdboxkey_vtable_initialized = TRUE;
	}
	self->component.vtable = &kbdboxkey_vtable;
}
void kbdboxkey_init_all(KbdBoxKey* self, psy_ui_Component* parent, psy_ui_Component* view, 
	int x, int y, int width, int height, const char* label)
{
	psy_ui_RealRectangle r;

	psy_ui_component_init(&self->component, parent, view);
	kbdboxkey_vtable_init(self);
	psy_ui_setrectangle(&r, x, y, width, height);
	if (strcmp(label, "EMPTY") != 0) {
		psy_ui_component_setstyletypes(&self->component,
			STYLE_KEY, STYLE_KEY_HOVER, psy_INDEX_INVALID,
			psy_INDEX_INVALID);
		psy_ui_component_setstyletype_active(&self->component,
			STYLE_KEY_ACTIVE);
		self->label = strdup(label);
	} else {
		self->label = strdup("");
	}
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make_px(&r));	
	
	self->desc0 = strdup("");
	self->desc1 = strdup("");
	self->desc2 = strdup("");
	self->desc3 = strdup("");
	self->marked = FALSE;	
}

void kbdboxkey_ondestroy(KbdBoxKey* self)
{
	free(self->label);
	free(self->desc0);
	free(self->desc1);
	free(self->desc2);
	free(self->desc3);
}

KbdBoxKey* kbdboxkey_allocinit_all(psy_ui_Component* parent, psy_ui_Component* view,
	int x, int y, int width, int height, const char* label)
{
	KbdBoxKey* rv;

	rv = (KbdBoxKey*)malloc(sizeof(KbdBoxKey));
	if (rv) {
		kbdboxkey_init_all(rv, parent, view, x, y, width, height, label);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void kbdboxkey_ondraw(KbdBoxKey* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	double cpx;
	double cpy;
	psy_ui_RealRectangle position;
	psy_ui_Size descsize;
	double descident;
			
	psy_ui_realrectangle_init_all(&position,
		psy_ui_realpoint_zero(),
		psy_ui_component_sizepx(&self->component));
	tm = psy_ui_component_textmetric(&self->component);
	descident = (tm->tmAveCharWidth * 2) * psy_strlen(self->label);
	if (self->marked) {
		psy_ui_setcolour(g, psy_ui_style(STYLE_KEY_SELECT)->colour);
		psy_ui_settextcolour(g, psy_ui_style(STYLE_KEY_SELECT)->colour);
	}	
	cpx = 4;
	cpy = 4;
	psy_ui_textout(g, cpx, cpy,
		self->label, psy_strlen(self->label));
	psy_ui_textout(g, cpx + descident, cpy,
		self->desc0, psy_strlen(self->desc0));
	psy_ui_textout(g, cpx, cpy + tm->tmHeight,
		self->desc1, psy_strlen(self->desc1));
	psy_ui_textout(g, cpx, cpy + tm->tmHeight * 2,
		self->desc2, psy_strlen(self->desc2));
	psy_ui_textout(g, cpx, cpy + tm->tmHeight * 3,
		self->desc3, psy_strlen(self->desc3));
}

// KbdBox
static void kbdbox_initfont(KbdBox*);
static void kbdbox_makekeys(KbdBox*);
static void kbdbox_ondestroy(KbdBox*, psy_ui_Component* sender);
static void kbdbox_ondraw(KbdBox*, psy_ui_Graphics*);
static void kbdbox_onmousedown(KbdBox*, psy_ui_MouseEvent*);
static void kbdbox_drawkey(KbdBox*, psy_ui_Graphics*, KbdBoxKey*);
static void kbdbox_addsmallkey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addmediumkey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addlargerkey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addlargekey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addwidekey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addemptykey(KbdBox*, uintptr_t keycode);
static void kbdbox_addkey(KbdBox* self, uintptr_t keycode, const char* label,
	double size);
static void kbdbox_onpreferredsize(KbdBox*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable kbdbox_vtable;
static bool kbdbox_vtable_initialized = FALSE;

static void kbdbox_vtable_init(KbdBox* self)
{
	if (!kbdbox_vtable_initialized) {
		kbdbox_vtable = *(self->component.vtable);
		kbdbox_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			kbdbox_onmousedown;
		kbdbox_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			kbdbox_onpreferredsize;
		kbdbox_vtable_initialized = TRUE;
	}
	self->component.vtable = &kbdbox_vtable;
}

void kbdbox_init(KbdBox* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(kbdbox_base(self), parent, NULL);
	kbdbox_vtable_init(self);	
	self->workspace = workspace;	
	psy_ui_component_doublebuffer(kbdbox_base(self));
	psy_signal_connect(&kbdbox_base(self)->signal_destroy, self,
		kbdbox_ondestroy);
	psy_table_init(&self->keys);
	self->keyset = NULL;
	kbdbox_initfont(self);
	kbdbox_makekeyproperties(self);	
	kbdbox_makekeys(self);
}

void kbdbox_ondestroy(KbdBox* self, psy_ui_Component* sender)
{
	psy_table_dispose(&self->keys);
}

void kbdbox_initfont(KbdBox* self)
{
	const psy_ui_Font* font;
	const psy_ui_TextMetric* tm;
	
	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;		

		fontinfo = psy_ui_font_fontinfo(font);
		fontinfo.lfHeight = (int)(fontinfo.lfHeight * 0.6);
		psy_ui_component_setfontinfo(&self->component, fontinfo);		
	}
	tm = psy_ui_component_textmetric(&self->component);
	self->corner.width = psy_ui_value_makepx(5);
	self->corner.height = psy_ui_value_makepx(5);
	self->descident = tm->tmAveCharWidth * 6;
	self->ident = (int)(tm->tmHeight * 0.3);
	self->keyheight = (int)(tm->tmHeight * 4.5);
	self->keywidth = tm->tmAveCharWidth * 20;
}

void kbdbox_onmousedown(KbdBox* self, psy_ui_MouseEvent* ev)
{
	psy_TableIterator it;
	psy_ui_RealRectangle position;
	
	for (it = psy_table_begin(&self->keys);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		KbdBoxKey* key;

		key = (KbdBoxKey*)psy_tableiterator_value(&it);
		position = psy_ui_component_position(&key->component);
		if (psy_ui_realrectangle_intersect(&position,
				psy_ui_realpoint_make(ev->pt.x, ev->pt.y))) {
			psy_EventDriver* kbd;
			psy_EventDriverInput input;
			double rowheight;
			double pos;
			bool shift = FALSE;
			bool ctrl = FALSE;
			
			rowheight = (position.bottom - position.top) / 3;
			pos = ev->pt.y - position.top;
			if (pos < rowheight) {
				shift = FALSE;
				ctrl = FALSE;
			} else if (pos < rowheight * 2) {
				if (strlen(key->desc1) != 0) {
					shift = TRUE;
				}
				ctrl = FALSE;
			} else if (pos < rowheight * 3) {
				shift = FALSE;
				if (strlen(key->desc2) != 0) {
					ctrl = TRUE;
				}
			}			
			input.message = psy_EVENTDRIVER_KEYDOWN;
			kbd = workspace_kbddriver(self->workspace);
			input.param1 = psy_audio_encodeinput(
				(uint32_t)psy_tableiterator_key(&it), shift, ctrl, 0);
			input.param2 = workspace_octave(self->workspace) * 12;
			psy_eventdriver_write(kbd, input);
			break;
		}
	}
}

void kbdbox_makekeys(KbdBox* self)
{
	if (self->keyset) {
		psy_Property* mainsection;

		self->cpx = 0;
		self->cpy = 0;

		mainsection = psy_property_find(self->keyset, "main", PSY_PROPERTY_TYPE_SECTION);
		if (mainsection) {
			psy_List* p;
			int currrow = 0;

			for (p = psy_property_begin(mainsection); p != NULL;
					psy_list_next(&p)) {
				psy_Property* property;
				intptr_t keycode;

				property = (psy_Property*)p->entry;				
				keycode = psy_property_at_int(property, "keycode", -1);
				if (keycode != -1) {
					const char* label;
					intptr_t size;
					intptr_t cr;

					label = psy_property_at_str(property, "label", "");
					size = psy_property_at_int(property, "size", 0);
					cr = psy_property_at_int(property, "cr", 0);

					if (cr) {
						self->cpy += self->keyheight + self->ident;
						self->cpx = 0;
					}
					if (size == 0) {
						kbdbox_addsmallkey(self, keycode, label);
					} else if (size == 1) {
						kbdbox_addmediumkey(self, keycode, label);
					} else if (size == 2) {
						kbdbox_addlargerkey(self, keycode, label);
					} else if (size == 3) {
						kbdbox_addlargekey(self, keycode, label);
					} else if (size == 4) {
						kbdbox_addwidekey(self, keycode, label);
					} else if (size == 5) {
						kbdbox_addemptykey(self, keycode);
					}
				}				
			}
		}
	}
}
	
void kbdbox_makekeyproperties(KbdBox* self)
{
	psy_Property* main;
	int kc;
	int col = 0;

	self->keyset = psy_property_allocinit_key(NULL);
	main = psy_property_append_section(self->keyset, "main");
		
	kc = 0x1000;
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_ESCAPE, "ESC", 1, 0);
	kbdbox_addkeyproperties(self, main, kc++, "", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F1, "F1", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F2, "F2", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F3, "F3", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F4, "F4", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F5, "F5", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F6, "F6", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F7, "F7", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F8, "F8", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F9, "F9", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F10, "F10", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F11, "F11", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F12, "F12", 0, 0);
	kbdbox_addkeyproperties(self, main, kc++, "EMPTY", 5, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_INSERT, "INS", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_HOME, "HOME", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_PRIOR, "PGUP", 0, 0);
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACKQUOTE, "`", 0, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT1, "1", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT2, "2", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT3, "3", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT4, "4", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT5, "5", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT6, "6", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT7, "7", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT8, "8", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT9, "9", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT0, "0", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_MINUS, "-", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_EQUAL, "=", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACK, "BACK", 1, 0);
	kbdbox_addkeyproperties(self, main, kc++, "EMPTY", 5, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DELETE, "DEL", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_END, "END", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_NEXT, "PGDWN", 0, 0);

	kbdbox_addkeyproperties(self, main, psy_ui_KEY_TAB, "TAB", 1, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_Q, "Q", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_W, "W", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_E, "E", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_R, "R", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_T, "T", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_Y, "Y", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_U, "U", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_I, "I", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_O, "O", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_P, "P", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BRACKETLEFT, "[", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BRACKETRIGHT, "]", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACKSLASH, "\\", 0, 0);
	
	kbdbox_addkeyproperties(self, main, kc++, "CAPS", 2, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_A, "A", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_S, "S", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_D, "D", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F, "F", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_G, "G", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_H, "H", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_J, "J", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_K, "K", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_L, "L", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SEMICOLON, ";", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACKSLASH, "\"", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_QUOTE, "'", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_RETURN, "ENTER", 2, 0);
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SHIFT, "SHIFT", 3, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_Z, "Z", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_X, "X", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_C, "C", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_V, "V", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_B, "B", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_N, "N", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_M, "M", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_COMMA, ",", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_PERIOD, ".", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SLASH, "/", 0, 0);
	kbdbox_addkeyproperties(self, main, kc++, "SHIFT", 3, 0);	
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_CONTROL, "CTRL", 2, 1);
	kbdbox_addkeyproperties(self, main, kc++, "", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_MENU, "ALT", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SPACE, "Psycle", 4, 0);
	kbdbox_addkeyproperties(self, main, kc++, "ALT", 1, 0);	
	kbdbox_addkeyproperties(self, main, kc++, "CTRL", 1, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_LEFT, "Left", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_RIGHT, "Right", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_UP, "Up", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DOWN, "Down", 0, 0);
}

void kbdbox_addkeyproperties(KbdBox* self, psy_Property* section,
	uintptr_t keycode, const char* label, int size, int cr)
{
	psy_Property* key;

	key = psy_property_append_section(section, "key");
	psy_property_append_int(key, "keycode", keycode, 0, 0);	
	psy_property_append_string(key, "label", label);
	psy_property_append_int(key, "size", size, 0, 0);
	psy_property_append_int(key, "cr", cr, 0, 0);
}

void kbdbox_addsmallkey(KbdBox* self, uintptr_t keycode, const char* label)
{
	kbdbox_addkey(self, keycode, label, 1.0);	
}

void kbdbox_addmediumkey(KbdBox* self, uintptr_t keycode, const char* label)
{
	kbdbox_addkey(self, keycode, label, 1.5);
}

void kbdbox_addlargerkey(KbdBox* self, uintptr_t keycode, const char* label)
{
	kbdbox_addkey(self, keycode, label, 1.78);	
}

void kbdbox_addlargekey(KbdBox* self, uintptr_t keycode, const char* label)
{
	kbdbox_addkey(self, keycode, label, 2.3);
}

void kbdbox_addwidekey(KbdBox* self, uintptr_t keycode, const char* label)
{
	kbdbox_addkey(self, keycode, label, 3.93);	
}

void kbdbox_addemptykey(KbdBox* self, uintptr_t keycode)
{
	kbdbox_addkey(self, keycode, "EMPTY", 0.25);
}

void kbdbox_addkey(KbdBox* self, uintptr_t keycode, const char* label, double size)
{
	if (!psy_table_exists(&self->keys, keycode)) {
		KbdBoxKey* key;

		key = kbdboxkey_allocinit_all(
			&self->component, &self->component, self->cpx, self->cpy,
			(int)(self->keywidth * size), self->keyheight, label);
		psy_table_insert(&self->keys, keycode, key);
		self->cpx += (int)(self->keywidth * size + self->ident);
	}

}

void kbdbox_markkey(KbdBox* self, uintptr_t keycode)
{
	KbdBoxKey* key;

	key = psy_table_at(&self->keys, keycode);
	if (key) {
		key->marked = TRUE;
		psy_ui_component_invalidate(kbdbox_base(self));
	}
}

void kbdbox_unmarkkey(KbdBox* self, uintptr_t keycode)
{
	KbdBoxKey* key;

	key = psy_table_at(&self->keys, keycode);
	if (key) {
		key->marked = FALSE;
		psy_ui_component_invalidate(kbdbox_base(self));
	}
}

void kbdbox_cleardescriptions(KbdBox* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->keys);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		KbdBoxKey* key;

		key = (KbdBoxKey*)psy_tableiterator_value(&it);
		free(key->desc0);
		free(key->desc1);
		free(key->desc2);
		key->desc0 = strdup("");
		key->desc1 = strdup("");
		key->desc2 = strdup("");
		key->marked = FALSE;
	}
	psy_ui_component_invalidate(kbdbox_base(self));
}

void kbdbox_setdescription(KbdBox* self, uintptr_t keycode, int shift, int ctrl,
	const char* text)
{
	KbdBoxKey* key;

	key = psy_table_at(&self->keys, keycode);
	if (key) {
		uintptr_t maxchars;
		uintptr_t numchars;
		char** desc;

		if (shift && ctrl) {
			desc = &key->desc3;
		} else if (shift) {
			desc = &key->desc1;
		} else  if (ctrl) {
			desc = &key->desc2;
		} else {
			desc = &key->desc0;
		}		
		free(*desc);
		maxchars = 12;
		numchars = psy_min(strlen(text), maxchars);
		if (numchars < strlen(text)) {
			*desc = strdup(text + (strlen(text) - maxchars));
		} else {
			*desc = strdup(text);
		}
		psy_ui_component_invalidate(kbdbox_base(self));
	}
}

void kbdbox_onpreferredsize(KbdBox* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_RealRectangle bounds;

	bounds = kbdbox_bounds(self);
	psy_ui_size_setpx(rv, bounds.right, bounds.bottom);	
}

psy_ui_RealRectangle kbdbox_bounds(KbdBox* self)
{
	psy_ui_RealRectangle rv;
	psy_TableIterator it;

	rv = psy_ui_realrectangle_zero();
	for (it = psy_table_begin(&self->keys);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(
			((psy_ui_Component*)psy_tableiterator_value(&it)));
		psy_ui_realrectangle_union(&rv, &r);
	}	
	psy_ui_realrectangle_expand(&rv, 0.0, 0.0, 0.0, 0.0);
	return rv;
}
