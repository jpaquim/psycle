/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uitextinput.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_textinput_ondestroy(psy_ui_TextInput*);
static void psy_ui_textinput_onpreferredsize(psy_ui_TextInput*, psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_textinput_onkeydown(psy_ui_TextInput*, psy_ui_KeyboardEvent*);
static void psy_ui_textinput_onfocus(psy_ui_TextInput*);
static void psy_ui_textinput_onfocuslost(psy_ui_TextInput*);
static void psy_ui_textinput_onmousehook(psy_ui_TextInput*, psy_ui_App* sender,
	psy_ui_MouseEvent*);
static void psy_ui_textinput_ondraw(psy_ui_TextInput*, psy_ui_Graphics*);
static void psy_ui_textinput_drawcursor(psy_ui_TextInput*, psy_ui_Graphics*);
static void psy_ui_textinput_onmousedown(psy_ui_TextInput*, psy_ui_MouseEvent*);
static void insertchar(psy_ui_TextInput*, char c);
static void deletechar(psy_ui_TextInput*);
static void removechar(psy_ui_TextInput*);
static char_dyn_t* lefttext(psy_ui_TextInput*, uintptr_t split);
static char_dyn_t* righttext(psy_ui_TextInput*, uintptr_t split);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_TextInput* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			psy_ui_textinput_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_textinput_ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_textinput_onpreferredsize;
		vtable.onfocus =
			(psy_ui_fp_component_event)
			psy_ui_textinput_onfocus;
		vtable.onfocuslost =
			(psy_ui_fp_component_event)
			psy_ui_textinput_onfocuslost;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			psy_ui_textinput_onkeydown;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_textinput_onmousedown;
		vtable_initialized = TRUE;
	}	
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_textinput_init(psy_ui_TextInput* self, psy_ui_Component* parent)
{ 
	psy_ui_component_init(psy_ui_textinput_base(self), parent, NULL);
	vtable_init(self);	
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);	
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_EDIT, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_component_setstyletype_focus(&self->component, psy_ui_STYLE_EDIT_FOCUS);
	self->charnumber = 0;
	self->linenumber = 1;
	self->isinputfield = FALSE;
	self->preventedit = TRUE;	
	self->text = psy_strdup("textinput");
	self->cp = 0;
}

void psy_ui_textinput_ondestroy(psy_ui_TextInput* self)
{
	psy_signal_dispose(&self->signal_change);
	psy_signal_dispose(&self->signal_accept);
	psy_signal_dispose(&self->signal_reject);
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_textinput_onmousehook);
	free(self->text);
}

void psy_ui_textinput_enableinputfield(psy_ui_TextInput* self)
{
	self->isinputfield = TRUE;
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_textinput_onmousehook);
	psy_signal_connect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_textinput_onmousehook);
}

void psy_ui_textinput_settext(psy_ui_TextInput* self, const char* text)
{	
	psy_strreset(&self->text, text);
	psy_ui_component_invalidate(&self->component);
}

 const char* psy_ui_textinput_text(const psy_ui_TextInput* self)
{	
	return self->text;
}

void psy_ui_textinput_setcharnumber(psy_ui_TextInput* self, int number)
{
	self->charnumber = number;
}

void psy_ui_textinput_setlinenumber(psy_ui_TextInput* self, int number)
{
	self->linenumber = number;
}

void psy_ui_textinput_onpreferredsize(psy_ui_TextInput* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{			
	if (rv) {		
		const psy_ui_TextMetric* tm;
		
		tm = psy_ui_component_textmetric(&self->component);			
		if (self->charnumber == 0) {
			psy_ui_Size size;
			
			size = psy_ui_component_textsize(&self->component,
				psy_ui_textinput_text(self));
			rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width,
				psy_ui_component_textmetric(&self->component), NULL) + 2);
			rv->height = psy_ui_value_make_px((int)(tm->tmHeight * self->linenumber));
		} else {				
			rv->width = psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber + 2);
			rv->height = psy_ui_value_make_px((int)(tm->tmHeight * self->linenumber));
		}
	}
}

