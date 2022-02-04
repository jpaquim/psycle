/* 
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TEXTAREA_H
#define psy_ui_TEXTAREA_H

#include "uiscroller.h"

/*
** Edit
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_TextArea
** Implementor			psy_ui_ComponentImp
** Concrete Implementor	psy_ui_win_ComponentImp/psy_ui_x11_ComponentImp/
**                      psy_ui_ViewComponentImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp
**      ^                     
**      |                     
**      |                     
** psy_ui_TextArea           
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_TextAreaPane {
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
	double charnumber;
	int linenumber;
    bool isinputfield;
    bool preventedit;
    char* text;
    uintptr_t cp;
} psy_ui_TextAreaPane;

void psy_ui_textareapane_init(psy_ui_TextAreaPane*, psy_ui_Component* parent);

void psy_ui_textareapane_enableinputfield(psy_ui_TextAreaPane*);
void psy_ui_textareapane_settext(psy_ui_TextAreaPane*, const char* text);
const char* psy_ui_textareapane_text(const psy_ui_TextAreaPane* self);
void psy_ui_textareapane_setcharnumber(psy_ui_TextAreaPane*, double number);
void psy_ui_textareapane_setlinenumber(psy_ui_TextAreaPane*, int number);
void psy_ui_textareapane_enableedit(psy_ui_TextAreaPane*);
void psy_ui_textareapane_preventedit(psy_ui_TextAreaPane*);
void psy_ui_textareapane_setsel(psy_ui_TextAreaPane*, uintptr_t cpmin,
    uintptr_t cpmax);

INLINE psy_ui_Component* psy_ui_textareapane_base(psy_ui_TextAreaPane* self)
{
    return &self->component;
}

/* psy_ui_TextArea */
typedef struct psy_ui_TextArea {
    psy_ui_Component component;
    psy_ui_Scroller scroller;
    psy_ui_TextAreaPane pane;
} psy_ui_TextArea;

void psy_ui_textarea_init(psy_ui_TextArea*, psy_ui_Component* parent);

INLINE void psy_ui_textarea_enableinputfield(psy_ui_TextArea* self)
{
    psy_ui_textareapane_enableinputfield(&self->pane);
}

INLINE void psy_ui_textarea_settext(psy_ui_TextArea* self, const char* text)
{
    psy_ui_textareapane_settext(&self->pane, text);
}

INLINE  const char* psy_ui_textarea_text(const psy_ui_TextArea* self)
{
    return psy_ui_textareapane_text(&self->pane);
}

INLINE void psy_ui_textarea_setcharnumber(psy_ui_TextArea* self, int number)
{
    psy_ui_textareapane_setcharnumber(&self->pane, number);
}

INLINE void psy_ui_textarea_setlinenumber(psy_ui_TextArea* self, int number)
{
    psy_ui_textareapane_setlinenumber(&self->pane, number);
}

INLINE void psy_ui_textarea_enableedit(psy_ui_TextArea* self)
{
    psy_ui_textareapane_enableedit(&self->pane);
}

INLINE void psy_ui_textarea_preventedit(psy_ui_TextArea* self)
{
    psy_ui_textareapane_preventedit(&self->pane);
}

INLINE void psy_ui_textarea_setsel(psy_ui_TextArea* self,
    uintptr_t cpmin, uintptr_t cpmax)
{
    psy_ui_textareapane_setsel(&self->pane, cpmin, cpmax);
}

INLINE psy_ui_Component* psy_ui_textarea_base(psy_ui_TextArea* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TEXTAREA_H */
