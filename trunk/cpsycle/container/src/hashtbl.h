// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(HASHTABL_H)
#define HASHTABL_H

#include <stddef.h>

#define TABLEKEYS 256

typedef struct HashEntry {
	size_t key;
	void* value;
	struct HashEntry* next;
} HashEntry;

typedef struct {
	HashEntry* keys[TABLEKEYS];
	size_t arraysize;
	size_t count;	
} Table;

typedef struct {
	size_t pos;
	size_t count;
	HashEntry* curr;
	Table* table;
} TableIterator;

void tableiterator_init(TableIterator*, Table*);
void tableiterator_inc(TableIterator*);
size_t tableiterator_equal(const TableIterator* lhs, const TableIterator* rhs);
size_t tableiterator_key(TableIterator*);
void* tableiterator_value(TableIterator*);

void table_init(Table*);
void table_dispose(Table*);
void table_insert(Table*, size_t k, void* value);
void table_remove(Table*, size_t k);
void* table_at(Table*, size_t k);
size_t table_size(Table*);
size_t table_exists(Table*, size_t k);
TableIterator table_begin(Table*);
const TableIterator* table_end(void);

size_t freetableentry(void*, void*, HashEntry*);

#endif
