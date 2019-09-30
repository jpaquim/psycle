// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PROPERTIES)
#define PROPERTIES

typedef enum {
	PROPERTY_TYP_INTEGER,
	PROPERTY_TYP_STRING,
	PROPERTY_TYP_DOUBLE,
	PROPERTY_TYP_BOOL,
	PROPERTY_TYP_CHOICE,
	PROPERTY_TYP_USERDATA,
	PROPERTY_TYP_SECTION
} PropertyType;

typedef enum {
	PROPERTY_HINT_HIDE,
	PROPERTY_HINT_EDIT,
	PROPERTY_HINT_EDITDIR,	
	PROPERTY_HINT_LIST,
	PROPERTY_HINT_CHECK,
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
} Property;

struct PropertiesStruct {
	Property item;	
	struct PropertiesStruct* next;
	struct PropertiesStruct* children;
	struct PropertiesStruct* parent;
	void (*dispose)(Property*);
};

typedef struct PropertiesStruct Properties;

void properties_init(Properties* );
Properties* properties_create(void);
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
Properties* properties_read(Properties*, const char* key);
int properties_int(Properties*, const char* key, int defaultvalue);
void properties_readbool(Properties*, const char* key, int* value, int defaultvalue);
void properties_readdouble(Properties*, const char* key, double* value, double defaultvalue);
void properties_readstring(Properties*, const char* key, char** text, char* defaulttext);
Properties* properties_write_string(Properties*, const char* key, const char* value);
Properties* properties_write_int(Properties*, const char* key, int value);
Properties* properties_write_bool(Properties*, const char* key, int value);
Properties* properties_write_choice(Properties*, const char* key, int value);
Properties* properties_write_double(Properties*, const char* key, double value);
void properties_enumerate(Properties*, void* target, int (*enumerate)(void* , struct PropertiesStruct* properties, int level));
Properties* properties_find(Properties*, const char* key);
const char* properties_key(Properties*);
int properties_checktype(Properties*, PropertyType);
int properties_value(Properties*);
Property* properties_entry(Properties*);
const char* properties_valuestring(Properties*);
void properties_load(Properties*, const char* path);
void properties_save(Properties*, const char* path);
void properties_settext(Properties*, const char* text);
const char* properties_text(Properties* self);


#endif
