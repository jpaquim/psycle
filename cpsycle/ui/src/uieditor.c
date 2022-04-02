/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uieditor.h"

#define BLOCKSIZE 128 * 1024

/* local */
/* platform */
#include "../../detail/portable.h"


static void psy_ui_editor_setfont(psy_ui_Editor*, psy_ui_Font*);
static void psy_ui_editor_getrange(psy_ui_Editor*, intptr_t start, intptr_t end,
	char* text);

/* implementation */
void psy_ui_editor_init(psy_ui_Editor* self, psy_ui_Component* parent)
{  	
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_textarea_init(&self->textarea, &self->component);
	psy_ui_component_set_style_type(&self->textarea.pane.component, psy_ui_STYLE_EDITOR);
	psy_ui_textarea_prevent_wrap(&self->textarea);
	psy_ui_textarea_line_wrap(&self->textarea);
	psy_ui_component_set_align(&self->textarea.component, psy_ui_ALIGN_CLIENT);
}

void psy_ui_editor_load(psy_ui_Editor* self, const char* path)
{	
	FILE* fp;

	psy_ui_textarea_settext(&self->textarea, "");
	fp = fopen(path, "rb");
	if (fp) {
		char data[BLOCKSIZE];
		uintptr_t lenfile;

		memset(data, 0, BLOCKSIZE);
		lenfile = fread(data, 1, sizeof(data), fp);
		while (lenfile > 0) {
			psy_ui_textarea_addtext(&self->textarea, (char*)data);
			lenfile = fread(data, 1, sizeof(data), fp);
		}
		fclose(fp);
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(psy_ui_textarea_base(&self->textarea));
	}
}

void psy_ui_editor_save(psy_ui_Editor* self, const char* path)
{
	FILE* fp;

	fp = fopen(path, "wb");
	if (fp) {
		char data[BLOCKSIZE + 1];
		intptr_t lengthdoc;
		intptr_t i;

		lengthdoc = psy_ui_textarea_length(&self->textarea);
		for (i = 0; i < lengthdoc; i += BLOCKSIZE) {
			intptr_t grabsize;
			
			grabsize = lengthdoc - i;
			if (grabsize > BLOCKSIZE)
				grabsize = BLOCKSIZE;
			psy_ui_editor_getrange(self, i, i + grabsize, data);
			fwrite(data, grabsize, 1, fp);
		}
		fclose(fp);		
	}
}

void psy_ui_editor_getrange(psy_ui_Editor* self, intptr_t start, intptr_t end, char* text)
{
	psy_ui_textarea_range(&self->textarea, start, end, text);
}

void psy_ui_editor_settext(psy_ui_Editor* self, const char* text)
{
	psy_ui_textarea_settext(&self->textarea, text);	
}

void psy_ui_editor_addtext(psy_ui_Editor* self, const char* text)
{
	psy_ui_textarea_addtext(&self->textarea, text);	
}

char* psy_ui_editor_text(psy_ui_Editor* self, uintptr_t maxlength, char* rv)
{
	const char* text;


	text = psy_ui_textarea_text(&self->textarea);
	psy_snprintf(rv, maxlength, text);
	return rv;
}

uintptr_t psy_ui_editor_length(psy_ui_Editor* self)
{
	return psy_ui_textarea_length(&self->textarea);	
}

void psy_ui_editor_clear(psy_ui_Editor* self)
{
	psy_ui_textarea_settext(&self->textarea, "");
}

void psy_ui_editor_preventedit(psy_ui_Editor* self)
{	
	psy_ui_textarea_preventedit(&self->textarea);	
}

void psy_ui_editor_enableedit(psy_ui_Editor* self)
{
	psy_ui_textarea_enableedit(&self->textarea);
}

void psy_ui_editor_wrap(psy_ui_Editor* self)
{
	psy_ui_textarea_word_wrap(&self->textarea);	
}

void psy_ui_editor_prevent_wrap(psy_ui_Editor* self)
{
	psy_ui_textarea_prevent_wrap(&self->textarea);	
}

void psy_ui_editor_gotoline(psy_ui_Editor* self, uintptr_t line)
{
	// sci(self, SCI_GOTOLINE, line, 0);
}

uintptr_t psy_ui_editor_cursor_line(const psy_ui_Editor* self)
{
	return psy_ui_textarea_cursor_line(&self->textarea);
}
