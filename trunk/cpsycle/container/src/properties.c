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

static char* psy_strdup(const char* str)
{
	return (str)
		? strdup(str)
		: NULL;
}

static intptr_t psy_strlen(const char* str)
{
	return (str)
		? strlen(str)
		: 0;
}

static char* psy_setstr(char** dest, const char* text) {
	assert(dest);

	if (text != *dest) {
		free(*dest);
		*dest = psy_strdup(text);
	}
	return *dest;
}

typedef struct {
	void* target;
	psy_PropertyCallback callback;
	int level;
} PropertiesCallbackContext;

static int properties_enumerate_rec(psy_Property*, PropertiesCallbackContext*);
static int OnSearchPropertiesEnum(psy_Property*, psy_Property*, int level);
static int OnPropertySearchPropertiesEnum(psy_Property*, psy_Property*,
	int level);

// psy_PropertyItem
// implementation
void psy_propertyitem_init(psy_PropertyItem* self)
{
	assert(self);

	self->key = NULL;
	self->text = NULL;
	self->shorttext = NULL;
	self->translation = NULL;
	self->comment = NULL;
	self->typ = PSY_PROPERTY_TYPE_INTEGER;
	self->value.i = 0;
	self->min = 0;
	self->max = 0;
	self->hint = PSY_PROPERTY_HINT_NONE;
	self->disposechildren = TRUE;
	self->save = TRUE;
	self->id = -1;
	self->allowappend = 0;
}

void psy_propertyitem_copy(psy_PropertyItem* self, psy_PropertyItem* source)
{
	assert(self);

	if (self == source) {
		return;
	}
	psy_propertyitem_dispose(self);
	self->key = psy_strdup(source->key);
	self->text = psy_strdup(source->text);
	self->shorttext = psy_strdup(source->shorttext);
	self->translation = psy_strdup(source->translation);
	self->comment = psy_strdup(source->comment);
	if (source->typ == PSY_PROPERTY_TYPE_STRING ||
			source->typ == PSY_PROPERTY_TYPE_FONT) {
		self->value.s = psy_strdup(source->value.s);
	} else {
		self->value = source->value;
	}
	self->min = source->min;
	self->max = source->max;
	self->typ = source->typ;
	self->hint = source->hint;
	self->disposechildren = source->disposechildren;
	self->save = source->save;
	self->id = source->id;
	self->allowappend = source->allowappend;
}

void psy_propertyitem_dispose(psy_PropertyItem* self)
{
	assert(self);

	free(self->key);
	free(self->text);
	free(self->shorttext);
	free(self->translation);
	free(self->comment);
	if (self->typ == PSY_PROPERTY_TYPE_STRING ||
			self->typ == PSY_PROPERTY_TYPE_FONT) {
		free(self->value.s);
	}
}

// psy_Property
// Prototypes
static psy_Property* psy_property_create_string(const char* key,
	const char* value);
static psy_Property* psy_property_create_font(const char* key,
	const char* value);
static psy_Property* psy_property_create_int(const char* key, int value,
	int min, int max);
static psy_Property* psy_property_create_bool(const char* key, int value);
static psy_Property* psy_property_create_choice(const char* key, int value);

// Implementation
void psy_property_init(psy_Property* self)
{
	assert(self);

	psy_property_init_type(self, "root", PSY_PROPERTY_TYPE_ROOT);
}

void psy_property_init_key(psy_Property* self, const char* key)
{
	assert(self);

	psy_property_init_type(self,
		(key)
			? key
			: "root",
		PSY_PROPERTY_TYPE_ROOT);
}

void psy_property_init_type(psy_Property* self, const char* key,
	psy_PropertyType typ)
{
	assert(self);

	psy_propertyitem_init(&self->item);	
	psy_setstr(&self->item.key, key);	
	self->item.typ = typ;
	self->item.hint = PSY_PROPERTY_HINT_EDIT;
	self->children = NULL;
	self->dispose = NULL;
	self->parent = NULL;
}

void psy_property_dispose(psy_Property* self)
{
	assert(self);

	if (self->item.disposechildren) {
		psy_list_deallocate(&self->children, (psy_fp_disposefunc)
			psy_property_dispose);
	}
	psy_propertyitem_dispose(&self->item);
}

psy_Property* psy_property_allocinit_key(const char* key)
{
	psy_Property* rv;

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		psy_property_init_key(rv, key);		
	}
	return rv;
}

void psy_property_deallocate(psy_Property* self)
{
	assert(self);

	psy_propertyitem_dispose(&self->item);
	free(self);
}