void psy_ui_textinput_enableedit(psy_ui_TextInput* self)
{
	psy_ui_component_enableinput(&self->component, psy_ui_NONRECURSIVE);
}

void psy_ui_textinput_preventedit(psy_ui_TextInput* self)
{
	psy_ui_component_preventinput(&self->component, psy_ui_NONRECURSIVE);
}

void psy_ui_textinput_setsel(psy_ui_TextInput* self, intptr_t cpmin, intptr_t cpmax)
{

}

/* signal_accept event handling */
void psy_ui_textinput_onkeydown(psy_ui_TextInput* self, psy_ui_KeyboardEvent* ev)
{
	assert(self);
	
	switch (psy_ui_keyboardevent_keycode(ev)) {
	case psy_ui_KEY_ESCAPE:
		if (self->isinputfield) {
			psy_ui_keyboardevent_prevent_default(ev);
			self->preventedit = TRUE;
			psy_ui_app_stopmousehook(psy_ui_app());
			psy_signal_emit(&self->signal_reject, self, 0);
			psy_ui_keyboardevent_prevent_default(ev);
		}
		break;
	case psy_ui_KEY_RETURN:
		if (self->isinputfield) {
			self->preventedit = TRUE;
			psy_ui_app_stopmousehook(psy_ui_app());
			psy_signal_emit(&self->signal_accept, self, 0);
			psy_ui_keyboardevent_prevent_default(ev);
		}
		break;			
	case psy_ui_KEY_LEFT:
		if (self->cp > 0) {
			--self->cp;
		}
		break;
	case psy_ui_KEY_RIGHT:
		if (self->cp < psy_strlen(self->text)) {
			++self->cp;
		}
		break;
	case psy_ui_KEY_UP:
		break;
	case psy_ui_KEY_DOWN:
		break;
	case psy_ui_KEY_HOME:
		self->cp = 0;
		break;
	case psy_ui_KEY_END:
		self->cp = psy_strlen(self->text);
		break;
	case psy_ui_KEY_BACK:
		deletechar(self);
		break;
	case psy_ui_KEY_DELETE:
		removechar(self);
		break;
	case psy_ui_KEY_SPACE:
		insertchar(self, ' ');
		break;
	case psy_ui_KEY_SHIFT:
	case psy_ui_KEY_CONTROL:
	case psy_ui_KEY_MENU:
		break;
	default:
		if (psy_ui_keyboardevent_shiftkey(ev)) {
			insertchar(self, psy_ui_keyboardevent_keycode(ev));
		} else if (psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_DIGIT0 &&
				psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DIGIT9) {
			insertchar(self, psy_ui_keyboardevent_keycode(ev));
		} else {
			insertchar(self, psy_ui_keyboardevent_keycode(ev) - 'A' + 'a');
		}						
		break;
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);	
}

void insertchar(psy_ui_TextInput* self, char c)
{
	char insert[2];
	char* left;
	char* right;

	insert[0] = c;
	insert[1] = '\0';
	left = lefttext(self, self->cp + 1);
	right = righttext(self, self->cp);
	left = psy_strcat_realloc(left, insert);
	if (psy_strlen(right) > 0) {
		left = psy_strcat_realloc(left, right);
	}
	psy_strreset(&self->text, left);
	free(left);
	left = NULL;
	free(right);
	right = NULL;
	++self->cp;
}

void deletechar(psy_ui_TextInput* self)
{
	char* left;
	char* right;

	left = lefttext(self, self->cp);
	right = righttext(self, self->cp);
	if (psy_strlen(right) > 0) {
		left = psy_strcat_realloc(left, right);
	}
	psy_strreset(&self->text, left);
	free(left);
	left = NULL;
	free(right);
	right = NULL;
	if (self->cp > 0) {
		--self->cp;
	}
}

