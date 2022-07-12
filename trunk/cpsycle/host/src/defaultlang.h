/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(DEFAULTLANG_H)
#define DEFAULTLANG

#include "../../detail/psyconf.h"

/* container */
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Defines a default dictionary in english. */
void make_translator_default(psy_Dictionary* lang);

#ifdef PSYCLE_MAKE_DEFAULT_LANG
/* Saves the eng default dictionary (psy_dir_config() + en.ini) */
void save_translator_default(void);
/* Saves the keys of the default dictionary (empty translation) */
/* in (psy_dir_config() + lang.ini) */
void save_translator_template(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEFAULTLANG_H */
