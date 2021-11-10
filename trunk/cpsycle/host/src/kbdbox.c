/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "kbdbox.h"
/* host */
#include "styles.h"

static double keyheight = 4.5;
static double keybasewidth = 20.0;
static double keywidths[] = { 1.0, 1.5, 1.75, 2.27, 3.8, 0.25 };

void kbdboxstate_init(KbdBoxState* self)
{
	self->pressedkey = 0;
	kbdboxstate_clearmodifier(self);
}

void kbdboxstate_clearmodifier(KbdBoxState* self)
{	
	self->shift = FALSE;
	self->ctrl = FALSE;
	self->alt = FALSE;
}

/* KbdBoxKey */
/* prototypes */
static void kbdboxkey_initlabel(KbdBoxKey*, psy_ui_Component* view,
	psy_ui_Label*, const char* text);
static void kbdboxkey_initstyle(KbdBoxKey*);
static void kbdboxkey_onmousedown(KbdBoxKey*, psy_ui_MouseEvent*);
/* vtable */
static psy_ui_ComponentVtable kbdboxkey_vtable;
static bool kbdboxkey_vtable_initialized = FALSE;

static void kbdboxkey_vtable_init(KbdBoxKey* self)
{
	if (!kbdboxkey_vtable_initialized) {
		kbdboxkey_vtable = *(self->component.vtable);		
		kbdboxkey_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			kbdboxkey_onmousedown;
		kbdboxkey_vtable_initialized = TRUE;
	}
	self->component.vtable = &kbdboxkey_vtable;
}
/* implementation */
void kbdboxkey_init_all(KbdBoxKey* self, psy_ui_Component* parent, psy_ui_Component* view, 
	uintptr_t size, uint32_t keycode, const char* label, Workspace* workspace, KbdBoxState* state)
{
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_px(2.0, 0.0, 0.0, 2.0));
	kbdboxkey_vtable_init(self);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());	
	self->keycode = keycode;
	self->state = state;
	self->workspace = workspace;			
	kbdboxkey_initlabel(self, view, &self->label, label);
	kbdboxkey_initlabel(self, view, &self->desc0, "");
	kbdboxkey_initlabel(self, view, &self->desc1, "");
	kbdboxkey_initlabel(self, view, &self->desc2, "");
	kbdboxkey_initlabel(self, view, &self->desc3, "");
	if (size != 5) { /* not empty */
		kbdboxkey_initstyle(self);
	}
	size = psy_max(0, psy_min(5, size));
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(keybasewidth * keywidths[size], keyheight));
}

KbdBoxKey* kbdboxkey_allocinit_all(psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t size, uint32_t keycode, const char* label,
	Workspace* workspace, KbdBoxState* state)
{
	KbdBoxKey* rv;

	rv = (KbdBoxKey*)malloc(sizeof(KbdBoxKey));
	if (rv) {
		kbdboxkey_init_all(rv, parent, view, size, keycode, label,
			workspace, state);
		psy_ui_component_deallocateafterdestroyed(&rv->component);		
	}
	return rv;
}

