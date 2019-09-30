#include "OctaveBar.h"

static void OnDestroy(OctaveBar*, ui_component* component);
static void BuildOctaveBox(OctaveBar* self);
static void OnOctaveBoxSelChange(OctaveBar*, ui_component* sender, int sel);
static void OnOctaveChanged(OctaveBar*, Workspace*, int octave);
static void OnSongChanged(OctaveBar*, Workspace*);

void InitOctaveBar(OctaveBar* self, ui_component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	ui_label_init(&self->headerlabel, &self->component);	
	ui_label_settext(&self->headerlabel, "Octave");		
	ui_combobox_init(&self->octavebox, &self->component);
	ui_combobox_setcharnumber(&self->octavebox, 2);	
	BuildOctaveBox(self);	
	signal_connect(&self->octavebox.signal_selchanged, self, OnOctaveBoxSelChange);	
	signal_connect(&workspace->signal_octavechanged, self, OnOctaveChanged);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	{
		List* children;
		ui_margin margin = { 0, 10, 3, 3 };
		
		children = ui_component_children(&self->component, 0);
		ui_components_setalign(children, UI_ALIGN_LEFT);
		ui_components_setmargin(children, &margin);
	}
}

void OnDestroy(OctaveBar* self, ui_component* component)
{
}

void BuildOctaveBox(OctaveBar* self)
{
	int octave;
	char text[20];

	for (octave = 0; octave < 9; ++octave) {
		_snprintf(text, 20, "%d", octave);		
		ui_combobox_addstring(&self->octavebox, text);
	}
	ui_combobox_setcursel(&self->octavebox, self->workspace->octave);
}

void OnOctaveBoxSelChange(OctaveBar* self, ui_component* sender, int sel)
{	
	workspace_setoctave(self->workspace, sel);
}

void OnOctaveChanged(OctaveBar* self, Workspace* workspace, int octave)
{
	ui_combobox_setcursel(&self->octavebox, workspace->octave);
}

void OnSongChanged(OctaveBar* self, Workspace* workspace)
{	
	ui_combobox_setcursel(&self->octavebox, workspace->octave);
}
