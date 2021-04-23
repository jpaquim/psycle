// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

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
	PROPERTIESIO_STATE_READCOMMENT = 5,
} PropertiesIOState;

static int reallocstr(char** str, size_t size, size_t* cap);
static int OnSaveIniEnum(FILE* file, psy_Property*, uintptr_t level);

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


int propertiesio_load(psy_Property* self, const psy_Path* path, int allowappend)
{
	FILE* fp;
				
	fp = fopen(psy_path_full(path), "rb");
	if (fp) {
		int c;
		uintptr_t cp = 0;
		PropertiesIOState state;
		char* key = 0;
		size_t keycap = 0;
		char* value = 0;
		size_t valcap = 0;
		psy_Property* curr;
		psy_Property* choice;
		int ischoice = 0;

		curr = self;
		choice = 0;
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
				if (c == ';') {
					state = PROPERTIESIO_STATE_READCOMMENT;
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
			} else
			if (state == PROPERTIESIO_STATE_READCOMMENT) {
				if (c == '\r') {
					state = PROPERTIESIO_STATE_READKEY;
				} else
					if (c == '\n') {
						state = PROPERTIESIO_STATE_READKEY;
					}
			}			
			if (state == PROPERTIESIO_STATE_ADDPROPERTY) {
				bool append;
				psy_Property* p = psy_property_at(curr, key, PSY_PROPERTY_TYPE_NONE);

				append = allowappend || (curr && curr->item.allowappend);
				if (p) {
					switch (p->item.typ) {
					case PSY_PROPERTY_TYPE_ROOT:
						break;
					case PSY_PROPERTY_TYPE_INTEGER:
						psy_property_set_int(curr, key, atoi(value));
						break;
					case PSY_PROPERTY_TYPE_BOOL:
						psy_property_set_bool(curr, key, atoi(value));
						break;
					case PSY_PROPERTY_TYPE_CHOICE:						
						choice = psy_property_set_choice(curr, key, atoi(value));
						ischoice = 1;
						break;
					case PSY_PROPERTY_TYPE_STRING:
						psy_property_set_str(curr, key, value);
						break;
					case PSY_PROPERTY_TYPE_FONT:
						psy_property_set_font(curr, key, value);
						break;
					default:
						break;
					}
				} else if (append) {
					int intval;
					char *stopstring;

					intval = strtol(value, &stopstring, 10);
					if (errno == ERANGE || strcmp(stopstring, "") != 0) {
						psy_property_append_str(curr, key, value);	
					} else {
						psy_property_append_int(curr, key, intval, 0, 0);											
					}
				}
				cp = 0;
				state = PROPERTIESIO_STATE_READKEY;
			} else
			if (state == PROPERTIESIO_STATE_ADDSECTION) {
				psy_Property* p;				
				psy_Property* prev = 0;				
				
				p = psy_property_findsectionex(self, key, &prev);				
				if (p == self) {
					curr = self;
					ischoice = 0;
				} else				
				if (p && p->item.typ == PSY_PROPERTY_TYPE_SECTION) {
					ischoice = 0;
					curr = p;
				} else
				if (ischoice) {
					curr = choice;
				} else
				if ((strcmp(key, "root") != 0) && allowappend) {
					char_dyn_t* trimkey;

					ischoice = 0;
					trimkey = lastkey(key);
					curr = psy_property_append_section(prev, trimkey);
					free(trimkey);
				} else {
					ischoice = 0;
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
			if (psy_property_at(curr, key, PSY_PROPERTY_TYPE_NONE)) {
				psy_property_set_str(curr, key, value);
			}			
		}
		free(key);
		free(value);
		return PSY_OK;
	}	
	return PSY_ERRFILE;
}

static uintptr_t skip;
static uintptr_t skiplevel;
static uintptr_t choicelevel;
static char* lastsection;

int propertiesio_save(const psy_Property* self, const char* filename)
{
	FILE* fp;

	assert(self);

	fp = fopen(filename, "wb");
	if (fp) {
		skip = 0;
		skiplevel = 0;
		choicelevel = 0;
		lastsection = 0;
		if (psy_strlen(psy_property_comment(self)) > 0) {			
			fwrite("; ", sizeof(char), 2, fp);
			fwrite(self->item.comment, sizeof(char),
				strlen(self->item.comment), fp);
			fwrite("\n", sizeof(char), 1, fp);			
		}
		psy_property_enumerate((psy_Property*)self, fp, (psy_PropertyCallback)
			OnSaveIniEnum);
		free(lastsection);
		fclose(fp);
		return PSY_OK;
	}	
	return PSY_ERRFILE;
}

int OnSaveIniEnum(FILE* fp, psy_Property* property, uintptr_t level)
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
	if (choicelevel != 0 && choicelevel != level) {
		char_dyn_t* sections;
		
		sections = lastsection;
		fwrite("\n", sizeof(char), 1, fp);
		fwrite("[", sizeof(char), 1, fp);
		if (sections[0] != '\0') {
			fwrite(sections, sizeof(char), strlen(sections), fp);
		}
		fwrite("]", sizeof(char), 1, fp);
		fwrite("\n", sizeof(char), 1, fp);
		choicelevel = 0;
	}
	if (property->item.key) {
		char text[40];
			
		if (property->item.comment) {
			fwrite("; ", sizeof(char), 2, fp);
			fwrite(property->item.comment, sizeof(char),
				strlen(property->item.comment), fp);
			fwrite("\n", sizeof(char), 1, fp);
		}
		if (property->item.typ == PSY_PROPERTY_TYPE_ROOT) {
			fwrite("[root]", sizeof(char), 6, fp);			
		} else
		if (property->item.typ == PSY_PROPERTY_TYPE_SECTION) {
			char_dyn_t* sections;

			sections = psy_property_sections(property);
			psy_strreset(&lastsection, sections);			
			fwrite("[", sizeof(char), 1, fp);
			if (sections[0] != '\0') {
				fwrite(sections, sizeof(char), strlen(sections), fp);
			}			
			fwrite("]", sizeof(char), 1, fp);
			free(sections);
		} else		
		if (property->item.typ != PSY_PROPERTY_TYPE_ACTION) {
			if (strcmp(psy_property_key(property), "favorite") == 0) {
				property = property;
				if (psy_property_item_int(property) > 0) {
					property = property;
				}
			}
			fwrite(psy_property_key(property), sizeof(char),
				strlen(psy_property_key(property)), fp);
			fwrite("=", sizeof(char), 1, fp);
			switch (property->item.typ) {				
				case PSY_PROPERTY_TYPE_INTEGER:
					psy_snprintf(text, 40, "%d", psy_property_item_int(property));
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);					
				break;
				case PSY_PROPERTY_TYPE_BOOL:
					psy_snprintf(text, 40, "%d", (int)psy_property_item_bool(
						property));
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);
				break;				
				case PSY_PROPERTY_TYPE_CHOICE: {
					char_dyn_t* sections;

					choicelevel = level + 1;
					psy_snprintf(text, 40, "%d", psy_property_item_int(
						property));
					text[39] = '\0';
					fwrite(text, sizeof(char), strlen(text), fp);

					sections = psy_property_sections(property);
					fwrite("\n", sizeof(char), 1, fp);
					fwrite("[", sizeof(char), 1, fp);
					if (sections[0] != '\0') {
						fwrite(sections, sizeof(char), strlen(sections), fp);
					}
					fwrite(".", sizeof(char), 1, fp);
					fwrite(property->item.key, sizeof(char),
						strlen(property->item.key), fp);
					fwrite("]", sizeof(char), 1, fp);
					free(sections);
					break; }
				case PSY_PROPERTY_TYPE_STRING:
				case PSY_PROPERTY_TYPE_FONT:
					fwrite(psy_property_item_str(property), sizeof(char),
						strlen(psy_property_item_str(property)), fp);
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
		*cap = psy_min(size + 256, MAXSTRINGSIZE);
		*str = realloc(*str, *cap);
		if (*cap == MAXSTRINGSIZE || *str == 0) {			
			err = 1;			
		}
	}
	return err;
}
