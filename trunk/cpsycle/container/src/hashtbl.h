// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(HASHTABL)
#define HASHTABL

struct IntHashEntryStruct {
  int key;
  void* value;
  struct IntHashEntryStruct* next;
};

typedef struct IntHashEntryStruct IntHashEntry;
typedef struct IntHashEntryStruct* PIntHashEntry;

struct IntHashTableStruct {
  PIntHashEntry keys[256];
  int size;
};

typedef struct IntHashTableStruct IntHashTable;
typedef struct IntHashTableStruct* PIntHashTable;


void InitIntHashTable(PIntHashTable table, int size);
void DisposeIntHashTable(PIntHashTable table);
void InsertIntHashTable(PIntHashTable table, int k, void* value);
void* SearchIntHashTable(PIntHashTable table, int k);
int ExistsIntHashTable(PIntHashTable table, int k);

#endif