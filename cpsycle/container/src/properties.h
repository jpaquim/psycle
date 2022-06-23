/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_PROPERTIES_H
#define psy_PROPERTIES_H

#include "../../detail/psydef.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Composition (Tree) of variant properties
**
**  psy_Property <>-------- psy_List<psy_Property*>
**         <> 1
**          |
**          | 1
**  psy_PropertyItem
**
**  psy_PropertyItem has a key value pair and additional fields
**  to define the type and additional information for the ui to
**  show a description or short text
*/

/* Variant types */
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

/* View/Edit Hints */
typedef enum {
	PSY_PROPERTY_HINT_NONE,
	PSY_PROPERTY_HINT_HIDE,
	PSY_PROPERTY_HINT_EDIT,	
	PSY_PROPERTY_HINT_EDITDIR,
	PSY_PROPERTY_HINT_EDITCOLOR,
	PSY_PROPERTY_HINT_EDITHEX,
	PSY_PROPERTY_HINT_SHORTCUT,
	PSY_PROPERTY_HINT_LIST,
	PSY_PROPERTY_HINT_CHECK,
	PSY_PROPERTY_HINT_SELECTION,
	PSY_PROPERTY_HINT_COMBO
} psy_PropertyHint;

typedef struct psy_PropertyItem {
	char* key;
	char* text;
	char* shorttext;	
	char* comment;
	union {
		char* s;
		intptr_t i;
		double d;
		void* ud;
	} value;
	intptr_t min;
	intptr_t max;
	int typ;
	int hint;
	bool readonly;
	bool allowappend;
	bool disposechildren;
	bool save;
	bool translate;
	intptr_t id;
	bool marked;
} psy_PropertyItem;

void psy_propertyitem_init(psy_PropertyItem*);
void psy_propertyitem_dispose(psy_PropertyItem*);
void psy_propertyitem_copy(psy_PropertyItem*, const psy_PropertyItem* source);

typedef struct psy_Property {
	psy_PropertyItem item;
	psy_List* children;
	struct psy_Property* parent;
	void (*dispose)(psy_PropertyItem*);
} psy_Property;

typedef int (*psy_PropertyCallback)(void*, psy_Property*, uintptr_t level);

/*
** Init/dispose
*/
/* Inits a property, key and type is root. */
void psy_property_init(psy_Property*);
/* Inits a property with a key. if key is zero, the key is "root". */
void psy_property_init_key(psy_Property*, const char* key);
/* Inits a property with a key and type. */
void psy_property_init_type(psy_Property*, const char* key, psy_PropertyType);
/* Disposes a property and its children, but doesn't free self. */
void psy_property_dispose(psy_Property*);

/*
**Allocation/deallocation
*/
/*
** Allocates memory for a property and inits it with a key,
** if key is zero, the key is "root".
*/
psy_Property* psy_property_allocinit_key(const char* key);
/* Disposes a property and its children and frees the memory of self. */
void psy_property_deallocate(psy_Property*);
/* Allocates memory needed to clone(full copy) the source property. */
psy_Property* psy_property_clone(const psy_Property* source);
/* Synchronizes recursively properties with the same keys. */
psy_Property* psy_property_sync(psy_Property*, const psy_Property* source);

/*
** Structure
*/
uintptr_t psy_property_size(const psy_Property*);
bool psy_property_empty(const psy_Property*);
psy_List* psy_property_begin(psy_Property*);
const psy_List* psy_property_begin_const(const psy_Property*);
psy_Property* psy_property_first(psy_Property*);
psy_Property* psy_property_parent(psy_Property*);
const psy_Property* psy_property_parent_const(const psy_Property*);
psy_Property* psy_property_parent_level(psy_Property*, uintptr_t level);
psy_Property* psy_property_remove(psy_Property*, psy_Property*);
psy_Property* psy_property_moveup(psy_Property*);
psy_Property* psy_property_movedown(psy_Property*);
psy_Property* psy_property_prev(psy_Property*);
psy_Property* psy_property_next(psy_Property*);
void psy_property_clear(psy_Property*);
bool psy_property_in_section(const psy_Property*, psy_Property* section);
void psy_property_enumerate(psy_Property*, void* target, psy_PropertyCallback);
psy_Property* psy_property_find(psy_Property*, const char* key, psy_PropertyType);
const psy_Property* psy_property_find_const(const psy_Property*, const char* key, psy_PropertyType);
psy_Property* psy_property_findsection(psy_Property*, const char* key);
const psy_Property* psy_property_findsection_const(const psy_Property*, const char* key);
psy_Property* psy_property_findsectionex(psy_Property*, const char* key,
	psy_Property** prev);
char_dyn_t* psy_property_sections(const psy_Property*);
void psy_property_sort(psy_Property*);
void psy_property_sort_keys(psy_Property*);
void psy_property_sort_keys_recursive(psy_Property*);

/*
** Setter key/value
*/
/* Changes property value or creates and inserts a new property */
psy_Property* psy_property_at(psy_Property*, const char* key, psy_PropertyType);
const psy_Property* psy_property_at_const(const psy_Property*, const char* key,
	psy_PropertyType);
psy_Property* psy_property_at_sorted(psy_Property*, const char* key,
	psy_PropertyType type);
