// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_BUTTON_H
#define psy_ui_BUTTON_H

#include "uicomponent.h"
#include "uiicondraw.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_ui_Button
//
// Displays a text field and/or a predefined Icon (psy_ui_ButtonIcon).
// Clicking with the mouse on it triggers the Signal signal_clicked.
// Style classes: psy_ui_STYLE_BUTTON (_HOVER, _SELECT)
// Callback signature: (void) -> (Context*) X (psy_ui_Button*)
//
// Example:
//
// 1a: psy_ui_button_init_text_connect(&example.button, "ok",
//         self, onbuttonclick);
//   Parameters:
//    - &example.button: reference to the button
//    - "ok": text to be displayed (translated)
//    - self: callback context
//    - onbuttonclick: functionpointer
// or
// 1b: psy_ui_button_init(&example.button);
//    psy_ui_button_preventtranslation(&example.button);
//    psy_ui_button_settext(&example.button, "ok");
//	  psy_ui_button_allowrightclick(&example.button);
//    psy_signal_connect(&example.button.signal_clicked, self, onbuttonclick);
//
// The function callback notified if the button click is triggered
// 2: void onbuttonclick(Example* self, psy_ui_Button* sender) { }
//
// psy_ui_Component <>----<> psy_ui_ComponentImp
//      ^
//      |                         
//      |
// psy_ui_Button

typedef struct psy_ui_Button {
	// inherits
	psy_ui_Component component;	
	// signals
	psy_Signal signal_clicked;
	// internal
	char_dyn_t* text;	
	int highlight;
	psy_ui_ButtonIcon icon;
	psy_ui_Bitmap bitmapicon;
	double charnumber;
	double linespacing;
	double bitmapident;
	psy_ui_Alignment textalignment;	
	bool shiftstate;
	bool ctrlstate;
	bool buttonstate;
	bool allowrightclick;
	char* translation;
	bool translate;	
	bool stoppropagation;
	uintptr_t data;
} psy_ui_Button;

void psy_ui_button_init(psy_ui_Button*, psy_ui_Component* parent,
	psy_ui_Component* view);
void psy_ui_button_init_text(psy_ui_Button*, psy_ui_Component* parent,
	psy_ui_Component* view, const char* text);
void psy_ui_button_init_connect(psy_ui_Button*, psy_ui_Component* parent,
	psy_ui_Component* view, void* context, void* fp);
void psy_ui_button_init_text_connect(psy_ui_Button*, psy_ui_Component* parent,
	psy_ui_Component* view, const char* text, void* context, void* fp);

psy_ui_Button* psy_ui_button_alloc(void);
psy_ui_Button* psy_ui_button_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view);

void psy_ui_button_settext(psy_ui_Button*, const char* text);
void psy_ui_button_seticon(psy_ui_Button*, psy_ui_ButtonIcon);
void psy_ui_button_highlight(psy_ui_Button*);
void psy_ui_button_disablehighlight(psy_ui_Button*);
bool psy_ui_button_highlighted(const psy_ui_Button*);
void psy_ui_button_settextcolour(psy_ui_Button*, psy_ui_Colour);
void psy_ui_button_setcharnumber(psy_ui_Button*, double number);
void psy_ui_button_setlinespacing(psy_ui_Button*, double spacing);
void psy_ui_button_settextalignment(psy_ui_Button*, psy_ui_Alignment);
void psy_ui_button_preventtranslation(psy_ui_Button*);


INLINE bool psy_ui_button_hasctrl(const psy_ui_Button* self)
{
	assert(self);

	return self->ctrlstate;
}

INLINE bool psy_ui_button_clickstate(const psy_ui_Button* self)
{
	assert(self);

	return self->buttonstate;
}

INLINE void psy_ui_button_allowrightclick(psy_ui_Button* self)
{
	assert(self);

	self->allowrightclick = TRUE;
}

// default
INLINE void psy_ui_button_preventrightclick(psy_ui_Button* self)
{
	assert(self);

	self->allowrightclick = FALSE;
}

INLINE bool psy_ui_button_hasshift(const psy_ui_Button* self)
{
	assert(self);

	return self->shiftstate;
}

INLINE psy_ui_Component* psy_ui_button_base(psy_ui_Button* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BUTTON_H */
