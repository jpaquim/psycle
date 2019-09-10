// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PROPERTIES)
#define PROPERTIES

enum {
	PROPERTY_TYP_INTEGER,
	PROPERTY_TYP_STRING,
	PROPERTY_TYP_DOUBLE,
	PROPERTY_TYP_CHOICE,
	PROPERTY_TYP_USERDATA
};

enum {
	PROPERTY_HINT_EDIT,
	PROPERTY_HINT_LIST
};

typedef struct {
	char* key;	
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
} Property;

struct PropertiesStruct {
	Property item;	
	struct PropertiesStruct* next;
	struct PropertiesStruct* children;
	void (*dispose)(Property*);
};

typedef struct PropertiesStruct Properties;


void properties_init(Properties* self);
Properties* properties_create(void);
void properties_free(Properties* self);
Properties* properties_append_string(Properties* self, const char* key, const char* value);
Properties* properties_append_choice(Properties* self, const char* key, int value);
Properties* properties_append_userdata(Properties* self, const char* key,
	void* value, void (*dispose)(Property*));
Properties* properties_append_int(Properties* self, const char* key, int value, int min, int max);
Properties* properties_append_double(Properties* self, const char* key, double value, double min, double max);
Properties* properties_read(Properties* self, const char* key);
void properties_readint(Properties* properties, const char* key, int* value, int defaultvalue);
void properties_readdouble(Properties* properties, const char* key, double* value, double defaultvalue);
void properties_write_string(Properties* self, const char* key, const char* value);
void properties_write_int(Properties* self, const char* key, int value);
void properties_write_double(Properties* self, const char* key, double value);
void properties_enumerate(Properties* self, void* target, int (*enumerate)(void* self, struct PropertiesStruct* properties, int level));

#endif
