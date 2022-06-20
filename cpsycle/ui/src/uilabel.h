/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_LABEL_H
#define psy_ui_LABEL_H

/* local */
#include "uicomponent.h"
#include "uitextformat.h"

/*
** psy_ui_Label
**
**  Displays a text field
**
**  psy_ui_Component <>----<> psy_ui_ComponentImp
**      ^
**      |
**  psy_ui_Label
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Label {
    /* inherits */
    psy_ui_Component component;
    /* internal */
    double charnumber;    
    char* text;
    char* defaulttext;
    char* translation;    
    bool translate;
    bool fadeout;    
    uintptr_t fadeoutcounter;
    psy_ui_TextFormat format;
} psy_ui_Label;

void psy_ui_label_init(psy_ui_Label*, psy_ui_Component* parent);
void psy_ui_label_init_text(psy_ui_Label*, psy_ui_Component* parent,
    const char* text);

psy_ui_Label* psy_ui_label_alloc(void);
psy_ui_Label* psy_ui_label_allocinit(psy_ui_Component* parent);

/* copies the text to the label */
void psy_ui_label_set_text(psy_ui_Label*, const char* text);
/* adds a copy of the text to the label */
void psy_ui_label_add_text(psy_ui_Label*, const char* text);
/* returns the text owned by the label */
const char* psy_ui_label_text(const psy_ui_Label*);
void psy_ui_label_set_default_text(psy_ui_Label*, const char* text);
void psy_ui_label_set_char_number(psy_ui_Label*, double number);
void psy_ui_label_set_line_spacing(psy_ui_Label*, double spacing);
void psy_ui_label_set_textalignment(psy_ui_Label*, psy_ui_Alignment);
void psy_ui_label_prevent_translation(psy_ui_Label*);
void psy_ui_label_enable_translation(psy_ui_Label*);
void psy_ui_label_fadeout(psy_ui_Label*);
void psy_ui_label_prevent_wrap(psy_ui_Label*);
void psy_ui_label_enable_wrap(psy_ui_Label*);

INLINE psy_ui_Component* psy_ui_label_base(psy_ui_Label* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LABEL_H */