psy_Property* psy_property_sync(psy_Property* self, psy_Property* source)
{
	assert(self);

	if (self != source) {
		psy_List* i;

		for (i = source->children; i != NULL; psy_list_next(&i)) {
			psy_Property* q;
			psy_Property* p;

			p = (psy_Property*)i->entry;
			q = psy_property_at(self, psy_property_key(p),
				PSY_PROPERTY_TYPE_NONE);
			if (q) {
				if (psy_property_type(p) == PSY_PROPERTY_TYPE_STRING) {
					psy_property_set_str(self, psy_property_key(p),
						psy_property_as_str(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_INTEGER) {
					psy_property_set_int(self, psy_property_key(p),
						psy_property_as_int(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_BOOL) {
					psy_property_set_bool(self, psy_property_key(p),
						psy_property_as_int(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_FONT) {
					psy_property_set_font(self, psy_property_key(p),
					psy_property_as_str(p));
				}
				if (q->children && p->children) {
					psy_property_sync(q, p);
				}
			}
		}
	}
	return self;
}

psy_Property* psy_property_append_property(psy_Property* self, psy_Property* property)
{
	assert(self);

	if (property) {
		psy_list_append(&self->children, property);
		if (psy_property_type(property) == PSY_PROPERTY_TYPE_ROOT) {
			property->item.typ = PSY_PROPERTY_TYPE_SECTION;
		}
		property->parent = self;
	}
	return property;
}

psy_Property* psy_property_append_section(psy_Property* self, const char* name)
{
	psy_Property* rv;

	assert(self);

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		psy_property_init_type(rv, name, PSY_PROPERTY_TYPE_SECTION);
	}
	return psy_property_append_property(self, rv);
}

psy_Property* psy_property_clone(psy_Property* self)
{
	psy_Property* rv;	

	assert(self);

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		rv->parent = NULL;
		rv->children = NULL;
		rv->dispose = NULL;
		psy_propertyitem_init(&rv->item);
		psy_propertyitem_copy(&rv->item, &self->item);
		if (self->children) {
			psy_List* p;

			for (p = self->children; p != NULL; psy_list_next(&p)) {
				psy_Property* q;

				q = psy_property_clone(p->entry);
				psy_list_append(&rv->children, q);
				q->parent = rv;
			}
		}
	}
	return rv;
}

psy_PropertyItem* psy_property_entry(psy_Property* self)
{
	assert(self);

	return &self->item;
}

psy_Property* psy_property_create_string(const char* key, const char* value)
{
	psy_Property* property;

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_STRING);
		property->item.value.s = psy_strdup(value);
	}
	return property;
}

psy_Property* psy_property_append_string(psy_Property* self, const char* key,
	const char* value)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_string(key, value));
}

psy_Property* psy_property_create_font(const char* key, const char* value)
{
	psy_Property* property;

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_FONT);
		property->item.value.s = psy_strdup(value);
	}
	return property;
}

psy_Property* psy_property_append_font(psy_Property* self, const char* key,
	const char* value)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_font(key, value));
}

psy_Property* psy_property_append_userdata(psy_Property* self, const char* key,
	void* value, void (*dispose)(psy_PropertyItem*))
{
	psy_Property* property;

	assert(self);

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_list_append(&self->children, property);
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_USERDATA);
		property->dispose = dispose;
		property->item.value.ud = value;
	}
	return property;
}

psy_Property* psy_property_create_int(const char* key, int value, int minval,
	int maxval)
{
	psy_Property* property;

	assert(minval <= maxval);
	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_INTEGER);
		property->item.value.i = value;
		property->item.min = minval;
		property->item.max = maxval;
	}
	return property;
}

psy_Property* psy_property_append_action(psy_Property* self, const char* key)
{
	psy_Property* property;

	assert(self);

	property = psy_property_create_int(key, 0, 0, 0);
	if (property) {
		property->item.typ = PSY_PROPERTY_TYPE_ACTION;
		property->item.hint = PSY_PROPERTY_HINT_CHECK;
		psy_property_append_property(self, property);
	}
	return property;
}

psy_Property* psy_property_append_int(psy_Property* self, const char* key,
	int value, int min, int max)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_int(key, value, min, max));
}

psy_Property* psy_property_create_bool(const char* key, int value)
{
	psy_Property* property;

	property = psy_property_create_int(key, value != 0, 0, 1);
	if (property) {
		property->item.typ = PSY_PROPERTY_TYPE_BOOL;
		property->item.hint = PSY_PROPERTY_HINT_CHECK;
	}
	return property;
}

