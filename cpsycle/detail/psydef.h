// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_DEF_H)
#define psy_DEF_H

#include "../../detail/psyconf.h"
#include "../../detail/stdint.h"
#include <stddef.h>
#include "psyversion.h"

#ifndef __cplusplus
typedef uint8_t bool;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

// psycle status

#define PSY_OK 0
#define PSY_ERRFILE 1

#ifndef INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define INLINE extern inline
# elif __GNUC__
#  define INLINE inline
# elif _MSC_VER
#  define INLINE __inline
# endif
#endif /* INLINE */


#endif
