// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "properties.h"
#include "list.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>

typedef struct {
	void* target;
	psy_PropertiesCallback callback;
	int level;
} PropertiesCallbackContext;

static int properties_enumerate_rec(psy_Properties*, PropertiesCallbackContext*);
static int OnSearchPropertiesEnum(psy_Properties*, psy_Properties*, int level);
static int OnPropertySearchPropertiesEnum(psy_Properties* self, 
	psy_Properties* property, int level);
static psy_Properties* tail(psy_Properties*);

static psy_Properties* psy_properties_create_string(const char* key, const char* value);
static psy_Properties* psy_properties_create_font(const char* key, const char* value);
static psy_Properties* psy_properties_create_int(const char* key, int value, int min, int max);
static psy_Properties* psy_properties_create_bool(const char* key, int value);
static psy_Properties* psy_properties_create_choice(const char* key, int value);


void psy_property_init(psy_Property* self)
{
	self->key = strdup("");
	self->text = strdup("");
	self->shorttext = strdup("");
	self->translation = strdup("");
	self->comment = strdup("");
	self->typ = PSY_PROPERTY_TYP_INTEGER;
	self->value.i = 0;	
	self->min = 0;
	self->max = 0;
	self->hint = PSY_PROPERTY_HINT_NONE;
	self->disposechildren = TRUE;
	self->save = TRUE;
	self->id = 0;
}

void psy_property_copy(psy_Property* self, psy_Property* src)
{
	psy_property_dispose(self);
	self->key = src->key ? strdup(src->key) : NULL;
	self->text = src->text ? strdup(src->text) : NULL;
	self->shorttext = src->shorttext ? strdup(src->shorttext) : NULL;
	self->translation = src->translation ? strdup(src->translation) : NULL;
	self->comment = src->comment ? strdup(src->comment) : NULL;
	if (src->typ == PSY_PROPERTY_TYP_STRING ||
			src->typ == PSY_PROPERTY_TYP_FONT) {
		self->value.s = (src->value.s) ? strdup(src->value.s) : NULL;
	} else {
		self->value = src->value;
	}	
	self->min = src->min;
	self->max = src->max;
	self->typ = src->typ;
	self->hint = src->hint;
	self->disposechildren = src->disposechildren;
	self->save = src->save;
	self->id = src->id;
}

void psy_property_dispose(psy_Property* self)
{
	free(self->key);
	free(self->text);
	free(self->shorttext);
	free(self->translation);
	free(self->comment);
	if (self->typ == PSY_PROPERTY_TYP_STRING ||
			self->typ == PSY_PROPERTY_TYP_FONT) {
		free(self->value.s);
	}	
}

void psy_properties_init_type(psy_Properties* self, const char* key, psy_PropertyType typ)
{		
	self->children = 0;
	self->parent = 0;
	self->next = 0;	
	self->dispose = 0;    
	memset(&self->item, 0, sizeof(psy_Property));    
	self->item.key = strdup(key);
	self->item.text = NULL;
	self->item.translation = NULL;
	self->item.shorttext = NULL;
	self->item.comment = NULL;
	self->item.typ = typ;
	self->item.hint = PSY_PROPERTY_HINT_EDIT;
	self->item.disposechildren = 1;	
	self->item.id = -1;
	self->item.save = 1;    
}

void psy_properties_free(psy_Properties* self)
{
	psy_Properties* p;
	psy_Properties* q;

	if (self) {
		p = self;
		for (p = self; p != NULL; p = q) {
			q = p->next;			
			if (p->dispose) {
				p->dispose(&p->item);
			} else
			if (p->children && p->item.disposechildren) {
				psy_properties_free(p->children);
				p->children = NULL;
			}
			psy_property_dispose(&p->item);
			free(p);			
		}
	}	
}

psy_Properties* psy_properties_create(void)
{
	psy_Properties* rv;
        
	rv = (psy_Properties*) malloc(sizeof(psy_Properties));
    if (rv) {	    
        psy_properties_init_type(rv, "root", PSY_PROPERTY_TYP_ROOT);
    }
	return rv;
}

void psy_properties_init(psy_Properties* self)
{
	psy_properties_init_type(self, "root", PSY_PROPERTY_TYP_ROOT);
}

