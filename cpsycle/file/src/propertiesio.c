// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "propertiesio.h"
#include <malloc.h>
#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#define MAXSTRINGSIZE 4096

static int reallocstr(char** str, size_t size, size_t* cap);
static int OnSaveIniEnum(FILE* file, Properties*, int level);

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
		size_t keycap = 0;
		char* value = 0;
		size_t valcap = 0;
		
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
	FILE* fp;
	fp = fopen(path, "wb");
	if (fp) {
		properties_enumerate(self, fp, (PropertiesCallback)OnSaveIniEnum);
		fclose(fp);
	}
}

int OnSaveIniEnum(FILE* fp, Properties* property, int level)
{
	if (property->item.key) {
		char text[40];
		char sections[MAXSTRINGSIZE];
		
		if (property->item.typ == PROPERTY_TYP_ROOT) {
			fwrite("[root]", sizeof(char), 6, fp);			
		} else
		if (property->item.typ == PROPERTY_TYP_SECTION) {
			properties_sections(property, sections);
			fwrite("[", sizeof(char), 1, fp);
			if (sections[0] != '\0') {
				fwrite(sections, sizeof(char), strlen(sections), fp);
			}			
			fwrite("]", sizeof(char), 1, fp);
		} else 
		if (property->item.typ != PROPERTY_TYP_ACTION) {
			fwrite(properties_key(property), sizeof(char),
				strlen(properties_key(property)), fp);
			fwrite("=", sizeof(char), 1, fp);
			switch (property->item.typ) {				
				case PROPERTY_TYP_INTEGER:
					_snprintf(text, 40, "%d", properties_value(property));
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);					
				break;
				case PROPERTY_TYP_BOOL:
					_snprintf(text, 40, "%d", property->item.value.i);
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);
				break;
				case PROPERTY_TYP_CHOICE:
					_snprintf(text, 40, "%d", property->item.value.i);
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);
				break;
				case PROPERTY_TYP_STRING:
					fwrite(properties_valuestring(property), sizeof(char),
						strlen(properties_valuestring(property)), fp);					
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
