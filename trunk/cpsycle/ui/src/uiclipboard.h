/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_CLIPBOARD_H
#define psy_ui_CLIPBOARD_H


#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Bitmap;

typedef struct psy_ui_Clipboard {	
	char* text;	
} psy_ui_Clipboard;

void psy_ui_clipboard_init(psy_ui_Clipboard*);
void psy_ui_clipboard_dispose(psy_ui_Clipboard*);

void psy_ui_clipboard_set_text(psy_ui_Clipboard*, const char* text);
const char* psy_ui_clipboard_text(const psy_ui_Clipboard*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_CLIPBOARD */
