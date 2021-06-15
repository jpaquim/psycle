/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMBOBOX_H
#define psy_ui_COMBOBOX_H

/* local */
#include "uibutton.h"
#include "uidropdownbox.h"
#include "uilabel.h"
#include "uilistbox.h"
/* container */
#include <hashtbl.h>

/*
** ComboBox
** Bridge
** Aim: avoid coupling to one platform (win32, x11, etc)
** Abstraction/Refined  psy_ui_ComboBox
** Implementor			psy_ui_ComponentImp
** Concrete Implementor	psy_ui_win_ComponentImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp
**         ^                                                        
**         |                                                        
**         |                               
** psy_ui_ComboBox <>----<>  psy_ui_DropDownBox <>----<> psy_ui_ListBox
**
*/

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ComponentImp;

typedef struct psy_ui_ComboBox {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;
    /* internal */
    psy_ui_DropDownBox dropdown;
    psy_ui_ListBox listbox;
    psy_ui_Button less;
    psy_ui_Button more;    
    psy_ui_Button expand;
    psy_ui_Label textfield;        
    psy_Table itemdata;   
} psy_ui_ComboBox;

void psy_ui_combobox_init(psy_ui_ComboBox*, psy_ui_Component* parent,
    psy_ui_Component* view);

psy_ui_ComboBox* psy_ui_combobox_alloc(void);
psy_ui_ComboBox* psy_ui_combobox_allocinit(psy_ui_Component* parent, 
    psy_ui_Component* view);

intptr_t psy_ui_combobox_addtext(psy_ui_ComboBox*, const char* text);
void psy_ui_combobox_clear(psy_ui_ComboBox*);
void psy_ui_combobox_setcursel(psy_ui_ComboBox*, intptr_t index);
intptr_t psy_ui_combobox_cursel(const psy_ui_ComboBox*);
void psy_ui_combobox_setcharnumber(psy_ui_ComboBox*, double num);
void psy_ui_combobox_setitemdata(psy_ui_ComboBox*, uintptr_t index, intptr_t data);
intptr_t psy_ui_combobox_itemdata(psy_ui_ComboBox*, uintptr_t index);
void psy_ui_combobox_text(psy_ui_ComboBox*, char* text);
intptr_t psy_ui_combobox_count(const psy_ui_ComboBox*);


INLINE psy_ui_Component* psy_ui_combobox_base(psy_ui_ComboBox* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMBOBOX_H */
