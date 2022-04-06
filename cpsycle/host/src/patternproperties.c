/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternproperties.h"
/* platform */
#include "../../detail/portable.h"

static void patternproperties_onpatternnamechanged(PatternProperties*,
	psy_audio_Pattern* sender);
static void patternproperties_onpatternlengthchanged(PatternProperties*,
	psy_audio_Pattern* sender);
static void patternproperties_onapply(PatternProperties*,
	psy_ui_Component* sender);
static void patternproperties_on_key_down(PatternProperties*, psy_ui_KeyboardEvent*);
static void patternproperties_onkeyup(PatternProperties*, psy_ui_KeyboardEvent*);
static void patternproperties_on_focus(PatternProperties*);
static void patternproperties_ontimesignominator(PatternProperties*, IntEdit* sender);
static void patternproperties_ontimesigdenominator(PatternProperties*, IntEdit* sender);
static psy_audio_Pattern* patternproperties_pattern(PatternProperties*);

static psy_ui_ComponentVtable patternproperties_vtable;
static bool patternproperties_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* patternproperties_vtable_init(PatternProperties* self)
{
	if (!patternproperties_vtable_initialized) {
		patternproperties_vtable = *(self->component.vtable);
		patternproperties_vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			patternproperties_on_key_down;
		patternproperties_vtable.onkeyup =
			(psy_ui_fp_component_on_key_event)
			patternproperties_onkeyup;
		patternproperties_vtable.on_focus =
			(psy_ui_fp_component_event)
			patternproperties_on_focus;
		patternproperties_vtable_initialized = TRUE;
	}
	return &patternproperties_vtable;
}

void patternproperties_init(PatternProperties* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_vtable(&self->component,
		patternproperties_vtable_init(self));	
	self->patterns = NULL;
	self->pattern_index = psy_INDEX_INVALID;
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_make_px(0), psy_ui_value_make_ew(2.0),
			psy_ui_value_make_eh(1.0), psy_ui_value_make_px(0)));	
	psy_ui_label_init_text(&self->namelabel, &self->component,
		"patternview.patname");	
	psy_ui_textarea_init_single_line(&self->nameedit, &self->component);	
	psy_ui_textarea_settext(&self->nameedit, "patternview.nopattern");
	psy_ui_textarea_setcharnumber(&self->nameedit, 40);
	// psy_ui_textarea_enableinputfield(&self->nameedit);
	psy_ui_label_init_text(&self->lengthlabel, &self->component,
		"patternview.length");
	psy_ui_label_set_textalignment(&self->lengthlabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_textarea_init_single_line(&self->lengthedit, &self->component);	
	psy_ui_textarea_setcharnumber(&self->lengthedit, 20);
	// psy_ui_textarea_enableinputfield(&self->lengthedit);
	psy_ui_button_init_connect(&self->applybutton, &self->component, self,
		patternproperties_onapply);	
	intedit_init(&self->timesig_numerator, &self->component,
		"Timesignature", 0, 0, 128);
	psy_ui_component_set_padding(&self->timesig_numerator.less.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_padding(&self->timesig_numerator.more.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	psy_signal_connect(&self->timesig_numerator.signal_changed, self,
		patternproperties_ontimesignominator);
	intedit_init(&self->timesig_denominator, &self->component,
		"", 0, 0, 128);
	psy_ui_component_set_padding(&self->timesig_denominator.less.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_padding(&self->timesig_denominator.more.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));	
	psy_signal_connect(&self->timesig_denominator.signal_changed, self,
		patternproperties_ontimesigdenominator);	
	psy_ui_button_set_text(&self->applybutton, "patternview.apply");
	psy_ui_component_set_padding(&self->applybutton.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	psy_ui_button_settextalignment(&self->applybutton, psy_ui_ALIGNMENT_LEFT);	
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->component);
}

void patternproperties_onapply(PatternProperties* self,
	psy_ui_Component* sender)
{
	psy_audio_Pattern* pattern;	

	pattern = patternproperties_pattern(self);
	if (pattern) {
		psy_audio_pattern_setname(pattern, psy_ui_textarea_text(&self->nameedit));				
		psy_audio_pattern_setlength(pattern, (psy_dsp_big_beat_t)
			atof(psy_ui_textarea_text(&self->lengthedit)));		
	}
}

void patternproperties_on_key_down(PatternProperties* self, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
		patternproperties_onapply(self, &self->component);
		psy_ui_keyboardevent_prevent_default(ev);
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void patternproperties_onkeyup(PatternProperties* self, psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
}

void patternproperties_on_focus(PatternProperties* self)
{
	psy_ui_component_set_focus(psy_ui_textarea_base(&self->lengthedit));
}

void patternproperties_onpatternnamechanged(PatternProperties* self,
	psy_audio_Pattern* pattern)
{	
	psy_ui_textarea_settext(&self->nameedit,
		psy_audio_pattern_name(pattern));	
}

void patternproperties_onpatternlengthchanged(PatternProperties* self,
	psy_audio_Pattern* sender)
{	
	char buffer[20];

	psy_snprintf(buffer, 20, "%.4f", sender->length);
	psy_ui_textarea_settext(&self->lengthedit, buffer);
}

void patternproperties_ontimesignominator(PatternProperties* self, IntEdit* sender)
{
	if (patternproperties_pattern(self)) {
		patternproperties_pattern(self)->timesig.numerator = intedit_value(sender);
	}
}

void patternproperties_ontimesigdenominator(PatternProperties* self, IntEdit* sender)
{
	if (patternproperties_pattern(self)) {
		patternproperties_pattern(self)->timesig.denominator = intedit_value(sender);
	}
}

void patternproperties_set_patterns(PatternProperties* self, psy_audio_Patterns* patterns)
{
	self->patterns = patterns;
	self->pattern_index = psy_INDEX_INVALID;
}

void patternproperties_select(PatternProperties* self, uintptr_t pattern_index)
{
	if (self->pattern_index != pattern_index) {
		psy_audio_Pattern* pattern;
		char buffer[64];

		self->pattern_index = pattern_index;
		pattern = patternproperties_pattern(self);
		if (pattern) {
			psy_ui_textarea_settext(&self->nameedit, psy_audio_pattern_name(pattern));
			psy_snprintf(buffer, 20, "%.4f", (float)psy_audio_pattern_length(pattern));
			intedit_setvalue(&self->timesig_numerator, (int)pattern->timesig.numerator);
			intedit_setvalue(&self->timesig_denominator, (int)pattern->timesig.denominator);
			psy_signal_connect(&pattern->signal_namechanged, self,
				patternproperties_onpatternnamechanged);
			psy_signal_connect(&pattern->signal_lengthchanged, self,
				patternproperties_onpatternlengthchanged);
		} else {
			psy_ui_textarea_settext(&self->nameedit, "");
			psy_snprintf(buffer, 10, "");
		}
		psy_ui_textarea_settext(&self->lengthedit, buffer);				
	}
}

psy_audio_Pattern* patternproperties_pattern(PatternProperties* self)
{
	return psy_audio_patterns_at(self->patterns, self->pattern_index);	
}
