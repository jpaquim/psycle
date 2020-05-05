// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_HASHTABL_H
#define psy_HASHTABL_H

#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_TABLEKEYS 256

typedef struct psy_TableHashEntry {
	uintptr_t key;
	void* value;
	struct psy_TableHashEntry* next;
} psy_TableHashEntry;

typedef struct {
	psy_TableHashEntry* keys[psy_TABLEKEYS];
	uintptr_t arraysize;
	uintptr_t count;
} psy_Table;

typedef struct {
	uintptr_t pos;
	uintptr_t count;
	psy_TableHashEntry* curr;
	psy_Table* table;
} psy_TableIterator;

extern psy_TableIterator tableend;

void psy_tableiterator_init(psy_TableIterator*, psy_Table*);
void psy_tableiterator_inc(psy_TableIterator*);
uintptr_t psy_tableiterator_equal(const psy_TableIterator* lhs, const psy_TableIterator* rhs);
uintptr_t psy_tableiterator_key(psy_TableIterator*);
int psy_tableiterator_strhash_equals(psy_TableIterator*, const char* str);
void* psy_tableiterator_value(psy_TableIterator*);

void psy_table_init(psy_Table*);
void psy_table_dispose(psy_Table*);
void psy_table_insert(psy_Table*, uintptr_t key, void* value);
void psy_table_insert_strhash(psy_Table*, const char* strkey, void* value);
void psy_table_remove(psy_Table*, uintptr_t key);
void* psy_table_at(psy_Table*, uintptr_t key);
void* psy_table_at_strhash(psy_Table*, const char* strkey);
uintptr_t psy_table_size(psy_Table*);
int psy_table_exists(psy_Table*, uintptr_t key);
int psy_table_exists_strhash(psy_Table*, const char* strkey);
void psy_table_clear(psy_Table*);
psy_TableIterator psy_table_begin(psy_Table*);
const psy_TableIterator* psy_table_end(void);

uintptr_t psy_table_freetableentry(void*, void*, psy_TableHashEntry*);

#ifdef __cplusplus
}
#endif

#endif /* psy_HASHTABL_H */
