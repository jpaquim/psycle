/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "inireader.h"
/* std */
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

#define MAXSTRINGSIZE 4096

typedef enum {
	INIREADER_STATE_READKEY = 0,
	INIREADER_STATE_READVAL = 1,
	INIREADER_STATE_ADDVAL = 2,
	INIREADER_STATE_READSECTION = 3,
	INIREADER_STATE_ADDSECTION = 4,
	INIREADER_STATE_READCOMMENT = 5,
} PropertiesIOState;

static int reallocstr(char** str, size_t size, size_t* cap);

static char_dyn_t* lastkey(const char* key)
{
	char_dyn_t* rv;
	char* text;
	char seps[] = ".";
	char* token;
	char* lasttoken;

	text = psy_strdup(key);
	token = strtok(text, seps);
	lasttoken = NULL;
	while (token != 0) {
		lasttoken = token;
		token = strtok(0, seps);
	}
	rv = psy_strdup(lasttoken);
	free(text);
	return rv;
}

void psy_inireader_init(psy_IniReader* self)
{
	assert(self);

	psy_signal_init(&self->signal_comment);
	psy_signal_init(&self->signal_read);
	psy_signal_init(&self->signal_section);
	self->section = NULL;
	self->cpp_comment = FALSE;
	self->dos_to_utf8 = FALSE;
}

void psy_inireader_dispose(psy_IniReader* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_comment);
	psy_signal_dispose(&self->signal_read);
	psy_signal_dispose(&self->signal_section);
	free(self->section);
	self->section = NULL;
}

int inireader_load(psy_IniReader* self, const char* path)
{
	FILE* fp;
				
	fp = fopen(path, "rb");
	if (fp) {
		int c;
		uintptr_t cp = 0;
		PropertiesIOState state;
		char* key = 0;			
		size_t keycap = 0;
		char* value = 0;
		size_t valcap = 0;
		bool testcppcomment;

		state = INIREADER_STATE_READKEY;
		reallocstr(&key, 256, &keycap);
		reallocstr(&value, 256, &valcap);
		free(self->section);
		self->section = NULL;
		testcppcomment = FALSE;
		while ((c = fgetc(fp)) != EOF) {	
			switch (state) {
				case INIREADER_STATE_READKEY:			
				if (c == '\r') {
					state = INIREADER_STATE_READKEY;
				} else if (c == '\n') {
					state = INIREADER_STATE_READKEY;
				} else if (c == ';') {
					state = INIREADER_STATE_READCOMMENT;
				} else if (c == '[') {
					state = INIREADER_STATE_READSECTION;
					key[cp] = '\0';
					cp = 0;
				} else if (c == '=') {
					state = INIREADER_STATE_READVAL;
					key[cp] = '\0';
					cp = 0;					
				} else { /* read key*/
					if (testcppcomment && c == '/') {
						--cp;
						key[cp] = '\0';
						state = INIREADER_STATE_READCOMMENT;
						testcppcomment = FALSE;
					} else if (self->cpp_comment && c == '/') {
						testcppcomment = TRUE;
					}
					if (state != INIREADER_STATE_READCOMMENT &&
						c != '\"' &&
						!reallocstr(&key, cp, &keycap)) {
						key[cp] = c;
						++cp;
					}
				}
				break;
			case INIREADER_STATE_READVAL:
				if (c == '\n' || c == '\r') {
					state = INIREADER_STATE_ADDVAL;
					value[cp] = '\0';
					cp = 0;
				} else if (!reallocstr(&value, cp, &valcap)) {
					value[cp] = c;
				}
				++cp;
				break;			
			case INIREADER_STATE_READSECTION:
				if (c == ']') {
					state = INIREADER_STATE_ADDSECTION;
					key[cp] = '\0';
					cp = 0;					
				} else if (!reallocstr(&key, cp, &keycap)) {
					key[cp] = c;
				}
				++cp;
				break;			
			case INIREADER_STATE_READCOMMENT:
				if (c == '\r' || c == '\n') {
					state = INIREADER_STATE_READKEY;
				}				
			break;
			default:
				break;
			}
			if (state == INIREADER_STATE_ADDVAL) {				
				if (self->dos_to_utf8 && psy_strlen(value)) {
					char* out;
									
					out = psy_dos_to_utf8(value, out);
					psy_signal_emit(&self->signal_read, self, 2, key, out);
					free(out);
					out = NULL;
				} else {
					psy_signal_emit(&self->signal_read, self, 2, key, value);
				}
				cp = 0;
				state = INIREADER_STATE_READKEY;
			} else if (state == INIREADER_STATE_ADDSECTION) {
				psy_strreset(&self->section, key);
				psy_signal_emit(&self->signal_section, self, 1, key);			
				cp = 0;
				state = INIREADER_STATE_READKEY;
			}
		}		
		fclose(fp);
		if (state == INIREADER_STATE_READVAL) {
			if (!reallocstr(&value, cp, &valcap)) {
				value[cp] = '\0';
			} else {
				value[MAXSTRINGSIZE - 1] = '\0';
			}
			if (self->dos_to_utf8 && psy_strlen(value)) {
				char* out;
								
				out = psy_dos_to_utf8(value, out);				
				psy_signal_emit(&self->signal_read, self, 2, key, out);
				free(out);
				out = NULL;
			} else {
				psy_signal_emit(&self->signal_read, self, 2, key, value);
			}
		}
		free(key);
		free(value);
		free(self->section);
		self->section = NULL;
		return PSY_OK;
	}	
	return PSY_ERRFILE;
}


int reallocstr(char** str, size_t size, size_t* cap)
{
	int err = 0;

	if (cap && *cap < size) {
		*cap = psy_min(size + 256, MAXSTRINGSIZE);
		*str = realloc(*str, *cap);
		if (*cap == MAXSTRINGSIZE || *str == 0) {			
			err = 1;			
		}
	}
	return err;
}
