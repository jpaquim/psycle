// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_PROPERTIES_H
#define psy_PROPERTIES_H

#include "../../detail/psydef.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

// Composition (Tree) of variant properties
//
// psy_Property <>-------- psy_List<psy_Property*>
//        <> 1
//         |
//         | 1
// psy_PropertyItem
//
// psy_PropertyItem has a key value pair and additional fields
// to define the type and additional information for the ui to
// show a description, translation or short text.

// Variant types
typedef enum {
	PSY_PROPERTY_TYPE_NONE,
	PSY_PROPERTY_TYPE_ROOT,
	PSY_PROPERTY_TYPE_INTEGER,
	PSY_PROPERTY_TYPE_STRING,
	PSY_PROPERTY_TYPE_FONT,
	PSY_PROPERTY_TYPE_DOUBLE,
	PSY_PROPERTY_TYPE_BOOL,	
	PSY_PROPERTY_TYPE_CHOICE,
	PSY_PROPERTY_TYPE_USERDATA,
	PSY_PROPERTY_TYPE_SECTION,
	PSY_PROPERTY_TYPE_ACTION
} psy_PropertyType;

// View/Edit Hints
typedef enum {
	PSY_PROPERTY_HINT_NONE,
	PSY_PROPERTY_HINT_HIDE,
	PSY_PROPERTY_HINT_EDIT,
	PSY_PROPERTY_HINT_EDITDIR,
	PSY_PROPERTY_HINT_EDITCOLOR,
	PSY_PROPERTY_HINT_INPUT,
	PSY_PROPERTY_HINT_LIST,
	PSY_PROPERTY_HINT_CHECK	
} psy_PropertyHint;

typedef struct psy_PropertyItem {
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
	bool readonly;
	int disposechildren;
	int save;
	int id;
	int allowappend;
} psy_PropertyItem;

void psy_propertyitem_init(psy_PropertyItem*);
void psy_propertyitem_dispose(psy_PropertyItem*);
void psy_propertyitem_copy(psy_PropertyItem*, psy_PropertyItem* source);

typedef struct psy_Property {
	psy_PropertyItem item;
	psy_List* children;
	struct psy_Property* parent;
	void (*dispose)(psy_PropertyItem*);
} psy_Property;

typedef int (*psy_PropertyCallback)(void* , psy_Property*, int level);

// Init/dispose
// {
// Inits a property, key and type is root.
void psy_property_init(psy_Property*);
// Inits a property with a key. if key is zero, the key is "root".
void psy_property_init_key(psy_Property*, const char* key);
// Inits a property with a key and type.
void psy_property_init_type(psy_Property*, const char* key, psy_PropertyType);
// Disposes a property and its children, but doesn't free self.
void psy_property_dispose(psy_Property*);
// }

// Allocation/deallocation
// {
// Allocates memory for a property and inits it with key "root".
psy_Property* psy_property_allocinit_key(const char* key);
// Disposes a property and its children and frees the memory of self.
void psy_property_deallocate(psy_Property*);
// Allocates memory needed to clone(full copy) the source property.
psy_Property* psy_property_clone(psy_Property* source);
// }
// Synchronizes recursively properties with same keys.
psy_Property* psy_property_sync(psy_Property*, psy_Property* source);
// Appends a property. If source typ is root, it changes to section.
psy_Property* psy_property_append_property(psy_Property*, psy_Property*);
// Creates and appends for different types a property to its children list.
// {
psy_Property* psy_property_append_section(psy_Property*, const char* key);
psy_Property* psy_property_append_string(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_append_font(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_append_choice(psy_Property*, const char* key, int value);
psy_Property* psy_property_append_userdata(psy_Property*, const char* key,
	void* value, void (*dispose)(psy_PropertyItem*));
psy_Property* psy_property_append_int(psy_Property*, const char* key, int value, int min, int max);
psy_Property* psy_property_append_bool(psy_Property*, const char* key, bool value);
psy_Property* psy_property_append_double(psy_Property*, const char* key, double value, double min, double max);
psy_Property* psy_property_append_action(psy_Property*, const char* key);
// }
// Getter/Cast
// {
const char* psy_property_key(psy_Property*);
int psy_property_type(psy_Property*);
psy_Property* psy_property_at(psy_Property*, const char* key, psy_PropertyType);
psy_Property* psy_property_at_choice(psy_Property*);
psy_Property* psy_property_at_index(psy_Property*, int index);
double psy_property_at_real(psy_Property*, const char* key, double defaultvalue);
const char* psy_property_at_str(psy_Property*, const char* key, const char* defaulttext);
int psy_property_at_int(psy_Property*, const char* key, int defaultvalue);
bool psy_property_at_bool(psy_Property*, const char* key, bool defaultvalue);
int psy_property_as_int(psy_Property*);
const char* psy_property_as_str(psy_Property*);
bool psy_property_int_valid(psy_Property*, int value);
bool psy_property_int_hasrange(psy_Property*);
// }
// Setter
// {
psy_Property* psy_property_set_section(psy_Property*, const char* sectionname);
psy_Property* psy_property_set_int(psy_Property*, const char* key, int value);
psy_Property* psy_property_set_str(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_set_font(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_set_bool(psy_Property*, const char* key, bool value);
psy_Property* psy_property_set_choice(psy_Property*, const char* key, int value);
psy_Property* psy_property_set_double(psy_Property*, const char* key, double value);
// }
void psy_property_enumerate(psy_Property*, void* target, psy_PropertyCallback);
psy_Property* psy_property_find(psy_Property*, const char* key, psy_PropertyType);
psy_Property* psy_property_findsection(psy_Property*, const char* key);
psy_Property* psy_property_findsectionex(psy_Property*, const char* key,
	psy_Property** prev);
char_dyn_t* psy_property_sections(psy_Property*);
int psy_property_insection(psy_Property*, psy_Property* section);
psy_PropertyItem* psy_property_entry(psy_Property*);
psy_Property* psy_property_settext(psy_Property*, const char* text);
psy_Property* psy_property_settranslation(psy_Property*, const char* text);
psy_Property* psy_property_setshorttext(psy_Property*, const char* text);
psy_Property* psy_property_setcomment(psy_Property*, const char* text);
psy_Property* psy_property_setreadonly(psy_Property*, bool on);
bool psy_property_readonly(const psy_Property*);
const char* psy_property_text(psy_Property*);
const char* psy_property_translation(psy_Property*);
const char* psy_property_shorttext(psy_Property*);
const char* psy_property_comment(psy_Property*);
psy_Property* psy_property_setid(psy_Property*, int id);
int psy_property_id(psy_Property* self);
psy_Property* psy_property_sethint(psy_Property*, psy_PropertyHint);
int psy_property_ischoiceitem(psy_Property*);
psy_PropertyHint psy_property_hint(psy_Property*);
psy_Property* psy_property_remove(psy_Property*, psy_Property*);
void psy_property_clear(psy_Property*);
uintptr_t psy_property_size(const psy_Property*);
bool psy_property_empty(const psy_Property*);
psy_List* psy_property_children(psy_Property*);

#ifdef __cplusplus
}

#endif

#endif /* psy_PROPERTIES_H */
