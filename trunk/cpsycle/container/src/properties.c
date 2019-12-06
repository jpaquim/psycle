// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "properties.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>

static int properties_enumerate_rec(Properties*);
static int OnSearchPropertiesEnum(Properties*, Properties*, int level);
static int OnPropertySearchPropertiesEnum(Properties* self, 
	Properties* property, int level);
static Properties* append(Properties* self, Properties* p);
static Properties* tail(Properties*);

static void* target;
static PropertiesCallback callback;
static int level;

#define MAXSTRINGSIZE 4096

void properties_init(Properties* self, const char* key, PropertyType typ)
{		
	self->children = 0;
	self->parent = 0;
	self->next = 0;	
	self->dispose = 0;
	memset(&self->item, 0, sizeof(Property));
	self->item.key = strdup(key);
	self->item.text = 0;
	self->item.typ = typ;
	self->item.hint = PROPERTY_HINT_EDIT;
	self->item.disposechildren = 1;	
	self->item.id = -1;
}

void properties_free(Properties* self)
{
	Properties* p;
	Properties* q;

	if (self) {
		p = self;
		for (p = self; p != 0; p = q) {
			q = p->next;
			if (p->dispose) {
				p->dispose(&p->item);
			}
			else
				if (p->children && p->item.disposechildren) {
					properties_free(p->children);
				}			
			free(p->item.key);
			free(p->item.text);
			if (p->item.typ == PROPERTY_TYP_STRING) {
				free(p->item.value.s);
			}
			free(p);			
		}
	}
}

Properties* properties_create(void)
{
	Properties* rv;
	rv = (Properties*) malloc(sizeof(Properties));	
	properties_init(rv, "root", PROPERTY_TYP_ROOT);
	return rv;
}

Properties* properties_createsection(Properties* self, const char* name)
{
	Properties* rv;
	
	assert(self);
	rv = (Properties*) malloc(sizeof(Properties));
	properties_init(rv, name, PROPERTY_TYP_SECTION);
	
	return append(self, rv);
}

Properties* properties_clone(Properties* self)
{
	Properties* first = 0;
	Properties* rv = 0;
	Properties* p = 0;
	Properties* q = 0;
	
	p = self;
	while (p) {				
		rv = (Properties*) malloc(sizeof(Properties));
		if (!first) {
			first = rv;			
		} else {
			q->next = rv;
		}
		rv->next = 0;		
		rv->parent = 0;
		rv->children = 0;
		rv->dispose = 0;			
		rv->item = p->item;

		rv->item.key = p->item.key ? strdup(p->item.key) : 0;
		rv->item.text = p->item.text ? strdup(p->item.text) : 0;
		rv->item.min = p->item.min;
		rv->item.max = p->item.max;
		rv->item.value = p->item.value;		
		rv->item.typ = p->item.typ;
		rv->item.hint = p->item.hint;
		rv->item.id = p->item.id;
		rv->item.disposechildren = 1;
		if (p->children) {
			Properties* i;
			rv->children = properties_clone(p->children);
			for (i = rv->children; i != 0; i = i->next) {
				i->parent = rv;
			}
		}
		q = rv;
		p = p->next;
	}
	return first;
}

Property* properties_entry(Properties* self)
{
	return &self->item;
}

Properties* properties_create_string(const char* key, const char* value)
{		
	Properties* p;

	p = (Properties*) malloc(sizeof(Properties));
	properties_init(p, key, PROPERTY_TYP_STRING);	
	p->item.value.s = strdup(value);	
	return p;
}

Properties* properties_append_string(Properties* self, const char* key, const char* value)
{	
	return append(self, properties_create_string(key, value));	
}

Properties* properties_append_userdata(Properties* self, const char* key,
	void* value, void (*dispose)(Property*))
{			
	Properties* p;

	if (!self) {
		return 0;
	}
	p = tail(self);
	p->next = (Properties*) malloc(sizeof(Properties));	
	properties_init(p->next, key, PROPERTY_TYP_USERDATA);
	p->next->dispose = dispose;	
	p->next->item.value.ud = value;	
	return p->next;
}

Properties* properties_create_int(const char* key, int value, int min, int max)
{
	Properties* p;

	p = (Properties*) malloc(sizeof(Properties));
	properties_init(p, key, PROPERTY_TYP_INTEGER);	
	p->item.value.i = value;
	return p;
}

Properties* properties_append_action(Properties* self, const char* key)
{			
	Properties* p;
	
	p = properties_create_int(key, 0, 0, 0);
	p->item.typ = PROPERTY_TYP_ACTION;
	p->item.hint = PROPERTY_HINT_CHECK;
	append(self, p);
	return p;
}

Properties* properties_append_int(Properties* self, const char* key, int value, int min, int max)
{			
	return append(self, properties_create_int(key, value, min, max));	
}