psy_Property* psy_property_append_bool(psy_Property* self,
	const char* key, bool value)
{
	psy_Property* rv;

	assert(self);

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		psy_property_init_type(rv, key, PSY_PROPERTY_TYPE_BOOL);
		rv->item.value.i = (value != FALSE);
		rv->item.hint = PSY_PROPERTY_HINT_CHECK;
	}
	return psy_property_append_property(self, rv);
}

psy_Property* psy_property_append_double(psy_Property* self, const char* key,
	double value, double min, double max)
{
	psy_Property* property;

	assert(self);

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_DOUBLE);
		property->item.value.d = value;
	}
	return psy_property_append_property(self, property);
}

psy_Property* psy_property_create_choice(const char* key, int value)
{
	psy_Property* property;

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_CHOICE);
		property->item.value.i = value;
		property->item.hint = PSY_PROPERTY_HINT_LIST;
	}
	return property;
}

psy_Property* psy_property_append_choice(psy_Property* self, const char* key,
	int value)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_choice(key, value));
}

char* pathend(const char* path, char* section, char* key)
{
	char* p;

	p = strrchr(path, '.');
	return p;
}

psy_Property* psy_property_at(psy_Property* self, const char* key,
	psy_PropertyType type)
{
	psy_List* p = NULL;
	char* c;

	assert(self);

	if (!key) {
		return NULL;
	}
	c = strrchr(key, '.');
	if (!c) {
		p = self->children;
	} else {
		char* path;
		ptrdiff_t count;

		count = c - key;
		path = malloc(count + 1);
		if (path) {
			psy_Property* q;

			strncpy(path, key, count);
			path[count] = '\0';
			key = c + 1;
			q = psy_property_findsection(self, path);
			if (q) {
				p = q->children;
			}
			free(path);
		}
	}
	while (p != NULL) {
		psy_Property* property;

		property = (psy_Property*)psy_list_entry(p);
		assert(property->item.key);
		if (((type == PSY_PROPERTY_TYPE_NONE) || (property->item.typ == type)) &&
				(strcmp(key, property->item.key) == 0)) {
			break;
		}
		psy_list_next(&p);
	}
	return (p)
		? (psy_Property*)psy_list_entry(p)
		: NULL;
}

int psy_property_at_int(psy_Property* self, const char* key, int defaultvalue)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property && (property->item.typ == PSY_PROPERTY_TYPE_INTEGER ||
			property->item.typ == PSY_PROPERTY_TYPE_CHOICE)) {
		return property->item.value.i;
	}
	return defaultvalue;
}

bool psy_property_at_bool(psy_Property* self, const char* key,
	bool defaultvalue)
{	
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property && (property->item.typ == PSY_PROPERTY_TYPE_BOOL ||
		property->item.typ == PSY_PROPERTY_TYPE_INTEGER)) {
		return property->item.value.i != FALSE;
	}
	return defaultvalue;	
}

double psy_property_at_real(psy_Property* self, const char* key,
	double defaultvalue)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_DOUBLE);
	if (property && property->item.typ == PSY_PROPERTY_TYPE_DOUBLE) {
		return property->item.value.d;
	}
	return defaultvalue;
}

const char* psy_property_at_str(psy_Property* self, const char* key,
	const char* defaulttext)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_STRING);
	if (property && property->item.typ == PSY_PROPERTY_TYPE_STRING ||
		property && property->item.typ == PSY_PROPERTY_TYPE_FONT) {
		return property->item.value.s;
	}
	return defaulttext;
}

psy_Property* psy_property_set_str(psy_Property* self, const char* key,
	const char* value)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_STRING);
	if (property) {
		property->item.typ = PSY_PROPERTY_TYPE_STRING;
		if (property->item.typ == PSY_PROPERTY_TYPE_STRING) {
			if (property->item.value.s != value) {
				free(property->item.value.s);
			} else {
				return property;
			}
		}
		property->item.value.s = psy_strdup(value);
	} else {
		property = psy_property_append_string(self, key, value);
	}
	return property;
}

psy_Property* psy_property_set_font(psy_Property* self, const char* key,
	const char* value)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_FONT);
	if (property) {
		if (property->item.typ == PSY_PROPERTY_TYPE_FONT) {
			free(property->item.value.s);
		}
		property->item.value.s = psy_strdup(value);
		property->item.typ = PSY_PROPERTY_TYPE_FONT;
	} else {
		property = psy_property_append_font(self, key, value);
	}
	return property;
}

psy_Property* psy_property_set_int(psy_Property* self, const char* key,
	int value)
{	
	psy_Property* property;
	
	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_INTEGER);
	if (property) {
		if (psy_property_int_valid(property, value)) {
			property->item.value.i = value;
			property->item.typ = PSY_PROPERTY_TYPE_INTEGER;
		}
	} else {
		property = psy_property_append_int(self, key, value, 0, 0);
	}
	return property;
}

