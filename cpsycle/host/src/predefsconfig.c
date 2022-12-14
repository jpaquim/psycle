/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "predefsconfig.h"
/* host */
#include "resources/resource.h"
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void predefsconfig_make(PredefsConfig*);
static void predefsconfig_on_property_changed(PredefsConfig*, psy_Property*
	sender);

/* implementation */
void predefsconfig_init(PredefsConfig* self, psy_Property* parent)
{
	assert(self);
	assert(parent);

	self->parent = parent;
	psy_signal_init(&self->signal_changed);
	predefsconfig_make(self);
}

void predefsconfig_dispose(PredefsConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void predefsconfig_make(PredefsConfig* self)
{
	assert(self);

	self->predefs = psy_property_append_section(self->parent, "Predefs");
	psy_property_set_icon(self->predefs, IDB_OPTIONS_LIGHT,
		IDB_OPTIONS_DARK);
	psy_property_connect(psy_property_append_str(self->predefs,
		"adsr", "2 0.0 0.0 1.0 1.0 2.0 0.5 3.0 0.0"),
		self, predefsconfig_on_property_changed);
	psy_property_connect(psy_property_append_str(self->predefs,
		"def1", ""), self, predefsconfig_on_property_changed);
	psy_property_connect(psy_property_append_str(self->predefs,
		"def2", ""), self, predefsconfig_on_property_changed);
	psy_property_connect(psy_property_append_str(self->predefs,
		"def3", ""), self, predefsconfig_on_property_changed);
	psy_property_connect(psy_property_append_str(self->predefs,
		"def4", ""), self, predefsconfig_on_property_changed);
	psy_property_connect(psy_property_append_str(self->predefs,
		"def5", ""), self, predefsconfig_on_property_changed);
	psy_property_connect(psy_property_append_str(self->predefs,
		"def6", ""), self, predefsconfig_on_property_changed);
}

void predefsconfig_predef(PredefsConfig* self, int index, psy_dsp_Envelope* rv)
{
	char key[10];
	const char* values;	

	assert(self && rv);

	if (index > 0) {
		psy_snprintf(key, 10, "def%i", index);
	} else {
		psy_snprintf(key, 10, "adsr");		
	}

	values = psy_property_at_str(self->predefs, key, NULL);
	if (!values || values[0] == '\0') {
		return;
	}
	psy_dsp_envelope_init_dispose(rv);
	psy_dsp_envelope_init(rv);
	if (values) {
		char* text;
		char* token;
		char seps[] = " ";
		int c;
		psy_dsp_seconds_t time;
		psy_dsp_amp_t value;

		text = psy_strdup(values);
		token = strtok(text, seps);
		time = 0.f;
		value = 0.f;
		c = 0;
		while (token != 0) {
			if (c == 0) {
				rv->sustainbegin = atoi(token);
			} else {
				if ((c % 2) != 0) {
					time = (psy_dsp_seconds_t)atof(token);
				} else {
					value = (psy_dsp_amp_t)atof(token);
					if (c == 2) { // first point
						psy_dsp_envelope_append(rv,
							psy_dsp_envelopepoint_make_all(
								time, value, 0.f, 0.f, 0.f, 0.f));
					} else {
						psy_dsp_envelope_append(rv,
							psy_dsp_envelopepoint_make_all(
								time, value, 0.f, 5.f, 0.f, 1.f));
					}
				}
			}			
			token = strtok(0, seps);
			++c;
		}
		free(text);
	}
}

void predefsconfig_storepredef(PredefsConfig* self, int index,
	psy_dsp_Envelope* env)
{
	char key[10];	

	assert(self && env);

	if (index > 0) {
		psy_snprintf(key, 10, "def%i", index);
	} else {
		psy_snprintf(key, 10, "adsr");
	}
	if (psy_dsp_envelope_empty(env)) {
		psy_property_set_str(self->predefs, key, "");
		return;
	}	
	psy_property_set_str(self->predefs, key, psy_dsp_envelope_tostring(env));	
}

void predefsconfig_on_property_changed(PredefsConfig* self, psy_Property*
	sender)
{
	assert(self && self->predefs);

	psy_signal_emit(&self->signal_changed, self, 1, sender);	
}

bool predefsconfig_hasproperty(const PredefsConfig* self,
	psy_Property* property)
{
	assert(self && self->predefs);

	return psy_property_in_section(property, self->predefs);
}
