/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* std */
#include "hashtbl.h"
#include <stdlib.h>
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

psy_TableIterator tableend;

static uintptr_t h(uintptr_t k, uintptr_t size) {  return k % size; }

void psy_table_init(psy_Table* self)
{
	assert(self);

	psy_table_init_keysize(self, psy_TABLEKEYS);	
}

void psy_table_init_keysize(psy_Table* self, uintptr_t keysize)
{
	assert(self);

	self->arraysize = keysize;
	self->keys = (psy_TableHashEntry**)malloc(sizeof(psy_TableHashEntry*) * self->arraysize);
	if (self->keys) {		
		memset(self->keys, 0, sizeof(psy_TableHashEntry*) * self->arraysize);
		self->count = 0;
	}
}

void psy_table_dispose(psy_Table* self)
{
  uintptr_t i;
  psy_TableHashEntry* p;
  psy_TableHashEntry* q;

  assert(self);
  /* double deletion */
  assert(self->keys);  

  for (i = 0; i < self->arraysize; ++i) {
    for (p = self->keys[i]; p != NULL; p = q) {      
	  q = p->next;
      free(p);      
    }
    self->keys[i] = 0;
  }
  self->count = 0;
  free(self->keys);
  self->keys = NULL;
}

void psy_table_dispose_all(psy_Table* self, psy_fp_disposefunc disposefunc)
{
	psy_TableIterator it;

	assert(self);
	if (disposefunc) {
		for (it = psy_table_begin(self);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			disposefunc(psy_tableiterator_value(&it));
			free(psy_tableiterator_value(&it));
		}
	} else {
		for (it = psy_table_begin(self);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			free(psy_tableiterator_value(&it));
		}
	}
	psy_table_dispose(self);
}

void psy_table_clear(psy_Table * self)
{
	uintptr_t keysize;

	assert(self);
	assert(self->keys);	

	keysize = self->arraysize;
	psy_table_dispose(self);
	psy_table_init_keysize(self, keysize);
}

void psy_table_insert(psy_Table* self, uintptr_t k, void* value)
{
	uintptr_t hn;
	psy_TableHashEntry* p;
	psy_TableHashEntry* newentry;

	assert(self);
	hn = h(k, self->arraysize);
	p = 0;
	if (self->keys[hn] != 0) {
		p = self->keys[hn];
		while (p != NULL) {
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

void psy_table_insert_strhash(psy_Table* self, const char* strkey, void* value)
{
	assert(self);

	psy_table_insert(self, psy_strhash(strkey), value);
}

void psy_table_remove(psy_Table* self, uintptr_t k)
{
	uintptr_t hn;
	psy_TableHashEntry* p;
	psy_TableHashEntry* q;

	assert(self);
	hn = h(k, self->arraysize);
	if (self->keys[hn] != 0) {	
		p = self->keys[hn];
		q = 0;
		while (p != NULL) {
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
	  	
	assert(self);
	if (self->count > 0) {
		uintptr_t hn;

		hn = h(k, self->arraysize);
		if (self->keys[hn] != 0) {		
			psy_TableHashEntry* p;

			p = self->keys[hn];
			while (p != NULL) {
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

const void* psy_table_at_const(const psy_Table* self, uintptr_t k)
{
	return psy_table_at((psy_Table*)self, k);
}

void* psy_table_at_strhash(psy_Table* self, const char* strkey)
{
	return psy_table_at(self, psy_strhash(strkey));
}

bool psy_table_exists(const psy_Table* self, uintptr_t k)
{	
	bool rv = 0;
	  	
	assert(self);
	if (self->count > 0) {
		uintptr_t hn;

		hn = h(k, self->arraysize);
		if (self->keys[hn] != 0) {		
			psy_TableHashEntry* p;

			p = self->keys[hn];
			while (p != NULL) {
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

bool psy_table_exists_strhash(const psy_Table* self, const char* strkey)
{
	assert(self);

	return psy_table_exists(self, psy_strhash(strkey));
}

uintptr_t psy_table_size(const psy_Table* self)
{
	assert(self);

	return self->count;
}

uintptr_t psy_table_maxsize(const psy_Table* self)
{
	assert(self);

	if (psy_table_size(self) > 0) {
		return psy_table_maxkey(self) + 1;
	}
	return 0;
}

uintptr_t psy_table_maxkey(const psy_Table* self)
{	
	psy_TableIterator it;
	uintptr_t rv;

	assert(self);
	if (self->count == 0) {
		return psy_INDEX_INVALID;
	}	
	for (rv = 0, it = psy_table_begin((psy_Table*)self);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		if (rv < psy_tableiterator_key(&it)) {
			rv = psy_tableiterator_key(&it);
		}
	}	
	return rv;
}

psy_TableIterator psy_table_begin(psy_Table* self)
{
	psy_TableIterator rv;

	assert(self);
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
	if (self->curr) {
		return self->curr->key;
	} else {
		return 0;
	}
}

int psy_tableiterator_strhash_equals(psy_TableIterator* self, const char* str)
{
	assert(self->curr);
	if (self->curr) {
		return self->curr->key == psy_strhash(str);
	} else {
		return 0;
	}
}

void* psy_tableiterator_value(psy_TableIterator* self)
{
	assert(self->curr);
	return self->curr->value;
}

uintptr_t psy_strhash(const char* str)
{
	uintptr_t hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}



/* psy_Dictionary */

#define PSY_DICTIONARY_HASH_SIZE 401

void psy_dictionary_init(psy_Dictionary* self)
{
	psy_table_init_keysize(&self->container, PSY_DICTIONARY_HASH_SIZE);
}

void psy_dictionary_dispose(psy_Dictionary* self)
{
	psy_table_dispose_all(&self->container, NULL);
}

void psy_dictionary_reset(psy_Dictionary* self)
{
	assert(self);

	psy_table_dispose_all(&self->container, NULL);
	psy_table_init_keysize(&self->container, PSY_DICTIONARY_HASH_SIZE);
}

const char* psy_dictionary_at(const psy_Dictionary* self, const char* key)
{
	assert(self);

	if (psy_strlen(key) > 0) {
		const char* rv;

		rv = psy_table_at_strhash(&(((psy_Dictionary*)self)->container), key);
		if (rv) {
			return rv;
		}
	}
	return NULL;
}

void psy_dictionary_set(psy_Dictionary* self, const char* key, const char* value)
{
	char* old;

	assert(self);

	if (old = psy_table_at_strhash(&self->container, key)) {
		if (value == old) {
			return;
		}
		free(old);
	}
	psy_table_insert_strhash(&self->container, key, psy_strdup(value));
}

void psy_dictionary_set_hash(psy_Dictionary* self, uintptr_t hash_key, const char* value)
{
	char* old;

	assert(self);

	if (old = psy_table_at(&self->container, hash_key)) {
		if (value == old) {
			return;
		}
		free(old);
	}
	psy_table_insert(&self->container, hash_key, psy_strdup(value));
}
