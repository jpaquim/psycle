/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "propertiesio.h"
/* local */
#include "inireader.h"
/* std */
#include <malloc.h>
#include <errno.h>
/* platform */
#include "../../detail/portable.h"


/* psy_PropertyReader */

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

static int _httoi(const char* value)
{
	return (int)strtol(value, 0, 16);
}


/* prototypes */
static void psy_propertyreader_onreadkey(psy_PropertyReader*,
	psy_IniReader* sender, const char* key, const char* valu);
static void psy_propertyreader_onreadsection(psy_PropertyReader*,
	psy_IniReader* sender, const char* key);
static bool psy_propertyreader_extract_type(psy_PropertyReader*,
	const char* text, const char** typestr, const char** valstr);

/* implementation */
void psy_propertyreader_init(psy_PropertyReader* self, psy_Property* root,
	const char* path)
{		
	self->root = root;
	self->path = psy_strdup(path);
	self->curr = self->choice = NULL;
	self->allowappend = FALSE;
	self->cpp_comment = FALSE;
	self->parse_types = FALSE;
	self->ignore_sections = FALSE;
	self->skip_double_quotes = FALSE;
	self->ischoice = 0;
}

void psy_propertyreader_dispose(psy_PropertyReader* self)
{	
	free(self->path);
	self->path = NULL;	
}

int psy_propertyreader_load(psy_PropertyReader* self)
{
	psy_IniReader inireader;
	int rv;

	assert(self);
			
	self->curr = self->root;
	self->choice = NULL;
	self->ischoice = 0;	
	psy_inireader_init(&inireader);
	inireader.cpp_comment = self->cpp_comment;
	psy_signal_connect(&inireader.signal_read, self,
		psy_propertyreader_onreadkey);
	psy_signal_connect(&inireader.signal_section, self,
		psy_propertyreader_onreadsection);
	rv = inireader_load(&inireader, self->path);
	psy_inireader_dispose(&inireader);
	return rv;
}

void psy_propertyreader_onreadkey(psy_PropertyReader* self, psy_IniReader* sender,
	const char* key, const char* value)
{
	bool append;
	psy_Property* p = psy_property_at(self->curr, key, PSY_PROPERTY_TYPE_NONE);

	append = self->allowappend || (self->curr && self->curr->item.allowappend);
	if (p) {
		const char* valstr;
		char* typestr;

		typestr = NULL;
		if (self->parse_types) {			
			psy_propertyreader_extract_type(self, value, &typestr, &valstr);
		} else {
			valstr = value;			
		}
		switch (p->item.typ) {
		case PSY_PROPERTY_TYPE_ROOT:
			break;
		case PSY_PROPERTY_TYPE_INTEGER:
			if (typestr && (strcmp(typestr, "dword:") == 0 || strcmp(typestr, "hex:") == 0)) {
				psy_property_set_int(self->curr, key, _httoi(valstr));
			} else {
				psy_property_set_int(self->curr, key, atoi(valstr));
			}
			break;
		case PSY_PROPERTY_TYPE_BOOL:
			psy_property_set_bool(self->curr, key, atoi(valstr));
			break;
		case PSY_PROPERTY_TYPE_CHOICE:
			self->choice = psy_property_set_choice(self->curr, key, atoi(valstr));
			self->ischoice = 1;
			break;
		case PSY_PROPERTY_TYPE_STRING:
			if (self->skip_double_quotes && value && value[0] == '\"') {
				char* temp;

				temp = psy_strdup(value + 1);
				if (psy_strlen(temp) > 0) {
					if (temp[psy_strlen(temp) - 1] == '\"') {
						temp[psy_strlen(temp) - 1] = '\0';
						psy_property_set_str(self->curr, key, temp);
					} else {
						psy_property_set_str(self->curr, key, value);
					}
					free(temp);
					temp = NULL;
				} else {
					psy_property_set_str(self->curr, key, "");
				}
			} else {
				psy_property_set_str(self->curr, key, value);
			}
			break;
		case PSY_PROPERTY_TYPE_FONT:
			psy_property_set_font(self->curr, key, value);
			break;
		default:
			break;
		}
	} else if (append) {
		int intval;
		char* stopstring;

		intval = strtol(value, &stopstring, 10);
		if (errno == ERANGE || strcmp(stopstring, "") != 0) {
			psy_property_append_str(self->curr, key, value);
		} else {
			psy_property_append_int(self->curr, key, intval, 0, 0);
		}
	}		
}

bool psy_propertyreader_extract_type(psy_PropertyReader* self,
	const char* text, const char** typestr, const char** valstr)
{
	char* p;
	static const char* dword = "dword:";
	static const char* hex = "hex:";

	p = strstr(text, dword);
	if (p == text) {
		*valstr = p + strlen(dword);
		*typestr = dword;
		return TRUE;
	}
	p = strstr(text, hex);
	if (p == text) {
		*valstr = p + strlen(dword);
		*typestr = dword;
		return TRUE;
	}
	*valstr = text;
	*typestr = NULL;
	return FALSE;
}

void psy_propertyreader_onreadsection(psy_PropertyReader* self, psy_IniReader* sender,
	const char* key)
{
	if (!self->ignore_sections) {
		psy_Property* p;
		psy_Property* prev = 0;

		p = psy_property_findsectionex(self->root, key, &prev);
		if (p == self->root) {
			self->curr = self->root;
			self->ischoice = 0;
		} else if (p && p->item.typ == PSY_PROPERTY_TYPE_SECTION) {
			self->ischoice = 0;
			self->curr = p;
		} else if (self->ischoice) {
			self->curr = self->choice;
		} else if ((strcmp(key, "root") != 0) && self->allowappend) {
			char_dyn_t* trimkey;

			self->ischoice = 0;
			trimkey = lastkey(key);
			self->curr = psy_property_append_section(prev, trimkey);
			free(trimkey);
		} else {
			self->ischoice = 0;
			self->curr = self->root;
		}
	}
}

