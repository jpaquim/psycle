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
static void psy_ui_textareapane_on_destroyed(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_on_preferred_size(psy_ui_TextAreaPane*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_textareapane_on_key_down(psy_ui_TextAreaPane*,
	psy_ui_KeyboardEvent*);
static void psy_ui_textareapane_on_focus_lost(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_on_draw(psy_ui_TextAreaPane*,
	psy_ui_Graphics*);
static void psy_ui_textareapane_on_mouse_down(psy_ui_TextAreaPane*,
	psy_ui_MouseEvent*);
static void insertchar(psy_ui_TextAreaPane*, char c);
static void deletechar(psy_ui_TextAreaPane*);
static void removechar(psy_ui_TextAreaPane*);
static char_dyn_t* lefttext(psy_ui_TextAreaPane*, uintptr_t split);
static char_dyn_t* righttext(psy_ui_TextAreaPane*, uintptr_t split);
static void psy_ui_textarea_on_edit_accept(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textarea_on_edit_reject(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textareapane_prev_col(psy_ui_TextAreaPane*,
	uintptr_t step);
static void psy_ui_textareapane_next_col(psy_ui_TextAreaPane*,
	uintptr_t step);
static void psy_ui_textareapane_prev_lines(psy_ui_TextAreaPane*,
	uintptr_t step);
static void psy_ui_textareapane_advance_lines(psy_ui_TextAreaPane*,
	uintptr_t step);
static void psy_ui_textareapane_scroll_up(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_scroll_down(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_scroll_left(psy_ui_TextAreaPane*);
static void psy_ui_textareapane_scroll_right(psy_ui_TextAreaPane*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_TextAreaPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			psy_ui_textareapane_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_textareapane_on_draw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_textareapane_on_preferred_size;		
		vtable.on_focuslost =
			(psy_ui_fp_component_event)
			psy_ui_textareapane_on_focus_lost;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			psy_ui_textareapane_on_key_down;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_textareapane_on_mouse_down;
		vtable_initialized = TRUE;
	}	
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_textareapane_init(psy_ui_TextAreaPane* self,
	psy_ui_Component* parent)
{ 
	psy_ui_component_init(psy_ui_textareapane_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_set_aligner(&self->component, NULL);
	psy_ui_component_set_tab_index(&self->component, 0);
	self->charnumber = 0.0;
	self->linenumber = 1;
	self->isinputfield = FALSE;	
	self->text = psy_strdup("textinput	tab1	tab2 word line break");
	self->cp = 0;
	psy_ui_textformat_init(&self->format);
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);	
	psy_ui_component_set_style_type(&self->component,
		psy_ui_STYLE_EDIT);
	psy_ui_component_set_style_type_focus(&self->component,
		psy_ui_STYLE_EDIT_FOCUS);
	psy_ui_component_setscrollstep(psy_ui_textareapane_base(self),
		psy_ui_size_make_em(1.0, 1.0));		
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_set_overflow(&self->component,
		psy_ui_OVERFLOW_SCROLL);
}

void psy_ui_textareapane_on_destroyed(psy_ui_TextAreaPane* self)
{	
	psy_signal_dispose(&self->signal_change);
	psy_signal_dispose(&self->signal_accept);
	psy_signal_dispose(&self->signal_reject);	
	free(self->text);
	self->text = NULL;
	psy_ui_textformat_dispose(&self->format);
}

void psy_ui_textareapane_enable_inputfield(psy_ui_TextAreaPane* self)
{
	self->isinputfield = TRUE;	
}

void psy_ui_textareapane_set_text(psy_ui_TextAreaPane* self,
	const char* text)
{	
	psy_strreset(&self->text, text);
	psy_ui_textformat_clear(&self->format);
	self->cp = psy_min(self->cp, psy_strlen(text));
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_textareapane_add_text(psy_ui_TextAreaPane* self,
	const char* text)
{
	self->text = psy_strcat_realloc(self->text, text);
	psy_ui_textformat_clear(&self->format);	
	psy_ui_component_invalidate(&self->component);
}

 const char* psy_ui_textareapane_text(const psy_ui_TextAreaPane* self)
{	
	return self->text;
}

 uintptr_t psy_ui_textarea_length(const psy_ui_TextArea* self)
 {
	 return psy_strlen(self->pane.text);
 }

 void psy_ui_textarea_range(psy_ui_TextArea* self, intptr_t start,
	intptr_t end, char* rv)
 {
	 if (end > start) {
		 psy_snprintf(rv, end - start, self->pane.text);
	 }
 }

void psy_ui_textareapane_set_char_number(psy_ui_TextAreaPane* self,
	double number)
{
	self->charnumber = number;
	psy_ui_textformat_clear(&self->format);
}

void psy_ui_textareapane_set_line_number(psy_ui_TextAreaPane* self,
	int number)
{
	self->linenumber = number;
}

void psy_ui_textareapane_on_preferred_size(psy_ui_TextAreaPane* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{			
	const psy_ui_TextMetric* tm;	
	double width;
	const psy_ui_Font* font;
	
	font = psy_ui_component_font(psy_ui_textareapane_base(self));
	tm = psy_ui_component_textmetric(psy_ui_textareapane_base(self));	
	width = (self->charnumber == 0 && limit)
		? psy_ui_value_px(&limit->width, tm, limit)
		: (self->charnumber) * tm->tmAveCharWidth;
	psy_ui_textformat_update(&self->format, self->text, width, font,
		tm);
	if (self->format.line_wrap && !self->format.word_wrap &&
			self->charnumber == 0.0) {
		width = psy_max(psy_ui_value_px(&limit->width, tm, limit),
			self->format.nummaxchars * tm->tmAveCharWidth);
	}
	*rv = psy_ui_size_make_px(width,
		psy_ui_textformat_numlines(&self->format) *
			(tm->tmHeight * 1.0));	
}

void psy_ui_textareapane_enable_edit(psy_ui_TextAreaPane* self)
{
	psy_ui_component_enable_input(&self->component,
		psy_ui_NONE_RECURSIVE);
}

void psy_ui_textareapane_prevent_edit(psy_ui_TextAreaPane* self)
{
	psy_ui_component_prevent_input(&self->component,
		psy_ui_NONE_RECURSIVE);
}

void psy_ui_textareapane_set_sel(psy_ui_TextAreaPane* self,
	uintptr_t cpmin, uintptr_t cpmax)
{

}

/* signal_accept event handling */
void psy_ui_textareapane_on_key_down(psy_ui_TextAreaPane* self,
	psy_ui_KeyboardEvent* ev)
{	
	assert(self);

	switch (psy_ui_keyboardevent_keycode(ev)) {
	case psy_ui_KEY_ESCAPE:
		if (self->isinputfield) {
			psy_ui_keyboardevent_prevent_default(ev);						
			psy_signal_emit(&self->signal_reject, self, 0);
			psy_ui_keyboardevent_prevent_default(ev);
		}
		break;	
	case psy_ui_KEY_LEFT:
		psy_ui_textareapane_prev_col(self, 1);		
		break;
	case psy_ui_KEY_RIGHT:
		psy_ui_textareapane_next_col(self, 1);
		break;
	case psy_ui_KEY_UP:
		psy_ui_textareapane_prev_lines(self, 1);
		break;
	case psy_ui_KEY_DOWN:
		psy_ui_textareapane_advance_lines(self, 1);		
		break;
	case psy_ui_KEY_PRIOR:
		psy_ui_textareapane_prev_lines(self, 16);
		break;
	case psy_ui_KEY_NEXT:
		psy_ui_textareapane_advance_lines(self, 16);
		break;
	case psy_ui_KEY_HOME: {		
		uintptr_t currline;

		currline = psy_ui_textareapane_cursor_line(self);
		if (currline > 0) {
			self->cp = psy_ui_textformat_line_at(&self->format,
				currline - 1) +
				1;
		} else {
			self->cp = 0;
		}
		psy_ui_textareapane_scroll_left(self);
		break; }
	case psy_ui_KEY_END: {
		uintptr_t currline;		

		currline = psy_ui_textareapane_cursor_line(self);						
		self->cp = psy_min(
			psy_ui_textformat_line_at(&self->format, currline),
			psy_strlen(self->text));
		psy_ui_textareapane_scroll_right(self);
		break; }
	case psy_ui_KEY_BACK:
		deletechar(self);
		break;
	case psy_ui_KEY_DELETE:
		removechar(self);
		break;
	case psy_ui_KEY_SPACE:
		insertchar(self, ' ');
		break;
	case psy_ui_KEY_TAB:
		insertchar(self, '\t');
		break;
	case psy_ui_KEY_SHIFT:
	case psy_ui_KEY_CONTROL:
	case psy_ui_KEY_MENU:
		break;
	default:
		if (psy_ui_keyboardevent_shiftkey(ev)) {
			insertchar(self, psy_ui_keyboardevent_keycode(ev));
		} else if (psy_ui_keyboardevent_keycode(ev) ==
				psy_ui_KEY_RETURN) {
			if (self->format.line_wrap) {
				insertchar(self, '\n');
				psy_ui_component_align(psy_ui_component_parent(
					&self->component));
			} else {
				if (self->isinputfield) {
					psy_signal_emit(&self->signal_accept, self, 0);
				}
			}
		} else if (psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_DIGIT0
			&& psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DIGIT9) {
			insertchar(self, psy_ui_keyboardevent_keycode(ev));
		} else {
			insertchar(self, psy_ui_keyboardevent_keycode(ev) -
				'A' + 'a');
		}						
		break;
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);	
}

void psy_ui_textareapane_prev_col(psy_ui_TextAreaPane* self,
	uintptr_t step)
{
	if (self->cp > step) {
		self->cp -= step;
	} else {
		self->cp = 0;
	}
	psy_ui_textareapane_scroll_left(self);
}

void psy_ui_textareapane_next_col(psy_ui_TextAreaPane* self,
	uintptr_t step)
{	
	self->cp = psy_min(self->cp + step, psy_strlen(self->text));
}

void psy_ui_textareapane_prev_lines(psy_ui_TextAreaPane* self,
	uintptr_t step)
{
	uintptr_t currline;
	double cpx;
	double cpy;
	const psy_ui_TextMetric* tm;
	uintptr_t oldlinestart;
	uintptr_t linestart;
	uintptr_t offset;

	currline = psy_ui_textareapane_cursor_line(self);
	if (currline > 0) {
		oldlinestart = psy_ui_textformat_line_at(&self->format,
			currline - 1);
	} else {
		oldlinestart = 0;
	}
	offset = self->cp - oldlinestart;
	if (currline > step) {
		currline -= step;
	} else {
		currline = 0;
	}
	tm = psy_ui_component_textmetric(&self->component);
	cpy = currline * self->format.linespacing * tm->tmHeight;
	if (currline > 0) {
		linestart = psy_ui_textformat_line_at(&self->format,
			currline - 1);
	} else {
		linestart = 0;
	}
	if (offset + linestart > oldlinestart) {
		offset = oldlinestart - linestart;
	}
	cpx = psy_ui_textformat_screen_offset(&self->format,
		self->text + linestart, offset,
		psy_ui_component_font(&self->component),
		psy_ui_component_textmetric(&self->component));
	self->cp = psy_ui_textformat_cursor_position(&self->format,
		self->text,
		psy_ui_realpoint_make(cpx, cpy),
		psy_ui_component_textmetric(&self->component),
		psy_ui_component_font(&self->component));
	psy_ui_textareapane_scroll_up(self);
}

void psy_ui_textareapane_advance_lines(psy_ui_TextAreaPane* self,
	uintptr_t step)
{
	uintptr_t currline;
	double cpx;
	double cpy;
	const psy_ui_TextMetric* tm;
	uintptr_t oldlinestart;
	uintptr_t linestart;
	uintptr_t lineend;
	uintptr_t offset;

	currline = psy_ui_textareapane_cursor_line(self);
	if (currline > 0) {
		oldlinestart = psy_ui_textformat_line_at(&self->format,
			currline - 1);
	} else {
		oldlinestart = 0;
	}
	offset = self->cp - oldlinestart;
	if (currline + step < psy_ui_textformat_numlines(&self->format)) {
		currline += step;
	} else {
		currline = psy_ui_textformat_numlines(&self->format);
		if (currline > 0) {
			--currline;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	cpy = currline * self->format.linespacing * tm->tmHeight;
	linestart = psy_ui_textformat_line_at(&self->format, currline - 1);
	lineend = psy_ui_textformat_line_at(&self->format, currline);
	if (offset + linestart > lineend) {
		offset = lineend - linestart;
	}
	cpx = psy_ui_textformat_screen_offset(&self->format,
		self->text + linestart, offset,
		psy_ui_component_font(&self->component),
		psy_ui_component_textmetric(&self->component));
	self->cp = psy_ui_textformat_cursor_position(&self->format,
		self->text,
		psy_ui_realpoint_make(cpx, cpy),
		psy_ui_component_textmetric(&self->component),
		psy_ui_component_font(&self->component));
	psy_ui_textareapane_scroll_down(self);
}

uintptr_t psy_ui_textareapane_cursor_line(const psy_ui_TextAreaPane*
	self)
{
	uintptr_t line;
	uintptr_t numlines;
	uintptr_t linestart;

	numlines = psy_ui_textformat_numlines(&self->format);
	if (numlines == 0) {
		return 0;
	}
	linestart = 0;
	for (line = 0; line < numlines; ++line) {
		uintptr_t cp;

		cp = psy_ui_textformat_line_at(&self->format, line);
		if (self->cp >= linestart && self->cp <= cp) {
			break;
		}
		linestart = cp + 1;
	}
	return line;
}

uintptr_t psy_ui_textareapane_cursor_column(const psy_ui_TextAreaPane*
	self)
{
	uintptr_t line;	
	uintptr_t linestart;

	line = psy_ui_textareapane_cursor_line(self);
	if (line > 0) {
		linestart = psy_ui_textformat_line_at(&self->format, line - 1) +
			1;
	} else {
		linestart = 0;
	}
	return self->cp - linestart;
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
	psy_signal_emit(&self->signal_change, self, 0);
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
	psy_signal_emit(&self->signal_change, self, 0);
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
	psy_signal_emit(&self->signal_change, self, 0);
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

void psy_ui_textareapane_on_focus_lost(psy_ui_TextAreaPane* self)
{
	assert(self);

	super_vtable.on_focuslost(&self->component);
	if (self->isinputfield) {		
		psy_signal_emit(&self->signal_accept, self, 0);		
	}	
}

void psy_ui_textareapane_on_draw(psy_ui_TextAreaPane* self,
	psy_ui_Graphics* g)
{		
	psy_ui_TextDraw textdraw;	
	
	psy_ui_textdraw_init(&textdraw, &self->format,
		psy_ui_component_size_px(psy_ui_textareapane_base(self)),
		self->text);
	psy_ui_textdraw_draw(&textdraw, g,
		(psy_ui_component_has_focus(&self->component))
		? self->cp
		: psy_INDEX_INVALID);	
	psy_ui_textdraw_dispose(&textdraw);
}

void psy_ui_textareapane_on_mouse_down(psy_ui_TextAreaPane* self,
	psy_ui_MouseEvent* ev)
{	
	self->cp = psy_min(
		psy_ui_textformat_cursor_position(&self->format, self->text,
			psy_ui_mouseevent_offset(ev),
			psy_ui_component_textmetric(&self->component),
			psy_ui_component_font(&self->component)),
		psy_strlen(self->text));
	psy_ui_mouseevent_stop_propagation(ev);	
	psy_ui_component_set_focus(&self->component);	
	psy_ui_component_invalidate(&self->component);
}

/* psy_ui_TextArea */

/* prototypes */
static void psy_ui_textarea_on_destroyed(psy_ui_TextArea*);
static void psy_ui_textarea_on_property_changed(psy_ui_TextArea*,
	psy_Property* sender);
static void psy_ui_textarea_before_property_destroyed(psy_ui_TextArea*,
	psy_Property* sender);
static void psy_ui_textarea_on_edit_accept(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textarea_on_edit_reject(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);
static void psy_ui_textarea_oneditchange(psy_ui_TextArea*,
	psy_ui_TextAreaPane* sender);

/* psy_ui_textarea_vtable */
static psy_ui_ComponentVtable psy_ui_textarea_vtable;
static psy_ui_ComponentVtable psy_ui_textarea_super_vtable;
static bool psy_ui_textarea_vtable_initialized = FALSE;

static void psy_ui_textarea_vtable_init(psy_ui_TextArea* self)
{
	if (!psy_ui_textarea_vtable_initialized) {
		psy_ui_textarea_vtable = *(self->component.vtable);
		psy_ui_textarea_super_vtable = *(self->component.vtable);
		psy_ui_textarea_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			psy_ui_textarea_on_destroyed;		
		psy_ui_textarea_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &psy_ui_textarea_vtable);
}

/* implementation */
void psy_ui_textarea_init(psy_ui_TextArea* self, psy_ui_Component* parent)
{
	psy_ui_component_init(psy_ui_textarea_base(self), parent, NULL);
	psy_ui_textarea_vtable_init(self);
	self->property = NULL;
	psy_ui_textareapane_init(&self->pane, psy_ui_textarea_base(self));
	psy_ui_scroller_init(&self->scroller, psy_ui_textarea_base(self),
		NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->pane.component);
	psy_ui_component_set_align(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);	
	psy_signal_connect(&self->pane.signal_accept, self,
		psy_ui_textarea_on_edit_accept);
	psy_signal_connect(&self->pane.signal_reject, self,
		psy_ui_textarea_on_edit_reject);
	psy_signal_connect(&self->pane.signal_change, self,
		psy_ui_textarea_oneditchange);
}

void psy_ui_textarea_init_single_line(psy_ui_TextArea* self,
	psy_ui_Component* parent)
{
	psy_ui_textarea_init(self, parent);
	psy_ui_textarea_prevent_wrap(self);
	psy_ui_textformat_set_alignment(&self->pane.format,
		psy_ui_textalignment_make(psy_ui_ALIGNMENT_LEFT |
			psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_component_set_align(&self->pane.component,
		psy_ui_ALIGN_CLIENT);
}

void psy_ui_textarea_on_destroyed(psy_ui_TextArea* self)
{
	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
	psy_signal_dispose(&self->signal_change);
	psy_signal_dispose(&self->signal_accept);
	psy_signal_dispose(&self->signal_reject);
}

psy_ui_TextArea* psy_ui_textarea_alloc(void)
{
	return (psy_ui_TextArea*)malloc(sizeof(psy_ui_TextArea));
}

psy_ui_TextArea* psy_ui_textarea_allocinit(psy_ui_Component* parent)
{
	psy_ui_TextArea* rv;

	rv = psy_ui_textarea_alloc();
	if (rv) {
		psy_ui_textarea_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}
psy_ui_TextArea* psy_ui_textarea_allocinit_single_line(
	psy_ui_Component* parent)
{
	psy_ui_TextArea* rv;

	rv = psy_ui_textarea_alloc();
	if (rv) {
		psy_ui_textarea_init_single_line(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_textarea_data_exchange(psy_ui_TextArea* self,
	psy_Property* property)
{
	self->property = property;
	if (self->property) {
		psy_ui_textarea_on_property_changed(self, self->property);
		psy_property_connect(self->property, self,
			psy_ui_textarea_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			psy_ui_textarea_before_property_destroyed);
	}
}

void psy_ui_textarea_on_property_changed(psy_ui_TextArea* self, psy_Property* sender)
{
	if (psy_property_is_int(sender)) {
		char text[64];

		psy_snprintf(text, 40,
			(psy_property_is_hex(self->property)) ? "%X" : "%d",
			(int)psy_property_item_int(self->property));
		psy_ui_textarea_set_text(self, text);
	} else if (psy_property_is_double(sender)) {
		char text[64];

		psy_snprintf(text, 40, "%f", psy_property_item_double(sender));
		psy_ui_textarea_set_text(self, text);
	} else if (psy_property_is_string(sender)) {
		psy_ui_textarea_set_text(self, psy_property_item_str(sender));
	}
}

void psy_ui_textarea_before_property_destroyed(psy_ui_TextArea* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void psy_ui_textarea_prevent_wrap(psy_ui_TextArea* self)
{
	psy_ui_textformat_prevent_word_wrap(&self->pane.format);
	psy_ui_textformat_prevent_line_wrap(&self->pane.format);
}

void psy_ui_textarea_word_wrap(psy_ui_TextArea* self)
{
	psy_ui_textformat_word_wrap(&self->pane.format);
}

void psy_ui_textarea_line_wrap(psy_ui_TextArea* self)
{
	psy_ui_textformat_line_wrap(&self->pane.format);
}

void psy_ui_textarea_on_edit_accept(psy_ui_TextArea* self,
	psy_ui_TextAreaPane* sender)
{	
	if (self->property) {
		if (psy_property_is_double(self->property)) {
			psy_property_set_item_double(self->property,
				strtof(psy_ui_textarea_text(self), NULL));
		} else if (psy_property_is_int(self->property)) {
			psy_property_set_item_int(self->property,
				(psy_property_is_hex(self->property))
				? strtol(psy_ui_textarea_text(self), NULL, 16)
				: atoi(psy_ui_textarea_text(self)));
		} else if (psy_property_is_string(self->property)) {
			psy_property_set_item_str(self->property, psy_ui_textarea_text(self));
		}
	}
	psy_signal_emit(&self->signal_accept, self, 0);
}

void psy_ui_textarea_on_edit_reject(psy_ui_TextArea* self,
	psy_ui_TextAreaPane* sender)
{
	psy_signal_emit(&self->signal_reject, self, 0);
}

void psy_ui_textarea_oneditchange(psy_ui_TextArea* self,
	psy_ui_TextAreaPane* sender)
{
	psy_signal_emit(&self->signal_change, self, 0);
}

void psy_ui_textareapane_scroll_up(psy_ui_TextAreaPane* self)
{
	intptr_t line;
	intptr_t topline;
	double top;
	double line_height;
	const psy_ui_TextMetric* tm;

	line = psy_ui_textareapane_cursor_line(self);
	tm = psy_ui_component_textmetric(&self->component);
	line_height = tm->tmHeight * self->format.linespacing;
	top = line_height * line;
	topline = 0;	
	if (psy_ui_component_scroll_top_px(&self->component) +
			topline * line_height > top) {
		intptr_t dlines;

		dlines = (intptr_t)((psy_ui_component_scroll_top_px(
			&self->component) + topline * line_height - top) /
			(line_height));
		psy_ui_component_set_scroll_top_px(&self->component,
			psy_ui_component_scroll_top_px(&self->component) -
			psy_ui_component_scroll_step_height_px(&self->component) *
				dlines);
	}
}

void psy_ui_textareapane_scroll_down(psy_ui_TextAreaPane* self)
{
	intptr_t line;
	intptr_t visilines;
	psy_ui_RealSize clientsize;
	double line_height;
	const psy_ui_TextMetric* tm;

	line = psy_ui_textareapane_cursor_line(self);
	tm = psy_ui_component_textmetric(&self->component);
	line_height = tm->tmHeight * self->format.linespacing;
	clientsize = psy_ui_component_clientsize_px(&self->component);
	visilines = (uintptr_t)(clientsize.height / line_height);		
	if (visilines < line - psy_ui_component_scroll_top_px(
			&self->component) / line_height) {
		intptr_t dlines;

		dlines = (intptr_t)
			(line - psy_ui_component_scroll_top_px(&self->component) /
				line_height - visilines);
		self->component.blitscroll = TRUE;
		psy_ui_component_set_scroll_top_px(&self->component,
			psy_ui_component_scroll_top_px(&self->component) +
			psy_ui_component_scroll_step_height_px(&self->component) *
				dlines);
		self->component.blitscroll = FALSE;
	}
}

void psy_ui_textareapane_scroll_left(psy_ui_TextAreaPane* self)
{
	intptr_t column;	
	intptr_t line;
	intptr_t linestart;
	double screen_offset;	
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);	
	line = psy_ui_textareapane_cursor_line(self);
	column = psy_ui_textareapane_cursor_column(self);
	if (line > 0) {
		linestart = psy_ui_textformat_line_at(&self->format, line - 1) +
			1;
	} else {
		linestart = 0;
	}
	screen_offset = psy_ui_textformat_screen_offset(&self->format,
		self->text + linestart, psy_max(0, column),
		psy_ui_component_font(&self->component),
		tm);	
			
	if (psy_ui_component_scroll_left_px(&self->component) >
			screen_offset) {
		intptr_t chars;
		
		chars = (intptr_t)(screen_offset / tm->tmAveCharWidth);
		psy_ui_component_set_scroll_left(&self->component,
			psy_ui_value_make_px((double)(chars *
				(intptr_t)tm->tmAveCharWidth)));
	}
}

void psy_ui_textareapane_scroll_right(psy_ui_TextAreaPane* self)
{
	intptr_t column;
	intptr_t line;
	intptr_t linestart;	
	psy_ui_RealSize client_size;
	const psy_ui_TextMetric* tm;
	double screen_offset;

	tm = psy_ui_component_textmetric(&self->component);
	client_size = psy_ui_component_clientsize_px(&self->component);
	line = psy_ui_textareapane_cursor_line(self);
	column = psy_ui_textareapane_cursor_column(self);
	if (line > 0) {
		linestart = psy_ui_textformat_line_at(&self->format, line - 1) +
			1;
	} else {
		linestart = 0;
	}
	screen_offset = psy_ui_textformat_screen_offset(&self->format,
		self->text + linestart, psy_max(0, column),
		psy_ui_component_font(&self->component),
		tm);	
	if (screen_offset >
			psy_ui_component_scroll_left_px(&self->component) +
			client_size.width) {				
		psy_ui_component_set_scroll_left(&self->component,
			psy_ui_value_make_px(psy_ui_component_scroll_left_px(
				&self->component) + tm->tmAveCharWidth));
	}
}

uintptr_t psy_ui_textarea_cursor_line(const psy_ui_TextArea* self)
{
	return psy_ui_textareapane_cursor_line(&self->pane);
}

uintptr_t psy_ui_textarea_cursor_column(const psy_ui_TextArea* self)
{
	return psy_ui_textareapane_cursor_column(&self->pane);
}
