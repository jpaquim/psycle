/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMBOBOX_H
#define psy_ui_COMBOBOX_H

/* local */
#include "uidropdownbox.h"
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

typedef enum psy_ui_ComboBoxHover {
    psy_ui_COMBOBOXHOVER_NONE = 0,
    psy_ui_COMBOBOXHOVER_DOWN,
    psy_ui_COMBOBOXHOVER_LESS,
    psy_ui_COMBOBOXHOVER_MORE
} psy_ui_ComboBoxHover;

typedef struct psy_ui_ComboBox {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;
    /* internal */    
    psy_ui_DropDownBox dropdown;
    psy_ui_ListBox listbox;
    int ownerdrawn;   
    psy_ui_ComboBoxHover hover;
    double charnumber;
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
