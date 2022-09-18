/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_STYLEPROPERTY_H
#define psy_ui_STYLEPROPERTY_H

/* local */
#include "uistyles.h"
/* container */
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

void psy_ui_styles_load(psy_Property* styles, const char* path);
void psy_ui_styles_save(psy_Property* styles, const char* path);

psy_Property* psy_ui_styles_property_make(psy_ui_Styles*);
void psy_ui_styles_property_append(psy_Property* parent, psy_ui_Styles*);
void psy_ui_style_property_make(psy_Property* parent, psy_ui_Style*);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_STYLEPROPERTY_H */
