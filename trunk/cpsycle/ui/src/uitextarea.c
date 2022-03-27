/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uitextarea.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"
#include "uitextformat.h"
#include "uitextdraw.h"
/* platform */
#include "../../detail/portable.h"

/* psy_ui_TextAreaPane */
/* prototypes */
static void psy_ui_textareapane_ondestroy(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_onpreferredsize(psy_ui_TextAreaPane*, psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_textareapane_onkeydown(psy_ui_TextAreaPane*, psy_ui_KeyboardEvent*);
static void psy_ui_textareapane_onfocus(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_onfocuslost(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_onmousehook(psy_ui_TextAreaPane*, psy_ui_App* sender,
	psy_ui_MouseEvent*);
static void psy_ui_textareapane_ondraw(psy_ui_TextAreaPane*, psy_ui_Graphics*);
static void psy_ui_textareapane_onmousedown(psy_ui_TextAreaPane*, psy_ui_MouseEvent*);
static void insertchar(psy_ui_TextAreaPane*, char c);
static void deletechar(psy_ui_TextAreaPane*);
static void removechar(psy_ui_TextAreaPane*);
static char_dyn_t* lefttext(psy_ui_TextAreaPane*, uintptr_t split);
static char_dyn_t* righttext(psy_ui_TextAreaPane*, uintptr_t split);
static void psy_ui_textarea_oneditaccept(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textarea_oneditreject(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_TextAreaPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			psy_ui_textareapane_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_textareapane_ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_textareapane_onpreferredsize;
		vtable.onfocus =
			(psy_ui_fp_component_event)
			psy_ui_textareapane_onfocus;
		vtable.onfocuslost =
			(psy_ui_fp_component_event)
			psy_ui_textareapane_onfocuslost;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			psy_ui_textareapane_onkeydown;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_textareapane_onmousedown;
		vtable_initialized = TRUE;
	}	
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_textareapane_init(psy_ui_TextAreaPane* self, psy_ui_Component* parent)
{ 
	psy_ui_component_init(psy_ui_textareapane_base(self), parent, NULL);
	vtable_init(self);	
	self->charnumber = 120.0;
	self->linenumber = 1;
	self->isinputfield = FALSE;
	self->preventedit = TRUE;
	self->text = psy_strdup("textinput");
	self->cp = 0;
	psy_ui_textformat_init(&self->format);
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);	
	psy_ui_component_set_style_types(&self->component,
		psy_ui_STYLE_EDIT, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_component_set_style_type_focus(&self->component,
		psy_ui_STYLE_EDIT_FOCUS);
	psy_ui_component_set_scrollstep_height(
		psy_ui_textareapane_base(self),
		psy_ui_value_make_eh(1.0));
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);

}

void psy_ui_textareapane_ondestroy(psy_ui_TextAreaPane* self)
{
	psy_signal_dispose(&self->signal_change);
	psy_signal_dispose(&self->signal_accept);
	psy_signal_dispose(&self->signal_reject);
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_textareapane_onmousehook);
	free(self->text);
	psy_ui_textformat_dispose(&self->format);
}

void psy_ui_textareapane_enableinputfield(psy_ui_TextAreaPane* self)
{
	self->isinputfield = TRUE;
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_textareapane_onmousehook);
	psy_signal_connect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_textareapane_onmousehook);
}

void psy_ui_textareapane_settext(psy_ui_TextAreaPane* self, const char* text)
{	
	psy_strreset(&self->text, text);
	psy_ui_textformat_clear(&self->format);
	psy_ui_component_invalidate(&self->component);
}

 const char* psy_ui_textareapane_text(const psy_ui_TextAreaPane* self)
{	
	return self->text;
}

void psy_ui_textareapane_setcharnumber(psy_ui_TextAreaPane* self, double number)
{
	self->charnumber = number;
	psy_ui_textformat_clear(&self->format);
}

void psy_ui_textareapane_setlinenumber(psy_ui_TextAreaPane* self, int number)
{
	self->linenumber = number;
}

void psy_ui_textareapane_onpreferredsize(psy_ui_TextAreaPane* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{			
	const psy_ui_TextMetric* tm;	
	double width;
	
	tm = psy_ui_component_textmetric(psy_ui_textareapane_base(self));	
	width = (self->charnumber == 0 && limit)
		? psy_ui_value_px(&limit->width, tm, limit)
		: self->charnumber * tm->tmAveCharWidth;	
	psy_ui_textformat_update(&self->format, self->text, width, tm);
	*rv = psy_ui_size_make_px(width,
		psy_ui_textformat_numlines(&self->format) * (tm->tmHeight * 1.0));	
}

void psy_ui_textareapane_enableedit(psy_ui_TextAreaPane* self)
{
	psy_ui_component_enableinput(&self->component, psy_ui_NONRECURSIVE);
}

void psy_ui_textareapane_preventedit(psy_ui_TextAreaPane* self)
{
	psy_ui_component_preventinput(&self->component, psy_ui_NONRECURSIVE);
}

void psy_ui_textareapane_setsel(psy_ui_TextAreaPane* self, uintptr_t cpmin, uintptr_t cpmax)
{

}

/* signal_accept event handling */
void psy_ui_textareapane_onkeydown(psy_ui_TextAreaPane* self, psy_ui_KeyboardEvent* ev)
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
		} else if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
			insertchar(self, '\n');
			psy_ui_component_align(psy_ui_component_parent(&self->component));
		} else if (psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_DIGIT0
			&& psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DIGIT9) {
			insertchar(self, psy_ui_keyboardevent_keycode(ev));
		} else {
			insertchar(self, psy_ui_keyboardevent_keycode(ev) - 'A' + 'a');
		}						
		break;
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);	
}