bool psy_property_int_valid(psy_Property* self, int value)
{
	assert(self);

	if (psy_property_int_hasrange(self)) {
		return (value >= self->item.min) && (value <= self->item.max);
	}
	return TRUE;
}

bool psy_property_int_hasrange(psy_Property* self)
{
	return !(self->item.min == 0 && self->item.max == 0);
}

psy_Property* psy_property_set_bool(psy_Property* self, const char* key,
	bool value)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_BOOL);
	if (property) {
		property->item.value.i = (value != FALSE);
		property->item.typ = PSY_PROPERTY_TYPE_BOOL;
		property->item.hint = PSY_PROPERTY_HINT_CHECK;
	} else {
		property = psy_property_append_int(self, key, value, 0, 0);
	}
	return property;
}

psy_Property* psy_property_set_choice(psy_Property* self, const char* key,
	int value)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property) {
		property->item.value.i = value;
		property->item.typ = PSY_PROPERTY_TYPE_CHOICE;
	} else {
		property = psy_property_append_int(self, key, value, 0, 0);
		property->item.typ = PSY_PROPERTY_TYPE_CHOICE;
	}
	return property;
}

psy_Property* psy_property_set_double(psy_Property* self, const char* key,
	double value)
{
	psy_Property* property;

	assert(self);

	property = psy_property_at(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property) {
		property->item.value.d = value;
		property->item.typ = PSY_PROPERTY_TYPE_DOUBLE;
	} else {
		property = psy_property_append_double(self, key, value, 0, 0);
	}
	return property;
}

void psy_property_enumerate(psy_Property* self, void* target,
	psy_PropertyCallback callback)
{
	PropertiesCallbackContext context;

	assert(self);

	context.target = target;
	context.callback = callback;
	context.level = 0;
	properties_enumerate_rec(self, &context);
}

int properties_enumerate_rec(psy_Property* self,
	PropertiesCallbackContext* context)
{
	psy_List* p;

	assert(self);

	p = self->children;
	while (p != NULL) {
		int walkoption = context->callback(context->target, p->entry,
			context->level);
		if (walkoption == 0) {
			return 0;
		} else
			if (walkoption == 1) {
				if (((psy_Property*)(p->entry))->children) {
					++context->level;
					if (!properties_enumerate_rec(p->entry, context)) {
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
static psy_Property* keyfound;
static psy_Property* searchproperty;


psy_Property* psy_property_find(psy_Property* self, const char* key,
	psy_PropertyType typ)
{
	assert(self);

	searchtyp = typ;
	searchkey = key;
	keyfound = 0;
	psy_property_enumerate(self, self, (psy_PropertyCallback)
		OnSearchPropertiesEnum);
	return keyfound;
}

int OnSearchPropertiesEnum(psy_Property* self, psy_Property* property, int level)
{
	assert(self);

	if (property->item.key &&
			(searchtyp == PSY_PROPERTY_TYPE_NONE ||
			property->item.typ == searchtyp) &&
			strcmp(property->item.key, searchkey) == 0) {
		keyfound = property;
		return 0;
	}
	return 1;
}

int OnPropertySearchPropertiesEnum(psy_Property* self, psy_Property* property,
	int level)
{
	assert(self);

	if (property == searchproperty) {
		keyfound = property;
		return 0;
	}
	return 1;
}

psy_Property* psy_property_findsection(psy_Property* self, const char* key)
{
	psy_Property* prev = 0;

	return psy_property_findsectionex(self, key, &prev);
}

int psy_property_insection(psy_Property* self, psy_Property* section)
{
	int rv = 0;

	assert(self);

	if (section) {
		keyfound = 0;
		searchproperty = self;

		psy_property_enumerate(section, section,
			(psy_PropertyCallback)OnPropertySearchPropertiesEnum);
		rv = keyfound != 0;
	}
	return rv;
}

psy_Property* psy_property_findsectionex(psy_Property* self, const char* key,
	psy_Property** prev)
{
	psy_Property* p;
	char* text;
	char seps[] = " .";
	char* token;

	assert(self);

	p = self;
	*prev = p;
	text = psy_strdup(key);
	token = strtok(text, seps);
	while (token != 0) {
		p = psy_property_find(p, token, PSY_PROPERTY_TYPE_SECTION);
		if (!p) {
			break;
		}
		*prev = p;
		token = strtok(0, seps);
	}
	free(text);
	return p;
}

int psy_property_type(psy_Property* self)
{
	assert(self);

	return self->item.typ;
}

const char* psy_property_key(psy_Property* self)
{
	assert(self);

	return self->item.key;
}

int psy_property_as_int(psy_Property* self)
{
	assert(self);

	return self->item.value.i;
}

const char* psy_property_as_str(psy_Property* self)
{
	assert(self);

	return (self->item.value.s) ? self->item.value.s : "";
}

char_dyn_t* psy_property_sections(psy_Property* self)
{
	char_dyn_t* rv;
	psy_Property* p;
	psy_List* tokens;
	psy_List* q;
	uintptr_t size;

	assert(self);

	p = self;
	tokens = NULL;
	size = 1;
	while (p != NULL) {
		if (p->item.typ == PSY_PROPERTY_TYPE_SECTION) {
			psy_list_insert(&tokens, NULL, (void*)psy_property_key(p));
			size += strlen(psy_property_key(p));
			if (p->parent) {
				++size;
			}
		}
		p = p->parent;
	}
	rv = (char_dyn_t*)malloc(size);
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

psy_Property* psy_property_settext(psy_Property* self, const char* text)
{
	assert(self);

	psy_setstr(&self->item.text, text);	
	return self;
}

const char* psy_property_text(psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.text) != 0)
		? self->item.text
		: (psy_strlen(self->item.key) != 0)
			? self->item.key
			: "";
}

psy_Property* psy_property_settranslation(psy_Property* self, const char* text)
{
	assert(self);

	psy_setstr(&self->item.translation, text);	
	return self;
}

psy_Property* psy_property_setcomment(psy_Property* self, const char* text)
{
	assert(self);

	psy_setstr(&self->item.comment, text);	
	return self;
}

const char* psy_property_translation(psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.translation) != 0)
		? self->item.translation
		: psy_property_text(self);
}

psy_Property* psy_property_setshorttext(psy_Property* self, const char* text)
{
	assert(self);

	psy_setstr(&self->item.shorttext, text);	
	return self;
}

const char* psy_property_shorttext(psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.shorttext) != 0)
		? self->item.shorttext
		: psy_strlen(self->item.text) != 0
			? self->item.text
			: psy_strlen(self->item.key) != 0
				? self->item.key
				: "";
}