void removechar(psy_ui_TextInput* self)
{
	char* left;
	char* right;

	left = lefttext(self, self->cp + 1);
	right = righttext(self, self->cp + 1);
	if (psy_strlen(right) > 0) {
		left = psy_strcat_realloc(left, right);
	}
	psy_strreset(&self->text, left);
	free(left);
	left = NULL;
	free(right);
	right = NULL;
}

char_dyn_t* lefttext(psy_ui_TextInput* self, uintptr_t split)
{
	char* rv;

	rv = malloc(split + 1);
	if (split > 0) {
		psy_snprintf(rv, split, "%s", self->text);
	} else {
		rv[0] = '\0';
	}
	return rv;
}

char_dyn_t* righttext(psy_ui_TextInput* self, uintptr_t split)
{
	char* rv;

	if (psy_strlen(self->text) > split) {
		uintptr_t num;

		num = psy_strlen(self->text) - split;
		rv = malloc(num + 1);
		psy_snprintf(rv, num + 1, "%s", self->text + split);
	} else {
		rv = NULL;
	}
	return rv;
}

void psy_ui_textinput_onfocus(psy_ui_TextInput* self)
{	
	super_vtable.onfocus(&self->component);
	if (self->isinputfield) {
		self->preventedit = FALSE;
		psy_ui_app_startmousehook(psy_ui_app());
	}
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_textinput_onfocuslost(psy_ui_TextInput* self)
{
	assert(self);

	super_vtable.onfocuslost(&self->component);
	if (self->isinputfield && !self->preventedit) {
		self->preventedit = TRUE;
		psy_ui_app_stopmousehook(psy_ui_app());
		psy_signal_emit(&self->signal_accept, self, 0);		
	}
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_textinput_onmousehook(psy_ui_TextInput* self, psy_ui_App* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->isinputfield) {
		if (psy_ui_component_visible(&self->component) && !self->preventedit) {
			psy_ui_RealRectangle position;

			position = psy_ui_component_screenposition(&self->component);			
			if (!psy_ui_realrectangle_intersect(&position, psy_ui_mouseevent_pt(ev))) {
				self->preventedit = TRUE;
				psy_ui_app_stopmousehook(psy_ui_app());
				psy_signal_emit(&self->signal_accept, self, 0);
				psy_ui_component_invalidate(&self->component);
			}
		}
	}
}

void psy_ui_textinput_ondraw(psy_ui_TextInput* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	psy_ui_RealRectangle r;
	double y;

	tm = psy_ui_component_textmetric(psy_ui_textinput_base(self));
	size = psy_ui_component_size_px(psy_ui_textinput_base(self));
	y = (size.height - tm->tmHeight) / 2.0;
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, y),
		psy_ui_realsize_make(size.width, tm->tmHeight));
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0.0, y),
		psy_ui_ETO_CLIPPED, r, self->text, strlen(self->text));
	psy_ui_textinput_drawcursor(self, g);
}

void psy_ui_textinput_drawcursor(psy_ui_TextInput* self, psy_ui_Graphics* g)
{
	if (psy_ui_component_hasfocus(&self->component)) {
		psy_ui_Size textsize;
		double x;
		double y;
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;

		tm = psy_ui_component_textmetric(psy_ui_textinput_base(self));
		size = psy_ui_component_size_px(psy_ui_textinput_base(self));
		textsize = psy_ui_textsize(g, self->text, self->cp);
		x = psy_ui_value_px(&textsize.width, tm, NULL);
		y = (size.height - tm->tmHeight) / 2.0;
		psy_ui_drawline(g,
			psy_ui_realpoint_make(x, y),
			psy_ui_realpoint_make(x, y + tm->tmHeight));
	}
}

void psy_ui_textinput_onmousedown(psy_ui_TextInput* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(&self->component);
	psy_ui_mouseevent_stop_propagation(ev);
}