/* Property save */

/* prototypes */
static int psy_propertywriter_save_enum(psy_PropertyWriter*, psy_Property*, uintptr_t level);

/* implementation */
void psy_propertywriter_init(psy_PropertyWriter* self, const psy_Property* root, const char* path)
{
	self->root = root;
	self->path = psy_strdup(path);
	self->skip = 0;
	self->skiplevel = 0;
	self->choicelevel = 0;
	self->lastsection = 0;
	self->fp = NULL;
}

void psy_propertywriter_dispose(psy_PropertyWriter* self)
{
	free(self->path);
	self->path = NULL;
}

int psy_propertywriter_save(psy_PropertyWriter* self)
{	
	assert(self);

	self->fp = fopen(self->path, "wb");
	if (self->fp) {
		self->skip = 0;
		self->skiplevel = 0;
		self->choicelevel = 0;
		self->lastsection = 0;
		if (psy_strlen(psy_property_comment(self->root)) > 0) {
			fwrite("; ", sizeof(char), 2, self->fp);
			fwrite(self->root->item.comment, sizeof(char),
				psy_strlen(self->root->item.comment), self->fp);
			fwrite("\n", sizeof(char), 1, self->fp);
		}
		psy_property_enumerate((psy_Property*)self->root, self,
			(psy_PropertyCallback)psy_propertywriter_save_enum);
		free(self->lastsection);
		fclose(self->fp);
		return PSY_OK;
	}
	return PSY_ERRFILE;
}

int psy_propertywriter_save_enum(psy_PropertyWriter* self, psy_Property* property, uintptr_t level)
{
	if (self->skip && level > self->skiplevel) {
		return 1;
	}
	self->skip = self->skiplevel = 0;
	if (!property->item.save) {
		self->skip = 1;
		self->skiplevel = level;
		return 1;
	}
	if (self->choicelevel != 0 && self->choicelevel != level) {
		char_dyn_t* sections;
		
		sections = self->lastsection;
		fwrite("\n", sizeof(char), 1, self->fp);
		fwrite("[", sizeof(char), 1, self->fp);
		if (sections[0] != '\0') {
			fwrite(sections, sizeof(char), psy_strlen(sections), self->fp);
		}
		fwrite("]", sizeof(char), 1, self->fp);
		fwrite("\n", sizeof(char), 1, self->fp);
		self->choicelevel = 0;
	}
	if (property->item.key) {
		char text[40];
			
		if (property->item.comment) {
			fwrite("; ", sizeof(char), 2, self->fp);
			fwrite(property->item.comment, sizeof(char),
				psy_strlen(property->item.comment), self->fp);
			fwrite("\n", sizeof(char), 1, self->fp);
		}
		if (property->item.typ == PSY_PROPERTY_TYPE_ROOT) {
			fwrite("[root]", sizeof(char), 6, self->fp);
		} else
		if (property->item.typ == PSY_PROPERTY_TYPE_SECTION) {
			char_dyn_t* sections;

			sections = psy_property_sections(property);
			psy_strreset(&self->lastsection, sections);
			fwrite("[", sizeof(char), 1, self->fp);
			if (sections[0] != '\0') {
				fwrite(sections, sizeof(char), psy_strlen(sections), self->fp);
			}			
			fwrite("]", sizeof(char), 1, self->fp);
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
				psy_strlen(psy_property_key(property)), self->fp);
			fwrite("=", sizeof(char), 1, self->fp);
			switch (property->item.typ) {				
				case PSY_PROPERTY_TYPE_INTEGER:
					psy_snprintf(text, 40, "%d", psy_property_item_int(property));
					text[39] = '\0';
					fwrite(text, sizeof(char), psy_strlen(text), self->fp);
				break;
				case PSY_PROPERTY_TYPE_BOOL:
					psy_snprintf(text, 40, "%d", (int)psy_property_item_bool(
						property));
					text[39] = '\0';
					fwrite(text, sizeof(char), psy_strlen(text), self->fp);
				break;				
				case PSY_PROPERTY_TYPE_CHOICE: {
					char_dyn_t* sections;

					self->choicelevel = level + 1;
					psy_snprintf(text, 40, "%d", psy_property_item_int(
						property));
					text[39] = '\0';
					fwrite(text, sizeof(char), psy_strlen(text), self->fp);

					sections = psy_property_sections(property);
					fwrite("\n", sizeof(char), 1, self->fp);
					fwrite("[", sizeof(char), 1, self->fp);
					if (sections[0] != '\0') {
						fwrite(sections, sizeof(char), psy_strlen(sections), self->fp);
					}
					fwrite(".", sizeof(char), 1, self->fp);
					fwrite(property->item.key, sizeof(char),
						psy_strlen(property->item.key), self->fp);
					fwrite("]", sizeof(char), 1, self->fp);
					free(sections);
					break; }
				case PSY_PROPERTY_TYPE_STRING:
				case PSY_PROPERTY_TYPE_FONT:
					fwrite(psy_property_item_str(property), sizeof(char),
						psy_strlen(psy_property_item_str(property)), self->fp);
				break;						
				default:
				break;
			}
		}		
		fwrite("\n", sizeof(char),  1, self->fp);
	}
	return 1;
}