Properties* properties_create_bool(const char* key, int value)
{
	Properties* p;
	
	p = properties_create_int(key, value != 0, 0, 1);
	p->item.typ = PROPERTY_TYP_BOOL;
	p->item.hint = PROPERTY_HINT_CHECK;
	return p;
}

Properties* properties_append_bool(Properties* self, const char* key, int value)
{
	Properties* p;
	
	p = properties_append_int(self, key, value != 0, 0, 1);
	p->item.typ = PROPERTY_TYP_BOOL;
	p->item.hint = PROPERTY_HINT_CHECK;

	return p;
}

Properties* properties_append_double(Properties* self, const char* key,
	double value, double min, double max)
{
	Properties* p;
		
	p = (Properties*) malloc(sizeof(Properties));
	properties_init(p, key, PROPERTY_TYP_DOUBLE);	
	p->item.value.d = value;		
	return append(self, p);	
}

Properties* properties_create_choice(const char* key, int value)
{
	Properties* p;

	p = (Properties*) malloc(sizeof(Properties));
	properties_init(p, key, PROPERTY_TYP_CHOICE);	
	p->item.value.i = value;		
	p->item.hint = PROPERTY_HINT_LIST;
	return p;
}

Properties* properties_append_choice(Properties* self, const char* key, int value)
{	
	return append(self, properties_create_choice(key, value));	
}

char* pathend(const char* path, char* section, char* key)
{
	char* p;
	
	p = strrchr(path, '.');
	return p;
}

Properties* properties_read(Properties* self, const char* key)
{
	Properties* p;	
	char* c;
	
	c = strrchr(key, '.');
	if (!c) {			
		p = self->children;		
	} else {
		char* path;
		ptrdiff_t count;
		
		count = c - key;
		path = malloc(count + 1);
		if (path) {			
			strncpy(path, key, count);
			path[count] = '\0';
			key = c + 1;
			p = properties_findsection(self, path);
			if (p) {
				p = p->children;
			}
			free(path);
		}
	}
	while (p != 0) {		
		if (p->item.key && strcmp(key, p->item.key) == 0) {
			break;
		}
		p = p->next;		
	}	
	return p;
}

int properties_int(Properties* properties, const char* key, int defaultvalue)
{
	int rv = defaultvalue;

	if (properties) {	
		Properties* property = properties_read(properties, key);
		if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
			rv = property->item.value.i;
		} else {
			rv = defaultvalue;
		}
	}
	return rv;
}

