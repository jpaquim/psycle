/* 
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TEXTINPUT_H
#define psy_ui_TEXTINPUT_H

#include "uicomponent.h"

/*
** Edit
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_TextInput
** Implementor			psy_ui_ComponentImp
** Concrete Implementor	psy_ui_win_ComponentImp/psy_ui_x11_ComponentImp/
**                      psy_ui_ViewComponentImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp
**      ^                     
**      |                     
**      |                     
** psy_ui_TextInput           
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_TextInput {
    /* inherits */
	psy_ui_Component component;
    /* signals */
    /* emits any change */
    psy_Signal signal_change;
    /*
    ** emits if edit is inputfield and
    ** - return pressed or
    ** - focus lost or
    ** - clicked outside
    */
    psy_Signal signal_accept;
    /*
    ** emits if edit is inputfield and
    ** - esc pressed
    */
    psy_Signal signal_reject;
    /* internal */
	int charnumber;
	int linenumber;
    bool isinputfield;
    bool preventedit;
    char* text;
    uintptr_t cp;
} psy_ui_TextInput;

void psy_ui_textinput_init(psy_ui_TextInput*, psy_ui_Component* parent);

void psy_ui_textinput_enableinputfield(psy_ui_TextInput*);
void psy_ui_textinput_settext(psy_ui_TextInput*, const char* text);
const char* psy_ui_textinput_text(const psy_ui_TextInput* self);
void psy_ui_textinput_setcharnumber(psy_ui_TextInput*, int number);
void psy_ui_textinput_setlinenumber(psy_ui_TextInput*, int number);
void psy_ui_textinput_enableedit(psy_ui_TextInput*);
void psy_ui_textinput_preventedit(psy_ui_TextInput*);
void psy_ui_textinput_setsel(psy_ui_TextInput* self, intptr_t cpmin, intptr_t cpmax);

INLINE psy_ui_Component* psy_ui_textinput_base(psy_ui_TextInput* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TEXTINPUT_H */
