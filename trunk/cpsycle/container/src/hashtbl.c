// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "hashtbl.h"
#include <stdlib.h>


void InitIntHashTable(PIntHashTable table, int size) {
  int i;
  table->size = size;
  for (i = 0; i < size; ++i) {
    table->keys[i] = 0;
  }
  table->count = 0;
  table->keymax = 0;
  table->keymin = 2147483647;
}

void DisposeIntHashTable(PIntHashTable table) {
  int i;
  PIntHashEntry ptr;
  PIntHashEntry next;

  for (i = 0; i < table->size; ++i) {
    ptr = table->keys[i];
    while (ptr != 0) {
      next = ptr->next;
      free(ptr);
      ptr = next;
    }
    table->keys[i] = 0;
  }
  table->count = 0;
}

static int inth(int k, int size) {  
  return k % size;
}

void InsertIntHashTable(PIntHashTable table, int k, void* value) {
	int hn;
	PIntHashEntry p;
	PIntHashEntry newentry;

	hn = inth(k, table->size);
	p = 0;
	if (table->keys[hn] != 0) {
		p = table->keys[hn];
		while (p != 0) {
			if (k == p->key) {
				p->value = value;
				break;
			}
			p = p->next;
		}
	}
	if (!p) {
		newentry = malloc(sizeof(IntHashEntry));
		newentry->key = k;
		newentry->value = value;
		newentry->next = 0;
		if (table->keys[hn] == 0) {
			table->keys[hn] = newentry;
		} else {
		p = table->keys[hn];
		while (p->next != 0) {
			p = p->next;
		}
		p->next = newentry;
		}
		if (table->keymin > newentry->key) {
			table->keymin = newentry->key;
		}
		if (table->keymax < newentry->key) {
			table->keymax = newentry->key;
		}
		++table->count;
	}
}

void RemoveIntHashTable(PIntHashTable table, int k)
{
	int hn;
	PIntHashEntry p;
	PIntHashEntry q;

	hn = inth(k, table->size);
	if (table->keys[hn] != 0) {	
		p = table->keys[hn];
		q = 0;
		while (p != 0) {
			if (k == p->key) {
				if (q) {				
					q->next = p->next;				
				} else {
					table->keys[hn] = p->next;
				}
				free(p);
				--table->count;
				break;
			}
			q = p;
			p = p->next;
		}  
	}
}

void* SearchIntHashTable(PIntHashTable table, int k) {
  int hn;
  PIntHashEntry ptr;
  void* rv;

  rv = 0;

  hn = inth(k, table->size);
  if (table->keys[hn] == 0) {
    rv = 0;
  } else
  {
    ptr = table->keys[hn];
    while (ptr != 0) {
      if (k == ptr->key) {
	rv = ptr->value;
	return rv;
      }
      ptr = ptr->next;
    }
    return 0;
  }
  return 0;
}

int ExistsIntHashTable(PIntHashTable table, int k)
{
	int hn;
	
	hn = inth(k, table->size);
	return table->keys[hn] != 0;
}

