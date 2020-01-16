// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_PROPERTIES_H)
#define PSY_PROPERTIES_H

#include "../../detail/stdint.h"

typedef enum {
	PSY_PROPERTY_TYP_ROOT,
	PSY_PROPERTY_TYP_INTEGER,
	PSY_PROPERTY_TYP_STRING,
	PSY_PROPERTY_TYP_DOUBLE,
	PSY_PROPERTY_TYP_BOOL,	
	PSY_PROPERTY_TYP_CHOICE,
	PSY_PROPERTY_TYP_USERDATA,
	PSY_PROPERTY_TYP_SECTION,
	PSY_PROPERTY_TYP_ACTION
} psy_PropertyType;

typedef enum {
	PSY_PROPERTY_HINT_NONE,
	PSY_PROPERTY_HINT_HIDE,
	PSY_PROPERTY_HINT_READONLY,
	PSY_PROPERTY_HINT_EDIT,
	PSY_PROPERTY_HINT_EDITDIR,
	PSY_PROPERTY_HINT_INPUT,
	PSY_PROPERTY_HINT_LIST,
	PSY_PROPERTY_HINT_CHECK	
} psy_PropertyHint;

typedef struct {
	char* key;
	char* text;
	union {
		char* s;
		int i;
		double d;
		void* ud;
	} value;
	int min;
	int max;
	int typ;
	int hint;	
	int disposechildren;
	int save;
	int id;
} psy_Property;

typedef struct psy_Properties {
	psy_Property item;	
	struct psy_Properties* next;
	struct psy_Properties* children;
	struct psy_Properties* parent;
	void (*dispose)(psy_Property*);
} psy_Properties;

typedef int (*psy_PropertiesCallback)(void* , psy_Properties*, int level);

void psy_properties_init(psy_Properties*, const char* key, psy_PropertyType);
psy_Properties* psy_properties_create(void);
psy_Properties* psy_properties_clone(psy_Properties*, int all);
psy_Properties* psy_properties_create_section(psy_Properties*, const char* name);
void properties_free(psy_Properties* );
psy_Properties* psy_properties_create_string(const char* key, const char* value);
psy_Properties* psy_properties_create_int(const char* key, int value, int min, int max);
psy_Properties* psy_properties_create_bool(const char* key, int value);
psy_Properties* psy_properties_create_choice(const char* key, int value);
psy_Properties* psy_properties_append_string(psy_Properties*, const char* key, const char* value);
psy_Properties* psy_properties_append_choice(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_append_userdata(psy_Properties*, const char* key,
	void* value, void (*dispose)(psy_Property*));
psy_Properties* psy_properties_append_int(psy_Properties*, const char* key, int value, int min, int max);
psy_Properties* psy_properties_append_bool(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_append_double(psy_Properties*, const char* key, double value, double min, double max);
psy_Properties* psy_properties_append_action(psy_Properties*, const char* key);
psy_Properties* psy_properties_append_property(psy_Properties*, psy_Properties*);
psy_Properties* psy_properties_read(psy_Properties*, const char* key);
int psy_properties_int(psy_Properties*, const char* key, int defaultvalue);
int psy_properties_bool(psy_Properties*, const char* key, int defaultvalue);
void psy_properties_readdouble(psy_Properties*, const char* key, double* value, double defaultvalue);
const char* psy_properties_readstring(psy_Properties*, const char* key, const char* defaulttext);
psy_Properties* psy_properties_read_choice(psy_Properties*);
psy_Properties* psy_properties_write_string(psy_Properties*, const char* key, const char* value);
psy_Properties* psy_properties_write_int(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_write_bool(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_write_choice(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_write_double(psy_Properties*, const char* key, double value);
void psy_properties_enumerate(psy_Properties*, void* target, psy_PropertiesCallback);
psy_Properties* psy_properties_find(psy_Properties*, const char* key);
psy_Properties* psy_properties_findsection(psy_Properties*, const char* key);
psy_Properties* psy_properties_findsectionex(psy_Properties*, const char* key,
	psy_Properties** prev);
void psy_properties_sections(psy_Properties*, char* text);
int psy_properties_insection(psy_Properties*, psy_Properties* section);
const char* psy_properties_key(psy_Properties*);
int psy_properties_type(psy_Properties*);
int psy_properties_value(psy_Properties*);
psy_Property* psy_properties_entry(psy_Properties*);
const char* psy_properties_valuestring(psy_Properties*);
psy_Properties* psy_properties_settext(psy_Properties*, const char* text);
const char* psy_properties_text(psy_Properties* self);
psy_Properties* psy_properties_setid(psy_Properties*, int id);
int psy_properties_id(psy_Properties* self);
psy_Properties* psy_properties_sethint(psy_Properties*, psy_PropertyHint);
int psy_properties_ischoiceitem(psy_Properties*);
psy_PropertyHint psy_properties_hint(psy_Properties*);
psy_Properties* psy_properties_next(psy_Properties*);
psy_Properties* psy_properties_remove(psy_Properties*, psy_Properties*);
void psy_properties_clear(psy_Properties*);
uintptr_t psy_properties_size(psy_Properties*);

#endif
