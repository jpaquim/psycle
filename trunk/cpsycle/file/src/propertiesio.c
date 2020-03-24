// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "propertiesio.h"
#include <malloc.h>
#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "../../detail/portable.h"

#define MAXSTRINGSIZE 4096

typedef enum {
	PROPERTIESIO_STATE_READKEY = 0,
	PROPERTIESIO_STATE_READVAL = 1,
	PROPERTIESIO_STATE_ADDPROPERTY = 2,
	PROPERTIESIO_STATE_READSECTION = 3,
	PROPERTIESIO_STATE_ADDSECTION = 4,
} PropertiesIOState;

static int reallocstr(char** str, size_t size, size_t* cap);
static int OnSaveIniEnum(FILE* file, psy_Properties*, int level);

int propertiesio_load(psy_Properties* self, const char* path, int allowappend)
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
		psy_Properties* curr;

		curr = self;		
		state = PROPERTIESIO_STATE_READKEY;
		reallocstr(&key, 256, &keycap);
		reallocstr(&value, 256, &valcap);				
		while ((c = fgetc(fp)) != EOF) {			
			if (state == PROPERTIESIO_STATE_READKEY) {
				if (c == '\r') {
					state = PROPERTIESIO_STATE_READKEY;
				} else
				if (c == '\n') {
					state = PROPERTIESIO_STATE_READKEY;
				} else
				if (c == '[') {
					state = PROPERTIESIO_STATE_READSECTION;
					key[cp] = '\0';
					cp = 0;
				} else
				if (c == '=') {
					state = PROPERTIESIO_STATE_READVAL;
					key[cp] = '\0';
					cp = 0;
				} else {
					if (!reallocstr(&key, cp, &keycap)) {
						key[cp] = c;
					}
					++cp;
				}
			} else
			if (state == PROPERTIESIO_STATE_READVAL) {
				if (c == '\n') {
					state = PROPERTIESIO_STATE_ADDPROPERTY;
					value[cp] = '\0';
					cp = 0;
				} else
				if (!reallocstr(&value, cp, &valcap)) {
					value[cp] = c;
				}
				++cp;
			} else 
			if (state == PROPERTIESIO_STATE_READSECTION) {
				if (c == ']') {
					state = PROPERTIESIO_STATE_ADDSECTION;
					key[cp] = '\0';
					cp = 0;
				} else
				if (!reallocstr(&key, cp, &keycap)) {
					key[cp] = c;
				}
				++cp;
			}
			if (state == PROPERTIESIO_STATE_ADDPROPERTY) {
				psy_Properties* p = psy_properties_read(curr, key);
				if (p) {					
					switch (p->item.typ) {
						case PSY_PROPERTY_TYP_ROOT:
						break;
						case PSY_PROPERTY_TYP_INTEGER:
							psy_properties_write_int(curr, key, atoi(value));
						break;
						case PSY_PROPERTY_TYP_BOOL:
							psy_properties_write_bool(curr, key, atoi(value));
						break;
						case PSY_PROPERTY_TYP_CHOICE:
							psy_properties_write_choice(curr, key, atoi(value));
						break;
						case PSY_PROPERTY_TYP_STRING:						
							psy_properties_write_string(curr, key, value);
						break;
						case PSY_PROPERTY_TYP_FONT:
							psy_properties_write_font(curr, key, value);
						break;
						default:
						break;
					}					
				} else
				if (allowappend) {
					int intval;
					char *stopstring;

					intval = strtol(value, &stopstring, 10);
					if (errno == ERANGE || strcmp(stopstring, "") != 0) {
						psy_properties_append_string(curr, key, value);	
					} else {
						psy_properties_append_int(curr, key, intval, 0, 0);											
					}
				}
				cp = 0;
				state = PROPERTIESIO_STATE_READKEY;
			} else
			if (state == PROPERTIESIO_STATE_ADDSECTION) {
				psy_Properties* p;				
				psy_Properties* prev = 0;
				p = psy_properties_findsectionex(self, key, &prev);
				if (p == self) {
					curr = self;
				} else
				if (p && p->item.typ == PSY_PROPERTY_TYP_SECTION) {
					curr = p;
				} else
				if (allowappend) {									
					curr = psy_properties_create_section(prev, key);
				} else {
					curr = self;
				}
				cp = 0;
				state = PROPERTIESIO_STATE_READKEY;
			}
		}		
		fclose(fp);
		if (state == PROPERTIESIO_STATE_READVAL) {
			if (!reallocstr(&value, cp, &valcap)) {
				value[cp] = '\0';
			} else {
				value[MAXSTRINGSIZE - 1] = '\0';
			}
			if (psy_properties_read(curr, key)) {
				psy_properties_write_string(curr, key, value);
			}			
		}
		free(key);
		free(value);
	}	
	return fp != 0;
}

static int skip;
static int skiplevel;

void propertiesio_save(psy_Properties* self, const char* path)
{
	FILE* fp;

	fp = fopen(path, "wb");
	if (fp) {
		skip = 0;
		skiplevel = 0;
		psy_properties_enumerate(self, fp, (psy_PropertiesCallback) OnSaveIniEnum);
		fclose(fp);
	}
}

int OnSaveIniEnum(FILE* fp, psy_Properties* property, int level)
{
	if (skip && level > skiplevel) {
		return 1;
	}
	skip = skiplevel = 0;	
	if (!property->item.save) {
		skip = 1;
		skiplevel = level;
		return 1;
	}
	if (property->item.key) {
		char text[40];
		char sections[MAXSTRINGSIZE];
		
		if (property->item.typ == PSY_PROPERTY_TYP_ROOT) {
			fwrite("[root]", sizeof(char), 6, fp);			
		} else
		if (property->item.typ == PSY_PROPERTY_TYP_SECTION) {
			psy_properties_sections(property, sections);
			fwrite("[", sizeof(char), 1, fp);
			if (sections[0] != '\0') {
				fwrite(sections, sizeof(char), strlen(sections), fp);
			}			
			fwrite("]", sizeof(char), 1, fp);
		} else 
		if (property->item.typ != PSY_PROPERTY_TYP_ACTION) {
			fwrite(psy_properties_key(property), sizeof(char),
				strlen(psy_properties_key(property)), fp);
			fwrite("=", sizeof(char), 1, fp);
			switch (property->item.typ) {				
				case PSY_PROPERTY_TYP_INTEGER:
					psy_snprintf(text, 40, "%d", psy_properties_value(property));
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);					
				break;
				case PSY_PROPERTY_TYP_BOOL:
					psy_snprintf(text, 40, "%d", property->item.value.i);
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);
				break;
				case PSY_PROPERTY_TYP_CHOICE:
					psy_snprintf(text, 40, "%d", property->item.value.i);
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);
				break;
				case PSY_PROPERTY_TYP_STRING:
				case PSY_PROPERTY_TYP_FONT:
					fwrite(psy_properties_valuestring(property), sizeof(char),
						strlen(psy_properties_valuestring(property)), fp);					
				break;						
				default:
				break;
			}
		}		
		fwrite("\n", sizeof(char),  1, fp);
	}
	return 1;
}

int reallocstr(char** str, size_t size, size_t* cap)
{
	int err = 0;

	if (cap && *cap < size) {
		*cap = min(size + 256, MAXSTRINGSIZE);		
		*str = realloc(*str, *cap);
		if (*cap == MAXSTRINGSIZE || *str == 0) {			
			err = 1;			
		}
	}
	return err;
}
