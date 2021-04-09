#if !defined(STRCASESTR_H)
#define STRCASESTR_H

#include "psydef.h"
#include "os.h"

#ifdef DIVERSALIS__OS__MICROSOFT

// musl as a whole is licensed under the following standard MIT license :
// Copyright © 2005-2012 Rich Felker
// https://github.com/BlankOn/musl

#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

INLINE int strncasecmp(const char* _l, const char* _r, size_t n)
{
	const unsigned char* l = (void*)_l, * r = (void*)_r;
	if (!n--) return 0;
	for (; *l && *r && n && (*l == *r || tolower(*l) == tolower(*r)); l++, r++, n--);
	return tolower(*l) - tolower(*r);
}

INLINE char* strcasestr(const char* h, const char* n)
{
	size_t l = strlen(n);
	for (; *h; h++) if (!strncasecmp(h, n, l)) return (char*)h;
	return 0;
}

#ifdef __cplusplus
}
#endif

#else
#define _GNU_SOURCE
#include <string.h>
#endif

#endif