/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_PROPERTIESIO_H
#define psy_PROPERTIESIO_H

/* local */
#include "dir.h"
/* container */
#include "properties.h"
/* std */
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROPERTIESIO_DEFAULT_COMMENT 0
#define PROPERTIESIO_CPP_COMMENT 1

typedef struct psy_PropertyReader {
	char* path;
	bool allowappend;
	bool cpp_comment;
	int ischoice;
	psy_Property* root;
	psy_Property* curr;
	psy_Property* choice;	
} psy_PropertyReader;

void psy_propertyreader_init(psy_PropertyReader*, psy_Property* root,
	const char* path);
void psy_propertyreader_dispose(psy_PropertyReader*);

int psy_propertyreader_load(psy_PropertyReader*);

INLINE void psy_propertyreader_allow_cpp_comments(psy_PropertyReader* self)
{
	self->cpp_comment = TRUE;
}

INLINE void psy_propertyreader_allow_append(psy_PropertyReader* self)
{
	self->allowappend = TRUE;
}

typedef struct psy_PropertyWriter {
	char* path;		
	const psy_Property* root;
	uintptr_t skip;
	uintptr_t skiplevel;
	uintptr_t choicelevel;
	char* lastsection;
	FILE* fp;
} psy_PropertyWriter;

void psy_propertywriter_init(psy_PropertyWriter*, const psy_Property* root,
	const char* path);
void psy_propertywriter_dispose(psy_PropertyWriter*);

int psy_propertywriter_save(psy_PropertyWriter*);


int propertiesio_save(psy_PropertyWriter*);

#ifdef __cplusplus
}
#endif

#endif /* psy_PROPERTIESIO_H */
