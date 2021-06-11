/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_ICONDRAW_H
#define psy_ui_ICONDRAW_H

/* local */
#include "uigraphics.h"
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_ICON_NONE,
	psy_ui_ICON_LESS,
	psy_ui_ICON_MORE,
	psy_ui_ICON_MOREMORE,
	psy_ui_ICON_LESSLESS,
	psy_ui_ICON_UP,
	psy_ui_ICON_DOWN
} psy_ui_ButtonIcon;

psy_ui_RealSize psy_ui_buttonicon_size(psy_ui_ButtonIcon);

typedef struct psy_ui_IconDraw {
	/* internal */
	psy_ui_ButtonIcon icon;		
} psy_ui_IconDraw;

void psy_ui_icondraw_init(psy_ui_IconDraw*, psy_ui_ButtonIcon);
void psy_ui_icondraw_draw(psy_ui_IconDraw*, psy_ui_Graphics*,
	psy_ui_RealPoint, psy_ui_Colour);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_ICONDRAW_H */