int properties_bool(Properties* properties, const char* key, int defaultvalue)
{
	int rv;

	if (!properties) {
		rv = defaultvalue;
	} else {
		Properties* property = properties_read(properties, key);
		if (property && (property->item.typ == PROPERTY_TYP_BOOL ||
				property->item.typ == PROPERTY_TYP_INTEGER)) {
			rv = property->item.value.i != 0;
		} else {
			rv = defaultvalue;
		}
	}
	return rv;
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

const char* properties_readstring(Properties* properties, const char* key, const char* defaulttext)
{
	const char* rv = 0;
	if (!properties) {
		rv = defaulttext;
	} else {
		Properties* property = properties_read(properties, key);
		if (property && property->item.typ == PROPERTY_TYP_STRING) {
			rv = property->item.value.s;
		} else {
			rv = defaulttext;
		}
	}
	return rv;
}

Properties* properties_write_string(Properties* self, const char* key, const char* value)
{
	Properties* p = properties_read(self, key);
	if (p) {
		if (p->item.typ == PROPERTY_TYP_STRING) {
			free(p->item.value.s);
		}
		p->item.value.s = strdup(value);
		p->item.typ = PROPERTY_TYP_STRING;
	} else {
		p = properties_append_string(self, key, value);
	}
	return p;
}

Properties* properties_write_int(Properties* self, const char* key, int value)
{
	Properties* p = properties_read(self, key);
	if (p) {		
		p->item.value.i = value;
		p->item.typ = PROPERTY_TYP_INTEGER;		
	} else {
		p = properties_append_int(self, key, value, 0, 0);
	}
	return p;
}

Properties* properties_write_bool(Properties* self, const char* key, int value)
{
	Properties* p;
	
	p = properties_write_int(self, key, value != 0);
	p->item.typ = PROPERTY_TYP_BOOL;
	p->item.hint = PROPERTY_HINT_CHECK;	
	return p;
}

Properties* properties_write_choice(Properties* self, const char* key, int value)
{
	Properties* p = properties_read(self, key);
	if (p) {		
		p->item.value.i = value;
		p->item.typ = PROPERTY_TYP_CHOICE;
	} else {
		p = properties_append_int(self, key, value, 0, 0);
		p->item.typ = PROPERTY_TYP_CHOICE;
	}
	return p;
}

Properties* properties_write_double(Properties* self, const char* key, double value)
{
	Properties* p = properties_read(self, key);
	if (p) {		
		p->item.value.d = value;
		p->item.typ = PROPERTY_TYP_DOUBLE;		
	} else {
		p = properties_append_double(self, key, value, 0, 0);
	}
	return p;
}

void properties_enumerate(Properties* self, void* t, int (*enumproc)(void* self, Properties* properties, int level))
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
static Properties* searchproperty;

Properties* properties_find(Properties* self, const char* key)
{
	searchkey = key;
	keyfound = 0;
	properties_enumerate(self, self, 
	    (PropertiesCallback)OnSearchPropertiesEnum);
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

int OnPropertySearchPropertiesEnum(Properties* self, Properties* property, int level)
{
	if (property == searchproperty) {
		keyfound = property;
		return 0;
	}
	return 1;
}

Properties* properties_findsection(Properties* self, const char* key)
{
	Properties* prev = 0;

	return properties_findsectionex(self, key, &prev);
}

int properties_insection(Properties* self, Properties* section)
{
	int rv = 0;
		
	if (section) {				
		keyfound = 0;
		searchproperty = self;
			
		properties_enumerate(section, section,
		    (PropertiesCallback)OnPropertySearchPropertiesEnum);
		rv = keyfound != 0;		
	}
	return rv;
}

Properties* properties_findsectionex(Properties* self, const char* key,
	Properties** prev)
{	
	Properties* p;	
	char text[MAXSTRINGSIZE];
	char seps[]   = " .";
	char *token;

	p = self;
	*prev = p;
	strcpy(text, key);
	token = strtok(text, seps );
	while(token != 0) {
		p = properties_find(self, token);	
		if (!p) {
			break;
		}		
		*prev = p;
		token = strtok(0, seps );		
	}	
	return p;
}

int properties_type(Properties* property)
{
	return property->item.typ;
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

void properties_sections(Properties* self, char* text)
{	
	Properties* p;

	text[0] = '\0';
	p = self;
	while (p) {
		if (p->item.typ == PROPERTY_TYP_SECTION &&
			 (strlen(text) + 1 + strlen(properties_key(p))) < MAXSTRINGSIZE) {
			char buffer[MAXSTRINGSIZE];
			
			strcpy(buffer, text);
			if (p->parent && p->parent->item.typ == PROPERTY_TYP_SECTION) {
				strcpy(text, ".");
			} else {
				text[0] = '\0';
			}
			strcat(text, properties_key(p));			
			strcat(text, buffer);			
		}
		p = p->parent;
	}	
}

Properties* properties_settext(Properties* self, const char* text)
{
	free(self->item.text);
	self->item.text = strdup(text);
	return self;
}

const char* properties_text(Properties* self)
{
	return self->item.text ? self->item.text : self->item.key ? self->item.key : "";
}

Properties* properties_setid(Properties* self, int id)
{	
	self->item.id = id;
	return self;
}

int properties_id(Properties* self)
{
	return self->item.id;
}

int properties_ischoiceitem(Properties* self)
{
	return self->parent && self->parent->item.typ == PROPERTY_TYP_CHOICE;	
}

Properties* tail(Properties* self)
{
	Properties* p;
	
	p = self;
	if (p) {
		while (p->next != 0) {
			p = p->next;		
		}
	}
	return p;
}

Properties* append(Properties* self, Properties* p)
{	
	if (self->children) {
		tail(self->children)->next = p;		
	} else {
		self->children = p;
	}
	p->parent = self;
	return p;
}

Properties* properties_sethint(Properties* self, PropertyHint hint)
{
	self->item.hint = hint;
	return self;
}

PropertyHint properties_hint(Properties* self)
{	
	return self->item.hint;
}

Properties* properties_next(Properties* self) {
	return self->next;
}

Properties* properties_remove(Properties* self, Properties* property)
{
	Properties* p;
	Properties* q;

	p = self->children;	
	q = 0;
	while (p != 0) {		
		if (p == property) {			
			if (q) {
				q->next = p->next;
			}
			properties_free(p);
		}		
		q = p;
		p = p->next;
	}
	return q;
}

void properties_clear(Properties* self)
{			
	properties_free(self->children);	
	self->children = 0;
}

unsigned int properties_size(Properties* self)
{
	unsigned int rv = 0;
	Properties* p;
		
	if (self) {
		for (p = self->children; p != 0; p = p->next, ++rv);
	}
	return rv;
}

Properties* properties_read_choice(Properties* self)
{
	Properties* rv = 0;	
		
	if (self) {		
		int choice;	
		Properties* p;
		int count = 0;		
		
		choice = properties_value(self);
		p = self->children;		
		while (p) {
			if (count == choice) {
				rv = p;
				break;
			}
			p = properties_next(p);
			++count;
		}
	}
	return rv;
}