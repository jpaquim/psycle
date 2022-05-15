/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditconfig.h"

static void seqeditconfig_make(SeqEditConfig*, psy_Property*);

void seqeditconfig_init(SeqEditConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
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

	self->seqedit = psy_property_setid(psy_property_settext(
		psy_property_append_section(parent, "seqedit"),
		"settingsview.seqeditor.seqeditor"), PROPERTY_ID_SEQEDIT);	
	psy_property_sethint(psy_property_settext(
		psy_property_append_int(self->seqedit, "machine", 0x3E, 0, 0x40),
		"settingsview.seqeditor.machine"),
		PSY_PROPERTY_HINT_EDITHEX);
}

/* properties */
uintptr_t seqeditconfig_machine(const SeqEditConfig* self)
{
	assert(self);

	return (psy_property_at_int(self->seqedit, "machine", 0x3E));
}

/* events */
uintptr_t seqeditconfig_onchanged(SeqEditConfig* self, psy_Property*
	property)
{
	psy_signal_emit(&self->signal_changed, self, 1, property);
	return psy_INDEX_INVALID;
}

bool seqeditconfig_hasproperty(const SeqEditConfig* self, psy_Property* property)
{
	assert(self && self->seqedit);

	return psy_property_in_section(property, self->seqedit);
}