void kbdboxkey_initlabel(KbdBoxKey* self, psy_ui_Component* view,
	psy_ui_Label* label, const char* text)
{
	psy_ui_label_init(label, &self->component, view);
	psy_ui_label_preventtranslation(label);
	psy_ui_label_setcharnumber(label, 8.0);
	psy_ui_label_settextalignment(label, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_settext(label, text);
}

void kbdboxkey_initstyle(KbdBoxKey* self)
{	
	uintptr_t select_style_id;

	switch (self->keycode) {
	case  psy_ui_KEY_SHIFT:
		select_style_id = STYLE_KEY_SHIFT_SELECT;
		break;
	case  psy_ui_KEY_MENU:
		select_style_id = STYLE_KEY_ALT_SELECT;
		break;
	case  psy_ui_KEY_CONTROL:
		select_style_id = STYLE_KEY_CTRL_SELECT;
		break;
	default:
		select_style_id = STYLE_KEY_SELECT;
		break;
	}
	psy_ui_component_setstyletypes(&self->component,
		STYLE_KEY, STYLE_KEY, select_style_id, psy_INDEX_INVALID);	
}

void kbdboxkey_cleardescriptions(KbdBoxKey* self)
{
	psy_ui_label_settext(&self->desc0, "");
	psy_ui_label_settext(&self->desc1, "");
	psy_ui_label_settext(&self->desc2, "");
	psy_ui_label_settext(&self->desc3, "");	
}

void kbdboxkey_setdescription(KbdBoxKey* self, uint32_t keycode,
	const char* text)
{		
	bool shift;
	bool ctrl;
	bool alt;
	bool up;
	
	psy_audio_decodeinput((uint32_t)keycode, &keycode, &shift, &ctrl, &alt, &up);
	if (shift && ctrl) {
		psy_ui_label_settext(&self->desc3, text);
	} else if (shift) {
		psy_ui_label_settext(&self->desc1, text);
	} else  if (ctrl) {
		psy_ui_label_settext(&self->desc2, text);
	} else {
		psy_ui_label_settext(&self->desc0, text);
	}	
}

void kbdboxkey_onmousedown(KbdBoxKey* self, psy_ui_MouseEvent* ev)
{
	self->state->pressedkey = self->keycode;
	if (self->keycode == psy_ui_KEY_SHIFT) {
		self->state->shift = !self->state->shift;		
		if (self->state->shift) {
			psy_ui_component_addstylestate(&self->component,
				psy_ui_STYLESTATE_SELECT);			
		} else {
			psy_ui_component_removestylestate(&self->component,
				psy_ui_STYLESTATE_SELECT);
		}		
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (self->keycode == psy_ui_KEY_CONTROL) {
		self->state->ctrl = !self->state->ctrl;
		if (self->state->ctrl) {
			psy_ui_component_addstylestate(&self->component,
				psy_ui_STYLESTATE_SELECT);
		} else {
			psy_ui_component_removestylestate(&self->component,
				psy_ui_STYLESTATE_SELECT);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (self->keycode == psy_ui_KEY_MENU) {
		self->state->alt = !self->state->alt;
		if (self->state->alt) {
			psy_ui_component_addstylestate(&self->component,
				psy_ui_STYLESTATE_SELECT);
		} else {
			psy_ui_component_removestylestate(&self->component,
				psy_ui_STYLESTATE_SELECT);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	} else {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);		
	}
}

/* KbdBox */
static void kbdbox_ondestroy(KbdBox*);
static void kbdbox_onmousedown(KbdBox*, psy_ui_MouseEvent*);
static void kbdbox_onmouseup(KbdBox*, psy_ui_MouseEvent*);
static void kbdbox_initfont(KbdBox*);
static void kbdbox_makekeys(KbdBox*);
static psy_Property* kbdbox_definekeys(KbdBox*);
static void kbdbox_definekey(KbdBox*, psy_Property* section,
	uintptr_t keycode, const char* label, int size, int cr);
static void kbdbox_addrow(KbdBox*);
static void kbdbox_addkey(KbdBox*, uint32_t keycode, uintptr_t size,
	const char* label);
static void kbdbox_resetmodstates(KbdBox*);
static void kbdbox_oninput(KbdBox*, psy_EventDriver* sender);
// vtable
static psy_ui_ComponentVtable kbdbox_vtable;
static bool kbdbox_vtable_initialized = FALSE;

static void kbdbox_vtable_init(KbdBox* self)
{
	if (!kbdbox_vtable_initialized) {
		kbdbox_vtable = *(self->component.vtable);
		kbdbox_vtable.ondestroy =
			(psy_ui_fp_component_event)
			kbdbox_ondestroy;
		kbdbox_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			kbdbox_onmousedown;
		kbdbox_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			kbdbox_onmouseup;
		kbdbox_vtable_initialized = TRUE;
	}
	self->component.vtable = &kbdbox_vtable;
}
/* implementation */
void kbdbox_init(KbdBox* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(kbdbox_base(self), parent, NULL);
	kbdbox_vtable_init(self);
	psy_ui_component_setdefaultalign(kbdbox_base(self), psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.3, 0.0));
	self->workspace = workspace;
	kbdboxstate_init(&self->state);
	psy_ui_component_doublebuffer(kbdbox_base(self));
	psy_table_init(&self->keys);	
	kbdbox_initfont(self);
	kbdbox_makekeys(self);	
	psy_signal_connect(&self->workspace->player.eventdrivers.signal_input,
		self, kbdbox_oninput);
}

void kbdbox_ondestroy(KbdBox* self)
{
	psy_table_dispose(&self->keys);
}

void kbdbox_initfont(KbdBox* self)
{
	const psy_ui_Font* font;
	
	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;		

		fontinfo = psy_ui_font_fontinfo(font);
		fontinfo.lfHeight = (int)(fontinfo.lfHeight * 0.6);
		psy_ui_component_setfontinfo(&self->component, fontinfo);		
	}
}

void kbdbox_makekeys(KbdBox* self)
{
	psy_Property* keyset;

	psy_ui_component_clear(&self->component);
	keyset = kbdbox_definekeys(self);	
	if (keyset) {		
		psy_List* p;
				
		kbdbox_addrow(self);
		for (p = psy_property_begin(keyset); p != NULL; p = p->next) {				
			psy_Property* property;
			intptr_t keycode;

			property = (psy_Property*)p->entry;				
			keycode = psy_property_at_int(property, "keycode", -1);
			if (keycode != -1) {				
				if (psy_property_at_int(property, "cr", 0)) {
					kbdbox_addrow(self);
				}				
				kbdbox_addkey(self, (uint32_t)keycode,
					(uintptr_t)psy_property_at_int(property, "size", 0),
					psy_property_at_str(property, "label", ""));
			}				
		}	
		psy_property_deallocate(keyset);
		keyset = NULL;
	}
}
	
psy_Property* kbdbox_definekeys(KbdBox* self)
{
	psy_Property* rv;	
	uintptr_t kc;	

	rv = psy_property_allocinit_key(NULL);	
	kc = 0x1000;
	kbdbox_definekey(self, rv, psy_ui_KEY_ESCAPE, "ESC", 1, 0);
	kbdbox_definekey(self, rv, kc++, "", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F1, "F1", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F2, "F2", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F3, "F3", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F4, "F4", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F5, "F5", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F6, "F6", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F7, "F7", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F8, "F8", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F9, "F9", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F10, "F10", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F11, "F11", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F12, "F12", 0, 0);
	kbdbox_definekey(self, rv, kc++, "", 5, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_INSERT, "INS", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_HOME, "HOME", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_PRIOR, "PGUP", 0, 0);
	
	kbdbox_definekey(self, rv, psy_ui_KEY_BACKQUOTE, "`", 0, 1);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT1, "1", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT2, "2", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT3, "3", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT4, "4", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT5, "5", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT6, "6", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT7, "7", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT8, "8", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT9, "9", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DIGIT0, "0", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_MINUS, "-", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_EQUAL, "=", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_BACK, "BACK", 1, 0);
	kbdbox_definekey(self, rv, kc++, "", 5, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DELETE, "DEL", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_END, "END", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_NEXT, "PGDWN", 0, 0);

	kbdbox_definekey(self, rv, psy_ui_KEY_TAB, "TAB", 1, 1);
	kbdbox_definekey(self, rv, psy_ui_KEY_Q, "Q", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_W, "W", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_E, "E", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_R, "R", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_T, "T", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_Y, "Y", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_U, "U", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_I, "I", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_O, "O", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_P, "P", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_BRACKETLEFT, "[", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_BRACKETRIGHT, "]", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_BACKSLASH, "\\", 0, 0);
	
	kbdbox_definekey(self, rv, kc++, "CAPS", 2, 1);
	kbdbox_definekey(self, rv, psy_ui_KEY_A, "A", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_S, "S", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_D, "D", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_F, "F", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_G, "G", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_H, "H", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_J, "J", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_K, "K", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_L, "L", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_SEMICOLON, ";", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_BACKSLASH, "\"", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_QUOTE, "'", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_RETURN, "ENTER", 2, 0);
	
	kbdbox_definekey(self, rv, psy_ui_KEY_SHIFT, "SHIFT", 3, 1);
	kbdbox_definekey(self, rv, psy_ui_KEY_Z, "Z", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_X, "X", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_C, "C", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_V, "V", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_B, "B", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_N, "N", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_M, "M", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_COMMA, ",", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_PERIOD, ".", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_SLASH, "/", 0, 0);
	kbdbox_definekey(self, rv, kc++, "SHIFT", 3, 0);
	
	kbdbox_definekey(self, rv, psy_ui_KEY_CONTROL, "CTRL", 2, 1);
	kbdbox_definekey(self, rv, kc++, "", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_MENU, "ALT", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_SPACE, "Psycle", 4, 0);
	kbdbox_definekey(self, rv, kc++, "ALT", 1, 0);	
	kbdbox_definekey(self, rv, kc++, "CTRL", 1, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_LEFT, "Left", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_RIGHT, "Right", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_UP, "Up", 0, 0);
	kbdbox_definekey(self, rv, psy_ui_KEY_DOWN, "Down", 0, 0);
	return rv;
}

void kbdbox_definekey(KbdBox* self, psy_Property* section,
	uintptr_t keycode, const char* label, int size, int cr)
{
	psy_Property* key;

	key = psy_property_append_section(section, "key");
	psy_property_append_int(key, "keycode", keycode, 0, 0);	
	psy_property_append_str(key, "label", label);
	psy_property_append_int(key, "size", size, 0, 0);
	psy_property_append_int(key, "cr", cr, 0, 0);
}

void kbdbox_addrow(KbdBox* self)
{
	self->currrow = psy_ui_component_allocinit(&self->component,
		&self->component);
	if (self->currrow) {
		psy_ui_component_setdefaultalign(self->currrow, psy_ui_ALIGN_LEFT,
			psy_ui_margin_make_em(0.0, 0.3, 0.0, 0.0));
		psy_ui_component_setalignexpand(self->currrow, psy_ui_HEXPAND);
	}
}

void kbdbox_addkey(KbdBox* self, uint32_t keycode, uintptr_t size,
	const char* label)
{	
	if (self->currrow && !psy_table_exists(&self->keys, keycode)) {
		KbdBoxKey* key;	

		key = kbdboxkey_allocinit_all(self->currrow, &self->component, size,
			keycode, label, self->workspace, &self->state);
		if (key) {
			psy_table_insert(&self->keys, keycode, key);																		
		}
	}
}

void kbdbox_cleardescriptions(KbdBox* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->keys);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		kbdboxkey_cleardescriptions((KbdBoxKey*)psy_tableiterator_value(&it));
	}	
}