psy_Property* psy_property_at_section(psy_Property*, const char* key);
const psy_Property* psy_property_at_section_const(const psy_Property*, const char* key);
psy_Property* psy_property_at_index(psy_Property*, intptr_t index);
const psy_Property* psy_property_at_index_const(const psy_Property*,
	intptr_t index);
uintptr_t psy_property_index(const psy_Property*);
psy_Property* psy_property_set_bool(psy_Property*, const char* key, bool value);
bool psy_property_at_bool(const psy_Property*, const char* key, bool defaultvalue);
psy_Property* psy_property_set_int(psy_Property*, const char* key, intptr_t value);
intptr_t psy_property_at_int(const psy_Property*, const char* key, intptr_t defaultvalue);
uint32_t psy_property_at_colour(const psy_Property*, const char* key, uint32_t defaultvalue);
psy_Property* psy_property_set_double(psy_Property*, const char* key, double value);
double psy_property_at_real(const psy_Property*, const char* key, double defaultvalue);
psy_Property* psy_property_set_str(psy_Property*, const char* key, const char* value);
const char* psy_property_at_str(const psy_Property*, const char* key, const char* defaulttext);
psy_Property* psy_property_set_font(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_set_choice(psy_Property*, const char* key, intptr_t value);
psy_Property* psy_property_at_choice(psy_Property*);
bool psy_property_is_choice_item(const psy_Property*);
intptr_t psy_property_choiceitem_index(const psy_Property*);
/* Appends a property. If source typ is root, it changes to section. */
psy_Property* psy_property_append_property(psy_Property*, psy_Property*);
/* Creates and appends a a new property */
psy_Property* psy_property_append_section(psy_Property*, const char* key);
psy_Property* psy_property_append_str(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_append_font(psy_Property*, const char* key, const char* value);
psy_Property* psy_property_append_choice(psy_Property*, const char* key, intptr_t value);
psy_Property* psy_property_append_userdata(psy_Property*, const char* key,
	void* value, void (*dispose)(psy_PropertyItem*));
psy_Property* psy_property_append_int(psy_Property*, const char* key, intptr_t value, intptr_t min, intptr_t max);
psy_Property* psy_property_append_bool(psy_Property*, const char* key, bool value);
psy_Property* psy_property_append_double(psy_Property*, const char* key, double value, double min, double max);
psy_Property* psy_property_append_action(psy_Property*, const char* key);

/*
** Item setter/getter
*/
/* Definition */
void psy_property_change_key(psy_Property*, const char* key);
const char* psy_property_key(const psy_Property*);
char_dyn_t* psy_property_fullkey(const psy_Property* self);
int psy_property_type(const psy_Property*);
psy_Property* psy_property_setreadonly(psy_Property*, bool on);
bool psy_property_readonly(const psy_Property*);
psy_Property* psy_property_sethint(psy_Property*, psy_PropertyHint);
psy_PropertyHint psy_property_hint(const psy_Property*);
psy_Property* psy_property_preventsave(psy_Property*);
psy_Property* psy_property_enablesave(psy_Property*);
psy_Property* psy_property_enableappend(psy_Property*);
psy_Property* psy_property_preventtranslate(psy_Property*);
bool psy_property_translation_prevented(const psy_Property*);
bool psy_property_hasid(const psy_Property* self, int id);
/* Value */
psy_Property* psy_property_setitem_bool(psy_Property*, bool value);
bool psy_property_item_bool(const psy_Property*);
psy_Property* psy_property_setitem_int(psy_Property*, intptr_t value);
intptr_t psy_property_item_int(const psy_Property*);
bool psy_property_int_valid(const psy_Property*, intptr_t value);
bool psy_property_int_hasrange(const psy_Property*);
psy_Property* psy_property_setitem_double(psy_Property*, double value);
double psy_property_item_double(const psy_Property*);
psy_Property* psy_property_set_item_str(psy_Property*, const char* str);
const char* psy_property_item_str(const psy_Property*);
psy_Property* psy_property_setitem_font(psy_Property*, const char* value);
const char* psy_property_item_font(const psy_Property*);
psy_Property* psy_property_item_choice_parent(psy_Property*);
bool psy_property_haskey(const psy_Property*, const char* key);
bool psy_property_hastype(const psy_Property*, psy_PropertyType);
uint32_t psy_property_item_colour(const psy_Property*);
bool psy_property_ishex(const psy_Property*);
bool psy_property_isbool(const psy_Property*);
bool psy_property_is_int(const psy_Property*);
bool psy_property_is_string(const psy_Property*);
bool psy_property_is_font(const psy_Property*);
bool psy_property_is_action(const psy_Property*);
bool psy_property_issection(const psy_Property*);

/* Description */
psy_Property* psy_property_set_id(psy_Property*, intptr_t id);
intptr_t psy_property_id(const psy_Property* self);
psy_Property* psy_property_settext(psy_Property*, const char* text);
const char* psy_property_text(const psy_Property*);
psy_Property* psy_property_setshorttext(psy_Property*, const char* text);
const char* psy_property_shorttext(const psy_Property*);
psy_Property* psy_property_set_comment(psy_Property*, const char* text);
const char* psy_property_comment(const psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* psy_PROPERTIES_H */
