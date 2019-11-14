// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(HASHTABL_H)
#define HASHTABL_H

#include "../../detail/stdint.h"

#define TABLEKEYS 256

typedef struct HashEntry {
	uintptr_t key;
	void* value;
	struct HashEntry* next;
} HashEntry;

typedef struct {
	HashEntry* keys[TABLEKEYS];
	uintptr_t arraysize;
	uintptr_t count;
} Table;

typedef struct {
	uintptr_t pos;
	uintptr_t count;
	HashEntry* curr;
	Table* table;
} TableIterator;

void tableiterator_init(TableIterator*, Table*);
void tableiterator_inc(TableIterator*);
uintptr_t tableiterator_equal(const TableIterator* lhs, const TableIterator* rhs);
uintptr_t tableiterator_key(TableIterator*);
void* tableiterator_value(TableIterator*);

void table_init(Table*);
void table_dispose(Table*);
void table_insert(Table*, uintptr_t k, void* value);
void table_remove(Table*, uintptr_t k);
void* table_at(Table*, uintptr_t k);
uintptr_t table_size(Table*);
int table_exists(Table*, uintptr_t k);
TableIterator table_begin(Table*);
const TableIterator* table_end(void);

uintptr_t freetableentry(void*, void*, HashEntry*);

#endif
