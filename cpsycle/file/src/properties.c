// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "properties.h"
#include <malloc.h>
#include <string.h>
#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

static int properties_enumerate_rec(Properties*);
static int OnSearchPropertiesEnum(Properties*, Properties*, int level);
static int OnPropertySearchPropertiesEnum(Properties* self, 
	Properties* property, int level);
static int OnSaveIniEnum(RiffFile* file, Properties*, int level);
static Properties* append(Properties* self, Properties* p);
static Properties* tail(Properties*);
static Properties* properties_findsectionex(Properties*, const char* key,
	Properties** prev);
void properties_sections(Properties*, char* text);


static void* target;
static PropertiesCallback callback;
static int level;

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
		if (p->children && p->item.disposechildren) {
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

Property* properties_entry(Properties* self)
{
	return &self->item;
}

Properties* properties_create_string(const char* key, const char* value)
{		
	Properties* p;

	p = (Properties*) malloc(sizeof(Properties));
	properties_init(p, key, PROPERTY_TYP_STRING);	
	p->item.value.s = _strdup(value);	
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
		int count;
		
		count = c - key;
		path = malloc(count + 1);
		strncpy(path, key, count);
		path[count] = '\0';		
		key = c + 1;		
		p = properties_findsection(self, path);
		if (p) {
			p = p->children;
		}
		free(path);
	}
	while (p != 0) {		
		if (p->item.key && strcmp(key, p->item.key) == 0) {
			break;
		}
		p = p->next;		
	}	
	return p;
}

int properties_int(Properties* properties, const char* key,
	int defaultvalue)
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

void properties_readstring(Properties* properties, const char* key, const char** text, const char* defaulttext)
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

Properties* properties_write_string(Properties* self, const char* key, const char* value)
{
	Properties* p = properties_read(self, key);
	if (p) {
		if (p->item.typ == PROPERTY_TYP_STRING) {
			free(p->item.value.s);
		}
		p->item.value.s = _strdup(value);
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
			
		properties_enumerate(section, section, OnPropertySearchPropertiesEnum);
		rv = keyfound != 0;		
	}
	return rv;
}

Properties* properties_findsectionex(Properties* self, const char* key,
	Properties** prev)
{	
	Properties* p;	
	char text[_MAX_PATH];
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

int properties_load(Properties* self, const char* path, int allowappend)
{
	FILE* fp;
	Properties* curr;

	curr = self;			
	fp = fopen(path, "rb");
	if (fp) {
		int c;
		int i;
		int state;
		char key[_MAX_PATH + 1];
		char value[_MAX_PATH + 1];
		
		i = 0;
		state = 0;
				
		while ((c = fgetc(fp)) != EOF) {			
			if (state == 0) {
				if (c == '\r') {
					state = 0;
				} else
				if (c == '\n') {
					state = 0;
				} else
				if (c == '[') {
					state = 3;
					key[i] = '\0';
					i = 0;
				} else
				if (c == '=') {
					state = 1;
					key[i] = '\0';
					i = 0;
				} else {
					if (i < _MAX_PATH) {
						key[i] = c;					
					}
					++i;
				}
			} else
			if (state == 1) {
				if (c == '\n') {
					state = 2;
					value[i] = '\0';
					i = 0;					
				} else
				if (i < _MAX_PATH) {
					value[i] = c;					
				}
				++i;
			} else 
			if (state == 3) {
				if (c == ']') {
					state = 4;
					key[i] = '\0';
					i = 0;					
				} else
				if (i < _MAX_PATH) {
					key[i] = c;					
				}
				++i;
			}
			if (state == 2) {
				Properties* p = properties_read(curr, key);
				if (p) {
					switch (p->item.typ) {
						case PROPERTY_TYP_ROOT:
						break;
						case PROPERTY_TYP_INTEGER:
							properties_write_int(curr, key, atoi(value));
						break;
						case PROPERTY_TYP_BOOL:
							properties_write_bool(curr, key, atoi(value));
						break;
						case PROPERTY_TYP_CHOICE:
							properties_write_choice(curr, key, atoi(value));
						break;
						case PROPERTY_TYP_STRING:
							properties_write_string(curr, key, value);
						break;
						default:
						break;
					}					
				} else
				if (allowappend) {
					int intval;
					char *stopstring;

					intval = strtol(value, &stopstring, 10);
					if (errno == ERANGE || strcmp(stopstring, "") != 0) {
						properties_append_string(curr, key, value);	
					} else {
						properties_append_int(curr, key, intval, 0, 0);											
					}
				}
				i = 0;
				state = 0;
			} else
			if (state == 4) {
				Properties* p;				
				Properties* prev = 0;
				p = properties_findsectionex(self, key, &prev);
				if (p && p->children) {
					curr = p;
				} else
				if (allowappend) {										
					curr = properties_createsection(prev, key);
				} else {
					curr = self;
				}
				i = 0;
				state = 0;
			}
		}		
		fclose(fp);
		if (state == 1) {
			if (i < _MAX_PATH) {
				value[i] = '\0';
			} else {
				value[_MAX_PATH - 1] = '\0';
			}
			if (properties_read(curr, key)) {
				properties_write_string(curr, key, value);
			}			
		}		
	}
	return fp != 0;
}

void properties_save(Properties* self, const char* path)
{
	RiffFile file;
	if (rifffile_create(&file, path, 1)) {
		properties_enumerate(self, &file, (PropertiesCallback)OnSaveIniEnum);
		rifffile_close(&file);
	}
}

int OnSaveIniEnum(RiffFile* file, Properties* property, int level)
{
	if (property->item.key) {
		char text[40];
		
		if (property->item.typ == PROPERTY_TYP_ROOT) {
			rifffile_write(file, "[root]", 6);
		} else
		if (property->item.typ == PROPERTY_TYP_SECTION) {
			char sections[_MAX_PATH];

			properties_sections(property, sections);
			rifffile_write(file, "[", 1);
			if (sections[0] != '\0') {
				rifffile_write(file, sections, strlen(sections));
			}
			rifffile_write(file, "]", 1);
		} else 
		if (property->item.typ != PROPERTY_TYP_ACTION) {			
			rifffile_write(file, property->item.key, strlen(property->item.key));
			rifffile_write(file, "=", strlen("="));
			switch (property->item.typ) {				
				case PROPERTY_TYP_INTEGER:
					_snprintf(text, 40, "%d", property->item.value.i);
					rifffile_write(file, text, strlen(text));
				break;
				case PROPERTY_TYP_BOOL:
					_snprintf(text, 40, "%d", property->item.value.i);
					rifffile_write(file, text, strlen(text));
				break;
				case PROPERTY_TYP_CHOICE:
					_snprintf(text, 40, "%d", property->item.value.i);
					rifffile_write(file, text, strlen(text));
				break;
				case PROPERTY_TYP_STRING:
					rifffile_write(file, property->item.value.s,
						strlen(property->item.value.s));
				break;						
				default:
				break;
			}
		}
		rifffile_write(file, "\n", strlen("\n"));
	}
	return 1;
}

void properties_sections(Properties* self, char* text)
{	
	Properties* p;

	text[0] = '\0';
	p = self;
	while (p) {
		if (p->item.typ == PROPERTY_TYP_SECTION &&
			 (strlen(text) + 1 + strlen(properties_key(p))) < _MAX_PATH) {
			char buffer[_MAX_PATH];
			
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
	self->item.text = _strdup(text);
	return self;
}

const char* properties_text(Properties* self)
{
	return self->item.text ? self->item.text : self->item.key ? self->item.key : "";
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