const char* psy_property_comment(psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.comment) != 0)
		? self->item.comment
		: "";
}

psy_Property* psy_property_setid(psy_Property* self, int id)
{
	assert(self);

	self->item.id = id;	
	return self;
}

int psy_property_id(psy_Property* self)
{
	assert(self);

	return self->item.id;
}

int psy_property_ischoiceitem(psy_Property* self)
{
	assert(self);

	return self->parent &&
		(self->parent->item.typ == PSY_PROPERTY_TYPE_CHOICE);
}

psy_Property* psy_property_sethint(psy_Property* self, psy_PropertyHint hint)
{
	assert(self);

	self->item.hint = hint;	
	return self;
}

psy_PropertyHint psy_property_hint(psy_Property* self)
{
	assert(self);

	return self->item.hint;
}

psy_Property* psy_property_remove(psy_Property* self, psy_Property* property)
{	
	psy_List* q = 0;
	psy_List* p;

	assert(self);

	p = self->children;
	while (p != NULL) {
		if (p->entry == property) {				
			psy_property_deallocate(p->entry);
			q = psy_list_remove(&self->children, p);
			break;
		}
		p = p->next;
	}
	if (q) {
		return q->entry;
	}
	return NULL;
}

void psy_property_clear(psy_Property* self)
{
	psy_List* p;

	assert(self);

	for (p = self->children; p != NULL; psy_list_next(&p)) {		
		psy_property_deallocate((psy_Property*)psy_list_entry(p));
	}
	psy_list_free(self->children);
	self->children = NULL;	
}

uintptr_t psy_property_size(const psy_Property* self)
{	
	assert(self);

	return psy_list_size(self->children);		
}

bool psy_property_empty(const psy_Property* self)
{
	assert(self);

	return self->children == NULL;	
}

psy_List* psy_property_children(psy_Property* self)
{
	assert(self);

	return self->children;
}

psy_Property* psy_property_at_choice(psy_Property* self)
{	
	assert(self);

	return psy_property_at_index(self, psy_property_as_int(self));
}

psy_Property* psy_property_at_index(psy_Property* self, int index)
{
	assert(self);

	if (index >= 0) {
		psy_List* p;

		p = psy_list_at(self->children, index);
		if (p) {
			return (psy_Property*)psy_list_entry(p);
		}
	}
	return NULL;
}
