// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "hashtbl.h"
#include <stdlib.h>

static int h(int k, int size) {  return k % size; }

void table_init(Table* self)
{
  int i;

  self->size = TABLEKEYS;
  for (i = 0; i < self->size; ++i) {
    self->keys[i] = 0;
  }
  self->count = 0;
  self->keymax = 0;
  self->keymin = 2147483647;
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
		if (self->keymin > newentry->key) {
			self->keymin = newentry->key;
		}
		if (self->keymax < newentry->key) {
			self->keymax = newentry->key;
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