void psy_properties_dispose(psy_Properties* self)
{	
	psy_property_dispose(&self->item);
	if (self->children != NULL) {
		psy_Properties* p;
		psy_Properties* q;

		for (p = self->children; p != NULL; p = q) {
			q = p->next;
			if (p->dispose) {
				p->dispose(&p->item);
			} else
				if (p->children && p->item.disposechildren) {
					psy_properties_free(p->children);
					p->children = NULL;
				}
			psy_property_dispose(&p->item);
			free(p);
		}
	}
}

psy_Properties* psy_properties_append_section(psy_Properties* self, const char* name)
{
	psy_Properties* rv;
	
	assert(self);
	rv = (psy_Properties*) malloc(sizeof(psy_Properties));
    if (rv) {
        psy_properties_init_type(rv, name, PSY_PROPERTY_TYP_SECTION);
    }
	return psy_properties_append_property(self, rv);
}

psy_Properties* psy_properties_clone(psy_Properties* self, int all)
{
	psy_Properties* first = NULL;
	psy_Properties* rv = NULL;
	psy_Properties* p = NULL;
	psy_Properties* q = NULL;
	
	p = self;
	while (p) {
		if (first != NULL && !all) {
			break;
		}
		rv = (psy_Properties*) malloc(sizeof(psy_Properties));
		if (!rv) {
			break;
		}
		if (first == NULL) {
			first = rv;			
		} else {		
			q->next = rv;
		}
		rv->next = NULL;		
		rv->parent = NULL;
		rv->children = NULL;
		rv->dispose = NULL;
		psy_property_init(&rv->item);
		psy_property_copy(&rv->item, &p->item);				
		if (p->children) {
			psy_Properties* i;
			rv->children = psy_properties_clone(p->children, 1);
			for (i = rv->children; i != 0; i = i->next) {
				i->parent = rv;
			}
		}
		q = rv;
		p = p->next;
	}
	return first;
}

psy_Properties* psy_properties_sync(psy_Properties* self, psy_Properties* src)
{
	psy_Properties* p;
	p = src->children;
	for (p = src->children; p != NULL; p = psy_properties_next(p)) {
		psy_Properties* q;

		q = psy_properties_at(self, psy_properties_key(p), PSY_PROPERTY_TYP_NONE);
		if (q) {
			if (psy_properties_type(p) == PSY_PROPERTY_TYP_STRING) {
				psy_properties_set_str(self, psy_properties_key(p),
					psy_properties_as_str(p));
			} else
			if (psy_properties_type(p) == PSY_PROPERTY_TYP_INTEGER) {
				psy_properties_set_int(self, psy_properties_key(p),
					psy_properties_as_int(p));
			} else
			if (psy_properties_type(p) == PSY_PROPERTY_TYP_BOOL) {
				psy_properties_set_bool(self, psy_properties_key(p),
					psy_properties_as_int(p));
			} else
			if (psy_properties_type(p) == PSY_PROPERTY_TYP_FONT) {
				psy_properties_set_font(self, psy_properties_key(p),
					psy_properties_as_str(p));
			}
			if (q->children && p->children) {
				psy_properties_sync(q, p);
			}
		}
	}
	return self;
}

psy_Property* psy_properties_entry(psy_Properties* self)
{
	return &self->item;
}

psy_Properties* psy_properties_create_string(const char* key, const char* value)
{		
	psy_Properties* p;

	p = (psy_Properties*) malloc(sizeof(psy_Properties));
	psy_properties_init_type(p, key, PSY_PROPERTY_TYP_STRING);	
	p->item.value.s = strdup(value);	
	return p;
}

psy_Properties* psy_properties_append_string(psy_Properties* self, const char* key, const char* value)
{	
	return psy_properties_append_property(self, psy_properties_create_string(key, value));	
}

psy_Properties* psy_properties_create_font(const char* key, const char* value)
{		
	psy_Properties* p;

	p = (psy_Properties*) malloc(sizeof(psy_Properties));
	psy_properties_init_type(p, key, PSY_PROPERTY_TYP_FONT);	
	p->item.value.s = strdup(value);	
	return p;
}

psy_Properties* psy_properties_append_font(psy_Properties* self, const char* key, const char* value)
{	
	return psy_properties_append_property(self, psy_properties_create_font(key, value));	
}

