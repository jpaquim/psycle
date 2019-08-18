// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "properties.h"
#include <malloc.h>
#include <string.h>


static int properties_enumarate_rec(Properties* self);
static void* target;
static int (*callback)(void* self, struct PropertiesStruct* properties, int level);
static int level;

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
	memset(&self->item, 0, sizeof(Properties));		
}

void properties_free(Properties* self)
{
	Properties* ptr;
	Properties* next;

	ptr = self;
	while (ptr != 0) {
		if (ptr->dispose) {
			ptr->dispose(&ptr->item);
		} else
		if (ptr->children) {
			properties_free(ptr->children);
		}		
		next = ptr->next;
		free(ptr->item.key);
		if (ptr->item.typ == PROPERTY_TYP_STRING) {
			free(ptr->item.value.s);
		}
		free(ptr);
		ptr = next;
	}	
}

Properties* properties_append_string(Properties* self, char* key, char* value)
{		
	Properties* ptr;

	if (!self) {
		return 0;
	}
	ptr = self;	
	while (ptr->next != 0) {
		ptr = ptr->next;		
	}
	ptr->next = (Properties*) malloc(sizeof(Properties));
	properties_init(ptr->next);
	ptr->next->item.key = strdup(key);
	ptr->next->item.value.s = strdup(value);	
	ptr->next->item.typ = PROPERTY_TYP_STRING;	
	return ptr->next;
}

Properties* properties_append_userdata(Properties* self, const char* key,
	void* value, void (*dispose)(Property*))
{			
	Properties* ptr;

	if (!self) {
		return 0;
	}
	ptr = self;	
	while (ptr->next != 0) {
		ptr = ptr->next;		
	}
	ptr->next = (Properties*) malloc(sizeof(Properties));	
	properties_init(ptr->next);
	ptr->next->dispose = dispose;
	ptr->next->item.key = strdup(key);
	ptr->next->item.value.ud = value;	
	ptr->next->item.typ = PROPERTY_TYP_USERDATA;	
	return ptr->next;
}

Properties* properties_append_int(Properties* self, char* key, int value, int min, int max)
{		
	Properties* ptr;

	if (!self) {
		return 0;
	}
	ptr = self;	
	while (ptr->next != 0) {
		ptr = ptr->next;		
	}
	ptr->next = (Properties*) malloc(sizeof(Properties));
	properties_init(ptr->next);
	ptr->next->item.key = strdup(key);
	ptr->next->item.value.i = value;	
	return ptr->next;
}


Properties* properties_append_choice(Properties* self, char* key, int value)
{
	Properties* ptr;

	if (!self) {
		return 0;
	}
	ptr = self;	
	while (ptr->next != 0) {
		ptr = ptr->next;		
	}
	ptr->next = (Properties*) malloc(sizeof(Properties));
	properties_init(ptr->next);
	ptr->next->item.key = strdup(key);
	ptr->next->item.value.i = value;	
	ptr->next->item.typ = PROPERTY_TYP_CHOICE;	
	ptr->next->item.hint = PROPERTY_HINT_LIST;	
	return ptr->next;
}

Properties* properties_read(Properties* self, char* key)
{
	Properties* ptr;	

	ptr = self;
	while (ptr != 0) {				
		if (ptr->item.key && strcmp(key, ptr->item.key) == 0) {
			break;
		}
		ptr = ptr->next;		
	}	
	return ptr;
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


void properties_write_string(Properties* self, char* key, char* value)
{
	Properties* ptr = properties_read(self, key);
	if (ptr) {
		if (ptr->item.typ == PROPERTY_TYP_STRING) {
			free(ptr->item.value.s);
		}
		ptr->item.value.s = strdup(value);
		ptr->item.typ = PROPERTY_TYP_STRING;
	} else {
		properties_append_string(self, key, value);
	}
}

void properties_write_int(Properties* self, char* key, int value)
{
	Properties* ptr = properties_read(self, key);
	if (ptr) {		
		ptr->item.value.i = value;
		ptr->item.typ = PROPERTY_TYP_INTEGER;		
	} else {
		properties_append_int(self, key, value, 0, 0);
	}
}

void properties_enumarate(Properties* self, void* t, int (*enumproc)(void* self, struct PropertiesStruct* properties, int level))
{
	target = t;
	level = 0;
	callback = enumproc;
	properties_enumarate_rec(self);
}

int properties_enumarate_rec(Properties* self)
{
	Properties* ptr;
	ptr = self;
	while (ptr != 0) {								
		if (!callback(target, ptr, level)) {
			return 0;
		}		
		if (ptr->children) {
			++level;
			if (!properties_enumarate_rec(ptr->children)) {
				--level;
				return 0;
			}
			--level;
		}
		ptr = ptr->next;		
	}	
	return 1;
}
