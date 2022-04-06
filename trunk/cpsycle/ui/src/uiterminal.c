/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiterminal.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_terminal_on_destroy(psy_ui_Terminal*);
static void psy_ui_terminal_on_timer(psy_ui_Terminal*, uintptr_t timerid);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Terminal* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			psy_ui_terminal_on_destroy;
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			psy_ui_terminal_on_timer;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_terminal_init(psy_ui_Terminal* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_label_init(&self->output, &self->component);	
	psy_ui_label_enable_wrap(&self->output);
	psy_ui_label_set_charnumber(&self->output, 120.0);
	psy_ui_label_prevent_translation(&self->output);
	psy_ui_label_set_textalignment(&self->output, psy_ui_ALIGNMENT_LEFT);
	psy_ui_component_set_scroll_step_height(
		psy_ui_label_base(&self->output),
		psy_ui_value_make_eh(1.0));
	psy_ui_component_set_wheel_scroll(&self->output.component, 4);
	psy_ui_component_set_align(psy_ui_label_base(&self->output),
		psy_ui_ALIGN_FIXED);
	psy_ui_component_set_overflow(&self->output.component,
		psy_ui_OVERFLOW_SCROLL);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->output.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(20.0, 20.0));
	psy_lock_init(&self->lock);
	self->strbuffer = NULL;	
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void psy_ui_terminal_on_destroy(psy_ui_Terminal* self)
{
	psy_list_deallocate(&self->strbuffer, NULL);
	psy_lock_dispose(&self->lock);
}

void psy_ui_terminal_output(psy_ui_Terminal* self, const char* text)
{	
	assert(self);

	if (text) {
		psy_lock_enter(&self->lock);
		psy_list_append(&self->strbuffer, psy_strdup(text));
		psy_lock_leave(&self->lock);
	}
}

void psy_ui_terminal_clear(psy_ui_Terminal* self)
{	
	psy_ui_label_set_text(&self->output, "");
}

void psy_ui_terminal_on_timer(psy_ui_Terminal* self, uintptr_t timerid)
{
	if (self->strbuffer) {
		const psy_List* p;

		psy_lock_enter(&self->lock);
		for (p = self->strbuffer; p != NULL; p = p->next) {
			psy_ui_label_add_text(&self->output, (const char*)p->entry);			
		}
		psy_list_deallocate(&self->strbuffer, NULL);
		psy_lock_leave(&self->lock);
		psy_ui_component_align(&self->scroller.pane);
		psy_ui_component_invalidate(&self->output.component);
	}
}