psy_Properties* psy_properties_append_userdata(psy_Properties* self, const char* key,
	void* value, void (*dispose)(psy_Property*))
{			
	psy_Properties* p;

	if (!self) {
		return 0;
	}
	p = tail(self);
	p->next = (psy_Properties*) malloc(sizeof(psy_Properties));	
	psy_properties_init_type(p->next, key, PSY_PROPERTY_TYP_USERDATA);
	p->next->dispose = dispose;	
	p->next->item.value.ud = value;	
	return p->next;
}

psy_Properties* psy_properties_create_int(const char* key, int value, int min, int max)
{
	psy_Properties* p;

	p = (psy_Properties*) malloc(sizeof(psy_Properties));
	psy_properties_init_type(p, key, PSY_PROPERTY_TYP_INTEGER);	
	p->item.value.i = value;
	return p;
}

psy_Properties* psy_properties_append_action(psy_Properties* self, const char* key)
{			
	psy_Properties* p;
	
	p = psy_properties_create_int(key, 0, 0, 0);
	p->item.typ = PSY_PROPERTY_TYP_ACTION;
	p->item.hint = PSY_PROPERTY_HINT_CHECK;
	psy_properties_append_property(self, p);
	return p;
}

psy_Properties* psy_properties_append_int(psy_Properties* self, const char* key, int value, int min, int max)
{			
	return psy_properties_append_property(self, psy_properties_create_int(key, value, min, max));	
}

psy_Properties* psy_properties_create_bool(const char* key, int value)
{
	psy_Properties* p;
	
	p = psy_properties_create_int(key, value != 0, 0, 1);
	p->item.typ = PSY_PROPERTY_TYP_BOOL;
	p->item.hint = PSY_PROPERTY_HINT_CHECK;
	return p;
}

psy_Properties* psy_properties_append_bool(psy_Properties* self, const char* key, bool value)
{
	psy_Properties* rv;

	rv = (psy_Properties*)malloc(sizeof(psy_Properties));
	if (rv) {
		psy_properties_init_type(rv, key, PSY_PROPERTY_TYP_BOOL);
		rv->item.value.i = (value != FALSE);
		rv->item.hint = PSY_PROPERTY_HINT_CHECK;
	}
	return psy_properties_append_property(self, rv);
}

psy_Properties* psy_properties_append_double(psy_Properties* self, const char* key,
	double value, double min, double max)
{
	psy_Properties* p;
		
	p = (psy_Properties*) malloc(sizeof(psy_Properties));
	psy_properties_init_type(p, key, PSY_PROPERTY_TYP_DOUBLE);	
	p->item.value.d = value;		
	return psy_properties_append_property(self, p);	
}

psy_Properties* psy_properties_create_choice(const char* key, int value)
{
	psy_Properties* p;

	p = (psy_Properties*) malloc(sizeof(psy_Properties));
	psy_properties_init_type(p, key, PSY_PROPERTY_TYP_CHOICE);	
	p->item.value.i = value;		
	p->item.hint = PSY_PROPERTY_HINT_LIST;
	return p;
}

psy_Properties* psy_properties_append_choice(psy_Properties* self, const char* key, int value)
{	
	return psy_properties_append_property(self, psy_properties_create_choice(key, value));	
}

char* pathend(const char* path, char* section, char* key)
{
	char* p;
	
	p = strrchr(path, '.');
	return p;
}

psy_Properties* psy_properties_at(psy_Properties* self, const char* key, psy_PropertyType type)
{
	psy_Properties* p;	
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
			p = psy_properties_findsection(self, path);
			if (p) {
				p = p->children;
			}
			free(path);
		}
	}
	while (p != NULL) {		
		if ((p->item.key != NULL) &&
				((type == PSY_PROPERTY_TYP_NONE) || (p->item.typ == type)) &&
				strcmp(key, p->item.key) == 0) {
			break;
		}
		p = p->next;		
	}	
	return p;
}

int psy_properties_at_int(psy_Properties* properties, const char* key, int defaultvalue)
{
	int rv = defaultvalue;

	if (properties) {
		psy_Properties* property = psy_properties_at(properties, key, PSY_PROPERTY_TYP_NONE);
		if (property &&
			(property->item.typ == PSY_PROPERTY_TYP_INTEGER ||
			 property->item.typ == PSY_PROPERTY_TYP_CHOICE)) {
			rv = property->item.value.i;
		} else {
			rv = defaultvalue;
		}
	}
	return rv;
}

