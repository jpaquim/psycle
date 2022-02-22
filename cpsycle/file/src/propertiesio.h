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

/* psy_PropertyReaderConfig */
typedef struct psy_PropertyReaderConfig {
	bool allowappend;
	bool cpp_comment;
	bool parse_types;
	bool ignore_sections;
	bool skip_double_quotes;
} psy_PropertyReaderConfig;

void psy_propertyreaderconfig_init(psy_PropertyReaderConfig*);

INLINE psy_PropertyReaderConfig psy_propertyreaderconfig_make(bool allowappend,
		bool cpp_comment, bool parse_types, bool ignore_sections,
		bool skip_double_quotes) {
	psy_PropertyReaderConfig rv;

	rv.allowappend = allowappend;
	rv.cpp_comment = cpp_comment;
	rv.parse_types = parse_types;
	rv.ignore_sections = ignore_sections;
	rv.skip_double_quotes = skip_double_quotes;
	return rv;
}

/* psy_PropertyReader */
typedef struct psy_PropertyReader {
	char* path;
	psy_PropertyReaderConfig config;
	int ischoice;
	psy_Property* root;
	psy_Property* curr;
	psy_Property* choice;	
} psy_PropertyReader;

void psy_propertyreader_init(psy_PropertyReader*, psy_Property* root,
	const char* path);
void psy_propertyreader_init_config(psy_PropertyReader*, psy_Property* root,
	const char* path, psy_PropertyReaderConfig);
void psy_propertyreader_dispose(psy_PropertyReader*);

int psy_propertyreader_load(psy_PropertyReader*);

INLINE void psy_propertyreader_allow_cpp_comments(psy_PropertyReader* self)
{
	self->config.cpp_comment = TRUE;
}

INLINE void psy_propertyreader_allow_append(psy_PropertyReader* self)
{
	self->config.allowappend = TRUE;
}

INLINE void psy_propertyreader_parse_types(psy_PropertyReader* self)
{
	self->config.parse_types = TRUE;
}

INLINE void psy_propertyreader_ignore_sections(psy_PropertyReader* self)
{
	self->config.ignore_sections = TRUE;
}

INLINE void psy_propertyreader_skip_double_quotes(psy_PropertyReader* self)
{
	self->config.skip_double_quotes = TRUE;
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
