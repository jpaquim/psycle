/* 
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TEXTAREA_H
#define psy_ui_TEXTAREA_H

/* local */
#include "uiscroller.h"
#include "uitextformat.h"

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
    psy_ui_TextFormat format;
    uintptr_t cp;     
} psy_ui_TextAreaPane;

void psy_ui_textareapane_init(psy_ui_TextAreaPane*, psy_ui_Component* parent);

void psy_ui_textareapane_enable_inputfield(psy_ui_TextAreaPane*);
void psy_ui_textareapane_set_text(psy_ui_TextAreaPane*, const char* text);
void psy_ui_textareapane_add_text(psy_ui_TextAreaPane*, const char* text);
const char* psy_ui_textareapane_text(const psy_ui_TextAreaPane* self);
void psy_ui_textareapane_set_char_number(psy_ui_TextAreaPane*, double number);
void psy_ui_textareapane_set_line_number(psy_ui_TextAreaPane*, int number);
void psy_ui_textareapane_enable_edit(psy_ui_TextAreaPane*);
void psy_ui_textareapane_prevent_edit(psy_ui_TextAreaPane*);
void psy_ui_textareapane_set_sel(psy_ui_TextAreaPane*, uintptr_t cpmin,
    uintptr_t cpmax);
uintptr_t psy_ui_textareapane_cursor_line(const psy_ui_TextAreaPane*);
uintptr_t psy_ui_textareapane_cursor_column(const psy_ui_TextAreaPane*);

INLINE psy_ui_Component* psy_ui_textareapane_base(psy_ui_TextAreaPane* self)
{
    return &self->component;
}

/* psy_ui_TextArea */
typedef struct psy_ui_TextArea {
    /* inherits */
    psy_ui_Component component;
    /* signals */
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
    psy_ui_Scroller scroller;
    psy_ui_TextAreaPane pane;
} psy_ui_TextArea;

void psy_ui_textarea_init(psy_ui_TextArea*, psy_ui_Component* parent);
void psy_ui_textarea_init_single_line(psy_ui_TextArea*, psy_ui_Component* parent);

psy_ui_TextArea* psy_ui_textarea_alloc(void);
psy_ui_TextArea* psy_ui_textarea_allocinit(psy_ui_Component* parent);
psy_ui_TextArea* psy_ui_textarea_allocinit_single_line(psy_ui_Component* parent);

INLINE void psy_ui_textarea_enable_input_field(psy_ui_TextArea* self)
{
    psy_ui_textareapane_enable_inputfield(&self->pane);
}

INLINE void psy_ui_textarea_set_text(psy_ui_TextArea* self, const char* text)
{
    psy_ui_textareapane_set_text(&self->pane, text);
}

INLINE void psy_ui_textarea_add_text(psy_ui_TextArea* self, const char* text)
{
    psy_ui_textareapane_add_text(&self->pane, text);
}

INLINE const char* psy_ui_textarea_text(const psy_ui_TextArea* self)
{
    return psy_ui_textareapane_text(&self->pane);
}

uintptr_t psy_ui_textarea_length(const psy_ui_TextArea*);
void psy_ui_textarea_range(psy_ui_TextArea*, intptr_t start, intptr_t end, char* text);

INLINE void psy_ui_textarea_setcharnumber(psy_ui_TextArea* self, int number)
{
    psy_ui_textareapane_set_char_number(&self->pane, number);
}

INLINE void psy_ui_textarea_setlinenumber(psy_ui_TextArea* self, int number)
{
    psy_ui_textareapane_set_line_number(&self->pane, number);
}

INLINE void psy_ui_textarea_enableedit(psy_ui_TextArea* self)
{
    psy_ui_textareapane_enable_edit(&self->pane);
}

INLINE void psy_ui_textarea_preventedit(psy_ui_TextArea* self)
{
    psy_ui_textareapane_prevent_edit(&self->pane);
}

INLINE void psy_ui_textarea_setsel(psy_ui_TextArea* self,
    uintptr_t cpmin, uintptr_t cpmax)
{
    psy_ui_textareapane_set_sel(&self->pane, cpmin, cpmax);
}

void psy_ui_textarea_prevent_wrap(psy_ui_TextArea*);
void psy_ui_textarea_word_wrap(psy_ui_TextArea*);
void psy_ui_textarea_line_wrap(psy_ui_TextArea*);
uintptr_t psy_ui_textarea_cursor_line(const psy_ui_TextArea*);
uintptr_t psy_ui_textarea_cursor_column(const psy_ui_TextArea*);

INLINE psy_ui_Component* psy_ui_textarea_base(psy_ui_TextArea* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TEXTAREA_H */