void kbdbox_setdescription(KbdBox* self, uint32_t keycode, const char* text)
{
	KbdBoxKey* key;
	uint32_t keycode_decoded;
	bool shift;
	bool ctrl;
	bool alt;
	bool up;

	psy_audio_decodeinput((uint32_t)keycode, &keycode_decoded, &shift, &ctrl, &alt, &up);
	key = (KbdBoxKey*)psy_table_at(&self->keys, keycode_decoded);
	if (key) {		
		kbdboxkey_setdescription(key, keycode, text);		
	}
}

void kbdbox_onmousedown(KbdBox* self, psy_ui_MouseEvent* ev)
{
	if (self->state.pressedkey != 0) {
		psy_EventDriverInput input;

		input.message = psy_EVENTDRIVER_PRESS;
		input.param1 = psy_audio_encodeinput(self->state.pressedkey,
			self->state.shift, self->state.ctrl, self->state.alt, 0);
		input.param2 = workspace_octave(self->workspace) * 12;
		psy_eventdriver_write(workspace_kbddriver(self->workspace), input);		
		kbdbox_resetmodstates(self);
	}
}

void kbdbox_onmouseup(KbdBox* self, psy_ui_MouseEvent* ev)
{
	bool ismod;

	ismod = (self->state.pressedkey == psy_ui_KEY_SHIFT) ||
		(self->state.pressedkey == psy_ui_KEY_CONTROL) ||
		(self->state.pressedkey == psy_ui_KEY_MENU);
	if (self->state.pressedkey != 0 && !ismod) {
		psy_EventDriverInput input;

		input.message = psy_EVENTDRIVER_RELEASE;
		input.param1 = psy_audio_encodeinput(self->state.pressedkey,
			self->state.shift, self->state.ctrl, self->state.alt, 0);
		input.param2 = workspace_octave(self->workspace) * 12;
		psy_eventdriver_write(workspace_kbddriver(self->workspace), input);		
		self->state.pressedkey = 0;
	}	
}

