/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uitextformat.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"


/* implementation */
void psy_ui_textformat_init(psy_ui_TextFormat* self,
	const psy_ui_TextMetric* tm, bool wrap)
{
	assert(self);
	
	self->tm = tm;
	self->wrap = wrap;	
}

psy_List* psy_ui_textformat_lines(psy_ui_TextFormat* self, const char* text,
	double width)
{
	psy_List* rv;
	psy_List* eols;	

	rv = NULL;
	if (self->wrap) {
		psy_List* p;
		uintptr_t cp;		

		eols = psy_ui_textformat_eols(self, text);
		psy_list_append(&eols, (void*)psy_strlen(text));
		cp = 0;
		for (p = eols; p != NULL; p = p->next) {
			uintptr_t eol;
			uintptr_t numout;
			psy_List* wraps;
			psy_List* q;

			eol = (uintptr_t)p->entry;
			numout = eol - cp;
			if (self->wrap) {
				uintptr_t wp;

				wraps = psy_ui_textformat_wraps(self, text + cp, numout, width);
				wp = 0;
				for (q = wraps; q != NULL; q = q->next) {
					uintptr_t wrap;

					wrap = (uintptr_t)q->entry;
					numout = wrap - wp;
					psy_list_append(&rv, (void*)(cp + wp + numout));
					wp = wrap + 1;
				}
				psy_list_free(wraps);
			} else {
				psy_list_append(&rv, (void*)(cp + numout));
			}
			cp = eol;
		}
		psy_list_free(eols);
		eols = NULL;		
	}
	if (!rv) {
		psy_list_append(&rv, (void*)psy_strlen(text));
	}
	return rv;
}

psy_List* psy_ui_textformat_eols(psy_ui_TextFormat* self, const char* text)
{
	psy_List* rv;	
	uintptr_t num;
	uintptr_t cp;
		
	rv = NULL;	
	num = psy_strlen(text);
	if (num == 0) {
		return NULL;
	}
	for (cp = 0; cp < num; ++cp) {
		if (text[cp] == '\n') {
			psy_list_append(&rv, (void*)cp);
		}
	}		
	return rv;
}

psy_List* psy_ui_textformat_wraps(psy_ui_TextFormat* self, const char* text,
	uintptr_t num, double width)
{
	psy_List* rv;	
	uintptr_t numavgchars;
	uintptr_t cp;	

	rv = NULL;	
	numavgchars = (uintptr_t)(width / (self->tm->tmAveCharWidth * 1.4));
	if (numavgchars == 0) {
		return rv;
	}
	cp = 0;		
	while (cp < num) {
		uintptr_t linestart;

		linestart = cp;
		cp += numavgchars;
		if (cp > num - linestart) {
			psy_list_append(&rv, (void*)num);
			return rv;
		}		
		while (cp > linestart + 1) {
			if (text[cp] == ' ') {				
				break;
			}
			--cp;
		}
		if (cp > linestart + 1) {
			psy_list_append(&rv, (void*)cp);
		} else {
			cp += numavgchars;
			cp = psy_min(num, cp);
			psy_list_append(&rv, (void*)cp);
		}				
	}
	return rv;
}
