// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEARCHFIELD_H)
#define SEARCHFIELD_H

// ui
#include <uitextarea.h>
#include <uiimage.h>

#ifdef __cplusplus
extern "C" {
#endif

// NewMachine
		
struct NewMachine;

typedef struct SearchField {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_changed;
	// intern
	psy_ui_Image image;
	psy_ui_TextArea edit;
	bool hasdefaulttext;
	char* defaulttext;	
} SearchField;

void searchfield_init(SearchField*, psy_ui_Component* parent);

void searchfield_setdefaulttext(SearchField*, const char* text);
const char* searchfield_text(const SearchField*);

INLINE psy_ui_Component* searchfield_base(SearchField* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SEARCHFIELD_H */
