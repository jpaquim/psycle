/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(INPUTDEFINER_H)
#define INPUTDEFINER_H

/* ui */
#include <uilabel.h>
#include <uibutton.h>

/*
** InputDefiner
**
** Component to define a keyboard shortcut. It is used by the SettingsView to
** edit the keyboard settings.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* InputDefinerKeyNames */
typedef struct InputDefinerKeyNames {
	psy_Table container;
} InputDefinerKeyNames;

void inputdefinerkeynames_init(InputDefinerKeyNames*);
void inputdefinerkeynames_dispose(InputDefinerKeyNames*);

void inputdefinerkeynames_add(InputDefinerKeyNames*,
	uintptr_t keycode, const char* name);
const char* inputdefinerkeynames_at(const InputDefinerKeyNames*,
	uintptr_t keycode);


/* InputDefiner */
typedef struct InputDefiner {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_ui_Label label;
	psy_ui_Button clear;
	psy_Signal signal_accept;
	/* internal */
	uint32_t input;
	uint32_t old_input;
	int regular_key;
	bool changed;
} InputDefiner;

void inputdefiner_init(InputDefiner*, psy_ui_Component* parent);

InputDefiner* inputdefiner_alloc(void);
InputDefiner* inputdefiner_allocinit(psy_ui_Component* parent);

void inputdefiner_set_input(InputDefiner*, uint32_t input);

INLINE unsigned int inputdefiner_input(const InputDefiner* self)
{
	assert(self);

	return self->input;
}

void inputdefiner_text(InputDefiner*, char* text);
void inputdefiner_input_to_text(uint32_t input, char* text);

INLINE psy_ui_Component* inputdefiner_base(InputDefiner* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* INPUTDEFINER_H */
