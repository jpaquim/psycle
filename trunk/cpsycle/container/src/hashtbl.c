// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "hashtbl.h"
#include <stdlib.h>
#include <assert.h>

psy_TableIterator tableend;

static uintptr_t h(uintptr_t k, uintptr_t size) {  return k % size; }

void psy_table_init(psy_Table* self)
{
  uintptr_t i;

  self->arraysize = psy_TABLEKEYS;
  for (i = 0; i < self->arraysize; ++i) {
    self->keys[i] = 0;
  }
  self->count = 0; 
}

void psy_table_dispose(psy_Table* self)
{
  uintptr_t i;
  psy_TableHashEntry* p;
  psy_TableHashEntry* q;

  for (i = 0; i < self->arraysize; ++i) {
    for (p = self->keys[i]; p != 0; p = q) {      
	  q = p->next;
      free(p);      
    }
    self->keys[i] = 0;
  }
  self->count = 0;
}

void psy_table_clear(psy_Table* self)
{
	psy_table_dispose(self);
	psy_table_init(self);
}

void psy_table_insert(psy_Table* self, uintptr_t k, void* value)
{
	uintptr_t hn;
	psy_TableHashEntry* p;
	psy_TableHashEntry* newentry;

	hn = h(k, self->arraysize);
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
		newentry = malloc(sizeof(psy_TableHashEntry));
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

void psy_table_remove(psy_Table* self, uintptr_t k)
{
	uintptr_t hn;
	psy_TableHashEntry* p;
	psy_TableHashEntry* q;

	hn = h(k, self->arraysize);
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

void* psy_table_at(psy_Table* self, uintptr_t k)
{
	void* rv = 0;
	  	
	if (self->count > 0) {
		uintptr_t hn;

		hn = h(k, self->arraysize);
		if (self->keys[hn] != 0) {		
			psy_TableHashEntry* p;

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

int psy_table_exists(psy_Table* self, uintptr_t k)
{	
	int rv = 0;
	  	
	if (self->count > 0) {
		uintptr_t hn;

		hn = h(k, self->arraysize);
		if (self->keys[hn] != 0) {		
			psy_TableHashEntry* p;

			p = self->keys[hn];
			while (p != 0) {
				if (k == p->key) {
					rv = 1;
					break;
				}
				p = p->next;
			}		
		}
	}
	return rv;
}

uintptr_t psy_table_size(psy_Table* self)
{
	return self->count;
}

uintptr_t psy_table_freetableentry(void* context, void* param, psy_TableHashEntry* entry)
{
	free(entry->value);
	return 1;
}

psy_TableIterator psy_table_begin(psy_Table* self)
{
	psy_TableIterator rv;

	psy_tableiterator_init(&rv, self);
	return rv;
}

const psy_TableIterator* psy_table_end(void)
{
	return &tableend;
}

void psy_tableiterator_init(psy_TableIterator* self, psy_Table* table)
{
	assert(table);
	self->table = table;
	if (table->count == 0) {
		self->curr = 0;
		return;
	}
	self->pos = 0;
	self->count = 0;	
	while (!table->keys[self->pos] && self->pos < psy_TABLEKEYS) {		
		++self->pos;		
	}	
	self->curr = self->pos < psy_TABLEKEYS ? table->keys[self->pos] : 0;
}

uintptr_t psy_tableiterator_equal(const psy_TableIterator* lhs, const psy_TableIterator* rhs)
{
	return lhs->curr == rhs->curr;
}

void psy_tableiterator_inc(psy_TableIterator* self)
{
	if (self->count == self->table->count) {
		self->curr = 0;
		return;
	}
	if (self->curr) {
		self->curr = self->curr->next;		
	}	
	while (!self->curr && self->pos + 1 < psy_TABLEKEYS) {
		++self->pos;
		self->curr = self->table->keys[self->pos];		
	}
	++self->count;
}

uintptr_t psy_tableiterator_key(psy_TableIterator* self)
{
	assert(self->curr);
	return self->curr->key;
}

void* psy_tableiterator_value(psy_TableIterator* self)
{
	assert(self->curr);
	return self->curr->value;
}
