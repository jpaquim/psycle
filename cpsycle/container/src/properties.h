// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PROPERTIES_H)
#define PROPERTIES_H

typedef enum {
	PROPERTY_TYP_ROOT,
	PROPERTY_TYP_INTEGER,
	PROPERTY_TYP_STRING,
	PROPERTY_TYP_DOUBLE,
	PROPERTY_TYP_BOOL,	
	PROPERTY_TYP_CHOICE,
	PROPERTY_TYP_USERDATA,
	PROPERTY_TYP_SECTION,
	PROPERTY_TYP_ACTION
} PropertyType;

typedef enum {
	PROPERTY_HINT_HIDE,
	PROPERTY_HINT_READONLY,
	PROPERTY_HINT_EDIT,
	PROPERTY_HINT_EDITDIR,
	PROPERTY_HINT_INPUT,
	PROPERTY_HINT_LIST,
	PROPERTY_HINT_CHECK	
} PropertyHint;

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
} Property;

typedef struct Properties {
	Property item;	
	struct Properties* next;
	struct Properties* children;
	struct Properties* parent;
	void (*dispose)(Property*);
} Properties;

typedef int (*PropertiesCallback)(void* , Properties*, int level);

void properties_init(Properties*, const char* key, PropertyType);
Properties* properties_create(void);
Properties* properties_clone(Properties*);
Properties* properties_createsection(Properties*, const char* name);
void properties_free(Properties* );
Properties* properties_create_string(const char* key, const char* value);
Properties* properties_create_int(const char* key, int value, int min, int max);
Properties* properties_create_bool(const char* key, int value);
Properties* properties_create_choice(const char* key, int value);
Properties* properties_append_string(Properties*, const char* key, const char* value);
Properties* properties_append_choice(Properties*, const char* key, int value);
Properties* properties_append_userdata(Properties*, const char* key,
	void* value, void (*dispose)(Property*));
Properties* properties_append_int(Properties*, const char* key, int value, int min, int max);
Properties* properties_append_bool(Properties*, const char* key, int value);
Properties* properties_append_double(Properties*, const char* key, double value, double min, double max);
Properties* properties_append_action(Properties*, const char* key);
Properties* properties_read(Properties*, const char* key);
int properties_int(Properties*, const char* key, int defaultvalue);
int properties_bool(Properties*, const char* key, int defaultvalue);
void properties_readdouble(Properties*, const char* key, double* value, double defaultvalue);
const char* properties_readstring(Properties*, const char* key, const char* defaulttext);
Properties* properties_read_choice(Properties*);
Properties* properties_write_string(Properties*, const char* key, const char* value);
Properties* properties_write_int(Properties*, const char* key, int value);
Properties* properties_write_bool(Properties*, const char* key, int value);
Properties* properties_write_choice(Properties*, const char* key, int value);
Properties* properties_write_double(Properties*, const char* key, double value);
void properties_enumerate(Properties*, void* target, PropertiesCallback);
Properties* properties_find(Properties*, const char* key);
Properties* properties_findsection(Properties*, const char* key);
Properties* properties_findsectionex(Properties*, const char* key,
	Properties** prev);
void properties_sections(Properties*, char* text);
int properties_insection(Properties*, Properties* section);
const char* properties_key(Properties*);
int properties_type(Properties*);
int properties_value(Properties*);
Property* properties_entry(Properties*);
const char* properties_valuestring(Properties*);
Properties* properties_settext(Properties*, const char* text);
const char* properties_text(Properties* self);
Properties* properties_setid(Properties*, int id);
int properties_id(Properties* self);
Properties* properties_sethint(Properties*, PropertyHint);
int properties_ischoiceitem(Properties*);
PropertyHint properties_hint(Properties*);
Properties* properties_next(Properties*);
Properties* properties_remove(Properties*, Properties*);
void properties_clear(Properties*);
unsigned int properties_size(Properties*);

#endif
