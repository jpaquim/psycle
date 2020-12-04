// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_LABEL_H
#define psy_ui_LABEL_H

#include "uicomponent.h"

// Label
//
// aim: displays a text field
//
// psy_ui_Component <>----<> psy_ui_ComponentImp
//      ^
//      |                         
//      |
// psy_ui_Label

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    psy_ui_Component component;
    int charnumber;
    double linespacing;
    psy_ui_Alignment textalignment;
    char* text;
    char* translation;
    bool translate;
} psy_ui_Label;

void psy_ui_label_init(psy_ui_Label*, psy_ui_Component* parent);
void psy_ui_label_init_text(psy_ui_Label*, psy_ui_Component* parent,
    const char* text);
void psy_ui_label_settext(psy_ui_Label*, const char* text);
void psy_ui_label_text(psy_ui_Label*, char* text);
void psy_ui_label_setcharnumber(psy_ui_Label*, int number);
void psy_ui_label_setlinespacing(psy_ui_Label*, double spacing);
// void psy_ui_label_setstyle(psy_ui_Label*, int style);
void psy_ui_label_settextalignment(psy_ui_Label*, psy_ui_Alignment alignment);
void psy_ui_label_preventtranslation(psy_ui_Label*);

INLINE psy_ui_Component* psy_ui_label_base(psy_ui_Label* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LABEL_H */
