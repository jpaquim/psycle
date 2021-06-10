/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITCONFIG_H)
#define SEQEDITCONFIG_H

/* audio */
#include <machinefactory.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MetronomeConfig */

enum {
	PROPERTY_ID_SEQEDIT = 70000
};

typedef struct SeqEditConfig {
	/* signals */
	psy_Signal signal_changed;
	psy_Property* seqedit;
	/* references */
	psy_Property* parent;	
} SeqEditConfig;

void seqeditconfig_init(SeqEditConfig*, psy_Property* parent);
void seqeditconfig_dispose(SeqEditConfig*);

uintptr_t seqeditconfig_machine(const SeqEditConfig*);

bool seqeditconfig_onchanged(SeqEditConfig*, psy_Property*);
bool seqeditconfig_hasproperty(const SeqEditConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* SEQEDITCONFIG_H */
