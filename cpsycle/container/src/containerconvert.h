// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_CONTAINERCONVERT_H)
#define psy_CONTAINERCONVERT_H

// local
#include "list.h"
#include "hashtbl.h"

#ifdef __cplusplus
extern "C" {
#endif

// alloc array;
void** psy_array_alloc(uintptr_t size);
void psy_array_clear(void**, uintptr_t size);
// list <-> array
void psy_list_to_array(void** dest, uintptr_t maxsize, psy_List* source);
psy_List* psy_array_to_list(void** source, uintptr_t size);
// table -> array
void psy_table_to_array(void** dest, uintptr_t maxsize, psy_Table* source);

#ifdef __cplusplus
}
#endif

#endif /* psy_CONTAINERCONVERT_H */