bool psy_properties_at_bool(psy_Properties* properties, const char* key, bool defaultvalue)
{
	bool rv;

	if (!properties) {
		rv = defaultvalue != FALSE;
	} else {
		psy_Properties* property = psy_properties_at(properties, key, PSY_PROPERTY_TYP_NONE);
		if (property && (property->item.typ == PSY_PROPERTY_TYP_BOOL ||
				property->item.typ == PSY_PROPERTY_TYP_INTEGER)) {
			rv = property->item.value.i != FALSE;
		} else {
			rv = defaultvalue;
		}
	}
	return rv;
}

double psy_properties_at_real(psy_Properties* properties, const char* key,
	double defaultvalue)
{
	double rv;

	if (!properties) {
		rv = defaultvalue;
	} else {
		psy_Properties* property = psy_properties_at(properties, key, PSY_PROPERTY_TYP_DOUBLE);
		if (property && property->item.typ == PSY_PROPERTY_TYP_DOUBLE) {
			rv = property->item.value.d;
		} else {
			rv = defaultvalue;
		}
	}
	return rv;
}

const char* psy_properties_at_str(psy_Properties* properties, const char* key,
	const char* defaulttext)
{
	const char* rv = 0;
	if (!properties) {
		rv = defaulttext;
	} else {
		psy_Properties* property = psy_properties_at(properties, key, PSY_PROPERTY_TYP_STRING);
		if (property && property->item.typ == PSY_PROPERTY_TYP_STRING ||
			property && property->item.typ == PSY_PROPERTY_TYP_FONT) {
			rv = property->item.value.s;
		} else {
			rv = defaulttext;
		}
	}
	return rv;
}

psy_Properties* psy_properties_set_str(psy_Properties* self, const char* key,
	const char* value)
{
	psy_Properties* p;
	
	p = psy_properties_at(self, key, PSY_PROPERTY_TYP_STRING);
	if (p) {
		if (p->item.typ == PSY_PROPERTY_TYP_STRING) {
			free(p->item.value.s);
		}
		p->item.value.s = strdup(value);
		p->item.typ = PSY_PROPERTY_TYP_STRING;
	} else {
		p = psy_properties_append_string(self, key, value);
	}
	return p;
}

psy_Properties* psy_properties_set_font(psy_Properties* self, const char* key,
	const char* value)
{
	psy_Properties* p;
	
	p = psy_properties_at(self, key, PSY_PROPERTY_TYP_FONT);
	if (p) {
		if (p->item.typ == PSY_PROPERTY_TYP_FONT) {
			free(p->item.value.s);
		}
		p->item.value.s = strdup(value);
		p->item.typ = PSY_PROPERTY_TYP_FONT;
	} else {
		p = psy_properties_append_font(self, key, value);
	}
	return p;
}

psy_Properties* psy_properties_set_int(psy_Properties* self, const char* key, int value)
{
	psy_Properties* p = psy_properties_at(self, key, PSY_PROPERTY_TYP_INTEGER);
	if (p) {		
		p->item.value.i = value;
		p->item.typ = PSY_PROPERTY_TYP_INTEGER;		
	} else {
		p = psy_properties_append_int(self, key, value, 0, 0);
	}
	return p;
}

psy_Properties* psy_properties_set_bool(psy_Properties* self, const char* key, bool value)
{	
	psy_Properties* p = psy_properties_at(self, key, PSY_PROPERTY_TYP_BOOL);
	if (p) {
		p->item.value.i = (value != FALSE);
		p->item.typ = PSY_PROPERTY_TYP_BOOL;
		p->item.hint = PSY_PROPERTY_HINT_CHECK;
	} else {
		p = psy_properties_append_int(self, key, value, 0, 0);
	}
	return p;
}

psy_Properties* psy_properties_set_choice(psy_Properties* self, const char* key, int value)
{
	psy_Properties* p = psy_properties_at(self, key, PSY_PROPERTY_TYP_NONE);
	if (p) {		
		p->item.value.i = value;
		p->item.typ = PSY_PROPERTY_TYP_CHOICE;
	} else {
		p = psy_properties_append_int(self, key, value, 0, 0);
		p->item.typ = PSY_PROPERTY_TYP_CHOICE;
	}
	return p;
}

psy_Properties* psy_properties_set_double(psy_Properties* self, const char* key, double value)
{
	psy_Properties* p = psy_properties_at(self, key, PSY_PROPERTY_TYP_NONE);
	if (p) {		
		p->item.value.d = value;
		p->item.typ = PSY_PROPERTY_TYP_DOUBLE;		
	} else {
		p = psy_properties_append_double(self, key, value, 0, 0);
	}
	return p;
}