void insertchar(psy_ui_TextAreaPane* self, char c)
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
	psy_ui_textformat_clear(&self->format);
}

void deletechar(psy_ui_TextAreaPane* self)
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
	psy_ui_textformat_clear(&self->format);
}

void removechar(psy_ui_TextAreaPane* self)
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
	psy_ui_textformat_clear(&self->format);
}

char_dyn_t* lefttext(psy_ui_TextAreaPane* self, uintptr_t split)
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

char_dyn_t* righttext(psy_ui_TextAreaPane* self, uintptr_t split)
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

void psy_ui_textareapane_onfocus(psy_ui_TextAreaPane* self)
{	
	super_vtable.onfocus(&self->component);
	if (self->isinputfield) {
		self->preventedit = FALSE;
		psy_ui_app_startmousehook(psy_ui_app());
	}
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_textareapane_onfocuslost(psy_ui_TextAreaPane* self)
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

void psy_ui_textareapane_onmousehook(psy_ui_TextAreaPane* self, psy_ui_App* sender,
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

void psy_ui_textareapane_ondraw(psy_ui_TextAreaPane* self, psy_ui_Graphics* g)
{		
	psy_ui_TextDraw textdraw;
			
	psy_ui_textdraw_init(&textdraw, &self->format, psy_ui_component_size_px(
		psy_ui_textareapane_base(self)), self->text);
	psy_ui_textdraw_draw(&textdraw, g, psy_ui_component_font(&self->component),
		psy_ui_component_textmetric(&self->component),
		(psy_ui_component_hasfocus(&self->component))
		? self->cp
		: psy_INDEX_INVALID);	
	psy_ui_textdraw_dispose(&textdraw);
}

void psy_ui_textareapane_onmousedown(psy_ui_TextAreaPane* self, psy_ui_MouseEvent* ev)
{	
	self->cp = psy_ui_textformat_cursor_position(&self->format,
		self->text,
		psy_ui_mouseevent_offset(ev),
		psy_ui_component_textmetric(&self->component),
		psy_ui_component_font(&self->component));
	psy_ui_mouseevent_stop_propagation(ev);	
	psy_ui_component_set_focus(&self->component);
	psy_ui_component_invalidate(&self->component);
}

/* psy_ui_TextArea */

/* prototypes */
static void psy_ui_textarea_ondestroy(psy_ui_TextArea*, psy_ui_Component* parent);
static void psy_ui_textarea_oneditaccept(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textarea_oneditreject(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textarea_oneditchange(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);

/* implementation */
void psy_ui_textarea_init(psy_ui_TextArea* self, psy_ui_Component* parent)
{
	psy_ui_component_init(psy_ui_textarea_base(self), parent, NULL);
	psy_ui_textareapane_init(&self->pane, psy_ui_textarea_base(self));
	psy_ui_scroller_init(&self->scroller, &self->pane.component,
		psy_ui_textarea_base(self));
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_textarea_ondestroy);
	psy_signal_connect(&self->pane.signal_accept, self,
		psy_ui_textarea_oneditaccept);
	psy_signal_connect(&self->pane.signal_reject, self,
		psy_ui_textarea_oneditreject);
	psy_signal_connect(&self->pane.signal_change, self,
		psy_ui_textarea_oneditchange);
}

void psy_ui_textarea_init_single_line(psy_ui_TextArea* self, psy_ui_Component* parent)
{
	psy_ui_textarea_init(self, parent);
	psy_ui_textarea_prevent_wrap(self);
	psy_ui_textformat_set_alignment(&self->pane.format, psy_ui_textalignment_make(
		psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_component_set_align(&self->pane.component, psy_ui_ALIGN_CLIENT);
}

void psy_ui_textarea_ondestroy(psy_ui_TextArea* self, psy_ui_Component* parent)
{
	psy_signal_dispose(&self->signal_change);
	psy_signal_dispose(&self->signal_accept);
	psy_signal_dispose(&self->signal_reject);
}


void psy_ui_textarea_prevent_wrap(psy_ui_TextArea* self)
{
	psy_ui_textformat_prevent_wrap(&self->pane.format);
}

void psy_ui_textarea_oneditaccept(psy_ui_TextArea* self,
	psy_ui_TextAreaPane* sender)
{	
	psy_signal_emit(&self->signal_accept, self, 0);
}

void psy_ui_textarea_oneditreject(psy_ui_TextArea* self,
	psy_ui_TextAreaPane* sender)
{
	psy_signal_emit(&self->signal_reject, self, 0);
}

void psy_ui_textarea_oneditchange(psy_ui_TextArea* self,
	psy_ui_TextAreaPane* sender)
{
	psy_signal_emit(&self->signal_change, self, 0);
}
