// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "propertiesio.h"
#include <malloc.h>
#include <string.h>
#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#define MAXSTRINGSIZE 4096

static int reallocstr(char** str, int size, int* cap);
static int OnSaveIniEnum(PsyFile* file, Properties*, int level);

int propertiesio_load(Properties* self, const char* path, int allowappend)
{
	FILE* fp;
	Properties* curr;

	curr = self;			
	fp = fopen(path, "rb");
	if (fp) {
		int c;
		int i;
		int state;
		char* key = 0;
		int keycap = 0;
		char* value = 0;
		int valcap = 0;
		
		i = 0;
		state = 0;

		reallocstr(&key, 256, &keycap);
		reallocstr(&value, 256, &valcap);		
				
		while ((c = fgetc(fp)) != EOF) {			
			if (state == 0) {
				if (c == '\r') {
					state = 0;
				} else
				if (c == '\n') {
					state = 0;
				} else
				if (c == '[') {
					state = 3;
					key[i] = '\0';
					i = 0;
				} else
				if (c == '=') {
					state = 1;
					key[i] = '\0';
					i = 0;
				} else {
					if (!reallocstr(&key, i, &keycap)) {
						key[i] = c;	
					}
					++i;
				}
			} else
			if (state == 1) {
				if (c == '\n') {
					state = 2;
					value[i] = '\0';
					i = 0;					
				} else
				if (!reallocstr(&value, i, &valcap)) {
					value[i] = c;					
				}
				++i;
			} else 
			if (state == 3) {
				if (c == ']') {
					state = 4;
					key[i] = '\0';
					i = 0;					
				} else
				if (!reallocstr(&key, i, &keycap)) {
					key[i] = c;					
				}
				++i;
			}
			if (state == 2) {
				Properties* p = properties_read(curr, key);
				if (p) {
					switch (p->item.typ) {
						case PROPERTY_TYP_ROOT:
						break;
						case PROPERTY_TYP_INTEGER:
							properties_write_int(curr, key, atoi(value));
						break;
						case PROPERTY_TYP_BOOL:
							properties_write_bool(curr, key, atoi(value));
						break;
						case PROPERTY_TYP_CHOICE:
							properties_write_choice(curr, key, atoi(value));
						break;
						case PROPERTY_TYP_STRING:
							properties_write_string(curr, key, value);
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
						properties_append_string(curr, key, value);	
					} else {
						properties_append_int(curr, key, intval, 0, 0);											
					}
				}
				i = 0;
				state = 0;
			} else
			if (state == 4) {
				Properties* p;				
				Properties* prev = 0;
				p = properties_findsectionex(self, key, &prev);
				if (p && p->children) {
					curr = p;
				} else
				if (allowappend) {										
					curr = properties_createsection(prev, key);
				} else {
					curr = self;
				}
				i = 0;
				state = 0;
			}
		}		
		fclose(fp);
		if (state == 1) {
			if (!reallocstr(&value, i, &valcap)) {
				value[i] = '\0';
			} else {
				value[MAXSTRINGSIZE-1] = '\0';
			}
			if (properties_read(curr, key)) {
				properties_write_string(curr, key, value);
			}			
		}
		free(key);
		free(value);
	}	
	return fp != 0;
}

void propertiesio_save(Properties* self, const char* path)
{
	PsyFile file;
	if (psyfile_create(&file, path, 1)) {
		properties_enumerate(self, &file, (PropertiesCallback)OnSaveIniEnum);
		psyfile_close(&file);
	}
}

int OnSaveIniEnum(PsyFile* file, Properties* property, int level)
{
	if (property->item.key) {
		char text[40];
		char sections[MAXSTRINGSIZE];
		
		if (property->item.typ == PROPERTY_TYP_ROOT) {
			psyfile_write(file, "[root]", 6);
		} else
		if (property->item.typ == PROPERTY_TYP_SECTION) {
			properties_sections(property, sections);
			psyfile_write(file, "[", 1);
			if (sections[0] != '\0') {
				psyfile_write(file, sections, strlen(sections));
			}
			psyfile_write(file, "]", 1);
		} else 
		if (property->item.typ != PROPERTY_TYP_ACTION) {			
			psyfile_write(file, property->item.key, strlen(property->item.key));
			psyfile_write(file, "=", strlen("="));
			switch (property->item.typ) {				
				case PROPERTY_TYP_INTEGER:
					_snprintf(text, 40, "%d", property->item.value.i);
					psyfile_write(file, text, strlen(text));
				break;
				case PROPERTY_TYP_BOOL:
					_snprintf(text, 40, "%d", property->item.value.i);
					psyfile_write(file, text, strlen(text));
				break;
				case PROPERTY_TYP_CHOICE:
					_snprintf(text, 40, "%d", property->item.value.i);
					psyfile_write(file, text, strlen(text));
				break;
				case PROPERTY_TYP_STRING:
					psyfile_write(file, property->item.value.s,
						strlen(property->item.value.s));
				break;						
				default:
				break;
			}
		}
		psyfile_write(file, "\n", strlen("\n"));
	}
	return 1;
}

int reallocstr(char** str, int size, int* cap)
{
	int err = 0;

	if (*cap < size) {
		*cap = min(size + 256, MAXSTRINGSIZE);		
		*str = realloc(*str, *cap);
		if (*cap == MAXSTRINGSIZE) {			
			err = 1;			
		}
	}
	return err;
}