void psy_properties_enumerate(psy_Properties* self, void* target,
	int (*enumproc)(void* self, psy_Properties* properties, int level))
{
	PropertiesCallbackContext context;

	context.target = target;
	context.callback = enumproc;
	context.level = 0;	
	properties_enumerate_rec(self, &context);	
}

int properties_enumerate_rec(psy_Properties* self, PropertiesCallbackContext* context)
{
	psy_Properties* p;
	p = self;
	while (p != NULL) {		
		int walkoption = context->callback(context->target, p, context->level);
		if (walkoption == 0) {
			return 0;
		} else
		if (walkoption == 1) {
			if (p->children) {
				++context->level;
				if (!properties_enumerate_rec(p->children, context)) {
					--context->level;
					return 0;
				}			
				--context->level;
			}
		}
		p = p->next;
	}	
	return 1;
}

static const char* searchkey;
static psy_PropertyType searchtyp;
static psy_Properties* keyfound;
static psy_Properties* searchproperty;


psy_Properties* psy_properties_find(psy_Properties* self, const char* key, psy_PropertyType typ)
{
	searchtyp = typ;
	searchkey = key;
	keyfound = 0;
	psy_properties_enumerate(self, self, (psy_PropertiesCallback) OnSearchPropertiesEnum);
	return keyfound;		
}

int OnSearchPropertiesEnum(psy_Properties* self, psy_Properties* property, int level)
{
	if (property->item.key && 
			(searchtyp == PSY_PROPERTY_TYP_NONE || property->item.typ == searchtyp)  &&
			strcmp(property->item.key, searchkey) == 0) {
		keyfound = property;
		return 0;
	}
	return 1;
}

int OnPropertySearchPropertiesEnum(psy_Properties* self, psy_Properties* property, int level)
{
	if (property == searchproperty) {
		keyfound = property;
		return 0;
	}
	return 1;
}

psy_Properties* psy_properties_findsection(psy_Properties* self, const char* key)
{
	psy_Properties* prev = 0;

	return psy_properties_findsectionex(self, key, &prev);
}

int psy_properties_insection(psy_Properties* self, psy_Properties* section)
{
	int rv = 0;
		
	if (section) {				
		keyfound = 0;
		searchproperty = self;
			
		psy_properties_enumerate(section, section,
		    (psy_PropertiesCallback)OnPropertySearchPropertiesEnum);
		rv = keyfound != 0;		
	}
	return rv;
}

psy_Properties* psy_properties_findsectionex(psy_Properties* self, const char* key,
	psy_Properties** prev)
{	
	psy_Properties* p;	
	char* text;
	char seps[]   = " .";
	char *token;

	p = self;
	*prev = p;
	text = strdup(key);
	token = strtok(text, seps );
	while(token != 0) {
		p = psy_properties_find(p, token, PSY_PROPERTY_TYP_SECTION);
		if (!p) {
			break;
		}		
		*prev = p;
		token = strtok(0, seps );		
	}
	free(text);
	return p;
}

int psy_properties_type(psy_Properties* self)
{
	return self ? self->item.typ : 0;
}

const char* psy_properties_key(psy_Properties* self)
{	
	return (self) ? self->item.key : "";
}

int psy_properties_as_int(psy_Properties* self)
{
	return (self) ? self->item.value.i : 0;
}

const char* psy_properties_as_str(psy_Properties* self)
{
	return (self && self->item.value.s) ? self->item.value.s : "";
}

char_dyn_t* psy_properties_sections(psy_Properties* self)
{	
	char_dyn_t* rv;
	psy_Properties* p;
	psy_List* tokens;
	psy_List* q;
	uintptr_t size;
	
	p = self;
	tokens = NULL;
	size = 1;
	while (p != NULL) {
		if (p->item.typ == PSY_PROPERTY_TYP_SECTION) {
			psy_list_insert(&tokens, NULL, (void*)psy_properties_key(p));
			size += strlen(psy_properties_key(p));
			if (p->parent) {
				++size;
			}
		}		
		p = p->parent;
	}
	rv = (char_dyn_t*) malloc(size);
	*rv = '\0';
	for (q = tokens; q != NULL; psy_list_next(&q)) {
		if (q->prev != NULL) {
			strcat(rv, ".");
		}
		strcat(rv, (char*)(q->entry));
	}
	psy_list_free(tokens);
	return rv;
}

