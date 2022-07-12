/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditconfig.h"
/* host */
#include "resources/resource.h"

/* prototypes */
static void seqeditconfig_make(SeqEditConfig*, psy_Property*);
static void seqeditconfig_on_property_changed(SeqEditConfig*, psy_Property*
	property);

/* implementation */
void seqeditconfig_init(SeqEditConfig* self, psy_Property* parent)
{
	assert(self);
	assert(parent);
	
	seqeditconfig_make(self, parent);
	psy_signal_init(&self->signal_changed);
}

void seqeditconfig_dispose(SeqEditConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void seqeditconfig_make(SeqEditConfig* self, psy_Property* parent)
{
	assert(self);

	self->seqedit = psy_property_set_id(psy_property_set_text(
		psy_property_append_section(parent, "seqedit"),
		"settingsview.seqeditor.seqeditor"), PROPERTY_ID_SEQEDIT);
	psy_property_set_icon(self->seqedit, IDB_STEP_LIGHT,
		IDB_STEP_DARK);
	psy_property_connect(psy_property_set_hint(psy_property_set_text(
		psy_property_append_int(self->seqedit, "machine", 0x3E, 0, 0x40),
		"settingsview.seqeditor.machine"), PSY_PROPERTY_HINT_EDITHEX),
		self, seqeditconfig_on_property_changed);
}

uintptr_t seqeditconfig_machine(const SeqEditConfig* self)
{
	assert(self);

	return (psy_property_at_int(self->seqedit, "machine", 0x3E));
}

void seqeditconfig_on_property_changed(SeqEditConfig* self, psy_Property*
	sender)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, sender);	
}

bool seqeditconfig_hasproperty(const SeqEditConfig* self, psy_Property* property)
{
	assert(self);

	return psy_property_in_section(property, self->seqedit);
}
