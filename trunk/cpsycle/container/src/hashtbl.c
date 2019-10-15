// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "hashtbl.h"
#include <stdlib.h>
#include <assert.h>

static int h(int k, int size) {  return k % size; }
static TableIterator tableend;

void table_init(Table* self)
{
  int i;

  self->size = TABLEKEYS;
  for (i = 0; i < self->size; ++i) {
    self->keys[i] = 0;
  }
  self->count = 0; 
}

void table_dispose(Table* self)
{
  int i;
  HashEntry* p;
  HashEntry* q;

  for (i = 0; i < self->size; ++i) {    
    for (p = self->keys[i]; p != 0; p = q) {      
	  q = p->next;
      free(p);      
    }
    self->keys[i] = 0;
  }
  self->count = 0;
}

void table_insert(Table* self, int k, void* value)
{
	int hn;
	HashEntry* p;
	HashEntry* newentry;

	hn = h(k, self->size);
	p = 0;
	if (self->keys[hn] != 0) {
		p = self->keys[hn];
		while (p != 0) {
			if (k == p->key) {
				p->value = value;
				break;
			}
			p = p->next;
		}
	}
	if (!p) {
		newentry = malloc(sizeof(HashEntry));
		newentry->key = k;
		newentry->value = value;
		newentry->next = 0;
		if (self->keys[hn] == 0) {
			self->keys[hn] = newentry;
		} else {
			p = self->keys[hn];
			while (p->next != 0) {
				p = p->next;
			}
			p->next = newentry;
		}				
		++self->count;
	}
}

void table_remove(Table* self, int k)
{
	int hn;
	HashEntry* p;
	HashEntry* q;

	hn = h(k, self->size);
	if (self->keys[hn] != 0) {	
		p = self->keys[hn];
		q = 0;
		while (p != 0) {
			if (k == p->key) {
				if (q) {				
					q->next = p->next;				
				} else {
					self->keys[hn] = p->next;
				}
				free(p);
				--self->count;
				break;
			}
			q = p;
			p = p->next;
		}		
	}
}

void* table_at(Table* self, int k)
{
	void* rv = 0;
	  	
	if (self->count > 0) {
		int hn;

		hn = h(k, self->size);
		if (self->keys[hn] != 0) {		
			HashEntry* p;

			p = self->keys[hn];
			while (p != 0) {
				if (k == p->key) {
					rv = p->value;
					break;
				}
				p = p->next;
			}		
		}
	}
	return rv;
}

int table_exists(Table* self, int k)
{	
	return (self->count > 0) && (self->keys[h(k, self->size)] != 0);
}

int freetableentry(void* context, void* param, HashEntry* entry)
{
	free(entry->value);
	return 1;
}

TableIterator table_begin(Table* self)
{
	TableIterator rv;

	tableiterator_init(&rv, self);
	return rv;
}

const TableIterator* table_end(void)
{
	return &tableend;
}

void tableiterator_init(TableIterator* self, Table* table)
{
	assert(table);
	self->table = table;
	if (table->count == 0) {
		self->curr = 0;
		return;
	}
	self->pos = 0;
	self->count = 0;	
	while (!table->keys[self->pos] && self->pos < TABLEKEYS) {		
		++self->pos;		
	}	
	self->curr = self->pos < TABLEKEYS ? table->keys[self->pos] : 0;
}

int tableiterator_equal(const TableIterator* lhs, const TableIterator* rhs)
{
	return lhs->curr == rhs->curr;
}

void tableiterator_inc(TableIterator* self)
{
	if (self->count == self->table->count) {
		self->curr = 0;
		return;
	}
	if (self->curr) {
		self->curr = self->curr->next;		
	}	
	while (!self->curr && self->pos + 1 < TABLEKEYS) {
		++self->pos;
		self->curr = self->table->keys[self->pos];		
	}
	++self->count;
}

int tableiterator_key(TableIterator* self)
{
	assert(self->curr);
	return self->curr->key;
}

void* tableiterator_value(TableIterator* self)
{
	assert(self->curr);
	return self->curr->value;
}