psy_Properties* psy_properties_settext(psy_Properties* self, const char* text)
{
	if (self) {
		char* temp;

		temp = self->item.text;
		self->item.text = (text) ? strdup(text) : strdup("");
		free(temp);
	}
	return self;
}

const char* psy_properties_text(psy_Properties* self)
{
	return self->item.text ? self->item.text : self->item.key ? self->item.key : "";
}

psy_Properties* psy_properties_settranslation(psy_Properties* self, const char* text)
{
	if (self) {
		char* temp;

		temp = self->item.translation;
		self->item.translation = text ? strdup(text) : NULL;
		free(temp);
	}
	return self;
}

psy_Properties* psy_properties_setcomment(psy_Properties* self, const char* text)
{
	if (self) {
		char* temp;

		temp = self->item.comment;
		self->item.comment = (text) ? strdup(text) : strdup("");
		free(temp);
	}
	return self;
}

const char* psy_properties_translation(psy_Properties* self)
{
	return (self->item.translation && strlen(self->item.translation) != 0)
		? self->item.translation
		: psy_properties_text(self);
}

psy_Properties* psy_properties_setshorttext(psy_Properties* self, const char* text)
{
	if (self) {
		char* temp;

		temp = self->item.shorttext;
		self->item.shorttext = (text) ? strdup(text) : strdup("");
		free(temp);
	}
	return self;
}

const char* psy_properties_shorttext(psy_Properties* self)
{
	return self->item.shorttext ? self->item.shorttext : 
		self->item.text ? self->item.text :
		self->item.key ? self->item.key : "";
}

const char* psy_properties_comment(psy_Properties* self)
{
	return self->item.comment ? self->item.comment : "";
}

psy_Properties* psy_properties_setid(psy_Properties* self, int id)
{	
	if (self) {
		self->item.id = id;
	}
	return self;
}

int psy_properties_id(psy_Properties* self)
{
	return self ? self->item.id : -1;
}

int psy_properties_ischoiceitem(psy_Properties* self)
{
	return self->parent && self->parent->item.typ == PSY_PROPERTY_TYP_CHOICE;	
}

psy_Properties* tail(psy_Properties* self)
{
	psy_Properties* p;
	
	p = self;
	if (p) {
		while (p->next != 0) {
			p = p->next;		
		}
	}
	return p;
}

psy_Properties* psy_properties_append_property(psy_Properties* self, psy_Properties* p)
{	
	if (self) {		
		if (self->children) {
			tail(self->children)->next = p;		
		} else {
			self->children = p;
		}
		p->parent = self;
	}
	return p;
}

psy_Properties* psy_properties_sethint(psy_Properties* self, psy_PropertyHint hint)
{
	if (self) {
		self->item.hint = hint;
	}
	return self;
}

psy_PropertyHint psy_properties_hint(psy_Properties* self)
{	
	return self ? self->item.hint : PSY_PROPERTY_HINT_NONE;
}

psy_Properties* psy_properties_next(psy_Properties* self) {
	return self ? self->next : 0;
}

psy_Properties* psy_properties_remove(psy_Properties* self, psy_Properties* property)
{
	psy_Properties* q = 0;
	if (self) {
		psy_Properties* p;

		p = self->children;		
		while (p != NULL) {
			if (p == property) {
				if (q) {
					q->next = p->next;
				}				
				p->next = NULL;
				psy_properties_free(p);
				break;
			}
			q = p;
			p = p->next;
		}
	}
	return q;
}

void psy_properties_clear(psy_Properties* self)
{			
	if (self) {
		psy_properties_free(self->children);
		self->children = NULL;
	}
}

uintptr_t psy_properties_size(psy_Properties* self)
{
	unsigned int rv = 0;
		
	if (self) {
		psy_Properties* p;
		for (p = self->children; p != NULL; p = p->next, ++rv);
	}
	return rv;
}

psy_Properties* psy_properties_at_choice(psy_Properties* self)
{
	psy_Properties* rv = 0;	
		
	if (self) {		
		int choice;	
		psy_Properties* p;
		int count = 0;		
		
		choice = psy_properties_as_int(self);
		p = self->children;		
		while (p) {
			if (count == choice) {
				rv = p;
				break;
			}
			p = psy_properties_next(p);
			++count;
		}
	}
	return rv;
}