void kbdbox_resetmodstates(KbdBox* self)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(&self->keys);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		KbdBoxKey* key;

		key = (KbdBoxKey*)psy_tableiterator_value(&it);
		if (key->keycode == psy_ui_KEY_SHIFT || key->keycode == psy_ui_KEY_CONTROL
				|| key->keycode == psy_ui_KEY_MENU) {
			psy_ui_component_removestylestate(kbdboxkey_base(key),
				psy_ui_STYLESTATE_SELECT);
		}
	}	
}

void kbdbox_oninput(KbdBox* self, psy_EventDriver* sender)
{
	if (sender != workspace_kbddriver(self->workspace)) {
		return;
	}
	if (psy_ui_component_drawvisible(&self->component)) {
		psy_EventDriverInput input;

		input = psy_eventdriver_input(sender);
		if (input.message == psy_EVENTDRIVER_PRESS) {
			kbdbox_presskey(self, (uint32_t)input.param1);
		} else if (input.message == psy_EVENTDRIVER_RELEASE) {
			kbdbox_releasekey(self, (uint32_t)input.param1);
		}
	}
}

void kbdbox_presskey(KbdBox* self, uint32_t keycode)
{
	KbdBoxKey* key;
	uint32_t keycode_decoded;	
	bool shift;
	bool ctrl;
	bool alt;
	bool up;

	psy_audio_decodeinput(keycode, &keycode_decoded, &shift, &ctrl, &alt, &up);
	key = (KbdBoxKey*)psy_table_at(&self->keys, keycode_decoded);
	if (key) {		
		psy_ui_component_addstylestate(&key->component,
			psy_ui_STYLESTATE_SELECT);
		if (shift) {
			self->state.shift = shift;
		}
		if (ctrl) {
			self->state.shift = ctrl;
		}
		if (alt) {
			self->state.alt = alt;
		}		
	}
}

void kbdbox_releasekey(KbdBox* self, uint32_t keycode)
{
	KbdBoxKey* key;
	uint32_t keycode_decoded;
	bool shift;
	bool ctrl;
	bool alt;
	bool up;

	psy_audio_decodeinput(keycode, &keycode_decoded, &shift, &ctrl, &alt, &up);
	key = (KbdBoxKey*)psy_table_at(&self->keys, keycode_decoded);
	if (key) {		
		psy_ui_component_removestylestate(&key->component,
			psy_ui_STYLESTATE_SELECT);
		kbdboxstate_clearmodifier(&self->state);		
	}
}
