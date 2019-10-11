// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(HASHTABL_H)
#define HASHTABL_H

#define TABLEKEYS 256

typedef struct HashEntry {
	int key;
	void* value;
	struct HashEntry* next;
} HashEntry;

typedef struct {
	HashEntry* keys[TABLEKEYS];
	int size;
	int count;	
} Table;

typedef struct {
	int pos;
	int count;
	HashEntry* curr;
	Table* table;
} TableIterator;

void tableiterator_init(TableIterator*, Table*);
void tableiterator_inc(TableIterator*);
int tableiterator_equal(const TableIterator* lhs, const TableIterator* rhs);
int tableiterator_key(TableIterator*);
void* tableiterator_value(TableIterator*);

void table_init(Table*);
void table_dispose(Table*);
void table_insert(Table*, int k, void* value);
void table_remove(Table*, int k);
void* table_at(Table*, int k);
int table_exists(Table*, int k);
TableIterator table_begin(Table*);
const TableIterator* table_end(void);

int freetableentry(void*, void*, HashEntry*);

#endif
