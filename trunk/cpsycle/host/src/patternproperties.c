#include "../../detail/prefix.h"

#include "patternproperties.h"
#include <stdio.h>

static void OnSize(PatternProperties* self, ui_component* sender, int width, int height);
static void OnApplyClicked(PatternProperties* self, ui_component* sender);
static void OnCloseClicked(PatternProperties* self, ui_component* sender);

void InitPatternProperties(PatternProperties* self, ui_component* parent, Pattern* pattern)
{			
	self->pattern = pattern;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_connect(&self->component.signal_size, self, OnSize);	
	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Name");
	ui_label_init(&self->lengthlabel, &self->component);
	ui_label_settext(&self->lengthlabel, "Length");
	ui_edit_init(&self->nameedit, &self->component, 0);
	ui_edit_settext(&self->nameedit, "No Pattern");
	ui_edit_init(&self->lengthedit, &self->component, 0);
	ui_button_init(&self->applybutton, &self->component);
	ui_button_settext(&self->applybutton, "Apply");	
	signal_connect(&self->applybutton.signal_clicked, self, OnApplyClicked);	
	ui_button_init(&self->closebutton, &self->component);
	ui_button_settext(&self->closebutton, "x");	
	signal_connect(&self->closebutton.signal_clicked, self, OnCloseClicked);	
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

void PatternPropertiesSetPattern(PatternProperties* self, Pattern* pattern)
{
	char buffer[20];
	self->pattern = pattern;
	if (self->pattern) {
		ui_edit_settext(&self->nameedit, pattern->label);
		_snprintf(buffer, 10, "%.4f", self->pattern->length);
	} else {
		ui_edit_settext(&self->nameedit, "");
		_snprintf(buffer, 10, "");
	}
	ui_edit_settext(&self->lengthedit, buffer);
}

void OnSize(PatternProperties* self, ui_component* sender, int width, int height)
{
	ui_component_move(&self->closebutton.component, width - 25, 5);
	ui_component_resize(&self->closebutton.component, 20, 20);
}

static void OnApplyClicked(PatternProperties* self, ui_component* sender)
{
	if (self->pattern) {
		pattern_setlabel(self->pattern, ui_edit_text(&self->nameedit));
		pattern_setlength(self->pattern, (float)atof(ui_edit_text(&self->lengthedit)));
	}
}

static void OnCloseClicked(PatternProperties* self, ui_component* sender)
{
	PatternPropertiesSetPattern(self, self->pattern);
	ui_component_hide(&self->component);	
}
