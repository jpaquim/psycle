// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "properties.h"
#include <malloc.h>
#include <string.h>


static int properties_enumerate_rec(Properties* self);
static void* target;
static int (*callback)(void* self, struct PropertiesStruct* properties, int level);
static int level;
static int OnSearchPropertiesEnum(Properties* self, Properties* property, int level);

Properties* properties_create(void)
{
	Properties* rv;
	rv = (Properties*) malloc(sizeof(Properties));	
	properties_init(rv);	
	return rv;
}

void properties_init(Properties* self)
{
	self->children = 0;
	self->next = 0;	
	self->dispose = 0;
	self->item.key = 0;
	memset(&self->item, 0, sizeof(Properties));		
}

void properties_free(Properties* self)
{
	Properties* p;
	Properties* next;

	p = self;
	while (p != 0) {
		if (p->dispose) {
			p->dispose(&p->item);
		} else
		if (p->children) {
			properties_free(p->children);
		}		
		next = p->next;
		free(p->item.key);
		if (p->item.typ == PROPERTY_TYP_STRING) {
			free(p->item.value.s);
		}
		free(p);
		p = next;
	}	
}

Properties* properties_append_string(Properties* self, const char* key, const char* value)
{		
	Properties* p;

	if (!self) {
		return 0;
	}
	p = self;	
	while (p->next != 0) {
		p = p->next;		
	}
	p->next = (Properties*) malloc(sizeof(Properties));
	properties_init(p->next);
	p->next->item.key = strdup(key);
	p->next->item.value.s = strdup(value);	
	p->next->item.typ = PROPERTY_TYP_STRING;	
	return p->next;
}

Properties* properties_append_userdata(Properties* self, const char* key,
	void* value, void (*dispose)(Property*))
{			
	Properties* p;

	if (!self) {
		return 0;
	}
	p = self;	
	while (p->next != 0) {
		p = p->next;		
	}
	p->next = (Properties*) malloc(sizeof(Properties));	
	properties_init(p->next);
	p->next->dispose = dispose;
	p->next->item.key = strdup(key);
	p->next->item.value.ud = value;	
	p->next->item.typ = PROPERTY_TYP_USERDATA;	
	return p->next;
}

Properties* properties_append_int(Properties* self, const char* key, int value, int min, int max)
{		
	Properties* p;

	if (!self) {
		return 0;
	}
	p = self;	
	while (p->next != 0) {
		p = p->next;		
	}
	p->next = (Properties*) malloc(sizeof(Properties));
	properties_init(p->next);
	p->next->item.key = strdup(key);
	p->next->item.value.i = value;	
	p->next->item.typ = PROPERTY_TYP_INTEGER;
	return p->next;
}

Properties* properties_append_double(Properties* self, const char* key, double value, double min, double max)
{
	Properties* p;

	if (!self) {
		return 0;
	}
	p = self;	
	while (p->next != 0) {
		p = p->next;		
	}
	p->next = (Properties*) malloc(sizeof(Properties));
	properties_init(p->next);
	p->next->item.key = strdup(key);
	p->next->item.value.d = value;	
	p->next->item.typ = PROPERTY_TYP_DOUBLE;
	return p->next;
}


Properties* properties_append_choice(Properties* self, const char* key, int value)
{
	Properties* p;

	if (!self) {
		return 0;
	}
	p = self;	
	while (p->next != 0) {
		p = p->next;		
	}
	p->next = (Properties*) malloc(sizeof(Properties));
	properties_init(p->next);
	p->next->item.key = strdup(key);
	p->next->item.value.i = value;	
	p->next->item.typ = PROPERTY_TYP_CHOICE;	
	p->next->item.hint = PROPERTY_HINT_LIST;	
	return p->next;
}

Properties* properties_read(Properties* self, const char* key)
{
	Properties* p;	

	p = self;
	while (p != 0) {				
		if (p->item.key && strcmp(key, p->item.key) == 0) {
			break;
		}
		p = p->next;		
	}	
	return p;
}

void properties_readint(Properties* properties, const char* key, int* value, int defaultvalue)
{
	if (!properties) {
		*value = defaultvalue;
	} else {
		Properties* property = properties_read(properties, key);
		if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
			*value = property->item.value.i;
		} else {
			*value = defaultvalue;
		}
	}
}

void properties_readdouble(Properties* properties, const char* key, double* value, double defaultvalue)
{
	if (!properties) {
		*value = defaultvalue;
	} else {
		Properties* property = properties_read(properties, key);
		if (property && property->item.typ == PROPERTY_TYP_DOUBLE) {
			*value = property->item.value.d;
		} else {
			*value = defaultvalue;
		}
	}
}

void properties_readstring(Properties* properties, const char* key, char** text, char* defaulttext)
{
	if (!properties) {
		*text = defaulttext;
	} else {
		Properties* property = properties_read(properties, key);
		if (property && property->item.typ == PROPERTY_TYP_STRING) {
			*text = property->item.value.s;
		} else {
			*text = defaulttext;
		}
	}
}

void properties_write_string(Properties* self, const char* key, const char* value)
{
	Properties* p = properties_read(self, key);
	if (p) {
		if (p->item.typ == PROPERTY_TYP_STRING) {
			free(p->item.value.s);
		}
		p->item.value.s = strdup(value);
		p->item.typ = PROPERTY_TYP_STRING;
	} else {
		properties_append_string(self, key, value);
	}
}

void properties_write_int(Properties* self, const char* key, int value)
{
	Properties* p = properties_read(self, key);
	if (p) {		
		p->item.value.i = value;
		p->item.typ = PROPERTY_TYP_INTEGER;		
	} else {
		properties_append_int(self, key, value, 0, 0);
	}
}

void properties_write_double(Properties* self, const char* key, double value)
{
	Properties* p = properties_read(self, key);
	if (p) {		
		p->item.value.d = value;
		p->item.typ = PROPERTY_TYP_DOUBLE;		
	} else {
		properties_append_double(self, key, value, 0, 0);
	}
}

void properties_enumerate(Properties* self, void* t, int (*enumproc)(void* self, struct PropertiesStruct* properties, int level))
{
	target = t;
	level = 0;
	callback = enumproc;
	properties_enumerate_rec(self);
}

int properties_enumerate_rec(Properties* self)
{
	Properties* p;
	p = self;
	while (p != 0) {								
		if (!callback(target, p, level)) {
			return 0;
		}		
		if (p->children) {
			++level;
			if (!properties_enumerate_rec(p->children)) {
				--level;
				return 0;
			}
			--level;
		}
		p = p->next;		
	}	
	return 1;
}

static const char* searchkey;
static Properties* keyfound;

Properties* properties_find(Properties* self, const char* key)
{
	searchkey = key;
	keyfound = 0;
	properties_enumerate(self, self, OnSearchPropertiesEnum);
	return keyfound;		
}

int OnSearchPropertiesEnum(Properties* self, Properties* property, int level)
{
	if (property->item.key && strcmp(property->item.key, searchkey) == 0) {
		keyfound = property;
		return 0;
	}
	return 1;
}

const char* properties_key(Properties* self)
{	
	return (self) ? self->item.key : "";
}

int properties_value(Properties* self)
{
	return (self) ? self->item.value.i : 0;
}

const char* properties_valuestring(Properties* self)
{
	return (self) ? self->item.value.s : "";
}
