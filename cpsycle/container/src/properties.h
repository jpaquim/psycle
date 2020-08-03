// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_PROPERTIES_H
#define psy_PROPERTIES_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	PSY_PROPERTY_TYP_NONE,
	PSY_PROPERTY_TYP_ROOT,
	PSY_PROPERTY_TYP_INTEGER,
	PSY_PROPERTY_TYP_STRING,
	PSY_PROPERTY_TYP_FONT,
	PSY_PROPERTY_TYP_DOUBLE,
	PSY_PROPERTY_TYP_BOOL,	
	PSY_PROPERTY_TYP_CHOICE,
	PSY_PROPERTY_TYP_USERDATA,
	PSY_PROPERTY_TYP_SECTION,
	PSY_PROPERTY_TYP_ACTION,
} psy_PropertyType;

typedef enum {
	PSY_PROPERTY_HINT_NONE,
	PSY_PROPERTY_HINT_HIDE,
	PSY_PROPERTY_HINT_READONLY,
	PSY_PROPERTY_HINT_EDIT,
	PSY_PROPERTY_HINT_EDITDIR,
	PSY_PROPERTY_HINT_EDITCOLOR,
	PSY_PROPERTY_HINT_INPUT,
	PSY_PROPERTY_HINT_LIST,
	PSY_PROPERTY_HINT_CHECK	
} psy_PropertyHint;

typedef struct {
	char* key;
	char* text;
	char* shorttext;
	char* translation;
	char* comment;
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

void psy_property_init(psy_Property*);
void psy_property_dispose(psy_Property*);
void psy_property_copy(psy_Property*, psy_Property* src);

typedef struct psy_Properties {
	psy_Property item;	
	struct psy_Properties* next;
	struct psy_Properties* children;
	struct psy_Properties* parent;
	void (*dispose)(psy_Property*);	
} psy_Properties;

typedef int (*psy_PropertiesCallback)(void* , psy_Properties*, int level);

void psy_properties_init(psy_Properties*);
void psy_properties_dispose(psy_Properties*);

void psy_properties_init_type(psy_Properties*, const char* key, psy_PropertyType);
psy_Properties* psy_properties_create(void);
void psy_properties_free(psy_Properties*);
psy_Properties* psy_properties_clone(psy_Properties*, int all);
psy_Properties* psy_properties_sync(psy_Properties*, psy_Properties* src);
psy_Properties* psy_properties_append_section(psy_Properties*, const char* key);
psy_Properties* psy_properties_append_string(psy_Properties*, const char* key, const char* value);
psy_Properties* psy_properties_append_font(psy_Properties*, const char* key, const char* value);
psy_Properties* psy_properties_append_choice(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_append_userdata(psy_Properties*, const char* key,
	void* value, void (*dispose)(psy_Property*));
psy_Properties* psy_properties_append_int(psy_Properties*, const char* key, int value, int min, int max);
psy_Properties* psy_properties_append_bool(psy_Properties*, const char* key, bool value);
psy_Properties* psy_properties_append_double(psy_Properties*, const char* key, double value, double min, double max);
psy_Properties* psy_properties_append_action(psy_Properties*, const char* key);
psy_Properties* psy_properties_append_property(psy_Properties*, psy_Properties*);
// getter
const char* psy_properties_key(psy_Properties*);
int psy_properties_type(psy_Properties*);
psy_Properties* psy_properties_at(psy_Properties*, const char* key, psy_PropertyType);
psy_Properties* psy_properties_at_choice(psy_Properties*);
double psy_properties_at_real(psy_Properties*, const char* key, double defaultvalue);
const char* psy_properties_at_str(psy_Properties*, const char* key, const char* defaulttext);
int psy_properties_at_int(psy_Properties*, const char* key, int defaultvalue);
bool psy_properties_at_bool(psy_Properties*, const char* key, bool defaultvalue);
// cast
int psy_properties_as_int(psy_Properties*);
const char* psy_properties_as_str(psy_Properties*);
// setter
psy_Properties* psy_properties_set_int(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_set_str(psy_Properties*, const char* key, const char* value);
psy_Properties* psy_properties_set_font(psy_Properties*, const char* key, const char* value);
psy_Properties* psy_properties_set_bool(psy_Properties*, const char* key, bool value);
psy_Properties* psy_properties_set_choice(psy_Properties*, const char* key, int value);
psy_Properties* psy_properties_set_double(psy_Properties*, const char* key, double value);
void psy_properties_enumerate(psy_Properties*, void* target, psy_PropertiesCallback);
psy_Properties* psy_properties_find(psy_Properties*, const char* key, psy_PropertyType);
psy_Properties* psy_properties_findsection(psy_Properties*, const char* key);
psy_Properties* psy_properties_findsectionex(psy_Properties*, const char* key,
	psy_Properties** prev);
char_dyn_t* psy_properties_sections(psy_Properties*);
int psy_properties_insection(psy_Properties*, psy_Properties* section);
psy_Property* psy_properties_entry(psy_Properties*);
psy_Properties* psy_properties_settext(psy_Properties*, const char* text);
psy_Properties* psy_properties_settranslation(psy_Properties*, const char* text);
psy_Properties* psy_properties_setshorttext(psy_Properties*, const char* text);
psy_Properties* psy_properties_setcomment(psy_Properties*, const char* text);
const char* psy_properties_text(psy_Properties*);
const char* psy_properties_translation(psy_Properties*);
const char* psy_properties_shorttext(psy_Properties*);
const char* psy_properties_comment(psy_Properties*);
psy_Properties* psy_properties_setid(psy_Properties*, int id);
int psy_properties_id(psy_Properties* self);
psy_Properties* psy_properties_sethint(psy_Properties*, psy_PropertyHint);
int psy_properties_ischoiceitem(psy_Properties*);
psy_PropertyHint psy_properties_hint(psy_Properties*);
psy_Properties* psy_properties_next(psy_Properties*);
psy_Properties* psy_properties_remove(psy_Properties*, psy_Properties*);
void psy_properties_clear(psy_Properties*);
uintptr_t psy_properties_size(psy_Properties*);

#ifdef __cplusplus
}

#endif

#endif /* psy_PROPERTIES_H */
