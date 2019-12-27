#include "../../detail/prefix.h"

#include "patternproperties.h"
#include <stdio.h>
#include <portable.h>

static void OnSize(PatternProperties* self, psy_ui_Component* sender, ui_size*);
static void OnApplyClicked(PatternProperties* self, psy_ui_Component* sender);
static void OnCloseClicked(PatternProperties* self, psy_ui_Component* sender);

void InitPatternProperties(PatternProperties* self, psy_ui_Component* parent, psy_audio_Pattern* pattern)
{			
	self->pattern = pattern;
	ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_size, self, OnSize);	
	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Name");
	ui_label_init(&self->lengthlabel, &self->component);
	ui_label_settext(&self->lengthlabel, "Length");
	ui_edit_init(&self->nameedit, &self->component, 0);
	ui_edit_settext(&self->nameedit, "No Pattern");
	ui_edit_init(&self->lengthedit, &self->component, 0);
	ui_button_init(&self->applybutton, &self->component);
	ui_button_settext(&self->applybutton, "Apply");	
	psy_signal_connect(&self->applybutton.signal_clicked, self, OnApplyClicked);	
	ui_button_init(&self->closebutton, &self->component);
	ui_button_settext(&self->closebutton, "x");	
	psy_signal_connect(&self->closebutton.signal_clicked, self, OnCloseClicked);	
	ui_component_move(&self->namelabel.component, 10, 10);
	ui_component_resize(&self->namelabel.component, 80, 20);
	ui_component_move(&self->lengthlabel.component, 10, 35);
	ui_component_resize(&self->lengthlabel.component, 80, 20);
	ui_component_move(&self->nameedit.component, 100, 10);
	ui_component_resize(&self->nameedit.component, 100, 20);
	ui_component_move(&self->lengthedit.component, 100, 35);
	ui_component_resize(&self->lengthedit.component, 100, 20);
	ui_component_move(&self->applybutton.component, 220, 35);
	ui_component_resize(&self->applybutton.component, 100, 20);	
}

void PatternPropertiesSetPattern(PatternProperties* self, psy_audio_Pattern* pattern)
{
	char buffer[20];
	self->pattern = pattern;
	if (self->pattern) {
		ui_edit_settext(&self->nameedit, pattern->label);
		psy_snprintf(buffer, 20, "%.4f", self->pattern->length);		
	} else {
		ui_edit_settext(&self->nameedit, "");
		psy_snprintf(buffer, 10, "");
	}
	ui_edit_settext(&self->lengthedit, buffer);
}

void OnSize(PatternProperties* self, psy_ui_Component* sender, ui_size* size)
{
	ui_component_move(&self->closebutton.component, size->width - 25, 5);
	ui_component_resize(&self->closebutton.component, 20, 20);
}

static void OnApplyClicked(PatternProperties* self, psy_ui_Component* sender)
{
	if (self->pattern) {
		pattern_setlabel(self->pattern, ui_edit_text(&self->nameedit));
		pattern_setlength(self->pattern, (psy_dsp_beat_t)atof(ui_edit_text(&self->lengthedit)));
	}
}

static void OnCloseClicked(PatternProperties* self, psy_ui_Component* sender)
{
	PatternPropertiesSetPattern(self, self->pattern);
	ui_component_hide(&self->component);	
}
